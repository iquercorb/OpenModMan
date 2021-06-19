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

#ifndef OMLOCATION_H
#define OMLOCATION_H

#include "OmGlobal.h"
#include "OmConfig.h"
#include "OmPackage.h"
#include "OmRepository.h"

class OmContext;

#define PKG_SORTING_STAT 0x1
#define PKG_SORTING_NAME 0x2
#define PKG_SORTING_VERS 0x4
#define PKG_SORTING_REVERSE 0x100

/// \brief Location object for Context.
///
/// The Location object defines environment for package installation
/// destination, library and backup. The object provide interface to
/// store, manage, install and uninstall packages.
///
class OmLocation
{
  friend class OmPackage;

  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmLocation(OmContext* pCtx);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmLocation();

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const wstring& lastError() const {
      return _error;
    }

    /// \brief Check whether is valid.
    ///
    /// Checks whether this instance is correctly loaded a ready to use.
    ///
    /// \return True if this instance is valid, false otherwise.
    ///
    bool isValid() const {
      return _valid;
    }

    /// \brief Get Location UUID.
    ///
    /// Returns Location UUID.
    ///
    /// \return Location UUID.
    ///
    const wstring& uuid() const {
      return _uuid;
    }

    /// \brief Get Location title.
    ///
    /// Returns Location title.
    ///
    /// \return Location title.
    ///
    const wstring& title() const {
      return _title;
    }

    /// \brief Get Location index.
    ///
    /// Returns Location ordering index.
    ///
    /// \return Ordering index number.
    ///
    unsigned index() const {
      return _index;
    }

    /// \brief Get Location home directory.
    ///
    /// Returns home directory.
    ///
    /// \return Location home directory.
    ///
    const wstring& home() const {
      return _home;
    }

    /// \brief Get Location definition path.
    ///
    /// Returns Location definition file path.
    ///
    /// \return Location definition file path.
    ///
    const wstring& path() const {
      return _path;
    }

    /// \brief Get Location destination path.
    ///
    /// Returns Location packages installation destination directory.
    ///
    /// \return Location destination directory.
    ///
    const wstring& dstDir() const {
      return _dstDir;
    }

    /// \brief Get Location library path.
    ///
    /// Returns Location packages library directory.
    ///
    /// \return Packages library directory.
    ///
    const wstring& libDir() const {
      return _libDir;
    }

    /// \brief Get Location backup path.
    ///
    /// Returns Location packages installation backup directory.
    ///
    /// \return Backup directory.
    ///
    const wstring& bckDir() const {
      return _bckDir;
    }

    /// \brief Get backup compression level.
    ///
    /// Returns defined backup compression level.
    ///
    /// \return Zip compression level.
    ///
    int bckZipLevel() const {
      return _bckZipLevel;
    }

    /// \brief Verify Library folder access.
    ///
    /// Checks whether the Library folder is accessible. If Library folder is
    /// not valid, the function prompt a message dialog box to warn user and
    /// select a valid folder.
    ///
    /// \return True if Library folder is accessible, false otherwise.
    ///
    bool checkAccessLib();

    /// \brief Verify Backup folder access.
    ///
    /// Checks whether the Backup folder is accessible. If Backup folder is
    /// not valid, the function prompt a message dialog box to warn user and
    /// select a valid folder.
    ///
    /// \return True if Backup folder is accessible, false otherwise.
    ///
    bool checkAccessBck();

    /// \brief Verify Location folder access.
    ///
    /// Checks whether the Location folder is accessible. If Location folder is
    /// not valid, the function prompt a message dialog box to warn user and
    /// select a valid folder.
    ///
    /// \return True if Location folder is accessible, false otherwise.
    ///
    bool checkAccessDst();

    /// \brief Open Location.
    ///
    /// Load Location from specified file.
    ///
    /// \param[in]  path    : File path of Location to be loaded.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool open(const wstring& path);

    /// \brief Close Location.
    ///
    /// Close and empty the current instance.
    ///
    void close();

    /// \brief Set Location title.
    ///
    /// Defines and save Location title.
    ///
    /// \param[in]  title   : Title to defines and save
    ///
    void setTitle(const wstring& title);

    /// \brief Set Location index.
    ///
    /// Defines and save Location index for ordering.
    ///
    /// \param[in]  index   : Index number to defines and save
    ///
    void setIndex(unsigned index);

    /// \brief Set Location destination path.
    ///
    /// Defines and save Location installation destination path.
    ///
    /// \param[in]  path    : destination path to save.
    ///
    void setDstDir(const wstring& path);

