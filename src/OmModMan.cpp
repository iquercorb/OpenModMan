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

#include "OmBaseApp.h"

#include "OmUtilFs.h"
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
  _applog_hfile(nullptr),
  _active_hub(-1),
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
  if(this->_applog_hfile) {
    CloseHandle(this->_applog_hfile);
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
      Om_dlgBox_err(L"Initialization", L"Mod Manager initialization failed", this->_lasterr);
      return false;
    }
  }

  // initialize log file
  OmWString log_path(this->_home + L"\\log.txt");

  // rename previous log file if exists
  if(Om_pathExists(log_path))
    Om_fileMove(log_path, this->_home + L"\\log.old.txt");

  this->_applog_hfile = CreateFileW(log_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

  // Load existing configuration or create a new one
  if(!this->_xmlconf.load(this->_home + L"\\config.xml", OM_XMAGIC_APP)) {

    this->_log(OM_LOG_WRN, L"", L"Missing configuration file, create new one");

    this->_xmlconf.init(OM_XMAGIC_APP);

    OmWString conf_path = this->_home + L"\\config.xml";

    if(!this->_xmlconf.save(conf_path)) {
      // this is not a fatal error, but this will surely be a problem...
      OmWString error_str = Om_errInit(L"Configuration file", conf_path, this->_xmlconf.lastErrorStr());
      this->_log(OM_LOG_WRN, L"", error_str);
      Om_dlgBox_wrn(L"Initialization", L"Mod Manager initialization error", error_str);
    }

    // default icons size
    this->setIconsSize(this->_icon_size);
  }

  // migrate config file
  this->_migrate();

  // load saved parameters
  if(this->_xmlconf.hasChild(L"icon_size")) {
    this->_icon_size = this->_xmlconf.child(L"icon_size").attrAsInt(L"pixels");
  }

  // load saved no-markdown option
  if(this->_xmlconf.hasChild(L"no_markdown")) {
    this->_no_markdown = this->_xmlconf.child(L"no_markdown").attrAsInt(L"enable");
  }

  // add the context file passed as argument if any
  if(strlen(arg)) {

    // convert to OmWString
    OmWString path;
    Om_fromAnsiCp(&path, arg);

    // check for quotes and removes them
    if(path.back() == L'"' && path.front() == L'"') {
      path.erase(0, 1);
      path.pop_back();
    }

    // try to open
    if(!this->openHub(path)) {
      Om_dlgBox_err(L"Open Mod Hub", L"Mod Hub \""+path+
                    L"\" loading failed because of the following error:",
                    this->lastError());
    }
  }

  this->_log(OM_LOG_OK, L"", L"Mod Manager successfully initialized");

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModMan::quit()
{
  for(size_t i = 0; i < this->_hub_list.size(); ++i)
    delete this->_hub_list[i];

  this->_hub_list.clear();

  this->_log(OM_LOG_OK, L"", L"Mod Man quit");

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::saveWindowRect(const RECT& rect)
{
  if(this->_xmlconf.valid()) {

    OmXmlNode window;
    if(this->_xmlconf.hasChild(L"window")) {
      window = this->_xmlconf.child(L"window");
    } else {
      window = this->_xmlconf.addChild(L"window");
    }

    window.setAttr(L"left", static_cast<int>(rect.left));
    window.setAttr(L"top", static_cast<int>(rect.top));
    window.setAttr(L"right", static_cast<int>(rect.right));
    window.setAttr(L"bottom", static_cast<int>(rect.bottom));

    this->_xmlconf.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::loadWindowRect(RECT& rect)
{
  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"window")) {

      OmXmlNode window = this->_xmlconf.child(L"window");

      rect.left = window.attrAsInt(L"left");
      rect.top = window.attrAsInt(L"top");
      rect.right = window.attrAsInt(L"right");
      rect.bottom = window.attrAsInt(L"bottom");
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::saveWindowFoot(int h)
{
  if(this->_xmlconf.valid()) {

    OmXmlNode window;
    if(this->_xmlconf.hasChild(L"window")) {
      window = this->_xmlconf.child(L"window");
    } else {
      window = this->_xmlconf.addChild(L"window");
    }

    window.setAttr(L"foot", h);

    this->_xmlconf.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::loadWindowFoot(int* h)
{
  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"window")) {

      OmXmlNode window = this->_xmlconf.child(L"window");

      *h = window.attrAsInt(L"foot");
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::saveRecentFile(const OmWString& path)
{
  if(this->_xmlconf.valid()) {

    OmXmlNode recent_list;
    if(this->_xmlconf.hasChild(L"recent_list")) {
      recent_list = this->_xmlconf.child(L"recent_list");
    } else {
      recent_list = this->_xmlconf.addChild(L"recent_list");
    }

    // get current <path> child entries in <recent_list>
    OmXmlNodeArray home_ls;
    recent_list.children(home_ls, L"home");

    for(size_t i = 0; i < home_ls.size(); ++i) {
      if(path == home_ls[i].content()) {
        recent_list.remChild(home_ls[i]);
        break;
      }
    }

    // now verify the count does not exceed the limit
    if(recent_list.childCount() > (OM_MANAGER_MAX_RECENT + 1)) {
      // remove the oldest entry to keep max entry count
      recent_list.remChild(recent_list.child(L"home",0));
    }

    // append path to end of list, for most recent one
    recent_list.addChild(L"home").setContent(path);

    this->_xmlconf.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::clearRecentFiles()
{
  if(this->_xmlconf.valid()) {

    OmXmlNode recent_list;

    if(this->_xmlconf.hasChild(L"recent_list")) {
      recent_list = this->_xmlconf.child(L"recent_list");
    } else {
      recent_list = this->_xmlconf.addChild(L"recent_list");
    }

    this->_xmlconf.remChild(recent_list);

    this->_xmlconf.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::loadRecentFiles(OmWStringArray& paths)
{
  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"recent_list")) {

      OmXmlNode recent_list = this->_xmlconf.child(L"recent_list");

      paths.clear();

      // retrieve all <path> child in <recent_list>
      OmXmlNodeArray home_ls;
      recent_list.children(home_ls, L"home");

      // verify each entries and remove ones which are no longer valid path
      for(size_t i = 0; i < home_ls.size(); ++i)
        if(!Om_isDir(home_ls[i].content()))
          recent_list.remChild(home_ls[i]);

      // retrieve (again) all <path> child in <recent_list> and fill path list
      home_ls.clear();

      recent_list.children(home_ls, L"home");

      for(size_t i = 0; i < home_ls.size(); ++i)
        paths.push_back(home_ls[i].content());

    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::saveDefaultLocation(const OmWString& path)
{
  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"default_location")) {
      this->_xmlconf.child(L"default_location").setContent(path);
    } else {
      this->_xmlconf.addChild(L"default_location").setContent(path);
    }

    this->_xmlconf.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::loadDefaultLocation(OmWString& path)
{
  if(this->_xmlconf.valid()) {
    if(this->_xmlconf.hasChild(L"default_location")) {
      path = this->_xmlconf.child(L"default_location").content();
    } else {
      wchar_t psz_path[MAX_PATH];
      SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, psz_path);
      path = psz_path;
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::saveStartHubs(bool enable, const OmWStringArray& path)
{
  if(this->_xmlconf.valid()) {

    OmXmlNode start_list;

    if(this->_xmlconf.hasChild(L"start_list")) {
      start_list = this->_xmlconf.child(L"start_list");
    } else {
      start_list = this->_xmlconf.addChild(L"start_list");
    }
    start_list.setAttr(L"enable", enable ? 1 : 0);

    OmXmlNodeArray home_ls;
    start_list.children(home_ls, L"home");

    // remove all current file list
    for(size_t i = 0; i < home_ls.size(); ++i)
      start_list.remChild(home_ls[i]);

    // add new list
    for(size_t i = 0; i < path.size(); ++i)
      start_list.addChild(L"home").setContent(path[i]);

    this->_xmlconf.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::loadStartHubs(bool* enable, OmWStringArray& path)
{
  path.clear();

  if(this->_xmlconf.valid()) {

    OmXmlNode start_list;
    if(this->_xmlconf.hasChild(L"start_list")) {
      start_list = this->_xmlconf.child(L"start_list");
    } else {
      *enable = false;
      return;
    }

    *enable = start_list.attrAsInt(L"enable");

    OmXmlNodeArray path_ls;
    start_list.children(path_ls, L"home");

    // get list
    for(size_t i = 0; i < path_ls.size(); ++i)
      path.push_back(path_ls[i].content());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::setIconsSize(unsigned size)
{
  this->_icon_size = size;

  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"icon_size")) {
      this->_xmlconf.child(L"icon_size").setAttr(L"pixels", (int)this->_icon_size);
    } else {
      this->_xmlconf.addChild(L"icon_size").setAttr(L"pixels", (int)this->_icon_size);
    }

    this->_xmlconf.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::setNoMarkdown(bool enable)
{
  this->_no_markdown = enable;

  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"no_markdown")) {
      this->_xmlconf.child(L"no_markdown").setAttr(L"enable", (int)this->_no_markdown);
    } else {
      this->_xmlconf.addChild(L"no_markdown").setAttr(L"enable", (int)this->_no_markdown);
    }

    this->_xmlconf.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModMan::createHub(const OmWString& path, const OmWString& name, bool open)
{
  // check whether install path exists
  if(!Om_isDir(path)) {
    this->_error(L"createHub", Om_errNotDir(L"Home location", path));
    return false;
  }

  // compose Mod Hub home path
  OmWString hub_home(path + L"\\" + name);

  // create Mod Hub home folder
  int32_t result = Om_dirCreate(hub_home);
  if(result != 0) {
    this->_error(L"createHub", Om_errCreate(L"Home directory", hub_home, result));
    return false;
  }

  // compose Mod Hub definition file name
  OmWString hub_path = hub_home + L"\\ModHub.xml";

  // initialize Mod Hub definition file
  OmXmlConf modhub_cfg(OM_XMAGIC_HUB);

  // create <uuid> and <title> nodes as base parameters
  modhub_cfg.addChild(L"uuid").setContent(Om_genUUID());
  modhub_cfg.addChild(L"title").setContent(name);


  // save and close definition file
  OmWString hub_file = hub_home + L"\\ModHub.xml";
  if(!modhub_cfg.save(hub_file)) {
    this->_error(L"createHub", Om_errSave(L"Definition file", hub_file, modhub_cfg.lastErrorStr()));
    return false;
  }

  // open the new created Mod Hub
  if(open)
    return this->openHub(hub_home);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModMan::openHub(const OmWString& path, bool select)
{
  // check whether Mod Hub is already opened
  for(size_t i = 0; i < this->_hub_list.size(); ++i)
    if(path == this->_hub_list[i]->path())
      return true;

  OmModHub* ModHub = new OmModHub(this);
  if(!ModHub->open(path)) {
    this->_error(L"openHub", ModHub->lastError());
    delete ModHub; return false;
  }

  this->_hub_list.push_back(ModHub);

  this->saveRecentFile(path);

  // the last loaded context become the active one
  if(select)
    this->selectHub(this->_hub_list.size() - 1);

  return true;
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
    this->_active_hub = index;
  } else {
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
OmModChan* OmModMan::activeChannel() const
{
  if(this->_active_hub >= 0)
    return this->_hub_list[this->_active_hub]->activeChannel();

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::addLogCallback(Om_onlogCb onlog_cb, void* user_ptr)
{
  this->_applog_cli_onlog.push_back(onlog_cb);
  this->_applog_cli_ptr.push_back(user_ptr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::removeLogCallback(Om_onlogCb onlog_cb)
{
  for(size_t i = 0; i < this->_applog_cli_onlog.size(); ++i) {

    if(this->_applog_cli_onlog[i] == onlog_cb) {

      this->_applog_cli_onlog.erase(this->_applog_cli_onlog.begin()+i);
      this->_applog_cli_ptr.erase(this->_applog_cli_ptr.begin()+i);
      break;
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModMan::_migrate()
{
  if(this->_xmlconf.valid()) {

    OmWStringArray temp_ls;

    // Migrate startup Mod Hub list if required
    OmXmlNode start_list;
    if(this->_xmlconf.hasChild(L"start_list")) {

      start_list = this->_xmlconf.child(L"start_list");

      if(start_list.hasChild(L"file")) {

        OmXmlNodeArray file_ls;
        start_list.children(file_ls, L"file");

        // get list
        for(size_t i = 0; i < file_ls.size(); ++i)
          temp_ls.push_back(file_ls[i].content());

        // remove all current file list
        for(size_t i = 0; i < file_ls.size(); ++i)
          start_list.remChild(file_ls[i]);

        // add new list
        for(size_t i = 0; i < temp_ls.size(); ++i)
          start_list.addChild(L"home").setContent(Om_getDirPart(temp_ls[i]));

        temp_ls.clear();
      }
    }

    // Migrate recent Mod Hub list if required
    if(this->_xmlconf.hasChild(L"recent_list")) {

      OmXmlNode recent_list = this->_xmlconf.child(L"recent_list");

      if(recent_list.hasChild(L"path")) {

        OmXmlNodeArray path_ls;
        recent_list.children(path_ls, L"path");

        // get list
        for(size_t i = 0; i < path_ls.size(); ++i)
          temp_ls.push_back(path_ls[i].content());

        // remove all current file list
        for(size_t i = 0; i < path_ls.size(); ++i)
          recent_list.remChild(path_ls[i]);

        // add new list
        for(size_t i = 0; i < temp_ls.size(); ++i)
          recent_list.addChild(L"home").setContent(Om_getDirPart(temp_ls[i]));
      }
    }

    this->_xmlconf.save();
  }

  return true;
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
  for(size_t i = 0; i < this->_applog_cli_onlog.size(); ++i)
    this->_applog_cli_onlog[i](this->_applog_cli_ptr[i], log_entry);

  // write to log file
  if(this->_applog_hfile) {

    DWORD wb;

    OmCString utf8_entry = Om_toUTF8(log_entry);

    WriteFile(this->_applog_hfile, utf8_entry.c_str(), utf8_entry.size(), &wb, nullptr);
  }

  this->_applog_str += log_entry;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModMan::_error(const OmWString& origin, const OmWString& detail)
{
  this->_lasterr = detail;
  this->_log(OM_LOG_ERR, origin, detail);
}
