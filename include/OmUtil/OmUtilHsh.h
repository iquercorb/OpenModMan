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
#ifndef OMUTILHASH_H_INCLUDED
#define OMUTILHASH_H_INCLUDED

#include "OmBase.h"

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
/// \param[in]  str    : Wide string to compute Hash.
///
/// \return Resulting 64 bits unsigned integer hash.
///
uint64_t Om_getXXHash3(const wstring& str);

/// \brief Get file checksum.
///
/// Calculates and returns the 16 characters hash string of the
/// given data.
///
/// \param[in]  path    : Path to file to generate checksum.
///
/// \return 16 hexadecimal characters hash string.
///
wstring Om_getXXHsum(const wstring& path);

/// \brief Get file checksum.
///
/// Calculates the 16 characters hash string of the given data.
///
/// \param[in]  hex     : String to set as checksum string.
/// \param[in]  path    : Path to file to generate checksum.
///
/// \return True if operation succeed, false otherwise
///
bool Om_getXXHsum(wstring& hex, const wstring& path);

/// \brief Compare file checksum.
///
/// Calculates the 16 characters hash string of the given data.
///
/// \param[in]  path    : Path to file to generate checksum.
/// \param[in]  hex     : Checksum hexadecimal string to compare.
///
/// \return true if checksum matches, false otherwise
///
bool Om_cmpXXHsum(const wstring& path, const wstring& hex);

/// \brief Get file MD5 checksum.
///
/// Calculates and returns the 32 characters MD5 digest string of the
/// given data.
///
/// \param[in]  path    : Path to file to generate checksum.
///
/// \return 32 hexadecimal characters hash string.
///
wstring Om_getMD5sum(const wstring& path);

/// \brief Get file MD5 checksum.
///
/// Calculates the 32 characters MD5 digest string of the given data.
///
/// \param[in]  hex     : String to set as MD5 checksum string.
/// \param[in]  path    : Path to file to generate checksum.
///
/// \return True if operation succeed, false otherwise
///
bool Om_getMD5sum(wstring& hex, const wstring& path);

/// \brief Compare file MD5 checksum.
///
/// Calculates the 32 characters MD5 digest string of the given data.
///
/// \param[in]  path    : Path to file to generate checksum.
/// \param[in]  hex     : Checksum hexadecimal string to compare.
///
/// \return true if checksum matches, false otherwise
///
bool Om_cmpMD5sum(const wstring& path, const wstring& hex);

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
  wchar_t num_buf[17];
  swprintf(num_buf, 17, L"%016llx", num);
  return wstring(num_buf);
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
  wchar_t num_buf[17];
  swprintf(num_buf, 17, L"%016llx", num);
  str = num_buf;
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

#endif // OMUTILHASH_H_INCLUDED
