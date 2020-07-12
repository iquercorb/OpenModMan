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

#ifndef OMGLOBAL_H
#define OMGLOBAL_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <wctype.h>

#define NTDDI_VERSION             0x06000000    // NTDDI_VISTA
#define WINVER                    0x0600        // _WIN32_WINNT_VISTA

#include <Windows.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <Gdiplus.h>
#include <Uxtheme.h>
#include <shellapi.h>

#define OMM_APP_NAME              L"Open Mod Manager"
#define OMM_APP_SHORT_NAME        L"OpenModMan"

#define OMM_APP_MAJ               0
#define OMM_APP_MIN               9
#define OMM_APP_REV               1
#ifdef _WIN64
  #define OMM_APP_ARCH            L"x64"
#else
  #define OMM_APP_ARCH            L"x86"
#endif
#define OMM_APP_DATE              L"July 2020"
#define OMM_APP_AUTHOR            L"Eric M. \"Sedenion\""
#define OMM_APP_CONTRIB           L""

#define OMM_CFG_SIGN_APP          L"Open_Mod_Manager_Main"
#define OMM_CFG_SIGN_CTX          L"Open_Mod_Manager_Context"
#define OMM_CFG_SIGN_LOC          L"Open_Mod_Manager_Location"
#define OMM_CFG_SIGN_PKG          L"Open_Mod_Manager_Package"
#define OMM_CFG_SIGN_BCK          L"Open_Mod_Manager_Backup"
#define OMM_CFG_SIGN_BAT          L"Open_Mod_Manager_Batch"

#define OMM_PKG_FILES_FILTER      L"Package File (*.zip,*.ozp)\0*.ZIP;*.OZP;\0"
#define OMM_BCK_FILES_FILTER      L"Backup File (*.zip,*.ozb)\0*.ZIP;*.OZB;\0"

#define OMM_CTX_DEF_FILE_FILER    L"Context definition (*.omc)\0*.OMC;\0"
#define OMM_LOC_DEF_FILE_FILER    L"Location definition (*.oml)\0*.OML;\0"
#define OMM_PKG_DEF_FILE_FILER    L"Package definition (*.omp)\0*.OMP;\0"
#define OMM_BCK_DEF_FILE_FILER    L"Backup definition (*.omk)\0*.OMK;\0"
#define OMM_BAT_DEF_FILE_FILER    L"Batch definition (*.omb)\0*.OMB;\0"

#define OMM_CTX_FILE_EXT          L"omc"
#define OMM_LOC_FILE_EXT          L"oml"
#define OMM_PKG_FILE_EXT          L"omp"
#define OMM_BCK_FILE_EXT          L"omk"
#define OMM_BAT_FILE_EXT          L"omb"

#define OMM_PKG_THMB_SIZE         128

#define OMM_MAX_PATH              1024

// This enables a "Slow Mode" for debug purposes
//#define DEBUG_SLOW          300         //< Sleep time (milliseconds)

using namespace std;

/// \brief Compute Hash.
///
/// Calculates and returns 64 bits unsigned integer hash (xxHash) of the given
/// data chunk.
///
/// \param[in]  data    : Data to compute Hash.
/// \param[in]  size    : Size of data in bytes.
///
/// \return Resulting 64 bits unsigned integer hash.
///
uint64_t Om_getXXHash3(const void* data, size_t size);

/// \brief Compute Hash.
///
/// Calculates and returns 64 bits unsigned integer hash (xxHash) of the given
/// wide string.
///
/// \param[in]  data    : Wide string to compute Hash.
///
/// \return Resulting 64 bits unsigned integer hash.
///
uint64_t Om_getXXHash3(const wstring& str);

/// \brief Calculate CRC64 value.
///
/// Calculates and returns the CRC64 unsigned integer value of the given
/// data chunk.
///
/// \param[in]  data    : Data chunk to create CRC from.
/// \param[in]  size    : Size of data.
///
/// \return Resulting CRC64 unsigned integer.
///
uint64_t Om_getCRC64(const void* data, size_t size);

