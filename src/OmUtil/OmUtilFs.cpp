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
#include <ShlwApi.h>          //< PathIsDirectoryEmptyW
#include <ShlObj.h>           //< SHCreateDirectoryExW

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isDirEmpty(const OmWString& path) {
  return PathIsDirectoryEmptyW(path.c_str());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_dirCreate(const OmWString& path) {
  if(!CreateDirectoryW(path.c_str(), nullptr)) {
    return GetLastError();
  }
  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_dirCreateRecursive(const OmWString& path)
{
  return SHCreateDirectoryExW(nullptr, path.c_str(), nullptr);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_dirDelete(const OmWString& path) {
  if(!RemoveDirectoryW(path.c_str())) {
    return GetLastError();
  }
  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_fileCopy(const OmWString& src, const OmWString& dst, bool ow = true) {
  if(!ow) {
    if(GetFileAttributesW(dst.c_str()) != INVALID_FILE_ATTRIBUTES)
      return 0; /* we do not write, but this is not a error */
  }
  if(!CopyFileW(src.c_str(),dst.c_str(), false)) {
    return GetLastError();
  }
  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_fileMove(const OmWString& src, const OmWString& dst) {
  if(!MoveFileExW(src.c_str(),dst.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH)) {
    return GetLastError();
  }
  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_fileDelete(const OmWString& path){
  if(!DeleteFileW(path.c_str())) {
    return GetLastError();
  }
  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isFile(const OmWString& path) {
  DWORD attr = GetFileAttributesW(path.c_str());
  if(attr != INVALID_FILE_ATTRIBUTES)
    return !(attr&FILE_ATTRIBUTE_DIRECTORY);
  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isDir(const OmWString& path) {
  DWORD attr = GetFileAttributesW(path.c_str());
  if(attr != INVALID_FILE_ATTRIBUTES)
    return (attr&FILE_ATTRIBUTE_DIRECTORY);
  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_pathIsNetwork(const OmWString& path) {
  return PathIsNetworkPathW(path.c_str());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_pathExists(const OmWString& path) {
  return PathFileExistsW(path.c_str());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_itemSetAttr(const OmWString& path, uint32_t attr) {
  return SetFileAttributesW(path.c_str(), attr);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_itemHasAttr(const OmWString& path, uint32_t mask) {
  DWORD attr = GetFileAttributesW(path.c_str());
  if(attr != INVALID_FILE_ATTRIBUTES)
    return ((mask & attr) == mask);
  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_dirDeleteRecursive(const OmWString& path)
{
  wchar_t path_buf[OM_MAX_PATH];

  wcscpy(path_buf, path.c_str());
  path_buf[path.size()+1] = 0; // the buffer must end with double null character

  SHFILEOPSTRUCTW fop = {};
  fop.pFrom = path_buf;
  fop.wFunc = FO_DELETE;
  fop.fFlags = FOF_NO_UI;

  int result = SHFileOperationW(&fop);

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isFileZip(const OmWString& path) {

  // Microsoft functions are ugly (this is not new) but they are proven to be
  // the most efficient for file I/O... maybe because they are directly sticked
  // to the kernel at low level...
  HANDLE hFile = CreateFileW(path.c_str(),
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             nullptr,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return false;

  // We have to seek over the file until we found something else
  // that zero, because a zip file can begin with a blank space before the
  // signature...

  char buf[1024]; // <- our read buffer
  unsigned* sign; // <- our future 4 bytes signature
  DWORD r = 0; //< count of bytes read

  do {
    SetFilePointer(hFile, r, nullptr, FILE_CURRENT);
    ReadFile(hFile, &buf, 1024, &r, nullptr);
    for(unsigned i = 0; i < r; ++i) {
      // check for something else than zero
      if(buf[i] != 0) {
        // we got something, we don't go further we close the file
        // and check the result.
        CloseHandle(hFile);
        sign = (unsigned*)&buf[i]; //< cast our buffer in unsigned pointer
        return ( *sign == 0x04034b50 );
      }
    }
  } while(r == 1024);

  CloseHandle(hFile);
  return false; // PKWARE Zip file signature
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsDir(OmWStringArray* ls, const OmWString& orig, bool absolute, bool hidden)
{
  OmWString item;

  OmWString srch(orig);
  srch += L"\\*";

  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
        // skip this and parent folder
        if(!wcscmp(fd.cFileName, L".")) continue;
        if(!wcscmp(fd.cFileName, L"..")) continue;

        // skip in case we do not include hidden items
        if(!hidden && (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN))
          continue;

        if(absolute) {
          item = orig; item += L"\\"; item += fd.cFileName;
          ls->push_back(item);
        } else {
          ls->push_back(fd.cFileName);
        }
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsFile(OmWStringArray* ls, const OmWString& orig, bool absolute, bool hidden)
{
  OmWString item;

  OmWString srch(orig);
  srch += L"\\*";

  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) {

        // skip in case we do not include hidden items
        if(!hidden && (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN))
          continue;

        if(absolute) {
          item = orig; item += L"\\"; item += fd.cFileName;
          ls->push_back(item);
        } else {
          ls->push_back(fd.cFileName);
        }
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}

/// \brief List files recursively
///
/// This is the private function used to list files recursively.
///
/// \param[out] ls      : Pointer to array of OmWString to be filled with result.
/// \param[in]  orig    : Path where to list items from.
/// \param[in]  from    : Path to prepend to result to obtain the item full
///                       path from the beginning of the tree exploration.
/// \param[in]  hidden  : Include items marked as Hidden.
///
static void __lsFile_Recurse(OmWStringArray* ls, const OmWString& orig, const OmWString& from, bool hidden)
{
  OmWString item;
  OmWString root;

  OmWString srch(orig);
  srch += L"\\*";

  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

      // skip in case we do not include hidden items
      if(!hidden && (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN))
        continue;

      if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        item = from; item += L"\\"; item += fd.cFileName;
        root = orig; root += L"\\"; root += fd.cFileName;
        // go deep in tree
        __lsFile_Recurse(ls, root, item, hidden);

      } else {
        item = from; item += L"\\"; item += fd.cFileName;
        ls->push_back(item);
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsFileRecursive(OmWStringArray* ls, const OmWString& origin, bool absolute, bool hidden)
{
  if(absolute) {
    __lsFile_Recurse(ls, origin.c_str(), origin.c_str(), hidden);
  } else {
    __lsFile_Recurse(ls, origin.c_str(), L"", hidden);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsFileFiltered(OmWStringArray* ls, const OmWString& orig, const OmWString& filter, bool absolute, bool hidden)
{
  OmWString item;
  OmWString root;

  OmWString srch(orig);
  srch += L"\\" + filter;

  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) {

        // skip in case we do not include hidden items
        if(!hidden && (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN))
          continue;

        if(absolute) {
          item = orig; item += L"\\"; item += fd.cFileName;
          ls->push_back(item);
        } else {
          ls->push_back(fd.cFileName);
        }
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsAll(OmWStringArray* ls, const OmWString& orig, bool absolute, bool hidden)
{
  OmWString item;

  OmWString srch(orig); srch += L"\\*";
  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

      // skip in case we do not include hidden items
      if(!hidden && (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN))
        continue;

      if(absolute) {
        item = orig; item += L"\\"; item += fd.cFileName;
        ls->push_back(item);
      } else {
        ls->push_back(fd.cFileName);
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}

/// \brief List files and folders recursively
///
/// This is the private function used to list files and folder recursively.
///
/// \param[out] ls      : Pointer to array of OmWString to be filled with result.
/// \param[in]  orig    : Path where to list items from.
/// \param[in]  from    : Path to prepend to result to obtain the item full
///                       path from the beginning of the tree exploration.
///
static void __lsAll_Recurse(OmWStringArray* ls, const OmWString& orig, const OmWString& from, bool hidden)
{
  OmWString item;
  OmWString root;

  OmWString srch(orig);
  srch += L"\\*";

  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

      // skip in case we do not include hidden items
      if(!hidden && (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN))
        continue;

      item = from; item += L"\\"; item += fd.cFileName;

      if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        ls->push_back(item);
        // go deep in tree
        root = orig; root += L"\\"; root += fd.cFileName;
        __lsAll_Recurse(ls, root, item, hidden);
      } else {
        ls->push_back(item);
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_lsAllRecursive(OmWStringArray* ls, const OmWString& origin, bool absolute, bool hidden)
{
  if(absolute) {
    __lsAll_Recurse(ls, origin.c_str(), origin.c_str(), hidden);
  } else {
    __lsAll_Recurse(ls, origin.c_str(), L"", hidden);
  }
}

/// \brief Compute folder size recursively
///
/// Private function to calculate the total size of folder content including
/// all it sub-folders. The function act recursively.
///
/// \param[out] size    : Pointer to variable to be incremented as result.
/// \param[in]  orig    : Path of folder to get total size (start of recursive
///                       exploration).
///
void __folderSize(uint64_t* size, const OmWString& orig)
{
  OmWString root;

  LARGE_INTEGER FileSize;

  OmWString srch(orig); srch += L"\\*";
  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

      if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        root = orig; root += L"\\"; root += fd.cFileName;

        // go deep in tree
        __folderSize(size, root);

      } else {
        root = orig; root += L"\\"; root += fd.cFileName;
        HANDLE hFile = CreateFileW(root.c_str(),
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   nullptr,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   nullptr);

        GetFileSizeEx(hFile, &FileSize);
        *size += FileSize.QuadPart;

        CloseHandle(hFile);
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t Om_itemSize(const OmWString& path)
{
  uint64_t ret = 0;

  if(Om_isFile(path)) {
    LARGE_INTEGER FileSize;
    HANDLE hFile = CreateFileW(path.c_str(),
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               nullptr,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               nullptr);

    GetFileSizeEx(hFile, &FileSize);
    CloseHandle(hFile);
    ret = FileSize.QuadPart;

  } else {
    ret = 0;
    __folderSize(&ret, path);
  }

  return ret;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
time_t Om_itemTime(const OmWString& path)
{
  HANDLE hFile = CreateFileW(path.c_str(),
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             nullptr,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return 0;

  FILETIME fTime;
  GetFileTime(hFile, nullptr, nullptr, &fTime);
  CloseHandle(hFile);

  ULARGE_INTEGER ull;
  ull.LowPart = fTime.dwLowDateTime;
  ull.HighPart = fTime.dwHighDateTime;

  return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_moveToTrash(const OmWString& path)
{
  wchar_t path_buf[OM_MAX_PATH];

  wcscpy(path_buf, path.c_str());
  path_buf[path.size()+1] = 0;

  SHFILEOPSTRUCTW fop = {};
  fop.pFrom = path_buf;
  fop.wFunc = FO_DELETE;
  fop.fFlags = FOF_NO_UI|FOF_ALLOWUNDO;

  int result = SHFileOperationW(&fop);

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_checkAccess(const OmWString& path, unsigned mask)
{
  // Thanks to this article for giving some clues :
  // http://blog.aaronballman.com/2011/08/how-to-check-access-rights/

  // STEP 1 - retrieve the "security descriptor" (i.e owner, group, access
  // rights, etc. ) of the specified file or folder.
  SECURITY_DESCRIPTOR* pSd;
  DWORD sdSize;
  // here is the mask for file permission informations we want to retrieve it
  // seem to be the minimum required for an access check request
  DWORD sdMask =  OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
                | DACL_SECURITY_INFORMATION;
  // first call to get required SECURITY_DESCRIPTOR size
  GetFileSecurityW(path.c_str(), sdMask, nullptr, 0, &sdSize);
  // allocate new SECURITY_DESCRIPTOR of the proper size
  pSd = reinterpret_cast<SECURITY_DESCRIPTOR*>(Om_alloc(sdSize + 1));
  // second call to get SECURITY_DESCRIPTOR data
  if(!GetFileSecurityW(path.c_str(), sdMask, pSd, sdSize, &sdSize)) {
    Om_free(pSd);
    #ifdef DEBUG
    std::wcout << "DEBUG => Om_checkAccess :: GetFileSecurityW failed\n";
    #endif
    return false;
  }

  // STEP 2 - creates a "security token" of the current application process
  //to be checked against the file or folder "security descriptor"
  DWORD daMask =  TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE
                | STANDARD_RIGHTS_READ;
  HANDLE hTokenProc = nullptr;
  if(!OpenProcessToken(GetCurrentProcess(), daMask, &hTokenProc)) {
    Om_free(pSd);
    #ifdef DEBUG
    std::wcout << "DEBUG => Om_checkAccess :: OpenProcessToken failed\n";
    #endif
    return false;
  }
  // the current process token is a "primary" one (don't know what that mean)
  // so we need to duplicate it to transform it into a standard "user" token by
  // impersonate it...
  HANDLE hTokenUser = nullptr;
  if(!DuplicateToken(hTokenProc, SecurityImpersonation, &hTokenUser)) {
    CloseHandle(hTokenProc); Om_free(pSd);
    #ifdef DEBUG
    std::wcout << "DEBUG => Om_checkAccess :: DuplicateToken failed\n";
    #endif
    return false;
  }

  // STEP 3 - Finally check if "security token" have the requested
  // "mask" access to the "security descriptor" of the specified file
  // or folder

  // the GENERIC_MAPPING seem to be never used in most common scenarios,
  // we set it here because the parameter is mandatory.
  GENERIC_MAPPING gm = {GENERIC_READ,GENERIC_WRITE,GENERIC_EXECUTE,GENERIC_ALL};
  PRIVILEGE_SET ps = {};
  DWORD psSize = sizeof(PRIVILEGE_SET);
  DWORD allowed = 0;      //< mask of allowed access
  BOOL  status = false;   //< access status according supplied GENERIC_MAPPING
  if(!AccessCheck(pSd, hTokenUser, mask, &gm, &ps, &psSize, &allowed, &status)) {
    CloseHandle(hTokenProc); CloseHandle(hTokenUser); Om_free(pSd);
    #ifdef DEBUG
    std::wcout << L"DEBUG => Om_checkAccess :: AccessCheck failed: " << GetLastError() << L"\n";
    #endif
  }

  if(!status) {
    #ifdef DEBUG
    std::wcout << "DEBUG => Om_checkAccess(";
    if(mask & FILE_LIST_DIRECTORY) std::wcout << "FILE_LIST_DIRECTORY | ";
    if(mask & FILE_TRAVERSE) std::wcout << "FILE_TRAVERSE | ";
    if(mask & FILE_ADD_FILE) std::wcout << "FILE_ADD_FILE | ";
    if(mask & FILE_ADD_SUBDIRECTORY) std::wcout << "FILE_ADD_SUBDIRECTORY | ";
    if(mask & FILE_READ_DATA) std::wcout << "FILE_READ_DATA | ";
    if(mask & FILE_WRITE_DATA) std::wcout << "FILE_WRITE_DATA | ";
    if(mask & FILE_APPEND_DATA) std::wcout << "FILE_APPEND_DATA | ";
    if(mask & FILE_EXECUTE) std::wcout << "FILE_EXECUTE";
    if(mask & FILE_READ_ATTRIBUTES) std::wcout << "FILE_READ_ATTRIBUTES";
    if(mask & FILE_WRITE_ATTRIBUTES) std::wcout << "FILE_WRITE_ATTRIBUTES";
    std::wcout << ") : denied, allowed access: \n";
    AccessCheck(pSd, hTokenUser, FILE_READ_DATA, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  FILE_READ_DATA + LIST_DIRECTORY\n";
    AccessCheck(pSd, hTokenUser, FILE_WRITE_DATA, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  FILE_WRITE_DATA + ADD_FILE\n";
    AccessCheck(pSd, hTokenUser, FILE_APPEND_DATA, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  FILE_APPEND_DATA + ADD_SUBDIRECTORY\n";
    AccessCheck(pSd, hTokenUser, FILE_READ_EA, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  FILE_READ_EA\n";
    AccessCheck(pSd, hTokenUser, FILE_WRITE_EA, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  FILE_WRITE_EA\n";
    AccessCheck(pSd, hTokenUser, FILE_EXECUTE, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  FILE_EXECUTE + TRAVERSE\n";
    AccessCheck(pSd, hTokenUser, FILE_DELETE_CHILD, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  FILE_DELETE_CHILD\n";
    AccessCheck(pSd, hTokenUser, FILE_READ_ATTRIBUTES, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  FILE_READ_ATTRIBUTES\n";
    AccessCheck(pSd, hTokenUser, FILE_WRITE_ATTRIBUTES, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  FILE_WRITE_ATTRIBUTES\n";
    AccessCheck(pSd, hTokenUser, DELETE, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  DELETE\n";
    AccessCheck(pSd, hTokenUser, READ_CONTROL, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  READ_CONTROL\n";
    AccessCheck(pSd, hTokenUser, WRITE_DAC, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  WRITE_DAC\n";
    AccessCheck(pSd, hTokenUser, WRITE_OWNER, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  WRITE_OWNER\n";
    AccessCheck(pSd, hTokenUser, SYNCHRONIZE, &gm, &ps, &psSize, &allowed, &status);
    std::wcout << ((status) ? "[x]" : "[ ]") << "  SYNCHRONIZE\n";
    #endif
    status = 0;
  }

  CloseHandle(hTokenProc);
  CloseHandle(hTokenUser);
  Om_free(pSd);

  return status;
}

/// \brief Load plain text.
///
/// Loads content of the specified file as plain-text into the given
/// string object.
///
/// \param[in] pstr    : Pointer to string to receive loaded data.
/// \param[in] path    : Path to text file to be loaded.
///
/// \return Count of bytes read.
///
inline static size_t __load_plaintxt(OmCString* pstr, const wchar_t* path)
{
  HANDLE hFile = CreateFileW( path, GENERIC_READ, FILE_SHARE_READ,
                              nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return 0;

  DWORD rb;
  size_t rt = 0;
  char cbuf[4097];

  while(ReadFile(hFile, cbuf, 4096, &rb, nullptr)) {

    if(rb == 0)
      break;

    rt += rb;

    pstr->append(cbuf, rb);
  }

  CloseHandle(hFile);

  return rt;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmCString Om_loadPlainText(const OmWString& path)
{
  OmCString result;
  __load_plaintxt(&result, path.c_str());
  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_loadPlainText(OmCString* text, const OmWString& path)
{
  return __load_plaintxt(text, path.c_str());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_loadBinary(uint64_t* size, const OmWString& path)
{
  // initialize size
  (*size) = 0;

  // open file for reading
  HANDLE hFile = CreateFileW( path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                              nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return nullptr;

  LARGE_INTEGER FileSize;
  GetFileSizeEx(hFile, &FileSize);
  uint64_t data_size = FileSize.QuadPart;

  // allocate buffer and read
  uint8_t* data = reinterpret_cast<uint8_t*>(Om_alloc(data_size));
  if(!data) return nullptr;

  // read full data at once
  DWORD rb;
  bool result = ReadFile(hFile, data, data_size, &rb, nullptr);

  // close file
  CloseHandle(hFile);

  if(!result) {
    Om_free(data);
    return nullptr;
  }

  (*size) = data_size;

  return data;
}