    /// \brief Set custom Library folder.
    ///
    /// Defines and save a custom package Library folder. If custom
    /// Library folder is not defined, the default location is used:
    /// <Location path>\library
    ///
    /// \param[in]  path    : Custom Library folder path to save.
    ///
    void setCustLibDir(const wstring& path);

    /// \brief Check for custom Library.
    ///
    /// Checks whether this instance currently has and use a custom
    /// Library folder path.
    ///
    /// \return True if a custom Library path is used, false otherwise.
    ///
    bool hasCustLibDir() const {
      return _libDirCust;
    }

    /// \brief Remove custom Backup.
    ///
    /// Removes the current custom Backup configuration and reset to
    /// default settings.
    ///
    void remCustLibDir();

    /// \brief Set custom Backup folder.
    ///
    /// Defines and save a custom package Backup folder. If custom
    /// Backup folder is not defined, the default location is used:
    /// <Location path>\backup
    ///
    /// \param[in]  path    : Custom Backup folder path to save.
    ///
    void setCustBckDir(const wstring& path);

    /// \brief Check for custom Backup.
    ///
    /// Checks whether this instance currently has and use a custom
    /// Backup folder path.
    ///
    /// \return True if a custom Backup path is used, false otherwise.
    ///
    bool hasCustBckDir() const {
      return _bckDirCust;
    }

    /// \brief Remove custom Backup.
    ///
    /// Removes the current custom Backup configuration and reset to
    /// default settings.
    ///
    void remCustBckDir();

    /// \brief Set Backup compression level.
    ///
    /// Defines and save backup compression level. A negative value
    /// means create directory backup instead of zipped backup.
    ///
    /// \param[in]  level   : Zip compression level to save.
    ///
    void setBckZipLevel(int level);

    /// \brief Clear Location package list.
    ///
    /// Resets Location package list.
    ///
    void libClear();

    /// \brief Refresh Location package list.
    ///
    /// Refreshes the Location package list.
    ///
    void libRefresh();

    /// \brief Set package list sorting.
    ///
    /// Sets the package list sorting, available values are the following:
    /// - \c PKG_SORTING_STAT : Sort by installation state
    /// - \c PKG_SORTING_NAME : Sort by name
    /// - \c PKG_SORTING_VERS : Sort by version
    ///
    /// \param[in]  sorting : Package list sorting type.
    ///
    void libSetSorting(unsigned sorting);

    /// \brief Get package list sorting.
    ///
    /// Sets the package list sorting.
    ///
    /// \return current package sorting mask.
    ///
    unsigned libGetSorting() const {
      return _pkgSorting;
    }

    /// \brief Get package count.
    ///
    /// Returns current package count in library.
    ///
    /// \return Package count.
    ///
    size_t pkgCount() const {
      return _pkgLs.size();
    }

    /// \brief Get package.
    ///
    /// Returns package at index.
    ///
    /// \param[in]  i       : Package index to get.
    ///
    /// \return Package.
    ///
    OmPackage* pkgGet(unsigned i) const {
      return _pkgLs[i];
    }

    /// \brief Find package by hash.
    ///
    /// Find package by its computed hash.
    ///
    /// \param[in] hash  : Package hash to search.
    ///
    /// \return Package or nullptr if not found.
    ///
    OmPackage* pkgFind(uint64_t hash) const {
      for(size_t i = 0; i < _pkgLs.size(); ++i) {
        if(_pkgLs[i]->hash() == hash) return _pkgLs[i];
      }
      return nullptr;
    }

    /// \brief Get package index.
    ///
    /// Returns package index in list.
    ///
    /// \param[in]  pPkg    : Package to get index of.
    ///
    /// \return Package index or -1 if not found.
    ///
    int pkgIndex(const OmPackage* pPkg) const {
      for(size_t i = 0; i < _pkgLs.size(); ++i) {
        if(pPkg == _pkgLs[i]) return i;
      }
      return -1;
    }

    /// \brief Find package index by hash.
    ///
    /// Find package index in list by its computed hash.
    ///
    /// \param[in] hash      : Package hash to search.
    ///
    /// \return Package index or -1 if not found.
    ///
    int pkgIndex(uint64_t hash) const{
      for(size_t i = 0; i < _pkgLs.size(); ++i) {
        if(_pkgLs[i]->hash() == hash) return i;
      }
      return -1;
    }

    /// \brief Rename Location
    ///
    /// Truly rename Location definition name and home folder according
    /// the given new name.
    ///
    /// \param[in]  name   : New name to rename subfolder and definition file.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool renameHome(const wstring& name);

    /// \brief Check whether has backup data
    ///
    /// Checks whether the Location currently has one or more backup data
    /// to be restored.
    ///
    /// \return True Location currently has backup data, false otherwise.
    ///
    bool bckHasData();

