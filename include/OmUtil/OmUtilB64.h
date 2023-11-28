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
#ifndef OMUTILBASE64_H
#define OMUTILBASE64_H

#include "OmBase.h"

/// \brief Encode bytes to Base64.
///
/// Encode the given binary data to Base64 string.
///
/// \param[in]  data    : Data to encode.
/// \param[in]  size    : data size in bytes.
///
/// \return String containing Base64 encoded data
///
OmWString Om_toBase64(const uint8_t* data, size_t size);

/// \brief Encode bytes to Base64.
///
/// Encode the given binary data to Base64 string.
///
/// \param[out] b64     : String to get result.
/// \param[in]  data    : Data to encode.
/// \param[in]  size    : data size in bytes.
///
void Om_toBase64(OmWString& b64, const uint8_t* data, size_t size);

/// \brief Decode Base64 to bytes.
///
/// Decode the given Base64 string to binary data.
///
/// \param[in]  size    : Pointer to receive decoded data size
/// \param[out] b64     : Base64 string to decode.
///
/// \return Pointer to decoded data.
///
uint8_t* Om_fromBase64(size_t* size, const OmWString& b64);

/// \brief Format to Base64 encoded Data URI.
///
/// Format the given data to Base64 encoded Data URI.
///
/// \param[out] uri       : String to get result.
/// \param[in]  mime_type : Data URI media type to set.
/// \param[in]  charset   : Optional text charset to define.
/// \param[in]  data      : Data to encode.
/// \param[in]  size      : data size in bytes.
///
void Om_encodeDataUri(OmWString& uri, const OmWString& mime_type, const OmWString& charset, const uint8_t* data, size_t size);

/// \brief Get data from Data URI.
///
/// Get decoded data from Data URI
///
/// \param[out] size      : Pointer to receive decoded data size
/// \param[out] mime_type : String to receive data type
/// \param[out] charset   : Text charset if any
/// \param[in]  uri       : Data URI string to parse
///
/// \return Pointer to decoded data.
///
uint8_t* Om_decodeDataUri(size_t* size, OmWString& mime_type, OmWString& charset, const OmWString& uri);

#endif // OMUTILBASE64_H
