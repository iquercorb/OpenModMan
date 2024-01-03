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
HBITMAP Om_getResImage(unsigned id, HINSTANCE hins)
{
  unsigned db_id = id - RES_IDB_BASE;

  if(__internal_bmp[db_id] == nullptr) {
    HINSTANCE hInstance = hins ? hins : GetModuleHandle(nullptr);
    HBITMAP hBm = static_cast<HBITMAP>(LoadImageW(hInstance,MAKEINTRESOURCEW(id),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION));
    __internal_bmp[db_id] = hBm;
  }

  return __internal_bmp[db_id];
}


/// \brief Loaded internal image
///
/// Array of loaded internal resource image.
///
static HBITMAP __internal_pabmp[200] = {nullptr};


/// Base index for image resource
///
#define RES_IDB_BASE   800

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_getResImagePremult(unsigned id, HINSTANCE hins)
{
  unsigned db_id = id - RES_IDB_BASE;
  if(__internal_pabmp[db_id] == nullptr) {

    HINSTANCE hInstance = hins ? hins : GetModuleHandle(nullptr);
    HBITMAP hBm = static_cast<HBITMAP>(LoadImageW(hInstance,MAKEINTRESOURCEW(id),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION));

    uint8_t* bgra = nullptr;
    HBITMAP hBmPa = (HBITMAP)CopyImage(hBm, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    // delete source Bitmap
    DeleteObject(hBm);

    // convert pixel data to premultiplied alpha
    HDC hDc = CreateCompatibleDC(nullptr);
    BITMAPINFO BmInfo = {}; BmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    // 1. get Bitmap Info
    GetDIBits(hDc, hBmPa, 0, 0, nullptr, &BmInfo, DIB_RGB_COLORS);
    bgra = static_cast<uint8_t*>(Om_alloc(BmInfo.bmiHeader.biSizeImage));
    GetDIBits(hDc, hBmPa, 0, BmInfo.bmiHeader.biHeight, bgra, &BmInfo, DIB_RGB_COLORS);

    // treat only 32 bpp images
    if(BmInfo.bmiHeader.biBitCount == 32) {
      if(bgra) {
        // Convert to premultiplied alpha
        for(size_t i = 0; i < BmInfo.bmiHeader.biSizeImage; i += 4) {
          bgra[i+0] = static_cast<uint8_t>(bgra[i+0] * bgra[i+3] / 255);
          bgra[i+1] = static_cast<uint8_t>(bgra[i+1] * bgra[i+3] / 255);
          bgra[i+2] = static_cast<uint8_t>(bgra[i+2] * bgra[i+3] / 255);
        }
        // 3. set replace pixels data into Bitmap
        SetDIBits(hDc, hBmPa, 0, BmInfo.bmiHeader.biHeight, bgra, &BmInfo, DIB_RGB_COLORS);
      }
    }

    // free pixel buffer
    if(bgra) free(bgra);

    ReleaseDC(nullptr, hDc);
    __internal_pabmp[db_id] = hBmPa;
  }

  return __internal_pabmp[db_id];
}

/// \brief Loaded internal icon
///
/// Array of loaded internal resource icon.
///
static HICON __internal_ico[100][3] = {nullptr};

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HICON Om_getResIcon(unsigned id, unsigned size, HINSTANCE hins)
{
  unsigned db_id = id - RES_IDB_BASE;

  unsigned w = (size == 0) ? 16 : (size == 1) ? 24 : 32;

  if(__internal_ico[db_id][size] == nullptr) {
    HINSTANCE hInstance = hins ? hins : GetModuleHandle(nullptr);
    HICON hIc = static_cast<HICON>(LoadImage(hInstance,MAKEINTRESOURCE(id),IMAGE_ICON,w,w,0));
    __internal_ico[db_id][size] = hIc;
  }

  return __internal_ico[db_id][size];
}


/// \brief Loaded internal icon
///
/// Array of loaded internal resource icon.
///
static HBITMAP __internal_paico[100][3] = {nullptr};


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_getResIconPremult(unsigned id, unsigned size, HINSTANCE hins)
{
  unsigned db_id = id - RES_IDB_BASE;

  unsigned w = (size == 0) ? 16 : (size == 1) ? 24 : 32;

  if(__internal_paico[db_id][size] == nullptr) {
    HINSTANCE hInstance = hins ? hins : GetModuleHandle(nullptr);
    HICON hIc = static_cast<HICON>(LoadImage(hInstance,MAKEINTRESOURCE(id),IMAGE_ICON,w,w,0));
    ICONINFO IconInfo;
    GetIconInfo(hIc, &IconInfo);

    uint8_t* bgra = nullptr;
    uint8_t* mask = nullptr;

    HBITMAP hBmRgb = (HBITMAP)CopyImage(IconInfo.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    HBITMAP hBmMsk = (HBITMAP)CopyImage(IconInfo.hbmMask, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    DeleteObject(hIc);
    // convert pixel data to premultiplied alpha
    HDC hDc = CreateCompatibleDC(nullptr);
    BITMAPINFO BmInfo = {}; BmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    // 1. get Bitmap Info
    GetDIBits(hDc, hBmRgb, 0, 0, nullptr, &BmInfo, DIB_RGB_COLORS);
    bgra = static_cast<uint8_t*>(Om_alloc(BmInfo.bmiHeader.biSizeImage));
    GetDIBits(hDc, hBmRgb, 0, BmInfo.bmiHeader.biHeight, bgra, &BmInfo, DIB_RGB_COLORS);

    // 1. get Bitmap Info
    GetDIBits(hDc, hBmMsk, 0, 0, nullptr, &BmInfo, DIB_RGB_COLORS);
    mask = static_cast<uint8_t*>(Om_alloc(BmInfo.bmiHeader.biSizeImage));
    GetDIBits(hDc, hBmMsk, 0, BmInfo.bmiHeader.biHeight, mask, &BmInfo, DIB_RGB_COLORS);

    // treat only 32 bpp images
    if(BmInfo.bmiHeader.biBitCount == 32) {
      if(bgra && mask) {
        // Convert to premultiplied alpha
        for(size_t i = 0; i < BmInfo.bmiHeader.biSizeImage; i += 4) {
          bgra[i+3] = (bgra[i+3] ^ mask[i]) ? bgra[i+3] : ~mask[i];
          bgra[i+0] = static_cast<uint8_t>(bgra[i+0] * bgra[i+3] / 255);
          bgra[i+1] = static_cast<uint8_t>(bgra[i+1] * bgra[i+3] / 255);
          bgra[i+2] = static_cast<uint8_t>(bgra[i+2] * bgra[i+3] / 255);
        }
        // 3. set replace pixels data into Bitmap
        SetDIBits(hDc, hBmRgb, 0, BmInfo.bmiHeader.biHeight, bgra, &BmInfo, DIB_RGB_COLORS);
      }
    }

    // free pixel buffer
    if(bgra) Om_free(bgra);
    if(mask) Om_free(mask);

    // free unused mask buffer
    DeleteObject(hBmMsk);

    ReleaseDC(nullptr, hDc);
    __internal_paico[db_id][size] = hBmRgb;
  }

  return __internal_paico[db_id][size];
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
   DWORD   dwBytesInRes;         // how many bytes in this resource?
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
   GRPICONDIRENTRY   idEntries[1]; // The entries for each image
} GRPICONDIR, *LPGRPICONDIR;
#pragma pack( pop )
/*
#include <list>

typedef std::list<ICONDIRENTRY> IconDirectory;

IconDirectory GetIconDirectory( HMODULE hMod, WORD Id )
{
    HRSRC hRsrc = FindResource(hMod, MAKEINTRESOURCE(Id), RT_GROUP_ICON);
    HGLOBAL hGlobal = LoadResource(hMod, hRsrc);
    ICONDIR* lpGrpIconDir = (ICONDIR*)LockResource(hGlobal);

    IconDirectory dir;
    for ( size_t i = 0; i < lpGrpIconDir->idCount; ++i ) {
        dir.push_back( lpGrpIconDir->idEntries[i] );
    }
    return dir;
}

HICON LoadSpecificIcon( HMODULE hMod, WORD Id )
{
    HRSRC hRsrc = FindResource( hMod, MAKEINTRESOURCE( Id ), RT_ICON );
    HGLOBAL hGlobal = LoadResource( hMod, hRsrc );
    BYTE* lpData = (BYTE*)LockResource( hGlobal );
    DWORD dwSize = SizeofResource( hMod, hRsrc );

    HICON hIcon = CreateIconFromResourceEx( lpData, dwSize, TRUE, 0x00030000,
                                            0, 0, LR_DEFAULTCOLOR );
    return hIcon;
}

void PrintIconDirEntry( const ICONDIRENTRY& DirEntry ) {
    _wprintf_p( L"ID: %04d; width=%02d; height=%02d; bpp=%02d\n",
                DirEntry.nID,
                DirEntry.bWidth, DirEntry.bHeight, DirEntry.wBitCount );
}

void PrintIconInfo( HICON hIcon ) {
    ICONINFO ii = {};
    GetIconInfo( hIcon, &ii );
    _wprintf_p( L"xHotspot=%02d; yHotspot=%02d\n", ii.xHotspot, ii.yHotspot );
}

typedef std::list<ICONDIRENTRY>::const_iterator IconDirectoryCIt;


void Om_getAppIconInfos(const OmWString& path, int16_t res_id)
{
  HMODULE hMod = LoadLibraryExW( path.c_str(), nullptr, LOAD_LIBRARY_AS_IMAGE_RESOURCE );

  IconDirectory dir = GetIconDirectory( hMod, res_id );

  for(IconDirectoryCIt it = dir.begin(); it != dir.end(); ++it ) {
    PrintIconDirEntry( *it );
    HICON hIcon = LoadSpecificIcon( hMod, it->nID );
    PrintIconInfo( hIcon );
    DestroyIcon( hIcon );
  }
}
*/
static uint64_t __Enumresname_type = 0;
static uint64_t __Enumresname_id = 0;
static wchar_t  __Enumresname_name[OM_MAX_ITEM];

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
BOOL WINAPI Enumresnameprocw(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
{
  OM_UNUSED(hModule); OM_UNUSED(lParam);

  __Enumresname_type = reinterpret_cast<int64_t>(lpType);
  __Enumresname_id = reinterpret_cast<int64_t>(lpName);

  if(!IS_INTRESOURCE(lpName)) {
    wcscpy(__Enumresname_name, lpName);
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_getAppIconImage(const OmWString& path, int32_t size)
{
  HMODULE hMod = LoadLibraryExW( path.c_str(), nullptr, LOAD_LIBRARY_AS_IMAGE_RESOURCE );

  __Enumresname_id = 0;
  __Enumresname_type = 0;
  __Enumresname_name[0] = 0;

  EnumResourceNamesW(hMod, (LPCWSTR)RT_GROUP_ICON, Enumresnameprocw, 0);

  if(__Enumresname_type) {

    HRSRC hRsrc;
    HGLOBAL hGlobal;

    if(IS_INTRESOURCE(__Enumresname_id)) {
      hRsrc = FindResourceW(hMod, MAKEINTRESOURCEW(__Enumresname_id), (LPCWSTR)RT_GROUP_ICON);
    } else {
      hRsrc = FindResourceW(hMod, __Enumresname_name, (LPCWSTR)RT_GROUP_ICON);
    }

    hGlobal = LoadResource(hMod, hRsrc);
    GRPICONDIR* GrpIconDir = static_cast<GRPICONDIR*>(LockResource(hGlobal));

    // select icon with the size closest to the requested size
    WORD s = 0;

    for(size_t i = 1; i < GrpIconDir->idCount; ++i) {

      int32_t s_with = GrpIconDir->idEntries[s].bWidth;
      int32_t i_with = GrpIconDir->idEntries[i].bWidth;

      if(abs(size - s_with) > abs(size - i_with))
        s = i;
    }

    hRsrc = FindResource(hMod, MAKEINTRESOURCE(GrpIconDir->idEntries[s].nID), RT_ICON);
    hGlobal = LoadResource(hMod, hRsrc);
    BYTE* lpData = (BYTE*)LockResource(hGlobal);
    DWORD dwSize = SizeofResource(hMod, hRsrc);

    HICON hIcon = CreateIconFromResourceEx(lpData,dwSize,true,0x00030000,0,0,LR_DEFAULTCOLOR);

    ICONINFO IconInfo;
    GetIconInfo(hIcon, &IconInfo);

    HBITMAP hBmp = (HBITMAP)CopyImage(IconInfo.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    return hBmp;
  }

  return nullptr;
}
