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

    /// \brief Create new Mod Hub.
    ///
    /// Create a new Mod Hub configuration in the specified path.
    ///
    /// \param[in]  path      : Path where to create Mod Hub.
    /// \param[in]  name      : New Mod Hub title.
    /// \param[in]  open      : Open newly created Mod Hub after creation.
    ///
    bool createHub(const OmWString& path, const OmWString& name, bool open = true);

    /// \brief Open Mod Hub.
    ///
    /// Open a Mod Hub configuration at the specified path.
    ///
    /// \param[in]  path      : Mod Hub file path.
    /// \param[in]  select    : Select the opened Mod Hub.
    ///
    bool openHub(const OmWString& path, bool select = true);

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

    /// \brief Save configuration window foot height.
    ///
    /// Stores the specified window foot frame height to configuration file.
    ///
    /// \param[in]  height  : Foot height to save.
    ///
    void saveWindowFoot(int height);

    /// \brief Load configuration window foot height.
    ///
    /// Retrieve the stored window foot frame height to configuration file.
    ///
    /// \param[out]  height : Foot height to save.
    ///
    void loadWindowFoot(int* height);

    /// \brief Save configuration recent file.
    ///
    /// Stores a new recent file path to configuration file.
    ///
    /// \param[in]  path    : Path to file.
    ///
    void saveRecentFile(const OmWString& path);

    /// \brief Load configuration recent files list.
    ///
    /// Retrieve recent files path list from configuration file.
    ///
    /// \param[in]  paths   : Reference to wide string array to get list.
    ///
    void loadRecentFiles(OmWStringArray& paths);

    /// \brief Clear configuration recent files list.
    ///
    /// Deletes all recent files path list from configuration file.
    ///
    void clearRecentFiles();

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

    /// \brief Save configuration startup Mod Hub.
    ///
    /// Stores startup Mod Hub path.
    ///
    /// \param[in]  enable  : enable auto-load.
    /// \param[in]  path    : startup Mod Hub(s) file path list.
    ///
    void saveStartHubs(bool enable, const OmWStringArray& path);

    /// \brief Load configuration startup Mod Hub(s) path.
    ///
    /// Retrieve path to Mod Hub(s) to be loaded at application start.
    ///
    /// \param[in]  path    : Pointer to boolean to get auto-load.
    /// \param[in]  path    : Reference to wide string to get path.
    ///
    void loadStartHubs(bool* enable, OmWStringArray& path);

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

    /// \brief Add log callback
    ///
    /// Add callback function to be called when new log is added
    ///
    /// \param[in] onlog_cb  : Pointer to callback function
    /// \param[in] user_ptr  : Custom user pointer to be passed to callback
    ///
    void addLogCallback(Om_onlogCb onlog_cb, void* user_ptr);

    /// \brief Remove log callback
    ///
    /// Remove the specified callback function
    ///
    /// \param[in] onlog_cb  : Pointer to callback function to remove
    ///
    void removeLogCallback(Om_onlogCb onlog_cb);

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
      return this->_applog_str;
    }

    /// \brief Escalate log.
    ///
    /// Public function to allow "children" item to escalate log
    ///
    void escalateLog(unsigned level, const OmWString& origin, const OmWString& detail) {
      this->_log(level, origin, detail);
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // logs and errors
    void                _log(unsigned level, const OmWString& origin, const OmWString& detail);

    void                _error(const OmWString& origin, const OmWString& detail);

    OmWString           _lasterr;

    // application log management
    OmWString           _applog_str;

    void*               _applog_hfile;

    std::vector<Om_onlogCb>  _applog_cli_onlog;

    std::vector<void*>       _applog_cli_ptr;

    // special migration function
    bool                _migrate();

    // required stuff
    OmXmlConf           _xmlconf;

    // general parameters
    OmWString           _home;

    // Mod Hubs management
    OmPModHubArray      _hub_list;

    int32_t             _active_hub;

    // general options
    unsigned            _icon_size;

    bool                _no_markdown;

};

#endif // OMMANAGER_H