/// \brief Calculate CRC64 value.
///
/// Calculates and returns the CRC64 unsigned integer value of the given
/// wide string.
///
/// \param[in]  str     : Wide string to create CRC from.
///
/// \return Resulting CRC64 unsigned integer.
///
uint64_t Om_getCRC64(const wstring& str);

/// \brief Get string representation of a 64 bits integer.
///
/// Returns the hexadecimal representation of the given 64 bits unsigned
/// integer value as a wide string.
///
/// \param[in]  num     : 64 bits unsigned integer.
///
/// \return Hexadecimal string representation of 64 bits integer
///
inline wstring Om_toHexString(uint64_t num) {
  wchar_t wcbuf[32];
  swprintf(wcbuf, 32, L"%llx", num);
  return wstring(wcbuf);
}

/// \brief Get string representation of a 64 bits integer.
///
/// Set the specified wind string to the hexadecimal representation of the
/// given 64 bits unsigned integer value.
///
/// \param[out] str     : String to get result.
/// \param[in]  num     : 64 bits unsigned integer.
///
inline void Om_toHexString(wstring& str, uint64_t num) {
  wchar_t wcbuf[32];
  swprintf(wcbuf, 32, L"%llx", num);
  str = wcbuf;
}

/// \brief Get 64 bits integer from string.
///
/// Returns the 64 bits unsigned integer value of the given hexadecimal number
/// string representation.
///
/// \param[in]  str     : Hexadecimal string represented number.
///
/// \return Converted 64 bits integer value.
///
inline uint64_t Om_toUint64(const wstring& str) {
  return wcstoull(str.c_str(), nullptr, 16);
}

/// \brief Generate random bytes.
///
/// Generate a random bytes sequence with values from 0 to 255 of
/// the desired length.
///
/// \param[out] dest    : Buffer to receive randomly generated sequence
/// \param[in]  size    : Buffer length
///
void Om_getRandBytes(uint8_t* dest, size_t size);

/// \brief Generate UUID.
///
/// Generate a random generated UUID version 4 string.
///
/// \return 36 characters UUID version 4 string.
///
wstring Om_genUUID();

/// \brief Wide string to multibyte string conversion.
///
/// Convert the given wide string into a multibyte string.
///
/// \param[in]  wcs     : Input wide string to convert.
///
/// \return Multibyte string conversion result.
///
string Om_toMbString(const wstring& wcs);

/// \brief Wide string to multibyte string conversion.
///
/// Convert the given wide string into a multibyte string.
///
/// \param[out] mbs     : Multibyte string to get conversion result.
/// \param[in]  wcs     : Input wide string to convert.
///
void Om_toMbString(string& mbs, const wstring& wcs);

/// \brief Multibyte string to wide string conversion.
///
/// Convert the given multibyte string into a wide string.
///
/// \param[in]  mbs     : Input multibyte string to convert.
///
/// \return Wide string conversion result.
///
wstring Om_toWcString(const string& mbs);

/// \brief Multibyte string to wide string conversion.
///
/// Convert the given multibyte string into a wide string.
///
/// \param[out] wcs     : Wide string to get conversion result.
/// \param[in]  mbs     : Input multibyte string to convert.
///
void Om_toWcString(wstring& wcs, const string& mbs);

/// \brief Convert path to Zip CDR entry
///
/// Convert the given standard path string into a properly formated Zip's
/// Central Directory Record entry.
///
/// \param[in]  path    : Path string to convert.
///
/// \return Suitable Zip CDR entry string.
///
wstring Om_toZipPath(const wstring& path);

/// \brief Convert path to Zip CDR entry
///
/// Convert the given standard path string into a properly formated Zip's
/// Central Directory Record entry.
///
/// \param[out] zcdr    : String to get conversion result.
/// \param[in]  path    : Path string to convert.
///
void Om_toZipPath(wstring& zcdr, const wstring& path);

/// \brief Convert Zip CDR entry to path
///
/// Convert the given Zip Central Directory Record entry string into
/// a standard Windows path format.
///
/// \param[in]  zcdr    : Zip CDR entry to convert.
///
/// \return Resulting Windows path version.
///
string Om_fromZipPath(const string& zcdr);

