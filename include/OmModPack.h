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
#ifndef OMMODNODE_H
#define OMMODNODE_H

#include "OmBase.h"

#include "OmImage.h"
#include "OmVersion.h"

class OmModChan;

/// \brief Mod Entry Attributes
///
/// Attributes mask for Mod Entry structure
///
enum OmModEntAttr : int32_t {
  OM_MODENTRY_DIR    =   0x1,
  OM_MODENTRY_DEL    =   0x2
};

/// \brief Mod Entry structure
///
/// Structure to describe a Mod entry, which is a file or folder to be
/// installed or restored which Package or Backup contain or references.
///
typedef struct OmModEntry_
{
  int32_t       attr;   ///< Entry attributes bits
  OmWString     path;   ///< Entry relative path
  int32_t       cdid;   ///< Entry zip central-directory index

} OmModEntry_t;

/// \brief OmModEntry_t array
///
/// Typedef for an STL vector of OmModEntry_t type
///
typedef std::vector<OmModEntry_t> OmModEntryArray;

/// \brief Package default category count.
///
/// Package default Mod category count.
///
const size_t OmPkgCatCnt = 12;

/// \brief Package default category list.
///
/// Package default Mod category string list.
///
const wchar_t OmModCategory[][16] = {
  L"GENERIC",
  L"TEXTURE",
  L"SKIN",
  L"MODEL",
  L"LEVEL",
  L"MISSION",
  L"UI",
  L"AUDIO",
  L"FEATURE",
  L"PLUGIN",
  L"SCRIPT",
  L"PATCH",
};

const size_t OmModCategoryCount = sizeof(OmModCategory) / 16;

