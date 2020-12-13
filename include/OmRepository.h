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

  string              link;   ///< file download URL

  wstring             desc;   ///< Entry description

  string              img;    ///< image src URL

};

/// \brief Repository object for Location.
///
/// The Repository object defines environment for network packages repository
/// and provide interface to retrieve and download repository definition.
///
class OmRepository
{
  friend class OmLocation;

  public:

    OmRepository(OmLocation* pLoc);

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

    const wstring& base() const {
      return _base;
    }

    const wstring& name() const {
      return _name;
    }

    bool setAddress(const wstring& base, const wstring& name);

    bool querry();

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