/// \brief Convert Zip CDR entry to path
///
/// Convert the given Zip Central Directory Record entry string into
/// a standard Windows path format.
///
/// \param[out] path    : String to get conversion result.
/// \param[in]  zcdr    : Zip CDR entry to convert.
///
void Om_fromZipPath(string& path, const string& zcdr);

/// \brief Sort strings
///
/// Sorts the given array of strings in alphanumeric order.
///
/// \param[in]  string  : Pointer to vector array of wstring to sort.
///
void Om_sortStrings(vector<wstring>* strings);

/// \brief Check file name or path validity
///
/// Checks whether the given string is suitable for file name/path or
/// contains illegal character(s).
///
/// \param[in]  name    : File name or path to check.
///
/// \return True if the given string is suitable for file name or path,
///         false otherwise
///
bool Om_isValidName(const wchar_t* name);

/// \brief Check file name or path validity
///
/// Checks whether the given string is suitable for Windows file name/path or
/// contains illegal character(s).
///
/// \param[in]  name    : File name or path to check.
///
/// \return True if the given string is suitable for file name or path,
///         false otherwise
///
bool Om_isValidName(const wstring& name);

/// \brief Extract file extension from path
///
/// Extract and returns the file extension part of the given full file path
/// or file name.
///
/// \param[in]  path    : Windows formated file path or name.
///
/// \return Extracted extension without the separating dot.
///
inline wstring Om_getExtensionPart(const wstring& path) {
  size_t d = path.find_last_of(L'.') + 1;
  return path.substr(d, wstring::npos);
}

/// \brief Extract file part from path
///
/// Extracts and returns the path part that follows the last
/// separator (backslash). Assuming the given path is a full file
/// path, the file name with its extension will be extracted.
///
/// \param[in]  path    : Windows formated full path.
///
/// \return Extracted path part.
///
inline wstring Om_getFilePart(const wstring& path){
  size_t s = path.find_last_of(L'\\') + 1;
  return path.substr(s, wstring::npos);
}

/// \brief Extract directory part from path
///
/// Extracts and returns the path part that precede the last
/// separator (backslash), including the last separator. Assuming the given
/// path is a full file path, the full path without the file name will be
/// extracted.
///
/// \param[in]  path    : Windows formated file path.
///
/// \return Extracted path part.
///
inline wstring Om_getDirPart(const wstring& uri){
  size_t e = uri.find_last_of(L'\\');
  return uri.substr(0, e);
}

/** Get file name (without extension) part of an URI.
 *
 * \param[in] uri     URI to extract name from.
 *
 * \return Extracted name of the given URI.
 */

/// \brief Extract file name without extension from path
///
/// Extracts and returns the file name, without its extension, from
/// the given full path.
///
/// \param[in]  path    : Windows formated file path.
///
/// \return Extracted file name without extension.
///
inline wstring Om_getNamePart(const wstring& uri){
  size_t s = uri.find_last_of(L'\\') + 1;
  size_t e = uri.find_last_of(L'.');
  return uri.substr(s, e-s);
}

/// \brief Check if names matches
///
/// Checks whether two strings are equals in case insensitive way.
///
/// \param[in]  left    : First string to test.
/// \param[in]  right   : Second string to test.
///
/// \return True if strings are same despite unmatched case, false otherwise.
///
inline bool Om_namesMatches(const wstring& left, const wstring& right)
{
  if(left.size() != right.size())
    return false;

  for(unsigned i = 0; i < left.size(); ++i) {
    if(towupper(left[i]) != towupper(right[i]))
      return false;
  }

  return true;
}

/// \brief Check if names matches
///
/// Checks whether two strings are equals in case insensitive way.
///
/// \param[in]  left    : First string to test.
/// \param[in]  right   : Second string to test.
///
/// \return True if strings are same despite unmatched case, false otherwise.
///
inline bool Om_namesMatches(const wstring& left, const wchar_t* right)
{
  if(left.size() != wcslen(right))
    return false;

  for(unsigned i = 0; i < left.size(); ++i) {
    if(towupper(left[i]) != towupper(right[i]))
      return false;
  }

  return true;
}

