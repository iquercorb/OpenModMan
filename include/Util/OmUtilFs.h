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
#ifndef OMUTILFS_H_INCLUDED
#define OMUTILFS_H_INCLUDED

#include "OmBase.h"

/// \brief Check empty folder
///
/// Checks whether the specified folder is empty.
///
/// \param[in]  path   : Path to folder to check.
///
/// \return True if the specified folder is empty, false otherwise.
///
bool Om_isDirEmpty(const wstring& path);

/// \brief Create folder
///
/// Creates the specified folder.
///
/// \param[in]  path   : Path of folder to create.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
int Om_dirCreate(const wstring& path);

/// \brief Create folder recursively
///
/// Creates the specified folder and its parent tree if needed.
///
/// \param[in]  path   : Path of folder(s) to create
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
int Om_dirCreateRecursive(const wstring& path);

/// \brief Delete folder
///
/// Deletes the specified folder.
///
/// \param[in]  path   : Path of folder to delete.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
int Om_dirDelete(const wstring& path);

/// \brief Delete folder recursively
///
/// Deletes the specified folder and all its content recursively.
///
/// \param[in]  path   : Path of folder to delete.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
int Om_dirDeleteRecursive(const wstring& path);

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
int Om_fileCopy(const wstring& src, const wstring& dst, bool ow = true);

/// \brief Copy file
///
/// Copy the given file to the specified location.
///
/// \param[in]  src    : Source file path to copy.
/// \param[in]  dst    : Destination file path.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
int Om_fileMove(const wstring& src, const wstring& dst);

/// \brief Delete file
///
/// Delete the specified file.
///
/// \param[in]  path   : Path to file to delete.
///
/// \return 0 if operation succeed, WinAPI error code otherwise.
///
int Om_fileDelete(const wstring& path);

/// \brief Check valid file
///
/// Checks whether the specified item is actually a valid file.
///
/// \param[in]  path   : Path to item to check.
///
/// \return True if item is actually a file, false if it does not exists or
///         is a folder.
///
bool Om_isFile(const wstring& path);

/// \brief Check valid folder
///
/// Checks whether the specified item is actually a valid folder.
///
/// \param[in]  path   : Path to item to check.
///
/// \return True if item is actually a folder, false if it does not exists or
///         is a file.
///
bool Om_isDir(const wstring& path);

/// \brief Check existing item
///
/// Checks whether the specified item exists, either as file or folder.
///
/// \param[in]  path   : Path to item to check.
///
/// \return True if item is actually a file or folder, false otherwise.
///
bool Om_pathExists(const wstring& path);

/// \brief Set item attribute
///
/// Set attribute to specified item, either file or folder.
///
/// \param[in]  path   : Path to item to set attribute.
/// \param[in]  attr   : Attribute to set.
///
/// \return True if operation succeed, false otherwise.
///
bool Om_itemSetAttr(const wstring& path, uint32_t attr);

/// \brief Check item attributes
///
/// Check whether item has the specified attribute mask, either file
/// or folder.
///
/// \param[in]  path   : Path to item to check attributes.
/// \param[in]  mask   : Attributes mask to check.
///
/// \return True if mask matches item attributes, false otherwise.
///
bool Om_itemHasAttr(const wstring& path, uint32_t mask);

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
/// the given custom filter.
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

/// \brief List folders and files with custom filter
///
/// Retrieves the list of folders and files contained in the specified origin location
/// using the given custom filter.
///
/// \param[out] ls      : Pointer to array of wstring to be filled with result.
/// \param[in]  origin  : Path where to list files from.
/// \param[in]  filter  : Custom filter to select files.
/// \param[in]  abs     : If true, returns files absolute path instead of
///                       files name alone.
///
void Om_lsAllFiltered(vector<wstring>* ls, const wstring& origin, const wstring& filter, bool abs = true);

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

/// \brief Read into directory access mask
///
/// Access mask for directory read/traverse content
///
#define OMM_ACCESS_DIR_READ     FILE_LIST_DIRECTORY|FILE_TRAVERSE|FILE_READ_ATTRIBUTES

/// \brief Write into directory access mask
///
/// Access mask for directory write/add content
///
#define OMM_ACCESS_DIR_WRITE    FILE_ADD_FILE|FILE_ADD_SUBDIRECTORY|FILE_WRITE_ATTRIBUTES

/// \brief Execute file access mask
///
/// Access mask for fie execute
///
#define OMM_ACCESS_FILE_EXEC    FILE_EXECUTE

/// \brief Read file access mask
///
/// Access mask for file read data
///
#define OMM_ACCESS_FILE_READ    FILE_READ_DATA|FILE_READ_ATTRIBUTES

/// \brief Write file access mask
///
/// Access mask for file write/append data
///
#define OMM_ACCESS_FILE_WRITE   FILE_WRITE_DATA|FILE_APPEND_DATA|FILE_WRITE_ATTRIBUTES

/// \brief check file or directory permission
///
/// Checks whether the current process application have the specified
/// permissions on the given file or folder.
///
/// \param[in]  path  : Path to file or folder to check permission on.
/// \param[in]  mask  : Mask for requested permission.
///
/// \return True if requested permission are allowed, false otherwise
///
bool Om_checkAccess(const wstring& path, unsigned mask);

/// \brief Load plan text.
///
/// Loads the specified file as plain text file.
///
/// \param[in] path    : Path to text file to be loaded.
///
/// \return String of file content
///
string Om_loadPlainText(const wstring& path);

/// \brief Load plan text.
///
/// Loads the specified file as plain text file.
///
/// \param[in] text    : String to receive loaded text data.
/// \param[in] path    : Path to text file to be loaded.
///
/// \return Count of bytes read.
///
size_t Om_loadPlainText(string& text, const wstring& path);

/// \brief Load binary file.
///
/// Loads the specified file as binary data.
///
/// \param[in] size    : Pointer to receive size of load data in bytes.
/// \param[in] path    : Path to file to be loaded.
///
/// \return Pointer to loaded data.
///
uint8_t* Om_loadBinary(size_t* size, const wstring& path);

#endif // OMUTILFS_H_INCLUDED
