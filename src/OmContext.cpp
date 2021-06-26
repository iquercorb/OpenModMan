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
#include "OmLocation.h"
#include "OmContext.h"


/// \brief Search for valid Location file
///
/// Search for one or more valid Location file(s) in the specified location.
///
/// \param[in]  omt_list  : Array of strings to be filled with valid file(s) path.
/// \param[in]  path      : Path where to search file.
///
/// \return The count of valid file found, or -1 if an error occurred.
///
static inline int __OmContext_lsLocDef(vector<wstring>& omt_list, const wstring& path)
{
  omt_list.clear();

  vector<wstring> ls;

  wstring filter = L"*.";
  filter += OMM_LOC_FILE_EXT;

  Om_lsFileFiltered(&ls, path, filter, true);

  OmConfig cfg;

  int n = 0;
  for(unsigned i = 0; i < ls.size(); ++i) {
    // search for proper XML root node
    if(cfg.open(ls[i], OMM_CFG_SIGN_LOC)) {
      omt_list.push_back(ls[i]); ++n;
    }
    cfg.close();
  }

  return n;
}


/*
/// \brief Location name comparison callback
///
/// std::sort callback comparison function for sorting Locations
/// by alphabetical order.
///
/// \param[in]  a     : Left Location.
/// \param[in]  b     : Right Location.
///
/// \return True if Location a is "before" Location b, false otherwise
///
static bool __OmContext_locCompareName(const OmLocation* a, const OmLocation* b)
{
  // test against the shorter string
  size_t l = a->title().size() > b->title().size() ? b->title().size() : a->title().size();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {
    if(towupper(a->title()[i]) != towupper(b->title()[i])) {
      if(towupper(a->title()[i]) < towupper(b->title()[i])) {
        return true;
      } else {
        return false;
      }
    }
  }

  // strings are equals in tester portion, sort by string size
  if(a->title().size() < b->title().size())
    return true;

  return false;
}
*/


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
static bool __OmContext_locCompareIndex(const OmLocation* a, const OmLocation* b)
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
static bool __OmContext_batCompareIndex(const OmBatch* a, const OmBatch* b)
{
  return (a->index() < b->index());
}




