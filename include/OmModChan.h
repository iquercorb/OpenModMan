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
#ifndef OMMODCHAN_H
#define OMMODCHAN_H

#include "OmBase.h"

#include "OmUtilFs.h"           //< OM_ACCESS_*

#include "OmXmlConf.h"
#include "OmConnect.h"
#include "OmModPack.h"
#include "OmNetPack.h"
#include "OmNetRepo.h"

class OmModHub;

/// \brief Mod Channel object for Mod Hub.
///
/// The Mod Channel object defines environment for package installation
/// destination, library and backup. The object provide interface to
/// store, manage, install and uninstall packages.
///
class OmModChan
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmModChan(OmModHub* pModHub);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmModChan();

    /// \brief Close Mod Channel.
    ///
    /// Close and empty the current instance.
    ///
    void close();

    /// \brief Open Mod Channel.
    ///
    /// Load Mod Channel from specified file.
    ///
    /// \param[in]  path    : File path of Mod Channel to be loaded.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool open(const OmWString& path);

    /// \brief Rename Mod Channel
    ///
    /// Change Mod Channel title and rename its home directory to reflect
    /// new title.
    ///
    /// \param[in] name   : New name to set.
    ///
    /// \return True if operation succeed, false if error occurred.
    ///
    OmResult rename(const OmWString& name);

    /// \brief Verify Target access.
    ///
    /// Checks whether the software has access to Target directory.
    ///
    /// \param[in] mask  : Bit mask for access type to check
    ///
    /// \return True if checked access allowed, false otherwise.
    ///
    bool accessesTarget(uint32_t mask = OM_ACCESS_DIR_READ|OM_ACCESS_DIR_WRITE);

    /// \brief Verify Library folder access.
    ///
    /// Checks whether the software has access to Library directory.
    ///
    /// \param[in]  mask  : Bit mask for access type to check
    ///
    /// \return True if checked access allowed, false otherwise.
    ///
    bool accessesLibrary(uint32_t mask = OM_ACCESS_DIR_READ);

    /// \brief Verify Backup folder access.
    ///
    /// Checks whether the software has access to Backup folder for
    /// reading or reading & writing.
    ///
    /// \param[in]  mask  : Bit mask for access type to check
    ///
    /// \return True if checked access allowed, false otherwise.
    ///
    bool accessesBackup(uint32_t mask = OM_ACCESS_DIR_READ|OM_ACCESS_DIR_WRITE);

    /// \brief Refresh Libraries
    ///
    /// Refresh both local Mod library then network Mod library
    ///
    bool refreshLibraries();

    /// \brief Clear Mod Library
    ///
    /// Clear the local Mod Library list.
    ///
    bool clearModLibrary();

    /// \brief Refresh Mod Library
    ///
    /// Refresh the local Mod Library list.
    ///
    bool refreshModLibrary();

    /// \brief Who you gonna call ?
    ///
    /// Check Local Mod Library for "ghost" Mod instances (that has no
    /// Source nor Backup) and removes them.
    ///
    bool ghostbusterModLibrary();

    /// \brief Library Mod Count
    ///
    /// Return count of Mod Pack in local Library
    ///
    size_t modpackCount() const {
      return this->_modpack_list.size();
    }

    /// \brief Get Mod Pack
    ///
    /// Get local Library Mod Pack object at specified index
    ///
    /// \param[in] index  : Index of Mod Pack to get.
    ///
    /// \return Pointer to Mod Pack object.
    ///
    OmModPack* getModpack(size_t index) const {
      return this->_modpack_list[index];
    }

    /// \brief Find Mod in Library
    ///
    /// Search for a Mod in Local Mod Library that matches
    /// the specified Hash value.
    ///
    /// \param[in] hash   : Mod hash value to search
    /// \param[in] nodir  : Ignores Directory (dev mode) Mods
    ///
    /// \return Pointer to Mod Pack object, or null if not found
    ///
    OmModPack* findModpack(uint64_t hash, bool nodir = false) const;

    /// \brief Find Mod in Library
    ///
    /// Search for a Mod in Local Mod Library that matches
    /// the specified Identity string.
    ///
    /// \param[in] iden   : Mod Identity string to search
    /// \param[in] nodir  : Ignores Directory (dev mode) Mods
    ///
    /// \return Pointer to Mod Pack object, or null if not found
    ///
    OmModPack* findModpack(const OmWString& iden, bool nodir = false) const;

    /// \brief Get Mod index
    ///
    /// Retrieve index of the given Mod in the Local Mod Library list
    ///
    /// \param[in] ModPack : Point to Mod Pack object to search
    ///
    /// \return Index of Mods in list or -1 if not found
    ///
    int32_t indexOfModpack(const OmModPack* ModPack) const;

    /// \brief Get Mod index
    ///
    /// Retrieve index of the given Mod in the Local Mod Library list
    ///
    /// \param[in] hash   : ModPack hash value to search
    ///
    /// \return Index of Mods in list or -1 if not found
    ///
    int32_t indexOfModpack(uint64_t hash) const;

    /// \brief Check Mod overlap
    ///
    /// Check whether the specified Mod is overlapped by other
    ///
    /// \param[in] i    : Mod index in Local Library to check
    ///
    /// \return True if Mod is overlapped by any other, false otherwise
    ///
    bool isOverlapped(size_t i) const;

    /// \brief Check Mod overlap
    ///
    /// Check whether the specified Mod is overlapped by other
    ///
    /// \param[in] ModPack : Mod index in Local Library to check
    ///
    /// \return True if Mod is overlapped by others, false otherwise
    ///
    bool isOverlapped(const OmModPack* ModPack) const;

    /// \brief Find overlapped Mods
    ///
    /// Get list of current library's Mods which may be overlapped by the
    /// specified Mod if it is installed.
    ///
    /// \param[in] ModPack  : Mod to test overlapping over library's Mods
    /// \param[in] overlaps : Pointer to array to be filled with hash values
    ///
    void findOverlaps(const OmModPack* ModPack, OmUint64Array* overlaps) const;

    /// \brief Find overlapped Mods
    ///
    /// Get list of current library's Mods which may be overlapped by the
    /// specified Mod if it is installed.
    ///
    /// \param[in] ModPack  : Mod to test overlapping over library's Mods
    /// \param[in] overlaps : Pointer to array to be filled with ModPacks objects
    ///
    void findOverlaps(const OmModPack* ModPack, OmPModPackArray* overlaps) const;

    /// \brief Get overlapped Mods
    ///
    /// Get list of found overlapped Mods as referenced by the specified
    /// Mod backup definition
    ///
    /// \param[in] ModPack  : Mod to get backup overlapped list
    /// \param[in] overlaps : Pointer to array to be filled with ModPacks objects
    ///
    void getOverlaps(const OmModPack* ModPack, OmPModPackArray* overlaps) const;

    /// \brief Check whether backup entry exists
    ///
    /// Check whether any currently installed Mod have an entry that matches the specified
    /// parameters in their backup entries list, meaning this entry was already created or
    /// modified by another Mod.
    ///
    /// \param[in] path   : Entry path to test check for.
    /// \param[in] attr   : Entry associated attributes bits to check for.
    ///
    /// \return True if matching entry was found, false otherwise.
    ///
    bool backupEntryExists(const OmWString& path, int32_t attr) const;

    /// \brief Check whether is dependency
    ///
    /// Check whether the specified Mod is a dependency of any other in the current Library
    ///
    /// \param[in] ModPack  : Mod to check dependency.
    ///
    /// \return True if Mod is a dependency, false otherwise.
    ///
    bool isDependency(const OmModPack* ModPack) const;

    /// \brief Check for missing dependency
    ///
    /// Check whether the specified Mod has at least one missing dependency.
    ///
    /// \param[in] ModPack  : Mod to check dependencies.
    ///
    /// \return True if at least one dependency is missing, false otherwise.
    ///
    bool hasMissingDepend(const OmModPack* ModPack) const;

    /// \brief Prepare Mods installation
    ///
    /// Performs Mods installation preparation for the given Mods selection. The function
    /// recursively check dependencies and overlapping over the current library's Mods then fill
    /// the given arrays with relevant data related to potential installations.
    ///
    /// \param[in]  selection     : List of desired Mods for installation.
    /// \param[out] installs      : Final list of Mods that should be installed, including dependencies
    /// \param[out] overlaps      : Identity list of overlapped Mods
    /// \param[out] depends       : Identity list of the supplementary installed dependencies Mods
    /// \param[out] missings      : Identity list of missing dependencies Mods
    ///
    void prepareInstalls(const OmPModPackArray& selection, OmPModPackArray* installs, OmWStringArray* overlaps, OmWStringArray* depends, OmWStringArray* missings) const;

    /// \brief Prepare Mods backup restoration
    ///
    /// Performs Mods backup restoration preparation for the given Mods selection. The function
    /// recursively check dependencies and overlapping over the current library's Mods then fill
    /// the given arrays with relevant data related to potential backup restoration.
    ///
    /// \param[in]  selection     : List of desired Mods for backup restoration.
    /// \param[out] restores      : Final list of Mods that should be restored, including dependencies
    /// \param[out] overlappers   : Identity list of the supplementary restored overlapping Mods
    /// \param[out] dependents    : Identity list of the supplementary restored dependent Mods
    ///
    void prepareRestores(const OmPModPackArray& selection, OmPModPackArray* restores, OmWStringArray* overlappers, OmWStringArray* dependents) const;

    /// \brief Prepare Mods cleaning
    ///
    /// Performs Mods cleaning preparation for the given Mods selection. The function
    /// recursively check dependencies and overlapping over the current library's Mods then fill
    /// the given arrays with relevant data related to potential cleaning.
    ///
    /// The Mod cleaning consist of standard backup restoration but also including the
    /// no longed used dependency nodes.
    ///
    /// \param[in]  selection     : List of desired Mods for backup restoration.
    /// \param[out] restores      : Final list of Mods that should be restored, including dependencies
    /// \param[out] depends       : Identity list of the supplementary restored unused dependency Mods
    /// \param[out] overlappers   : Identity list of the supplementary restored overlapping Mods
    /// \param[out] dependents    : Identity list of the supplementary restored dependent Mods
    ///
    void prepareCleaning(const OmPModPackArray& selection, OmPModPackArray* restores, OmWStringArray* depends, OmWStringArray* overlappers, OmWStringArray* dependents) const;

    /// \brief Add Mods to Mod-Operations queue
    ///
    /// Add to Mod-Operations queue the given Mods.
    /// The Mod-Operations consists of installing or uninstalling (backup restoration) Mods
    /// to the Channel's Target directory. Mods in the queue are treated sequentially, those
    /// with backup data are uninstalled, other are installed.
    ///
    /// This function act asynchronously and returns immediately.
    ///
    /// Within this context, the \c param parameter of each callback functions is a pointer to
    /// the currently processing Mod Pack object.
    ///
    /// \param[in] selection     : List of Mods to add to
    /// \param[in] begin_cb      : Callback function to be called each operation begin.
    /// \param[in] progress_cb   : Callback function to be called during operation progression.
    /// \param[in] result_cb     : Callback function to be called each operation end.
    /// \param[in] user_ptr      : Custom pointer to be passed to callback functions.
    ///
    void queueModOps(const OmPModPackArray& selection, Om_beginCb begin_cb = nullptr, Om_progressCb progress_cb = nullptr, Om_resultCb result_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Execute Mod-Operations
    ///
    /// Execute the Mod-Operations for the given Mods.
    /// The Mod-Operations consists of installing or uninstalling (backup restoration) Mods
    /// to the Channel's Target directory. Mods in the queue are treated sequentially, those
    /// with backup data are uninstalled, other are installed.
    ///
    /// This function act synchronously and is blocking until all operations ended.
    ///
    /// Within this context, the \c param parameter of each callback functions is a pointer to
    /// the currently processing Mod Pack object.
    ///
    /// \param[in] selection     : List of Mods to add to
    /// \param[in] begin_cb      : Callback function to be called each operation begin.
    /// \param[in] progress_cb   : Callback function to be called during operation progression.
    /// \param[in] result_cb     : Callback function to be called each operation end.
    /// \param[in] user_ptr      : Custom pointer to be passed to callback functions.
    ///
    /// \return Result code of the operations:
    ///         OM_RESULT_OK is returned of all operations succeeds,
    ///         OM_RESULT_ERROR is returned if an error occurred at some point,
    ///         OM_RESULT_ABORT is returned if operations aborted by callback or in case of illegal call.
    ///
    OmResult execModOps(const OmPModPackArray& selection, Om_beginCb begin_cb = nullptr, Om_progressCb progress_cb = nullptr, Om_resultCb result_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Abort Mod-Operations
    ///
    /// Abort the currently processing Mod-Operations and flush queue
    ///
    void abortModOps();

    /// \brief Mod-Operations progression
    ///
    /// Returns the currently processing Mod-Operations queue global progression in percent.
    ///
    /// \return Mod-Operations queue progression in percent
    ///
    uint32_t modOpsProgress() const {
      return this->_modops_percent;
    }

    /// \brief Mod-Operations queue size
    ///
    /// Returns current count of Mods in the Mod-Operations queue
    ///
    /// \return Mod-Operations queue size
    ///
    size_t modOpsQueueSize() const {
      return this->_modops_queue.size();
    }

    /// \brief Check for locked Mod Library
    ///
    /// Returns locked state of Mod Library. The Mod Library is locked in case of
    /// processing Mod-Operations and while this, some other operations are forbidden
    /// to prevent undefined behaviors.
    ///
    /// \return True if Mod library is locked, false otherwise
    ///
    bool lockedModLibrary() const {
      return this->_locked_mod_library;
    }

    /// \brief Discard all backup data
    ///
    /// Discard all backup data for this channel.
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool discardBackups(const OmPModPackArray& selection, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Sort local Mod Library
    ///
    /// Sort local Mod Library using the current sorting mode
    ///
    void sortModLibrary();

    /// \brief Set Mod Library sorting.
    ///
    /// Sets the Mod Library list sorting attribute and parameters.
    ///
    /// \param[in]  sorting : Attribute to sort Mod library.
    ///
    void setModLibrarySort(OmSort sorting);

    /// \brief Check has backup data
    ///
    /// Checks whether this instance currently has one or more backup data
    /// to be restored.
    ///
    /// \return True if has backup data, false otherwise.
    ///
    bool hasBackupData();

    /// \brief Purge backup data.
    ///
    /// Uninstall all installed packages to restore and delete all backup data.
    ///
    /// \param[in]  progress_cb : Optional progression callback function.
    /// \param[in]  user_ptr    : Optional pointer to user data passed to progression callback.
    ///
    /// \return True if operation succeed, false if error occurred.
    ///
    bool purgeBackupData(Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Clear Metwork Library
    ///
    /// Clear the Network Mod Library list.
    ///
    void clearNetLibrary();

    /// \brief Refresh Network Library
    ///
    /// Refresh the Network Mod Library list.
    ///
    bool refreshNetLibrary();

    /// \brief Get Net Pack count
    ///
    /// Returns network Library Net Pack count.
    ///
    /// \return Count of Net Pack in network Library
    ///
    size_t netpackCount() const {
      return this->_netpack_list.size();
    }

    /// \brief Get Net Pack
    ///
    /// Get local network Library Net Pack object at specified index
    ///
    /// \param[in] index  : Index of Net Pack to get.
    ///
    /// \return Pointer to Net Pack object.
    ///
    OmNetPack* getNetpack(size_t index) const {
      return this->_netpack_list[index];
    }

    /// \brief Find Net Pack
    ///
    /// Search in the network Library for Net Pack object with the specified hash value.
    ///
    /// \param[in] hash   : Hash value to search.
    ///
    /// \return Pointer to Net Pack object or nullptr if not found.
    ///
    OmNetPack* findNetpack(uint64_t hash) const;

    /// \brief Find Net Pack
    ///
    /// Search in the network Library for Net Pack object with the specified identity.
    ///
    /// \param[in] iden   : Identity string to search.
    ///
    /// \return Pointer to Net Pack object or nullptr if not found.
    ///
    OmNetPack* findNetpack(const OmWString& iden) const;

    /// \brief Get Network Mod index
    ///
    /// Retrieve index of the given Network Mod in the Network Mod Library list
    ///
    /// \param[in] NetPack : Pointer to Net Pack object to search
    ///
    /// \return Index of Network Mods in list or -1 if not found
    ///
    int32_t indexOfNetpack(const OmNetPack* NetPack) const;

    /// \brief Get Network Pack Dependencies
    ///
    /// Retrieve list of dependencies of the specified Net Pack among those available
    /// in the network Library.
    ///
    /// \param[in]  NetPack   : Pointer to Net Pack object to retrieve dependencies.
    /// \param[out] depends   : Array to be filled with found dependencies.
    /// \param[out] missings  : Array to be filled with missing dependencies identity string.
    ///
    void getDepends(const OmNetPack* NetPack, OmPNetPackArray* depends, OmWStringArray* missings) const;

    /// \brief Prepare Mods downloads
    ///
    /// Performs Mods downloads preparation for the given Mods selection. The function
    /// recursively check dependencies over the current local and network library then fill the
    /// given arrays with relevant data related to potential downloads.
    ///
    /// \param[in]  selection     : List of desired Mods for installation.
    /// \param[out] downloads     : Final list of Mods that should be downloaded, including dependencies
    /// \param[out] depends       : Identity list of the supplementary downloaded dependencies
    /// \param[out] missings      : Identity list of missing dependencies Mods
    /// \param[out] breaking      : Identity list of dependency-breakable dependents Mods
    ///
    void prepareDownloads(const OmPNetPackArray& selection, OmPNetPackArray* downloads, OmWStringArray* depends, OmWStringArray* missings, OmWStringArray* breaking) const;

    /// \brief Start Mods download
    ///
    /// The the download of Mods in the given selection list.
    ///
    /// This function act asynchronously and returns immediately.
    ///
    /// Within this context, the \c param parameter of each callback functions is a pointer to
    /// the currently processing Net Pack object.
    ///
    /// \param[in] selection     : List of Mods to add to
    /// \param[in] progress_cb   : Callback function to be called during download progression.
    /// \param[in] result_cb     : Callback function to be called each download end.
    /// \param[in] user_ptr      : Custom pointer to be passed to callback functions.
    ///
    void startDownloads(const OmPNetPackArray& selection, Om_downloadCb download_cb = nullptr, Om_resultCb result_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Stop all downloads
    ///
    /// Stops all the currently processing downloads.
    ///
    void stopDownloads();

    /// \brief Stop Mod download
    ///
    /// Stops download of the specified Net Pack at index
    ///
    /// \param[in] index  : Index of Net Pack in network Library.
    ///
    void stopDownload(size_t index);

    /// \brief Downloads progression
    ///
    /// Returns the current cumulative downloads progression in percent.
    ///
    /// \return Cumulative downloads progression in percent.
    ///
    uint32_t downloadsProgress() const {
      return this->_download_percent;
    }

    /// \brief Add Mods to Upgrades queue
    ///
    /// Add to Upgrades queue the given Mods.
    ///
    /// The Upgrades consists of uninstalling (backup restoration) of old-version Mod before
    /// deleting it (moving to trash), leaving only the new version in the local Library.
    ///
    /// This function act asynchronously and returns immediately.
    ///
    /// Within this context, the \c param parameter of each callback functions is a pointer to
    /// the currently processing Mod Pack object.
    ///
    /// \param[in] selection     : List of Mods to add to
    /// \param[in] begin_cb      : Callback function to be called each operation begin.
    /// \param[in] progress_cb   : Callback function to be called during operation progression.
    /// \param[in] result_cb     : Callback function to be called each operation end.
    /// \param[in] user_ptr      : Custom pointer to be passed to callback functions.
    ///
    void queueUpgrades(const OmPNetPackArray& selection, Om_beginCb begin_cb = nullptr, Om_progressCb progress_cb = nullptr, Om_resultCb result_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Stop upgrades
    ///
    /// Abort processing upgrade and flush queue.
    ///
    void abortUpgrades();

    /// \brief Upgrades progression
    ///
    /// Returns the current cumulative upgrades progression in percent.
    ///
    /// \return Cumulative upgrades progression in percent.
    ///
    uint32_t upgradesProgress() const {
      return this->_upgrade_percent;
    }

    /// \brief Sort network Mod Library
    ///
    /// Sort network Mod Library using the current sorting mode
    ///
    void sortNetLibrary();

    /// \brief Set Net Library sorting.
    ///
    /// Sets the Net Library list sorting attribute and parameters.
    ///
    /// \param[in]  sorting : Attribute to sort Net library.
    ///
    void setNetLibrarySort(OmSort sorting);

    /// \brief Add Network Repository
    ///
    /// Add new Network Repository parameters in Channel.
    ///
    /// \return True if operation succeed, false in case of invalid parameters
    ///
    bool addRepository(const OmWString& base, const OmWString& name);

    /// \brief Remove Network Repository
    ///
    /// Delete the Network Repository at given index.
    ///
    /// \param[in] index  : Index of Network Repository in list.
    ///
    void removeRepository(size_t index);

    /// \brief Get Net Repository count
    ///
    /// Returns count of configured Net Repository for this Mod Channel
    ///
    /// \return Count of Net Repository
    ///
    size_t repositoryCount() const {
      return this->_repository_list.size();
    }

    /// \brief Get Net Repository.
    ///
    /// Returns Net Repository repository object at index.
    ///
    /// \param[in] i    : Index to get Net Repository
    ///
    /// \return Net Repository object
    ///
    OmNetRepo* getRepository(size_t i) const {
      return this->_repository_list[i];
    }

    /// \brief Get Repository index.
    ///
    /// Retrieve Repository index in repositories list.
    ///
    /// \param[in] NetRepo  : Net Repository object to search.
    ///
    /// \return Index int list or -1 if not found.
    ///
    int32_t indexOfRepository(OmNetRepo* NetRepo) const;

    /// \brief Add Net Repositories to query queue
    ///
    /// Adds the given selection of Network Repositories to the Query queue. The
    /// repositories will be queried sequentially and according their position in queue.
    ///
    /// Within this context, the \c param parameter of each callback functions is a pointer to
    /// the currently processing Net Repo object.
    ///
    /// \param[in] selection  : Selected Repositories list to query
    /// \param[in] begin_cb   : Callback function called for processing begin
    /// \param[in] result_cb  : Callback function called for processing result
    /// \param[in] user_ptr   : User pointer to pass to result callback
    ///
    /// \return True if request was sent, false if query already processing.
    ///
    void queueQueries(const OmPNetRepoArray& selection, Om_beginCb begin_cb = nullptr, Om_resultCb result_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Abort Net Repository query.
    ///
    /// Abort all processing Net repository query
    ///
    void abortQueries();

    /// \brief Respository queries progression
    ///
    /// Returns repository queries queue progression in percent
    ///
    /// \return Queue progression in percent
    ///
    uint32_t queriesProgress() const {
      return this->_query_percent;
    }

    /// \brief Check whether is valid.
    ///
    /// Checks whether this instance is correctly loaded a ready to use.
    ///
    /// \return True if this instance is valid, false otherwise.
    ///
    bool valid() const {
      return this->_xmlconf.valid();
    }

    /// \brief Get Mod Channel UUID.
    ///
    /// Returns Mod Channel UUID.
    ///
    /// \return Mod Channel UUID.
    ///
    const OmWString& uuid() const {
      return this->_uuid;
    }

    /// \brief Get Mod Channel title.
    ///
    /// Returns Mod Channel title.
    ///
    /// \return Mod Channel title.
    ///
    const OmWString& title() const {
      return this->_title;
    }

    /// \brief Set Mod Channel title.
    ///
    /// Defines and save Mod Channel title.
    ///
    /// \param[in]  title   : Title to defines and save
    ///
    void setTitle(const OmWString& title);

    /// \brief Get Mod Channel index.
    ///
    /// Returns Mod Channel ordering index.
    ///
    /// \return Ordering index number.
    ///
    unsigned index() const {
      return this->_index;
    }

    /// \brief Set Mod Channel index.
    ///
    /// Defines and save Mod Channel index for ordering.
    ///
    /// \param[in]  index   : Index number to defines and save
    ///
    void setIndex(unsigned index);

    /// \brief Get Mod Channel home directory.
    ///
    /// Returns home directory.
    ///
    /// \return Mod Channel home directory.
    ///
    const OmWString& home() const {
      return this->_home;
    }

    /// \brief Get Mod Channel definition path.
    ///
    /// Returns Mod Channel definition file path.
    ///
    /// \return Mod Channel definition file path.
    ///
    const OmWString& path() const {
      return this->_path;
    }

    /// \brief Get Target path.
    ///
    /// Returns Mod Channel packages installation destination directory.
    ///
    /// \return Mod Channel Target directory.
    ///
    const OmWString& targetPath() const {
      return this->_target_path;
    }

    /// \brief Set Mod Channel destination path.
    ///
    /// Defines and save Mod Channel installation destination path.
    ///
    /// \param[in] path    : Target directory path to set.
    ///
    /// \return Operation result code:
    ///         OM_RESULT_OK if operation succeed,
    ///         OM_RESULT_ABORT in case of illegal call,
    ///         OM_RESULT_ERROR if an error occured during backup data transfer.
    ///
    OmResult setTargetPath(const OmWString& path);

    /// \brief Get Mod Channel library path.
    ///
    /// Returns Mod Channel packages library directory.
    ///
    /// \return Packages library directory.
    ///
    const OmWString& libraryPath() const {
      return this->_library_path;
    }

    /// \brief Set custom Library directory.
    ///
    /// Set a custom Mod Library directory.
    ///
    /// \param[in] path    : Custom Library directory path to set.
    ///
    /// \return Operation result code:
    ///         OM_RESULT_OK if operation succeed,
    ///         OM_RESULT_ABORT in case of illegal call,
    ///         OM_RESULT_ERROR if an error occured during backup data transfer.
    ///
    OmResult setCustLibraryPath(const OmWString& path);

    /// \brief Has custom Library directory.
    ///
    /// Checks whether this instance currently has and use a custom
    /// Library folder path.
    ///
    /// \return True if a custom Library path is used, false otherwise.
    ///
    bool hasCustLibraryPath() const {
      return this->_cust_library_path;
    }

    /// \brief Set default Library directory.
    ///
    /// Remove the current custom Library configuration and reset to
    /// default setting.
    ///
    /// \return Operation result code:
    ///         OM_RESULT_OK if operation succeed,
    ///         OM_RESULT_ABORT in case of illegal call,
    ///         OM_RESULT_ERROR if an error occured during backup data transfer.
    ///
    OmResult setDefLibraryPath();

    /// \brief Get Mod Channel backup path.
    ///
    /// Returns Mod Channel packages installation backup directory.
    ///
    /// \return Backup directory.
    ///
    const OmWString& backupPath() const {
      return this->_backup_path;
    }

    /// \brief Has custom Backup directory
    ///
    /// Checks whether this instance currently use a custom
    /// Backup directory.
    ///
    /// \return True if Backup directory is custom, false otherwise.
    ///
    bool hasCustBackupPath() const {
      return this->_cust_backup_path;
    }

    /// \brief Set custom Backup directory
    ///
    /// Set a custum Backup data directory. If Backup data
    /// directory is not empty, all data is moved to the new location.
    ///
    /// \param[in]  path    : Custom Backup directory path to set.
    ///
    /// \return Operation result code:
    ///         OM_RESULT_OK if operation succeed,
    ///         OM_RESULT_ABORT in case of illegal call,
    ///         OM_RESULT_ERROR if an error occured during backup data transfer.
    ///
    OmResult setCustBackupPath(const OmWString& path);

    /// \brief Set default Backup directory
    ///
    /// Remove the current custom Backup configuration and reset to
    /// default setting. If Backup data directory is not empty, all
    /// data is moved to the new location.
    ///
    /// \return Operation result code:
    ///         OM_RESULT_OK if operation succeed,
    ///         OM_RESULT_ABORT in case of illegal call,
    ///         OM_RESULT_ERROR if an error occured during backup data transfer.
    ///
    OmResult setDefBackupPath();

    /// \brief Get backup compression method.
    ///
    /// Returns defined backup compression method.
    ///
    /// \return Zip compression method.
    ///
    int backupCompMethod() const {
      return _backup_comp_method;
    }

    /// \brief Get backup compression method.
    ///
    /// Returns defined backup compression method.
    ///
    /// \return Zip compression method.
    ///
    int backupCompLevel() const {
      return _backup_comp_level;
    }

    /// \brief Set Backup compression level.
    ///
    /// Defines and save backup compression level. A negative value
    /// means create directory backup instead of zipped backup.
    ///
    /// \param[in]  method  : Compression method
    /// \param[in]  level   : Compression level
    ///
    void setBackupComp(int32_t method, int32_t level);

    /// \brief Get package legacy support size option.
    ///
    /// Returns package legacy support option value.
    ///
    /// \return Package legacy support option value.
    ///
    bool libraryDevmod() const {
      return _library_devmode;
    }

    /// \brief Set package legacy support option.
    ///
    /// Define and save package legacy support option value.
    ///
    /// \param[in]  enable    : Package legacy support enable or disable.
    ///
    void setLibraryDevmod(bool enable);

    /// \brief Show hidden files and folders option.
    ///
    /// Returns show hidden option value option value.
    ///
    /// \return Show hidden option value.
    ///
    bool libraryShowhidden() const {
      return _library_showhidden;
    }

    /// \brief Set show hidden files and folders option.
    ///
    /// Define show hidden files and folders option value.
    ///
    /// \param[in]  enable    : Show hidden enable or disable.
    ///
    void setLibraryShowhidden(bool enable);

    /// \brief Get warning for overlaps option.
    ///
    /// Returns warning for overlaps option value.
    ///
    /// \return Warning for overlaps option value.
    ///
    bool warnOverlaps() const {
      return _warn_overlaps;
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
      return _warn_extra_inst;
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
      return _warn_miss_deps;
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
      return _warn_extra_unin;
    }

    /// \brief Set warning for extra uninstall option.
    ///
    /// Define and save warning for extra uninstall option value.
    ///
    /// \param[in]  enable    : Warning for extra uninstall enable or disable.
    ///
    void setWarnExtraUnin(bool enable);

    /// \brief Get upgrade rename mode.
    ///
    /// Returns defined upgrade by rename mode for remote packages.
    ///
    /// \return Upgrade rename mode enabled.
    ///
    bool upgdRename() const {
      return _upgd_rename;
    }

    /// \brief Set upgrade rename mode.
    ///
    /// Defines and save upgrade rename mode for remote packages.
    ///
    /// \param[in]  enable   : Enable or disable upgrade rename mode.
    ///
    void setUpgdRename(bool enable);

    /// \brief Get warning for extra download option.
    ///
    /// Returns warning for extra download option value.
    ///
    /// \return Warning for extra download option value.
    ///
    bool warnExtraDnld() const {
      return _warn_extra_dnld;
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
      return _warn_miss_dnld;
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
      return _warn_upgd_brk_deps;
    }

    /// \brief Set warning for upgrade breaking dependencies.
    ///
    /// Define and save warning for upgrade breaking dependencies option value.
    ///
    /// \param[in]  enable    : Warning for upgrade breaking dependencies enable or disable.
    ///
    void setWarnUpgdBrkDeps(bool enable);

    /// \brief Get Mod Hub
    ///
    /// Return affiliated Mod Hub.
    ///
    OmModHub* ModHub() const {
      return this->_Modhub;
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

    /// \brief Escalate log.
    ///
    /// Public function to allow "children" item to escalate log
    ///
    void escalateLog(unsigned level, const OmWString& origin, const OmWString& detail) const {
      this->_log(level, origin, detail);
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // logs and errors
    void                  _log(unsigned level, const OmWString& origin, const OmWString& detail) const;

    void                  _error(const OmWString& origin, const OmWString& detail);

    OmWString             _lasterr;

    // related objects
    OmModHub*             _Modhub;

    // related XML config
    OmXmlConf             _xmlconf;

    // mod operations helpers
    void                  _get_modops_depends(const OmModPack*, OmPModPackArray*, OmWStringArray*) const;

    void                  _get_backup_relations(const OmModPack*, OmPModPackArray*, OmWStringArray*, OmWStringArray*) const;

    void                  _get_cleaning_depends(const OmModPack*, const OmPModPackArray&, OmPModPackArray*) const;

    void                  _get_cleaning_relations(const OmModPack*, const OmPModPackArray&, OmPModPackArray*, OmWStringArray*) const;

    void                  _get_missing_depends(const OmModPack*, OmWStringArray*) const;

    void                  _get_source_downloads(const OmModPack*, OmPNetPackArray*, OmWStringArray*) const;

    void                  _get_replace_breaking(const OmNetPack*, OmWStringArray*) const;

    // sorting comparison functions
    static bool           _compare_mod_name(const OmModPack* a, const OmModPack* b);
    static bool           _compare_mod_stat(const OmModPack* a, const OmModPack* b);
    static bool           _compare_mod_vers(const OmModPack* a, const OmModPack* b);
    static bool           _compare_mod_cate(const OmModPack* a, const OmModPack* b);
    static bool           _compare_net_name(const OmNetPack* a, const OmNetPack* b);
    static bool           _compare_net_stat(const OmNetPack* a, const OmNetPack* b);
    static bool           _compare_net_vers(const OmNetPack* a, const OmNetPack* b);
    static bool           _compare_net_cate(const OmNetPack* a, const OmNetPack* b);
    static bool           _compare_net_size(const OmNetPack* a, const OmNetPack* b);

    // channel properties
    OmWString             _path;

    OmWString             _home;

    OmWString             _uuid;

    OmWString             _title;

    int32_t               _index;

    // channel main paths
    OmWString             _target_path;

    bool                  _cust_library_path;

    OmWString             _library_path;

    bool                  _cust_backup_path;

    OmWString             _backup_path;

    // mods library
    OmPModPackArray       _modpack_list;

    int32_t               _modpack_list_sort;

    // network library
    OmPNetPackArray       _netpack_list;

    int32_t               _netpack_list_sort;

    // repositories
    OmPNetRepoArray       _repository_list;

    // threads management
    bool                  _locked_mod_library;

    bool                  _locked_net_library;

    // mods install/restore
    bool                  _modops_abort;

    void*                 _modops_hth;

    void*                 _modops_hwo;

    OmPModPackQueue       _modops_queue;

    uint32_t              _modops_dones;

    uint32_t              _modops_percent;

    static DWORD WINAPI   _modops_run_fn(void*);

    static bool           _modops_progress_fn(void*, size_t, size_t, uint64_t);

    static VOID WINAPI    _modops_end_fn(void*,uint8_t);

    Om_beginCb            _modops_begin_cb;

    Om_progressCb         _modops_progress_cb;

    Om_resultCb           _modops_result_cb;

    void*                 _modops_user_ptr;

    // mods download stuff
    bool                  _download_abort;

    OmPNetPackQueue       _download_queue;

    uint32_t              _download_dones;

    uint32_t              _download_percent;

    static void           _download_result_fn(void*, OmResult, uint64_t);

    static bool           _download_download_fn(void*, int64_t, int64_t, int64_t, uint64_t);

    Om_downloadCb         _download_download_cb;

    Om_resultCb           _download_result_cb;

    void*                 _download_user_ptr;

    // mods upgrade stuff
    bool                  _upgrade_abort;

    void*                 _upgrade_hth;

    void*                 _upgrade_hwo;

    OmPNetPackQueue       _upgrade_queue;

    uint32_t              _upgrade_dones;

    uint32_t              _upgrade_percent;

    static DWORD WINAPI   _upgrade_run_fn(void*);

    static bool           _upgrade_progress_fn(void*, size_t, size_t, uint64_t);

    static VOID WINAPI    _upgrade_end_fn(void*,uint8_t);

    Om_beginCb            _upgrade_begin_cb;

    Om_progressCb         _upgrade_progress_cb;

    Om_resultCb           _upgrade_result_cb;

    void*                 _upgrade_user_ptr;

    // repositories query stuff
    bool                  _query_abort;

    void*                 _query_hth;

    void*                 _query_hwo;

    OmPNetRepoQueue       _query_queue;

    uint32_t              _query_dones;

    uint32_t              _query_percent;

    static DWORD WINAPI   _query_run_fn(void*);

    static VOID WINAPI    _query_end_fn(void*,uint8_t);

    Om_beginCb            _query_begin_cb;

    Om_resultCb           _query_result_cb;

    void*                 _query_user_ptr;

    // channel options


    bool                  _library_devmode;

    bool                  _library_showhidden;

    bool                  _warn_overlaps;

    bool                  _warn_extra_inst;

    int32_t               _backup_comp_method;

    int32_t               _backup_comp_level;

    bool                  _warn_extra_unin;

    bool                  _warn_extra_dnld;

    bool                  _warn_miss_deps;

    bool                  _warn_miss_dnld;

    bool                  _warn_upgd_brk_deps;

    bool                  _upgd_rename;
};

/// \brief OmModChan pointer array
///
/// Typedef for an STL vector of OmModChan pointer type
///
typedef std::vector<OmModChan*> OmPModChanArray;

#endif // OMMODCHAN_H
