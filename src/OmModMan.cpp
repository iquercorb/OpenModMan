/*
  This file is part of Open Mod Manager.

  Open Mod Manager is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Mod Manager is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Open Mod Manager. If not, see <http://www.gnu.org/licenses/>.
*/
#include <ctime>

#include "OmBaseWin.h"
#include <ShlObj.h>

#include "OmBaseUi.h"
#include "OmBaseApp.h"


#include "OmUtilFs.h"
#include "OmUtilAlg.h"
#include "OmUtilStr.h"
#include "OmUtilHsh.h"
#include "OmUtilDlg.h"
#include "OmUtilErr.h"
#include "OmUtilSys.h"

#include "OmDialog.h"

#include "OmXmlConf.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmModMan.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModMan::OmModMan() :
  _active_hub(-1),
  _modlib_notify_cb(nullptr),
  _modlib_notify_ptr(nullptr),
  _netlib_notify_cb(nullptr),
  _netlib_notify_ptr(nullptr),
  _log_hfile(nullptr),
  _icon_size(16),
  _no_markdown(false)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModMan::~OmModMan()
{
  for(size_t i = 0; i < this->_hub_list.size(); ++i)
    delete this->_hub_list[i];

  // close log file
  if(this->_log_hfile) {
    CloseHandle(this->_log_hfile);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModMan::init(const char* arg)
{
  // Create application folder if does not exists
  wchar_t psz_path[MAX_PATH];
  SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, psz_path);

  this->_home = psz_path;
  this->_home.append(L"\\" OM_APP_NAME);

  // try to create directory (this should work)
  if(!Om_isDir(this->_home)) {

    int32_t result = Om_dirCreate(this->_home);
    if(result != 0) {
      this->_error(L"", Om_errCreate(L"Application data directory", this->_home, result));
      Om_dlgBox_err(L"Initialization error", L"Mod Manager initialization failed:", this->_lasterr);
      return false;
    }
  }

  // initialize log file
  OmWString log_path(this->_home + L"\\log.txt");

  // rename previous log file if exists
  if(Om_pathExists(log_path))
    Om_fileMove(log_path, this->_home + L"\\log.old.txt");

  this->_log_hfile = CreateFileW(log_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

  // Load existing configuration or create a new one
  if(!this->_xml.load(this->_home + L"\\config.xml", OM_XMAGIC_APP)) {

    this->_log(OM_LOG_WRN, L"Manager.init", L"missing configuration file, create new one");

    this->_xml.init(OM_XMAGIC_APP);

    OmWString conf_path = this->_home + L"\\config.xml";

    if(!this->_xml.save(conf_path)) {
      // this is not a fatal error, but this will surely be a problem...
      OmWString error_str = Om_errInit(L"Configuration file", conf_path, this->_xml.lastErrorStr());
      this->_log(OM_LOG_WRN, L"", error_str);
      Om_dlgBox_wrn(L"Initialization error", L"Mod Manager initialization error:", error_str);
    }

    // default icons size
    this->setIconsSize(this->_icon_size);
  }

  // migrate config file
  this->_migrate_120();

  // load saved parameters
  if(this->_xml.hasChild(L"icon_size")) {
    this->_icon_size = this->_xml.child(L"icon_size").attrAsInt(L"pixels");
  }

  // load saved no-markdown option
  if(this->_xml.hasChild(L"no_markdown")) {
    this->_no_markdown = this->_xml.child(L"no_markdown").attrAsInt(L"enable");
  }

  // load startup Mod Hub files if any
  bool autoload;
  OmWStringArray path_ls;

  this->getStartHubs(&autoload, path_ls);

  if(autoload) {

    OmWStringArray remv_ls; //< in case we must remove entries

    for(size_t i = 0; i < path_ls.size(); ++i) {

      if(OM_RESULT_OK != this->openHub(path_ls[i], false)) {

        Om_dlgBox_okl(nullptr, L"Hub startup load", IDI_DLG_ERR, L"Startup Hub open failed",
                      path_ls[i], this->lastError());

        if(Om_dlgBox_ynl(nullptr, L"Hub startup load", IDI_DLG_WRN, L"Remove invalid startup Hub",
                         L"The following Hub cannot be loaded, do you want to remove it from startup load list ?",
                         path_ls[i])) {

          remv_ls.push_back(path_ls[i]);
        }
      }
    }

    // Remove invalid startup Mod Hub
    if(remv_ls.size()) {

      for(size_t i = 0; i < remv_ls.size(); ++i)
        path_ls.erase(find(path_ls.begin(), path_ls.end(), remv_ls[i]));

      this->saveStartHubs(autoload, path_ls);
    }

    // if no active hub, select the last in list
    if(!this->activeHub())
      this->selectHub(this->_hub_list.size() - 1);
  }

  // load the Hub file passed as argument if any
  if(strlen(arg)) {

    // try to open
    OmResult result = this->openArg(arg, true);
    if(result != OM_RESULT_OK && result != OM_RESULT_PENDING) {

      // convert to UTF-16
      OmWString path; Om_fromAnsiCp(&path, arg);

      // check for quotes and removes them
      if(path.back() == L'"' && path.front() == L'"') {
        path.erase(0, 1); path.pop_back();
      }

      Om_dlgBox_err(L"Error opening file", L"Unable to open file \""+path+L"\":", this->lastError());
    }
  }

  this->_log(OM_LOG_OK, L"Manager.init", L"OK");

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModMan::openArg(const char* arg, bool select)
{
  // convert to UTF-16
  Om_fromAnsiCp(&this->_args, arg);

  // check for quotes and removes them
  if(this->_args.back() == L'"' && this->_args.front() == L'"') {
    this->_args.erase(0, 1); this->_args.pop_back();
  }

  this->_log(OM_LOG_OK, L"Manager.openArg", this->_args);

  OmXmlConf unknown_cfg;

  // check whether file is Hub definition
  if(unknown_cfg.load(this->_args, OM_XMAGIC_HUB)) {
    unknown_cfg.clear();
    this->_args.empty(); //< no pending arg
    return this->openHub(this->_args, select);
  }

  bool try_parent = false;
  bool arg_remain = false;

  // check whether file is a Mod Package
  if(Om_extensionMatches(this->_args, OM_PKG_FILE_EXT)) {
    try_parent = true;
    arg_remain = true; //< try open into editor
  }

  // check whether file is a Mod Backup
  if(!try_parent) {
    if(Om_extensionMatches(this->_args, OM_BCK_FILE_EXT)) {
      try_parent = true;
    }
  }

  // check whether file is Repository definition
  if(!try_parent) {
    if(unknown_cfg.load(this->_args, OM_XMAGIC_REP)) {
      unknown_cfg.clear();
      arg_remain = true; //< try open into editor
      try_parent = true;
    }
  }

  // check whether file is Channel definition
  if(!try_parent) {
    if(unknown_cfg.load(this->_args, OM_XMAGIC_CHN)) {
      unknown_cfg.clear();
      arg_remain = true; //< try to select channel
      try_parent = true;
    }
  }

  // check whether file is Preset definition
  if(!try_parent) {
    if(unknown_cfg.load(this->_args, OM_XMAGIC_PST)) {
      unknown_cfg.clear();
      try_parent = true;
    }
  }

  if(!try_parent) {
    this->_error(L"Manager.openArg", L"unknown file type");
    this->_args.empty(); //< no pending arg
    return OM_RESULT_ERROR;
  }

  // try to find Hub OMX file in parent directories
  OmWStringArray files;
  OmWString parent_dir = Om_getDirPart(this->_args); //< remove file name

  OmResult result = OM_RESULT_UNKNOW;

  uint8_t up_limit = 2; //< limit for up directory jump
  while(up_limit--) {

    // up to parent directory
    parent_dir = Om_getDirPart(parent_dir); //< remove last directory
    // in case we arrived at disk root
    if(parent_dir.size() < 5)
      break;

    files.clear();
    Om_lsFileFiltered(&files, parent_dir, L"*." OM_XML_DEF_EXT, true);

    for(size_t i = 0; i < files.size(); ++i) {
      if(unknown_cfg.load(files[i], OM_XMAGIC_HUB)) {
        unknown_cfg.clear();
        result = this->openHub(files[i], select);
        break;
      }
    }
  }

  if(!arg_remain)
    this->_args.empty(); //< no pending arg

  if(result == OM_RESULT_UNKNOW) {
    this->_error(L"Manager.openArg", L"no hub definition file found in expected location");
    result = OM_RESULT_ERROR;
  }

  return arg_remain ? OM_RESULT_PENDING : result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModMan::quit()
{
  for(size_t i = 0; i < this->_hub_list.size(); ++i)
    delete this->_hub_list[i];

  this->_hub_list.clear();

  this->_log(OM_LOG_OK, L"Manager.quit", L"goodbye");

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::saveWindowRect(const RECT& rect)
{
  if(!this->_xml.valid())
    return;

  OmXmlNode window;
  if(this->_xml.hasChild(L"window")) {
    window = this->_xml.child(L"window");
  } else {
    window = this->_xml.addChild(L"window");
  }

  window.setAttr(L"left", static_cast<int>(rect.left));
  window.setAttr(L"top", static_cast<int>(rect.top));
  window.setAttr(L"right", static_cast<int>(rect.right));
  window.setAttr(L"bottom", static_cast<int>(rect.bottom));

  this->_xml.save();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::loadWindowRect(RECT& rect)
{
  if(!this->_xml.valid())
    return;

  if(this->_xml.hasChild(L"window")) {

    OmXmlNode window = this->_xml.child(L"window");

    rect.left = window.attrAsInt(L"left");
    rect.top = window.attrAsInt(L"top");
    rect.right = window.attrAsInt(L"right");
    rect.bottom = window.attrAsInt(L"bottom");
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::saveWindowFoot(int h)
{
  if(!this->_xml.valid())
    return;

  OmXmlNode window;
  if(this->_xml.hasChild(L"window")) {

    window = this->_xml.child(L"window");

  } else {

    window = this->_xml.addChild(L"window");
  }

  window.setAttr(L"foot", h);

  this->_xml.save();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::loadWindowFoot(int* h)
{
  if(!this->_xml.valid())
    return;

  if(this->_xml.hasChild(L"window")) {

    OmXmlNode window = this->_xml.child(L"window");

    *h = window.attrAsInt(L"foot");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::addRecentFile(const OmWString& path)
{
  if(!this->_xml.valid())
    return;

  OmXmlNode recent_list_node;

  if(this->_xml.hasChild(L"recent_list")) {
    recent_list_node = this->_xml.child(L"recent_list");
  } else {
    recent_list_node = this->_xml.addChild(L"recent_list");
  }

  // get current <path> child entries in <recent_list>
  OmXmlNodeArray path_nodes;
  recent_list_node.children(path_nodes, L"path");

  for(size_t i = 0; i < path_nodes.size(); ++i) {
    if(path == path_nodes[i].content()) {
      recent_list_node.remChild(path_nodes[i]);
      break;
    }
  }

  // now verify the count does not exceed the limit
  if(recent_list_node.childCount() > (OM_MANAGER_MAX_RECENT + 1)) {
    // remove the oldest entry to keep max entry count
    recent_list_node.remChild(recent_list_node.child(L"path",0));
  }

  // append path to end of list, for most recent one
  recent_list_node.addChild(L"path").setContent(path);

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::clearRecentFileList()
{
  if(!this->_xml.valid())
    return;

  OmXmlNode recent_list;

  if(this->_xml.hasChild(L"recent_list")) {

    recent_list = this->_xml.child(L"recent_list");

  } else {

    recent_list = this->_xml.addChild(L"recent_list");
  }

  this->_xml.remChild(recent_list);

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::getRecentFileList(OmWStringArray& paths)
{
  if(!this->_xml.valid())
    return;

  if(!this->_xml.hasChild(L"recent_list"))
    return;

  OmXmlNode recent_list_node = this->_xml.child(L"recent_list");

  paths.clear();

  // retrieve all <path> child in <recent_list>
  OmXmlNodeArray path_nodes;
  recent_list_node.children(path_nodes, L"path");

  // verify each entries and remove ones which are no longer valid path
  for(size_t i = 0; i < path_nodes.size(); ++i)
    if(!Om_isFile(path_nodes[i].content()))
      recent_list_node.remChild(path_nodes[i]);

  // retrieve (again) all <path> child in <recent_list> and fill path list
  path_nodes.clear();

  recent_list_node.children(path_nodes, L"path");

  for(size_t i = 0; i < path_nodes.size(); ++i)
    paths.push_back(path_nodes[i].content());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModMan::removeRecentFile(const OmWString& path)
{
  if(!this->_xml.valid())
    return false;

  if(!this->_xml.hasChild(L"recent_list"))
    return false;

  OmXmlNode recent_list_node = this->_xml.child(L"recent_list");

  OmXmlNodeArray path_nodes;
  recent_list_node.children(path_nodes, L"path");

  bool has_remove = false;

  for(size_t i = 0; i < path_nodes.size(); ++i) {

    if(path_nodes[i].content() == path) {

      recent_list_node.remChild(path_nodes[i]);
      has_remove = true; break;
    }
  }

  this->_xml.save();

  return has_remove;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::saveDefaultLocation(const OmWString& path)
{
  if(!this->_xml.valid())
    return;

  if(this->_xml.hasChild(L"default_location")) {

    this->_xml.child(L"default_location").setContent(path);

  } else {

    this->_xml.addChild(L"default_location").setContent(path);
  }

  this->_xml.save();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::loadDefaultLocation(OmWString& path)
{
  if(!this->_xml.valid())
    return;

  if(this->_xml.hasChild(L"default_location")) {

    path = this->_xml.child(L"default_location").content();

  } else {

    wchar_t psz_path[OM_MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, psz_path);

    path = psz_path;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::saveStartHubs(bool enable, const OmWStringArray& path)
{
  if(!this->_xml.valid())
    return;

  OmXmlNode start_list_node;

  if(this->_xml.hasChild(L"start_list")) {

    start_list_node = this->_xml.child(L"start_list");

  } else {

    start_list_node = this->_xml.addChild(L"start_list");

  }

  start_list_node.setAttr(L"enable", enable ? 1 : 0);

  OmXmlNodeArray path_nodes;
  start_list_node.children(path_nodes, L"path");

  // remove all current file list
  for(size_t i = 0; i < path_nodes.size(); ++i)
    start_list_node.remChild(path_nodes[i]);

  // add new list
  for(size_t i = 0; i < path.size(); ++i)
    start_list_node.addChild(L"path").setContent(path[i]);

  this->_xml.save();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::getStartHubs(bool* enable, OmWStringArray& paths)
{
  paths.clear();

  if(!this->_xml.valid())
    return;

  OmXmlNode start_list_node;

  if(this->_xml.hasChild(L"start_list")) {

    start_list_node = this->_xml.child(L"start_list");

  } else {

    *enable = false;

    return;
  }

  *enable = start_list_node.attrAsInt(L"enable");

  OmXmlNodeArray path_nodes;
  start_list_node.children(path_nodes, L"path");

  // get list
  for(size_t i = 0; i < path_nodes.size(); ++i)
      paths.push_back(path_nodes[i].content());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModMan::removeStartHub(const OmWString& path)
{
  if(!this->_xml.valid())
    return false;

  if(!this->_xml.hasChild(L"start_list"))
    return false;

  OmXmlNode start_list_node = this->_xml.child(L"start_list");

  OmXmlNodeArray path_nodes;
  start_list_node.children(path_nodes, L"path");

  bool has_remove = false;

  for(size_t i = 0; i < path_nodes.size(); ++i) {

    if(path_nodes[i].content() == path) {

      start_list_node.remChild(path_nodes[i]);
      has_remove = true; break;
    }
  }

  this->_xml.save();

  return has_remove;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::addStartHub(const OmWString& path)
{
  if(!this->_xml.valid())
    return;

  OmXmlNode start_list_node;

  if(this->_xml.hasChild(L"start_list")) {

    start_list_node = this->_xml.child(L"start_list");

  } else {

    start_list_node = this->_xml.addChild(L"start_list");
  }

  // verify the path does not already exists
  OmXmlNodeArray path_nodes;
  start_list_node.children(path_nodes, L"path");

  for(size_t i = 0; i < path_nodes.size(); ++i)
    if(path_nodes[i].content() == path)
      return;

  start_list_node.addChild(L"path").setContent(path);

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::setIconsSize(unsigned size)
{
  this->_icon_size = size;

  if(!this->_xml.valid())
    return;

  if(this->_xml.hasChild(L"icon_size")) {

    this->_xml.child(L"icon_size").setAttr(L"pixels", (int)this->_icon_size);

  } else {

    this->_xml.addChild(L"icon_size").setAttr(L"pixels", (int)this->_icon_size);
  }

  this->_xml.save();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::setNoMarkdown(bool enable)
{
  this->_no_markdown = enable;

  if(!this->_xml.valid())
    return;

  if(this->_xml.hasChild(L"no_markdown")) {

    this->_xml.child(L"no_markdown").setAttr(L"enable", (int)this->_no_markdown);

  } else {

    this->_xml.addChild(L"no_markdown").setAttr(L"enable", (int)this->_no_markdown);
  }

  this->_xml.save();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModMan::createHub(const OmWString& path, const OmWString& name, bool open)
{
  // check whether install path exists
  if(!Om_isDir(path)) {
    this->_error(L"createHub", Om_errNotDir(L"home location", path));
    return OM_RESULT_ERROR;
  }

  // compose Mod Hub home path
  OmWString hub_home = Om_concatPaths(path, name);

  // create Mod Hub home folder
  if(!Om_isDir(hub_home)) {
    int32_t result = Om_dirCreate(hub_home);
    if(result != 0) {
      this->_error(L"createHub", Om_errCreate(L"home directory", hub_home, result));
      return OM_RESULT_ERROR_IO;
    }
  } else {
    if(Om_isDirEmpty(hub_home)) {
      this->_log(OM_LOG_WRN, L"createHub", L"chosen home directory already exists");
    } else {
      this->_error(L"createHub", L"chosen home directory already exists and is not empty");
      return OM_RESULT_ABORT;
    }
  }

  // initialize Mod Hub definition file
  OmXmlConf modhub_cfg(OM_XMAGIC_HUB);

  // create <uuid> and <title> nodes as base parameters
  modhub_cfg.addChild(L"uuid").setContent(Om_genUUID());
  modhub_cfg.addChild(L"title").setContent(name);

  // compose Hub definition file name
  OmWString hub_path = Om_concatPaths(hub_home, OM_MODHUB_FILENAME);

  // save and close definition file
  if(!modhub_cfg.save(hub_path)) {
    this->_error(L"createHub", Om_errSave(L"Definition file", hub_path, modhub_cfg.lastErrorStr()));
    return OM_RESULT_ERROR_IO;
  }

  // open the new created Mod Hub
  if(open)
    return this->openHub(hub_path);

  return OM_RESULT_OK;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModMan::openHub(const OmWString& path, bool select)
{
  this->_log(OM_LOG_OK, L"Manager.openHub", path);

  // check whether Mod Hub is already opened
  for(size_t i = 0; i < this->_hub_list.size(); ++i)
    if(Om_namesMatches(this->_hub_list[i]->path(), path)) {

      if(select)
        this->selectHub(i);

      return OM_RESULT_OK;
    }

  OmModHub* ModHub = new OmModHub(this);

  if(!ModHub->open(path)) {
    this->_error(L"openHub", ModHub->lastError());
    delete ModHub; return OM_RESULT_ERROR;
  }

  this->_hub_list.push_back(ModHub);

  this->addRecentFile(path);

  // the last loaded context become the active one
  if(select)
    this->selectHub(this->_hub_list.size() - 1);

  return OM_RESULT_OK;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::closeHub(int32_t index)
{
  if(index < 0) {

    if(this->_active_hub < 0)
      return;

    index = this->_active_hub;

    this->_active_hub = -1;
  }

  if(index < static_cast<int>(this->_hub_list.size())) {

    this->_log(OM_LOG_OK, L"Manager.closeHub", this->_hub_list[index]->home());

    this->_hub_list[index]->close();

    delete _hub_list[index];

    this->_hub_list.erase(this->_hub_list.begin() + index);
  }

  // the last loaded context become the active one
  this->selectHub(this->_hub_list.size() - 1);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::selectHub(int32_t index)
{
  if(index >= 0 && index < static_cast<int32_t>(this->_hub_list.size())) {

    // disable library notifications from previous hub
    this->_modlib_notify_enable(false);
    this->_netlib_notify_enable(false);

    this->_active_hub = index;

    // enable library notifications
    this->_modlib_notify_enable(true);
    this->_netlib_notify_enable(true);

  } else {

    // disable library notifications from previous hub
    this->_modlib_notify_enable(false);
    this->_netlib_notify_enable(false);

    this->_active_hub = -1;

  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModHub* OmModMan::activeHub() const
{
  if(this->_active_hub >= 0)
      return this->_hub_list[this->_active_hub];

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmModMan::indexOfHub(const OmModHub* ModHub)
{
  for(size_t i = 0; i < this->_hub_list.size(); ++i)
    if(ModHub == this->_hub_list[i])
      return i;

  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModChan* OmModMan::activeChannel() const
{
  if(this->_active_hub >= 0)
    return this->_hub_list[this->_active_hub]->activeChannel();

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::_modlib_notify_enable(bool enable)
{
  if(this->_active_hub >= 0) {

    OmModHub* ModHub = this->_hub_list[this->_active_hub];

    if(enable) {
      ModHub->notifyModLibraryStart(OmModMan::_modlib_notify_fn, this);
    } else {
      ModHub->notifyModLibraryStop();
    }

  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::_modlib_notify_fn(void* ptr, OmNotify notify, uint64_t param)
{
  OmModMan* self = static_cast<OmModMan*>(ptr);

  // call client callback
  if(self->_modlib_notify_cb)
    self->_modlib_notify_cb(self->_modlib_notify_ptr, notify, param);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::notifyModLibraryStart(Om_notifyCb notify_cb,  void* user_ptr)
{
  this->_modlib_notify_cb = notify_cb;
  this->_modlib_notify_ptr = user_ptr;

  this->_modlib_notify_enable(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::notifyModLibraryStop()
{
  this->_modlib_notify_cb = nullptr;
  this->_modlib_notify_ptr = nullptr;

  this->_modlib_notify_enable(false);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::_netlib_notify_enable(bool enable)
{
  if(this->_active_hub >= 0) {

    OmModHub* ModHub = this->_hub_list[this->_active_hub];

    if(enable) {
      ModHub->notifyNetLibraryStart(OmModMan::_netlib_notify_fn, this);
    } else {
      ModHub->notifyNetLibraryStop();
    }

  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::_netlib_notify_fn(void* ptr, OmNotify notify, uint64_t param)
{
  OmModMan* self = static_cast<OmModMan*>(ptr);

  // call client callback
  if(self->_netlib_notify_cb)
    self->_netlib_notify_cb(self->_netlib_notify_ptr, notify, param);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::notifyNetLibraryStart(Om_notifyCb notify_cb, void* user_ptr)
{
  this->_netlib_notify_cb = notify_cb;
  this->_netlib_notify_ptr = user_ptr;

  this->_netlib_notify_enable(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::notifyNetLibraryStop()
{
  this->_netlib_notify_cb = nullptr;
  this->_netlib_notify_ptr = nullptr;

  this->_netlib_notify_enable(false);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::addLogNotify(Om_notifyCb notify_cb, void* user_ptr)
{
  if(!Om_arrayContain(this->_log_notify_cb, notify_cb)) {

    this->_log_notify_cb.push_back(notify_cb);

    this->_log_user_ptr.push_back(user_ptr);

  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::removeLogNotify(Om_notifyCb notify_cb)
{
  for(size_t i = 0; i < this->_log_notify_cb.size(); ++i) {

    if(this->_log_notify_cb[i] == notify_cb) {

      this->_log_notify_cb.erase(this->_log_notify_cb.begin()+i);

      this->_log_user_ptr.erase(this->_log_user_ptr.begin()+i);

      break;
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::_log(unsigned level, const OmWString& origin, const OmWString& detail)
{
  // get local time
  int t_h, t_m, t_s;
  Om_getTime(&t_s, &t_m, &t_h);

  wchar_t hour[32];
  swprintf(hour, 32, L"[%02d:%02d:%02d]", t_h, t_m, t_s);

  // initialize with time stamp
  OmWString log_entry = hour;

  // add level code
  switch(level) {
  case 0:   log_entry += L" X "; break;
  case 1:   log_entry += L" ! "; break;
  default:  log_entry += L"   "; break;
  }

  // concatenate elements and finalize
  log_entry += origin;

  if(detail.size()) {
    log_entry += L": "; log_entry += detail;
  }

  log_entry += L"\r\n";

  #ifdef DEBUG
  std::wcout << log_entry; //< print to standard output
  #endif

  // send new log to callback functions
  for(size_t i = 0; i < this->_log_notify_cb.size(); ++i)
    this->_log_notify_cb[i](this->_log_user_ptr[i], OM_NOTIFY_CREATED, reinterpret_cast<uint64_t>(log_entry.c_str()));

  // write to log file
  if(this->_log_hfile) {

    DWORD wb;

    OmCString utf8_entry = Om_toUTF8(log_entry);

    WriteFile(this->_log_hfile, utf8_entry.c_str(), utf8_entry.size(), &wb, nullptr);
  }

  this->_log_str += log_entry;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::_error(const OmWString& origin, const OmWString& detail)
{
  this->_lasterr = detail;
  this->_log(OM_LOG_ERR, origin, detail);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModMan::_migrate_120()
{
  if(!this->_xml.valid())
    return false;

  // Migrate startup Mod Hub list if required
  if(this->_xml.hasChild(L"start_list")) {

    OmXmlNode start_list = this->_xml.child(L"start_list");

    if(start_list.hasChild(L"file")) {

      OmXmlNodeArray file_ls;
      start_list.children(file_ls, L"file");

      OmWStringArray temp_ls;

      // get list
      for(size_t i = 0; i < file_ls.size(); ++i)
        temp_ls.push_back(file_ls[i].content());

      // remove all current file list
      for(size_t i = 0; i < file_ls.size(); ++i)
        start_list.remChild(file_ls[i]);

      // add new list
      for(size_t i = 0; i < temp_ls.size(); ++i)
        start_list.addChild(L"path").setContent(temp_ls[i]);
    }
  }
  // Migrate startup Mod Hub list if required
  if(this->_xml.hasChild(L"recent_list")) {

    OmXmlNode recent_list = this->_xml.child(L"recent_list");

    if(recent_list.hasChild(L"file")) {

      OmXmlNodeArray file_ls;
      recent_list.children(file_ls, L"file");

      OmWStringArray temp_ls;

      // get list
      for(size_t i = 0; i < file_ls.size(); ++i)
        temp_ls.push_back(file_ls[i].content());

      // remove all current file list
      for(size_t i = 0; i < file_ls.size(); ++i)
        recent_list.remChild(file_ls[i]);

      // add new list
      for(size_t i = 0; i < temp_ls.size(); ++i)
        recent_list.addChild(L"path").setContent(temp_ls[i]);
    }
  }

  this->_xml.save();


  return true;
}
