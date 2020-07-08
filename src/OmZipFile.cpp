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

#include "thirdparty/miniz/miniz.h"
#include "OmZipFile.h"

/// \brief Zip status
///
/// Status definitions for Zip object
///
#define ZIP_READER  0x1 //< Zip is in read mode
#define ZIP_WRITER  0x2 //< Zip is in write mode
#define ZIP_ERROR   0x4 //< Zip is in error state

/// \brief Zip compression level map
///
/// Zip compression level conversion map for Miniz.c
///
static unsigned __minizLevel[] = {  0,    //< MZ_NO_COMPRESSION
                                    1,    //< MZ_BEST_SPEED
                                    6,    //< MZ_DEFAULT_LEVEL
                                    9 };  //< MZ_BEST_COMPRESSION

/// local static string conversion functions to optimize operations.
///
/// Miniz works only with C char while we are standardized to wide string. Zip
/// functions requires a lot of string manipulations for paths retrieving,
/// supplying and reformat.
///
/// the amount of computation spent to create temporary string, wstring and
/// calls to conversion functions afraid me, so, I decided to optimize that all
/// with some inline functions and fixed size buffers.
#define ZMBUFF_SIZE 1080

/// \brief wide string to char conversion
///
/// Simple function to convert a wide string into char string.
///
/// \param[in]  buf   : Buffer to be filled then returned.
/// \param[in]  str   : Source wstring to convert.
///
/// \return The char pointer passed as buffer parameter
///
inline static char* __toCchar(char* buf, const wstring& str)
{
  wcstombs(buf, str.c_str(), ZMBUFF_SIZE);
  return buf;
}
/// \brief Windows to Zip CDR path
///
/// Convert the given wide string Windows path into multibyte Zip CDR path.
///
/// \param[in]  buf   : Buffer to be as result of the conversion.
/// \param[in]  str   : Wide string Windows path to convert
///
/// \return The char pointer passed as buffer parameter
///
inline static char* __toCDRpath(char* buf, const wstring& str)
{
  wcstombs(buf, str.c_str(), ZMBUFF_SIZE);

  // paths for Zip file index (CDR) must have a forward slash separator
  // while Windows use backslash, so we need to replace all separators.
  char* p = buf;
  while(*p != 0) {
    if(*p == '\\') { *p = '/'; }
    ++p;
  }

  return buf;
}

