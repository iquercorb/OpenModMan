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
#include "OmBase.h"           //< string, vector, Om_alloc, OM_MAX_PATH, etc.
#include <regex>


///
///  -  -  -  -  -  -  -  -  - Base64 implementation  -  -  -  -  -  -  -  -  -
///
static const wchar_t __b64_enc_table[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const uint8_t __b64_dec_table[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,62, 0, 0, 0,63,52,53,54,55,56,57,58,59,60,61, 0, 0, 0, 0, 0, 0,
                                          0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25, 0, 0, 0, 0, 0,
                                          0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51, 0, 0, 0, 0, 0};

/// \brief Base64 encode.
///
/// Encode given data to Base64 string.
///
/// \param[out] out_b64 : Output Base64 encoding result string.
/// \param[in]  in_data : Input data to encode.
/// \param[in]  in_size : Input size of data in bytes.
///
static inline void __base64_encode(OmWString& out_b64, const uint8_t* in_data, size_t in_size)
{
  // compute string size now
  size_t size = 4 * ((in_size + 2) / 3);

  // reserve buffer for encoded data
  out_b64.clear();
  out_b64.reserve(size);

  uint8_t b[3];
  uint32_t t;

  // main block, per triplets
  for(unsigned i = 0; i < in_size; ) {
    b[0] = (i < in_size) ? in_data[i++] : 0;
    b[1] = (i < in_size) ? in_data[i++] : 0;
    b[2] = (i < in_size) ? in_data[i++] : 0;
    t = (b[0] << 0x10) + (b[1] << 0x08) + b[2];
    out_b64.push_back(__b64_enc_table[0x3F & (t >> 18)]);
    out_b64.push_back(__b64_enc_table[0x3F & (t >> 12)]);
    out_b64.push_back(__b64_enc_table[0x3F & (t >>  6)]);
    out_b64.push_back(__b64_enc_table[0x3F & (t)]);
  }

  unsigned r = in_size % 3; //< remaining bytes after per-triplet division
  if(r > 0) {
    for(unsigned i = 0; i < 3 - r; ++i)
      out_b64[(size - 1) - i] = L'=';
  }
}


/// \brief Base64 decode.
///
/// Decode Base64 string to buffer.
///
/// \param[out] out_size  : Output size of decoded data in bytes.
/// \param[in]  in_b64    : Input Base64 string to decode.
///
/// \return Pointer to decoded data.
///
static inline uint8_t* __base64_decode(size_t* out_size, const OmWString& in_b64)
{
  // check whether input data is valid
  if(in_b64.size() % 4 != 0)
    return nullptr;

  // compute output data size now
  size_t size = (in_b64.size() / 4) * 3;
  if(in_b64[in_b64.size() - 1] == '=') size--;
  if(in_b64[in_b64.size() - 2] == '=') size--;

  // allocate output data buffer
  uint8_t* data = reinterpret_cast<uint8_t*>(Om_alloc(size));
  if(!data) return nullptr;

  // decode data
  uint32_t t;
  uint8_t s[4];

  for(unsigned i = 0, j = 0; i < in_b64.size(); ) {
    s[0] = (in_b64[i] == L'=')? 0 : __b64_dec_table[static_cast<uint8_t>(in_b64[i])]; i++;
    s[1] = (in_b64[i] == L'=')? 0 : __b64_dec_table[static_cast<uint8_t>(in_b64[i])]; i++;
    s[2] = (in_b64[i] == L'=')? 0 : __b64_dec_table[static_cast<uint8_t>(in_b64[i])]; i++;
    s[3] = (in_b64[i] == L'=')? 0 : __b64_dec_table[static_cast<uint8_t>(in_b64[i])]; i++;
    t = (s[0] << 18) + (s[1] << 12) + (s[2] << 6) + s[3];
    if(j < size) data[j++] = 0xFF & (t >> 16);
    if(j < size) data[j++] = 0xFF & (t >>  8);
    if(j < size) data[j++] = 0xFF & (t);
  }

  (*out_size) = size;

  return data;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_toBase64(const uint8_t* data, size_t size)
{
  OmWString b64;
  __base64_encode(b64, data, size);
  return b64;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_toBase64(OmWString& b64, const uint8_t* data, size_t size)
{
  __base64_encode(b64, data, size);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_fromBase64(size_t* size, const OmWString& b64)
{
  return __base64_decode(size, b64);
}

/// \brief Data URI Regex pattern
///
/// Regular expression pattern for Data URI.
///
//static const std::wregex __data_uri_reg(LR"(^(data:)([\w\/-]+);([\w]+)=?([\w\d-]+)?,([\w\W]+))"); // cause crash
static const std::wregex __data_uri_reg(LR"(^(data:)([\w\/-]+);([\w]+)=?([\w\d-]+)?,)");


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_encodeDataUri(OmWString& uri, const OmWString& mime_type, const OmWString& charset, const uint8_t* data, size_t size)
{
  // compose final data URI string
  uri.clear();
  uri.append(L"data:");
  uri.append(mime_type);

  OmWString uri_data;

  if(!charset.empty()) {
    uri.append(L";charset=");
    uri.append(charset);
    // convert data to OmWString
    for(unsigned i = 0; i < size; ++i)
      uri_data.push_back(static_cast<wchar_t>(data[i]));
  } else {
    uri.append(L";base64");
    // encode binary data to base64
    __base64_encode(uri_data, data, size);
  }

  // finally append data
  uri.push_back(L',');
  uri.append(uri_data);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_decodeDataUri(size_t* size, OmWString& mime_type, OmWString& charset, const OmWString& uri)
{
  // initialize values
  (*size) = 0;
  mime_type = L"";
  charset = L"";

  // check for regex matches
  std::match_results<const wchar_t*> matches;
  if(std::regex_search(uri.c_str(), matches, __data_uri_reg)) {

    // matches :
    // 0) full match
    // 1) data:
    // 2) mime-type (eg. image/jpeg)
    // 3) encoding (eg. base64) or literally "charset"
    // 4) used charset (eg. UTF-8)
    // suffix()) the data...

    // search for full match
    if(matches.size()) {

      // set data mime type
      mime_type = matches[2];

      // check whether we got a charset
      if(matches[3] == L"charset") {

        // set the parsed charset
        charset = matches[4];

        // get data as wide string
        OmWString wdata = matches.suffix();

        // allocate new buffer to hold data
        uint8_t* ascii = reinterpret_cast<uint8_t*>(Om_alloc(wdata.size()));
        if(!ascii) return nullptr;

        // convert the wchar_t to uint8_t by value. Since
        // plain text data URI should always have 8 bits
        // content, this should be OK
        for(unsigned i = 0; i < wdata.size(); ++i)
          ascii[i] = wdata[i];

        (*size) = wdata.size();

        return ascii;

      } else if(matches[3] == L"base64") {
        // decode base64 data
        return __base64_decode(size, matches.suffix());
      }
    }
  }

  return nullptr;
}
