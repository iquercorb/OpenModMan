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
#ifndef OMUTILHASH_H
#define OMUTILHASH_H

#include "OmBase.h"

/// \brief Get hexadecimal string representation.
///
/// Create hexadecimal string representation of the given bytes sequence in
/// big-endian way.
///
/// \param[in]  dest    : Pointer to string that receive result.
/// \param[in]  data    : Data to create representation.
/// \param[in]  size    : Size of data in bytes.
///
void Om_bytesToStrBe(OmWString* dest, const uint8_t* data, size_t size);

/// \brief Get hexadecimal string representation.
///
/// Create hexadecimal string representation of the given bytes sequence in
/// little-endian way.
///
/// \param[in]  dest    : Pointer to string that receive result.
/// \param[in]  data    : Data to create representation.
/// \param[in]  size    : Size of data in bytes.
///
void Om_bytesToStrLe(OmWString* dest, const uint8_t* data, size_t size);

/// \brief Get string representation of a 64 bits integer.
///
/// Returns the hexadecimal representation of the given 64 bits unsigned
/// integer value as a wide string.
///
/// \param[in]  num     : 64 bits unsigned integer.
///
/// \return Hexadecimal string representation of 64 bits integer
///
OmWString Om_uint64ToStr(uint64_t num);

/// \brief Get string representation of a 64 bits integer.
///
/// Set the specified wind string to the hexadecimal representation of the
/// given 64 bits unsigned integer value.
///
/// \param[out] str     : Pointer to string that receive result.
/// \param[in]  num     : 64 bits unsigned integer.
///
void Om_uint64ToStr(OmWString* str, uint64_t num);

/// \brief Get 64 bits integer from string.
///
/// Returns the 64 bits unsigned integer value of the given hexadecimal number
/// string representation.
///
/// \param[in]  str     : Hexadecimal number string to parse.
///
/// \return Converted 64 bits integer value.
///
uint64_t Om_strToUint64(const OmWString& str);

/// \brief Get 64 bits integer from string.
///
/// Returns the 64 bits unsigned integer value of the given
/// hexadecimal number string representation.
///
/// \param[in]  str     : Hexadecimal number string to parse.
///
/// \return Converted 64 bits integer value.
///
uint64_t Om_strToUint64(const wchar_t* str);

/// \brief Compute XXH3 Hash.
///
/// Calculates and returns 64 bits unsigned integer hash (XXH3) of the given
/// data chunk.
///
/// \param[in]  data    : Data to compute Hash.
/// \param[in]  size    : Size of data in bytes.
///
/// \return Resulting 64 bits unsigned integer hash.
///
uint64_t Om_getXXHash3(const void* data, size_t size);

/// \brief Compute XXHash3 Hash.
///
/// Calculates and returns 64 bits unsigned integer hash (XXHash3) of the given
/// wide string.
///
/// \param[in]  str    : Wide string to compute Hash.
///
/// \return Resulting 64 bits unsigned integer hash.
///
uint64_t Om_getXXHash3(const OmWString& str);

/// \brief Compute XXHash3 digest from file.
///
/// Calculates and returns 64 bits unsigned integer digest (XXHash3) of the
/// given file.
///
/// \param[in]  xxh   : Pointer to uint64_t that receive XXHash3 value;
/// \param[in]  path  : Path to file to compute XXHash3 digest.
///
/// \return True if operation succeed, false if open file error.
///
bool Om_getXXHdigest(uint64_t* xxh, const OmWString& path);

/// \brief Get file XXHash3 checksum.
///
/// Calculates and returns the 16 characters hash string of the
/// given data.
///
/// \param[in]  path    : Path to file to generate checksum.
///
/// \return 16 hexadecimal characters hash string.
///
OmWString Om_getXXHsum(const OmWString& path);

/// \brief Get file XXHash3 checksum.
///
/// Calculates the 16 characters hash string of the given data.
///
/// \param[in]  pstr    : String to set as checksum string.
/// \param[in]  path    : Path to file to generate checksum.
///
/// \return True if operation succeed, false if open file error.
///
bool Om_getXXHsum(OmWString* pstr, const OmWString& path);

/// \brief Compare file XXHash3 checksum.
///
/// Calculates the 16 characters hash string of the given data.
///
/// \param[in]  path    : Path to file to generate checksum.
/// \param[in]  str     : Checksum hexadecimal string to compare.
///
/// \return true if checksum matches, false otherwise
///
bool Om_cmpXXHsum(const OmWString& path, const OmWString& str);

/// \brief Get file MD5 digest.
///
/// Compute the MD5 digest of the given file.
///
/// \param[in]  md5     : Pointer to 16 bytes buffer that receive MD5 digest.
/// \param[in]  path    : Path to file to generate checksum.
///
/// \return True if operation succeed, false if open file error.
///
bool Om_getMD5digest(uint8_t* md5, const OmWString& path);

/// \brief Get file MD5 checksum.
///
/// Calculates and returns the 32 characters MD5 digest string of the
/// given data.
///
/// \param[in]  path    : Path to file to generate checksum.
///
/// \return MD5 checksum string.
///
OmWString Om_getMD5sum(const OmWString& path);

/// \brief Get file MD5 checksum.
///
/// Calculates the 32 characters MD5 digest string of the given data.
///
/// \param[in]  pstr    : Pointer to string that receive MD5 checksum string.
/// \param[in]  path    : Path to file to generate checksum.
///
/// \return True if operation succeed, false if open file error.
///
bool Om_getMD5sum(OmWString* pstr, const OmWString& path);

/// \brief Compare file MD5 checksum.
///
/// Calculates the 32 characters MD5 digest string of the given data.
///
/// \param[in]  path    : Path to file to generate checksum.
/// \param[in]  str     : Checksum hexadecimal string to compare.
///
/// \return true if checksum matches, false otherwise
///
bool Om_cmpMD5sum(const OmWString& path, const OmWString& str);

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
uint64_t Om_getCRC64(const OmWString& str);

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
OmWString Om_genUUID();

#endif // OMUTILHASH_H
