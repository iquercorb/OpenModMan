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
#include <algorithm>          //< std::replace
#include <ctime>              //< time()

#include "minizip-ng/mz.h"
#include "minizip-ng/mz_os.h"
#include "minizip-ng/mz_strm.h"
#include "minizip-ng/mz_strm_os.h"
#include "minizip-ng/mz_strm_buf.h"
#include "minizip-ng/mz_strm_mem.h"
#include "minizip-ng/mz_strm_split.h"
#include "minizip-ng/mz_zip.h"
#include "minizip-ng/mz_zip_rw.h"

#include "OmBaseWin.h"        //< WinAPI
#include "OmUtilWin.h"
#include "OmUtilStr.h"
#include "OmUtilFs.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmArchive.h"

/// \brief zip status
///
/// Status definitions for zip object
///
#define ZIP_READER  0x1 //< Zip is in read mode
#define ZIP_WRITER  0x2 //< Zip is in write mode
#define ZIP_ERROR   0x4 //< Zip is in error state

#define ZIP_IO_BUF_SIZE   262144

/// \brief Zip context structure
///
/// Internal reader/writer structure to work with mz_zip API
///
typedef struct zip_context_
{
  void*         zip_hnd;

  void*         strm_file;

  void*         strm_buff;

  void*         strm_splt;

  int32_t       cmp_level;

  int32_t       cmp_method;

  int32_t       mz_err;

  OmWString     ws_err;

  uint8_t       buffer[ZIP_IO_BUF_SIZE];

} zip_context_t;