///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmContext::OmContext(OmManager* pMgr) :
  _manager(pMgr),
  _config(),
  _path(),
  _uuid(),
  _title(),
  _home(),
  _icon(nullptr),
  _locLst(),
  _locCur(nullptr),
  _batLst(),
  _valid(false),
  _error()
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

  this->log(2, L"Context(<anonymous>) Load", L"\""+path+L"\"");

  // try to open and parse the XML file
  if(!this->_config.open(path, OMM_CFG_SIGN_CTX)) {
    this->_error =  L"Definition file \""+path+L"\"";
    this->_error += OMM_STR_ERR_DEFOPEN(this->_config.lastErrorStr());
    this->log(0, L"Context(<anonymous>) Load", this->_error);
    return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_config.xml().hasChild(L"uuid")) {
    this->_error =  L"\"" + Om_getFilePart(path);
    this->_error += L"\" invalid definition: <uuid> node missing.";
    log(0, L"Context(<anonymous>) Load", this->_error);
    return false;
  }

  // check for the presence of <title> entry
  if(!this->_config.xml().hasChild(L"title")) {
    this->_error =  L"\"" + Om_getFilePart(path);
    this->_error += L"\" invalid definition: <title> node missing.";
    log(0, L"Context(<anonymous>) Load", this->_error);
    return false;
  }

  // right now this Context appear usable, even if it is empty
  this->_path = path;
  this->_home = Om_getDirPart(this->_path);
  this->_uuid = this->_config.xml().child(L"uuid").content();
  this->_title = this->_config.xml().child(L"title").content();
  this->_valid = true;

  // lookup for a icon
  if(this->_config.xml().hasChild(L"icon")) {

    // we got a banner
    wstring src = this->_config.xml().child(L"icon").content();

    HICON hicon = nullptr;
    ExtractIconExW(src.c_str(), 0, &hicon, nullptr, 1); //< large icon
    //ExtractIconExW(src.c_str(), 0, nullptr, &hicon, 1); //< small icon

    if(hicon) {
      this->_icon = hicon;
    } else {
      this->log(1, L"Context("+this->_title+L") Load", L"\""+src+L"\" icon extraction failed.");
    }
  }

  // load Locations for this Context
  vector<wstring> subdir;
  Om_lsDir(&subdir, this->_home, false);

  if(subdir.size()) {

    vector<wstring> omt_list;

    for(size_t i = 0; i < subdir.size(); ++i) {

      // search for file(s) with the OMM_LOC_FILE_EXT extension within the sub-folder
      if(__OmContext_lsLocDef(omt_list, this->_home + L"\\" + subdir[i]) > 0) {

        // we use the first file we found
        verbose =  L"Found Location definition: \"";
        verbose += Om_getFilePart(omt_list[0]) + L"\".";
        this->log(2, L"Context("+this->_title+L") Load", verbose);

        OmLocation* pLoc = new OmLocation(this);

        if(pLoc->open(omt_list[0])) {
          this->_locLst.push_back(pLoc);
        } else {
          delete pLoc;
          verbose =  L"Ignoring Location (open failed): \"";
          verbose += Om_getFilePart(omt_list[0]) + L"\".";
          this->log(1, L"Context("+this->_title+L") Load", verbose);
        }
      }
    }

    // sort Locations by index
    if(this->_locLst.size() > 1)
      sort(this->_locLst.begin(), this->_locLst.end(), __OmContext_locCompareIndex);
  }

  // Load batches for this Context
  vector<wstring> omb_list;
  wstring filter = L"*."; filter += OMM_BAT_FILE_EXT;
  Om_lsFileFiltered(&omb_list, this->_home, filter, true);

  if(omb_list.size()) {

    for(size_t i = 0; i < omb_list.size(); ++i) {

      verbose =  L"Found Batch definition: \"";
      verbose += Om_getFilePart(omb_list[i]) + L"\".";
      this->log(2, L"Context("+this->_title+L") Load", verbose);

      OmBatch* pBat = new OmBatch(this);

      if(pBat->open(omb_list[i])) {

        this->_batLst.push_back(pBat);

        verbose =  L"Batch \"" + pBat->title();
        verbose += L"\" added to list.";
        this->log(2, L"Context("+this->_title+L") Load", verbose);

      } else {

        verbose =  L"Batch \"" + pBat->title();
        verbose += L"\" parse error:";
        verbose += pBat->lastError();
        this->log(1, L"Context("+this->_title+L") Load", verbose);

        delete pBat;
      }
    }

    // sort Batches by index
    if(this->_batLst.size() > 1)
      sort(this->_batLst.begin(), this->_batLst.end(), __OmContext_batCompareIndex);
  }

  this->log(2, L"Context("+this->_title+L") Load", L"Success");

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

    for(size_t i = 0; i < this->_locLst.size(); ++i)
      delete this->_locLst[i];

    this->_locLst.clear();

    this->_locCur = nullptr;

    this->_valid = false;

    this->log(2, L"Context("+title+L") Close", L"Success");
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

    HICON hicon = nullptr;

    // empty source path mean remove icon
    if(src.empty()) {

      // remove icon entry
      if(this->_icon) DestroyIcon(this->_icon);
      this->_icon = nullptr;

      if(this->_config.xml().hasChild(L"icon")) {
        this->_config.xml().remChild(this->_config.xml().child(L"icon"));
      }

    } else {

      if(Om_isFile(src)) {
        ExtractIconExW(src.c_str(), 0, &hicon, nullptr, 1);
      }

      if(hicon) {
        if(this->_icon) DeleteObject(_icon);
        this->_icon = hicon;

        if(this->_config.xml().hasChild(L"icon")) {
          this->_config.xml().child(L"icon").setContent(src);
        } else {
          this->_config.xml().addChild(L"icon").setContent(src);
        }

      } else {
        this->log(1, L"Context("+this->_title+L") Set Icon", L"\""+src+L"\" icon extraction failed.");
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
  for(size_t i = 0; i < this->_locLst.size(); ++i) {
    if(uuid == this->_locLst[i]->uuid())
      return this->_locLst[i];
  }

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::locSort()
{
  if(this->_locLst.size() > 1)
    sort(this->_locLst.begin(), this->_locLst.end(), __OmContext_locCompareIndex);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::locSel(int i)
{
  if(i >= 0) {
    if(i < (int)this->_locLst.size()) {
      this->_locCur = this->_locLst[i];
      this->log(2, L"Context("+this->_title+L") Select Location", L"\""+this->_locCur->title()+L"\".");
    } else {
      return false;
    }
  } else {
    this->_locCur = nullptr;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::locSel(const wstring& uuid)
{
  for(size_t i = 0; i < this->_locLst.size(); ++i) {
    if(uuid == this->_locLst[i]->uuid()) {
      this->_locCur = this->_locLst[i];
      this->log(2, L"Context("+this->_title+L") Select Location", L"\""+this->_locCur->title()+L"\".");
      return true;
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmContext::locFind(const wstring& uuid)
{
  for(size_t i = 0; i < this->_locLst.size(); ++i) {
    if(uuid == this->_locLst[i]->uuid()) {
      return i;
    }
  }

  return -1;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmContext::locCurId() const
{
  if(this->_locCur != nullptr)
    for(size_t i = 0; i < this->_locLst.size(); ++i)
      if(this->_locCur == this->_locLst[i])  return i;

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

  // compose Location home path
  wstring loc_home = this->_home + L"\\" + title;

  // create Location sub-folder
  if(!Om_isDir(loc_home)) {
    int result = Om_dirCreate(loc_home);
    if(result != 0) {
      this->_error = L"Location subfolder \""+loc_home+L"\"";
      this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
      this->log(0, L"Context("+this->_title+L") Create Location", this->_error);
      return false;
    }
  } else {
    this->_error =  L"Location subfolder \""+loc_home+L"\"";
    this->_error += L" already exists";
    this->log(1, L"Context("+this->_title+L") Create Location", this->_error);
  }

  // compose Location definition file name
  wstring loc_def_path = loc_home + L"\\" + title;
  loc_def_path += L"."; loc_def_path += OMM_LOC_FILE_EXT;

  // check whether definition file already exists and delete it
  if(Om_isFile(loc_def_path)) {
    this->_error =  L"Definition file \""+loc_def_path+L"\"";
    this->_error += L" already exists, deleting previous file.";
    this->log(1, L"Context("+this->_title+L") Create Location", this->_error);
    int result = Om_fileDelete(loc_def_path);
    if(result != 0) {
      this->_error = L"Previous definition \""+loc_def_path+L"\"";
      this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
      this->log(0, L"Context("+this->_title+L") Create Location", this->_error);
      return false;
    }
  }

  // initialize new definition file
  OmConfig loc_def;
  if(!loc_def.init(loc_def_path, OMM_CFG_SIGN_LOC)) {
    this->_error =  L"Definition file \""+loc_def_path+L"\"";
    this->_error += OMM_STR_ERR_DEFINIT(loc_def.lastErrorStr());
    this->log(0, L"Context("+this->_title+L") Create Location", this->_error);
    return false;
  }

  // Generate a new UUID for this Location
  wstring uuid = Om_genUUID();

  // Get XML document instance
  OmXmlNode xml_def = loc_def.xml();

  // define uuid and title in definition file
  xml_def.addChild(L"uuid").setContent(uuid);
  xml_def.addChild(L"title").setContent(title);

  // define ordering index in definition file
  xml_def.child(L"title").setAttr(L"index", static_cast<int>(this->_locLst.size()));

  // define installation destination folder in definition file
  xml_def.addChild(L"install").setContent(install);

  // checks whether we have custom Backup folder
  if(backup.empty()) {
    // Create the default backup sub-folder
    Om_dirCreate(loc_home + L"\\Backup");
  } else {
    // check whether custom Library folder exists
    if(!Om_isDir(backup)) {
      this->_error = L"Custom backup folder \""+backup+L"\"";
      this->_error += OMM_STR_ERR_ISDIR;
      this->log(1, L"Context("+this->_title+L") Create Location", this->_error);
    }
    // add custom backup in definition
    xml_def.addChild(L"backup").setContent(backup);
  }

  // checks whether we have custom Library folder
  if(library.empty()) {
    // Create the default library sub-folder
    Om_dirCreate(loc_home + L"\\Library");
  } else {
    // check whether custom Library folder exists
    if(!Om_isDir(library)) {
      this->_error = L"Custom library folder \""+library+L"\"";
      this->_error += OMM_STR_ERR_ISDIR;
      this->log(1, L"Context("+this->_title+L") Create Location", this->_error);
    }
    // add custom library in definition
    xml_def.addChild(L"library").setContent(library);
  }

  // save and close definition file
  loc_def.save();
  loc_def.close();

  this->log(2, L"Context("+this->_title+L") Create Location", L"Location \""+title+L")\" created.");

  // load the newly created Location
  OmLocation* pLoc = new OmLocation(this);
  pLoc->open(loc_def_path);
  this->_locLst.push_back(pLoc);

  // sort locations by index
  this->locSort();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::locRem(unsigned id)
{
  if(id >= this->_locLst.size())
    return false;

  OmLocation* pLoc = this->_locLst[id];

  if(pLoc->bckHasData()) {
    this->_error = L"The Location \""+pLoc->title()+L"\"";
    this->_error += L" cannot be deleted: Location still has backup data";
    this->_error += L"to be restored.";
    this->log(1, L"Context("+this->_title+L") Delete Location", this->_error);
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
      this->_error =  L"Backup folder \""+bck_path+L"\"";
      this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
      this->log(1, L"Context("+this->_title+L") Delete Location", this->_error);
    }
  }

  // remove the default Library folder
  wstring lib_path = loc_home + L"\\Library";
  if(Om_isDir(lib_path)) {
    // this will fails if folder not empty, this is intended
    int result = Om_dirDelete(lib_path);
    if(result != 0) {
      this->_error =  L"Library folder \""+lib_path+L"\"";
      this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
      this->log(1, L"Context("+this->_title+L") Delete Location", this->_error);
    }
  }

  // remove the definition file
  if(Om_isFile(loc_path)) {
    // close the definition file
    this->_config.close();
    int result = Om_fileDelete(loc_path);
    if(result != 0) {
      this->_error =  L"Definition file \""+loc_path+L"\"";
      this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
      this->log(1, L"Context("+this->_title+L") Delete Location", this->_error);
      has_error = true; //< this is considered as a real error
    }
  }

  // check if location home folder is empty, if yes, we delete it
  if(Om_isDirEmpty(loc_home)) {
    int result = Om_dirDelete(loc_home);
    if(result != 0) {
      this->_error =  L"Location subfolder \""+loc_home+L"\"";
      this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
      this->log(1, L"Context("+this->_title+L") Delete Location", this->_error);
      has_error = true; //< this is considered as a real error
    }
  } else {
    this->_error = L"Non-empty subfolder \""+loc_home+L"\"";
    this->_error += L" will not be deleted";
    this->log(1, L"Context("+this->_title+L") Delete Location", this->_error);
  }

  this->log(2, L"Context("+this->_title+L") Delete Location", L"Location \""+loc_title+L"\" deleted.");

  // delete object
  delete pLoc;

  // remove from list
  this->_locLst.erase(this->_locLst.begin()+id);

  // update locations order indexing
  for(size_t i = 0; i < this->_locLst.size(); ++i) {
    this->_locLst[i]->setIndex(i);
  }

  // sort Locations by index
  this->locSort();

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::batSort()
{
  if(this->_batLst.size() > 1)
    sort(this->_batLst.begin(), this->_batLst.end(), __OmContext_batCompareIndex);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::batAdd(const wstring& title, const vector<wstring>& loc_uuid, const vector<vector<uint64_t>>& loc_hash_list)
{
  // check whether we have same count of Location and hash list
  if(loc_hash_list.size() != loc_uuid.size()) {
    this->_error = L"Hash list and Location count mismatches, expected \"";
    this->_error += loc_uuid.size();
    this->_error += L"\", found ";
    this->_error += loc_hash_list.size();
    this->log(0, L"Context("+this->_title+L") Create Batch", this->_error);
    return false;
  }

  // compose path using title and context home
  wstring bat_def_path = this->_home + L"\\";
  bat_def_path += title; bat_def_path += L"."; bat_def_path += OMM_BAT_FILE_EXT;

  // initialize new definition file
  OmConfig bat_def;
  if(!bat_def.init(bat_def_path, OMM_CFG_SIGN_BAT)) {
    this->_error = L"Definition file \""+bat_def_path+L"\"";
    this->_error += OMM_STR_ERR_DEFINIT(bat_def.lastErrorStr());
    this->log(0, L"Context("+this->_title+L") Create Batch", this->_error);
    return false;
  }

  // Generate a new UUID for this Batch
  wstring uuid = Om_genUUID();

  // Get XML document instance
  OmXmlNode def_xml = bat_def.xml();

  // set uuid and title
  def_xml.addChild(L"uuid").setContent(uuid);
  def_xml.addChild(L"title").setContent(title);

  // define ordering index in definition file
  def_xml.child(L"title").setAttr(L"index", static_cast<int>(this->_batLst.size()));

  // useful variables
  OmLocation* pLoc;
  OmXmlNode xml_loc, xml_ins;

  // For each Location defined
  for(size_t l = 0; l < loc_uuid.size(); ++l) {

    pLoc = this->locGet(loc_uuid[l]);

    if(pLoc != nullptr) {

      // add <location> entry
      xml_loc = def_xml.addChild(L"location");
      xml_loc.setAttr(L"uuid", loc_uuid[l]);

      for(size_t i = 0; i < loc_hash_list[l].size(); ++i) {
        // check whether package with this hash exists
        if(pLoc->pkgFind(loc_hash_list[l][i])) {
          xml_ins = xml_loc.addChild(L"install");
          xml_ins.setAttr(L"hash", Om_toHexString(loc_hash_list[l][i]));
        } else {
          this->_error = L"Package with hash "+Om_toHexString(loc_hash_list[l][i]);
          this->_error += L" was not found in Location.";
          this->log(1, L"Context("+this->_title+L") Create Batch", this->_error);
        }
      }
    } else {
      this->_error = L"Location with UUID "+loc_uuid[l];
      this->_error += L" was not found.";
      this->log(1, L"Context("+this->_title+L") Create Batch", this->_error);
    }
  }

  // save and close definition file
  bat_def.save();
  bat_def.close();

  this->log(2, L"Context("+this->_title+L") Create Batch", L"Batch \""+title+L"\" created.");

  // load the newly created Batch
  OmBatch* pBat = new OmBatch(this);
  pBat->open(bat_def_path);
  this->_batLst.push_back(pBat);

  // sort Batches by index
  this->batSort();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::batRem(unsigned id)
{
  if(id < this->_batLst.size()) {

    OmBatch* pBat = this->_batLst[id];

    wstring bat_path = pBat->path();
    wstring bat_title = pBat->title();


    // remove the definition file
    if(Om_isFile(bat_path)) {
      // close the definition file
      this->_config.close();
      int result = Om_fileDelete(bat_path);
      if(result != 0) {
        this->_error = L"Unable to delete Batch definition file \""+bat_path+L"\": ";
        this->_error += Om_getErrorStr(result);
        this->log(1, L"Context("+this->_title+L") Delete Batch", this->_error);
        return false;
      }
    }

    // delete batch object
    delete pBat;

    // remove from list
    this->_batLst.erase(this->_batLst.begin()+id);

    // update batches order indexing
    for(size_t i = 0; i < this->_batLst.size(); ++i) {
      this->_batLst[i]->setIndex(i);
    }

    // sort Batches by index
    this->batSort();

    this->log(2, L"Context("+this->_title+L") Delete Batch", L"Batch \""+bat_title+L"\" deleted.");

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::log(unsigned level, const wstring& head, const wstring& detail)
{
  wstring log_str = L"Manager:: "; log_str.append(head);

  this->_manager->log(level, log_str, detail);
}