    /// \brief Move backup
    ///
    /// If destination folder is different from the current Backup folder,
    /// moves all current Backup folder content to the specified location.
    ///
    /// Notice this function only moves data and does not changes the
    /// current Backup folder path. This function should be used in conjunction
    /// with Location.setCustBackup or Location.remCustBackup.
    ///
    /// \param[in]  dest      : Destination folder to move backup data to.
    /// \param[in]  hPb       : Progress Bar control handle to be updated during process.
    /// \param[in]  hSc       : Static Label control handle to be updated during process.
    /// \param[in]  pAbort    : Pointer to boolean to cancel operation.
    ///
    /// \return True if operation succeed, false is error occurred.
    ///
    bool bckMove(const wstring& dest, HWND hPb = nullptr, HWND hSc = nullptr, const bool *pAbort = nullptr);

    /// \brief Cleanup backup data.
    ///
    /// Uninstall all installed package package(s) to cleanup all backup data.
    ///
    /// \param[in]  hWnd       : Parent window handle for warning messages.
    /// \param[in]  hPb        : Progress Bar control handle to be updated during process.
    /// \param[in]  hSc        : Static Label control handle to be updated during process.
    /// \param[in]  pAbort     : Pointer to boolean to cancel operation.
    ///
    /// \return True if operation succeed, false is error occurred.
    ///
    bool bckPurge(HWND hPb = nullptr, HWND hSc = nullptr, const bool *pAbort = nullptr);

    /// \brief Discard backup data.
    ///
    /// Discard/delete all backup data without restoring. This function is used as an
    /// emergency reset to prevent restoring old or corrupted data.
    ///
    /// \param[in]  hWnd       : Parent window handle for warning messages.
    /// \param[in]  hPb        : Progress Bar control handle to be updated during process.
    /// \param[in]  hSc        : Static Label control handle to be updated during process.
    /// \param[in]  pAbort     : Pointer to boolean to cancel operation.
    ///
    /// \return True if operation succeed, false is error occurred.
    ///
    bool bckDiscard(HWND hPb = nullptr, HWND hSc = nullptr, const bool *pAbort = nullptr);

    /// \brief Install package(s).
    ///
    /// Installs one or several package(s) according Location environment.
    ///
    /// \param[in]  selec_list  : List of package index to install.
    /// \param[in]  quiet       : Do not throw warning messages.
    /// \param[in]  hWnd        : Parent window handle for warning messages.
    /// \param[in]  hLv         : List View control handle to be updated during process.
    /// \param[in]  hPb         : Progress Bar control handle to be updated during process.
    /// \param[in]  hSc         : Progress Bar control handle to be updated during process.
    /// \param[in]  pAbort      : Pointer to boolean to cancel operation.
    ///
    void pkgInst(const vector<unsigned>& selec_list, bool quiet, HWND hWnd = nullptr, HWND hLv = nullptr, HWND hPb = nullptr, const bool *pAbort = nullptr);

    /// \brief Uninstall package(s).
    ///
    /// Uninstall one or several package(s) according Location environment.
    ///
    /// \param[in]  selec_list  : List of package index to uninstall.
    /// \param[in]  quiet       : Do not throw warning messages.
    /// \param[in]  hWnd        : Parent window handle (HWND) for warning messages.
    /// \param[in]  hLv         : List View control handle (HWND) to be updated during process.
    /// \param[in]  hSc         : Progress Bar control handle (HWND) to be updated during process.
    /// \param[in]  pAbort      : Pointer to boolean to cancel operation.
    ///
    void pkgUnin(const vector<unsigned>& selec_list, bool quiet, HWND hWnd = nullptr, HWND hLv = nullptr, HWND hPb = nullptr, const bool *pAbort = nullptr);

    /// \brief Get installation overlap list.
    ///
    /// Retrieve the list of Packages the specified Package could overlap at
    /// installation.
    ///
    /// \param[in]  pkg_list  : Array vector of Package object pointer to receive found packages.
    /// \param[in]  package   : Package to get backup overlap.
    ///
    /// \return Count of Package found.
    ///
    size_t pkgFindOverlaps(vector<OmPackage*>& pkg_list, const OmPackage* package) const;

    /// \brief Get installation overlap list.
    ///
    /// Retrieve the list of Packages the specified Package could overlap at
    /// installation.
    ///
    /// \param[in]  hash_list : Array vector of uint64_t to receive found packages Hash.
    /// \param[in]  package   : Package to get backup overlap.
    ///
    /// \return Count of Package found.
    ///
    size_t pkgFindOverlaps(vector<uint64_t>& hash_list, const OmPackage* package) const;