/// \brief Check if for file name extension matches
///
/// Checks whether the given file has the specified extension.
///
/// \param[in]  file    : file name or full path to test extension.
/// \param[in]  ext     : Three or four letters extension to check for.
///
/// \return True if given filename has the requested extension, false otherwise.
///
inline bool Om_extensionMatches(const wstring& file, const wchar_t* ext)
{
  size_t d = file.find_last_of(L'.') + 1;
  if(d > 0) {
    return (0 == file.compare(d, -1, ext));
  } else {
    return false;
  }
}

/// \brief Concatenate paths
///
/// Concatenates two paths, adding separator if necessary.
///
/// \param[in]  left    : Left path part to concatenate.
/// \param[in]  right   : Right path part to concatenate.
///
/// \return Result of the concatenation.
///
inline wstring Om_concatPaths(const wstring& left, const wstring& right) {

  wstring result;
  if(left.empty()) {
    result = right;
  } else {
    result = left;
    if(left.back() != L'\\' && right.front() != L'\\') result += L"\\";
    result += right;
  }
  return result;
}

/// \brief Concatenate paths
///
/// Concatenates two paths, adding separator if necessary.
///
/// \param[out] conc    : String to be set as the result of concatenation.
/// \param[in]  left    : Left path part to concatenate.
/// \param[in]  right   : Right path part to concatenate.
///
inline void Om_concatPaths(wstring& conc, const wstring& left, const wstring& right) {

  if(left.empty()) {
    conc = right;
  } else {
    conc = left;
    if(left.back() != L'\\' && right.front() != L'\\') conc += L"\\";
    conc += right;
  }
}

/// \brief Compare path roots
///
/// Checks whether the given path has the specified root.
///
/// \param[in]  root    : Root path for verification.
/// \param[in]  item    : Path to verify if it has the specified root.
///
/// \return True if path actually has the specified root, false otherwise.
///
inline bool Om_isRootOfPath(const wstring& root, const wstring& path) {

  size_t l = root.size();

  if(l > path.size())
    return false;

  if(path.compare(0,l,root) == 0) {
    return (path[l] == L'\\'); //< verify this is a folder and not a file
  }

  return false;
}

/// \brief Get relative part of path
///
/// Checks whether the given path has the specified root, then extract its
/// relative part.
///
/// \param[out] rel     : String to be set as the extracted relative path.
/// \param[in]  root    : Root of path used to check and extract relative part.
/// \param[in]  item    : Path to be checked and to get the relative part from.
///
/// \return True if path actually has the specified root and operation succeed,
///         false otherwise.
///
inline bool Om_getRelativePath(wstring& rel, const wstring& root, const wstring& path) {

  size_t l = root.size();

  if(l > path.size())
    return false;

  if(path.compare(0,l,root) == 0) {
    if(path[l] == L'\\') { //< verify this is a folder
      l++;
      if(path.size() > l) {
        rel = path.substr(l, -1);
        return true;
      }
    }
  }

  return false;
}

/// \brief Get formated bytes/octets size string
///
/// Create a formated string of the given size in bytes in human readable
/// form with proper unit conversion.
///
/// \param[in]  bytes   : Size in bytes.
/// \param[in]  octet   : Write string using O for octet instead of B for bytes.
///
/// \return Formated string describing size.
///
wstring Om_sizeString(size_t bytes, bool octet = false);

/// \brief Check whether is version string
///
/// Checks whether the given string can be parsed as valid version number(s).
///
/// \param[in] str       : String to test.
///
/// \return True if the given string can be parsed as version number(s).
///
bool Om_isVersionStr(const wstring& str);

/// \brief Parse Package filename
///
/// Parse the Packag display name and potential version substring from its
/// file name.
///
/// \param[out] name      : Parsed display name.
/// \param[out] vers      : Parsed version if any.
/// \param[in]  filename  : Filename to be parsed.
/// \param[in]  isfile    : Specify whether filename is file or a folder name.
/// \param[in]  us2spc    : Specify whether underscores must be replaced by spaces.
///
/// \return True if version string candidate was found, false otherwise
///
bool Om_parsePkgIdent(wstring& name, wstring& vers, const wstring& filename, bool isfile = true, bool us2spc = true);

