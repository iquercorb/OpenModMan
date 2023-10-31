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
#include "OmBase.h"
#include <algorithm>            //< std::sort

#include "OmBaseApp.h"

#include "OmManager.h"
#include "OmLocation.h"

#include "OmUtilFs.h"
#include "OmUtilHsh.h"
#include "OmUtilErr.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmContext.h"


/// \brief Location index comparison callback
///
/// std::sort callback comparison function for sorting Locations
/// by index number order.
///
/// \param[in]  a     : Left Location.
/// \param[in]  b     : Right Location.
///
/// \return True if Location a is "before" Location b, false otherwise
///
static bool __loc_sort_index_fn(const OmLocation* a, const OmLocation* b)
{
  return (a->index() < b->index());
}


/// \brief OmBatch index comparison callback
///
/// std::sort callback comparison function for sorting Locations
/// by index number order.
///
/// \param[in]  a     : Left OmBatch.
/// \param[in]  b     : Right OmBatch.
///
/// \return True if OmBatch a is "before" OmBatch b, false otherwise
///
static bool __bat_sort_index_fn(const OmBatch* a, const OmBatch* b)
{
  return (a->index() < b->index());
}




///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmContext::OmContext(OmManager* pMgr) :
  _manager(pMgr), _config(), _path(), _uuid(), _title(), _home(), _icon(nullptr),
  _locLs(), _locCur(-1), _batLs(), _batQuietMode(true), _valid(false), _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmContext::~OmContext()
{
  this->close();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::open(const wstring& path)
{
  wstring verbose; //< for log message compositing

  this->close();

  // try to open and parse the XML file
  if(!this->_config.open(path, OMM_XMAGIC_CTX)) {
    this->_error = Om_errParse(L"Definition file", path, this->_config.lastErrorStr());
    this->log(0, L"Context(<anonymous>) Open", this->_error);
    return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_config.xml().hasChild(L"uuid")) {
    this->_error =  L"\""+Om_getFilePart(path)+L"\" invalid definition: <uuid> node missing.";
    log(0, L"Context(<anonymous>) Open", this->_error);
    return false;
  }

  // check for the presence of <title> entry
  if(!this->_config.xml().hasChild(L"title")) {
    this->_error = L"\""+Om_getFilePart(path)+L"\" invalid definition: <title> node missing.";
    log(0, L"Context(<anonymous>) Open", this->_error);
    return false;
  }

  // right now this Context appear usable, even if it is empty
  this->_path = path;
  this->_home = Om_getDirPart(this->_path);
  this->_uuid = this->_config.xml().child(L"uuid").content();
  this->_title = this->_config.xml().child(L"title").content();
  this->_valid = true;

  this->log(2, L"Context("+this->_title+L") Open",
            L"Definition parsed.");

  // lookup for a icon
  if(this->_config.xml().hasChild(L"icon")) {

    // we got a banner
    wstring ico_path = this->_config.xml().child(L"icon").content();

    this->log(2, L"Context("+this->_title+L") Open",
              L"Associated icon \""+ico_path+L"\"");

    HICON hIc = nullptr;
    ExtractIconExW(ico_path.c_str(), 0, &hIc, nullptr, 1); //< large icon
    //ExtractIconExW(ico_path.c_str(), 0, nullptr, &hIc, 1); //< small icon

    if(hIc) {
      this->_icon = hIc;
    } else {
      this->log(1, L"Context("+this->_title+L") Open",
                L"\""+ico_path+L"\" icon extraction failed.");
    }
  }

  // we check for saved batches quiet mode option
  if(this->_config.xml().hasChild(L"batches_quietmode")) {
    this->_batQuietMode = this->_config.xml().child(L"batches_quietmode").attrAsInt(L"enable");
  } else {
    this->setBatQuietMode(this->_batQuietMode); //< create default
  }

  // load Locations for this Context
  vector<wstring> file_ls;
  vector<wstring> subdir_ls;
  Om_lsDir(&subdir_ls, this->_home, false);

  if(subdir_ls.size()) {

    OmConfig cfg;

    for(size_t i = 0; i < subdir_ls.size(); ++i) {

      // check for presence of Target Location definition file
      file_ls.clear();
      Om_lsFileFiltered(&file_ls, this->_home+L"\\"+subdir_ls[i], L"*." OMM_LOC_DEF_FILE_EXT, true);

      // we parse the fist definition file found in directory
      if(file_ls.size()) {

        this->log(2, L"Context("+this->_title+L") Open",
                  L"Linking Location \""+Om_getFilePart(file_ls[0])+L"\"");

        // we use the first file we found
        OmLocation* pLoc = new OmLocation(this);

        if(pLoc->open(file_ls[0])) {
          this->_locLs.push_back(pLoc);
        } else {
          delete pLoc;
        }
      }
    }
  }

  // Search for Installation Batches within Context home and subfolders
  file_ls.clear();
  Om_lsFileFiltered(&file_ls, this->_home, L"*." OMM_BAT_DEF_FILE_EXT, true);
  for(size_t i = 0; i < subdir_ls.size(); ++i) {
    Om_lsFileFiltered(&file_ls, this->_home+L"\\"+subdir_ls[i], L"*." OMM_BAT_DEF_FILE_EXT, true);
  }

  if(file_ls.size()) {

    for(size_t i = 0; i < file_ls.size(); ++i) {

      this->log(2, L"Context("+this->_title+L") Open",
                L"Bind Batch \""+Om_getFilePart(file_ls[i])+L"\"");

      OmBatch* pBat = new OmBatch(this);

      if(pBat->open(file_ls[i])) {
        this->_batLs.push_back(pBat);
      } else {
        delete pBat;
      }
    }
  }

  // sort Locations by index
  if(this->_locLs.size() > 1)
    sort(this->_locLs.begin(), this->_locLs.end(), __loc_sort_index_fn);

  // sort Batches by index
  if(this->_batLs.size() > 1)
    sort(this->_batLs.begin(), this->_batLs.end(), __bat_sort_index_fn);

  // the first location in list become the default active one
  if(this->_locLs.size()) {
    this->locSel(0);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::close()
{
  if(this->_valid) {

    wstring title = this->_title;

    this->_path.clear();
    this->_home.clear();
    this->_title.clear();

    if(this->_icon) DestroyIcon(this->_icon);
    this->_icon = nullptr;

    this->_config.close();

    for(size_t i = 0; i < this->_locLs.size(); ++i)
      delete this->_locLs[i];

    this->_locLs.clear();

    this->_locCur = -1;

    for(size_t i = 0; i < this->_batLs.size(); ++i)
      delete this->_batLs[i];

    this->_batLs.clear();

    this->_valid = false;

    this->log(2, L"Context("+title+L") Close",
              L"Success");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::setTitle(const wstring& title)
{
  if(this->_config.valid()) {

    this->_title = title;

    if(this->_config.xml().hasChild(L"title")) {
      this->_config.xml().child(L"title").setContent(title);
    } else {
      this->_config.xml().addChild(L"title").setContent(title);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::setIcon(const wstring& src)
{
  if(this->_config.valid()) {

    // delete previous object
    if(this->_icon) DestroyIcon(this->_icon);
    this->_icon = nullptr;

    // empty source path mean remove icon
    if(!src.empty()) {

      HICON hIc = nullptr;

      if(Om_isFile(src))
        ExtractIconExW(src.c_str(), 0, &hIc, nullptr, 1);

      if(hIc) {

        this->_icon = hIc;

        if(this->_config.xml().hasChild(L"icon")) {
          this->_config.xml().child(L"icon").setContent(src);
        } else {
          this->_config.xml().addChild(L"icon").setContent(src);
        }

      } else {
        this->log(1, L"Context("+this->_title+L") Set Icon",
                  L"\""+src+L"\" icon extraction failed.");
      }
    }

    if(!this->_icon) {

      if(this->_config.xml().hasChild(L"icon")) {
        this->_config.xml().remChild(this->_config.xml().child(L"icon"));
      }
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmLocation* OmContext::locGet(const wstring& uuid)
{
  for(size_t i = 0; i < this->_locLs.size(); ++i) {
    if(uuid == this->_locLs[i]->uuid())
      return this->_locLs[i];
  }

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::locSort()
{
  if(this->_locLs.size() > 1)
    sort(this->_locLs.begin(), this->_locLs.end(), __loc_sort_index_fn);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::locSel(int i)
{
  if(i >= 0) {
    if(i < (int)this->_locLs.size()) {
      this->_locCur = i;
      this->log(2, L"Context("+this->_title+L") Select Location",
                L"\""+this->_locLs[_locCur]->title()+L"\".");
    } else {
      return false;
    }
  } else {
    this->_locCur = -1;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::locSel(const wstring& uuid)
{
  for(size_t i = 0; i < this->_locLs.size(); ++i) {
    if(uuid == this->_locLs[i]->uuid()) {
      this->_locCur = i;
      this->log(2, L"Context("+this->_title+L") Select Location",
                L"\""+this->_locLs[_locCur]->title()+L"\".");
      return true;
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmContext::locIndex(const wstring& uuid)
{
  for(size_t i = 0; i < this->_locLs.size(); ++i) {
    if(uuid == this->_locLs[i]->uuid()) {
      return i;
    }
  }

  return -1;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::locAdd(const wstring& title, const wstring& install, const wstring& library, const wstring& backup)
{
  // this theoretically can't happen, but we check to be sure
  if(!this->isValid()) {
    this->_error = L"Context is empty.";
    this->log(0, L"Context(<anonymous>) Create Location", this->_error);
    return false;
  }

  int result;

  // compose Location home path
  wstring loc_home = this->_home + L"\\" + title;

  // create Location sub-folder
  if(!Om_isDir(loc_home)) {
    result = Om_dirCreate(loc_home);
    if(result != 0) {
      this->_error = Om_errCreate(L"Location home", loc_home, result);
      this->log(0, L"Context("+this->_title+L") Create Location", this->_error);
      return false;
    }
  } else {
    this->log(1, L"Context("+this->_title+L") Create Location",
              Om_errExists(L"Location home",loc_home));
  }

  // compose Location definition file name
  wstring loc_def_path = loc_home + L"\\" + title;
  loc_def_path += L"."; loc_def_path += OMM_LOC_DEF_FILE_EXT;

  // check whether definition file already exists and delete it
  if(Om_isFile(loc_def_path)) {

    this->log(1, L"Context("+this->_title+L") Create Location",
              Om_errExists(L"Definition file",loc_def_path));

    int result = Om_fileDelete(loc_def_path);
    if(result != 0) {
      this->_error = Om_errDelete(L"Old definition file", loc_def_path, result);
      this->log(0, L"Context("+this->_title+L") Create Location", this->_error);
      return false;
    }
  }

  // initialize new definition file
  OmConfig loc_def;
  if(!loc_def.init(loc_def_path, OMM_XMAGIC_LOC)) {
    this->_error = Om_errInit(L"Definition file", loc_def_path, loc_def.lastErrorStr());
    this->log(0, L"Context("+this->_title+L") Create Location", this->_error);
    return false;
  }

  // Generate a new UUID for this Location
  wstring uuid = Om_genUUID();

  // Get XML document instance
  OmXmlNode loc_xml = loc_def.xml();

  // define uuid and title in definition file
  loc_xml.addChild(L"uuid").setContent(uuid);
  loc_xml.addChild(L"title").setContent(title);

  // define ordering index in definition file
  loc_xml.child(L"title").setAttr(L"index", static_cast<int>(this->_locLs.size()));

  // define installation destination folder in definition file
  loc_xml.addChild(L"install").setContent(install);

  // checks whether we have custom Backup folder
  if(backup.empty()) {
    // Create the default backup sub-folder
    Om_dirCreate(loc_home + L"\\Backup");
  } else {
    // check whether custom Library folder exists
    if(!Om_isDir(backup)) {
      this->log(1, L"Context("+this->_title+L") Create Location",
                Om_errIsDir(L"Custom Backup folder", backup));
    }
    // add custom backup in definition
    loc_xml.addChild(L"backup").setContent(backup);
  }

  // checks whether we have custom Library folder
  if(library.empty()) {
    // Create the default library sub-folder
    Om_dirCreate(loc_home + L"\\Library");
  } else {
    // check whether custom Library folder exists
    if(!Om_isDir(library)) {
      this->log(1, L"Context("+this->_title+L") Create Location",
                Om_errIsDir(L"Custom Library folder", library));
    }
    // add custom library in definition
    loc_xml.addChild(L"library").setContent(library);
  }

  // save and close definition file
  loc_def.save();
  loc_def.close();

  this->log(2, L"Context("+this->_title+L") Create Location", L"Location \""+title+L")\" created.");

  // load the newly created Location
  OmLocation* pLoc = new OmLocation(this);
  pLoc->open(loc_def_path);
  this->_locLs.push_back(pLoc);

  // sort locations by index
  this->locSort();

  // select the last added location
  this->locSel(this->_locLs.size() - 1);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::locRem(unsigned id)
{
  if(id >= this->_locLs.size())
    return false;

  OmLocation* pLoc = this->_locLs[id];

  if(pLoc->bckHasData()) {
    this->_error = L"Aborted: Still have backup data to be restored.";
    this->log(0, L"Context("+this->_title+L") Delete Location", this->_error);
    return false;
  }

  bool has_error = false;

  // keep Location paths
  wstring loc_title = pLoc->title();
  wstring loc_home = pLoc->home();
  wstring loc_path = pLoc->path();

  // close Location
  pLoc->close();

  // remove the default backup folder
  wstring bck_path = loc_home + L"\\Backup";
  if(Om_isDir(bck_path)) {
    // this will fails if folder not empty, this is intended
    int result = Om_dirDelete(bck_path);
    if(result != 0) {
      this->log(1, L"Context("+this->_title+L") Delete Location",
                Om_errDelete(L"Backup folder", bck_path, result));
    }
  }

  // remove the default Library folder
  wstring lib_path = loc_home + L"\\Library";
  if(Om_isDir(lib_path)) {
    // this will fails if folder not empty, this is intended
    if(Om_isDirEmpty(lib_path)) {
      int result = Om_dirDelete(lib_path);
      if(result != 0) {
        this->log(1, L"Context("+this->_title+L") Delete Location",
                  Om_errDelete(L"Library folder", lib_path, result));
      }
    } else {
      this->log(1, L"Context("+this->_title+L") Delete Location",
              L"Non-empty Library folder will not be deleted");
    }
  }

  // remove the definition file
  if(Om_isFile(loc_path)) {
    // close the definition file
    this->_config.close();
    int result = Om_fileDelete(loc_path);
    if(result != 0) {
      this->_error = Om_errDelete(L"Definition file", loc_path, result);
      this->log(1, L"Context("+this->_title+L") Delete Location", this->_error);
      has_error = true; //< this is considered as a real error
    }
  }

  // check if location home folder is empty, if yes, we delete it
  if(Om_isDirEmpty(loc_home)) {
    int result = Om_dirDelete(loc_home);
    if(result != 0) {
      this->_error = Om_errDelete(L"Home folder", loc_home, result);
      this->log(1, L"Context("+this->_title+L") Delete Location", this->_error);
      has_error = true; //< this is considered as a real error
    }
  } else {
    this->log(1, L"Context("+this->_title+L") Delete Location",
              L"Non-empty home folder \""+loc_home+L"\" will not be deleted");
  }

  this->log(2, L"Context("+this->_title+L") Delete Location",
            L"Location \""+loc_title+L"\" deleted.");

  // delete object
  delete pLoc;

  // remove from list
  this->_locLs.erase(this->_locLs.begin()+id);

  // update locations order indexing
  for(size_t i = 0; i < this->_locLs.size(); ++i) {
    this->_locLs[i]->setIndex(i);
  }

  // sort Locations by index
  this->locSort();

  // select the first available location
  this->locSel(0);

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::batSort()
{
  if(this->_batLs.size() > 1)
    sort(this->_batLs.begin(), this->_batLs.end(), __bat_sort_index_fn);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmBatch* OmContext::batAdd(const wstring& title)
{
  // compose path using title and context home
  wstring path = this->_home + L"\\";
  path += title; path += L"."; path += OMM_BAT_DEF_FILE_EXT;

  // Create new batch object
  OmBatch* pBat = new OmBatch(this);
  pBat->init(path, title, this->_batLs.size());
  this->_batLs.push_back(pBat);

  this->log(2, L"Context("+this->_title+L") Create Batch", L"Batch \""+title+L"\" created.");

  // sort Batches by index
  this->batSort();

  return pBat;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::batRem(unsigned id)
{
  if(id < this->_batLs.size()) {

    OmBatch* pBat = this->_batLs[id];

    wstring bat_path = pBat->path();
    wstring bat_title = pBat->title();


    // remove the definition file
    if(Om_isFile(bat_path)) {
      // close the definition file
      this->_config.close();
      int result = Om_fileDelete(bat_path);
      if(result != 0) {
        this->_error = Om_errDelete(L"Batch definition file", bat_path, result);
        this->log(1, L"Context("+this->_title+L") Delete Batch", this->_error);
        return false;
      }
    }

    // delete batch object
    delete pBat;

    // remove from list
    this->_batLs.erase(this->_batLs.begin()+id);

    // update batches order indexing
    for(size_t i = 0; i < this->_batLs.size(); ++i) {
      this->_batLs[i]->setIndex(i);
    }

    // sort Batches by index
    this->batSort();

    this->log(2, L"Context("+this->_title+L") Delete Batch", L"Installation Batch \""+bat_title+L"\" deleted.");

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::setBatQuietMode(bool enable)
{
  this->_batQuietMode = enable;

  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"batches_quietmode")) {
      this->_config.xml().child(L"batches_quietmode").setAttr(L"enable", this->_batQuietMode ? 1 : 0);
    } else {
      this->_config.xml().addChild(L"batches_quietmode").setAttr(L"enable", this->_batQuietMode ? 1 : 0);
    }

    this->_config.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::log(unsigned level, const wstring& head, const wstring& detail)
{
  wstring log_str = L"Manager:: "; log_str.append(head);

  this->_manager->log(level, log_str, detail);
}