    /// \brief Get installation dependency list.
    ///
    /// Retrieve the package installation dependencies.
    ///
    /// \param[in]  pkg_list  : Array vector of Package pointer to receive found packages.
    /// \param[in]  miss_list : Array vector of Package pointer to receive missing packages name.
    /// \param[in]  i         : Package index in Library to get installation dependencies.
    ///
    /// \return Count of Package found.
    ///
    size_t pkgGetDepends(vector<OmPackage*>& pkg_list, vector<wstring>& miss_list, unsigned i) const {
      return pkgGetDepends(pkg_list, miss_list, _pkgLs[i]);
    }

    /// \brief Get installation dependency list.
    ///
    /// Retrieve the package installation dependencies.
    ///
    /// \param[in]  pkg_list  : Array vector of Package pointer to receive found packages.
    /// \param[in]  miss_list : Array vector of Package pointer to receive missing packages name.
    /// \param[in]  package   : Package to get installation dependencies.
    ///
    /// \return Count of Package found.
    ///
    size_t pkgGetDepends(vector<OmPackage*>& pkg_list, vector<wstring>& miss_list, const OmPackage* package) const;

    /// \brief Get restoration dependency list.
    ///
    /// Retrieve the package restoration dependencies, meaning, the list
    /// of overlapping backup of this package.
    ///
    /// \param[in]  pkg_list  : Array vector of Package pointer to receive list.
    /// \param[in]  i         : Package index in Library to get restoration dependencies.
    ///
    /// \return Count of Package found.
    ///
    size_t bckGetOverlaps(vector<OmPackage*>& pkg_list, unsigned i) const {
      return bckGetOverlaps(pkg_list, _pkgLs[i]);
    }

    /// \brief Get restoration dependency list.
    ///
    /// Retrieve the package restoration dependencies, meaning, the list
    /// of overlapping backup of this package.
    ///
    /// \param[in]  pkg_list  : Array vector of Package pointer to receive list.
    /// \param[in]  package   : Package to get restoration dependencies.
    ///
    /// \return Count of Package found.
    ///
    size_t bckGetOverlaps(vector<OmPackage*>& pkg_list, const OmPackage* package) const;

    /// \brief check backup overlap.
    ///
    /// Checks whether the given package backup is overlapped by one or more
    /// other package(s).
    ///
    /// \param[in]  i           : Package index in Library to checks backup overlap.
    ///
    /// \return True if package is overlapped, false otherwise.
    ///
    bool bckOverlapped(unsigned i) const {
      return bckOverlapped(_pkgLs[i]);
    }

    /// \brief check backup overlap.
    ///
    /// Checks whether the given package backup is overlapped by one or more
    /// other package(s).
    ///
    /// \param[in]  package     : Package to checks backup overlap.
    ///
    /// \return True if package is overlapped, false otherwise.
    ///
    bool bckOverlapped(const OmPackage* package) const {
      for(size_t i = 0; i < _pkgLs.size(); ++i) {
        if(_pkgLs[i]->ovrHas(package->_hash))
          return true;
      }
      return false;
    }

    /// \brief Get repository URL count.
    ///
    /// Returns current repository URL count in library.
    ///
    /// \return Repository URL count.
    ///
    size_t repCount() const {
      return _repLs.size();
    }

    /// \brief Get repository URL.
    ///
    /// Returns defined repository URL at given index.
    ///
    /// \param[in]  i       : Repository URL index to get.
    ///
    /// \return Repository URL string.
    ///
    OmRepository* repGet(unsigned i) const {
      return _repLs[i];
    }

    /// \brief Remove custom Backup.
    ///
    /// Removes the current custom Backup configuration and reset to
    /// default settings.
    ///
    /// \param[in]  base  Repository base URL.
    /// \param[in]  name  Repository name.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool repAdd(const wstring& base, const wstring& name);

    /// \brief Remove custom Backup.
    ///
    /// Removes the current custom Backup configuration and reset to
    /// default settings.
    ///
    void repRem(unsigned i);

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _pkgSort();

    OmContext*          _context;

    OmConfig            _config;

    wstring             _uuid;

    wstring             _title;

    unsigned            _index;

    wstring             _home;

    wstring             _path;

    wstring             _dstDir;

    wstring             _libDir;

    bool                _libDirCust;

    wstring             _bckDir;

    bool                _bckDirCust;

    vector<OmPackage*>  _pkgLs;

    int                 _bckZipLevel;

    unsigned            _pkgSorting;

    vector<OmRepository*>  _repLs;

    bool                _valid;

    wstring             _error;
};

#endif // OMLOCATION_H
