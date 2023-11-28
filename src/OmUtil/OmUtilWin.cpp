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
#include "OmBase.h"           //< string, vector, Om_alloc, OM_MAX_PATH, etc.
#include "OmBaseWin.h"        //< WinAPI

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_getErrorStr(int code) {

  wchar_t num_buf[32];
  swprintf(num_buf, 32, L"%x", code);

  OmWString ret = L"(0x"; ret.append(num_buf); ret.append(L") ");

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
  // PRE Win-32 error code that may be returned by SHFileOperation
  case 0x71 /*DE_SAMEFILE*/: ret.append(L"SAME_FILE"); break;
  case 0x72 /*DE_MANYSRC1DEST*/: ret.append(L"MANY_SRC_1_DEST"); break;
  case 0x73 /*DE_DIFFDIR*/: ret.append(L"DIFF_DIR"); break;
  case 0x74 /*DE_ROOTDIR*/: ret.append(L"ROOT_DIR"); break;
  case 0x75 /*DE_OPCANCELLED*/: ret.append(L"OP_CANCELLED"); break;
  case 0x76 /*DE_DESTSUBTREE*/: ret.append(L"DEST_SUBTREE"); break;
  case 0x78 /*DE_ACCESSDENIEDSRC*/: ret.append(L"ACCESS_DENIED_SRC"); break;
  case 0x79 /*DE_PATHTOODEEP*/: ret.append(L"PATH_TOO_DEEP"); break;
  case 0x7A /*DE_MANYDEST*/: ret.append(L"MANY_DEST"); break;
  case 0x7C /*DE_INVALIDFILES*/: ret.append(L"INVALID_FILES"); break;
  case 0x7D /*DE_DESTSAMETREE*/: ret.append(L"DEST_SAME_TREE"); break;
  case 0x7E /*DE_FLDDESTISFILE*/: ret.append(L"FLD_DEST_IS_FILE"); break;
  case 0x80 /*DE_FILEDESTISFLD*/: ret.append(L"FILE_DEST_IS_FLD"); break;
  case 0x81 /*DE_FILENAMETOOLONG*/: ret.append(L"FILENAME_TOO_LONG"); break;
  case 0x82 /*DE_DEST_IS_CDROM*/: ret.append(L"DEST_IS_CDROM"); break;
  case 0x83 /*DE_DEST_IS_DVD*/: ret.append(L"DEST_IS_DVD"); break;
  case 0x84 /*DE_DEST_IS_CDRECORD*/: ret.append(L"DEST_IS_CDRECORD"); break;
  case 0x85 /*DE_FILE_TOO_LARGE*/: ret.append(L"FILE_TOO_LARGE"); break;
  case 0x86 /*DE_SRC_IS_CDROM*/: ret.append(L"SRC_IS_CDROM"); break;
  case 0x87 /*DE_SRC_IS_DVD*/: ret.append(L"SRC_IS_DVD"); break;
  case 0x88 /*DE_SRC_IS_CDRECORD*/: ret.append(L"SRC_IS_CDRECORD"); break;
  //case 0xB7 /*DE_ERROR_MAX*/: ret.append(L"ERROR_MAX"); break; //< same as ALREADY_EXISTS
  case 0x402 /* Unknown error */: ret.append(L"UNKNOW_ERROR"); break;
  case 0x10000 /*ERRORONDEST*/: ret.append(L"ERROR_ON_DEST"); break;
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

#include <list>

#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
    BYTE   bWidth;               // Width, in pixels, of the image
    BYTE   bHeight;              // Height, in pixels, of the image
    BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
    BYTE   bReserved;            // Reserved
    WORD   wPlanes;              // Color Planes
    WORD   wBitCount;            // Bits per pixel
    DWORD  dwBytesInRes;         // how many bytes in this resource?
    WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;
#pragma pack( pop )

#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
    WORD            idReserved;   // Reserved (must be 0)
    WORD            idType;       // Resource type (1 for icons)
    WORD            idCount;      // How many images?
    GRPICONDIRENTRY idEntries[1]; // The entries for each image
} GRPICONDIR, *LPGRPICONDIR;
#pragma pack( pop )

typedef std::list<GRPICONDIRENTRY> IconDirectory;

IconDirectory GetIconDirectory( HMODULE hMod, WORD Id ) {
    HRSRC hRsrc = FindResource( hMod, MAKEINTRESOURCE( Id ), RT_GROUP_ICON );
    HGLOBAL hGlobal = LoadResource( hMod, hRsrc );
    GRPICONDIR* lpGrpIconDir = (GRPICONDIR*)LockResource( hGlobal );

    IconDirectory dir;
    for ( size_t i = 0; i < lpGrpIconDir->idCount; ++i ) {
        dir.push_back( lpGrpIconDir->idEntries[ i ] );
    }
    return dir;
}

HICON LoadSpecificIcon( HMODULE hMod, WORD Id ) {
    HRSRC hRsrc = FindResource( hMod, MAKEINTRESOURCE( Id ), RT_ICON );
    HGLOBAL hGlobal = LoadResource( hMod, hRsrc );
    BYTE* lpData = (BYTE*)LockResource( hGlobal );
    DWORD dwSize = SizeofResource( hMod, hRsrc );

    HICON hIcon = CreateIconFromResourceEx( lpData, dwSize, TRUE, 0x00030000,
                                            0, 0, LR_DEFAULTCOLOR );
    return hIcon;
}
void PrintIconDirEntry( const GRPICONDIRENTRY& DirEntry ) {
    _wprintf_p( L"ID: %04d; width=%02d; height=%02d; bpp=%02d\n",
                DirEntry.nID,
                DirEntry.bWidth, DirEntry.bHeight, DirEntry.wBitCount );
}

void PrintIconInfo( HICON hIcon ) {
    ICONINFO ii = { 0 };
    GetIconInfo( hIcon, &ii );
    _wprintf_p( L"xHotspot=%02d; yHotspot=%02d\n", ii.xHotspot, ii.yHotspot );
}

typedef std::list<GRPICONDIRENTRY>::const_iterator IconDirectoryCIt;


void Om_getAppIconInfos(const OmWString& path, int16_t res_id)
{
  HMODULE hMod = LoadLibraryExW( path.c_str(), nullptr, LOAD_LIBRARY_AS_IMAGE_RESOURCE );
  IconDirectory dir = GetIconDirectory( hMod, res_id );
  for( IconDirectoryCIt it = dir.begin(); it != dir.end(); ++it ) {
    PrintIconDirEntry( *it );
    HICON hIcon = LoadSpecificIcon( hMod, it->nID );
    PrintIconInfo( hIcon );
    DestroyIcon( hIcon );
  }
}

BOOL Enumresnameprocw(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
{
  std::wcout << L"DEBUG => Enumresnameprocw - lpType: " << std::to_wstring((int64_t)lpType) << L"    lpName: " << std::to_wstring((int64_t)lpName) << L"\n";
}

void Om_getAppIconImage(const OmWString& path)
{
  HMODULE hMod = LoadLibraryExW( path.c_str(), nullptr, LOAD_LIBRARY_AS_IMAGE_RESOURCE );
  //HMODULE hMod = LoadLibraryExW( path.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE );

  EnumResourceNamesW(hMod, (LPCWSTR)RT_ICON, Enumresnameprocw, 0);
  //LoadImageW(hMod, MAKEINTRESOURCE(3), IMAGE_ICON, 0, 0, 0);
}
