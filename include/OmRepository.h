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
#include "OmRemote.h"

class OmLocation;

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

    /// \brief Set repository parameters.
    ///
    /// Defines repository address parameter to query data.
    ///
    /// \param[in]  base    : Repository HTTP base address.
    /// \param[in]  name    : Repository HTTP suffix name.
    ///
    /// \return True if parameters makes a valid HTTP URL, false otherwise.
    ///
    bool init(const wstring& base, const wstring& name);

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

    /// \brief Get base address.
    ///
    /// Returns repository HTTP base address.
    ///
    /// \return HTTP base address.
    ///
    const wstring& base() const {
      return _base;
    }

    /// \brief Get name.
    ///
    /// Returns repository suffix name (definition name).
    ///
    /// \return Suffix name.
    ///
    const wstring& name() const {
      return _name;
    }

    /// \brief Get URL.
    ///
    /// Returns real definition file URL as combination of base and name.
    ///
    /// \return Definition file URL.
    ///
    const wstring& url() const {
      return _url;
    }

    /// \brief Query remote data.
    ///
    /// Get data from remote server to update local data.
    ///
    /// \return True if update succeed, false if an error occurred.
    ///
    bool query();

    /// \brief Get title.
    ///
    /// Returns repository indicative title.
    ///
    /// \return Indicative title.
    ///
    const wstring& uuid() const {
      return _uuid;
    }

    /// \brief Get title.
    ///
    /// Returns repository indicative title.
    ///
    /// \return Indicative title.
    ///
    const wstring& title() const {
      return _title;
    }

    /// \brief Get download path.
    ///
    /// Returns repository common download path, this is the
    /// path added to base address where to find files to be
    /// downloaded.
    ///
    /// \return Common download path.
    ///
    const wstring& downpath() const {
      return _downpath;
    }

    /// \brief Merge remote package list.
    ///
    /// Parse remote packages list and merge them to the given
    /// array. Remote package with same identity are merged with
    /// multiples download URL.
    ///
    /// \param[in]  rmt_ls  : Package list to be filled
    ///
    /// \return Parsed remote packages count.
    ///
    size_t rmtMerge(vector<OmRemote*>& rmt_ls);

    /// \brief Get remote package count.
    ///
    /// Returns remote package count of the repository.
    ///
    /// \return Packages count.
    ///
    size_t rmtCount();

    /// \brief Check whether has remote.
    ///
    /// Check whether repository has remote with the specified identity.
    ///
    /// \return True if remote package found, false otherwise.
    ///
    bool rmtHas(const wstring& ident);

    /// \brief Clear repository.
    ///
    /// Resets repository. This function does not erase the
    /// initialization parameters (base URL and name).
    ///
    void clear();

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmLocation*         _pLoc;

    OmConfig            _config;

    wstring             _base;

    wstring             _name;

    wstring             _url;

    wstring             _uuid;

    wstring             _title;

    wstring             _downpath;

    bool                _valid;

    wstring             _error;
};

#endif // OMREPOSITORY_H
