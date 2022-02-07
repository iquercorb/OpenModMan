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
#ifndef OMBASE_H_INCLUDED
#define OMBASE_H_INCLUDED

#include <string>
#include <vector>

#define OMM_MAX_PATH  1024
#define OMM_ITM_BUFF  256

#ifdef DEBUG
// sleep value slow mod
#include <iostream>
#define OMM_DEBUG_SLOW            50
#endif

using namespace std;

/// \brief Progress callback.
///
/// Generic callback function for process progression.
///
/// \param[in]  ptr   : User data pointer.
/// \param[in]  tot   : Number total count of items.
/// \param[in]  cur   : Number processed item count.
/// \param[in]  data  : Custom internal opaque data.
///
/// \return True to continue, false to abort process.
///
typedef bool (*Om_progressCb)(void* ptr, size_t tot, size_t cur, uint64_t data);

/// \brief Progress callback.
///
/// Generic callback function for process progression.
///
/// \param[in]  ptr   : User data pointer.
/// \param[in]  tot   : Total bytes to download.
/// \param[in]  cur   : Bytes downloaded.
/// \param[in]  spd   : Download speed.
/// \param[in]  data  : Custom internal opaque data.
///
/// \return True to continue, false to abort process.
///
typedef bool (*Om_downloadCb)(void* ptr, double tot, double cur, double spd, uint64_t data);

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

/// \brief Free allocated data.
///
/// Properly free a previously allocated data using Om_alloc.
///
/// \param[in]  data  : Previously allocated to free.
///
inline void Om_free(void* data) {
  if(data) free(data);
}

#endif // OMBASE_H_INCLUDED