/// \brief Check empty folder
///
/// Checks whether the specified folder is empty.
///
/// \param[in]  path   : Path to folder to check.
///
/// \return True if the specified folder is empty, false otherwise.
///
inline bool Om_isDirEmpty(const wstring& path) {
  return PathIsDirectoryEmptyW(path.c_str());
}

/// \brief Create folder
///
/// Creates the specified folder.
///
/// \param[in]  path   : Path of folder to create.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
inline int Om_dirCreate(const wstring& path) {
  if(!CreateDirectoryW(path.c_str(), nullptr)) {
    return GetLastError();
  }
  return 0;
}


/// \brief Create folder recursively
///
/// Creates the specified folder and its parent tree if needed.
///
/// \param[in]  path   : Path of folder(s) to create
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
inline int Om_dirCreateRecursive(const wstring& path)
{
  return SHCreateDirectoryExW(nullptr, path.c_str(), nullptr);
}

/// \brief Delete folder
///
/// Deletes the specified folder.
///
/// \param[in]  path   : Path of folder to delete.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
inline int Om_dirDelete(const wstring& path) {
  if(!RemoveDirectoryW(path.c_str())) {
    return GetLastError();
  }
  return 0;
}

/// \brief Delete folder recursively
///
/// Deletes the specified folder and all its content recursively.
///
/// \param[in]  path   : Path of folder to delete.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
inline int Om_dirDeleteRecursive(const wstring& path)
{
  wchar_t wcbuf[512];
  wcscpy(wcbuf, path.c_str());
  wcbuf[path.size()+1] = 0; // the buffer must end with double null character

  SHFILEOPSTRUCTW fop = {};
  fop.pFrom = wcbuf;
  fop.wFunc = FO_DELETE;
  fop.fFlags = FOF_NO_UI;

  return SHFileOperationW(&fop);
}

/// \brief Copy file
///
/// Copy the given file to the specified location.
///
/// \param[in]  src    : Source file path to copy.
/// \param[in]  dst    : Destination file path.
/// \param[in]  ow     : Overwrite destination if exists.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
inline int Om_fileCopy(const wstring& src, const wstring& dst, bool ow = true) {
  if(!ow) {
    if(GetFileAttributesW(dst.c_str()) != INVALID_FILE_ATTRIBUTES)
      return 0; /* we do not write, but this is not a error */
  }
  if(!CopyFileW(src.c_str(),dst.c_str(), false)) {
    return GetLastError();
  }
  return 0;
}


/// \brief Copy file
///
/// Copy the given file to the specified location.
///
/// \param[in]  src    : Source file path to copy.
/// \param[in]  dst    : Destination file path.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
inline int Om_fileMove(const wstring& src, const wstring& dst) {
  if(!MoveFileW(src.c_str(),dst.c_str())) {
    return GetLastError();
  }
  return 0;
}


/// \brief Delete file
///
/// Delete the specified file.
///
/// \param[in]  path   : Path to file to delete.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
inline int Om_fileDelete(const wstring& path){
  if(!DeleteFileW(path.c_str())) {
    return GetLastError();
  }
  return 0;
}

/// \brief Check valid file
///
/// Checks whether the specified item is actually a valid file.
///
/// \param[in]  path   : Path to item to check.
///
/// \return True if item is actually a file, false if it does not exists or
///         is a folder.
///
inline bool Om_isFile(const wstring& path) {
  DWORD attr = GetFileAttributesW(path.c_str());
  if(attr != INVALID_FILE_ATTRIBUTES)
    return !(attr&FILE_ATTRIBUTE_DIRECTORY);
  return false;
}

/// \brief Check valid folder
///
/// Checks whether the specified item is actually a valid folder.
///
/// \param[in]  path   : Path to item to check.
///
/// \return True if item is actually a folder, false if it does not exists or
///         is a file.
///
inline bool Om_isDir(const wstring& path) {
  DWORD attr = GetFileAttributesW(path.c_str());
  if(attr != INVALID_FILE_ATTRIBUTES)
    return (attr&FILE_ATTRIBUTE_DIRECTORY);
  return false;
}

