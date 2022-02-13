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
#include "OmBase.h"           //< string, vector, Om_alloc, OMM_MAX_PATH, etc.
#include "OmBaseWin.h"        //< WinAPI

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_getErrorStr(int code) {

  wchar_t num_buf[32];
  swprintf(num_buf, 32, L"%x", code);

  wstring ret = L"(0x"; ret.append(num_buf); ret.append(L") ");

  switch(code)
  {
  case ERROR_FILE_NOT_FOUND: ret.append(L"FILE_NOT_FOUND"); break;
  case ERROR_PATH_NOT_FOUND: ret.append(L"PATH_NOT_FOUND"); break;
  case ERROR_TOO_MANY_OPEN_FILES: ret.append(L"TOO_MANY_OPEN_FILES"); break;
  case ERROR_ACCESS_DENIED: ret.append(L"ACCESS_DENIED"); break;
  case ERROR_ARENA_TRASHED: ret.append(L"ARENA_TRASHED"); break;
  case ERROR_NOT_ENOUGH_MEMORY: ret.append(L"NOT_ENOUGH_MEMORY"); break;
  case ERROR_INVALID_BLOCK: ret.append(L"INVALID_BLOCK"); break;
  case ERROR_INVALID_ACCESS: ret.append(L"INVALID_ACCESS"); break;
  case ERROR_INVALID_DATA: ret.append(L"INVALID_DATA"); break;
  case ERROR_OUTOFMEMORY: ret.append(L"OUTOFMEMORY"); break;
  case ERROR_INVALID_DRIVE: ret.append(L"INVALID_DRIVE"); break;
  case ERROR_CURRENT_DIRECTORY: ret.append(L"CURRENT_DIRECTORY"); break;
  case ERROR_NOT_SAME_DEVICE: ret.append(L"NOT_SAME_DEVICE"); break;
  case ERROR_WRITE_PROTECT: ret.append(L"WRITE_PROTECT"); break;
  case ERROR_CRC: ret.append(L"ERROR_CRC"); break;
  case ERROR_SEEK: ret.append(L"ERROR_SEEK"); break;
  case ERROR_WRITE_FAULT: ret.append(L"WRITE_FAULT"); break;
  case ERROR_READ_FAULT: ret.append(L"READ_FAULT"); break;
  case ERROR_SHARING_VIOLATION: ret.append(L"SHARING_VIOLATION"); break;
  case ERROR_LOCK_VIOLATION: ret.append(L"LOCK_VIOLATION"); break;
  case ERROR_WRONG_DISK: ret.append(L"WRONG_DISK"); break;
  case ERROR_HANDLE_DISK_FULL: ret.append(L"HANDLE_DISK_FULL"); break;
  case ERROR_FILE_EXISTS: ret.append(L"FILE_EXISTS"); break;
  case ERROR_DRIVE_LOCKED: ret.append(L"DRIVE_LOCKED"); break;
  case ERROR_OPEN_FAILED: ret.append(L"OPEN_FAILED"); break;
  case ERROR_BUFFER_OVERFLOW: ret.append(L"BUFFER_OVERFLOW"); break;
  case ERROR_DISK_FULL: ret.append(L"DISK_FULL"); break;
  case ERROR_INVALID_NAME: ret.append(L"INVALID_NAME"); break;
  case ERROR_DIR_NOT_EMPTY: ret.append(L"DIR_NOT_EMPTY"); break;
  case ERROR_ALREADY_EXISTS: ret.append(L"ALREADY_EXISTS"); break;
  }

  return ret;
}

/// \brief Loaded shell large icons
///
/// Array of loaded shell large icons
///
static HICON __shell_large_icon[100] = {nullptr};

/// \brief Loaded shell small icons
///
/// Array of loaded shell small icons
///
static HICON __shell_small_icon[100] = {nullptr};

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HICON Om_getShellIcon(unsigned id, bool large)
{
  if(large) {
    if(__shell_large_icon[id])
      return __shell_large_icon[id];
  } else {
    if(__shell_small_icon[id])
      return __shell_small_icon[id];
  }

  SHSTOCKICONINFO sIi = {};
  sIi.cbSize = sizeof(SHSTOCKICONINFO);

  SHGetStockIconInfo(static_cast<SHSTOCKICONID>(id),
                    (large) ? SHGSI_ICON|SHGSI_LARGEICON : SHGSI_ICON|SHGSI_SMALLICON,
                    &sIi);

  if(large) {
    __shell_large_icon[id] = sIi.hIcon;
  } else {
    __shell_small_icon[id] = sIi.hIcon;
  }

  return sIi.hIcon;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_loadShellBitmap(unsigned id, bool large)
{
  SHSTOCKICONINFO sIi = {};
  sIi.cbSize = sizeof(SHSTOCKICONINFO);

  SHGetStockIconInfo(static_cast<SHSTOCKICONID>(id),
                    (large) ? SHGSI_ICON|SHGSI_LARGEICON : SHGSI_ICON|SHGSI_SMALLICON,
                    &sIi);

  ICONINFO iCi = {};
  GetIconInfo(sIi.hIcon, &iCi);
  HBITMAP result = static_cast<HBITMAP>(CopyImage(iCi.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_COPYDELETEORG));
  DestroyIcon(sIi.hIcon);

  return result;
}

/// \brief Loaded internal image
///
/// Array of loaded internal resource image.
///
static HBITMAP __internal_bmp[200] = {nullptr};


/// Base index for image resource
///
#define RES_IDB_BASE   800

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_getResImage(HINSTANCE hins, unsigned id)
{
  unsigned db_id = id - RES_IDB_BASE;

  if(__internal_bmp[db_id] == nullptr) {
    HBITMAP hBm = static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(id),IMAGE_BITMAP,0,0,0));
    __internal_bmp[db_id] = hBm;
  }

  return __internal_bmp[db_id];
}


/// \brief Loaded internal icon
///
/// Array of loaded internal resource icon.
///
static HICON __internal_ico[100][3] = {nullptr};


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HICON Om_getResIcon(HINSTANCE hins, unsigned id, unsigned size)
{
  unsigned db_id = id - RES_IDB_BASE;

  unsigned w = (size == 0) ? 16 : (size == 1) ? 24 : 32;

  if(__internal_ico[db_id][size] == nullptr) {
    HICON hIc = static_cast<HICON>(LoadImage(hins,MAKEINTRESOURCE(id),IMAGE_ICON,w,w,0));
    __internal_ico[db_id][size] = hIc;
  }

  return __internal_ico[db_id][size];
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HFONT Om_createFont(unsigned pt, unsigned weight, const wchar_t* name)
{
  return CreateFontW( pt, 0, 0, 0, weight,
                      false, false, false,
                      ANSI_CHARSET,
                      OUT_TT_PRECIS, 0, CLEARTYPE_QUALITY, 0,
                      name);
}
