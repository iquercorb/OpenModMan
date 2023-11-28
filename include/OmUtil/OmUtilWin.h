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
#ifndef OMUTILWIN_H
#define OMUTILWIN_H

#include "OmBase.h"
#include "OmBaseWin.h"

/// \brief Get Windows error string
///
/// Returns the error string corresponding to the given Windows error code.
///
/// \param[in] code    : Windows error code.
///
/// \return Error string.
///
OmWString Om_getErrorStr(int code);

/// \brief Load stock shell icon
///
/// Loads and returns the specified Windows Shell Stock icon.
///
/// \param[in] id      : Stock icon id, form SHSTOCKICONID enum
/// \param[in] large   : If true, load the large size icon version
///
/// \return Icon handle (HICON) of the specified Shell Stock icon
///
HICON Om_getShellIcon(unsigned id, bool large = false);

/// \brief Load stock shell icon as HBITMAP
///
/// Loads and returns the specified Windows Shell Stock icon as HBITMAP.
///
/// \param[in] id      : Stock icon id, form SHSTOCKICONID enum
/// \param[in] large   : If true, load the large size icon version
///
/// \return Bitmap handle (HBITMAP) of the specified Shell Stock icon
///
HBITMAP Om_loadShellBitmap(unsigned id, bool large = false);

/// \brief Get internal resource image
///
/// Return internal resource image as HBITMAP, created handle does not
/// have to be deleted.
///
/// \param[in] hins    : Handle instance to get internal resource.
/// \param[in] id      : Image internal resource id.
///
/// \return Bitmap handle (HBITMAP) of the internal image or nullptr.
///
HBITMAP Om_getResImage(HINSTANCE hins, unsigned id);

/// \brief Get internal resource icon
///
/// Return internal resource icon as HICON, created handle does not
/// have to be deleted.
///
/// \param[in] hins    : Handle instance to get internal resource.
/// \param[in] id      : Icon internal resource id.
/// \param[in] size    : Icon size to get.
///
/// \return Bitmap handle (HBITMAP) of the internal image or nullptr.
///
HICON Om_getResIcon(HINSTANCE hins, unsigned id, unsigned size = 0);

/// \brief Create font object
///
/// Create and returns a font object according the specified parameters.
///
/// \param[in] pt      : Font size
/// \param[in] weight  : Font weight
/// \param[in] style   : Font style
/// \param[in] name    : Font name
///
/// \return Font handle (HFONT) of the created font
///
HFONT Om_createFont(unsigned pt, unsigned weight, const wchar_t* name);

void Om_getAppIconInfos(const OmWString& path, int16_t res_id);
void Om_getAppIconImage(const OmWString& path);

#endif // OMUTILWIN_H
