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

#include "OmManager.h"
#include "OmDialog.h"
#include <time.h>


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmManager::OmManager() :
  _home(),
  _config(),
  _context(),
  _curContext(nullptr),
  _iconsSize(16),
  _folderPackages(true),
  _warnEnabled(true),
  _warnOverlaps(false),
  _warnExtraInstall(true),
  _warnMissingDepend(true),
  _warnExtraUninst(true),
  _quietBatches(true),
  _error(),
  _log(),
  _logHwnd(nullptr),
  _logFile(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmManager::~OmManager()
{
  for(unsigned i = 0; i < this->_context.size(); ++i)
    delete this->_context[i];

  // close log file
  if(this->_logFile) {
    CloseHandle(reinterpret_cast<HANDLE>(this->_logFile));
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmManager::init()
{
  // Create application folder if does not exists
  wchar_t wcbuf[OMM_MAX_PATH];
  SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, wcbuf);
  this->_home = wcbuf;
  this->_home += L"\\";
  this->_home += OMM_APP_NAME;

  // for application home directory creation result
  int result;

  // try to create directory (this should work)
  if(!Om_isDir(this->_home)) {

    result = Om_dirCreate(this->_home);
    if(result != 0) {
      this->_error = L"Unable to create home directory \"";
      this->_error += this->_home + L"\": ";
      this->_error += Om_getErrorStr(result);
      Om_dialogBoxErr(nullptr, L"Manager Initialization error", this->_error);
      return false;
    }
  }

  // initialize log file
  swprintf(wcbuf, OMM_MAX_PATH, L"%ls\\log.txt", this->_home.c_str());
  this->_logFile = CreateFileW(wcbuf, GENERIC_WRITE, 0, nullptr,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  // first log line
  this->log(2, L"Manager", L"Initialization");

  // Load existing configuration or create a new one
  if(!this->_config.open(this->_home + L"\\config.xml", OMM_CFG_SIGN_APP)) {

    this->log(2, L"Manager", L"Create new configuration file from scratch.");

    if(!_config.init(this->_home + L"\\config.xml", OMM_CFG_SIGN_APP)) {
      this->_error = L"Unable to initialize main configuration file \"";
      this->_error += this->_home + L"\\config.xml\": ";
      this->_error += this->_config.lastErrorStr();
      this->log(1, L"Manager", this->_error);
      Om_dialogBoxWarn(nullptr, L"Manager Initialization error", this->_error);
      // this is not a fatal error, but this will surely be a problem...
    }
    // default icons size
    this->setIconsSize(this->_iconsSize);
    // default package options
    this->setLegacySupport(this->_folderPackages);
    this->setWarnOverlaps(this->_warnOverlaps);
    this->setWarnExtraInst(this->_warnExtraInstall);
    this->setWarnMissDpnd(this->_warnMissingDepend);
    this->setWarnExtraUnin(this->_warnExtraUninst);
    this->setQuietBatches(this->_quietBatches);
  }

  // load saved parameters
  if(this->_config.xml().hasChild(L"icon_size")) {
    this->_iconsSize = this->_config.xml().child(L"icon_size").attrAsInt(L"pixels");
  }
  if(this->_config.xml().hasChild(L"legacy_support")) {
    this->_folderPackages = this->_config.xml().child(L"legacy_support").attrAsInt(L"enable");
  }
  if(this->_config.xml().hasChild(L"warn_overlaps")) {
    this->_warnOverlaps = this->_config.xml().child(L"warn_overlaps").attrAsInt(L"enable");
  }
  if(this->_config.xml().hasChild(L"warn_extra_inst")) {
    this->_warnExtraInstall = this->_config.xml().child(L"warn_extra_inst").attrAsInt(L"enable");
  }
  if(this->_config.xml().hasChild(L"warn_miss_dpnd")) {
    this->_warnMissingDepend = this->_config.xml().child(L"warn_miss_dpnd").attrAsInt(L"enable");
  }
  if(this->_config.xml().hasChild(L"warn_extra_unin")) {
    this->_warnExtraUninst = this->_config.xml().child(L"warn_extra_unin").attrAsInt(L"enable");
  }
  if(this->_config.xml().hasChild(L"quiet_batches")) {
    this->_quietBatches = this->_config.xml().child(L"quiet_batches").attrAsInt(L"enable");
  }

  // load startup Context files if any
  bool autoload;
  vector<wstring> start_files;
  this->getStartContexts(&autoload, start_files);
  if(autoload) {
    this->log(2, L"Manager", L"Load startup file(s)");
    for(unsigned i = 0; i < start_files.size(); ++i) {
      this->openContext(start_files[i]);
    }
    // select the last loaded Context
    this->selContext(this->contextCount()-1);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmManager::quit()
{
  this->log(2, L"Manager", L"Quit");

  for(unsigned i = 0; i < this->_context.size(); ++i)
    delete this->_context[i];
  this->_context.clear();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::saveWindowRect(const RECT& rect)
{
  if(this->_config.valid()) {

    OmXmlNode window;
    if(this->_config.xml().hasChild(L"window")) {
      window = this->_config.xml().child(L"window");
    } else {
      window = this->_config.xml().addChild(L"window");
    }

    window.setAttr(L"left", (int)rect.left);
    window.setAttr(L"top", (int)rect.top);
    window.setAttr(L"right", (int)rect.right);
    window.setAttr(L"bottom", (int)rect.bottom);

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::loadWindowRect(RECT& rect)
{
  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"window")) {

      OmXmlNode window = this->_config.xml().child(L"window");

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
void OmManager::saveRecentFile(const wstring& path)
{
  if(this->_config.valid()) {

    OmXmlNode recent_list;
    if(this->_config.xml().hasChild(L"recent_list")) {
      recent_list = this->_config.xml().child(L"recent_list");
    } else {
      recent_list = this->_config.xml().addChild(L"recent_list");
    }

    // get current <path> child entries in <recent_list>
    vector<OmXmlNode> path_list;
    recent_list.children(path_list, L"path");

    for(unsigned i = 0; i < path_list.size(); ++i) {
      if(path == path_list[i].content()) {
        recent_list.remChild(path_list[i]);
        break;
      }
    }

    // now verify the count does not exceed the limit
    if(recent_list.childCount() > (OM_MANAGER_MAX_RECENT + 1)) {
      // remove the oldest entry to keep max entry count
      recent_list.remChild(recent_list.child(L"path",0));
    }

    // append path to end of list, for most recent one
    recent_list.addChild(L"path").setContent(path);

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::clearRecentFiles()
{
  if(this->_config.valid()) {

    OmXmlNode recent_list;
    if(this->_config.xml().hasChild(L"recent_list")) {
      recent_list = this->_config.xml().child(L"recent_list");
    } else {
      recent_list = this->_config.xml().addChild(L"recent_list");
    }

    this->_config.xml().remChild(recent_list);

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::loadRecentFiles(vector<wstring>& paths)
{
  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"recent_list")) {

      OmXmlNode recent_list = this->_config.xml().child(L"recent_list");

      paths.clear();

      // retrieve all <path> child in <recent_list>
      vector<OmXmlNode> path_list;
      recent_list.children(path_list, L"path");

      // verify each entries and remove ones which are no longer valid path
      for(unsigned i = 0; i < path_list.size(); ++i) {
        if(!Om_isFile(path_list[i].content())) {
          recent_list.remChild(path_list[i]);
        }
      }

      // retrieve (again) all <path> child in <recent_list> and fill path list
      path_list.clear();
      recent_list.children(path_list, L"path");
      for(unsigned i = 0; i < path_list.size(); ++i) {
        paths.push_back(path_list[i].content());
      }

    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::saveDefaultLocation(const wstring& path)
{
  if(this->_config.valid()) {
    if(this->_config.xml().hasChild(L"default_location")) {
      this->_config.xml().child(L"default_location").setContent(path);
    } else {
      this->_config.xml().addChild(L"default_location").setContent(path);
    }
    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::getDefaultLocation(wstring& path)
{
  if(this->_config.valid()) {
    if(this->_config.xml().hasChild(L"default_location")) {
      path = this->_config.xml().child(L"default_location").content();
    } else {
      wchar_t wcbuf[OMM_MAX_PATH];
      SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, wcbuf);
      path = wcbuf;
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::saveStartContexts(bool enable, const vector<wstring>& path)
{
  if(this->_config.valid()) {

    OmXmlNode start_list;
    if(this->_config.xml().hasChild(L"start_list")) {
      start_list = this->_config.xml().child(L"start_list");
    } else {
      start_list = this->_config.xml().addChild(L"start_list");
    }
    start_list.setAttr(L"enable", (int)enable);

    vector<OmXmlNode> start_file;
    start_list.children(start_file, L"file");

    // remove all current file list
    for(unsigned i = 0; i < start_file.size(); ++i)
      start_list.remChild(start_file[i]);

    // add new list
    for(unsigned i = 0; i < path.size(); ++i)
      start_list.addChild(L"file").setContent(path[i]);

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::getStartContexts(bool* enable, vector<wstring>& path)
{
  path.clear();

  if(this->_config.valid()) {

    OmXmlNode start_list;
    if(this->_config.xml().hasChild(L"start_list")) {
      start_list = this->_config.xml().child(L"start_list");
    } else {
      *enable = false;
      return;
    }

    *enable = start_list.attrAsInt(L"enable");

    vector<OmXmlNode> start_file;
    start_list.children(start_file, L"file");

    // get list
    for(unsigned i = 0; i < start_file.size(); ++i)
      path.push_back(start_file[i].content());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::setIconsSize(unsigned size)
{
  this->_iconsSize = size;

  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"icon_size")) {
      this->_config.xml().child(L"icon_size").setAttr(L"pixels", (int)this->_iconsSize);
    } else {
      this->_config.xml().addChild(L"icon_size").setAttr(L"pixels", (int)this->_iconsSize);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::setLegacySupport(bool enable)
{
  this->_folderPackages = enable;

  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"legacy_support")) {
      this->_config.xml().child(L"legacy_support").setAttr(L"enable", (this->_folderPackages)?L"1":L"0");
    } else {
      this->_config.xml().addChild(L"legacy_support").setAttr(L"enable", (this->_folderPackages)?L"1":L"0");
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::setWarnOverlaps(bool enable)
{
  this->_warnOverlaps = enable;

  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"warn_overlaps")) {
      this->_config.xml().child(L"warn_overlaps").setAttr(L"enable", (this->_warnOverlaps)?L"1":L"0");
    } else {
      this->_config.xml().addChild(L"warn_overlaps").setAttr(L"enable", (this->_warnOverlaps)?L"1":L"0");
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::setWarnExtraInst(bool enable)
{
  this->_warnExtraInstall = enable;

  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"warn_extra_inst")) {
      this->_config.xml().child(L"warn_extra_inst").setAttr(L"enable", (this->_warnExtraInstall)?L"1":L"0");
    } else {
      this->_config.xml().addChild(L"warn_extra_inst").setAttr(L"enable", (this->_warnExtraInstall)?L"1":L"0");
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::setWarnMissDpnd(bool enable)
{
  this->_warnMissingDepend = enable;

  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"warn_miss_dpnd")) {
      this->_config.xml().child(L"warn_miss_dpnd").setAttr(L"enable", (this->_warnMissingDepend)?L"1":L"0");
    } else {
      this->_config.xml().addChild(L"warn_miss_dpnd").setAttr(L"enable", (this->_warnMissingDepend)?L"1":L"0");
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::setWarnExtraUnin(bool enable)
{
  this->_warnExtraUninst = enable;

  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"warn_extra_unin")) {
      this->_config.xml().child(L"warn_extra_unin").setAttr(L"enable", (this->_warnExtraUninst)?L"1":L"0");
    } else {
      this->_config.xml().addChild(L"warn_extra_unin").setAttr(L"enable", (this->_warnExtraUninst)?L"1":L"0");
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::setQuietBatches(bool enable)
{
  this->_quietBatches = enable;

  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"quiet_batches")) {
      this->_config.xml().child(L"quiet_batches").setAttr(L"enable", (this->_quietBatches)?L"1":L"0");
    } else {
      this->_config.xml().addChild(L"quiet_batches").setAttr(L"enable", (this->_quietBatches)?L"1":L"0");
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmManager::makeContext(const wstring& title, const wstring& path, bool open)
{
  // check whether install path exists
  if(!Om_isDir(path)) {
    this->_error = L"error creating Context in \"";
    this->_error += path + L"\": ";
    this->_error += L"path doesn't exists or is not a directory";
    this->log(0, L"Manager", this->_error);
    return false;
  }

  // compose Context home path
  wstring ctx_home = path + L"\\" + title;

  // create Context home folder
  int result = Om_dirCreate(ctx_home);
  if(result != 0) {
    this->_error = L"error creating Context \"";
    this->_error += ctx_home + L"\": ";
    this->_error += Om_getErrorStr(result);
    this->log(0, L"Manager", this->_error);
    return false;
  }

  // compose Context definition file name
  wstring ctx_def_path = ctx_home + L"\\" + title;
  ctx_def_path += L"."; ctx_def_path += OMM_CTX_FILE_EXT;

  // initialize an empty Context definition file
  OmConfig ctx_def;
  if(!ctx_def.init(ctx_def_path, OMM_CFG_SIGN_CTX)) {
    this->_error = L"error creating Context definition \"";
    this->_error += ctx_def_path + L"\": ";
    this->_error += ctx_def.lastErrorStr();
    this->log(0, L"Manager", this->_error);
    return false;
  }

  // generate a new random UUID for this Context
  wstring uuid = Om_genUUID();

  // Get instance of XML document
  OmXmlNode def_xml = ctx_def.xml();

  // create uuid and title entry in Context definition
  def_xml.addChild(L"uuid").setContent(uuid);
  def_xml.addChild(L"title").setContent(title);

  // save and close definition file
  ctx_def.save();
  ctx_def.close();

  // open the new created Context
  if(open) {
    return this->openContext(ctx_def_path);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmManager::openContext(const wstring& path)
{
  // check whether Context is already opened
  for(unsigned i = 0; i < _context.size(); ++i) {
    if(path == _context[i]->path())
      return true;
  }

  OmContext* context = new OmContext(this);

  if(!context->open(path)) {
    this->_error = context->lastError();
    delete context;
    return false;
  }

  this->_context.push_back(context);

  this->saveRecentFile(path);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::closeCurrContext()
{
  for(unsigned i = 0; i < this->_context.size(); ++i) {
    if(this->_curContext == this->_context[i]) {
      this->_context[i]->close();
      delete _context[i];
      this->_context.erase(this->_context.begin()+i);
      break;
    }
  }

  this->_curContext = nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::selContext(int i)
{
  if(i >= 0 && i < (int)this->_context.size()) {
    this->_curContext = _context[i];
    this->log(2, L"Manager", L"Select Context("+this->_curContext->title()+L")");
  } else {
    this->_curContext = nullptr;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::setLogOutput(HWND hWnd) {

  this->_logHwnd = hWnd;

  if(this->_logHwnd) {
    SendMessage((HWND)this->_logHwnd, EM_SETLIMITTEXT, 0, 0);
    SendMessageW((HWND)this->_logHwnd, WM_SETTEXT, 0, (LPARAM)this->_log.c_str());
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmManager::log(unsigned level, const wstring& head, const wstring& detail)
{
  time_t rtime;
  time(&rtime);
  struct tm *ltime = localtime(&rtime);

  wchar_t hour[32];
  wstring entry;

  swprintf(hour, L"[%02d:%02d:%02d]", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
  entry = hour;

  switch(level)
  {
  case 0:
    entry += L"XX ";
    break;
  case 1:
    entry += L"!! ";
    break;
  default:
    entry += L"   ";
    break;
  }

  // build log entry line
  entry += head;
  if(detail.size()) {
    entry += L":: ";
    entry += detail;
  }
  entry += L"\r\n";

  // output to log window
  if(this->_logHwnd) {
    unsigned s = SendMessageW((HWND)this->_logHwnd, WM_GETTEXTLENGTH, 0, 0);
    SendMessage((HWND)this->_logHwnd, EM_SETSEL, s, s);
    SendMessageW((HWND)this->_logHwnd, EM_REPLACESEL, 0, (LPARAM)entry.c_str());
    SendMessage((HWND)this->_logHwnd, WM_VSCROLL, SB_BOTTOM, (LPARAM)0);
    RedrawWindow((HWND)this->_logHwnd, nullptr, nullptr, RDW_ERASE|RDW_INVALIDATE);
  }

  // output to standard output
  std::wcout << entry;

  // write to log file
  if(this->_logFile) {
    DWORD wb;
    string data;
    Om_toMbString(data, entry);
    WriteFile(reinterpret_cast<HANDLE>(this->_logFile), data.c_str(), data.size(), &wb, nullptr);
  }

  this->_log += entry;
}
