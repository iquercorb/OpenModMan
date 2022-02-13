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
#include "OmUtilStr.h"

#include "miniz/miniz.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
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
static unsigned __mzlvl[] = { 0,    //< MZ_NO_COMPRESSION
                              1,    //< MZ_BEST_SPEED
                              6,    //< MZ_DEFAULT_LEVEL
                              9 };  //< MZ_BEST_COMPRESSION


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmZipFile::OmZipFile() :
  _data(new mz_zip_archive()), _stat(0)
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
  string ansi_path;

  Om_toANSI(&ansi_path, path);

  if(!mz_zip_writer_init_file(static_cast<mz_zip_archive*>(_data), ansi_path.c_str(), 0)) {
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
  string ansi_path;

  Om_toANSI(&ansi_path, path);

  if(!mz_zip_reader_init_file(static_cast<mz_zip_archive*>(_data), ansi_path.c_str(), 0)) {
    return false;
  }

  _stat |= ZIP_READER;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::append(const wstring& src, const wstring& dst, unsigned lvl)
{
  if(_stat & ZIP_WRITER) {

    string zcdr_dst;
    string ansi_src;

    Om_toZipCDR(&zcdr_dst, dst);
    Om_toANSI(&ansi_src, src);

    if(mz_zip_writer_add_file(static_cast<mz_zip_archive*>(_data), zcdr_dst.c_str(), ansi_src.c_str(), nullptr, 0, __mzlvl[lvl])) {
      return true;
    }

  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::append(const void* data, size_t size, const wstring& dst, unsigned lvl)
{
  if(_stat & ZIP_WRITER) {

    string zcdr_dst;

    Om_toZipCDR(&zcdr_dst, dst);

    if(!mz_zip_writer_add_mem(static_cast<mz_zip_archive*>(_data), zcdr_dst.c_str(), data, size, __mzlvl[lvl])) {
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
      Om_fromZipCDR(&ret, zf.m_filename);
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
      Om_fromZipCDR(&path, zf.m_filename);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::indexIsDir(unsigned i) const
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

    string zcdr_src;

    Om_toZipCDR(&zcdr_src, src);

    return mz_zip_reader_locate_file(static_cast<mz_zip_archive*>(_data), zcdr_src.c_str(), "", 0);

  }
  return -1;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile::extract(const wstring& src, const wstring& dst) const
{
  if(_stat & ZIP_READER) {

    string zcdr_src;
    string ansi_dst;

    Om_toZipCDR(&zcdr_src, src);
    Om_toANSI(&ansi_dst, dst);

    int i = mz_zip_reader_locate_file(static_cast<mz_zip_archive*>(_data), zcdr_src.c_str(), "", 0);
    if(i != -1) {
      if(mz_zip_reader_extract_to_file(static_cast<mz_zip_archive*>(_data), i, ansi_dst.c_str(), 0)) {
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

    string ansi_dst;

    Om_toANSI(&ansi_dst, dst);

    if(mz_zip_reader_extract_to_file(static_cast<mz_zip_archive*>(_data), i, ansi_dst.c_str(), 0)) {
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

    string zcdr_src;

    Om_toZipCDR(&zcdr_src, src);

    int i = mz_zip_reader_locate_file(static_cast<mz_zip_archive*>(_data), zcdr_src.c_str(), "", 0);
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
size_t OmZipFile::size(const wstring& src) const
{
  if(_stat & ZIP_READER) {

    string zcdr_src;

    Om_toZipCDR(&zcdr_src, src);

    int i = mz_zip_reader_locate_file(static_cast<mz_zip_archive*>(_data), zcdr_src.c_str(), "", 0);
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
