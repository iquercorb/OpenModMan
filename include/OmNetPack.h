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
#ifndef OMNETPACK_H
#define OMNETPACK_H

#include "OmBase.h"
#include "OmModPack.h"

#include "OmImage.h"
#include "OmVersion.h"
#include "OmConnect.h"

class OmModChan;
class OmNetRepo;

enum OmPackStat : uint32_t {
  PACK_UKN = 0x00,
  PACK_NEW = 0x01,    ///< Remote package is new (not in local library)
  PACK_UPG = 0x02,    ///< Remote package is an upgrade (newer version)
  PACK_OLD = 0x04,    ///< Remote package is a downgrade (older version)
  PACK_DEP = 0x08,    ///< Remote package is downloaded but has missing dependencies
  PACK_DNL = 0x10,    ///< Remote package is downloading
  PACK_ERR = 0x20,    ///< Remote package download error
  PACK_WIP = 0x40,    ///< Remote package is work in progress
  PACK_PRT = 0x80,    ///< Remote package has partial download
};

class OmNetPack
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmNetPack();

    /// \brief Constructor.
    ///
    /// Constructor with parent Mod Channel
    ///
    /// \param[in] ModChan  : Parent Mod Channel
    ///
    OmNetPack(OmModChan* ModChan);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmNetPack();

    /// \brief Parse Repository Mod
    ///
    /// Try to parse Mod Repository reference node to be used as
    /// online (downloadable) Mod.
    ///
    /// \param[in]  ModRepo : Mod Repository to parse from.
    /// \param[in]  i       : Repository reference index to parse.
    ///
    /// \return True operation succeed, false otherwise
    ///
    bool parseReference(OmNetRepo* NetRepo, size_t i);

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

    /// \brief Mod description.
    ///
    /// Returns Mod description as defined by Mod author.
    ///
    /// \return Wide string.
    ///
    const OmWString& description() const {
      return this->_description;
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

    /// \brief Dependencies count
    ///
    /// Returns count of dependencies referenced by this Mod.
    ///
    /// \return Unsigned integer
    ///
    size_t dependCount() const {
      return this->_depend.size();
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
      return this->_depend[i];
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

    /// \brief Downloadable file name
    ///
    /// Downloadable Mod file name.
    ///
    /// \return Wide string
    ///
    const OmWString& fileName() const {
      return this->_file;
    }

    /// \brief Downloadable file size
    ///
    /// Downloadable Mod file size in byte
    ///
    /// \return 64-bit unsigned integer
    ///
    uint64_t fileSize() const {
      return this->_size;
    }

    /// \brief Mod File URL
    ///
    /// Mod File download URL
    ///
    /// \return Wide string
    ///
    const OmWString& fileUrl() const {
      return this->_url;
    }

    /// \brief Mod File Checksum
    ///
    /// Mod File Checksum string
    ///
    /// \return Wide string
    ///
    const OmWString& fileChecksum() const {
      return this->_csum;
    }

    /// \brief MD5 Checksum
    ///
    /// Returns whether checksum is an MD5 digest, otherwise it is xxHash.
    ///
    /// \return True if checksum is MD5
    ///
    bool checksumIsMd5() const {
      return this->_csum_is_md5;
    }

    /// \brief Get status
    ///
    /// Returns status bits that describes this instance situation
    /// in regard to the current Mod Library and its internal states
    ///
    /// \return Net Pack Status
    ///
    uint32_t status() const {
      return this->_stat;
    }

    /// \brief Check status bit
    ///
    /// Checks whether this instance has the specified status bits enabled
    ///
    /// \return True if status bits matches, false otherwise
    ///
    bool hasStatus(uint32_t stat) const {
      return OM_HAS_BIT(this->_stat, stat);
    }

    /// \brief Refresh Net Pack status
    ///
    /// Performs analysis of this instance against the Mod Library and
    /// current internal state to enable or disable proper Status bits.
    ///
    /// \return True if status changed, false otherwise
    ///
    bool refreshStatus();

    /// \brief Upgradable Mod count
    ///
    /// Returns count of Mod that are upgradable by this instance.
    ///
    /// \return Unsigned integer
    ///
    size_t upgradableCount() const {
      return this->_upgrade.size();
    }

    /// \brief Upgradable Mod
    ///
    /// Returns upgradable Mod at specified index.
    ///
    /// \param[in] index  : Upgradable Mod index to get
    ///
    /// \return Pointer to Mod Pack
    ///
    OmModPack* getUpgradable(size_t index) const {
      return this->_upgrade[index];
    }

    /// \brief Downgradable Mod count
    ///
    /// Returns count of Mod that are downgradable by this instance.
    ///
    /// \return Unsigned integer
    ///
    size_t downgradableCount() const {
      return this->_dngrade.size();
    }

    /// \brief Downgradable Mod
    ///
    /// Returns downgradable Mod at specified index.
    ///
    /// \param[in] index  : Downgradable Mod index to get
    ///
    /// \return Pointer to Mod Pack
    ///
    OmModPack* getDowngradable(size_t index) const {
      return this->_dngrade[index];
    }

    /// \brief Check for local version
    ///
    /// check whether this instance already has local version, therefor
    /// is already downloaded
    ///
    /// \return True if has local version, false otherwise.
    ///
    bool hasLocal() const {
      return this->_has_local;
    }

    /// \brief Check for error state
    ///
    /// Check whether this instance is in error stat following download or
    /// upgrading operation
    ///
    /// \return True if this instance has error, false otherwise
    ///
    bool hasError() const {
      return this->_has_error;
    }

    /// \brief Check for missing dependencies
    ///
    /// Check whether the local version of this instance
    /// have missing dependencies
    ///
    /// \return True if this instance has error, false otherwise
    ///
    bool hasMissingDepend() const {
      return this->_has_misg_dep;
    }

    /// \brief Checks whether is downloading
    ///
    /// Check whether this instance is currently downloading
    ///
    /// \return True if instance is downloading, false otherwise
    ///
    bool isDownloading() const;

    /// \brief Check resumable download
    ///
    /// Checks whether resumable partial download data exists
    ///
    /// \return True if resumable, false otherwise
    ///
    bool isResumable() const {
      return this->_has_part;
    }

    /// \brief Revoke resumable download
    ///
    /// Delete the existing partial download data to allow new
    /// download from beginning
    ///
    void revokeDownload();

    /// \brief Start download
    ///
    /// Stats download process for this Net Pack using parsed parameters
    ///
    /// \param[in] download_cb  : Callback for download progression.
    /// \param[in] result_cb    : Callback for download result
    /// \param[in] user_ptr     : User pointer to be passed to callback functions
    ///
    /// \return True if download successfully started, false otherwise
    ///
    bool startDownload(Om_downloadCb download_cb = nullptr, Om_resultCb result_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Download progress
    ///
    /// Get current download progression in percent
    ///
    /// \return Download progress in percent.
    ///
    uint32_t downloadProgress() const {
      return this->_dnl_percent;
    }

    /// \brief Download remain time
    ///
    /// Returns current download remaining time in seconds according
    /// current transfer rate.
    ///
    /// \return Remaining time in seconds
    ///
    uint32_t downloadRemain() const {
      return this->_dnl_remain;
    }

    /// \brief Stop download
    ///
    /// Stops the currently running download, the downloaded data is not
    /// deleted so download can be resumed later.
    ///
    void stopDownload();

    /// \brief Close download
    ///
    /// Finalize download process by checking download result and downloaded
    /// file integrity then rename temporary file to make it valid Mod.
    ///
    /// This function must be called after download process ended.
    ///
    /// \return True if operation succeed, false if error occurred.
    ///
    bool finalizeDownload();

    /// \brief Check whether is upgraded
    ///
    /// Checks whether this instance is currently in upgrading operation
    ///
    /// \return True if upgrading, false otherwise
    ///
    bool isUpgrading() const {
      return this->_is_upgrading;
    }

    /// \brief upgrade and replace
    ///
    /// Proceed to "Library Upgrading" by removing older version Mods
    /// this instance replaces. If older version Mods are currently installed, they
    /// are uninstalled, then the new one is reinstalled.
    ///
    /// This function must be called after successful download closing or will fail.
    ///
    /// \param[in] progress_cb  : Callback for install and uninstall progress
    /// \param[in] user_ptr     : Custom pointer to pass to callback
    ///
    /// \return True if operation succeed, false if error occurred.
    ///
    OmResult upgradeReplace(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Upgrade progress
    ///
    /// Get current upgrading job progression in percent
    ///
    /// \return Upgrading progress in percent.
    ///
    uint32_t upgradeProgress() const {
      return this->_upg_percent;
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

    /// \brief Get linked Mod Repository
    ///
    /// Returns linked Mod Repository of this Mod Pack
    ///
    /// \return Pointer to Mod Repository or nullptr
    ///
    OmNetRepo* NetRepo() const {
      return this->_NetRepo;
    }

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const OmWString& lastError() const {
      return _lasterr;
    }

  private:

    // management
    void                _log(unsigned level, const OmWString& origin, const OmWString& detail);

    void                _error(const OmWString& origin, const OmWString& detail);

    OmWString           _lasterr;

    // linking
    OmModChan*          _ModChan;

    OmNetRepo*          _NetRepo;

    // reference Mod properties
    OmWString           _iden;

    uint64_t            _hash;

    OmWString           _core;

    OmWString           _name;

    OmVersion           _version;

    OmWString           _category;

    OmWString           _description;

    OmImage             _thumbnail;

    OmWStringArray      _depend;

    // reference download properties
    OmWString           _file;

    uint64_t            _size;

    OmWString           _csum;

    bool                _csum_is_md5;

    OmWString           _url;

    // analytical properties
    OmPModPackArray     _upgrade;

    OmPModPackArray     _dngrade;

    uint32_t            _stat;

    bool                _has_part;

    bool                _has_local;

    bool                _has_error;

    bool                _has_misg_dep;

    bool                _is_upgrading;

    // client parameters
    void*               _cli_ptr;

    Om_resultCb         _cli_result_cb;

    Om_downloadCb       _cli_download_cb;

    Om_progressCb       _cli_progress_cb;

    // download stuff
    OmConnect           _connect;

    OmWString           _dnl_path;

    OmWString           _dnl_temp;

    OmResult            _dnl_result;

    uint32_t            _dnl_remain;

    uint32_t            _dnl_percent;

    static void         _dnl_result_fn(void*, OmResult, uint64_t);

    static bool         _dnl_download_fn(void*, int64_t, int64_t, int64_t, uint64_t);

    // upgrade-replace stuff
    uint32_t            _upg_percent;

    static bool         _upg_progress_fn(void*, size_t, size_t, uint64_t);
};

/// \brief OmNetPack pointer array
///
/// Typedef for an STL vector of OmNetPack pointer type
///
typedef std::vector<OmNetPack*> OmPNetPackArray;


/// \brief OmNetPack pointer queue
///
/// Typedef for an STL deque of OmNetPack pointer type
///
typedef std::deque<OmNetPack*> OmPNetPackQueue;


#endif // OMNETPACK_H
