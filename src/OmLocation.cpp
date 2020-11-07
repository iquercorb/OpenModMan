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
  if(a->isArchive() && !b->isArchive())
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
  if(a->hasBackup() && b->hasBackup()) {
    return __OmLocation_pkgCompareName(a, b);
  } else {
    return (a->hasBackup() && !b->hasBackup());
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
  _installDir(),
  _libraryDir(),
  _custLibraryDir(false),
  _backupDir(),
  _custBackupDir(false),
  _package(),
  _backupZipLevel(-1),
  _packageSorting(PKG_SORTING_NAME),
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
inline void OmLocation::_packageSort()
{
 if(this->_packageSorting & PKG_SORTING_STAT) //< sorting by status
    std::sort(this->_package.begin(), this->_package.end(), __OmLocation_pkgCompareStat);

  if(this->_packageSorting & PKG_SORTING_NAME) //< sorting by name (alphabetical order)
    std::sort(this->_package.begin(), this->_package.end(), __OmLocation_pkgCompareName);

  if(this->_packageSorting & PKG_SORTING_VERS) //< sorting by version (ascending)
    std::sort(this->_package.begin(), this->_package.end(), __OmLocation_pkgCompareVers);

  // check whether we need a normal or reverse sorting
  if(this->_packageSorting & PKG_SORTING_REVERSE) {
    std::reverse(this->_package.begin(), this->_package.end());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setPackageSorting(unsigned sorting)
{
  // we check if the requested sorting kind is the same as the currently
  // used, in this case, this mean the sorting order must be reversed

  if(sorting & this->_packageSorting) {

    // check if current sorting is reversed, then switch order
    if(this->_packageSorting & PKG_SORTING_REVERSE) {
      this->_packageSorting &= ~PKG_SORTING_REVERSE; //< remove reversed flag
    } else {
      this->_packageSorting |= PKG_SORTING_REVERSE; //< add reversed flag
    }

  } else {

    this->_packageSorting = sorting;
  }

  // save the current sorting
  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"library_sort")) {
      this->_config.xml().child(L"library_sort").setAttr(L"sort", (int)_packageSorting);
    } else {
      this->_config.xml().addChild(L"library_sort").setAttr(L"sort", (int)_packageSorting);
    }

    this->_config.save();
  }

  // finally sort packages
  this->_packageSort();
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
    this->_installDir = this->_config.xml().child(L"install").content();
    if(!Om_isDir(this->_installDir)) {
      verbose = L"Destination folder \""+this->_installDir+L"\"";
      verbose += OMM_STR_ERR_ISDIR;
      this->log(1, L"Location("+this->_title+L") Load", verbose);
    } else {
      verbose = L"Using destination folder: \"";
      verbose += this->_installDir + L"\".";
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
    this->_libraryDir = this->_config.xml().child(L"library").content();
    // notify we use a custom Library path
    this->_custLibraryDir = true;
    if(!Om_isDir(this->_libraryDir)) {
      verbose = L"Custom library folder \""+this->_libraryDir+L"\"";
      verbose += OMM_STR_ERR_ISDIR;
      this->log(1, L"Location("+this->_title+L") Load", verbose);
    } else {
      verbose = L"Using custom library folder: \""+this->_libraryDir+L"\".";
      this->log(2, L"Location("+this->_title+L") Load", verbose);
    }
  } else {
    // no <library> node in config, use default settings
    this->_libraryDir = this->_home + L"\\library";
    if(!Om_isDir(this->_libraryDir)) {
      int result = Om_dirCreate(this->_libraryDir);
      if(result != 0) {
        this->_error = L"Default library folder \""+this->_libraryDir+L"\".";
        this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
        this->log(0, L"Location("+this->_title+L") Load", this->_error);
        this->close();
        return false;
      }
    }
    verbose = L"Using default library folder: \""+this->_libraryDir+L"\".";
    this->log(2, L"Location("+this->_title+L") Load", verbose);
  }

  // check for the presence of <backup> entry for custom Backup path
  if(this->_config.xml().hasChild(L"backup")) {
    // get the custom Backup path in config
    this->_backupDir = this->_config.xml().child(L"backup").content();
    // notify we use a custom Backup path
    this->_custBackupDir = true;
    if(!Om_isDir(this->_backupDir)) {
      verbose = L"Custom backup folder \""+this->_backupDir+L"\"";
      verbose += OMM_STR_ERR_ISDIR;
      this->log(1, L"Location("+this->_title+L") Load", verbose);
    } else {
      verbose = L"Using custom backup folder: \""+this->_backupDir+L"\".";
      this->log(2, L"Location("+this->_title+L") Load", verbose);
    }
  } else {
    // no <backup> node in config, use default settings
    this->_backupDir = this->_home + L"\\backup";
    if(!Om_isDir(this->_backupDir)) {
      int result = Om_dirCreate(this->_backupDir);
      if(result != 0) {
        this->_error = L"Default backup folder \""+this->_backupDir+L"\"";
        this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
        this->log(0, L"Location("+this->_title+L") Load", this->_error);
        this->close();
        return false;
      }
    }
    verbose = L"Using default backup folder: \""+this->_backupDir+L"\".";
    this->log(2, L"Location("+this->_title+L") Load", verbose);
  }

  // we check for backup compression level
  if(this->_config.xml().hasChild(L"backup_comp")) {
    this->_backupZipLevel = this->_config.xml().child(L"backup_comp").attrAsInt(L"level");

    // check whether we have a correct value
    if(this->_backupZipLevel > 3)
      this->_backupZipLevel = -1;
  }

  // we check for saved library sorting
  if(this->_config.xml().hasChild(L"library_sort")) {
    this->_packageSorting = this->_config.xml().child(L"library_sort").attrAsInt(L"sort");
  }

  // this location is OK and ready
  this->_valid = true;

  this->log(2, L"Location("+this->_title+L") Load", L"Success");

  // Refresh library
  this->packageListRefresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::close()
{
  wstring title = this->_title;

  this->packageListClear();
  this->_home.clear();
  this->_path.clear();
  this->_title.clear();
  this->_installDir.clear();
  this->_libraryDir.clear();
  this->_custLibraryDir = false;
  this->_backupDir.clear();
  this->_custBackupDir = false;
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
  if(Om_isDir(this->_libraryDir)) {
    // checks for proper permissions on folder
    if(!Om_checkAccess(this->_libraryDir, OMM_ACCESS_DIR_READ)) {
      this->_error =  L"Library folder \""+this->_libraryDir+L"\"";
      this->_error += OMM_STR_ERR_READ;
      access_ok = false;
    }
  } else {
    if(this->_custLibraryDir) {
      this->_error =  L"Custom library folder \""+this->_libraryDir+L"\"";
      this->_error += OMM_STR_ERR_ISDIR;
      access_ok = false;
    } else {
      // try to create it
      int result = Om_dirCreate(this->_libraryDir);
      if(result != 0) {
        this->_error = L"Default library folder \""+this->_libraryDir+L"\"";
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
  if(Om_isDir(this->_backupDir)) {
    // checks for proper permissions on folder
    if(Om_checkAccess(this->_backupDir, OMM_ACCESS_DIR_READ)) {
      if(!Om_checkAccess(this->_backupDir, OMM_ACCESS_DIR_WRITE)) {
        this->_error = L"Backup folder \""+this->_backupDir+L"\"";
        this->_error += OMM_STR_ERR_WRITE;
        access_ok = false;
      }
    } else {
      this->_error = L"Backup folder \""+this->_backupDir+L"\"";
      this->_error += OMM_STR_ERR_READ;
      access_ok = false;
    }
  } else {
    if(this->_custBackupDir) {
      this->_error =  L"Custom backup folder \""+this->_backupDir+L"\"";
      this->_error += OMM_STR_ERR_ISDIR;
      access_ok = false;
    } else {
      // try to create it
      int result = Om_dirCreate(this->_backupDir);
      if(result != 0) {
        this->_error = L"Default backup folder \""+this->_backupDir+L"\"";
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
  if(Om_isDir(this->_installDir)) {
    // checks for proper permissions on folder
    if(Om_checkAccess(this->_installDir, OMM_ACCESS_DIR_READ)) {
      if(!Om_checkAccess(this->_installDir, OMM_ACCESS_DIR_WRITE)) {
        this->_error = L"Destination folder \""+this->_installDir+L"\"";
        this->_error += OMM_STR_ERR_WRITE;
        access_ok = false;
      }
    } else {
      this->_error = L"Destination folder \""+this->_installDir+L"\"";
      this->_error += OMM_STR_ERR_READ;
      access_ok = false;
    }
  } else {
    this->_error =  L"Destination folder \""+this->_installDir+L"\"";
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
void OmLocation::packageListClear()
{
  for(size_t i = 0; i < this->_package.size(); ++i)
    delete this->_package[i];
  this->_package.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::packageListRefresh()
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

  vector<wstring> ls;
  OmPackage* pPkg;

  // our package list is not empty, we will check for added or removed item
  if(this->_package.size()) {

    // get content of the package Library folder
    if(this->_context->_manager->legacySupport()) {
      Om_lsAll(&ls, this->_libraryDir, true);
    } else {
      Om_lsFile(&ls, this->_libraryDir, true);
    }

    bool in_list;

    // search for unavailable Packages Sources
    for(size_t p = 0; p < this->_package.size(); ++p) {

      // search this Package Source in Library folder item list
      in_list = false;
      for(size_t k = 0; k < ls.size(); ++k) {
        if(this->_package[p]->sourcePath() == ls[k]) { //< compare Source paths
          in_list = true; break;
        }
      }

      // this Package Source is no longer in Library folder
      if(!in_list) {
        // check whether this Package is installed (has backup)
        if(this->_package[p]->hasBackup()) {
          // this Package is installed, in this case we don't remove it from
          // list, but we revoke its "Source" property since it is no longer
          // available
          this->_package[p]->sourceClear();
        } else {
          // The Package has no Backup and Source is no longer available
          // this is an empty shell, we have to remove it
          this->_package.erase(this->_package.begin()+p);
          --p;
        }
      }
    }

    uint64_t full_hash;

    // Search for new Packages Source
    for(size_t i = 0; i < ls.size(); ++i) {
      // search in all packages to found this file
      in_list = false;

      full_hash = Om_getXXHash3(Om_getFilePart(ls[i]));

      for(size_t p = 0; p < this->_package.size(); ++p) {
        // we first test against the Source Path
        if(ls[i] == this->_package[p]->sourcePath()) {
          in_list = true; break;
        }
        // checks whether Hash values matches
        if(full_hash == this->_package[p]->hash()) {
          // this Package Source obviously matches to a currently
          // installed one, since we got a Package with the same Hash but
          // Source is missing, so we add the Source to this Package Backup
          this->_package[p]->sourceParse(ls[i]);
          in_list = true; break;
        }
      }
      // This is a new Package Source
      if(!in_list) {
        pPkg = new OmPackage(this);
        if(pPkg->sourceParse(ls[i])) {
          this->_package.push_back(pPkg);
        } else {
          delete pPkg;
        }
      }
    }

    this->_packageSort();

    return;
  }

  // if we are here in the function, this mean the package list is empty,
  // either because it is Location initialization or because list was purged.
  // so, here we go to recreate the package list from scratch.

  // get Backup folder content
  Om_lsAll(&ls, this->_backupDir, true);

  // add all available and valid Backups
  for(size_t i = 0; i < ls.size(); ++i) {
    pPkg = new OmPackage(this);
    if(pPkg->backupParse(ls[i])) {
      this->_package.push_back(pPkg);
    } else {
      delete pPkg;
    }
  }

  // get Backup folder content
  if(this->_context->_manager->legacySupport()) {
    // for legacy support, we also use folders as Package Source
    Om_lsAll(&ls, this->_libraryDir, true);
  } else {
    Om_lsFile(&ls, this->_libraryDir, true);
  }

  bool has_bck;

  // Link Sources to matching Backup, or add new Sources
  for(size_t i = 0; i < ls.size(); ++i) {
    has_bck = false;
    // check whether this Source matches an existing Backup
    for(size_t p = 0; p < this->_package.size(); p++) {
      if(this->_package[p]->isBackupOf(ls[i])) {
        this->_package[p]->sourceParse(ls[i]);
        has_bck = true;
        break;
      }
    }
    // non Backup found for this Source, adding new Source
    if(!has_bck) {
      pPkg = new OmPackage(this);
      if(pPkg->sourceParse(ls[i])) {
        this->_package.push_back(pPkg);
      } else {
        delete pPkg;
      }
    }
  }

  this->_packageSort();
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
void OmLocation::setInstallDir(const wstring& path)
{
  if(this->_config.valid()) {

    this->_installDir = path;

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
void OmLocation::setCustLibraryDir(const wstring& path)
{
  if(this->_config.valid()) {

    this->_libraryDir = path;
    // notify we use a custom Library path
    this->_custLibraryDir = true;

    if(this->_config.xml().hasChild(L"library")) {
      this->_config.xml().child(L"library").setContent(path);
    } else {
      this->_config.xml().addChild(L"library").setContent(path);
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->packageListClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::remCustLibraryDir()
{
  if(this->_config.valid()) {

    this->_libraryDir = this->_home + L"\\Library";
    // notify we use default settings
    this->_custLibraryDir = false;

    if(this->_config.xml().hasChild(L"library")) {
      this->_config.xml().remChild(L"library");
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->packageListClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setCustBackupDir(const wstring& path)
{
  if(this->_config.valid()) {

    this->_backupDir = path;
    // notify we use a custom Library path
    this->_custBackupDir = true;

    if(this->_config.xml().hasChild(L"backup")) {
      this->_config.xml().child(L"backup").setContent(path);
    } else {
      this->_config.xml().addChild(L"backup").setContent(path);
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->packageListClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::remCustBackupDir()
{
  if(this->_config.valid()) {

    this->_backupDir = this->_home + L"\\Backup";
    // notify we use default settings
    this->_custBackupDir = false;

    if(this->_config.xml().hasChild(L"backup")) {
      this->_config.xml().remChild(L"backup");
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->packageListClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setBackupZipLevel(int level)
{
  if(this->_config.valid()) {

    this->_backupZipLevel = level;

    if(_config.xml().hasChild(L"backup_comp")) {
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
  wstring new_file = name; new_file += L"."; new_file += OMM_LOC_FILE_EXT;

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
bool OmLocation::backupsMove(const wstring& path, HWND hPb, HWND hSc, const bool *pAbort)
{
  bool has_error = false;

  if(path != this->_backupDir) {

    // checks whether we have a valid old Backup folder
    if(!this->checkAccessBck()) {
      this->_error =  L"Backup folder \""+this->_backupDir+L"\"";
      this->_error += OMM_STR_ERR_DIRACCESS;
      this->log(1, L"Location("+this->_title+L") Move backup", this->_error);
      return false;
    }

    // update dialog message
    wstring item_str;

    if(hSc) SetWindowTextW(hSc, L"Analyzing...");

    vector<wstring> ls;
    Om_lsAll(&ls, this->_backupDir, false);

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
          this->log(1, L"Location("+this->_title+L") Move backup", L"Process aborted");
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

      src = this->_backupDir + L"\\" + ls[i];
      dst = path + L"\\" + ls[i];

      result = Om_fileMove(src, dst);
      if(result != 0) {
        this->_error =  L"Backup data \""+src+L"\"";
        this->_error += OMM_STR_ERR_MOVE(Om_getErrorStr(result));
        this->log(1, L"Location("+this->_title+L") Move backup", this->_error);
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
    this->packageListClear();

    this->log(2, L"Location("+this->_title+L") Move backup", L"Data transfered to \""+path+L"\".");
  }

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::hasBackupData()
{
  for(size_t i = 0; i < _package.size(); ++i) {
    if(_package[i]->hasBackup()) return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::backupsPurge(HWND hPb, HWND hSc, const bool *pAbort)
{
  // checks whether we have a valid Destination folder
  if(!this->checkAccessDst()) {
    this->_error =  L"Destination folder \""+this->_installDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Purge backup", this->_error);
    return false;
  }

  // checks whether we have a valid Backup folder
  if(!this->checkAccessBck()) {
    this->_error =  L"Backup folder \""+this->_backupDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Purge backup", this->_error);
    return false;
  }

  // update dialog message
  wstring item_str;

  if(hSc) SetWindowTextW(hSc, L"Analyzing...");

  // first we gather all installed package, we will use the same algorithm than
  // for standard uninstall list, but here we uninstall all packages.

  vector<unsigned> selec_list; //< our select list
  for(size_t i = 0; i < this->_package.size(); ++i) {
    if(this->_package[i]->hasBackup())
      selec_list.push_back(i);
  }

  // check whether we have something to proceed
  if(selec_list.empty())
    return true;

  vector<OmPackage*> uninst_list; //< our real uninstall list

  // Build the real packages uinstall list including dependencies created
  // when packages installation overlaps others:
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
  for(size_t i = 0; i < selec_list.size(); ++i) {

    // get the dependencies for this package, recursively. This will
    // also give us a uninstall list in the right order, built by tree
    // exploration in depth
    this->getUninOwList(uninst_list, selec_list[i]);
  }

  bool unique;

  // create the extra uninstall list, i.e the new packages to be uninstalled
  // in addition to the user selected ones.
  //
  // This process have two purposes, the first is to avoid doubles, since user
  // selection may already contain packages found during dependencies listing, the
  // second is to provide a concise list to warn user of additional package to
  // be uninstalled.
  for(size_t i = 0; i < selec_list.size(); ++i) {

    // we add only if it is not already in the uninstall list created
    // from dependencies tree exploration
    unique = true;
    for(size_t j = 0; j < uninst_list.size(); ++j) {
      if(this->_package[selec_list[i]] == uninst_list[j]) {
        unique = false; break;
      }
    }

    if(unique)
      uninst_list.push_back(this->_package[selec_list[i]]);
  }

  // initialize the progress bar
  if(hPb) {
    SendMessageW(hPb, PBM_SETRANGE, 0, MAKELPARAM(0, uninst_list.size()));
    SendMessageW(hPb, PBM_SETSTEP, 1, 0);
    SendMessageW(hPb, PBM_SETPOS, 0, 0);
  }

  bool has_error = false;

  for(size_t i = 0; i < uninst_list.size(); ++i) {

    // update dialog message
    if(hSc) SetWindowTextW(hSc, uninst_list[i]->name().c_str());

    // check whether abort is requested
    if(pAbort) {
      if(*pAbort) {
        this->log(1, L"Location("+this->_title+L") Purge backup", L"Process aborted.");
        SetWindowTextW(hSc, L"Process aborted");
        break;
      }
    }

    if(uninst_list[i]->hasBackup()) { //< this should be always the case

      // we uninstall package and restore backup, without forcing for recursive
      // uninstall since our uninstall list is already ordered according
      // dependency tree
      if(!uninst_list[i]->uninst(nullptr, pAbort)) {
        this->_error =  L"Backup data \""+uninst_list[i]->name()+L"\"";
        this->_error += L" restoration failed: "+uninst_list[i]->lastError();
        this->log(0, L"Location("+this->_title+L") Purge backup", this->_error);
        has_error = true;
      }
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
  this->packageListClear();

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::packagesInst(const vector<unsigned>& selec_list, bool quiet, HWND hWnd, HWND hLv, HWND hPb, const bool *pAbort)
{
  // checks whether we have a valid Destination folder
  if(!this->checkAccessDst()) {
    this->_error =  L"Destination folder \""+this->_installDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Packages Install", this->_error);
    Om_dialogBoxErr(hWnd, L"Package(s) installation aborted", this->_error);
    return;
  }

  // checks whether we have a valid Library folder
  if(!this->checkAccessLib()) {
    this->_error =  L"Library folder \""+this->_libraryDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Packages Install", this->_error);
    Om_dialogBoxErr(hWnd, L"Package(s) installation aborted", this->_error);
    return;
  }

  // checks whether we have a valid Backup folder
  if(!this->checkAccessBck()) {
    this->_error =  L"Backup folder \""+this->_backupDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Packages Install", this->_error);
    Om_dialogBoxErr(hWnd, L"Package(s) installation aborted", this->_error);
    return;
  }

  vector<OmPackage*> insta_list; //< our real install list
  vector<wstring>    missg_list; //< missing packages to install
  vector<OmPackage*> olaps_list; //< package overlapping list

  // Here we go for a smart install process
  //
  // We build the dependency tree for each package to create the real
  // install list, we will also built a missing dependency list.
  bool unique;

  // so, we first get all dependency tree for packages selected by user
  for(size_t i = 0; i < selec_list.size(); ++i) {

    // get the recursive dependency tree list for this package, this
    // will also give use an install list in the right order according
    // dependency hierarchy
    missg_list.clear();
    this->getInstDpList(insta_list, missg_list, selec_list[i]);

    // some dependencies are missing, we ask user if he want to continue and
    // force installation anyway
    if(!quiet && this->_context->_manager->warnMissDpnd() && missg_list.size()) {

      wstring qry = L"The package \"";
      qry += this->_package[selec_list[i]]->name();
      qry += L"\" have missing dependencies. The following package(s) are "
              L"required, but are not available in your library:\n";

      for(size_t i = 0; i < missg_list.size(); ++i) {
        qry += L"\n ";
        qry += missg_list[i];
      }

      qry += L"\n\nDo you want to continue and force installation anyway ?";

      if(!Om_dialogBoxQuerryWarn(hWnd, L"Dependencies missing", qry))
        return;
    }
  }

  // we create the extra list, to warn user that additional package
  // will be installed. This is a reverse search from the user selection
  if(!quiet && this->_context->_manager->warnExtraInst()) {

    vector<OmPackage*> extra_list; //< packages to install not selected by user

    for(size_t i = 0; i < insta_list.size(); ++i) {

      // add only if the package is not in the user selection and
      // not already installed
      unique = true;
      for(size_t j = 0; j < selec_list.size(); ++j) {
        if(insta_list[i] == this->_package[selec_list[j]]) {
          unique = false; break;
        }
      }

      if(unique) {
        extra_list.push_back(insta_list[i]);
      }
    }

    // if we got some additional dependencies, we built the warning message
    // and ask user to continue (or not)
    if(extra_list.size()) {

      // we have some additional package to install, we warn the user
      // and ask him if he want to continue
      wstring qry = L"One or more selected package(s) have dependencies, the "
                     L"following package(s) also need to be installed:\n";

      for(size_t i = 0; i < extra_list.size(); ++i) {
        qry += L"\n ";
        qry += extra_list[i]->ident();
      }

      qry += L"\n\nContinue installation ?";

      if(!Om_dialogBoxQuerryWarn(hWnd, L"Package(s) installation dependencies", qry))
        return;
    }
  }

  // we now add the user selection into our real install list
  for(size_t i = 0; i < selec_list.size(); ++i) {

    // we add only if it is not already in the installation list created
    // from dependencies tree exploration
    unique = true;
    for(size_t j = 0; j < insta_list.size(); ++j) {
      if(this->_package[selec_list[i]] == insta_list[j]) {
        unique = false; break;
      }
    }

    if(unique) {
      insta_list.push_back(this->_package[selec_list[i]]);
    }
  }


  // this is to update list view item's icon individually so to avoid
  // full refresh of the page. Refreshing all page each time work well but
  // it is always good to optimize and doing clean things when possible.
  LVITEMW lvi;
  lvi.mask = LVIF_IMAGE;
  lvi.iSubItem = 0;

  for(size_t i = 0; i < insta_list.size(); ++i) {

    // check whether abort is requested
    if(pAbort) {
      if(*pAbort) break;
    }

    // we need the index of this package in list to change the image
    // in the list view control, this is not really elegant, but we have
    // no other choice
    lvi.iItem = packageIndex(insta_list[i]);
    lvi.iImage = 4; // IDB_PKG_PRC
    if(hLv) SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));

    if(insta_list[i]->hasSource() && !insta_list[i]->hasBackup()) {


      // we check overlapping before installation, we must do it in order
      // step by step because overlapping are cumulative over previously
      // installed packages, this is why the warning is disabled by default
      olaps_list.clear();

      this->getInstOwList(olaps_list, insta_list[i]);

      if(!quiet && this->_context->_manager->warnOverlaps()) {
        // we create a warning to ask user if he really want to continue
        // the installation
        if(olaps_list.size()) {

          wstring qry = L"Installing the package \"";
          qry += insta_list[i]->name();
          qry += L"\" will overlaps and modify file(s) previously installed or "
                  L"modified by the following package(s):\n";

          for(size_t j = 0; j < olaps_list.size(); ++j) {
            qry += L"\n ";
            qry += olaps_list[j]->name();
          }

          qry += L"\n\nDo you want to continue installation anyway ?";

          if(!Om_dialogBoxQuerryWarn(hWnd, L"Package(s) installation overlap", qry)) {
            break;
          }
        }
      }



      if(insta_list[i]->install(this->_backupZipLevel, hPb, pAbort)) { //< pass Progress Bar Handle

        // ensure package is installed, ti may be aborted
        if(insta_list[i]->hasBackup()) {

          // select proper image depending installation status
          if(this->isBakcupOverlapped(insta_list[i])) {
            lvi.iImage = 6; //< IDB_PKG_OWR
          } else {
            lvi.iImage = 5; //< IDB_PKG_BCK
          }
        } else {
          lvi.iImage = -1; //< not installed
        }

      } else {
        wstring err = L"The package \"";
        err += insta_list[i]->name();
        err += L"\" has not been installed because the following error occurred:\n\n";
        err += insta_list[i]->lastError();
        Om_dialogBoxErr(hWnd, L"Package install failed", err);
        lvi.iImage = -1; //< not installed
      }
    }

    if(hLv) SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));

    // reset progress bar
    if(hPb) SendMessageW(hPb, PBM_SETPOS, 0, 0);

    #ifdef DEBUG_SLOW
    Sleep(DEBUG_SLOW); //< for debug
    #endif
  }

  // reset progress bar
  if(hPb) SendMessageW(hPb, PBM_SETPOS, 0, 0);

  // package status may change after installation or backup restoration of
  // others, to ensure all icons are reflect the real status, we make an final
  // global update of the list. We avoid recreating the list at this stage, this
  // more convenient than a full refresh.
  if(hLv) {
    for(size_t i = 0; i < this->_package.size(); ++i) {
      lvi.iItem = i;
      if(this->_package[i]->isType(PKG_TYPE_BCK)) {
        if(this->isBakcupOverlapped(this->_package[i])) {
          lvi.iImage = 6; //< IDB_PKG_OWR
        } else {
          lvi.iImage = 5; //< IDB_PKG_BCK
        }
      } else {
        lvi.iImage = -1; //< none
      }
      SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
    }
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::packagesUnin(const vector<unsigned>& selec_list, bool quiet, HWND hWnd, HWND hLv, HWND hPb, const bool *pAbort)
{
  // checks whether we have a valid Destination folder
  if(!this->checkAccessDst()) {
    this->_error = L"Destination folder \""+this->_installDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Packages Uninstall", this->_error);
    Om_dialogBoxErr(hWnd, L"Package(s) uninstallation aborted", this->_error);
    return;
  }

  // checks whether we have a valid Backup folder
  if(!this->checkAccessBck()) {
    this->_error = L"Backup folder \""+this->_backupDir+L"\"";
    this->_error += OMM_STR_ERR_DIRACCESS;
    this->log(1, L"Location("+this->_title+L") Packages Uninstall", this->_error);
    Om_dialogBoxErr(hWnd, L"Package(s) uninstallation aborted", this->_error);
    return;
  }

  vector<OmPackage*> uninst_list; //< our real uninstall list

  // Build the real packages uninstall list including dependencies created
  // when packages installation overlaps others:
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
  for(size_t i = 0; i < selec_list.size(); ++i) {

    // get the dependencies for this package, recursively. This will
    // also give us a unsinstall list in the right order, built by tree
    // exploration in depth
    this->getUninOwList(uninst_list, selec_list[i]);
  }

  bool unique;

  // create the extra unsinstall list, i.e the new packages to be unsinstalled
  // in addition to the user selected ones.
  //
  // This process have two purposes, the first is to avoid doubles, since user
  // selection may already contain packages found during dependencies listing,
  // the second is to provide a concise list to warn user of additional package
  // to be unsinstalled.
  if(!quiet && this->_context->_manager->warnExtraUnin()) {

    vector<OmPackage*> extra_list; //< package to uninstall not selected by user

    for(size_t i = 0; i < uninst_list.size(); ++i) {

      // add only if the package is not in the user selection
      unique = true;
      for(size_t j = 0; j < selec_list.size(); ++j) {
        if(uninst_list[i] == this->_package[selec_list[j]]) {
          unique = false; break;
        }
      }

      if(unique)
        extra_list.push_back(uninst_list[i]);
    }

    // if we got some additional dependencies, we built the warning message
    // and ask user to continue (or not)
    if(extra_list.size()) {

      // we have some additional package to uninstall, we warn the user
      // and ask him if he want to continue
      wstring qry = L"One or more selected package(s) are currently overlapped "
                    L"by other(s). To keep backups integrity the following "
                    L"Package(s) will also be uninstalled:\n";

      for(size_t i = 0; i < extra_list.size(); ++i) {
        qry += L"\n ";
        qry += extra_list[i]->name();
      }

      qry += L"\n\nDo you want to continue anyway ?";

      if(!Om_dialogBoxQuerryWarn(hWnd, L"Backup(s) restoration overlap", qry))
        return;
    }
  }

  // we now add the user selection into our real restoration list
  for(size_t i = 0; i < selec_list.size(); ++i) {

    // we add only if it is not already in the restoration list created
    // from dependencies tree exploration
    unique = true;
    for(size_t j = 0; j < uninst_list.size(); ++j) {
      if(this->_package[selec_list[i]] == uninst_list[j]) {
        unique = false; break;
      }
    }

    if(unique)
      uninst_list.push_back(this->_package[selec_list[i]]);
  }



  // this is to update list view item's icon individually so to avoid
  // full refresh of the page. Refreshing all page each time work well but
  // it is always good to optimize and doing clean things when possible.
  LVITEMW lvi;
  lvi.mask = LVIF_IMAGE;
  lvi.iSubItem = 0;

  for(size_t i = 0; i < uninst_list.size(); ++i) {

    // check whether abort is requested
    if(pAbort) {
      if(*pAbort) break;
    }

    // we need the index of this package in list to change the image
    // in the list view control, this is not really elegant, but we have
    // no other choice
    lvi.iItem = packageIndex(uninst_list[i]);
    lvi.iImage = 4;
    if(hLv) SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));

    if(uninst_list[i]->hasBackup()) { // <- this should be always the case

      // we uninstall package and restore backup, without forcing for recursive
      // restoration since our restoration list is already ordered according
      // dependency tree
      if(uninst_list[i]->uninst(hPb, pAbort)) {

        // ensure package backup is restored, it may be aborted
        if(!uninst_list[i]->hasBackup()) {
          lvi.iImage = -1; //< this one is uninstalled
        } else {
          // set proper image depending install status
          if(this->isBakcupOverlapped(uninst_list[i])) {
            lvi.iImage = 6; //< IDB_PKG_OWR
          } else {
            lvi.iImage = 5; //< IDB_PKG_BCK
          }
        }

      } else {
        wstring err = L"The backup of \"";
        err += uninst_list[i]->name();
        err += L"\" has not been properly restored because the following error occurred:\n\n";
        err += uninst_list[i]->lastError();
        Om_dialogBoxErr(hWnd, L"Package uninstall failed", err);
        if(this->isBakcupOverlapped(uninst_list[i])) {
          lvi.iImage = 6; //< IDB_PKG_OWR
        } else {
          lvi.iImage = 5; //< IDB_PKG_BCK
        }

      }
    }

    if(hLv) SendMessageW(hLv, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));

    // reset progress bar
    if(hPb) SendMessageW(hPb, PBM_SETPOS, 0, 0);

    #ifdef DEBUG_SLOW
    Sleep(DEBUG_SLOW); //< for debug
    #endif
  }

  // reset progress bar
  if(hPb) SendMessageW(hPb, PBM_SETPOS, 0, 0);

  // package status may change after installation or backup restoration of
  // others, to ensure all icons are reflect the real status, we make an final
  // global update of the list. We avoid recreating the list at this stage, this
  // more convenient than a full refresh.
  if(hLv) {
    for(size_t i = 0; i < _package.size(); ++i) {
      lvi.iItem = i;
      if(this->_package[i]->isType(PKG_TYPE_BCK)) {
        if(this->isBakcupOverlapped(this->_package[i])) {
          lvi.iImage = 6; //< IDB_PKG_OWR
        } else {
          lvi.iImage = 5; //< IDB_PKG_BCK
        }
      } else {
        lvi.iImage = -1; //< none
      }
      SendMessageW(hLv ,LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::getInstOwList(vector<OmPackage*>& pkg_list, const OmPackage* package) const
{
  size_t n = 0;

  for(size_t i = 0; i < _package.size(); ++i) {
    if(package->couldOverlap(this->_package[i])) {
      pkg_list.push_back(this->_package[i]);
      ++n;
    }
  }

  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::getInstOwList(vector<uint64_t>& hash_list, const OmPackage* package) const
{
  size_t n = 0;

  for(size_t i = 0; i < _package.size(); ++i) {
    if(package->couldOverlap(this->_package[i])) {
      hash_list.push_back(this->_package[i]->hash());
      ++n;
    }
  }

  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::getInstDpList(vector<OmPackage*>& pkg_list, vector<wstring>& miss_list, const OmPackage* package) const
{
  size_t n = 0;

  bool unique;
  bool dpend_found;
  uint64_t file_hash;

  for(size_t i = 0; i < package->dependCount(); ++i) {

    file_hash = Om_getXXHash3(package->depend(i));

    dpend_found = false;
    for(size_t j = 0; j < this->_package.size(); ++j) {

      if(file_hash == this->_package[j]->hash()) {

        n += this->getInstDpList(pkg_list, miss_list, this->_package[j]);
        // we add to list only if unique and not already installed, this allow
        // us to get a consistent dependency list for a bunch of package by
        // calling this function for each package without clearing the list
        if(!this->_package[j]->hasBackup()) {
          unique = true;
          for(unsigned k = 0; k < pkg_list.size(); ++k) {
            if(pkg_list[k] == this->_package[j]) {
              unique = false; break;
            }
          }
          if(unique) {
            pkg_list.push_back(this->_package[j]);
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
        if(miss_list[j] == package->depend(i)) {
          unique = false; break;
        }
      }
      if(unique) {
        miss_list.push_back(package->depend(i));
      }
    }
  }

  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::getUninOwList(vector<OmPackage*>& pkg_list, const OmPackage* package) const
{
  size_t n = 0;
  bool unique;
  for(size_t i = 0; i < this->_package.size(); ++i) {
    if(this->_package[i]->hasOverlap(package->_hash)) {

      // the function is recursive, we want the full list like a
      // depth-first search in the right order
      n += this->getUninOwList(pkg_list, this->_package[i]);

      // recursive way can produce doubles, we want to avoid it
      // so we add only if not already in the list
      unique = true;
      for(size_t j = 0; j < pkg_list.size(); ++j) {
        if(pkg_list[j] == this->_package[i]) {
          unique = false; break;
        }
      }

      if(unique) {
        pkg_list.push_back(this->_package[i]);
        ++n;
      }
    }
  }
  return n;
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