class OmModPack
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmModPack();

    /// \brief Constructor.
    ///
    /// Constructor with parent Mod Channel
    ///
    /// \param[in] ModChan  : Parent Mod Channel
    ///
    OmModPack(OmModChan* ModChan);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmModPack();

    /// \brief Clear properties
    ///
    /// Clear all Mod general properties, parsed source and backup data.
    ///
    void clearAll();

    /// \brief Clear analytical properties
    ///
    /// Clear status and analytical properties related to previous operation
    /// such as install or restoration
    ///
    void clearAnalytics();

    /// \brief Parse Mod Source
    ///
    /// Try to load and parse zip file or directory to be used as
    /// source for Mod install.
    ///
    /// \param[in]  path    : Path to Source file or folder to parse.
    ///
    /// \return True operation succeed, false otherwise
    ///
    bool parseSource(const OmWString& path);

    /// \brief Refresh Mod source
    ///
    /// Check for source file or directory modification time and parse
    /// again if required.
    ///
    /// \return True if source required a refresh, false otherwise
    ///
    bool refreshSource();

    /// \brief Revoke and clear Source
    ///
    /// Clear parsed data and parameters of the Source side of this instance.
    ///
    void clearSource();

    /// \brief Instance has Source
    ///
    /// Check whether this instance has valid Source side to be installed.
    ///
    /// \return True instance has valid Source, false otherwise.
    ///
    bool hasSource() const {
      return this->_has_src;
    }

    /// \brief Source is a directory
    ///
    /// Check whether the Source side of this instance is a directory.
    ///
    /// \return True Source side is a directory, false otherwise.
    ///
    bool sourceIsDir() const {
      return this->_src_isdir;
    }

    /// \brief Source path
    ///
    /// Get source file or directory path
    ///
    /// \return wide string
    ///
    const OmWString& sourcePath() const {
      return this->_src_path;
    }

    /// \brief Source entry count
    ///
    /// Get count of parsed Source entry
    ///
    /// \return wide string
    ///
    size_t sourceEntryCount() const {
      return this->_src_entry.size();
    }

    /// \brief Get Source entry
    ///
    /// Returns Source entry at specified index
    ///
    /// \param[in] i  : Entry index to get
    ///
    /// \return wide string
    ///
    const OmModEntry_t& getSourceEntry(size_t i) const {
      return this->_src_entry[i];
    }

    /// \brief Parse Mod Backup
    ///
    /// Try to load and parse zip file or directory to be used as
    /// backup for Mod uninstall.
    ///
    /// \param[in]  path    : Path to Source file or folder to parse.
    ///
    /// \return True operation succeed, false otherwise
    ///
    bool parseBackup(const OmWString& path);

    /// \brief Revoke and clear Backup
    ///
    /// Clear parsed data and parameters of the Backup side of this instance.
    ///
    void clearBackup();

    /// \brief Instance has Backup
    ///
    /// Check whether this instance has valid Backup side to be installed.
    ///
    /// \return True instance has valid Backup, false otherwise.
    ///
    bool hasBackup() const {
      return this->_has_bck;
    }

    /// \brief Backup is a directory
    ///
    /// Check whether the Backup side of this instance is a directory.
    ///
    /// \return True Backup side is a directory, false otherwise.
    ///
    bool backupIsDir() const {
      return this->_bck_isdir;
    }

    /// \brief Backuo path
    ///
    /// Get Backuo file or directory path
    ///
    /// \return wide string
    ///
    const OmWString& backupPath() const {
      return this->_bck_path;
    }

    /// \brief Backup entry count
    ///
    /// Get count of parsed Backup entry
    ///
    /// \return wide string
    ///
    size_t backupEntryCount() const {
      return this->_bck_entry.size();
    }

    /// \brief Get Backup entry
    ///
    /// Returns Backup entry at specified index
    ///
    /// \param[in] i  : Entry index to get
    ///
    /// \return wide string
    ///
    const OmModEntry_t& getBackupEntry(size_t i) const {
      return this->_bck_entry[i];
    }

    /// \brief Test if Backup has directory
    ///
    /// Check whether the Backup side of this instance has an entry
    /// matching the specified parameters.
    ///
    /// \return True a match was found, false otherwise
    ///
    bool backupHasEntry(const OmWString& path, int32_t attr) const;

    /// \brief Mod hash value
    ///
    /// Mod filename hash value the backup data is related to
    ///
    /// \return 64 bit unsigned integer xxHahs value
    ///
    uint64_t hash() const {
      return this->_hash;
    }

    /// \brief Mod identity
    ///
    /// Mod identity string the backup data is related to
    ///
    /// \return Wide string
    ///
    const OmWString& iden() const {
      return this->_iden;
    }

    /// \brief Mod displayed name
    ///
    /// Mod displayed name string parsed from Mod identity.
    ///
    /// \return Wide string
    ///
    const OmWString& name() const {
      return this->_name;
    }

    /// \brief Mod core name
    ///
    /// Mod core name string parsed from Mod identity.
    ///
    /// \return Wide string
    ///
    const OmWString& core() const {
      return this->_core;
    }

    /// \brief Mod version
    ///
    /// Mod version parsed from Mod identity.
    ///
    /// \return Version (OmVersion) object
    ///
    const OmVersion& version() const {
      return this->_version;
    }

    /// \brief Mod category.
    ///
    /// Returns Mod category as defined by Mod author.
    ///
    /// \return Wide string.
    ///
    const OmWString& category() const {
      return this->_category;
    }

    /// \brief Set Mod category.
    ///
    /// Set or replace the Mod category.
    ///
    /// \param[in]  cate  : Category to set.
    ///
    void setCategory(const OmWString& cate) {
      this->_category = cate;
    }

    /// \brief Mod description.
    ///
    /// Returns Mod description as defined by Mod author.
    ///
    /// \return Wide string.
    ///
    const OmWString& description() const {
      return this->_description;
    }

    /// \brief Load directory Mod description.
    ///
    /// Loads or update description found beside the Mod directory
    /// according naming rules.
    ///
    /// \return True if description was found and updated, false otherwise.
    ///
    bool loadDirDescription();

    /// \brief Set Mod description.
    ///
    /// Set or replace the Mod description.
    ///
    /// \param[in]  desc  : Description text to set.
    ///
    void setDescription(const OmWString& desc) {
      this->_description = desc;
    }

    /// \brief Mod thumbnail image.
    ///
    /// Returns Mod thumbnail image as defined by Mod author.
    ///
    /// \return Image (OmImage) object.
    ///
    const OmImage& thumbnail() const {
      return this->_thumbnail;
    }

    /// \brief Load directory Mod thumbnail.
    ///
    /// Loads or update thumbnail found beside the Mod directory
    /// according naming rules.
    ///
    /// \return True if thumbnail was found and updated, false otherwise.
    ///
    bool loadDirThumbnail();

    /// \brief Set Mod Thumbnail
    ///
    /// Set or replace the Mod thumbnail.
    ///
    /// \param[in]  path  : Path to image to load.
    ///
    void setThumbnail(const OmWString& path);

    /// \brief Remove Mod Thumbnail
    ///
    /// Remove the Mod thumbnail inf any.
    ///
    void clearThumbnail();

    /// \brief Dependencies count
    ///
    /// Returns count of dependencies referenced by this Mod.
    ///
    /// \return Unsigned integer
    ///
    size_t dependCount() const {
      return this->_src_depend.size();
    }

    /// \brief Dependency Mod identity
    ///
    /// Returns dependency Mod identity at specified index.
    ///
    /// \param[in] i   : Reference index to get
    ///
    /// \return Wide string
    ///
    const OmWString& getDependIden(size_t i) const {
      return this->_src_depend[i];
    }

    /// \brief Check for dependency Mod identity
    ///
    /// Checks whether dependency Mod identity is referenced by this Mod.
    ///
    /// \param[in] iden  : Mod identity to test
    ///
    /// \return True if Mod identity is referenced, false otherwise
    ///
    bool hasDepend(const OmWString& iden) const;

    /// \brief Add dependency Mod identity
    ///
    /// Add dependency Mod identity reference for this Mod.
    ///
    /// \param[in] iden  : Mod identity to add
    ///
    void addDependIden(const OmWString& iden) {
      return this->_src_depend.push_back(iden);
    }

    /// \brief Clear dependency Mod list
    ///
    /// Remove all dependency Mod identity reference for this Mod.
    ///
    void clearDepend() {
      this->_src_depend.clear();
    }

    /// \brief Get install footprint.
    ///
    /// Simulate package installation and create entries list as if
    /// package were installed according current state.
    ///
    /// \param[out] footprint  : Pointer to Mod Entry vector to be filled
    ///
    void getFootprint(OmModEntryArray* footprint) const;

    /// \brief Overlapped Mod count
    ///
    /// Returns count of overlapped Mod referenced by this Backup data.
    ///
    /// When Mods are installed it occurs that some installed files overlaps ones
    /// already modified files by the installation of a previous Mod. The Mod already
    /// installed whom installed files was subsequently overwritten by another Mod
    /// are called "overlapped".
    ///
    /// \return Unsigned integer
    ///
    size_t overlapCount() const {
      return this->_bck_overlap.size();
    }

    /// \brief Overlapped Mod hash
    ///
    /// Returns overlapped Mod identity hash at specified index.
    ///
    /// When Mods are installed it occurs that some installed files overlaps ones
    /// already modified files by the installation of a previous Mod. The Mod already
    /// installed whom installed files was subsequently overwritten by another Mod
    /// are called "overlapped".
    ///
    /// \param[in]  i   : Reference index to get
    ///
    /// \return 64 bit unsigned integer xxHahs value
    ///
    size_t getOverlapHash(size_t i) const {
      return this->_bck_overlap[i];
    }

    /// \brief Check for overlapped Mod hash
    ///
    /// Checks whether overlapped Mod identity hash is referenced by
    /// Backup data.
    ///
    /// \param[in]  hash    : Mod identity hash to test
    ///
    /// \return True if Mod hash is referenced, false otherwise
    ///
    bool hasOverlap(uint64_t hash) const;

    /// \brief Test Mod overlapping
    ///
    /// Checks whether this instance share some files with another Mod meaning
    /// installation overlap and potential Backup corruption.
    ///
    /// \param[in]  other   : Other Mod Pack to test against.
    ///
    /// \return True if Mod overlap, false otherwise
    ///
    bool canOverlap(const OmModPack* other) const;

    /// \brief Test Mod overlapping
    ///
    /// Checks whether this instance share some files with Mod footprint meaning
    /// installation overlap and potential Backup corruption.
    ///
    /// \param[in]  footprint : Mod install footprint to test against.
    ///
    /// \return True if Mod overlap, false otherwise
    ///
    bool canOverlap(const OmModEntryArray& footprint) const;

    /// \brief Make Backup data
    ///
    /// Create Backup data according Mod Source files and current linked
    /// Mod Channel parameters.
    ///
    /// \param[in] progress_cb  : Optional progression callback function
    /// \param[in] user_ptr     : Optional user pointer to be passed to callback
    ///
    /// \return OM_RESULT_OK if operation succeed, OM_RESULT_ERROR if an error occurred
    ///         and OM_RESULT_ABORT if operation was aborted by callback or on invalid call.
    ///
    OmResult makeBackup(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Restore from Backup data
    ///
    /// Restore files from saved Backup data according current linked
    /// Mod Channel parameters.
    ///
    /// \param[in] progress_cb  : Optional progression callback function
    /// \param[in] user_ptr     : Optional user pointer to be passed to callback
    /// \param[in] isundo       : Specify that process an undo following install faillure
    ///
    /// \return OM_RESULT_OK if operation succeed, OM_RESULT_ERROR if an error occurred
    ///         and OM_RESULT_ABORT if invalid call
    ///
    OmResult restoreData(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr, bool isundo = false);

    /// \brief Install Mod
    ///
    /// Install files from Mod source according current linked
    /// Mod Channel parameters.
    ///
    /// \param[in] progress_cb  : Optional progression callback function
    /// \param[in] user_ptr     : Optional user pointer to be passed to callback
    ///
    /// \return OM_RESULT_OK if operation succeed, OM_RESULT_ERROR if an error occurred
    ///         and OM_RESULT_ABORT if operation was aborted by callback or on invalid call.
    ///
    OmResult applySource(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Discard Backup data
    ///
    /// Permanently delete Backup data to avoid having restoring it to prevent
    /// corrupted restoration on updated/replace Target data.
    ///
    /// \return OM_RESULT_OK if operation succeed, OM_RESULT_ERROR if an error occurred
    ///         and OM_RESULT_ABORT if invalid call
    ///
    OmResult discardBackup();

    /// \brief Save Mod Pack
    ///
    /// Save this instance source data with current properties as a Mod Pack archive file.
    ///
    /// \param[in] path         : Path and filename of Mod Pack archive file to create
    /// \param[in] method       : Compression method for archive file
    /// \param[in] level        : Compression level for archive file
    /// \param[in] progress_cb  : Optional progression callback function
    /// \param[in] user_ptr     : Optional user pointer to be passed to callback
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool saveAs(const OmWString& path, int32_t method = 93, int32_t level = 6, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Check for error
    ///
    /// Checks whether this instance has encountered error during last operation
    ///
    /// \return True if has error, false otherwise
    ///
    bool hasError() const {
      return this->_error_backup || this->_error_restore || this->_error_apply;
    }

    /// \brief Check for backup error
    ///
    /// Checks whether this instance has encountered error during last backup operation
    ///
    /// \return True if has backup error, false otherwise
    ///
    bool backupHasError() const {
      return this->_error_backup;
    }

    /// \brief Check for data restore error
    ///
    /// Checks whether this instance has encountered error during last data restore operation
    ///
    /// \return True if has data restore error, false otherwise
    ///
    bool restoreHasError() const {
      return this->_error_backup;
    }

    /// \brief Check for apply error
    ///
    /// Checks whether this instance has encountered error during last apply operation
    ///
    /// \return True if has apply error, false otherwise
    ///
    bool applyHasError() const {
      return this->_error_apply;
    }

    /// \brief Check whether in backup operation
    ///
    /// Checks whether this instance is currently processing backup operation
    ///
    /// \return True if processing backup operation, false otherwise
    ///
    bool isBackuping() const {
      return this->_op_backup;
    }

    /// \brief Check whether in data restore operation
    ///
    /// Checks whether this instance is currently processing data restore operation
    ///
    /// \return True if processing data restore operation, false otherwise
    ///
    bool isRestoring() const {
      return this->_op_restore;
    }

    /// \brief Check whether in apply operation
    ///
    /// Checks whether this instance is currently processing apply operation
    ///
    /// \return True if processing apply operation, false otherwise
    ///
    bool isApplying() const {
      return this->_op_apply;
    }

    /// \brief Operation progression
    ///
    /// Returns current running operation progression in percent
    ///
    /// \return Operation progression in percent
    ///
    uint32_t operationProgress() const {
      return this->_op_progress;
    }

    /// \brief Get Mod Channel
    ///
    /// Returns linked Mod Channel of this Mod Pack
    ///
    /// \return Pointer to Mod Channel or nullptr
    ///
    OmModChan* ModChan() const {
      return this->_ModChan;
    }

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const OmWString& lastError() const {
      return this->_lasterr;
    }

  private:

    // management
    void                _log(unsigned level, const OmWString& origin, const OmWString& detail);

    void                _error(const OmWString& origin, const OmWString& detail);

    OmWString           _lasterr;

    // linking
    OmModChan*          _ModChan;

    // common properties
    OmWString           _iden;

    uint64_t            _hash;

    OmWString           _core;

    OmWString           _name;

    OmVersion           _version;

    // optional properties
    OmWString           _category;

    OmWString           _description;

    time_t              _description_time;

    OmImage             _thumbnail;

    time_t              _thumbnail_time;

    // pack source properties
    bool                _has_src;

    time_t              _src_time;

    OmWString           _src_path;

    OmWString           _src_home;

    bool                _src_isdir;

    OmWString           _src_root;

    OmModEntryArray     _src_entry;

    OmWStringArray      _src_depend;

    // backup data properties
    bool                _has_bck;

    OmWString           _bck_path;

    bool                _bck_isdir;

    OmWString           _bck_root;

    OmModEntryArray     _bck_entry;

    OmUint64Array       _bck_overlap;

    // analytical properties
    bool                _error_backup;

    bool                _error_restore;

    bool                _error_apply;

    bool                _op_backup;

    bool                _op_restore;

    bool                _op_apply;

    uint32_t            _op_progress;
};

/// \brief OmModPack pointer array
///
/// Typedef for an STL vector of OmModPack pointer type
///
typedef std::vector<OmModPack*> OmPModPackArray;

/// \brief OmModPack pointer queue
///
/// Typedef for an STL deque of OmModPack pointer type
///
typedef std::deque<OmModPack*> OmPModPackQueue;

#endif // OMMODNODE_H
