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
#ifndef OMUTILPKG_H_INCLUDED
#define OMUTILPKG_H_INCLUDED

#include "OmBase.h"

/// \brief Parse Package filename
///
/// Parse the Packag display name and potential version substring from its
/// file name.
///
/// \param[out] name      : Parsed display name.
/// \param[out] core      : Parsed core name.
/// \param[out] vers      : Parsed version if any.
/// \param[in]  filename  : Filename to be parsed.
/// \param[in]  isfile    : Specify whether filename is file or a folder name.
/// \param[in]  us2spc    : Specify whether underscores must be replaced by spaces.
///
/// \return True if version string candidate was found, false otherwise
///
bool Om_parsePkgIdent(wstring& name, wstring& core, wstring& vers, const wstring& filename, bool isfile = true, bool us2spc = true);

#endif // OMUTILPKG_H_INCLUDED
