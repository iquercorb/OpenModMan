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
#include "OmContext.h"
#include "OmLocation.h"

/// \brief Package name comparison callback
///
/// std::sort callback comparison function for sorting
/// package by name in alphabetical order.
///
/// \param[in]  a     : Left Package.
/// \param[in]  b     : Right Package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __OmLocation_pkgCompareName(const OmPackage* a, const OmPackage* b)
{
  // test against the shorter string
  size_t l = a->name().size() > b->name().size() ? b->name().size() : a->name().size();

  const wchar_t* a_srt = a->name().c_str();
  const wchar_t* b_str = b->name().c_str();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {
    if(towupper(a_srt[i]) != towupper(b_str[i])) {
      if(towupper(a_srt[i]) < towupper(b_str[i])) {
        return true;
      } else {
        return false;
      }
    }
  }

  // strings are equals in tester portion, sort by string size
  if(a->name().size() < b->name().size())
    return true;

  // strings are strictly equals, we sort by "IsZip" status
  if(a->isZip() && !b->isZip())
    return true;

  return false;
}


/// \brief Package version comparison callback
///
/// std::sort callback comparison function for sorting package
/// by version ascending.
///
/// \param[in]  a     : Left Package.
/// \param[in]  b     : Right Package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __OmLocation_pkgCompareVers(const OmPackage* a, const OmPackage* b)
{
  if(a->version() == b->version()) {
    return __OmLocation_pkgCompareName(a, b);
  } else {
    return (a->version() < b->version());
  }
}


