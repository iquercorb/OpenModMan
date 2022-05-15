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

#ifndef OMCONTEXT_H
#define OMCONTEXT_H

#include "OmBase.h"
#include "OmBaseWin.h"

#include "OmConfig.h"
#include "OmLocation.h"
#include "OmBatch.h"

class OmManager;

/// \brief OmManager Context object.
///
/// The Context object describe a global environment for package management.
///
class OmContext
{
  friend class OmLocation;
  friend class Package;

  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmContext(OmManager* pMgr);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmContext();

    /// \brief Open Context.
    ///
    /// Load Context from specified file.
    ///
    /// \param[in]  path    : File path of Context to be loaded.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool open(const wstring& path);

    /// \brief Close Context.
    ///
    /// Close and empty the current instance.
    ///
    void close();

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

    /// \brief Get Context file path.
    ///
    /// Returns Context file path.
    ///
    /// \return Context file path.
    ///
    const wstring& path() const {
      return _path;
    }

    /// \brief Get Context UUID.
    ///
    /// Returns Context UUID.
    ///
    /// \return Context UUID.
    ///
    const wstring& uuid() const {
      return _uuid;
    }

    /// \brief Get Context title.
    ///
    /// Returns Context title.
    ///
    /// \return Context title.
    ///
    const wstring& title() const {
      return _title;
    }

    /// \brief Get Context home directory.
    ///
    /// Returns home directory.
    ///
    /// \return Context home directory.
    ///
    const wstring& home() const {
      return _home;
    }

    /// \brief Get Context icon.
    ///
    /// Returns Context icon as icon handle.
    ///
    /// \return Banner bitmap handle.
    ///
    const HICON& icon() const {
      return _icon;
    }

    /// \brief Set Context title.
    ///
    /// Defines and save Context title.
    ///
    /// \param[in]  title   : Title to defines and save
    ///
    void setTitle(const wstring& title);

    /// \brief Set Context icon.
    ///
    /// Defines the Context icon source file. This must be a valid path to
    /// an icon or executable file or empty string to remove current setting.
    ///
    /// \param[in]  src     : Path to file to extract or empty
    ///                       string to remove current.
    ///
    void setIcon(const wstring& src);

    /// \brief Get Context Location count.
    ///
    /// Returns count of Location defined in the Context.
    ///
    /// \return Location count.
    ///
    size_t locCount() {
      return _locLs.size();
    }

    /// \brief Get Location.
    ///
    /// Returns Location at index.
    ///
    /// \param[in]  i      : Location index.
    ///
    /// \return Location object at index or nullptr if index is out of bound.
    ///
    OmLocation* locGet(unsigned i) {
      return (i < _locLs.size()) ? _locLs[i] : nullptr;
    }

    /// \brief Get Location.
    ///
    /// Returns Location with specified UUID.
    ///
    /// \param[in]  uuid     : Location UUID to search.
    ///
    /// \return Location object or nullptr if not found.
    ///
    OmLocation* locGet(const wstring& uuid);

    /// \brief Get Location index.
    ///
    /// Returns the index of the Location that matches the specified UUID.
    ///
    /// \param[in]  uuid     : Location UUID to search.
    ///
    /// \return Location index or -1 if not found.
    ///
    int locIndex(const wstring& uuid);

    /// \brief Sort Location list.
    ///
    /// Sort Location list according Location ordering index.
    ///
    void locSort();

    /// \brief Select Location.
    ///
    /// Sets the specified Location as active one.
    ///
    /// \param[in]  i       : Location index or -1 to unselect.
    ///
    /// \return True if operation succeed, false if id is out of bound.
    ///
    bool locSel(int i);

    /// \brief Select Location.
    ///
    /// Sets the specified Location as active one.
    ///
    /// \param[in]  uuid    : Location UUID to select.
    ///
    /// \return True if operation succeed, false if Location with such UUID does not exists.
    ///
    bool locSel(const wstring& uuid);

    /// \brief Get active Location.
    ///
    /// Returns current active Location.
    ///
    /// \return Current active Location or nullptr if none is active.
    ///
    OmLocation* locCur() {
      return _locCur >= 0 ? _locLs[_locCur] : nullptr;
    }

    /// \brief Get active Location index.
    ///
    /// Returns index of the current active Location.
    ///
    /// \return Index of the active Location.
    ///
    int locCurId() const {
      return _locCur;
    }

    /// \brief Make new Location.
    ///
    /// Creates a new Location within the Context.
    ///
    /// \param[in]  title     : Title of new Location to be created.
    /// \param[in]  install   : Package installation destination folder path.
    /// \param[in]  library   : Custom package Library folder path.
    /// \param[in]  backup    : Custom package Backup folder path.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool locAdd(const wstring& title, const wstring& install, const wstring& library, const wstring& backup);

    /// \brief Purge existing Location.
    ///
    /// Cleanup and removes a Location. Notice that this operation actually delete
    /// the Location folder and configuration files.
    ///
    /// \param[in]  i           : Location index to be removed.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool locRem(unsigned i);

    /// \brief Get Context Batch count.
    ///
    /// Returns count of Batch defined in the Context.
    ///
    /// \return Batch count.
    ///
    size_t batCount() {
      return _batLs.size();
    }

    /// \brief Get Batch.
    ///
    /// Returns Batch at index.
    ///
    /// \param[in]  i      : Batch index to get.
    ///
    /// \return Batch object at index.
    ///
    OmBatch* batGet(unsigned i) {
      return (i < _batLs.size()) ? _batLs[i] : nullptr;
    }

    /// \brief Sort Batches list.
    ///
    /// Sort Batches list according Location ordering index.
    ///
    void batSort();

    /// \brief Make new Batch.
    ///
    /// Creates a new Batch within the Context.
    ///
    /// \param[in]  title         : Title of new Batch to be created.
    ///
    /// \return Newly created batch object
    ///
    OmBatch* batAdd(const wstring& title);

    /// \brief Delete Batch.
    ///
    /// Delete batch definition file and remove it from list.
    ///
    /// \param[in]  i     : Batch index to delete.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool batRem(unsigned i);

    /// \brief Get batches warning quiet mode.
    ///
    /// Returns batches warning quiet mode option value.
    ///
    /// \return warning quiet mode option value.
    ///
    bool batQuietMode() const {
      return _batQuietMode;
    }

    /// \brief Set batches warning quiet mode.
    ///
    /// Define and save batches warning quiet mode option value.
    ///
    /// \param[in]  enable    : Warning quiet mode enable or disable.
    ///
    void setBatQuietMode(bool enable);

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

  private: ///              - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmManager*          _manager;

    OmConfig            _config;

    wstring             _path;

    wstring             _uuid;

    wstring             _title;

    wstring             _home;

    HICON               _icon;

    vector<OmLocation*> _locLs;

    int                 _locCur;

    vector<OmBatch*>    _batLs;

    bool                _batQuietMode;

    bool                _valid;

    wstring             _error;
};

#endif // OMCONTEXT_H
