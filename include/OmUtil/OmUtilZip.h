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
#ifndef OMUTILZIP_H
#define OMUTILZIP_H

/// \brief Compress data
///
/// Compress the supplied data using Deflate algorithm.
///
/// \param[out] out_size  : Output compressed data size in bytes.
/// \param[in]  in_data   : Input data to be compressed.
/// \param[in]  in_size   : Input data size in bytes.
/// \param[in]  level     : Deflate compression level 0 to 9.
///
/// \return Pointer to compressed data or nullptr if failed.
///
uint8_t* Om_zDeflate(size_t* out_size, const uint8_t* in_data, size_t in_size, unsigned level = 9);

/// \brief Uncompress data
///
/// Uncompress the supplied data using Deflate algorithm.
///
/// \param[in]  in_data   : Input data to be decompressed.
/// \param[in]  in_size   : Input data size in bytes.
/// \param[in]  def_size  : Original size of decompressed data (must have been stored before compression)
///
/// \return Pointer to uncompressed data or nullptr if failed.
///
uint8_t* Om_zInflate(const uint8_t* in_data, size_t in_size, size_t def_size);

#endif // OMUTILZIP_H