/// \brief Package state comparison callback
///
/// std::sort callback comparison function for sorting package
/// by installation state order.
///
/// \param[in]  a     : Left Package.
/// \param[in]  b     : Right Package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __OmLocation_pkgCompareStat(const OmPackage* a, const OmPackage* b)
{
  if(a->hasBck() && b->hasBck()) {
    return __OmLocation_pkgCompareName(a, b);
  } else {
    return (a->hasBck() && !b->hasBck());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmLocation::OmLocation(OmContext* pCtx) :
  _context(pCtx),
  _config(),
  _uuid(),
  _title(),
  _index(0),
  _home(),
  _path(),
  _dstDir(),
  _libDir(),
  _libDirCust(false),
  _bckDir(),
  _bckDirCust(false),
  _pkgLs(),
  _bckZipLevel(-1),
  _pkgSorting(PKG_SORTING_NAME),
  _valid(false),
  _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmLocation::~OmLocation()
{
  this->close();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
inline void OmLocation::_pkgSort()
{
 if(this->_pkgSorting & PKG_SORTING_STAT) //< sorting by status
    std::sort(this->_pkgLs.begin(), this->_pkgLs.end(), __OmLocation_pkgCompareStat);

  if(this->_pkgSorting & PKG_SORTING_NAME) //< sorting by name (alphabetical order)
    std::sort(this->_pkgLs.begin(), this->_pkgLs.end(), __OmLocation_pkgCompareName);

  if(this->_pkgSorting & PKG_SORTING_VERS) //< sorting by version (ascending)
    std::sort(this->_pkgLs.begin(), this->_pkgLs.end(), __OmLocation_pkgCompareVers);

  // check whether we need a normal or reverse sorting
  if(this->_pkgSorting & PKG_SORTING_REVERSE) {
    std::reverse(this->_pkgLs.begin(), this->_pkgLs.end());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::libSetSorting(unsigned sorting)
{
  // we check if the requested sorting kind is the same as the currently
  // used, in this case, this mean the sorting order must be reversed

  if(sorting & this->_pkgSorting) {

    // check if current sorting is reversed, then switch order
    if(this->_pkgSorting & PKG_SORTING_REVERSE) {
      this->_pkgSorting &= ~PKG_SORTING_REVERSE; //< remove reversed flag
    } else {
      this->_pkgSorting |= PKG_SORTING_REVERSE; //< add reversed flag
    }

  } else {

    this->_pkgSorting = sorting;
  }

  // save the current sorting
  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"library_sort")) {
      this->_config.xml().child(L"library_sort").setAttr(L"sort", (int)_pkgSorting);
    } else {
      this->_config.xml().addChild(L"library_sort").setAttr(L"sort", (int)_pkgSorting);
    }

    this->_config.save();
  }

  // finally sort packages
  this->_pkgSort();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::open(const wstring& path)
{
  wstring verbose; //< for log message compositing

  this->close();

  this->log(2, L"Location(<anonymous>) Load", L"\""+path+L"\"");

  // try to open and parse the XML file
  if(!this->_config.open(path, OMM_CFG_SIGN_LOC)) {
    this->_error =  L"\"" + Om_getFilePart(path);
    this->_error += L"\" definition file open error: ";
    this->_error += this->_config.lastErrorStr();
    this->log(0, L"Location(<anonymous>) Load", this->_error);
    return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_config.xml().hasChild(L"uuid")) {
    this->_error =  L"\"" + Om_getFilePart(path);
    this->_error += L"\" invalid definition: <uuid> node missing.";
    log(0, L"Location(<anonymous>) Load", this->_error);
    return false;
  }

  // check for the presence of <title> entry
  if(!this->_config.xml().hasChild(L"title")) {
    this->_error =  L"\"" + Om_getFilePart(path);
    this->_error += L"\" invalid definition: <title> node missing.";
    log(0, L"Location(<anonymous>) Load", this->_error);
    return false;
  }

  // at this point the Location may be valid
  this->_path = path;
  this->_home = Om_getDirPart(this->_path);
  this->_uuid = this->_config.xml().child(L"uuid").content();
  this->_title = this->_config.xml().child(L"title").content();
  this->_index = this->_config.xml().child(L"title").attrAsInt(L"index");

  // check for the presence of <install> entry
  if(this->_config.xml().hasChild(L"install")) {
    // we check whether destination folder is valid
    this->_dstDir = this->_config.xml().child(L"install").content();
    if(!Om_isDir(this->_dstDir)) {
      verbose = L"Destination folder \""+this->_dstDir+L"\"";
      verbose += OMM_STR_ERR_ISDIR;
      this->log(1, L"Location("+this->_title+L") Load", verbose);
    } else {
      verbose = L"Using destination folder: \"";
      verbose += this->_dstDir + L"\".";
      this->log(2, L"Location("+this->_title+L") Load", verbose);
    }
  } else {
    this->_error = L"Invalid definition: <install> node missing.";
    this->log(0, L"Location("+this->_title+L") Load", this->_error);
    this->close();
    return false;
  }

  // check for the presence of <library> entry for custom Library path
  if(this->_config.xml().hasChild(L"library")) {
    // get the custom Library path in config
    this->_libDir = this->_config.xml().child(L"library").content();
    // notify we use a custom Library path
    this->_libDirCust = true;
    if(!Om_isDir(this->_libDir)) {
      verbose = L"Custom library folder \""+this->_libDir+L"\"";
      verbose += OMM_STR_ERR_ISDIR;
      this->log(1, L"Location("+this->_title+L") Load", verbose);
    } else {
      verbose = L"Using custom library folder: \""+this->_libDir+L"\".";
      this->log(2, L"Location("+this->_title+L") Load", verbose);
    }
  } else {
    // no <library> node in config, use default settings
    this->_libDir = this->_home + L"\\library";
    if(!Om_isDir(this->_libDir)) {
      int result = Om_dirCreate(this->_libDir);
      if(result != 0) {
        this->_error = L"Default library folder \""+this->_libDir+L"\".";
        this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
        this->log(0, L"Location("+this->_title+L") Load", this->_error);
        this->close();
        return false;
      }
    }
    verbose = L"Using default library folder: \""+this->_libDir+L"\".";
    this->log(2, L"Location("+this->_title+L") Load", verbose);
  }

  // check for the presence of <backup> entry for custom Backup path
  if(this->_config.xml().hasChild(L"backup")) {
    // get the custom Backup path in config
    this->_bckDir = this->_config.xml().child(L"backup").content();
    // notify we use a custom Backup path
    this->_bckDirCust = true;
    if(!Om_isDir(this->_bckDir)) {
      verbose = L"Custom backup folder \""+this->_bckDir+L"\"";
      verbose += OMM_STR_ERR_ISDIR;
      this->log(1, L"Location("+this->_title+L") Load", verbose);
    } else {
      verbose = L"Using custom backup folder: \""+this->_bckDir+L"\".";
      this->log(2, L"Location("+this->_title+L") Load", verbose);
    }
  } else {
    // no <backup> node in config, use default settings
    this->_bckDir = this->_home + L"\\backup";
    if(!Om_isDir(this->_bckDir)) {
      int result = Om_dirCreate(this->_bckDir);
      if(result != 0) {
        this->_error = L"Default backup folder \""+this->_bckDir+L"\"";
        this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
        this->log(0, L"Location("+this->_title+L") Load", this->_error);
        this->close();
        return false;
      }
    }
    verbose = L"Using default backup folder: \""+this->_bckDir+L"\".";
    this->log(2, L"Location("+this->_title+L") Load", verbose);
  }

  // we check for backup compression level
  if(this->_config.xml().hasChild(L"backup_comp")) {
    this->_bckZipLevel = this->_config.xml().child(L"backup_comp").attrAsInt(L"level");

    // check whether we have a correct value
    if(this->_bckZipLevel > 3)
      this->_bckZipLevel = -1;
  }

  // we check for saved library sorting
  if(this->_config.xml().hasChild(L"library_sort")) {
    this->_pkgSorting = this->_config.xml().child(L"library_sort").attrAsInt(L"sort");
  }

  // Get network repository list
  if(this->_config.xml().hasChild(L"network")) {

    OmXmlNode xml_network = this->_config.xml().child(L"network");

    // check whether repository already exists
    vector<OmXmlNode> xml_rep_list;
    xml_network.children(xml_rep_list, L"repo");

    OmRepository* pRep;

    for(size_t i = 0; i < xml_rep_list.size(); ++i) {
      pRep = new OmRepository(this);
      if(pRep->define(xml_rep_list[i].attrAsString(L"base"), xml_rep_list[i].attrAsString(L"name"))) {
        this->_repLs.push_back(pRep);
        verbose = L"Add Repository: \""+Om_fromUtf8(pRep->url().c_str())+L"\".";
        this->log(2, L"Location("+this->_title+L") Load", verbose);
      } else {
        delete pRep;
      }
    }
  }

  // this location is OK and ready
  this->_valid = true;

  this->log(2, L"Location("+this->_title+L") Load", L"Success");

  // Refresh library
  this->libRefresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::close()
{
  wstring title = this->_title;

  this->libClear();
  this->_home.clear();
  this->_path.clear();
  this->_title.clear();
  this->_dstDir.clear();
  this->_libDir.clear();
  this->_libDirCust = false;
  this->_bckDir.clear();
  this->_bckDirCust = false;
  this->_config.close();
  this->_valid = false;

  this->log(2, L"Location("+title+L") Close", L"Success");
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::checkAccessLib()
{
  bool access_ok = true;

  // checks whether folder exists
  if(Om_isDir(this->_libDir)) {
    // checks for proper permissions on folder
    if(!Om_checkAccess(this->_libDir, OMM_ACCESS_DIR_READ)) {
      this->_error =  L"Library folder \""+this->_libDir+L"\"";
      this->_error += OMM_STR_ERR_READ;
      access_ok = false;
    }
  } else {
    if(this->_libDirCust) {
      this->_error =  L"Custom library folder \""+this->_libDir+L"\"";
      this->_error += OMM_STR_ERR_ISDIR;
      access_ok = false;
    } else {
      // try to create it
      int result = Om_dirCreate(this->_libDir);
      if(result != 0) {
        this->_error = L"Default library folder \""+this->_libDir+L"\"";
        this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
        access_ok = false;
      }
    }
  }

  if(!access_ok) {

    this->log(0, L"Location("+this->_title+L") Library access", this->_error);

    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::checkAccessBck()
{
  bool access_ok = true;

  // checks whether folder exists
  if(Om_isDir(this->_bckDir)) {
    // checks for proper permissions on folder
    if(Om_checkAccess(this->_bckDir, OMM_ACCESS_DIR_READ)) {
      if(!Om_checkAccess(this->_bckDir, OMM_ACCESS_DIR_WRITE)) {
        this->_error = L"Backup folder \""+this->_bckDir+L"\"";
        this->_error += OMM_STR_ERR_WRITE;
        access_ok = false;
      }
    } else {
      this->_error = L"Backup folder \""+this->_bckDir+L"\"";
      this->_error += OMM_STR_ERR_READ;
      access_ok = false;
    }
  } else {
    if(this->_bckDirCust) {
      this->_error =  L"Custom backup folder \""+this->_bckDir+L"\"";
      this->_error += OMM_STR_ERR_ISDIR;
      access_ok = false;
    } else {
      // try to create it
      int result = Om_dirCreate(this->_bckDir);
      if(result != 0) {
        this->_error = L"Default backup folder \""+this->_bckDir+L"\"";
        this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
        access_ok = false;
      }
    }
  }

  if(!access_ok) {

    this->log(0, L"Location("+this->_title+L") Backup access", this->_error);

    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::checkAccessDst()
{
  bool access_ok = true;

  // checks whether folder exists
  if(Om_isDir(this->_dstDir)) {
    // checks for proper permissions on folder
    if(Om_checkAccess(this->_dstDir, OMM_ACCESS_DIR_READ)) {
      if(!Om_checkAccess(this->_dstDir, OMM_ACCESS_DIR_WRITE)) {
        this->_error = L"Destination folder \""+this->_dstDir+L"\"";
        this->_error += OMM_STR_ERR_WRITE;
        access_ok = false;
      }
    } else {
      this->_error = L"Destination folder \""+this->_dstDir+L"\"";
      this->_error += OMM_STR_ERR_READ;
      access_ok = false;
    }
  } else {
    this->_error =  L"Destination folder \""+this->_dstDir+L"\"";
    this->_error += OMM_STR_ERR_ISDIR;
    access_ok = false;
  }

  if(!access_ok) {

    this->log(0, L"Location("+this->_title+L") Destination access", this->_error);

    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::libClear()
{
  for(size_t i = 0; i < this->_pkgLs.size(); ++i)
    delete this->_pkgLs[i];
  this->_pkgLs.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::libRefresh()
{
  // some explanation about how Packages and Backups are managed...
  //
  // One Package object can represent two totally different things that appear
  // as the same thing in the user point of view: A "Backup" and a "Source".
  //
  // The "Source" refers to the Package itself, i.e, the Mod's files to be
  // installed into the Destination tree.
  // The "Backup" refer to original application files saved in a safe place
  // before they were overwritten by the Mod's modified files.
  //
  // So, the Package object is double-sided, one side is the "Backup", the
  // other is the "Source". It may have only one of the two, or both at the
  // same time, depending which element is actually available.
  //
  //   Backup File/Dir
  //            \ ______ Package
  //            /        Object
  //   Source File/Dir
  //
  // In this function, we create the Packages list the user will manipulate and
  // Package objects will be created with one, or both side depending what is
  // found in backup and library folders.
  //
  // "Source" and "Backup" are linked together using a unsigned 64 bit integer
  // xxHash value created from the "Source" Package file (or folder) name.
  //
  // When a "Source" is installed, this Hash is stored in a Backup definition
  // file within the Backup zip file or sub-folder, allowing to matches the
  // available Backups with available Sources.
  //
  // Notice that if user renames its package "Source" file, it will no longer
  // be linkable to an existing "Backup", it will be threated as a separate
  // Package.


  // This function is called either to initializes the package list, or to
  // refresh it because of a change in the Library folder.
  //
  // We want to avoid re-parsing all Backup and Sources for a small changes in
  // the Library folder or Backup status, so, if the package list if not empty,
  // we only update only the necessary.
  //
  // Notice that Backup are tracked internally, the algorithm does not support
  // changes in Backup folder by third-party.

  vector<wstring> path_ls;
  OmPackage* pPkg;

  // our package list is not empty, we will check for added or removed item
  if(this->_pkgLs.size()) {

    // get content of the package Library folder
    if(this->_context->_manager->legacySupport()) {
      Om_lsAll(&path_ls, this->_libDir, true);
    } else {
      Om_lsFile(&path_ls, this->_libDir, true);
    }

    bool in_list;

    // search for unavailable Sources
    for(size_t p = 0; p < this->_pkgLs.size(); ++p) {

      // search this Source in Library folder item list
      in_list = false;
      for(size_t k = 0; k < path_ls.size(); ++k) {
        if(this->_pkgLs[p]->srcPath() == path_ls[k]) { //< compare Source paths
          in_list = true; break;
        }
      }

      // this Source is no longer in Library folder
      if(!in_list) {
        // check whether this Package is installed (has backup)
        if(this->_pkgLs[p]->hasBck()) {
          // this Package is installed, in this case we don't remove it from
          // list, but we revoke its "Source" property since it is no longer
          // available
          this->_pkgLs[p]->srcClear();
        } else {
          // The Package has no Backup and Source is no longer available
          // this is an empty shell, we have to remove it
          this->_pkgLs.erase(this->_pkgLs.begin()+p);
          --p;
        }
      }
    }

    uint64_t pkg_hash;

    // Search for new Sources
    for(size_t i = 0; i < path_ls.size(); ++i) {
      // search in all packages to found this file
      in_list = false;

      pkg_hash = Om_getXXHash3(Om_getFilePart(path_ls[i]));

      for(size_t p = 0; p < this->_pkgLs.size(); ++p) {
        // we first test against the Source Path
        if(path_ls[i] == this->_pkgLs[p]->srcPath()) {
          in_list = true; break;
        }
        // checks whether Hash values matches
        if(pkg_hash == this->_pkgLs[p]->hash()) {
          // this Package Source obviously matches to a currently
          // installed one, since we got a Package with the same Hash but
          // Source is missing, so we add the Source to this Package Backup
          this->_pkgLs[p]->srcParse(path_ls[i]);
          in_list = true; break;
        }
      }
      // This is a new Package Source
      if(!in_list) {
        pPkg = new OmPackage(this);
        if(pPkg->srcParse(path_ls[i])) {
          this->_pkgLs.push_back(pPkg);
        } else {
          delete pPkg;
        }
      }
    }

  } else {

    // get Backup folder content
    Om_lsAll(&path_ls, this->_bckDir, true);

    // add all available and valid Backups
    for(size_t i = 0; i < path_ls.size(); ++i) {
      pPkg = new OmPackage(this);
      if(pPkg->bckParse(path_ls[i])) {
        this->_pkgLs.push_back(pPkg);
      } else {
        delete pPkg;
      }
    }

    // get Backup folder content
    if(this->_context->_manager->legacySupport()) {
      // for legacy support, we also use folders as Package Source
      Om_lsAll(&path_ls, this->_libDir, true);
    } else {
      Om_lsFile(&path_ls, this->_libDir, true);
    }

    bool has_bck;

    // Link Sources to matching Backup, or add new Sources
    for(size_t i = 0; i < path_ls.size(); ++i) {
      has_bck = false;
      // check whether this Source matches an existing Backup
      for(size_t p = 0; p < this->_pkgLs.size(); p++) {
        if(this->_pkgLs[p]->isBckOf(path_ls[i])) {
          this->_pkgLs[p]->srcParse(path_ls[i]);
          has_bck = true;
          break;
        }
      }
      // non Backup found for this Source, adding new Source
      if(!has_bck) {
        pPkg = new OmPackage(this);
        if(pPkg->srcParse(path_ls[i])) {
          this->_pkgLs.push_back(pPkg);
        } else {
          delete pPkg;
        }
      }
    }

  }

  this->_pkgSort();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setTitle(const wstring& title)
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
void OmLocation::setIndex(unsigned index)
{
  if(this->_config.valid()) {

    this->_index = index;

    if(this->_config.xml().hasChild(L"title")) {
      this->_config.xml().child(L"title").setAttr(L"index", static_cast<int>(index));
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setDstDir(const wstring& path)
{
  if(this->_config.valid()) {

    this->_dstDir = path;

    if(this->_config.xml().hasChild(L"install")) {
      this->_config.xml().child(L"install").setContent(path);
    } else {
      this->_config.xml().addChild(L"install").setContent(path);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setCustLibDir(const wstring& path)
{
  if(this->_config.valid()) {

    this->_libDir = path;
    // notify we use a custom Library path
    this->_libDirCust = true;

    if(this->_config.xml().hasChild(L"library")) {
      this->_config.xml().child(L"library").setContent(path);
    } else {
      this->_config.xml().addChild(L"library").setContent(path);
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->libClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::remCustLibDir()
{
  if(this->_config.valid()) {

    this->_libDir = this->_home + L"\\Library";
    // notify we use default settings
    this->_libDirCust = false;

    if(this->_config.xml().hasChild(L"library")) {
      this->_config.xml().remChild(L"library");
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->libClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setCustBckDir(const wstring& path)
{
  if(this->_config.valid()) {

    this->_bckDir = path;
    // notify we use a custom Library path
    this->_bckDirCust = true;

    if(this->_config.xml().hasChild(L"backup")) {
      this->_config.xml().child(L"backup").setContent(path);
    } else {
      this->_config.xml().addChild(L"backup").setContent(path);
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->libClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::remCustBckDir()
{
  if(this->_config.valid()) {

    this->_bckDir = this->_home + L"\\Backup";
    // notify we use default settings
    this->_bckDirCust = false;

    if(this->_config.xml().hasChild(L"backup")) {
      this->_config.xml().remChild(L"backup");
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->libClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setBckZipLevel(int level)
{
  if(this->_config.valid()) {

    this->_bckZipLevel = level;

    if(this->_config.xml().hasChild(L"backup_comp")) {
      this->_config.xml().child(L"backup_comp").setAttr(L"level", (int)level);
    } else {
      this->_config.xml().addChild(L"backup_comp").setAttr(L"level", (int)level);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::renameHome(const wstring& name)
{
  wstring title = this->_title;
  wstring old_path = this->_path;
  wstring old_home = this->_home;

  // Close Location to safe rename and reload it after
  this->close();

  bool has_error = false;

  // compose new Location definition file name
  wstring new_file = name;
  new_file += L".";
  new_file += OMM_LOC_FILE_EXT;

  // Rename Location definition file
  int result = Om_fileMove(old_path, old_home + L"\\" + new_file);
  if(result != 0) {
    this->_error =  L"Definition file \""+old_path+L"\"";
    this->_error += OMM_STR_ERR_RENAME(Om_getErrorStr(result));
    this->log(0, L"Location("+title+L") Rename", this->_error);
    // get back the old file name to restore Location properly
    new_file = Om_getFilePart(old_path);
    has_error = true;
  } else {
    this->log(2, L"Location("+title+L") Rename", L"definition file renamed to \""+new_file+L"\"");
  }

  // compose new Location home folder
  wstring new_home = old_home.substr(0, old_home.find_last_of(L"\\") + 1);
  new_home += name;

  std::wcout << new_home << L"\n";

  // Rename Location home folder
  result = Om_fileMove(old_home, new_home);
  if(result != 0) {
    this->_error =  L"Location subfolder \""+old_home+L"\"";
    this->_error += OMM_STR_ERR_RENAME(Om_getErrorStr(result));
    this->log(0, L"Location("+title+L") Rename", this->_error);
    // get back the old home folder to restore Location properly
    new_home = old_home;
    has_error = true;
  } else {
    this->log(2, L"Location("+title+L") Rename", L"subfolder renamed to \""+new_home+L"\"");
  }

  // Reload location
  this->open(new_home + L"\\" + new_file);

  if(has_error) {
    this->log(2, L"Location("+title+L") Rename", L"Failed");
  } else {
    this->log(2, L"Location("+title+L") Rename", L"Success");
  }

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckMove(const wstring& path, HWND hPb, HWND hSc, const bool *pAbort)
{
  bool has_error = false;

  if(path != this->_bckDir) {

    // checks whether we have a valid old Backup folder
    if(!this->checkAccessBck()) {
      this->_error =  L"Backup folder \""+this->_bckDir+L"\"";
      this->_error += OMM_STR_ERR_DIRACCESS;
      this->log(1, L"Location("+this->_title+L") Move backups", this->_error);
      return false;
    }

    if(hSc) SetWindowTextW(hSc, L"Analyzing...");

    vector<wstring> ls;
    Om_lsAll(&ls, this->_bckDir, false);

    // initialize the progress bar
    if(hPb) {
      SendMessageW(hPb, PBM_SETRANGE, 0, MAKELPARAM(0, ls.size()));
      SendMessageW(hPb, PBM_SETSTEP, 1, 0);
      SendMessageW(hPb, PBM_SETPOS, 0, 0);
    }

    int result;
    wstring src, dst;

    for(size_t i = 0; i < ls.size(); ++i) {

      // check whether abort is requested
      if(pAbort) {
        if(*pAbort) {
          this->log(1, L"Location("+this->_title+L") Move backups", L"Process aborted");
          SetWindowTextW(hSc, L"Process aborted");
          break;
        }
      }

      // update dialog message
      if(hSc) SetWindowTextW(hSc, ls[i].c_str());

      // step progress bar
      if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);
      #ifdef DEBUG_SLOW
      Sleep(DEBUG_SLOW); //< for debug
      #endif

      src = this->_bckDir + L"\\" + ls[i];
      dst = path + L"\\" + ls[i];

      result = Om_fileMove(src, dst);
      if(result != 0) {
        this->_error =  L"Backup data \""+src+L"\"";
        this->_error += OMM_STR_ERR_MOVE(Om_getErrorStr(result));
        this->log(1, L"Location("+this->_title+L") Move backups", this->_error);
        has_error = true;
      }
    }

    // update dialog message
    if(hSc) {
      if(pAbort) {
        if(*pAbort != true) {
          SetWindowTextW(hSc, L"Process completed");
        }
      } else {
        SetWindowTextW(hSc, L"Process completed");
      }
    }

    // Force a full refresh for the next time
    this->libClear();

    this->log(2, L"Location("+this->_title+L") Move backups", L"Data transfered to \""+path+L"\".");
  }

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckHasData()
{
  for(size_t i = 0; i < _pkgLs.size(); ++i) {
    if(_pkgLs[i]->hasBck()) return true;
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckPurge(HWND hPb, HWND hSc, const bool *pAbort)
{
  // checks whether we have a valid Destination folder
  if(!this->checkAccessDst()) {
    this->_error =  L"Destination folder \""+this->_dstDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Purge backups", this->_error);
    return false;
  }

  // checks whether we have a valid Backup folder
  if(!this->checkAccessBck()) {
    this->_error =  L"Backup folder \""+this->_bckDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Purge backups", this->_error);
    return false;
  }

  if(hSc) SetWindowTextW(hSc, L"Analyzing...");

  // first we gather all installed package, we will use the same algorithm than
  // for standard uninstall list, but here we uninstall all packages.

  vector<unsigned> sel_ls; //< our select list
  for(size_t i = 0; i < this->_pkgLs.size(); ++i) {
    if(this->_pkgLs[i]->hasBck())
      sel_ls.push_back(i);
  }

  // check whether we have something to proceed
  if(sel_ls.empty())
    return true;

  vector<OmPackage*> unin_ls; //< real uninstall list

  // Even if we uninstall all packages, we need to build a properly sorted
  // uninstall list according packages overlapping
  for(size_t i = 0; i < sel_ls.size(); ++i) {

    // get list of all packages that overlaps the selection, theses packages
    // must be uninstalled first in order to prevent original files corruption
    this->bckGetOverlaps(unin_ls, sel_ls[i]);
  }

  // Within this function the only goal of this operation is to remove doubles
  // and create the final properly sorted uninstall list.
  bool unique;
  for(size_t i = 0; i < sel_ls.size(); ++i) {

    // we add only if it is not already in the uninstall list created
    // from overlapping tree exploration
    unique = true;
    for(size_t j = 0; j < unin_ls.size(); ++j) {
      if(this->_pkgLs[sel_ls[i]] == unin_ls[j]) {
        unique = false; break;
      }
    }
    if(unique) unin_ls.push_back(this->_pkgLs[sel_ls[i]]);
  }

  // initialize the progress bar
  if(hPb) {
    SendMessageW(hPb, PBM_SETRANGE, 0, MAKELPARAM(0, unin_ls.size()));
    SendMessageW(hPb, PBM_SETSTEP, 1, 0);
    SendMessageW(hPb, PBM_SETPOS, 0, 0);
  }

  bool has_error = false;

  for(size_t i = 0; i < unin_ls.size(); ++i) {

    // update dialog message
    if(hSc) SetWindowTextW(hSc, unin_ls[i]->name().c_str());

    // check whether abort is requested
    if(pAbort) {
      if(*pAbort) {
        this->log(1, L"Location("+this->_title+L") Purge backups", L"Process aborted.");
        SetWindowTextW(hSc, L"Process aborted");
        break;
      }
    }

    // Package uninstall
    if(!unin_ls[i]->uninst(nullptr, pAbort)) {
      this->_error =  L"Backup data \""+unin_ls[i]->name()+L"\"";
      this->_error += L" restoration failed: "+unin_ls[i]->lastError();
      this->log(0, L"Location("+this->_title+L") Purge backups", this->_error);
      has_error = true;
    }

    // step progress bar
    if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);

    #ifdef DEBUG_SLOW
    Sleep(DEBUG_SLOW); //< for debug
    #endif
  }

  // update dialog message
  if(hSc) {
    if(pAbort) {
      if(*pAbort != true) {
        SetWindowTextW(hSc, L"Process completed");
      }
    } else {
      SetWindowTextW(hSc, L"Process completed");
    }
  }

  // Force a full refresh for the next time
  this->libClear();

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckDiscard(HWND hPb, HWND hSc, const bool *pAbort)
{
  // checks whether we have a valid Backup folder
  if(!this->checkAccessBck()) {
    this->_error =  L"Backup folder \""+this->_bckDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Discard backups", this->_error);
    return false;
  }

  if(hSc) SetWindowTextW(hSc, L"Analyzing...");

  // gather all installed package, we will then discard backup for all
  vector<OmPackage*> reset_ls;
  for(size_t i = 0; i < this->_pkgLs.size(); ++i) {
    if(this->_pkgLs[i]->hasBck())
      reset_ls.push_back(this->_pkgLs[i]);
  }

  // check whether we have something to proceed
  if(reset_ls.empty())
    return true;

  // initialize the progress bar
  if(hPb) {
    SendMessageW(hPb, PBM_SETRANGE, 0, MAKELPARAM(0, reset_ls.size()));
    SendMessageW(hPb, PBM_SETSTEP, 1, 0);
    SendMessageW(hPb, PBM_SETPOS, 0, 0);
  }

  bool has_error = false;

  // Discard backup data for all packages
  for(size_t i = 0; i < reset_ls.size(); ++i) {

    // update dialog message
    if(hSc) SetWindowTextW(hSc, reset_ls[i]->name().c_str());

    // check whether abort is requested
    if(pAbort) {
      if(*pAbort) {
        this->log(1, L"Location("+this->_title+L") Discard backups", L"Process aborted.");
        SetWindowTextW(hSc, L"Process aborted");
        break;
      }
    }

    // Discard backup of this package
    if(!reset_ls[i]->unbackup(pAbort)) {
      this->_error =  L"Backup data \""+reset_ls[i]->name()+L"\"";
      this->_error += L" discard failed: "+reset_ls[i]->lastError();
      this->log(0, L"Location("+this->_title+L") Discard backups", this->_error);
      has_error = true;
    }

    // step progress bar
    if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);

    #ifdef DEBUG_SLOW
    Sleep(DEBUG_SLOW); //< for debug
    #endif
  }

  // update dialog message
  if(hSc) {
    if(pAbort) {
      if(*pAbort != true) {
        SetWindowTextW(hSc, L"Process completed");
      }
    } else {
      SetWindowTextW(hSc, L"Process completed");
    }
  }

  // Force a full refresh for the next time
  this->libClear();

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::pkgInst(const vector<unsigned>& sel_ls, bool quiet, HWND hWnd, HWND hLv, HWND hPb, const bool *pAbort)
{
  // checks whether we have a valid Destination folder
  if(!this->checkAccessDst()) {
    this->_error =  L"Destination folder \""+this->_dstDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Packages Install", this->_error);
    Om_dialogBoxErr(hWnd, L"Package(s) installation aborted", this->_error);
    return;
  }

  // checks whether we have a valid Library folder
  if(!this->checkAccessLib()) {
    this->_error =  L"Library folder \""+this->_libDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Packages Install", this->_error);
    Om_dialogBoxErr(hWnd, L"Package(s) installation aborted", this->_error);
    return;
  }

  // checks whether we have a valid Backup folder
  if(!this->checkAccessBck()) {
    this->_error =  L"Backup folder \""+this->_bckDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Packages Install", this->_error);
    Om_dialogBoxErr(hWnd, L"Package(s) installation aborted", this->_error);
    return;
  }

  vector<OmPackage*> inst_ls; //< our real install list


  // so, we first get all dependency tree for the current selection
  vector<wstring> miss_ls; //< missing packages to install
  for(size_t i = 0; i < sel_ls.size(); ++i) {

    // get the recursive dependency tree list for this package, this
    // will also give use an install list in the right order according
    // dependency hierarchy
    miss_ls.clear();
    this->pkgGetDepends(inst_ls, miss_ls, sel_ls[i]);

    // if some dependencies are missing, we ask user if he want to continue and
    // force installation anyway
    if(!quiet && this->_context->_manager->warnMissDpnd() && miss_ls.size()) {

      wstring qry = L"The package \"" + this->_pkgLs[sel_ls[i]]->name();
      qry +=  L"\" have missing dependencies. The following package(s) "
              L"are required, but are not available in your library:\n";

      for(size_t i = 0; i < miss_ls.size(); ++i)
        qry += L"\n " + miss_ls[i];

      qry +=  L"\n\nDo you want to continue and force installation anyway ?";

      if(!Om_dialogBoxQuerryWarn(hWnd, L"Dependencies missing", qry))
        return;
    }
  }

  // we create the extra list, to warn user that additional package
  // will be installed. This is a reverse search from the user selection
  bool unique;
  if(inst_ls.size() && !quiet && this->_context->_manager->warnExtraInst()) {

    vector<OmPackage*> extra_ls; //< packages to install not selected by user

    for(size_t i = 0; i < inst_ls.size(); ++i) {
      // add only if not in the initial selection
      unique = true;
      for(size_t j = 0; j < sel_ls.size(); ++j) {
        if(inst_ls[i] == this->_pkgLs[sel_ls[j]]) {
          unique = false; break;
        }
      }
      if(unique) extra_ls.push_back(inst_ls[i]);
    }

    if(extra_ls.size()) {
      // we have some additional package to install, we warn the user
      // and ask him if he want to continue
      wstring qry = L"One or more selected package(s) have dependencies, "
                    L"the following package(s) also need to be installed:\n";

      for(size_t i = 0; i < extra_ls.size(); ++i)
        qry += L"\n " + extra_ls[i]->ident();

      qry +=  L"\n\nContinue installation ?";

      if(!Om_dialogBoxQuerryWarn(hWnd, L"Package(s) installation dependencies", qry))
        return;
    }
  }

  // we now add the user selection into our real install list
  for(size_t i = 0; i < sel_ls.size(); ++i) {
    // add only if not already in install list
    unique = true;
    for(size_t j = 0; j < inst_ls.size(); ++j) {
      if(this->_pkgLs[sel_ls[i]] == inst_ls[j]) {
        unique = false; break;
      }
    }
    if(unique) inst_ls.push_back(this->_pkgLs[sel_ls[i]]);
  }

  // this is to update list view item's icon individually so to avoid
  // full refresh of the page. Refreshing all page each time work well but
  // it is always good to optimize and doing clean things when possible.
  LVITEMW lvi;
  lvi.mask = LVIF_IMAGE;
  lvi.iSubItem = 0;

  OmPackage* pPkg;
  vector<OmPackage*> ovlp_ls; //< package overlapping list

  for(size_t i = 0; i < inst_ls.size(); ++i) {

    pPkg = inst_ls[i];

    // check whether abort is requested
    if(pAbort) {
      if(*pAbort) break;
    }

    // we need the index of this package in list to change the image
    // in the list view control, this is not really elegant, but we have
    // no other choice
    if(hLv) {
      lvi.iItem = this->pkgIndex(pPkg);
      lvi.iImage =  4; //< WIP
      SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
    }

    if(pPkg->hasSrc() && !pPkg->hasBck()) {

      // we check overlapping before installation, we must do it step by step
      // because overlapping are cumulative with previously installed packages
      ovlp_ls.clear();
      this->pkgFindOverlaps(ovlp_ls, pPkg);

      // if there is overlapping, ask user if he really want to continue installation
      if(ovlp_ls.size() && !quiet && this->_context->_manager->warnOverlaps()) {

        wstring qry = L"Installing the package \"" + pPkg->name();
        qry +=  L"\" will overlaps and modify file(s) previously installed "
                L"or modified by the following package(s):\n";

        for(size_t j = 0; j < ovlp_ls.size(); ++j)
          qry += L"\n "+ovlp_ls[j]->name();

        qry +=  L"\n\nDo you want to continue installation anyway ?";

        if(!Om_dialogBoxQuerryWarn(hWnd, L"Package(s) installation overlap", qry))
          break;
      }

      // install package
      if(!pPkg->install(this->_bckZipLevel, hPb, pAbort)) {
        wstring err = L"The package \"" + pPkg->name();
        err += L"\" has not been installed because the following error occurred:\n\n";
        err += pPkg->lastError();
        Om_dialogBoxErr(hWnd, L"Package install failed", err);
      }
    }

    if(hLv) {
      // update package icon in ListView
      if(pPkg->hasBck()) {
        lvi.iImage = 5; //< BCK
        SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
        // update icons for overlapped packages
        for(size_t j = 0; j < ovlp_ls.size(); ++j) {
          lvi.iItem = this->pkgIndex(ovlp_ls[j]);
          lvi.iImage = 6; //< OWR
          SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
        }
      } else {
        lvi.iImage = -1; //< not installed
        SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      }
    }

    // reset progress bar
    if(hPb) SendMessageW(hPb, PBM_SETPOS, 0, 0);

    #ifdef DEBUG_SLOW
    Sleep(DEBUG_SLOW); //< for debug
    #endif
  }

  // reset progress bar
  if(hPb) SendMessageW(hPb, PBM_SETPOS, 0, 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::pkgUnin(const vector<unsigned>& sel_ls, bool quiet, HWND hWnd, HWND hLv, HWND hPb, const bool *pAbort)
{
  // checks whether we have a valid Destination folder
  if(!this->checkAccessDst()) {
    this->_error = L"Destination folder \""+this->_dstDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Packages Uninstall", this->_error);
    Om_dialogBoxErr(hWnd, L"Package(s) uninstallation aborted", this->_error);
    return;
  }

  // checks whether we have a valid Backup folder
  if(!this->checkAccessBck()) {
    this->_error = L"Backup folder \""+this->_bckDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Packages Uninstall", this->_error);
    Om_dialogBoxErr(hWnd, L"Package(s) uninstallation aborted", this->_error);
    return;
  }

  vector<OmPackage*> unin_ls; //< our real uninstall list

  // Build the real packages uninstall list including packages which overlaps
  // the selection:
  //
  // When installed, a package may overlaps a previously installed one,
  // overwriting and backing elements already modified by another package.
  // To prevent original files corruption, the install and uninstall process
  // takes this into account and overlaps informations are stored into
  // the backup definition.
  //
  // This stage explores the overlaps informations of all currently available
  // backups, in recursive way, to build a full dependencies list of package to
  // uninstall in order to prevent data corruption.
  for(size_t i = 0; i < sel_ls.size(); ++i) {
    // get list of all packages that overlaps the selection, theses packages
    // must be uninstalled first in order to prevent original files corruption
    this->bckGetOverlaps(unin_ls, sel_ls[i]);
  }

  // create the extra unsinstall list, i.e the packages to be unsinstalled
  // in addition to the initial selection.
  //
  // This process have two purposes, the first is to avoid doubles in the final
  // list, the second is to provide a list for the warning message.
  bool unique;
  if(unin_ls.size() && !quiet && this->_context->_manager->warnExtraUnin()) {

    vector<OmPackage*> extra_ls; //< package to uninstall not selected by user

    for(size_t i = 0; i < unin_ls.size(); ++i) {
      // add only if not in the initial selection
      unique = true;
      for(size_t j = 0; j < sel_ls.size(); ++j) {
        if(unin_ls[i] == this->_pkgLs[sel_ls[j]]) {
          unique = false; break;
        }
      }
      if(unique) extra_ls.push_back(unin_ls[i]);
    }

    if(extra_ls.size()) {
      // we have some additional package to uninstall, we warn the user
      // and ask him if he want to continue
      wstring qry = L"One or more selected package(s) are currently overlapped "
                    L"by other(s). To keep backups integrity the following "
                    L"Package(s) will also be uninstalled:\n";

      for(size_t i = 0; i < extra_ls.size(); ++i)
        qry += L"\n " + extra_ls[i]->name();

      qry += L"\n\nDo you want to continue anyway ?";

      if(!Om_dialogBoxQuerryWarn(hWnd, L"Backup(s) restoration overlap", qry))
        return;
    }
  }

  // we now add the selection into the uninstall list
  for(size_t i = 0; i < sel_ls.size(); ++i) {
    // add only if not already in the uninstall list
    unique = true;
    for(size_t j = 0; j < unin_ls.size(); ++j) {
      if(this->_pkgLs[sel_ls[i]] == unin_ls[j]) {
        unique = false; break;
      }
    }
    if(unique) unin_ls.push_back(this->_pkgLs[sel_ls[i]]);
  }

  // this is to update list view item's icon individually so to avoid
  // full refresh of the page. Refreshing all page each time work well but
  // it is always good to optimize and doing clean things when possible.
  LVITEMW lvi;
  lvi.mask = LVIF_IMAGE;
  lvi.iSubItem = 0;

  OmPackage* pPkg;
  vector<OmPackage*> ovlp_ls; //< overlapped packages list

  for(size_t i = 0; i < unin_ls.size(); ++i) {

    pPkg = unin_ls[i];

    // check whether abort is requested
    if(pAbort) {
      if(*pAbort) break;
    }

    // we need the index of this package in list to change the image
    // in the list view control, this is not really elegant, but we have
    // no other choice
    lvi.iItem = this->pkgIndex(pPkg);
    lvi.iImage = 4; //< WIP
    if(hLv) SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));

    if(!pPkg->hasBck()) //< this should be always the case
      continue;

    // before uninstall, get list of overlapped packages (by this one)
    ovlp_ls.clear();
    for(size_t j = 0; j < pPkg->ovrCount(); ++j) {
      ovlp_ls.push_back(this->pkgFind(pPkg->ovrGet(j)));
    }

    // uninstall package (restore backup)
    if(!pPkg->uninst(hPb, pAbort)) {
      wstring err = L"The backup of \"" + pPkg->name();
      err += L"\" has not been properly restored because the following error occurred:\n\n";
      err += pPkg->lastError();
      Om_dialogBoxErr(hWnd, L"Package uninstall failed", err);
    }

    if(hLv) {
      if(pPkg->hasBck()) { //< this mean something went wrong
        lvi.iImage = this->bckOverlapped(pPkg) ? 6 /*OWR*/ : 5 /*BCK*/;
        SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      } else {
        lvi.iImage = -1; //< not installed
        SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
        // update status icon for overlapped packages
        for(size_t j = 0; j < ovlp_ls.size(); ++j) {
          lvi.iItem = pkgIndex(ovlp_ls[j]);
          lvi.iImage = this->bckOverlapped(ovlp_ls[j]) ? 6 /*OWR*/ : 5 /*BCK*/;
          SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
        }
      }
    }

    // reset progress bar
    if(hPb) SendMessageW(hPb, PBM_SETPOS, 0, 0);

    #ifdef DEBUG_SLOW
    Sleep(DEBUG_SLOW); //< for debug
    #endif
  }

  // reset progress bar
  if(hPb) SendMessageW(hPb, PBM_SETPOS, 0, 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::pkgFindOverlaps(vector<OmPackage*>& pkg_list, const OmPackage* pkg) const
{
  size_t n = 0;

  for(size_t i = 0; i < _pkgLs.size(); ++i) {
    if(pkg->ovrTest(this->_pkgLs[i])) {
      pkg_list.push_back(this->_pkgLs[i]);
      ++n;
    }
  }

  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::pkgFindOverlaps(vector<uint64_t>& hash_list, const OmPackage* pkg) const
{
  size_t n = 0;

  for(size_t i = 0; i < _pkgLs.size(); ++i) {
    if(pkg->ovrTest(this->_pkgLs[i])) {
      hash_list.push_back(this->_pkgLs[i]->hash());
      ++n;
    }
  }

  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::pkgGetDepends(vector<OmPackage*>& pkg_list, vector<wstring>& miss_list, const OmPackage* pkg) const
{
  size_t n = 0;

  bool unique;
  bool dpend_found;

  for(size_t i = 0; i < pkg->depCount(); ++i) {

    dpend_found = false;
    for(size_t j = 0; j < this->_pkgLs.size(); ++j) {

      // rely only on packages
      if(!this->_pkgLs[j]->isZip())
        continue;

      if(pkg->depGet(i) == this->_pkgLs[j]->ident()) {

        n += this->pkgGetDepends(pkg_list, miss_list, this->_pkgLs[j]);
        // we add to list only if unique and not already installed, this allow
        // us to get a consistent dependency list for a bunch of package by
        // calling this function for each package without clearing the list
        if(!this->_pkgLs[j]->hasBck()) {
          unique = true;
          for(unsigned k = 0; k < pkg_list.size(); ++k) {
            if(pkg_list[k] == this->_pkgLs[j]) {
              unique = false; break;
            }
          }
          if(unique) {
            pkg_list.push_back(this->_pkgLs[j]);
            ++n;
          }
        }

        dpend_found = true;
        break;
      }
    }

    if(!dpend_found) {
      // we add to list only if unique, this allow us to get a consistent
      // dependency list for a bunch of package by calling this function for each
      // package without clearing the list */
      unique = true;
      for(size_t j = 0; j < miss_list.size(); ++j) {
        if(miss_list[j] == pkg->depGet(i)) {
          unique = false; break;
        }
      }
      if(unique) {
        miss_list.push_back(pkg->depGet(i));
      }
    }
  }

  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::bckGetOverlaps(vector<OmPackage*>& pkg_list, const OmPackage* pkg) const
{
  size_t n = 0;
  bool unique;
  for(size_t i = 0; i < this->_pkgLs.size(); ++i) {
    if(this->_pkgLs[i]->ovrHas(pkg->_hash)) {

      // the function is recursive, we want the full list like a
      // depth-first search in the right order
      n += this->bckGetOverlaps(pkg_list, this->_pkgLs[i]);

      // recursive way can produce doubles, we want to avoid it
      // so we add only if not already in the list
      unique = true;
      for(size_t j = 0; j < pkg_list.size(); ++j) {
        if(pkg_list[j] == this->_pkgLs[i]) {
          unique = false; break;
        }
      }

      if(unique) {
        pkg_list.push_back(this->_pkgLs[i]);
        ++n;
      }
    }
  }
  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::repAdd(const wstring& base, const wstring& name)
{
  if(this->_config.valid()) {

    if(!this->_config.xml().hasChild(L"network")) {
      this->_config.xml().addChild(L"network");
    }

    OmXmlNode xml_net = this->_config.xml().child(L"network");

    // check whether repository already exists
    vector<OmXmlNode> xml_rep_list;
    xml_net.children(xml_rep_list, L"repo");

    for(size_t i = 0; i < xml_rep_list.size(); ++i) {
      if(base == xml_rep_list[i].attrAsString(L"base") && name == xml_rep_list[i].attrAsString(L"name")) {
        this->_error = L"Repository with same parameters already exists";
        this->log(1, L"Location("+this->_title+L") Add Repository", this->_error);
        return false;
      }
    }

    // add repository entry in definition
    OmXmlNode xml_rep = xml_net.addChild(L"repo");
    xml_rep.setAttr(L"base", base);
    xml_rep.setAttr(L"name", name);

    this->_config.save();

    // add repository in local list
    OmRepository* pRep = new OmRepository(this);

    // set repository parameters
    if(!pRep->define(base, name)) {
      this->log(1, L"Location("+this->_title+L") Add Repository", pRep->lastError());
      delete pRep;
      return false;
    }

    // add to list
    this->_repLs.push_back(pRep);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::repRem(unsigned id)
{
  if(this->_config.valid()) {

    OmRepository* pRep = this->_repLs[id];

    // remove repository from definition
    if(this->_config.xml().hasChild(L"network")) {
      OmXmlNode xml_net = this->_config.xml().child(L"network");

      // check whether repository already exists
      vector<OmXmlNode> xml_rep_list;
      xml_net.children(xml_rep_list, L"repo");

      for(size_t i = 0; i < xml_rep_list.size(); ++i) {
        if(pRep->base() == xml_rep_list[i].attrAsString(L"base") &&
           pRep->name() == xml_rep_list[i].attrAsString(L"name")) {
          xml_net.remChild(xml_rep_list[i]);
          break;
        }
      }
    }

    this->_config.save();

    // delete object
    delete pRep;

    // remove from local list
    this->_repLs.erase(this->_repLs.begin()+id);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::log(unsigned level, const wstring& head, const wstring& detail)
{
  wstring log_str = L"Context("; log_str.append(this->_context->title());
  log_str.append(L"):: "); log_str.append(head);

  this->_context->log(level, log_str, detail);
}