/// \brief Check existing item
///
/// Checks whether the specified item exists, either as file or folder.
///
/// \param[in]  path   : Path to item to check.
///
/// \return True if item is actually a file or folder, false otherwise.
///
inline bool Om_pathExists(const wstring& path) {
  return PathFileExistsW(path.c_str());
}

/// \brief Check valid Zip file
///
/// Checks whether the specified item is file with Zip signature.
///
/// \param[in]  path   : Path to item to check.
///
/// \return True if item is actually a file with Zip signature, false otherwise.
///
bool Om_isFileZip(const wstring& path);

/// \brief List folders
///
/// Retrieves the list of folders contained in the specified origin location.
///
/// \param[out] ls      : Pointer to array of wstring to be filled with result.
/// \param[in]  origin  : Path where to list folder from.
/// \param[in]  abs     : If true, returns folder absolute path instead of
///                       folder name alone.
///
void Om_lsDir(vector<wstring>* ls, const wstring& origin, bool abs = true);

/// \brief List files
///
/// Retrieves the list of files contained in the specified origin location.
///
/// \param[out] ls      : Pointer to array of wstring to be filled with result.
/// \param[in]  origin  : Path where to list files from.
/// \param[in]  abs     : If true, returns files absolute path instead of
///                       files name alone.
///
void Om_lsFile(vector<wstring>* ls, const wstring& origin, bool abs = true);

/// \brief List files recursively
///
/// Retrieves the list of files contained in the specified origin
/// location, exploring sub-folders recursively.
///
/// \param[out] ls      : Pointer to array of wstring to be filled with result.
/// \param[in]  origin  : Path where to list items from.
/// \param[in]  abs     : If true, returns items absolute path instead of
///                       items name alone.
///
void Om_lsFileRecursive(vector<wstring>* ls, const wstring& origin, bool abs = true);

/// \brief List files with custom filter
///
/// Retrieves the list of files contained in the specified origin location using
/// the supplied custom filter.
///
/// \param[out] ls      : Pointer to array of wstring to be filled with result.
/// \param[in]  origin  : Path where to list files from.
/// \param[in]  filter  : Custom filter to select files.
/// \param[in]  abs     : If true, returns files absolute path instead of
///                       files name alone.
///
void Om_lsFileFiltered(vector<wstring>* ls, const wstring& origin, const wstring& filter, bool abs = true);

/// \brief List files and folders
///
/// Retrieves the list of files and folders contained in the specified origin
/// location.
///
/// \param[out] ls      : Pointer to array of wstring to be filled with result.
/// \param[in]  origin  : Path where to list items from.
/// \param[in]  abs     : If true, returns items absolute path instead of
///                       items name alone.
///
void Om_lsAll(vector<wstring>* ls, const wstring& origin, bool abs = true);

/// \brief List files and folders recursively
///
/// Retrieves the list of files and folders contained in the specified origin
/// location, exploring sub-folders recursively.
///
/// \param[out] ls      : Pointer to array of wstring to be filled with result.
/// \param[in]  origin  : Path where to list items from.
/// \param[in]  abs     : If true, returns items absolute path instead of
///                       items name alone.
///
void Om_lsAllRecursive(vector<wstring>* ls, const wstring& origin, bool abs = true);

/// \brief Get item total size
///
/// Retrieves the total size of the specified file or folder, including
/// all its content recursively.
///
/// \param[in]  path    : Item path.
///
/// \return Total size in bytes of the specified item
///
size_t Om_itemSize(const wstring& path);

/// \brief Get item last time
///
/// Retrieves the last write time of the specified file or folder.
///
/// \param[in]  path    : Item path.
///
/// \return Item last write time.
///
time_t Om_itemTime(const wstring& path);

/// \brief Move to trash
///
/// Moves the specified item to trash.
///
/// \param[in]  path    : Item path.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
int Om_moveToTrash(const wstring& path);

/// \brief Check read access
///
/// Checks whether application have read access to the specified
/// file or folder.
///
/// \param[in]  path    : Item path.
///
/// \return True if application have read access, false otherwise.
///
bool Om_checkAccessRead(const wstring& path);

