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

#ifndef OMREPOSITORY_H
#define OMREPOSITORY_H

#include "OmGlobal.h"
#include "OmConfig.h"

class OmLocation;

/// \brief Package item.
///
/// Simple structure for package file or folder item.
///
struct OmRepoItem {

  wstring             ident;  ///< Entry identity

  string              href;   ///< file download URL

  wstring             desc;   ///< Entry description

  string              img;    ///< image src URL

};

/// \brief Repository object for Context.
///
/// The Repository object defines environment for network packages repository
/// and provide interface to retrieve and download repository definition.
///
class OmRepository
{
  friend class OmLocation;

  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmRepository(OmLocation* pLoc);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmRepository();

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

    /// \brief Get Repository base.
    ///
    /// Returns Repository HTTP base address.
    ///
    /// \return Repository HTTP base address.
    ///
    const wstring& base() const {
      return _base;
    }

    /// \brief Get Repository name.
    ///
    /// Returns Repository HTTP suffix name.
    ///
    /// \return Repository HTTP suffix name.
    ///
    const wstring& name() const {
      return _name;
    }

    /// \brief Get Repository URL.
    ///
    /// Returns Repository URL.
    ///
    /// \return Repository URL.
    ///
    const string& url() const {
      return _url;
    }

    /// \brief Set Repository parameters.
    ///
    /// Define the Repository base and suffix to define HTTP URL.
    ///
    /// \param[in]  base    : Repository HTTP base address.
    /// \param[in]  name    : Repository HTTP suffix name.
    ///
    /// \return True if parameters makes a valid HTTP URL, false otherwise.
    ///
    bool define(const wstring& base, const wstring& name);

    /// \brief Update from remote data.
    ///
    /// Get data from remote server to update local data.
    ///
    /// \return True if update succeed, false if an error occurred.
    ///
    bool update();

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmLocation*         _location;

    wstring             _base;

    wstring             _name;

    string              _url;

    vector<OmRepoItem>  _item;

    bool                _valid;

    wstring             _error;
};

#endif // OMREPOSITORY_H
