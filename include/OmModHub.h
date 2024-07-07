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

#ifndef OMMODHUB_H
#define OMMODHUB_H

#include "OmBase.h"
#include "OmBaseWin.h"

#include "OmXmlConf.h"

class OmModMan;
#include "OmModChan.h"
#include "OmModPset.h"
#include "OmDirNotify.h"

/// \brief Mod Hub object.
///
/// The Mod Hub object describe a global environment for package management.
///
class OmModHub
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in] ModMan : Pointer to parent Mod Manager object
    ///
    OmModHub(OmModMan* ModMan);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmModHub();

    /// \brief Open Mod Hub.
    ///
    /// Load Mod Hub from specified file.
    ///
    /// \param[in]  path    : File path of Mod Hub to be loaded.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool open(const OmWString& path);

    /// \brief Close Mod Hub.
    ///
    /// Close and empty the current instance.
    ///
    void close();

    /// \brief Check whether is valid.
    ///
    /// Checks whether this instance is correctly loaded a ready to use.
    ///
    /// \return True if this instance is valid, false otherwise.
    ///
    bool valid() const {
      return this->_xml.valid();
    }

    /// \brief Get Mod Hub file path.
    ///
    /// Returns Mod Hub file path.
    ///
    /// \return Mod Hub file path.
    ///
    const OmWString& path() const {
      return this->_path;
    }

    /// \brief Get Mod Hub UUID.
    ///
    /// Returns Mod Hub UUID.
    ///
    /// \return Mod Hub UUID.
    ///
    const OmWString& uuid() const {
      return this->_uuid;
    }

    /// \brief Get Mod Hub title.
    ///
    /// Returns Mod Hub title.
    ///
    /// \return Mod Hub title.
    ///
    const OmWString& title() const {
      return this->_title;
    }

    /// \brief Set Mod Hub title.
    ///
    /// Defines and save Mod Hub title.
    ///
    /// \param[in]  title   : Title to defines and save
    ///
    void setTitle(const OmWString& title);

    /// \brief Get Mod Hub home directory.
    ///
    /// Returns home directory.
    ///
    /// \return Mod Hub home directory.
    ///
    const OmWString& home() const {
      return this->_home;
    }

    /// \brief Get Mod Hub icon.
    ///
    /// Returns Mod Hub icon as icon handle.
    ///
    /// \return Banner bitmap handle.
    ///
    const HICON& icon() const {
      return this->_icon_handle;
    }
    /// \brief Get Mod Hub icon source path.
    ///
    /// Returns Mod Hub icon source path.
    ///
    /// \return Path to icon or executable.
    ///
    const OmWString& iconSource() const {
      return this->_icon_source;
    }

    /// \brief Set Mod Hub icon.
    ///
    /// Defines the Mod Hub icon source file, either icon file
    /// or executable.
    ///
    /// \param[in]  src     : Path to file to extract
    ///
    void setIcon(const OmWString& src);

    /// \brief Make new Mod Channel.
    ///
    /// Creates a new Mod Channel within the Mod Hub.
    ///
    /// \param[in]  title     : Title of new Mod Channel to be created.
    /// \param[in]  install   : Package installation destination folder path.
    /// \param[in]  library   : Custom package Library folder path.
    /// \param[in]  backup    : Custom package Backup folder path.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool createChannel(const OmWString& title, const OmWString& install, const OmWString& library, const OmWString& backup);

    /// \brief Purge existing Mod Channel.
    ///
    /// Cleanup and removes a Mod Channel. Notice that this operation actually delete
    /// the Mod Channel folder and configuration files.
    ///
    /// \param[in] index        : Mod Channel index to be removed.
    /// \param[in] progress_cb  : Callback function for backup purge progression.
    /// \param[in] user_ptr     : Custom pointer to be passed to callback function.
    ///
    /// \return
    ///
    OmResult deleteChannel(size_t index, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr);

    /// \brief Get Mod Channel count.
    ///
    /// Returns count of Mod Channel defined in the Mod Hub.
    ///
    /// \return Mod Channel count.
    ///
    size_t channelCount() const {
      return this->_channel_list.size();
    }

    /// \brief Get Mod Channel.
    ///
    /// Returns Mod Channel at index.
    ///
    /// \param[in]  index  : Mod Channel index.
    ///
    /// \return Mod Channel object at index or nullptr if index is out of bound.
    ///
    OmModChan* getChannel(size_t index) const {
      return this->_channel_list[index];
    }

    /// \brief Get Mod Channel.
    ///
    /// Returns Mod Channel with specified UUID.
    ///
    /// \param[in]  uuid     : Mod Channel UUID to search.
    ///
    /// \return Mod Channel object or nullptr if not found.
    ///
    OmModChan* findChannel(const OmWString& uuid) const;

    /// \brief Get Mod Channel index.
    ///
    /// Returns the index of the Mod Channel that matches the specified UUID.
    ///
    /// \param[in]  uuid     : Mod Channel UUID to search.
    ///
    /// \return Mod Channel index or -1 if not found.
    ///
    int32_t indexOfChannel(const OmWString& uuid);

    /// \brief Get Mod Channel index.
    ///
    /// Returns the index of the Mod Channel that matches the specified UUID.
    ///
    /// \param[in]  ModChan   : Pointer to Mod Channel object to get index of
    ///
    /// \return Mod Channel index or -1 if not found.
    ///
    int32_t indexOfChannel(const OmModChan* ModChan);

    /// \brief Sort Mod Channel list.
    ///
    /// Sort Mod Channel list according Mod Channel ordering index.
    ///
    void sortChannels();

    /// \brief Select Mod Channel.
    ///
    /// Sets the specified Mod Channel as active one.
    ///
    /// \param[in]  index   : Mod Channel index or -1 to unselect.
    ///
    /// \return True if operation succeed, false if id is out of bound.
    ///
    bool selectChannel(int32_t index);

    /// \brief Select Mod Channel.
    ///
    /// Sets the specified Mod Channel as active one.
    ///
    /// \param[in]  uuid    : Mod Channel UUID to select.
    ///
    /// \return True if operation succeed, false if Mod Channel with such UUID does not exists.
    ///
    bool selectChannel(const OmWString& uuid);

    /// \brief Get active Mod Channel.
    ///
    /// Returns current active Mod Channel.
    ///
    /// \return Current active Mod Channel or nullptr if none is active.
    ///
    OmModChan* activeChannel() const;

    /// \brief Get active Mod Channel index.
    ///
    /// Returns index of the current active Mod Channel.
    ///
    /// \return Index of the active Mod Channel.
    ///
    int32_t activeChannelIndex() const {
      return this->_active_channel;
    }

    /// \brief Start active Channel Local Library changes notifications
    ///
    /// Set parameters and enable active channel Local Library changes notifications
    ///
    /// \param[in] notify_cb  : Callback for Local library notifications changes
    /// \param[in] user_ptr   : Custom pointer passed to callbacks
    ///
    void notifyModLibraryStart(Om_notifyCb notify_cb, void* user_ptr = nullptr);

    /// \brief Stop active Channel Local Library changes notifications
    ///
    /// Stops active Channel Local Library changes notifications
    ///
    void notifyModLibraryStop();

    /// \brief Start active Channel Local Library changes notifications
    ///
    /// Set parameters and enable active channel Local Library changes notifications
    ///
    /// \param[in] notify_cb  : Callback for Local library notifications changes
    /// \param[in] user_ptr   : Custom pointer passed to callbacks
    ///
    void notifyNetLibraryStart(Om_notifyCb notify_cb, void* user_ptr = nullptr);

    /// \brief Stop active Channel Local Library changes notifications
    ///
    /// Stops active Channel Local Library changes notifications
    ///
    void notifyNetLibraryStop();

    /// \brief Get Mod Preset count.
    ///
    /// Returns count of Mod Preset referenced in this instance.
    ///
    /// \return Mod Preset count.
    ///
    size_t presetCount() {
      return this->_preset_list.size();
    }

    /// \brief Get Mod Preset
    ///
    /// Returns Mod Preset at index
    ///
    /// \param[in]  index  : index to get Mod Preset
    ///
    /// \return Mod Preset object at index
    ///
    OmModPset* getPreset(size_t index) {
      return this->_preset_list[index];
    }

    /// \brief Index of Mod Preset
    ///
    /// Returns index of the given Mod Present in list
    ///
    /// \param[in]  ModPset  : Mod Preset object to search.
    ///
    /// \return Index in list or -1 if not found
    ///
    int32_t indexOfPreset(const OmModPset* ModPset) const;

    /// \brief Sort Batches list.
    ///
    /// Sort Batches list according Mod Channel ordering index.
    ///
    void sortPresets();

    /// \brief Create new Mod Preset.
    ///
    /// Creates a new Mod Preset within the Mod Hub.
    ///
    /// \param[in]  title   : Title of new Mod Preset to be created.
    ///
    /// \return Newly created batch object
    ///
    OmModPset* createPreset(const OmWString& title);

    /// \brief Delete Mod Preset.
    ///
    /// Delete Mod Preset file and remove it from list.
    ///
    /// \param[in]  index : Mod Preset index to delete.
    ///
    /// \return Operation result code:
    ///         OM_RESULT_OK is returned if operation succedd,
    ///         OM_RESULT_ERROR is returned if an error occurred and
    ///         OM_RESULT_ABORT is returned in case of invalid call
    ///
    OmResult deletePreset(size_t index);

    /// \brief Rename Mod Preset.
    ///
    /// Rename Mod Preset file and update it from list.
    ///
    /// \param[in]  index : Mod Preset index to rename.
    /// \param[in]  title : New name to set.
    ///
    /// \return Operation result code:
    ///         OM_RESULT_OK is returned if operation succedd,
    ///         OM_RESULT_ERROR is returned if an error occurred and
    ///         OM_RESULT_ABORT is returned in case of invalid call
    ///
    OmResult renamePreset(size_t index, const OmWString& title);

    /// \brief Check whether Presets are locked
    ///
    /// Returns whether Presets list and parameters are currently
    /// locked by processing.
    ///
    /// \return True if Presets are locked, false otherwise
    ///
    bool lockedPresets() const {
      return this->_locked_presets;
    }

    void queuePresets(OmModPset* ModPset, Om_beginCb begin_cb = nullptr, Om_progressCb progress_cb = nullptr, Om_resultCb result_cb = nullptr, void* user_ptr = nullptr);

    void abortPresets();

    /// \brief Get batches warning quiet mode.
    ///
    /// Returns batches warning quiet mode option value.
    ///
    /// \return warning quiet mode option value.
    ///
    bool presetQuietMode() const {
      return this->_presets_quietmode;
    }

    /// \brief Set batches warning quiet mode.
    ///
    /// Define and save batches warning quiet mode option value.
    ///
    /// \param[in]  enable    : Warning quiet mode enable or disable.
    ///
    void setPresetQuietMode(bool enable);

    /// \brief Get Show Channels option value.
    ///
    /// Returns Show Channels layout option value.
    ///
    /// \return True or false.
    ///
    bool layoutChannelsShow() const {
      return this->_layout_channels_show;
    }

    /// \brief Set Show Channels option value.
    ///
    /// Define and save Show Channels layout option value.
    ///
    /// \param[in]  enable    : Show Channels enable or disable.
    ///
    void setLayoutChannelsShow(bool enable);

    /// \brief Get Show Presets option value.
    ///
    /// Returns Show Presets layout option value.
    ///
    /// \return True or false.
    ///
    bool layoutPresetsShow() const {
      return this->_layout_presets_show;
    }

    /// \brief Set Show Presets option value.
    ///
    /// Define and save Show Presets layout option value.
    ///
    /// \param[in]  enable    : Show Presets enable or disable.
    ///
    void setLayoutPresetsShow(bool enable);

    /// \brief Get Show Overview option value.
    ///
    /// Returns Show Overview layout option value.
    ///
    /// \return True or false.
    ///
    bool layoutOverviewShow() const {
      return this->_layout_overview_show;
    }

    /// \brief Set Show Overview option value.
    ///
    /// Define and save Show Overview layout option value.
    ///
    /// \param[in]  enable    : Show Overview enable or disable.
    ///
    void setLayoutOverviewShow(bool enable);

    /// \brief Get Channels list layout.
    ///
    /// Returns saved Channels list layout span value.
    ///
    /// \return Height value in pixels.
    ///
    int layoutChannelsSpan() const {
      return this->_layout_channels_span;
    }

    /// \brief Set Channels list layout.
    ///
    /// Define and save Channels list layout span value.
    ///
    /// \param[in]  span     : Channels list layout span value.
    ///
    void setLayoutChannelsSpan(int span);

    /// \brief Get Presets list layout.
    ///
    /// Returns saved Presets list layout span value.
    ///
    /// \return Height value in pixels.
    ///
    int layoutPresetsSpan() const {
      return this->_layout_presets_span;
    }

    /// \brief Set Presets list layout.
    ///
    /// Define and save Presets list layout span value.
    ///
    /// \param[in]  span    : Presets list layout span value.
    ///
    void setLayoutPresetsSpan(int span);

    /// \brief Get Overview list layout.
    ///
    /// Returns saved Overview list layout span value.
    ///
    /// \return Height value in pixels.
    ///
    int layoutOverviewSpan() const {
      return this->_layout_overview_span;
    }

    /// \brief Set Overview list layout.
    ///
    /// Define and save Overview list layout span value.
    ///
    /// \param[in]  span    : Overview list layout span value.
    ///
    void setLayoutOverviewSpan(int span);

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const OmWString& lastError() const {
      return this->_lasterr;
    }

    /// \brief Escalate log.
    ///
    /// Public function to allow "children" item to escalate log
    ///
    void escalateLog(unsigned level, const OmWString& origin, const OmWString& detail) const {
      this->_log(level, origin, detail);
    }

  private: ///            - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // linking
    OmModMan*             _ModMan;

    // needed objects
    OmXmlConf             _xml;

    // sorting comparison functions
    static bool           _compare_chn_index(const OmModChan*, const OmModChan*);

    static bool           _compare_pst_index(const OmModPset*, const OmModPset*);

    // channel properties
    OmWString             _path;

    OmWString             _home;

    OmWString             _uuid;

    OmWString             _title;

    OmWString             _icon_source;

    HICON                 _icon_handle;

    // active channel libraries monitoring
    void                  _modlib_notify_enable(bool enable);

    static void           _modlib_notify_fn(void*, OmNotify, uint64_t);

    Om_notifyCb           _modlib_notify_cb;

    void*                 _modlib_notify_ptr;

    void                  _netlib_notify_enable(bool enable);

    static void           _netlib_notify_fn(void*, OmNotify, uint64_t);

    Om_notifyCb           _netlib_notify_cb;

    void*                 _netlib_notify_ptr;

    // Mod Channels
    OmPModChanArray       _channel_list;

    int32_t               _active_channel;

    // Mod Presets
    OmPModPsetArray       _preset_list;

    // threads management
    bool                  _locked_presets;

    // mods install/restore
    bool                  _psetup_abort;

    void*                 _psetup_hth;

    void*                 _psetup_hwo;

    OmPModPsetQueue       _psetup_queue;

    uint32_t              _psetup_dones;

    uint32_t              _psetup_percent;

    static DWORD WINAPI   _psetup_run_fn(void*);


    static VOID WINAPI    _psetup_end_fn(void*,uint8_t);

    Om_beginCb            _psetup_begin_cb;

    Om_progressCb         _psetup_progress_cb;

    Om_resultCb           _psetup_result_cb;

    void*                 _psetup_user_ptr;

    // options
    bool                  _presets_quietmode;

    bool                  _layout_channels_show;

    int32_t               _layout_channels_span;

    bool                  _layout_presets_show;

    int32_t               _layout_presets_span;

    bool                  _layout_overview_show;

    int32_t               _layout_overview_span;

    // logs and errors
    void                  _log(unsigned level, const OmWString& origin, const OmWString& detail) const;

    void                  _error(const OmWString& origin, const OmWString& detail);

    OmWString             _lasterr;
};

/// \brief OmModHub pointer array
///
/// Typedef for an STL vector of OmModHub pointer type
///
typedef std::vector<OmModHub*> OmPModHubArray;

#endif // OMMODHUB_H
