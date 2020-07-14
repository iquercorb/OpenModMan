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

#include "OmGlobal.h"
#include "OmConfig.h"
#include "OmContext.h"

// maximum count of recent file path to store
#define OM_MANAGER_MAX_RECENT     10


/// \brief Main manager application object.
///
/// This is the main "back end" application entry point object.
///
class OmManager
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmManager();

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmManager();

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const wstring& lastError() const {
      return _error;
    }

    /// \brief Get log string.
    ///
    /// Returns log string.
    ///
    /// \return Log string.
    ///
    const wstring& getLog() const {
      return _log;
    }

    /// \brief Initialize application.
    ///
    /// Initializes the application, load or create initial configuration.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool init();

    /// \brief Quit application.
    ///
    /// Clean the instance and quit safely.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool quit();

    /// \brief Get Context count.
    ///
    /// Returns loaded Context count.
    ///
    /// \return Loaded Context count.
    ///
    size_t contextCount() const {
      return _context.size();
    }

    /// \brief Get Context.
    ///
    /// Returns loaded Context instance at index.
    ///
    /// \param[in]  i     : Context index.
    ///
    /// \return Loaded Context instance.
    ///
    OmContext* context(unsigned i) {
      return _context[i];
    }

    /// \brief Set selected Context.
    ///
    /// Select the specified Context at index. The selected Context will be
    /// available by via the OmManager.curContext Context.
    ///
    /// \param[in]  i     : Index of Context to select or -1 to unselect.
    ///
    void selContext(int i);

    /// \brief Get selected Context.
    ///
    /// Returns current selected Context.
    ///
    /// \return Current selected Context.
    ///
    OmContext* curContext() const {
      return _curContext;
    }

    /// \brief Save configuration window RECT.
    ///
    /// Stores the specified window RECT to configuration file.
    ///
    /// \param[in]  rect    : Window RECT to save.
    ///
    void saveWindowRect(const RECT& rect);

    /// \brief Load configuration window RECT.
    ///
    /// Retrieve the specified window RECT from configuration file.
    ///
    /// \param[in]  rect    : RECT reference to get saved data.
    ///
    void loadWindowRect(RECT& rect);

    /// \brief Save configuration recent file.
    ///
    /// Stores a new recent file path to configuration file.
    ///
    /// \param[in]  path    : Path to file.
    ///
    void saveRecentFile(const wstring& path);

    /// \brief Load configuration recent files list.
    ///
    /// Retrieve recent files path list from configuration file.
    ///
    /// \param[in]  paths   : Reference to wide string array to get list.
    ///
    void loadRecentFiles(vector<wstring>& paths);

    /// \brief Clear configuration recent files list.
    ///
    /// Deletes all recent files path list from configuration file.
    ///
    void clearRecentFiles();

    /// \brief Save default working location.
    ///
    /// Saves the default working location path in configuration file.
    ///
    /// \param[in]  path    : last used Context path.
    ///
    void saveDefaultLocation(const wstring& path);

    /// \brief Get default working location
    ///
    /// Retrieve best default save location path, according last created
    /// Context and or environment variables.
    ///
    /// \param[in]  path    : Reference to wide string array to get path.
    ///
    void getDefaultLocation(wstring& path);

    /// \brief Save configuration startup Context.
    ///
    /// Stores startup Context path.
    ///
    /// \param[in]  enable  : enable auto-load.
    /// \param[in]  path    : startup Context(s) file path list.
    ///
    void saveStartContexts(bool enable, const vector<wstring>& path);

    /// \brief Load configuration startup Context(s) path.
    ///
    /// Retrieve path to Context(s) to be loaded at application start.
    ///
    /// \param[in]  path    : Pointer to boolean to get auto-load.
    /// \param[in]  path    : Reference to wide string to get path.
    ///
    void getStartContexts(bool* enable, vector<wstring>& path);

    /// \brief Get icons size option.
    ///
    /// Returns icons size option value.
    ///
    /// \return Icons size in pixels
    ///
    unsigned iconsSize() const {
      return _iconsSize;
    }

    /// \brief Set icons size option.
    ///
    /// Define and save icons size option value.
    ///
    /// \param[in]  size    : Icons size in pixels
    ///
    void setIconsSize(unsigned size);

    /// \brief Get package legacy support size option.
    ///
    /// Returns package legacy support option value.
    ///
    /// \return Package legacy support option value.
    ///
    bool legacySupport() const {
      return _folderPackages;
    }

    /// \brief Set package legacy support option.
    ///
    /// Define and save package legacy support option value.
    ///
    /// \param[in]  enable    : Package legacy support enable or disable.
    ///
    void setLegacySupport(bool enable);

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
      return _warnExtraInstall;
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
    bool warnMissDpnd() const {
      return _warnMissingDepend;
    }

    /// \brief Set warning for missing dependency option.
    ///
    /// Define and save warning for missing dependency option value.
    ///
    /// \param[in]  enable    : Warning for missing dependency enable or disable.
    ///
    void setWarnMissDpnd(bool enable);

    /// \brief Get warning for extra uninstall option.
    ///
    /// Returns warning for extra uninstall option value.
    ///
    /// \return Warning for extra uninstall option value.
    ///
    bool warnExtraUnin() const {
      return _warnExtraUninst;
    }

    /// \brief Set warning for extra uninstall option.
    ///
    /// Define and save warning for extra uninstall option value.
    ///
    /// \param[in]  enable    : Warning for extra uninstall enable or disable.
    ///
    void setWarnExtraUnin(bool enable);

    /// \brief Get warning quiet for batches option.
    ///
    /// Returns warning quiet for batches option value.
    ///
    /// \return Warning quiet for batches option value.
    ///
    bool quietBatches() const {
      return _quietBatches;
    }

    /// \brief Set warning quiet for batches option.
    ///
    /// Define and save warning quiet for batches option value.
    ///
    /// \param[in]  enable    : Warning quiet for batches enable or disable.
    ///
    void setQuietBatches(bool enable);

    /// \brief Create new Context.
    ///
    /// Create a new Context configuration in the specified path.
    ///
    /// \param[in]  title     : New Context title.
    /// \param[in]  path      : Path where to create Context.
    /// \param[in]  open      : Open newly created Context after creation.
    ///
    bool makeContext(const wstring& title, const wstring& path, bool open = true);

    /// \brief Open Context.
    ///
    /// Open a Context configuration at the specified path.
    ///
    /// \param[in]  path      : Context file path.
    ///
    bool openContext(const wstring& path);

    /// \brief Close current Context.
    ///
    /// Close current Context.
    ///
    void closeCurrContext();

    /// \brief Set log output.
    ///
    /// Defines an Edit Control to output log in real time.
    ///
    /// \param[in]  hwnd      : Windows handle (HWND) to Edit Control.
    ///
    void setLogOutput(HWND hWnd);

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    wstring             _home;

    OmConfig            _config;

    vector<OmContext*>  _context;

    OmContext*          _curContext;

    // general options
    unsigned            _iconsSize;

    bool                _folderPackages;

    bool                _warnEnabled;

    bool                _warnOverlaps;

    bool                _warnExtraInstall;

    bool                _warnMissingDepend;

    bool                _warnExtraUninst;

    bool                _quietBatches;


    wstring             _error;

    wstring             _log;

    void*               _logHwnd;

    void*               _logFile;

};

#endif // OMMANAGER_H
