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
#ifndef OMMANAGER_H
#define OMMANAGER_H

#include "OmBase.h"

#include "OmXmlConf.h"
#include "OmModHub.h"

/// \brief Log callback.
///
/// Generic callback function for log operation
///
/// \param[in]  ptr   : User data pointer.
/// \param[in]  log   : Log line string.
///
/// \return True to continue, false to abort process.
///
typedef bool (*Om_onlogCb)(void* ptr, const OmWString& log);


// maximum count of recent file path to store
#define OM_MANAGER_MAX_RECENT     10

/// \brief Main manager application object.
///
/// This is the main "back end" application entry point object.
///
class OmModMan
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmModMan();

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmModMan();

    /// \brief Initialize application.
    ///
    /// Initializes the application, load or create initial configuration.
    ///
    /// \param[in]  hwnd  : Main Windows handle.
    /// \param[in]  arg   : Optional command line arguments string.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool init(const char* arg = nullptr);

    /// \brief Quit application.
    ///
    /// Clean the instance and quit safely.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool quit();

    /// \brief Pending command line arguments
    ///
    /// Pending command line arguments string that may be processed by UI
    ///
    /// \return Command line argument string.
    ///
    OmWString pendingArg() const {
      return this->_args;
    }

    /// \brief Clear pending arguments
    ///
    /// Clear pending command line arguments string
    ///
    void clearPendingArg() {
      this->_args.clear();
    }

    /// \brief Open from argument.
    ///
    /// Try to open Mod Hub from argument string, seeking for
    /// known file type and expected Hub home organization.
    ///
    /// \param[in]  arg       : Argument string.
    /// \param[in]  select    : Select the opened Mod Hub.
    ///
    /// \return Operation result code, OM_RESULT_PENDING is returned if
    ///         further operation is possible, like opening Mod Pack in
    ///         editor.
    ///
    OmResult openArg(const char* arg, bool select = true);

    /// \brief Create new Mod Hub.
    ///
    /// Create a new Mod Hub configuration in the specified path.
    ///
    /// \param[in]  path      : Path where to create Mod Hub.
    /// \param[in]  name      : New Mod Hub title.
    /// \param[in]  open      : Open newly created Mod Hub after creation.
    ///
    /// \return Operation result code.
    ///
    OmResult createHub(const OmWString& path, const OmWString& name, bool open = true);

    /// \brief Open Mod Hub.
    ///
    /// Open a Mod Hub configuration at the specified path.
    ///
    /// \param[in]  path      : Mod Hub file path.
    /// \param[in]  select    : Select the opened Mod Hub.
    ///
    /// \return Operation result code.
    ///
    OmResult openHub(const OmWString& path, bool select = true);

    /// \brief Close Mod Hub.
    ///
    /// Close the specified Mod Hub at index.
    ///
    /// \param[in] index  : Mod Hub index to close, or -1 for current selected.
    ///
    void closeHub(int32_t index = -1);

    /// \brief Get Mod Hub count.
    ///
    /// Returns loaded Mod Hub count.
    ///
    /// \return Loaded Mod Hub count.
    ///
    size_t hubCount() const {
      return this->_hub_list.size();
    }

    /// \brief Get Mod Hub.
    ///
    /// Returns loaded Mod Hub instance at index.
    ///
    /// \param[in] index  : Mod Hub index.
    ///
    /// \return Loaded Mod Hub instance.
    ///
    OmModHub* getHub(size_t index) {
      return this->_hub_list[index];
    }

    /// \brief Set selected Mod Hub.
    ///
    /// Select the specified Mod Hub at index. The selected Mod Hub will be
    /// available by via the OmModMan.ctxCur Mod Hub.
    ///
    /// \param[in] index  : Index of Mod Hub to select or -1 to unselect.
    ///
    void selectHub(int32_t index);

    /// \brief Get active Mod Hub.
    ///
    /// Returns current active Mod Hub.
    ///
    /// \return Current active Mod Hub.
    ///
    OmModHub* activeHub() const;

    /// \brief Get active Mod Hub index.
    ///
    /// Returns current active Mod Hub index.
    ///
    /// \return Current active Mod Hub index.
    ///
    int activeHubIndex() const {
      return this->_active_hub;
    }

    /// \brief Get Hub index in list
    ///
    /// Returns index of the given Hub object in list.
    ///
    /// \return Mod Hub index or -1 if not found.
    ///
    int32_t indexOfHub(const OmModHub* ModHub);

    /// \brief Get active Mod Channel
    ///
    /// Returns active Mod Channel of the current active Mod Hub
    ///
    /// \return Active Mod Channel or nullptr.
    ///
    OmModChan* activeChannel() const;

    /// \brief Save configuration window RECT.
    ///
    /// Stores the specified window RECT to configuration file.
    ///
    /// \param[in]  rect    : Window RECT to save.
    ///
    void saveWindowRect(const RECT& rect);

    /// \brief Load configuration window RECT.
    ///
    /// Retrieve the stored window RECT from configuration file.
    ///
    /// \param[out]  rect   : RECT reference to get saved data.
    ///
    void loadWindowRect(RECT& rect);

    /// \brief Add path to recent file list.
    ///
    /// Add a new path ot recent file list.
    ///
    /// \param[in]  path    : Path to add.
    ///
    void addRecentFile(const OmWString& path);

    /// \brief Load recent file path list.
    ///
    /// Retrieve saved recent files path list.
    ///
    /// \param[in]  paths   : Wide string array to be filled.
    ///
    void getRecentFileList(OmWStringArray& paths);

    /// \brief Remove recent files path
    ///
    /// Remove specified path from recent files path list.
    ///
    /// \param[in]  path    : Path to search and remove.
    ///
    /// \return True if path was removed, false otherwise
    ///
    bool removeRecentFile(const OmWString& path);

    /// \brief Clear configuration recent files list.
    ///
    /// Deletes all recent files path list from configuration file.
    ///
    void clearRecentFileList();

    /// \brief Save default working location.
    ///
    /// Saves the default working location path in configuration file.
    ///
    /// \param[in]  path    : last used Mod Hub path.
    ///
    void saveDefaultLocation(const OmWString& path);

    /// \brief Get default working location
    ///
    /// Retrieve best default save location path, according last created
    /// Mod Hub and or environment variables.
    ///
    /// \param[in]  path    : Reference to wide string array to get path.
    ///
    void loadDefaultLocation(OmWString& path);

    /// \brief Save configuration startup Hubs.
    ///
    /// Stores startup Hub path list.
    ///
    /// \param[in]  enable  : enable auto-load.
    /// \param[in]  path    : startup Mod Hub(s) file path list.
    ///
    void saveStartHubs(bool enable, const OmWStringArray& path);

    /// \brief Load configuration startup Hubs.
    ///
    /// Retrieve path and enabled state of startup Hub list
    ///
    /// \param[in]  path    : Pointer to boolean to get auto-load.
    /// \param[in]  path    : Reference to wide string to get path.
    ///
    void getStartHubs(bool* enable, OmWStringArray& path);

    /// \brief Remove startup Hub path.
    ///
    /// Remove specified path from startup Hub list.
    ///
    /// \param[in]  path    : Path to search and remove.
    ///
    /// \return True if path was removed, false otherwise
    ///
    bool removeStartHub(const OmWString& path);

    /// \brief Add startup Hub path.
    ///
    /// Add the specified path to startup Hub list.
    ///
    /// \param[in]  path    : Path to add.
    ///
    void addStartHub(const OmWString& path);

    /// \brief Get icons size option.
    ///
    /// Returns icons size option value.
    ///
    /// \return Icons size in pixels
    ///
    unsigned iconsSize() const {
      return this->_icon_size;
    }

    /// \brief Set icons size option.
    ///
    /// Define and save icons size option value.
    ///
    /// \param[in]  size    : Icons size in pixels
    ///
    void setIconsSize(unsigned size);

    /// \brief Get no-markdown option.
    ///
    /// Returns no-markdown option value.
    ///
    /// \return True if enabled, false otherwise.
    ///
    bool noMarkdown() const {
      return this->_no_markdown;
    }

    /// \brief Set no-markdown option.
    ///
    /// Define and save no-markdown option value.
    ///
    /// \param[in]  enable  : Boolean value to set.
    ///
    void setNoMarkdown(bool enable);

    /// \brief Get Link Warning option.
    ///
    /// Returns Link Warning option value.
    ///
    /// \return True if enabled, false otherwise.
    ///
    bool linkConfirm() const {
      return this->_link_confirm;
    }

    /// \brief Set Link Warning option.
    ///
    /// Define and save Link Warning option value.
    ///
    /// \param[in]  enable  : Boolean value to set.
    ///
    void setLinkConfirm(bool enable);

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

    /// \brief Add log callback
    ///
    /// Add callback function to be called when new log is added
    ///
    /// \param[in] notify_cb  : Pointer to callback function
    /// \param[in] user_ptr  : Custom user pointer to be passed to callback
    ///
    void addLogNotify(Om_notifyCb notify_cb, void* user_ptr);

    /// \brief Remove log callback
    ///
    /// Remove the specified callback function
    ///
    /// \param[in] notify_cb  : Pointer to callback function to remove
    ///
    void removeLogNotify(Om_notifyCb notify_cb);

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const OmWString& origin, const OmWString& detail);

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const OmWString& lastError() const {
      return this->_lasterr;
    }

    /// \brief Get log string.
    ///
    /// Returns log string.
    ///
    /// \return Log string.
    ///
    const OmWString& currentLog() const {
      return this->_log_str;
    }

    /// \brief Escalate log.
    ///
    /// Public function to allow "children" item to escalate log
    ///
    void escalateLog(unsigned level, const OmWString& origin, const OmWString& detail) {
      this->_log(level, origin, detail);
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // required stuff
    OmXmlConf             _xml;

    // general parameters
    OmWString             _home;

    // Arguments management
    OmWString             _args;

    // Mod Hubs management
    OmPModHubArray        _hub_list;

    int32_t               _active_hub;

    // active channel libraries monitoring
    void                  _modlib_notify_enable(bool enable);

    static void           _modlib_notify_fn(void*, OmNotify, uint64_t);

    Om_notifyCb           _modlib_notify_cb;

    void*                 _modlib_notify_ptr;

    void                  _netlib_notify_enable(bool enable);

    static void           _netlib_notify_fn(void*, OmNotify, uint64_t);

    Om_notifyCb           _netlib_notify_cb;

    void*                 _netlib_notify_ptr;

    // application log management
    OmWString             _log_str;

    void*                 _log_hfile;

    OmNotifyCbArray       _log_notify_cb;

    OmPVoidArray          _log_user_ptr;

    // general options
    unsigned              _icon_size;

    bool                  _no_markdown;

    bool                  _link_confirm;

    // logs and errors
    void                  _log(unsigned level, const OmWString& origin, const OmWString& detail);

    void                  _error(const OmWString& origin, const OmWString& detail);

    OmWString             _lasterr;
};

#endif // OMMANAGER_H
