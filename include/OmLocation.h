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


/// \brief List sorting types
///
/// Location packages and remote packages lists sorting type.
///
enum OmLocLsSort : unsigned {
  LS_SORT_STAT = 0x1,
  LS_SORT_NAME = 0x2,
  LS_SORT_VERS = 0x4,
  LS_SORT_SIZE = 0x8,
  LS_SORT_REVERSE = 0x100
};

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

    /// \brief Get Destination path.
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

    /// \brief Get upgrade rename mode.
    ///
    /// Returns defined upgrade by rename mode for remote packages.
    ///
    /// \return Upgrade rename mode enabled.
    ///
    bool upgdRename() const {
      return _upgdRename;
    }
    /// \brief Get package legacy support size option.
    ///
    /// Returns package legacy support option value.
    ///
    /// \return Package legacy support option value.
    ///
    bool libDevMode() const {
      return _libDevMode;
    }

    /// \brief Set package legacy support option.
    ///
    /// Define and save package legacy support option value.
    ///
    /// \param[in]  enable    : Package legacy support enable or disable.
    ///
    void setLibDevMode(bool enable);

    /// \brief Get warning for overlaps option.
    ///
    /// Returns warning for overlaps option value.
    ///
    /// \return Warning for overlaps option value.
    ///
    bool warnOverlaps() const {
      return _warnOverlaps;
    }

    /// \brief Set warning for overlaps option.
    ///
    /// Define and save warning for overlaps option value.
    ///
    /// \param[in]  enable    : Warning for overlaps enable or disable.
    ///
    void setWarnOverlaps(bool enable);

    /// \brief Get warning for extra install option.
    ///
    /// Returns warning for extra install option value.
    ///
    /// \return Warning for extra install option value.
    ///
    bool warnExtraInst() const {
      return _warnExtraInst;
    }

    /// \brief Set warning for extra install option.
    ///
    /// Define and save warning for extra install option value.
    ///
    /// \param[in]  enable    : Warning for extra install enable or disable.
    ///
    void setWarnExtraInst(bool enable);

    /// \brief Get warning for missing dependency option.
    ///
    /// Returns warning for missing dependency option value.
    ///
    /// \return Warning for missing dependency option value.
    ///
    bool warnMissDeps() const {
      return _warnMissDeps;
    }

    /// \brief Set warning for missing dependency option.
    ///
    /// Define and save warning for missing dependency option value.
    ///
    /// \param[in]  enable    : Warning for missing dependency enable or disable.
    ///
    void setWarnMissDeps(bool enable);

    /// \brief Get warning for extra uninstall option.
    ///
    /// Returns warning for extra uninstall option value.
    ///
    /// \return Warning for extra uninstall option value.
    ///
    bool warnExtraUnin() const {
      return _warnExtraUnin;
    }

    /// \brief Set warning for extra uninstall option.
    ///
    /// Define and save warning for extra uninstall option value.
    ///
    /// \param[in]  enable    : Warning for extra uninstall enable or disable.
    ///
    void setWarnExtraUnin(bool enable);


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

    /// \brief Set upgrade rename mode.
    ///
    /// Defines and save upgrade rename mode for remote packages.
    ///
    /// \param[in]  enable   : Enable or upgrade rename mode.
    ///
    void setUpgdRename(bool always);

    /// \brief Verify Destination folder access.
    ///
    /// Checks whether the software has access to Destination folder for
    /// reading or reading & writing.
    ///
    /// \param[in]  rw  : Also check for write access.
    ///
    /// \return True if software has required access, false otherwise.
    ///
    bool dstDirAccess(bool rw = true);

    /// \brief Verify Library folder access.
    ///
    /// Checks whether the software has access to Library folder for
    /// reading or reading & writing.
    ///
    /// \param[in]  rw  : Also check for write access.
    ///
    /// \return True if software has required access, false otherwise.
    ///
    bool libDirAccess(bool rw = false);

    /// \brief Verify Backup folder access.
    ///
    /// Checks whether the software has access to Backup folder for
    /// reading or reading & writing.
    ///
    /// \param[in]  rw  : Also check for write access.
    ///
    /// \return True if software has required access, false otherwise.
    ///
    bool bckDirAccess(bool rw = true);

    /// \brief Clear Library.
    ///
    /// Empty and reset the packages list.
    ///
    /// \return True if list composition changed, false otherwise.
    ///
    bool libClear();

    /// \brief Refresh Library.
    ///
    /// Update or rebuild the packages list.
    ///
    /// \return True if list composition changed, false otherwise.
    ///
    bool libRefresh();

    /// \brief Clean Library.
    ///
    /// Remove ghost packages from packages list.
    ///
    /// \return True if list composition changed, false otherwise.
    ///
    bool libClean();

    /// \brief Set package list sorting.
    ///
    /// Sets the package list sorting, available values are the following:
    /// - \c LS_SORT_STAT : Sort by installation state
    /// - \c LS_SORT_NAME : Sort by name
    /// - \c LS_SORT_VERS : Sort by version
    /// - \c LS_SORT_SIZE : Sort by size
    ///
    /// \param[in]  sorting : Package list sorting type.
    ///
    void libSetSorting(OmLocLsSort sorting);

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
    /// \param[in]  dest        : Destination folder to move backup data to.
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return True if operation succeed, false is error occurred.
    ///
    bool bckMove(const wstring& dest, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Discard backup data.
    ///
    /// Discard/delete all backup data without restoring. This function is used as an
    /// emergency reset to prevent restoring old or corrupted data.
    ///
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return True if operation succeed, false if error occurred.
    ///
    bool bckDcard(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Purge backup data.
    ///
    /// Uninstall all installed packages to restore and delete all backup data.
    ///
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return True if operation succeed, false if error occurred.
    ///
    bool bckPurge(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Prepare packages installation.
    ///
    /// Creates the full installation list, additional install list and missing
    /// dependencies list from the given initial packages installation selection.
    ///
    /// \param[out] ins_ls  : Output full list of packages to be installed.
    /// \param[out] ovr_ls  : Output list of overlapped packages after installation.
    /// \param[out] dep_ls  : Output list of additional dependencies packages to be installed.
    /// \param[out] mis_ls  : Output list of missing dependencies identities.
    /// \param[in]  pkg_ls  : Input initial packages install selection to be installed.
    ///
    void pkgPrepareInst(vector<OmPackage*>& ins_ls, vector<OmPackage*>& ovr_ls, vector<OmPackage*>& dep_ls, vector<wstring>& mis_ls, const vector<OmPackage*>& pkg_ls) const;

    /// \brief Prepare bakcups restoration.
    ///
    /// Creates the full installation list, additional install list and missing
    /// dependencies list from the given initial packages installation selection.
    ///
    /// \param[out] uni_ls  : Output full list of packages to be uninstalled.
    /// \param[out] dep_ls  : Output list of additional packages which depend on selection.
    /// \param[out] ovr_ls  : Output list of additional packages which overlap the selection.
    /// \param[in]  pkg_ls  : Input initial packages install selection to be uninstalled.
    ///
    void bckPrepareUnin(vector<OmPackage*>& uni_ls, vector<OmPackage*>& dep_ls, vector<OmPackage*>& ovr_ls, const vector<OmPackage*>& pkg_ls) const;

    /// \brief Get installation overlap list.
    ///
    /// Retrieve the list of Packages the specified Package could overlap at
    /// installation.
    ///
    /// \param[in]  ovr_ls  : Array vector of Package object pointer to receive found packages.
    /// \param[in]  pkg     : Package to get backup overlap.
    ///
    /// \return Count of Package found.
    ///
    size_t pkgFindOverlaps(vector<OmPackage*>& ovr_ls, const OmPackage* pkg) const;

    /// \brief Get installation overlap list.
    ///
    /// Retrieve the list of Packages the specified Package could overlap at
    /// installation.
    ///
    /// \param[in]  hash_ls : Array vector of uint64_t to receive found packages Hash.
    /// \param[in]  pkg     : Package to get backup overlap.
    ///
    /// \return Count of Package found.
    ///
    size_t pkgFindOverlaps(vector<uint64_t>& hash_ls, const OmPackage* pkg) const;

    /// \brief Get installation dependency list.
    ///
    /// Retrieve the package installation dependencies.
    ///
    /// \param[in]  dep_ls  : Array vector of Package pointer to receive found packages.
    /// \param[in]  mis_ls  : Array vector of Package pointer to receive missing packages name.
    /// \param[in]  pkg     : Package to get installation dependencies.
    ///
    /// \return Count of Package found.
    ///
    size_t pkgGetDepends(vector<OmPackage*>& dep_ls, vector<wstring>& mis_ls, const OmPackage* pkg) const;

    /// \brief Check for remote package dependencies.
    ///
    /// Checks whether remote package have unsatisfied (not downloaded) dependencies. Notice
    /// that this function checks only
    ///
    /// \param[in]  pkg     : Package to get check dependencies.
    ///
    /// \return True if dependencies are satisfied, false otherwise.
    ///
    bool pkgChkDepends(const OmPackage* pkg) const;

    /// \brief Get backup overlaps list.
    ///
    /// Retrieve list of overlapping installed packages of the specified package.
    ///
    /// \param[in]  ovr_ls  : Array vector of Package pointer to receive list.
    /// \param[in]  pkg     : Package to get backup overlaps.
    ///
    /// \return Count of Package found.
    ///
    size_t bckGetOverlaps(vector<OmPackage*>& ovr_ls, const OmPackage* pkg) const;

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
    /// \param[in]  pkg   : Package to checks backup overlap.
    ///
    /// \return True if package is overlapped, false otherwise.
    ///
    bool bckOverlapped(const OmPackage* pkg) const {
      for(size_t i = 0; i < _pkgLs.size(); ++i) {
        if(_pkgLs[i]->ovrHas(pkg->_hash))
          return true;
      }
      return false;
    }

    /// \brief Get backup dependents list.
    ///
    /// Retrieve list of dependent packages of the specified installed target.
    ///
    /// \param[out] dpt_ls  : Array vector of Package pointer to receive list.
    /// \param[in]  pkg     : Package to get dependent packages list.
    ///
    /// \return Count of additional package found.
    ///
    size_t bckGetDependents(vector<OmPackage*>& dpt_ls, const OmPackage* pkg) const;

    /// \brief Get backup relations list.
    ///
    /// Retrieve both (and recursive) list of overlapping packages and
    /// are dependent packages of the the specified installed package.
    ///
    /// Notice that the routine performs dual test and recursive search,
    /// considering each additional uninstall (either because of overlaps,
    /// or by dependency) may expand the graph of overlaps and dependencies.
    ///
    /// This method produce a consistent, sorted and exhaustive uninstall
    /// list, but can be costly on huge packages list with lot of overlaps and
    /// dependencies.
    ///
    /// \param[out] rel_ls  : Output sorted and cumulative list of overlapping and dependents packages.
    /// \param[out] dep_ls  : Output list of packages which depend on the specified one.
    /// \param[out] ovr_ls  : Output list of packages which overlaps the specified one.
    /// \param[in]  pkg     : Package to get lists from.
    ///
    /// \return Count of additional package found.
    ///
    size_t bckGetRelations(vector<OmPackage*>& rel_ls, vector<OmPackage*>& dep_ls, vector<OmPackage*>& ovr_ls, const OmPackage* pkg) const;

    /// \brief Check whether backup item exists.
    ///
    /// Check whether any package has the backup item corresponding to
    /// the given criteria.
    ///
    /// \param[in]  path      : Backup item path.
    /// \param[in]  dest      : Backup item destination.
    ///
    /// \return True if item found, false otherwise.
    ///
    bool bckItemExists(const wstring& path, OmPkgItemDest dest) const;

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
    /// \param[in]  i       : Repository URL index to delete.
    ///
    void repRem(unsigned i);

    /// \brief Query repository.
    ///
    /// Query network repository to refresh available remote packages.
    ///
    /// \param[in]  i       : Repository URL index to query.
    ///
    /// \return True if operation succeed, false if error occurred.
    ///
    bool repQuery(unsigned i);

    /// \brief Get warning for extra download option.
    ///
    /// Returns warning for extra download option value.
    ///
    /// \return Warning for extra download option value.
    ///
    bool warnExtraDnld() const {
      return _warnExtraDnld;
    }

    /// \brief Set warning for extra download option.
    ///
    /// Define and save warning for extra download option value.
    ///
    /// \param[in]  enable    : Warning for extra download enable or disable.
    ///
    void setWarnExtraDnld(bool enable);

    /// \brief Get warning for missing remote dependency option.
    ///
    /// Returns warning for missing remote dependency option value.
    ///
    /// \return Warning for missing remote dependency option value.
    ///
    bool warnMissDnld() const {
      return _warnMissDnld;
    }

    /// \brief Set warning for missing remote dependency option.
    ///
    /// Define and save warning for missing remote dependency option value.
    ///
    /// \param[in]  enable    : Warning for missing remote dependency enable or disable.
    ///
    void setWarnMissDnld(bool enable);


    /// \brief Get warning for upgrade breaking dependencies option.
    ///
    /// Returns warning for upgrade breaking dependencies option value.
    ///
    /// \return Warning for upgrade breaking dependencies option value.
    ///
    bool warnUpgdBrkDeps() const {
      return _warnUpgdBrkDeps;
    }

    /// \brief Set warning for upgrade breaking dependencies.
    ///
    /// Define and save warning for upgrade breaking dependencies option value.
    ///
    /// \param[in]  enable    : Warning for upgrade breaking dependencies enable or disable.
    ///
    void setWarnUpgdBrkDeps(bool enable);

    /// \brief Refresh Remote list.
    ///
    /// Refresh remote package state against local packages lst.
    ///
    /// \param[in] force    : Force refresh even if no changes occurred in Library.
    ///
    /// \return True if any remote package state changed, false otherwise.
    ///
    bool rmtRefresh(bool force = false);

    /// \brief Set remote package list sorting.
    ///
    /// Sets the remote package list sorting, available values are the following:
    /// - \c LS_SORT_STAT : Sort by installation state
    /// - \c LS_SORT_NAME : Sort by name
    /// - \c LS_SORT_VERS : Sort by version
    /// - \c LS_SORT_SIZE : Sort by size
    ///
    /// \param[in]  sorting : Package list sorting type.
    ///
    void rmtSetSorting(OmLocLsSort sorting);

    /// \brief Get remote package list sorting.
    ///
    /// Sets the remote package list sorting.
    ///
    /// \return current remote package sorting mask.
    ///
    unsigned rmtGetSorting() const {
      return _rmtSorting;
    }

    /// \brief Get package count.
    ///
    /// Returns current package count in library.
    ///
    /// \return Package count.
    ///
    size_t rmtCount() const {
      return _rmtLs.size();
    }

    /// \brief Get package.
    ///
    /// Returns package at index.
    ///
    /// \param[in]  i       : Package index to get.
    ///
    /// \return Package.
    ///
    OmRemote* rmtGet(unsigned i) const {
      return _rmtLs[i];
    }

    /// \brief Find package by hash.
    ///
    /// Find package by its computed hash.
    ///
    /// \param[in] hash  : Package hash to search.
    ///
    /// \return Package or nullptr if not found.
    ///
    OmRemote* rmtFind(uint64_t hash) const {
      for(size_t i = 0; i < _rmtLs.size(); ++i) {
        if(_rmtLs[i]->hash() == hash) return _rmtLs[i];
      }
      return nullptr;
    }

    /// \brief Prepare remote packages download.
    ///
    /// Creates the full installation list, additional install list and missing
    /// dependencies list from the given initial remote packages download selection.
    ///
    /// \param[out] dnl_ls  : Output full list of remote packages to be downloaded.
    /// \param[out] dep_ls  : Output list of additional dependencies remote packages to be downloaded.
    /// \param[out] mis_ls  : Output list of missing dependencies identities.
    /// \param[out] old_ls  : Output list of old packages required as dependency that selection will supersedes.
    /// \param[in]  sel_ls  : Input initial packages install selection to be downloaded.
    ///
    void rmtPrepareDown(vector<OmRemote*>& dnl_ls, vector<OmRemote*>& dep_ls, vector<wstring>& mis_ls,  vector<OmPackage*>& old_ls, const vector<OmRemote*>& sel_ls) const;

    /// \brief Get remote package dependency list.
    ///
    /// Retrieve the remote package download dependencies.
    ///
    /// \param[in]  dep_ls  : Array vector of Remote package pointer to receive found packages.
    /// \param[in]  mis_ls  : Array vector of Remote package pointer to receive missing packages name.
    /// \param[in]  rmt     : Remote package to get download dependencies.
    ///
    /// \return Count of remote package found.
    ///
    size_t rmtGetDepends(vector<OmRemote*>& dep_ls, vector<wstring>& mis_ls, const OmRemote* rmt) const;

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _pkgSort();

    void                _rmtSort();

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

    bool                _libDevMode;

    wstring             _bckDir;

    bool                _bckDirCust;

    vector<OmPackage*>  _pkgLs;

    int                 _bckZipLevel;

    unsigned            _pkgSorting;

    vector<OmRepository*>  _repLs;

    vector<OmRemote*>   _rmtLs;

    bool                _upgdRename;

    unsigned            _rmtSorting;

    bool                _warnOverlaps;

    bool                _warnExtraInst;

    bool                _warnMissDeps;

    bool                _warnExtraUnin;

    bool                _warnExtraDnld;

    bool                _warnMissDnld;

    bool                _warnUpgdBrkDeps;

    bool                _valid;

    wstring             _error;
};

#endif // OMLOCATION_H