/// \brief Check write access
///
/// Checks whether application have write access to the specified
/// file or folder.
///
/// \param[in]  path    : Item path.
///
/// \return True if application have write access, false otherwise.
///
bool Om_checkAccessWrite(const wstring& path);

/// \brief Check read and write access
///
/// Checks whether application have both read and write access to
/// the specified file or folder.
///
/// \param[in]  path    : Item path.
///
/// \return True if application have both read and write access, false otherwise.
///
bool Om_checkAccessReadWrite(const wstring& path);

/// \brief Error message box.
///
/// Displays a standard Windows error message dialog box.
///
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  header  : Formated message header/title (not the dialog title).
/// \param[in]  detail  : Formated message details paragraph.
///
void Om_dialogBoxErr(HWND hWnd, const wstring& header, const wstring& detail);

/// \brief Warning message box.
///
/// Displays a standard Windows warning message dialog box.
///
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  header  : Formated message header/title (not the dialog title).
/// \param[in]  detail  : Formated message details paragraph.
///
void Om_dialogBoxWarn(HWND hWnd, const wstring& header, const wstring& detail);

/// \brief Notice message box.
///
/// Displays a standard Windows notice message dialog box.
///
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  header  : Formated message header/title (not the dialog title).
/// \param[in]  detail  : Formated message details paragraph.
///
void Om_dialogBoxInfo(HWND hWnd, const wstring& header, const wstring& detail);

/// \brief Question dialog box.
///
/// Displays a standard Windows question dialog box with YES/NO buttons.
///
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  header  : Formated message header/title (not the dialog title).
/// \param[in]  detail  : Formated message details paragraph.
///
/// \return True if user clicked on the YES button, false otherwise.
///
bool Om_dialogBoxQuerry(HWND hWnd, const wstring& header, const wstring& detail);

/// \brief Question warning dialog box.
///
/// Displays a standard Windows question with warning dialog box with
/// YES/NO buttons.
///
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  header  : Formated message header/title (not the dialog title).
/// \param[in]  detail  : Formated message details paragraph.
///
/// \return True if user clicked on the YES button, false otherwise.
///
bool Om_dialogBoxQuerryWarn(HWND hWnd, const wstring& header, const wstring& detail);

/// \brief Select folder dialog box.
///
/// Opens a folder selection dialog box.
///
/// \param[out] result  : Buffer to receive the selected folder path.
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  title   : Dialog window title.
/// \param[in]  start   : Path to default location where to begin browsing.
/// \param[in]  captive : If true, the default start location is set as root
///                       and user cannot browse up to parent folder.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dialogBrowseDir(wchar_t* result, HWND hWnd, const wchar_t* title, const wchar_t* start, bool captive = false);

/// \brief Open file dialog box.
///
/// Opens a file open selection dialog box.
///
/// \param[out] result  : Buffer to receive the selected folder path.
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  title   : Dialog window title.
/// \param[in]  filter  : File type and extension filter string.
/// \param[in]  start   : Path to default location where to begin browsing.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dialogOpenFile(wchar_t* result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const wchar_t* start);

/// \brief Save file dialog box.
///
/// Opens a file save as selection dialog box.
///
/// \param[out] result  : Buffer to receive the selected folder path.
/// \param[in]  hwnd    : Parent window handle or nullptr to ignore.
/// \param[in]  title   : Dialog window title.
/// \param[in]  filter  : File type and extension filter string.
/// \param[in]  start   : Path to default location where to begin browsing.
///
/// \return True if succeed, false if user canceled or an error occurred.
///
bool Om_dialogSaveFile(wchar_t* result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const wchar_t* start);

/// \brief Load plan text.
///
/// Loads the specified file as plain text file.
///
/// \param[in] path    : Source file path to open.
///
/// \return String of file content
///
string Om_loadPlainText(const wstring& path);

/// \brief Save as BMP file.
///
/// Create a new BMP file at the specified location with the given pixel data.
///
/// \param[in]  path    : File full path and name to save.
/// \param[in]  w       : Image width in pixel.
/// \param[in]  h       : Image depth in pixel.
/// \param[in]  d       : Image bits per pixel, either 24 or 32 are supported.
/// \param[in]  pixels  : Image pixels data.
///
/// \return True if succeed, false if write error occurred.
///
bool Om_saveBitmap(const wstring& path, unsigned w, unsigned h, unsigned d, const unsigned char* pixels);

