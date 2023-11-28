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
#ifndef OMBASE_H
#define OMBASE_H

#include <cstring>              //< memset

#include <string>
#include <vector>
#include <deque>
#include <map>

#define OM_MAX_PATH       1500    //< hope this will be sufficient for the next 10 years...
#define OM_MAX_ITEM       500     //< most screens are not large enough to print 250 chars in single line...

#define OM_LOG_OK           2     //< Log level Info/Notice
#define OM_LOG_WRN          1     //< Log level Warning
#define OM_LOG_ERR          0     //< Log level Error

#ifdef DEBUG
#include <iostream>
#endif

//using namespace std;

#define OM_HAS_BIT(a, b)  (b == (a & b))
#define OM_ADD_BIT(a, b)  (a |= b)
#define OM_REM_BIT(a, b)  (a &= ~ b)
#define OM_TOG_BIT(a, b)  (a ^= b)

/// \brief STL C string
///
/// Typedef for an STL char string
///
typedef std::string OmCString;

/// \brief STL wstring
///
/// Typedef for an STL wide char string
///
typedef std::wstring OmWString;

/// \brief STL wstring array
///
/// Typedef for an STL vector of STL wide char string type
///
typedef std::vector<OmWString> OmWStringArray;

/// \brief uint64_t array
///
/// Typedef for an STL vector of uint64_t type
///
typedef std::vector<uint64_t> OmUint64Array;

/// \brief Indexes array
///
/// Typedef for an STL vector of uint32_t type
///
typedef std::vector<uint32_t> OmIndexArray;

/// \brief Result codes
///
/// Enumerator for result code.
///
enum OmResult : int32_t
{
  OM_RESULT_OK            = 0,
  OM_RESULT_ERROR         = 0x1,
  OM_RESULT_ABORT         = 2,
  OM_RESULT_ERROR_IO      = 0x3,
  OM_RESULT_ERROR_ALLOC   = 0x5,
  OM_RESULT_ERROR_PARSE   = 0x7,
  OM_RESULT_UNKNOW        = -1,
  OM_RESULT_PENDING       = -2,
};

/// \brief List sorting
///
/// Enumerator for list sorting by attribute
///
enum OmSort : int32_t
{
  OM_SORT_STAT = 0x01,
  OM_SORT_NAME = 0x02,
  OM_SORT_VERS = 0x04,
  OM_SORT_SIZE = 0x08,
  OM_SORT_CATE = 0x10,
  OM_SORT_INVT = 0x100
};

/// \brief Progress callback.
///
/// Generic callback function for process progression.
///
/// \param[in]  ptr   : User data pointer.
/// \param[in]  tot   : Number total count of items.
/// \param[in]  cur   : Number processed item count.
/// \param[in]  param : Context dependent extra parameter.
///
/// \return True to continue, false to abort process.
///
typedef bool (*Om_progressCb)(void* ptr, size_t tot, size_t cur, uint64_t param);

/// \brief Download callback.
///
/// Generic callback function for download progression.
///
/// \param[in]  ptr   : User data pointer.
/// \param[in]  tot   : Total bytes to download.
/// \param[in]  cur   : Bytes downloaded.
/// \param[in]  spd   : Download speed.
/// \param[in]  param : Context dependent extra parameter.
///
/// \return True to continue, false to abort process.
///
typedef bool (*Om_downloadCb)(void* ptr, int64_t tot, int64_t cur, int64_t spd, uint64_t param);

/// \brief Response callback.
///
/// Generic callback function for request response.
///
/// \param[in]  ptr   : User data pointer.
/// \param[in]  len   : Response data size in bytes
/// \param[in]  buf   : Response data buffer
/// \param[in]  param : Context dependent extra parameter.
///
typedef void (*Om_responseCb)(void* ptr, uint8_t* buf, uint64_t len, uint64_t param);

/// \brief Result callback.
///
/// Generic callback function for request result.
///
/// \param[in]  ptr     : User data pointer
/// \param[in]  result  : Result value
/// \param[in]  param   : Context dependent extra parameter.
///
typedef void (*Om_resultCb)(void* ptr, OmResult result, uint64_t param);

/// \brief Begin callback.
///
/// Generic callback function for request begin.
///
/// \param[in]  ptr     : User data pointer
/// \param[in]  param   : Context dependent extra parameter.
///
typedef void (*Om_beginCb)(void* ptr, uint64_t param);

/// \brief Memory allocation.
///
/// Allocate new buffer of the specified size. Allocated data
/// must be free using the Om_free function.
///
/// \param[in]  size  : Size to allocate in bytes.
///
/// \return Pointer to allocated buffer or nullptr if failed.
///
inline void* Om_alloc(size_t size) {
  return malloc(size);
}

/// \brief Reallocate memory.
///
/// Changes the size of the given memory block to the
/// specified size.
///
/// \param[in] buff : Pointer to a memory block previously allocated with Om_alloc or Om_realloc.
/// \param[in] size : New size for the memory block, in bytes.
///
/// \return Pointer to allocated buffer or nullptr if failed.
///
inline void* Om_realloc(void* buff, size_t size) {
  return realloc(buff, size);
}

/// \brief Copy block of memory
///
/// Copies the values of num bytes from the location pointed to by source
/// directly to the memory block pointed to by destination.
///
/// \param[in]  destination : Pointer to the destination array where the content is to be copied.
/// \param[in]  source      : Pointer to the source of data to be copied.
/// \param[in]  num         : Number of bytes to copy.
///
inline void Om_memcpy(void* destination, void* source, size_t num) {
  memcpy(destination, source, num);
}

/// \brief Fill block of memory
///
///  Sets the first num bytes of the block of memory pointed by ptr to the
/// specified value (interpreted as an unsigned char).
///
/// \param[in]  ptr   : Pointer to the block of memory to fill.
/// \param[in]  value : Value to be set.
/// \param[in]  num   : Number of bytes to be set to the value.
///
inline void Om_memset(void* ptr, uint8_t value, size_t num) {
  memset(ptr, value, num);
}

/// \brief Free allocated data.
///
/// Properly free a previously allocated data using Om_alloc.
///
/// \param[in]  data  : Previously allocated to free.
///
inline void Om_free(void* data) {
  if(data) free(data);
}


#endif // OMBASE_H
