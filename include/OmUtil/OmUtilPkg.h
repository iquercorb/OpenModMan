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
#ifndef OMUTILPKG_H
#define OMUTILPKG_H

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
bool Om_parsePkgIdent(OmWString& name, OmWString& core, OmWString& vers, const OmWString& filename, bool isfile = true, bool us2spc = true);


/// \brief Parse Mod file name
///
/// Parse Mod identity, core name, version string and displayed name from file name.
///
/// \param[out] filename  : File name to parse
/// \param[out] iden      : Parsed Mod identity
/// \param[out] core      : Parsed Mod core name
/// \param[in]  vers      : Parsed Mod version string
/// \param[in]  name      : Parsed and formated Mod displayed name
///
/// \return True if version string candidate was found, false otherwise
///
bool Om_parseModFilename(const OmWString& filename, OmWString* iden, OmWString* core, OmWString* vers, OmWString* name);


/// \brief Parse Mod directory name
///
/// Parse Mod identity, core name, version string and displayed name from directory name.
///
/// \param[out] dirname   : Directory name to parse
/// \param[out] iden      : Parsed Mod identity
/// \param[out] core      : Parsed Mod core name
/// \param[in]  vers      : Parsed Mod version string
/// \param[in]  name      : Parsed and formated Mod displayed name
///
/// \return True if version string candidate was found, false otherwise
///
bool Om_parseModDirname(const OmWString& dirname, OmWString* iden, OmWString* core, OmWString* vers, OmWString* name);


/// \brief Parse Mod identity
///
/// Parse Mod core name, version string and displayed name and from directory name.
///
/// \param[out] iden      : Mod identity to parse
/// \param[out] core      : Parsed Mod core name
/// \param[in]  vers      : Parsed Mod version string
/// \param[in]  name      : Parsed and formated Mod displayed name
///
/// \return True if version string candidate was found, false otherwise
///
bool Om_parseModIdent(const OmWString& iden, OmWString* core, OmWString* vers, OmWString* name);

#endif // OMUTILPKG_H