/// \brief Zip CDR to Windows path
///
/// Convert the given Zip CDR path into its wide string Windows version.
///
/// \param[in]  ret   : wide string to be set as result of the conversion.
/// \param[in]  str   : Zip CDR path to convert.
///
inline static void __fromCDRpath(wstring& ret, const char* str)
{
  wchar_t buf[281];

  mbstowcs(buf, str, 280);

  //  paths for Zip file index (CDR) must have a forward slash separator
  // while Windows use backslash, so we need to replace all separators.
  wchar_t* p = buf;
  while(*p != 0) {
    if(*p == L'/') { *p = L'\\'; }
    ++p;
  }

  ret = buf;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmZipFile::OmZipFile() :
  _data(new mz_zip_archive()),
  _stat(0)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmZipFile::~OmZipFile()
{
  close();
  delete static_cast<mz_zip_archive*>(_data);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::init(const wstring& path)
{
  char dbuf[ZMBUFF_SIZE];

  if(!mz_zip_writer_init_file(static_cast<mz_zip_archive*>(_data), __toCchar(dbuf, path), 0)) {
    return false;
  }

  _stat |= ZIP_WRITER;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::load(const wstring& path)
{
  char sbuf[ZMBUFF_SIZE];

  if(!mz_zip_reader_init_file(static_cast<mz_zip_archive*>(_data), __toCchar(sbuf, path), 0)) {
    return false;
  }

  _stat |= ZIP_READER;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::append(const wstring& src, const wstring& dst, unsigned level)
{
  if(_stat & ZIP_WRITER) {

    char dbuf[ZMBUFF_SIZE];
    char sbuf[ZMBUFF_SIZE];

    if(mz_zip_writer_add_file(static_cast<mz_zip_archive*>(_data),
                                    __toCDRpath(dbuf, dst),
                                    __toCchar(sbuf, src),
                                    nullptr, 0,
                                    __minizLevel[level])) {
      return true;
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::append(const void* data, size_t size, const wstring& dst, unsigned level)
{
  if(_stat & ZIP_WRITER) {

    char dbuf[ZMBUFF_SIZE];

    if(!mz_zip_writer_add_mem(static_cast<mz_zip_archive*>(_data),
                                __toCDRpath(dbuf, dst),
                                data, size,
                                __minizLevel[level])) {
      return false;
    }
    return true;
  }

  return false;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmZipFile::indexCount() const
{
  if(_stat & ZIP_READER) {
    return mz_zip_reader_get_num_files(static_cast<mz_zip_archive*>(_data));
  }
  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring OmZipFile::index(unsigned i) const
{
  wstring ret;

  if(_stat & ZIP_READER) {
    mz_zip_archive_file_stat zf; // zip file stat struct
    if(mz_zip_reader_file_stat(static_cast<mz_zip_archive*>(_data), i, &zf)) {
      __fromCDRpath(ret, zf.m_filename);
    }
  }

  return ret;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmZipFile::index(wstring& path, unsigned i) const
{
  if(_stat & ZIP_READER) {
    mz_zip_archive_file_stat zf; // zip file stat struct
    if(mz_zip_reader_file_stat(static_cast<mz_zip_archive*>(_data), i, &zf)) {
      __fromCDRpath(path, zf.m_filename);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::indexOm_isDir(unsigned i) const
{
  if(_stat & ZIP_READER) {
    return mz_zip_reader_is_file_a_directory(static_cast<mz_zip_archive*>(_data), i);
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmZipFile::locate(const wstring& src) const
{
  if(_stat & ZIP_READER) {

    char sbuf[ZMBUFF_SIZE];

    return mz_zip_reader_locate_file(static_cast<mz_zip_archive*>(_data), __toCDRpath(sbuf, src), "", 0);
  }
  return -1;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::extract(const wstring& src, const wstring& dst) const
{
  if(_stat & ZIP_READER) {

    char dbuf[ZMBUFF_SIZE];
    char sbuf[ZMBUFF_SIZE];

    int i = mz_zip_reader_locate_file(static_cast<mz_zip_archive*>(_data), __toCDRpath(sbuf, src), "", 0);
    if(i != -1) {
      if(mz_zip_reader_extract_to_file(static_cast<mz_zip_archive*>(_data), i, __toCchar(dbuf, dst), 0)) {
        return true;
      }
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::extract(unsigned i, const wstring& dst) const
{
  if(_stat & ZIP_READER) {

    char dbuf[ZMBUFF_SIZE];

    if(mz_zip_reader_extract_to_file(static_cast<mz_zip_archive*>(_data), i, __toCchar(dbuf, dst), 0)) {
      return true;
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::extract(const wstring& src, void* buffer, size_t size) const
{
  if(_stat & ZIP_READER) {

    char sbuf[ZMBUFF_SIZE];

    int i = mz_zip_reader_locate_file(static_cast<mz_zip_archive*>(_data), __toCDRpath(sbuf, src), "", 0);
    if(i != -1) {
      if(mz_zip_reader_extract_to_mem(static_cast<mz_zip_archive*>(_data), i, buffer, size, 0)) {
        return true;
      }
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::extract(unsigned i, void* buffer, size_t size) const
{
  if(_stat & ZIP_READER) {
    if(mz_zip_reader_extract_to_mem(static_cast<mz_zip_archive*>(_data), i, buffer, size, 0)) {
      return true;
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmZipFile::size(unsigned i) const
{
  if(_stat & ZIP_READER) {
    mz_zip_archive_file_stat zf; // zip file stat struct
    if(mz_zip_reader_file_stat(static_cast<mz_zip_archive*>(_data), i, &zf)){
      return zf.m_uncomp_size;
    }
  }
  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmZipFile::lastError() const
{
  return static_cast<mz_zip_archive*>(_data)->m_last_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring OmZipFile::lastErrorStr() const
{
  wstring err;

  switch(static_cast<mz_zip_archive*>(_data)->m_last_error)
  {
  case MZ_ZIP_NO_ERROR: err = L"ZIP_NO_ERROR"; break;
  case MZ_ZIP_UNDEFINED_ERROR: err = L"ZIP_UNDEFINED_ERROR"; break;
  case MZ_ZIP_TOO_MANY_FILES: err = L"ZIP_TOO_MANY_FILES"; break;
  case MZ_ZIP_FILE_TOO_LARGE: err = L"ZIP_FILE_TOO_LARGE"; break;
  case MZ_ZIP_UNSUPPORTED_METHOD: err = L"ZIP_UNSUPPORTED_METHOD"; break;
  case MZ_ZIP_UNSUPPORTED_ENCRYPTION: err = L"ZIP_UNSUPPORTED_ENCRYPTION"; break;
  case MZ_ZIP_UNSUPPORTED_FEATURE: err = L"ZIP_UNSUPPORTED_FEATURE"; break;
  case MZ_ZIP_FAILED_FINDING_CENTRAL_DIR: err = L"ZIP_FAILED_FINDING_CENTRAL_DIR"; break;
  case MZ_ZIP_NOT_AN_ARCHIVE: err = L"ZIP_NOT_AN_ARCHIVE"; break;
  case MZ_ZIP_INVALID_HEADER_OR_CORRUPTED: err = L"ZIP_INVALID_HEADER_OR_CORRUPTED"; break;
  case MZ_ZIP_UNSUPPORTED_MULTIDISK: err = L"ZIP_UNSUPPORTED_MULTIDISK"; break;
  case MZ_ZIP_DECOMPRESSION_FAILED: err = L"ZIP_DECOMPRESSION_FAILED"; break;
  case MZ_ZIP_COMPRESSION_FAILED: err = L"ZIP_COMPRESSION_FAILED"; break;
  case MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE: err = L"ZIP_UNEXPECTED_DECOMPRESSED_SIZE"; break;
  case MZ_ZIP_CRC_CHECK_FAILED: err = L"ZIP_CRC_CHECK_FAILED"; break;
  case MZ_ZIP_UNSUPPORTED_CDIR_SIZE: err = L"ZIP_UNSUPPORTED_CDIR_SIZE"; break;
  case MZ_ZIP_ALLOC_FAILED: err = L"ZIP_ALLOC_FAILED"; break;
  case MZ_ZIP_FILE_OPEN_FAILED: err = L"ZIP_FILE_OPEN_FAILED"; break;
  case MZ_ZIP_FILE_CREATE_FAILED: err = L"ZIP_FILE_CREATE_FAILED"; break;
  case MZ_ZIP_FILE_WRITE_FAILED: err = L"ZIP_FILE_WRITE_FAILED"; break;
  case MZ_ZIP_FILE_READ_FAILED: err = L"ZIP_FILE_READ_FAILED"; break;
  case MZ_ZIP_FILE_CLOSE_FAILED: err = L"ZIP_FILE_CLOSE_FAILED"; break;
  case MZ_ZIP_FILE_SEEK_FAILED: err = L"ZIP_FILE_SEEK_FAILED"; break;
  case MZ_ZIP_FILE_STAT_FAILED: err = L"ZIP_FILE_STAT_FAILED"; break;
  case MZ_ZIP_INVALID_PARAMETER: err = L"ZIP_INVALID_PARAMETER"; break;
  case MZ_ZIP_INVALID_FILENAME: err = L"ZIP_INVALID_FILENAME"; break;
  case MZ_ZIP_BUF_TOO_SMALL: err = L"ZIP_BUF_TOO_SMALL"; break;
  case MZ_ZIP_INTERNAL_ERROR: err = L"ZIP_INTERNAL_ERROR"; break;
  case MZ_ZIP_FILE_NOT_FOUND: err = L"ZIP_FILE_NOT_FOUND"; break;
  case MZ_ZIP_ARCHIVE_TOO_LARGE: err = L"ZIP_ARCHIVE_TOO_LARGE"; break;
  case MZ_ZIP_VALIDATION_FAILED: err = L"ZIP_VALIDATION_FAILED"; break;
  case MZ_ZIP_WRITE_CALLBACK_FAILED: err = L"ZIP_WRITE_CALLBACK_FAILED"; break;
  case MZ_ZIP_TOTAL_ERRORS: err = L"ZIP_TOTAL_ERRORS"; break;
  default: err = L"ZIP_UNKNOW_ERROR"; break;
  }

  return err;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmZipFile::size(const wstring& src) const
{
  if(_stat & ZIP_READER) {

    char sbuf[ZMBUFF_SIZE];

    int i = mz_zip_reader_locate_file(static_cast<mz_zip_archive*>(_data), __toCDRpath(sbuf, src), "", 0);
    if(i != -1) {
      mz_zip_archive_file_stat zf; // zip file stat struct
      if(mz_zip_reader_file_stat(static_cast<mz_zip_archive*>(_data), i, &zf)){
        return zf.m_uncomp_size;
      }
    }
  }
  return -1;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmZipFile::close()
{
  if(_stat & ZIP_WRITER) {
    mz_zip_writer_finalize_archive(static_cast<mz_zip_archive*>(_data));
    mz_zip_writer_end(static_cast<mz_zip_archive*>(_data));
  }

  if(_stat & ZIP_READER) {
    mz_zip_reader_end(static_cast<mz_zip_archive*>(_data));
  }
}
