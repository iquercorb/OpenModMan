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
#ifndef OMARCHIVE_H
#define OMARCHIVE_H

#include "OmBase.h"

enum OmArchiveMethod : int32_t
{
  OM_METHOD_STORE     = 0,    //< MZ_COMPRESS_METHOD_STORE
  OM_METHOD_DEFLATE   = 8,    //< MZ_COMPRESS_METHOD_DEFLATE
  OM_METHOD_LZMA      = 14,   //< MZ_COMPRESS_METHOD_LZMA
  OM_METHOD_LZMA2     = 95,   //< MZ_COMPRESS_METHOD_XZ
  OM_METHOD_ZSTD      = 93    //< MZ_COMPRESS_METHOD_ZSTD
};

enum OmArchiveLevel : int32_t
{
  OM_LEVEL_NONE    = 0,
  OM_LEVEL_FAST    = 2,
  OM_LEVEL_SLOW    = 6,
  OM_LEVEL_BEST    = 9
};

/// \brief Zip file interface.
///
/// Object to handle a Zip file.
///
class OmArchive
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmArchive();

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmArchive();

    /// \brief Open a zip file for writing.
    ///
    /// Initializes an existing zip file for writing operation.
    ///
    /// \param[in]  path    : Path to file to open.
    /// \param[in]  method  : Compression algorithm to use
    /// \param[in]  level   : Compression level
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool write(const OmWString& path, int32_t method = OM_METHOD_DEFLATE, int32_t level = OM_LEVEL_SLOW);

    /// \brief Compress and add file to zip
    ///
    /// Compress and add the specified file to zip
    ///
    /// \param[in] src     : Path to file to to compress
    /// \param[in] dst     : File name/path in zip
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool entryAdd(const OmWString& src, const OmWString& dst, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr) const;

    /// \brief Compress and add file to zip
    ///
    /// Compress and add the specified file to zip
    ///
    /// \param[in] data   : Buffer containing data to add.
    /// \param[in] size   : Size of data to add.
    /// \param[in] dst    : File name/path in zip
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool entryAdd(const void* data, uint64_t size, const OmWString& dst, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr) const;

    /// \brief Open a zip file for reading.
    ///
    /// Initializes an existing zip file for reading operation.
    ///
    /// \param[in]  path  : Path to file to open.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool read(const OmWString& path);

    /// \brief Get entries count.
    ///
    /// Returns the count of central directory entries of the current loaded zip file.
    ///
    /// \return Entries count.
    ///
    size_t entryCount() const {
      return this->_zent_size;
    }

    /// \brief Get entry path/filename
    ///
    /// Returns the entry path/filename at specified index.
    ///
    /// \param[in] i       : Entry index
    ///
    /// \return Entry path or filename as wide string.
    ///
    const wchar_t* entryPath(size_t i) const;

    /// \brief Get entry path/filename
    ///
    /// Returns the entry path/filename at specified index.
    ///
    /// \param[in] path    : Wide char string to be set
    /// \param[in] i       : Entry index
    ///
    /// \return Entry path or filename as wide string.
    ///
    void entryPath(size_t i, OmWString& path) const;

    /// \brief Get entry uncompressed size
    ///
    /// Returns the uncompressed size of the specified entry file
    ///
    /// \param[in] i       : Entry index
    ///
    /// \return Entry file uncompressed size in bytes
    ///
    uint64_t entrySize(size_t i) const;

    /// \brief Check whether entry is a directory
    ///
    /// Checks whether the entry at specified index is a directory
    ///
    /// \param[in] i       : Entry index
    ///
    /// \return True if entry is a directory, false otherwise
    ///
    bool entryIsDir(size_t i) const;

    /// \brief Extract and save as file
    ///
    /// Extract and save specified entry as file
    ///
    /// \param[in] i       : Entry index
    /// \param[in] dest    : Path to destination file to save
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool entrySave(size_t i, const OmWString& dest, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr) const;

    /// \brief Extract and save as file
    ///
    /// Extract and save specified entry as file
    ///
    /// \param[in] entry   : Entry filenale/path to locate
    /// \param[in] dest    : Path to destination file to save
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool entrySave(const OmWString& entry, const OmWString& dest, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr) const;

    /// \brief Extract and save in memory
    ///
    /// Extract and save specified entry in memory buffer. The buffer
    /// must be large enough to accept entry uncompressed size.
    ///
    /// \param[in] i        : Entry index
    /// \param[in] buffer   : Pointer to buffer to be filled
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool entrySave(size_t i, void* buffer, Om_progressCb progress_cb = nullptr, void* user_ptr = nullptr) const;

    /// \brief Locate entry index
    ///
    /// Search for the specified entry in zip Central Directory.
    ///
    /// \param[in] filename : Filename or path to search in Central Directory
    ///
    /// \return If found, the index in Central Directory, -1 otherwise
    ///
    uint32_t entryLocate(const OmWString& filename) const;

    /// \brief Close and finalize the zip file.
    ///
    /// Close the zip file handle, and finalize archive if zip was
    /// initialized for writing operation.
    ///
    bool close();

    /// \brief Get zip last error string.
    ///
    /// Returns error string corresponding to the last encountered error.
    ///
    /// \return Formated error string.
    ///
    OmWString lastErrorStr() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void*               _zctx;        //< minizip reader/writer struct

    void*               _zent;        //< zip central-directory mirror

    uint64_t            _zent_size;   //< zip central-directory entry count

    uint32_t            _stat;        //< file status
};

#endif // OMARCHIVE_H