/// \brief Save as BMP file.
///
/// Create a new BMP file at the specified location from the specified
/// HBITMAP object handle object.
///
/// \param[in] path    : File full path and name to save.
/// \param[in] hbmp    : WinAPI HBITMAP object handle to save as BMP file.
///
/// \return True if succeed, false if write error occurred.
///
bool Om_saveBitmap(const wstring& path, void* hbmp);

/// \brief Load image as HBITMAP.
///
/// Loads the specified image file as a WinAPI bitmap HBITMAP handle. The
/// supported image formats are JPEG, TIFF, BMP, GIF and PNG.
///
/// \param[in] path    : Source file path to open (jpeg, tiff, bmp, gif, png).
/// \param[in] width   : Image desired width or 0 to keep original size.
/// \param[in] height  : Image desired height or 0 to keep original size.
/// \param[in] aspect  : If true, the image original aspect ratio is conserved.
///
/// \return Pointer to be casted as HBITMAP object handle.
///
HBITMAP Om_loadBitmap(const wstring& path, unsigned width = 0, unsigned height = 0, bool aspect = true);

/// \brief Load buffer as HBITMAP.
///
/// Loads the specified image data as a WinAPI bitmap HBITMAP handle. The
/// supported image formats are JPEG, TIFF, BMP, GIF and PNG.
///
/// \param[in] data    : Source data to convert (jpeg, tiff, bmp, gif, png).
/// \param[in] size    : Size of source data in bytes.
/// \param[in] width   : Image desired width or 0 to keep original size.
/// \param[in] height  : Image desired height or 0 to keep original size.
/// \param[in] aspect  : If true, the image original aspect ratio is conserved.
///
/// \return Pointer to be casted as HBITMAP object handle.
///
HBITMAP Om_loadBitmap(const void* data, size_t size, unsigned width = 0, unsigned height = 0, bool aspect = true);

/// \brief Get bitmap thumbnail
///
/// Returns resized version of the given image according specified width and
/// height in pixel.
///
/// \param[in] hBmp    : Handle (HBITMAP) to bitmap image to resize.
/// \param[in] width   : Image desired width.
/// \param[in] height  : Image desired height.
/// \param[in] aspect  : If true, the image original aspect ratio is conserved.
///
/// \return Handle (HBITMAP) to resized bitmap.
///
HBITMAP Om_getBitmapThumbnail(HBITMAP hBmp, unsigned width, unsigned height, bool aspect = true);

/// \brief Convert bitmap to PNG
///
/// Convert the given bitmap to PNG image data.
///
/// \param[in] hBmp    : Handle (HBITMAP) to bitmap image to convert.
/// \param[out] size   : Size in byte of PNG data.
///
/// \return Pointer to PNG data.
///
void* Om_getPngData(HBITMAP hBmp, size_t* size);

/// \brief Load stock shell icon
///
/// Loads and returns the specified Windows Shell Stock icon.
///
/// \param[in] id      : Stock icon id, form SHSTOCKICONID enum
/// \param[in] large   : If true, load the large size icon version
///
/// \return Icon handle (HICON) of the specified Shell Stock icon
///
void* Om_loadShellIcon(unsigned id, bool large = false);

/// \brief Load stock shell icon as HBITMAP
///
/// Loads and returns the specified Windows Shell Stock icon as HBITMAP.
///
/// \param[in] id      : Stock icon id, form SHSTOCKICONID enum
/// \param[in] large   : If true, load the large size icon version
///
/// \return Bitmap handle (HBITMAP) of the specified Shell Stock icon
///
void* Om_loadShellBitmap(unsigned id, bool large = false);

/// \brief Get Windows error string
///
/// Returns the error string corresponding to the given Windows error code.
///
/// \param[in] code    : Windows error code.
///
/// \return Error string.
///
wstring Om_getErrorStr(int code);

#endif // OMGLOBAL_H