/// \brief Zip entry structure
///
/// Internal structure to store zip file entry informations
///
typedef struct zip_entry_
{
  int64_t         offset;

  int32_t         is_dir;

  uint64_t        file_size;

  wchar_t         file_path[OM_MAX_PATH];

} zip_entry_t;


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmArchive::OmArchive():
  _zctx(nullptr),
  _zent(nullptr),
  _zent_size(0),
  _stat(0)
{
  // create zip base architecture
  this->_zctx = calloc(1, sizeof(zip_context_t));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmArchive::~OmArchive()
{
  this->close();

  if(this->_zctx != nullptr) {
    free(this->_zctx);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmArchive::read(const OmWString& path)
{
  // close and reset interface if any
  this->close();

  zip_context_t* zctx = static_cast<zip_context_t*>(this->_zctx);

  // let be it a reader
  this->_stat = ZIP_READER;

  // create zip reader architecture
  zctx->strm_file = mz_stream_os_create();
  if(!zctx->strm_file) {
    zctx->mz_err = MZ_MEM_ERROR; zctx->ws_err = L"stream OS create error";
    this->close();
    return false; // MZ_MEM_ERROR;
  }

  zctx->strm_buff = mz_stream_buffered_create();
  if(!zctx->strm_buff) {
    zctx->mz_err = MZ_MEM_ERROR; zctx->ws_err = L"stream buffered create error";
    this->close();
    return false; // MZ_MEM_ERROR;
  }

  zctx->strm_splt = mz_stream_split_create();
  if(!zctx->strm_splt) {
    zctx->mz_err = MZ_MEM_ERROR; zctx->ws_err = L"stream split create error";
    this->close();
    return false; // MZ_MEM_ERROR;
  }

  mz_stream_set_base(zctx->strm_buff, zctx->strm_file);
  mz_stream_set_base(zctx->strm_splt, zctx->strm_buff);

  zctx->zip_hnd = mz_zip_create();
  if(!zctx->zip_hnd) {
    zctx->mz_err = MZ_MEM_ERROR; zctx->ws_err = L"zip handle create error";
    this->close();
    return false; // MZ_MEM_ERROR;
  }

  int32_t mz_err = MZ_OK;

  OmCString utf8_path;
  Om_toUTF8(&utf8_path, path);

  mz_err = mz_stream_open(zctx->strm_splt, utf8_path.c_str(), MZ_OPEN_MODE_READ);
  if(mz_err != MZ_OK) {
    this->close();
    zctx->mz_err = mz_err; zctx->ws_err = L"file stream open error";
    return false;
  }

  // mz_zip_reader_open
  mz_err = mz_zip_open(zctx->zip_hnd, zctx->strm_splt, MZ_OPEN_MODE_READ);
  if(mz_err != MZ_OK) {
    this->close();
    zctx->mz_err = mz_err;  zctx->ws_err = L"zip file open error";
    return false;
  }

  // mz_zip_reader_unzip_cd
  mz_zip_file *cd_info = nullptr;

  // check weather zip central directory is simple or zipped
  mz_err = mz_zip_goto_first_entry(zctx->zip_hnd);
  if(mz_err == MZ_OK)
    mz_err = mz_zip_entry_get_info(zctx->zip_hnd, &cd_info);

  if(mz_err != MZ_OK) {
    this->close();
    zctx->mz_err = mz_err;  zctx->ws_err = L"central directory parse error";
    return false;
  }

  // Check whether this is a compressed central directory
  if(strcmp(cd_info->filename, "__cdcd__") == 0) {

    // Unzip the central directory

    uint64_t number_entry = 0;

    // Try to open entry for reading
    if(mz_zip_entry_is_open(zctx->zip_hnd) != MZ_OK) {
      // if open fail zip file may be encrypted, we do not support it
      if(cd_info->flag & MZ_ZIP_FLAG_ENCRYPTED) {
        this->close();
        zctx->mz_err = MZ_PASSWORD_ERROR;  zctx->ws_err = L"zip file is password protected";
        return false;
      }
    }

    void *file_extra_stream = nullptr;

    file_extra_stream = mz_stream_mem_create();
    if(!file_extra_stream) {
      this->close();
      zctx->mz_err = MZ_MEM_ERROR;  zctx->ws_err = L"central directory unzip error";
      return false;
    }

    mz_stream_mem_set_buffer(file_extra_stream, (void*)cd_info->extrafield, cd_info->extrafield_size);
    mz_err = mz_zip_extrafield_find(file_extra_stream, 0xcdcd, INT32_MAX, nullptr);
    if(mz_err == MZ_OK)
      mz_err = mz_stream_read_uint64(file_extra_stream, &number_entry);
    mz_stream_mem_delete(&file_extra_stream);

    if(mz_err != MZ_OK) {
      this->close();
      zctx->mz_err = MZ_MEM_ERROR;  zctx->ws_err = L"central directory unzip error";
      return false;
    }

    void *cd_mem_stream = nullptr;

    mz_zip_get_cd_mem_stream(zctx->zip_hnd, &cd_mem_stream);
    if(mz_stream_mem_is_open(cd_mem_stream) != MZ_OK)
        mz_stream_mem_open(cd_mem_stream, nullptr, MZ_OPEN_MODE_CREATE);

    mz_err = mz_stream_seek(cd_mem_stream, 0, MZ_SEEK_SET);
    if(mz_err == MZ_OK)
      //mz_err = mz_stream_copy_stream(cd_mem_stream, nullptr, handle, mz_zip_reader_entry_read, (int32_t)cd_info->uncompressed_size);
      mz_stream_copy_stream(cd_mem_stream, nullptr, zctx->zip_hnd, mz_zip_entry_read, (int32_t)cd_info->uncompressed_size);

    if(mz_err == MZ_OK) {
      mz_zip_set_cd_stream(zctx->zip_hnd, 0, cd_mem_stream);
      mz_zip_set_number_entry(zctx->zip_hnd, number_entry);
    }
  }

  mz_err = mz_zip_goto_first_entry(zctx->zip_hnd);
  if(mz_err != MZ_OK) {
    this->close();
    zctx->mz_err = mz_err;  zctx->ws_err = L"central-directory unzip error";
    return false;
  }

  // allocate local zip central directory mirror
  mz_zip_get_number_entry(zctx->zip_hnd, &this->_zent_size);

  this->_zent = Om_alloc(this->_zent_size * sizeof(zip_entry_t));
  if(!this->_zent) {
    this->close();
    zctx->mz_err = MZ_MEM_ERROR; zctx->ws_err = L"mirror central-directory error";
    return false;
  }

  Om_memset(this->_zent, 0, this->_zent_size * sizeof(zip_entry_t));

  zip_entry_t* zent = static_cast<zip_entry_t*>(this->_zent);

  mz_zip_file *file_info = nullptr;

  do {
    mz_err = mz_zip_entry_get_info(zctx->zip_hnd, &file_info);
    if(mz_err != MZ_OK) break;

    zent->offset = mz_zip_get_entry(zctx->zip_hnd);
    zent->is_dir = (mz_zip_entry_is_dir(zctx->zip_hnd) == MZ_OK);
    zent->file_size = file_info->uncompressed_size;
    // convert filename UTF-8 to UTF-16
    MultiByteToWideChar(CP_UTF8, 0, file_info->filename, -1, zent->file_path, OM_MAX_PATH);
    // replace slash by back-slash
    for(size_t i = 0; zent->file_path[i] != 0; ++i)
      if(zent->file_path[i] == L'/') zent->file_path[i] = L'\\';

    // next entry
    zent++;

    if(mz_zip_entry_is_open(zctx->zip_hnd) == MZ_OK)
      mz_zip_entry_close(zctx->zip_hnd);

    mz_err = mz_zip_goto_next_entry(zctx->zip_hnd);
    if(mz_err != MZ_OK && mz_err != MZ_END_OF_LIST)
      break;

  } while(mz_err == MZ_OK);

  if(mz_err != MZ_OK && mz_err != MZ_END_OF_LIST) {
    this->close();
    zctx->mz_err = mz_err;  zctx->ws_err = L"central directory walk error";
    return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
const wchar_t* OmArchive::entryPath(size_t i) const {
  return static_cast<zip_entry_t*>(this->_zent)[i].file_path;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmArchive::entryPath(size_t i, OmWString& path) const {
  path = static_cast<zip_entry_t*>(this->_zent)[i].file_path;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t OmArchive::entrySize(size_t i) const {
  return static_cast<zip_entry_t*>(this->_zent)[i].file_size;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmArchive::entryIsDir(size_t i) const {
  return static_cast<zip_entry_t*>(this->_zent)[i].is_dir;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmArchive::entrySave(size_t i, const OmWString& dst, Om_progressCb progress_cb, void* user_ptr) const
{
  int32_t mz_err;

  if(this->_stat & ZIP_READER) {

    zip_context_t* zctx = static_cast<zip_context_t*>(this->_zctx);
    zip_entry_t* zent = static_cast<zip_entry_t*>(this->_zent);

    mz_err = mz_zip_goto_entry(zctx->zip_hnd, zent[i].offset);
    if(mz_err != MZ_OK) {
      zctx->mz_err = mz_err;  zctx->ws_err = L"entry goto error";
      return false;
    }

    // get zipped file info
    mz_zip_file *file_info = nullptr;
    mz_err = mz_zip_entry_get_info(zctx->zip_hnd, &file_info);

    // check whether zip entry is a directory
    if( (mz_zip_entry_is_dir(zctx->zip_hnd) == MZ_OK) &&
        (mz_zip_entry_is_symlink(zctx->zip_hnd) != MZ_OK)) {

      if(!Om_isDir(dst)) {
        // we simply create directory
        mz_err = Om_dirCreateRecursive(dst);
        if(mz_err != ERROR_SUCCESS) {
          zctx->mz_err = mz_err;  zctx->ws_err = L"create directory error";
          return false;
        }
      }

      return true;
    }

    // TODO: implement symlink creation

    OmWString dst_dir = Om_getDirPart(dst);

    // create the destination path tree if required
    if(!Om_isDir(dst_dir)) {
      // we simply create directory
      mz_err = Om_dirCreateRecursive(dst_dir);
      if(mz_err != ERROR_SUCCESS) {
        zctx->mz_err = mz_err;  zctx->ws_err = L"create directory error";
        return false;
      }
    }

    // Create the file on disk so we can save to it
    void *stream = mz_stream_os_create();
    if(!stream) {
      zctx->mz_err = MZ_MEM_ERROR;  zctx->ws_err = L"create stream OS error";
      return false;
    }

    OmCString utf8_dst;
    Om_toUTF8(&utf8_dst, dst);

    mz_err = mz_stream_os_open(stream, utf8_dst.c_str(), MZ_OPEN_MODE_CREATE);

    if(mz_err == MZ_OK) {

      // If the entry isn't open for reading, open it
      if(mz_zip_entry_is_open(zctx->zip_hnd) != MZ_OK)
        mz_err = mz_zip_entry_read_open(zctx->zip_hnd, 0, nullptr);

      if(mz_err != MZ_OK) {
        zctx->mz_err = mz_err;  zctx->ws_err = L"entry read open error";
        return false;
      }

      int32_t wb = 0;
      int32_t rb = 0;

      // Write data to stream until done
      while(mz_err == MZ_OK) {
        rb = mz_zip_entry_read(zctx->zip_hnd, zctx->buffer, sizeof(zctx->buffer));
        if(rb > 0) {
            wb = mz_stream_write(stream, zctx->buffer, rb);
            if(wb != rb) {
              mz_err = MZ_WRITE_ERROR;
              break;
            }
        } else if(rb < 0) {
          mz_err = rb;
          break;
        } else {
          mz_err = MZ_END_OF_STREAM;
          break;
        }
        if(progress_cb) {
          progress_cb(user_ptr, file_info->uncompressed_size, wb, 0);
        }
      }
      mz_zip_entry_close(zctx->zip_hnd);
    }

    mz_stream_close(stream);
    mz_stream_delete(&stream);

    if(mz_err != MZ_OK && mz_err != MZ_END_OF_STREAM ) {
      zctx->mz_err = mz_err; zctx->ws_err = L"file stream error";
      return false;
    }

    // Set the time of the file that has been created
    mz_os_set_file_date(utf8_dst.c_str(), file_info->modified_date, file_info->accessed_date, file_info->creation_date);

    uint32_t target_attrib = 0;

    // Set file attributes for the correct system
    int32_t err_attrib = mz_zip_attrib_convert(MZ_HOST_SYSTEM(file_info->version_madeby),
      file_info->external_fa, MZ_VERSION_MADEBY_HOST_SYSTEM, &target_attrib);

    if(err_attrib == MZ_OK)
      mz_os_set_file_attribs(utf8_dst.c_str(), file_info->external_fa);

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmArchive::entrySave(const OmWString& entry, const OmWString& dst, Om_progressCb progress_cb, void* user_ptr) const
{
  zip_entry_t* zent = static_cast<zip_entry_t*>(this->_zent);

  for(uint64_t i = 0; i < this->_zent_size; ++i) {
    if(Om_namesMatches(zent[i].file_path, entry))
      return this->entrySave(i, dst, progress_cb, user_ptr);
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmArchive::entrySave(size_t i, void* buffer, Om_progressCb progress_cb, void* user_ptr) const
{
  int32_t mz_err;

  if(this->_stat & ZIP_READER) {

    zip_context_t* zctx = static_cast<zip_context_t*>(this->_zctx);
    zip_entry_t* zent = static_cast<zip_entry_t*>(this->_zent);

    mz_err = mz_zip_goto_entry(zctx->zip_hnd, zent[i].offset);
    if(mz_err != MZ_OK) {
      zctx->mz_err = mz_err;  zctx->ws_err = L"entry goto error";
      return false;
    }

    // get zipped file info
    mz_zip_file *file_info = nullptr;
    mz_err = mz_zip_entry_get_info(zctx->zip_hnd, &file_info);

    if(file_info->uncompressed_size > INT32_MAX) {
      zctx->mz_err = MZ_PARAM_ERROR;  zctx->ws_err = L"file too large";
      return false;
    }

    /* Create a memory stream backed by our buffer and save to it */
    void* stream = mz_stream_mem_create();
    if(!stream) {
      zctx->mz_err = MZ_MEM_ERROR;  zctx->ws_err = L"create stream mem error";
      return false;
    }

    mz_stream_mem_set_buffer(stream, buffer, file_info->uncompressed_size);

    mz_err = mz_stream_mem_open(stream, nullptr, MZ_OPEN_MODE_READ);

    if(mz_err == MZ_OK) {

      // If the entry isn't open for reading, open it
      if(mz_zip_entry_is_open(zctx->zip_hnd) != MZ_OK)
        mz_err = mz_zip_entry_read_open(zctx->zip_hnd, 0, nullptr);

      if(mz_err != MZ_OK) {
        zctx->mz_err = mz_err;  zctx->ws_err = L"entry read open error";
        return false;
      }

      int32_t wb = 0;
      int32_t rb = 0;

      // Write data to stream until done
      while(mz_err == MZ_OK) {
        rb = mz_zip_entry_read(zctx->zip_hnd, zctx->buffer, sizeof(zctx->buffer));
        if(rb > 0) {
            wb = mz_stream_mem_write(stream, zctx->buffer, rb);
            if(wb != rb) {
              mz_err = MZ_WRITE_ERROR;
              break;
            }
        } else if(rb < 0) {
          mz_err = rb;
          break;
        } else {
          mz_err = MZ_END_OF_STREAM;
          break;
        }

        if(progress_cb) {
          progress_cb(user_ptr, file_info->uncompressed_size, wb, 0);
        }
      }

      mz_zip_entry_close(zctx->zip_hnd);
    }

    mz_stream_mem_delete(&stream);

    if(mz_err != MZ_OK && mz_err != MZ_END_OF_STREAM ) {
      zctx->mz_err = mz_err; zctx->ws_err = L"stream error";
      return false;
    }

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmArchive::write(const OmWString& path, int32_t method, int32_t level)
{
  // close and reset interface if any
  this->close();

  zip_context_t* zctx = static_cast<zip_context_t*>(this->_zctx);

  // let be it a writer
  this->_stat = ZIP_WRITER;

  // create zip writer architecture
  zctx->strm_file = mz_stream_os_create();
  if(!zctx->strm_file) {
    zctx->mz_err = MZ_MEM_ERROR; zctx->ws_err = L"stream OS create error";
    this->close();
    return false; // MZ_MEM_ERROR;
  }

  zctx->strm_buff = mz_stream_buffered_create();
  if(!zctx->strm_buff) {
    zctx->mz_err = MZ_MEM_ERROR; zctx->ws_err = L"stream buffered create error";
    this->close();
    return false; // MZ_MEM_ERROR;
  }

  zctx->strm_splt = mz_stream_split_create();
  if(!zctx->strm_splt) {
    zctx->mz_err = MZ_MEM_ERROR; zctx->ws_err = L"stream split create error";
    this->close();
    return false; // MZ_MEM_ERROR;
  }

  mz_stream_set_base(zctx->strm_buff, zctx->strm_file);
  mz_stream_set_base(zctx->strm_splt, zctx->strm_buff);

  mz_stream_split_set_prop_int64(zctx->strm_splt, MZ_STREAM_PROP_DISK_SIZE, 0);

  zctx->zip_hnd = mz_zip_create();
  if(!zctx->zip_hnd) {
    zctx->mz_err = MZ_MEM_ERROR; zctx->ws_err = L"zip handle create error";
    this->close();
    return false; // MZ_MEM_ERROR;
  }

  int32_t mz_err = MZ_OK;

  OmWString path_dir = Om_getDirPart(path);

  // create the path tree if required
  if(!Om_isDir(path_dir)) {
    // we simply create directory
    mz_err = Om_dirCreateRecursive(path_dir);
    if(mz_err != ERROR_SUCCESS) {
      zctx->mz_err = mz_err;  zctx->ws_err = L"create directory error";
      return false;
    }
  }

  // we never append to file, maybe later implementation
  int32_t mode = MZ_OPEN_MODE_READWRITE|MZ_OPEN_MODE_CREATE;

  zctx->cmp_level = level;
  zctx->cmp_method = method;

  OmCString utf8_path;
  Om_toUTF8(&utf8_path, path);

  mz_err = mz_stream_open(zctx->strm_splt, utf8_path.c_str(), mode);
  if(mz_err != MZ_OK) {
    this->close();
    zctx->mz_err = mz_err;  zctx->ws_err = L"open stream error";
    return false;
  }

  mz_err = mz_zip_open(zctx->zip_hnd, zctx->strm_splt, mode);
  if(mz_err != MZ_OK) {
    this->close();
    zctx->mz_err = mz_err;  zctx->ws_err = L"zip file open error";
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmArchive::entryAdd(const OmWString& src, const OmWString& dst, Om_progressCb progress_cb, void* user_ptr) const
{
  int32_t mz_err;

  if(this->_stat & ZIP_WRITER) {

    zip_context_t* zctx = static_cast<zip_context_t*>(this->_zctx);

    mz_zip_file file_info;
    memset(&file_info, 0, sizeof(file_info));

    OmCString utf8_src, zcdr_dst;

    Om_toUTF8(&utf8_src, src);
    Om_toZipCDR(&zcdr_dst, dst);

    /* Get information about the file on disk so we can store it in zip */
    file_info.version_madeby = MZ_VERSION_MADEBY;
    file_info.compression_method = zctx->cmp_method;

    file_info.filename = zcdr_dst.c_str();
    file_info.uncompressed_size = mz_os_get_file_size(utf8_src.c_str());
    file_info.flag = MZ_ZIP_FLAG_UTF8;
    mz_os_get_file_date(utf8_src.c_str(), &file_info.modified_date, &file_info.accessed_date, &file_info.creation_date);
    mz_os_get_file_attribs(utf8_src.c_str(), &file_info.external_fa);

    void *stream = nullptr;

    if(!Om_isDir(src)) {

      stream = mz_stream_os_create();
      if(!stream) {
        zctx->mz_err = MZ_MEM_ERROR;  zctx->ws_err = L"create stream OS error";
        return false;
      }

      mz_err = mz_stream_os_open(stream, utf8_src.c_str(), MZ_OPEN_MODE_READ);
      if(!stream) {
        zctx->mz_err = MZ_MEM_ERROR;  zctx->ws_err = L"stream open error";
        return false;
      }
    }

    // Add to zip
    mz_err = mz_zip_entry_write_open(zctx->zip_hnd, &file_info, zctx->cmp_level, 0, nullptr);
    if(mz_err != MZ_OK) {
      zctx->mz_err = mz_err;  zctx->ws_err = L"entry write open error";
      if(stream) {
        mz_stream_close(stream);
        mz_stream_delete(&stream);
      }
      return false;
    }

    // if source is not directory, compress and write data
    if(mz_zip_attrib_is_dir(file_info.external_fa, file_info.version_madeby) != MZ_OK) {

      int32_t wb = 0;
      int32_t rb = 0;

      while(mz_err == MZ_OK) {
        rb = mz_stream_read(stream, zctx->buffer, sizeof(zctx->buffer));
        if(rb > 0) {
            wb = mz_zip_entry_write(zctx->zip_hnd, zctx->buffer, rb);
            if(wb != rb) {
              mz_err = MZ_WRITE_ERROR;
              break;
            }
        } else if(rb < 0) {
          mz_err = rb;
          break;
        } else {
          mz_err = MZ_END_OF_STREAM;
          break;
        }

        if(progress_cb) {
          progress_cb(user_ptr, file_info.uncompressed_size, rb, 0);
        }
      }
    }

    if(stream) {
      mz_stream_close(stream);
      mz_stream_delete(&stream);
    }

    if(mz_err != MZ_OK && mz_err != MZ_END_OF_STREAM ) {
      zctx->mz_err = mz_err; zctx->ws_err = L"stream error";
      return false;
    }

    mz_err = mz_zip_entry_close(zctx->zip_hnd);
    if(mz_err != MZ_OK && mz_err != MZ_END_OF_STREAM ) {
      zctx->mz_err = mz_err; zctx->ws_err = L"entry close error";
      return false;
    }

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmArchive::entryAdd(const void* data, uint64_t size, const OmWString& dst, Om_progressCb progress_cb, void* user_ptr) const
{
  int32_t mz_err;

  if(this->_stat & ZIP_WRITER) {

    zip_context_t* zctx = static_cast<zip_context_t*>(this->_zctx);

    mz_zip_file file_info;
    memset(&file_info, 0, sizeof(file_info));

    OmCString zcdr_dst;
    Om_toZipCDR(&zcdr_dst, dst);

    /* Get information about the file on disk so we can store it in zip */
    file_info.version_madeby = MZ_VERSION_MADEBY;
    file_info.compression_method = zctx->cmp_method;
    file_info.filename = zcdr_dst.c_str();
    file_info.uncompressed_size = size;
    file_info.flag = MZ_ZIP_FLAG_UTF8;
    time_t time_now = time(nullptr);
    file_info.creation_date = time_now;
    file_info.modified_date = time_now;
    file_info.accessed_date = time_now;

    // by convention, null size mean directory entry
    if(size > 0) {
      file_info.external_fa = 0x80; // FILE_ATTRIBUTE_NORMAL
    } else {
      file_info.external_fa = 0x10; // FILE_ATTRIBUTE_DIRECTORY
    }

    // Create a memory stream backed by our buffer and add from it
    void *stream = nullptr;

    stream = mz_stream_mem_create();
    if(!stream) {
      zctx->mz_err = MZ_STREAM_ERROR;  zctx->ws_err = L"create stream mem error";
      return false;
    }

    mz_stream_mem_set_buffer(stream, const_cast<void*>(data), size);

    mz_err = mz_stream_mem_open(stream, nullptr, MZ_OPEN_MODE_READ);
    if(mz_err != MZ_OK) {
      zctx->mz_err = mz_err;  zctx->ws_err = L"stream mem open error";
      mz_stream_close(stream);
      mz_stream_delete(&stream);
      return false;
    }

    // Add to zip
    mz_err = mz_zip_entry_write_open(zctx->zip_hnd, &file_info, zctx->cmp_level, 0, nullptr);
    if(mz_err != MZ_OK) {
      zctx->mz_err = mz_err;  zctx->ws_err = L"entry write open error";
      mz_stream_close(stream);
      mz_stream_delete(&stream);
      return false;
    }

    // if source is not directory, compress and write data
    if(mz_zip_attrib_is_dir(file_info.external_fa, file_info.version_madeby) != MZ_OK) {

      int32_t wb = 0;
      int32_t rb = 0;

      while(mz_err == MZ_OK) {
        rb = mz_stream_mem_read(stream, zctx->buffer, sizeof(zctx->buffer));
        if(rb > 0) {
            wb = mz_zip_entry_write(zctx->zip_hnd, zctx->buffer, rb);
            if(wb != rb) {
              mz_err = MZ_WRITE_ERROR;
              break;
            }
        } else if(rb < 0) {
          mz_err = rb;
          break;
        } else {
          mz_err = MZ_END_OF_STREAM;
          break;
        }

        if(progress_cb) {
          progress_cb(user_ptr, file_info.uncompressed_size, rb, 0);
        }
      }
    }

    if(stream) {
      mz_stream_close(stream);
      mz_stream_delete(&stream);
    }

    if(mz_err != MZ_OK && mz_err != MZ_END_OF_STREAM ) {
      zctx->mz_err = mz_err; zctx->ws_err = L"stream error";
      return false;
    }

    mz_err = mz_zip_entry_close(zctx->zip_hnd);
    if(mz_err != MZ_OK && mz_err != MZ_END_OF_STREAM ) {
      zctx->mz_err = mz_err; zctx->ws_err = L"entry close error";
      return false;
    }

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint32_t OmArchive::entryLocate(const OmWString& entry) const
{
  zip_entry_t* zent = static_cast<zip_entry_t*>(this->_zent);

  for(size_t i = 0; i < this->_zent_size; ++i) {
    if(Om_namesMatches(zent[i].file_path, entry))
      return i;
  }

  return -1;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmArchive::close()
{
  int32_t mz_err;

  if(this->_zent) {
    Om_free(this->_zent);
    this->_zent = nullptr;
  }

  zip_context_t* zctx = static_cast<zip_context_t*>(this->_zctx);

  if(this->_stat & ZIP_READER) {

    zip_context_t* zctx = static_cast<zip_context_t*>(this->_zctx);

    if(zctx->zip_hnd) {
      mz_err = mz_zip_close(zctx->zip_hnd);
      mz_zip_delete(&zctx->zip_hnd);
    }

    if(zctx->strm_splt) {
      mz_stream_split_close(zctx->strm_splt);
      mz_stream_split_delete(&zctx->strm_splt);
    }

    if(zctx->strm_buff)
      mz_stream_buffered_delete(&zctx->strm_buff);

    if(zctx->strm_file)
      mz_stream_os_delete(&zctx->strm_file);
  }

  if(this->_stat & ZIP_WRITER) {

    zip_context_t* zctx = static_cast<zip_context_t*>(this->_zctx);

    if(zctx->zip_hnd) {

      mz_zip_set_version_madeby(zctx->zip_hnd, MZ_VERSION_MADEBY);
      mz_err = mz_zip_close(zctx->zip_hnd);
      mz_zip_delete(&zctx->zip_hnd);
    }

    if(zctx->strm_splt) {
      mz_stream_split_close(zctx->strm_splt);
      mz_stream_split_delete(&zctx->strm_splt);
    }

    if(zctx->strm_buff)
      mz_stream_buffered_delete(&zctx->strm_buff);

    if(zctx->strm_file)
      mz_stream_os_delete(&zctx->strm_file);

  }

  this->_stat = 0;

  if(mz_err != MZ_OK) {
    zctx->mz_err = mz_err; zctx->ws_err = L"zip close error";
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString OmArchive::lastErrorStr() const
{
  OmWString err_str;

  zip_context_t* zctx = static_cast<zip_context_t*>(this->_zctx);

  err_str = zctx->ws_err + L" (";

  if(zctx->mz_err < 0) {
    switch(zctx->mz_err)
    {
    case MZ_STREAM_ERROR: err_str += L"STREAM_ERROR"; break;
    case MZ_DATA_ERROR: err_str += L"DATA_ERROR"; break;
    case MZ_MEM_ERROR: err_str += L"MEM_ERROR"; break;
    case MZ_BUF_ERROR: err_str += L"BUF_ERROR"; break;
    case MZ_VERSION_ERROR: err_str += L"VERSION_ERROR"; break;
    case MZ_END_OF_LIST: err_str += L"END_OF_LIST"; break;
    case MZ_END_OF_STREAM: err_str += L"END_OF_STREAM"; break;
    case MZ_PARAM_ERROR: err_str += L"PARAM_ERROR"; break;
    case MZ_FORMAT_ERROR: err_str += L"FORMAT_ERROR"; break;
    case MZ_INTERNAL_ERROR: err_str += L"INTERNAL_ERROR"; break;
    case MZ_CRC_ERROR: err_str += L"CRC_ERROR"; break;
    case MZ_CRYPT_ERROR: err_str += L"CRYPT_ERROR"; break;
    case MZ_EXIST_ERROR: err_str += L"EXIST_ERROR"; break;
    case MZ_PASSWORD_ERROR: err_str += L"PASSWORD_ERROR"; break;
    case MZ_SUPPORT_ERROR: err_str += L"SUPPORT_ERROR"; break;
    case MZ_HASH_ERROR: err_str += L"HASH_ERROR"; break;
    case MZ_OPEN_ERROR: err_str += L"OPEN_ERROR"; break;
    case MZ_CLOSE_ERROR: err_str += L"CLOSE_ERROR"; break;
    case MZ_SEEK_ERROR: err_str += L"SEEK_ERROR"; break;
    case MZ_TELL_ERROR: err_str += L"TELL_ERROR"; break;
    case MZ_READ_ERROR: err_str += L"READ_ERROR"; break;
    case MZ_WRITE_ERROR: err_str += L"WRITE_ERROR"; break;
    case MZ_SIGN_ERROR: err_str += L"SIGN_ERROR"; break;
    case MZ_SYMLINK_ERROR: err_str += L"SYMLINK_ERROR"; break;
    default: err_str += L"DEFAULT_ERROR"; break;
    }
  } else {
    err_str += Om_getErrorStr(zctx->mz_err);
  }

  err_str +=  L")";

  return err_str;
}
