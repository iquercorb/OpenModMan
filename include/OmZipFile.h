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
#ifndef OMZIPFILE_H
#define OMZIPFILE_H

#include "OmBase.h"

/// \brief Zip file interface.
///
/// Object to handle a Zip file.
///
class OmZipFile
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmZipFile();

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmZipFile();

    /// \brief Initialize a zip file.
    ///
    /// Initializes a new or existing zip file for writing operation, this function
    /// must be called in order to use the append() method. This function does not
    /// allow to read zip file content.
    ///
    /// \param[in]  pat   : Path to Zip file to initialize.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool init(const wstring& path);

    /// \brief Append data to zip file.
    ///
    /// Adds data and index entry to the zip file from the specified file.
    ///
    /// \param[in]  src   : Path to file to get data from.
    /// \param[in]  dst   : Destination path in zip index.
    /// \param[in]  level : Compression level, where 0 is none and 3 is best.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool append(const wstring& src, const wstring& dst, unsigned level);

    /// \brief Append data to zip file.
    ///
    /// Adds data and index entry to the zip file from the specified buffer.
    ///
    /// \param[in]  data  : Buffer containing data to add.
    /// \param[in]  src   : Size of data to add.
    /// \param[in]  dst   : Destination path in zip index.
    /// \param[in]  level : Compression level, where 0 is none and 3 is best.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool append(const void* data, size_t size, const wstring& dst, unsigned level);

    /// \brief Load a zip file.
    ///
    /// Initializes an existing zip file for reading operation, this function
    /// must be called in order to use the extract(), locate(), index() and
    /// related method. This function does not allow to add content to zip file.
    ///
    /// \param[in]  path  : Path to Zip file to load.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool load(const wstring& path);

    /// \brief Zip index entries count.
    ///
    /// Returns the count of index entries in the current loaded zip file.
    ///
    /// \return Zip file index entries count.
    ///
    unsigned indexCount() const;

    /// \brief Index content.
    ///
    /// Returns the specified index's content (path).
    ///
    /// \param[in] i       : Index to get.
    ///
    /// \return The specified index's content.
    ///
    wstring index(unsigned i) const;

    /// \brief Index content.
    ///
    /// Retrieves the specified index's content (path).
    ///
    /// \param[in]  path  : Wstring object to get result.
    /// \param[in]  i     : Index to get.
    ///
    void index(wstring& path, unsigned i) const;

    /// \brief Check index directory status.
    ///
    /// Checks whether an index entry corresponds to a directory or a file.
    ///
    /// \param[in]  i      : Index to check.
    ///
    /// \return True if the specified index correspond to a directory, false
    /// otherwise
    ///
    bool indexIsDir(unsigned i) const;

    /// \brief Locate index from path.
    ///
    /// Retrieves the index position that matches the given path.
    ///
    /// \param[in]  path  : Path to search in index entries.
    ///
    /// \return Index position that matches the given path, or -1 if not found.
    ///
    int locate(const wstring& src) const;

    /// \brief Extract zip data to file.
    ///
    /// Extracts zip data to the specified file.
    ///
    /// \param[in]  src   : Path to search in zip index entries.
    /// \param[in]  dst   : Destination file to write.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool extract(const wstring& src, const wstring& dst) const;

    /// \brief Extract zip data to file.
    ///
    /// Extracts zip data to the specified file.
    ///
    /// \param[in]  i     : Zip index entry.
    /// \param[in]  dst   : Destination file to write.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool extract(unsigned i, const wstring& dst) const;

    /// \brief Extract zip data to buffer.
    ///
    /// Extracts zip data to the specified buffer.
    ///
    /// \param[in]  src     : Path to search in zip index entries.
    /// \param[in]  buffer  : Destination buffer to get data.
    /// \param[in]  size    : Maximum size to write in buffer.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool extract(const wstring& src, void* buffer, size_t size) const;

    /// \brief Extract zip data to buffer.
    ///
    /// Extracts zip data to the specified buffer.
    ///
    /// \param[in] i       : Zip index entry.
    /// \param[in] buffer  : Destination buffer to get data.
    /// \param[in] size    : Maximum size to write in buffer.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool extract(unsigned i, void* buffer, size_t size) const;

    /// \brief Get size of uncompressed data.
    ///
    /// Returns the size of a zip data entry once inflated.
    ///
    /// \param[in]  i      : Zip index entry.
    ///
    /// \return Uncompressed size of data or 0 if error occurred.
    ///
    size_t size(unsigned i) const;

    /// \brief Get size of uncompressed data.
    ///
    /// Returns the size of a zip data entry once inflated.
    ///
    /// \param[in]  src    : Path to search in zip index entries.
    ///
    /// \return Uncompressed size of data or 0 if error occurred.
    ///
    size_t size(const wstring& src) const;

    /// \brief Get zip last error code.
    ///
    /// Returns error code corresponding to the last encountered error.
    ///
    /// \return Miniz error code.
    ///
    unsigned lastError() const;

    /// \brief Get zip last error string.
    ///
    /// Returns error string corresponding to the last encountered error.
    ///
    /// \return Miniz error string.
    ///
    wstring lastErrorStr() const;

    /// \brief Close and finalize the zip file.
    ///
    /// Close the zip file handle, and finalize archive if Zip was initialized for
    /// writing operation.
    ///
    void close();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void*               _data;        //< internal data structure

    void*               _file;        //< internal file pointer

    unsigned            _stat;        //< file status
};

#endif // OMZIPFILE_H
