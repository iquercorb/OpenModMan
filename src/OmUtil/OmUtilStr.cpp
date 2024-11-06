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
#include <regex>              //< std::wregex
#include <algorithm>          //< std::replace

#include "OmBaseWin.h"        //< WinAPI
#include <ShlWApi.h>          //< StrFromKBSizeW, etc.

#define READ_BUF_SIZE 524288

/// \brief Hexadecimal digits
///
/// Static translation string to convert integer value to hexadecimal digit.
///
static const wchar_t __hex_digit[] = L"0123456789ABCDEF";

/// \brief Window-1252 to Unicode table
///
/// Translation table to convert Window-125 character to
/// its unicode counterpart.
///
static const uint32_t __cp1252_unicode_map[] = {
  0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
  0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017D, 0x0000,
  0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
  0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x017E, 0x0178};

/// \brief UTF-8 BOM sequence
///
/// Byte sequence to identify UTF-8 Byte Order Mark to be skipped
///
static const uint8_t __utf8_bom[] = {0xEF, 0xBB, 0xBF};

/// \brief Illegal Windows characters
///
/// List of forbidden characters to test validity of file name or path.
///
static const wchar_t __illegal_win_chr[] = L"/*?\"<>|\\";

/// \brief whitespace characters
///
/// List of standard whitespace characters
///
static const wchar_t __wspace_chr[] = L" \r\n\t\xA0";

/// \brief URL Regex pattern
///
/// Regular expression pattern for URL.
///
static const std::wregex __url_reg(LR"(^(https?:\/\/)([\da-z\.-]+\.[\da-z\.-]+)(:[\d]+)?([\/\.\w%-]*\/)?([\w.%-]*)?(\?[\w%-=&]+)?)");

/// \brief Illegal URL path characters
///
/// List of forbidden characters to test validity of URL path.
///
static const wchar_t __illegal_url_chr[] = L" \"\\#<>|{}^[]`+$:@";

/* Deprecated implementations.

/// \brief UTF-8 decode
///
/// Convert the given multibyte string into wide char string assuming UTF-8
/// encoding.
///
/// This function si no longer used in current implementation but conserved
/// here in cas this is needed.
///
/// \param[in]  pwcs    : Wide char string to receive conversion result.
/// \param[out] utf8    : Pointer to null-terminated Multibyte string to convert.
///
inline static void __utf8_decode(OmWString* pwcs, const char* str)
{
  pwcs->clear();
  pwcs->reserve(strlen(str));

  uint32_t u;
  unsigned char c;
  size_t p = 0;

  while(str[p] != 0) {

    c = str[p];

    if(c <= 0x7F) {
      u = c;
    } else if(c <= 0xBF) {
      u = (u << 6) | (c & 0x3F);
    } else if(c <= 0xdf) {
      u = c & 0x1F;
    } else if(c <= 0xEF) {
      u = c & 0x0F;
    } else {
      u = c & 0x07;
    }

    ++p;

    if(((str[p] & 0xC0) != 0x80) && (u <= 0x10FFFF)) {
      if(u > 0xFFFF)  {
        pwcs->push_back(static_cast<wchar_t>(0xD800 + (u >> 10)));
        pwcs->push_back(static_cast<wchar_t>(0xDC00 + (u & 0x03FF)));
      } else if(u < 0xD800 || u >= 0xE000) {
        pwcs->push_back(static_cast<wchar_t>(u));
      }
    }
  }
}
*/

/* Deprecated implementations.

/// \brief UTF-8 encode
///
/// Convert the given wide char string to multibyte string using UTF-8
/// encoding.
///
/// This function si no longer used in current implementation but conserved
/// here in cas this is needed.
///
/// \param[out] utf8    : Multibyte string to receive conversion result.
/// \param[in]  wstr    : Wide char string to convert.
///
inline static void __utf8_encode(OmCString& utf8, const OmWString& wstr)
{
  utf8.clear();
  utf8.reserve(wstr.size() * 4);

  char16_t c;
  uint32_t u;

  for(size_t i = 0; i < wstr.size(); ++i) {

    c = wstr[i];

    if(c >= 0xD800 && c <= 0xDBFF) {
      u = ((c - 0xD800) << 10) + 0x10000;
      continue; //< get next char
    } else if(c >= 0xDC00 && c <= 0xDFFF) {
      u |= c - 0xDC00;
    } else {
      u = c;
    }

    if(u <= 0x7F) {
      utf8.push_back(static_cast<char>(u));
    }else if(u <= 0x7FF) {
      utf8.push_back(static_cast<char>(0xC0 | ((u >> 6) & 0x1F)));
      utf8.push_back(static_cast<char>(0x80 | ( u       & 0x3F)));
    } else if(u <= 0xFFFF) {
      utf8.push_back(static_cast<char>(0xE0 | ((u >> 12) & 0x0F)));
      utf8.push_back(static_cast<char>(0x80 | ((u >>  6) & 0x3F)));
      utf8.push_back(static_cast<char>(0x80 | ( u        & 0x3F)));
    } else {
      utf8.push_back(static_cast<char>(0xF0 | ((u >> 18) & 0x07)));
      utf8.push_back(static_cast<char>(0x80 | ((u >> 12) & 0x3F)));
      utf8.push_back(static_cast<char>(0x80 | ((u >>  6) & 0x3F)));
      utf8.push_back(static_cast<char>(0x80 | ( u        & 0x3F)));
    }

    u = 0;
  }
}
*/

/// \brief Multibyte Decode
///
/// Static inlined function to convert the given multibyte string into wide
/// char string assuming the specified encoding.
///
/// This function use the WinAPI MultiByteToWideChar implementation witch is
/// currently the known fastest way.
///
/// \param[in]  cp      : Code page to use in performing the conversion.
/// \param[in]  pwcs    : Wide char string to receive conversion result.
/// \param[out] utf8    : Pointer to null-terminated Multibyte string to convert.
///
inline static size_t __multibyte_decode(UINT cp, OmWString* pwcs, const char* utf8)
{
  int n = MultiByteToWideChar(cp, 0, utf8, -1, nullptr, 0);
  if(n > 0) {
    pwcs->resize(n - 1);
    // NOTICE: here bellow, the string object is used as C char buffer, in
    // theory this is not allowed since std::string is not required to store
    // its contents contiguously in memory.
    //
    // HOWEVER, in practice, there is no know situation where std::string does
    // not store its content contiguous, so, this should work anyway.
    //
    // If some problem emerge from this function, change this implementation for
    // a more regular approach.
    return static_cast<size_t>(MultiByteToWideChar(cp, 0, utf8, -1, &(*pwcs)[0], n));
  }

  return 0;
}

/// \brief Multibyte encode
///
/// Static inlined function to convert the given wide char string to multibyte
/// string using the specified encoding.
///
/// This function use the WinAPI MultiByteToWideChar implementation witch is
/// currently the known fastest way.
///
/// \param[in]  cp      : Code page to use in performing the conversion.
/// \param[out] utf8    : Multibyte string to receive conversion result.
/// \param[in]  wstr    : Wide char string to convert.
///
inline static size_t __multibyte_encode(UINT cp, OmCString* pstr, const wchar_t* wstr)
{
  int n = WideCharToMultiByte(cp, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
  if(n > 0) {
    pstr->resize(n - 1);
    // NOTICE: here bellow, the string object is used as C char buffer, in
    // theory this is not allowed since std::string is not required to store
    // its contents contiguously in memory.
    //
    // HOWEVER, in practice, there is no know situation where std::string does
    // not store its content contiguous, so, this should work anyway.
    //
    // If some problem emerge from this function, change this implementation for
    // a more regular approach.
    return static_cast<size_t>(WideCharToMultiByte(cp, 0, wstr, -1, &(*pstr)[0], n, nullptr, nullptr));
  }
  return 0;
}

/// \brief Encode data to UTF-16
///
/// Guess the text data encoding and couvert it to UTF-16
///
/// \param[out] pcws  : Pointer to wide string that receive the converted data.
/// \param[in]  data  : Input raw data to decode.
/// \param[in]  size  : Size of input data.
///
/// \return Count of written UTF16 codet.
///
static inline size_t __utf16_encode(OmWString* pwcs, const uint8_t* data, size_t size)
{
  // Macros to check valid unicode trail bytes
  #define IS_UTF8_2BYTES(a) (((a)[1] & 0xC0) == 0x80)
  #define IS_UTF8_3BYTES(a) (((a)[1] & 0xC0) == 0x80) && \
                            (((a)[2] & 0xC0) == 0x80)
  #define IS_UTF8_4BYTES(a) (((a)[1] & 0xC0) == 0x80) && \
                            (((a)[2] & 0xC0) == 0x80) && \
                            (((a)[3] & 0xC0) == 0x80)
  uint32_t u;

  const uint8_t* c;

  size_t off = 0;
  size_t len = 0;

  /* check for UTF-8 BOM to skip */
  if(size > 2 && memcmp(data, __utf8_bom, 3) == 0)
    off += 3;

  while(off < size) {

    c = data + off;

    // checks for non-ASCII value
    if(c[0] > 0x7F) {

      u = 0;

      if(c[0] < 0xA0) { //< Windows-1252 specific range
        u = __cp1252_unicode_map[c[0] - 0x80];
        off++;
      } else if((c[0] & 0xE0) == 0xC0) { //< 2 bytes unicode (110X XXXX)
        if((size - off) > 1) { //< need one more byte
          if(IS_UTF8_2BYTES(c)) {
            u = (c[0] & 0x1F) <<  6 |
                (c[1] & 0x3F);
            off += 2; /* 2 bytes */
          }
        }
      } else if((c[0] & 0xF0) == 0xE0) { //< 2 bytes unicode (1110 XXXX)
        if((size - off) > 2) { //< need 2 more bytes
          if(IS_UTF8_3BYTES(c)) {
            u = (c[0] & 0x0F) << 12 |
                (c[1] & 0x3F) <<  6 |
                (c[2] & 0x3F);
            off += 3; /* 3 bytes */
          }
        }
      } else if((c[0] & 0xF8) == 0xF0) { //< 4 bytes unicode (1111 0XXX)
        if((size - off) > 3) { //< need 3 more bytes
          if(IS_UTF8_2BYTES(c)) {
            u = (c[0] & 0x07) << 18 |
                (c[1] & 0x3F) << 12 |
                (c[2] & 0x3F) <<  6 |
                (c[3] & 0x3F);
            off += 4;
          }
        }
      }

      if(u == 0) { //< ISO 8859-1 (Latin1) or unsupported encoding
        pwcs->push_back(static_cast<wchar_t>(c[0]));
        off++; len++;
      } else if(u > 0xFFFF)  { //< 4 bytes unicode
        pwcs->push_back(static_cast<wchar_t>(0xD800 + (u >> 10)));
        pwcs->push_back(static_cast<wchar_t>(0xDC00 + (u & 0x03FF)));
        len+=2;
      } else if(u < 0xD800 || u >= 0xE000) { //< 2 bytes unicode
        pwcs->push_back(static_cast<wchar_t>(u));
        len++;
      }

    } else {
      pwcs->push_back(static_cast<wchar_t>(c[0]));
      off++; len++;
    }
  }

  return len;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toUTF16(OmWString* pwcs, const OmCString& utf8)
{
  return __multibyte_decode(CP_UTF8, pwcs, utf8.data());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toUTF16(OmWString* pwcs, const char* utf8)
{
  return __multibyte_decode(CP_UTF8, pwcs, utf8);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_toUTF16(const OmCString& utf8)
{
  OmWString result;
  __multibyte_decode(CP_UTF8, &result, utf8.data());
  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_toUTF16(const char* utf8)
{
  OmWString result;
  __multibyte_decode(CP_UTF8, &result, utf8);
  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toUTF16(OmWString* pwcs, const uint8_t* data, size_t size)
{
  return __utf16_encode(pwcs, data, size);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_toUTF16(const uint8_t* data, size_t size)
{
  OmWString result;
  __utf16_encode(&result, data, size);
  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_loadToUTF16(OmWString* result, const OmWString& path)
{
  size_t len = 0;

  // open file for reading
  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return len;

  uint8_t read_buf[READ_BUF_SIZE];
  DWORD rb;

  while(ReadFile(hFile, read_buf, READ_BUF_SIZE, &rb, nullptr)) {

    if(rb == 0)
      break;

    // guess encoding then convert to UTF-16
    len += __utf16_encode(result, read_buf, rb);
  }

  CloseHandle(hFile);

  return len;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toUTF8(char* utf8, size_t len, const OmWString& wstr)
{
  // The WinAPI implementation is the fastest one at this time
  int n = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8, len, nullptr, nullptr);
  return (n > 0) ? static_cast<size_t>(n) : 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmCString Om_toUTF8(const OmWString& wstr)
{
  OmCString result;
  __multibyte_encode(CP_UTF8, &result, wstr.c_str());
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toUTF8(OmCString* utf8, const OmWString& wstr)
{
  return __multibyte_encode(CP_UTF8, utf8, wstr.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toANSI(char* ansi, size_t len, const OmWString& wstr)
{
  // The WinAPI implementation is the fastest one at this time
  int n = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, ansi, len, nullptr, nullptr);
  return (n > 0) ? static_cast<size_t>(n) : 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toANSI(OmCString* ansi, const OmWString& wstr)
{
  return __multibyte_encode(CP_ACP, ansi, wstr.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_fromAnsiCp(OmWString* wstr, const char* ansi)
{
  return __multibyte_decode(CP_ACP, wstr, ansi);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toZipCDR(char* cdr, size_t len, const OmWString& wstr)
{
  // The WinAPI implementation is the fastest one at this time
  int n = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, cdr, len, nullptr, nullptr);

  for(size_t i = 0; cdr[i] != 0; ++i) {
    if(cdr[i] == '\\') cdr[i] = '/';
  }

  return (n > 0) ? static_cast<size_t>(n) : 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toZipCDR(OmCString* cdr, const OmWString& wstr)
{
  // "remove" the leading slash if exists
  const wchar_t* c_wstr = wstr.c_str();
  if(c_wstr[0] == L'/' || c_wstr[0] == L'\\')
    c_wstr += 1;

  __multibyte_encode(CP_UTF8, cdr, c_wstr);

  std::replace(cdr->begin(), cdr->end(), '\\', '/');

  return cdr->size();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_fromZipCDR(OmWString* wstr, const char* cdr)
{
  __multibyte_decode(CP_UTF8, wstr, cdr);

  std::replace(wstr->begin(), wstr->end(), L'/', L'\\');

  return wstr->size();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_strToUpper(OmWString* str) {
  for(size_t i = 0; i < str->size(); ++i)
    str->at(i) = towupper(str->at(i));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_namesMatches(const OmWString& left, const OmWString& right)
{
  if(left.size() != right.size())
    return false;

  // test from end of string because this function is mainly used to compare
  // paths, so the first part of string is often exactly the same.

  size_t i = left.size();
  while(i--) {
    if(towupper(left[i]) != towupper(right[i]))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_namesMatches(const OmWString& left, const wchar_t* right)
{
  if(left.size() != wcslen(right))
    return false;

  // test from end of string because this function is mainly used to compare
  // paths, so the first part of string is often exactly the same.

  size_t i = left.size();
  while(i--) {
    if(towupper(left[i]) != towupper(right[i]))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_trim(OmWString* wstr)
{
  // erase trailing whitespace chars
  wstr->erase(wstr->find_last_not_of(__wspace_chr) + 1);

  // erase leading whitespace chars
  wstr->erase(0, wstr->find_first_not_of(__wspace_chr));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_trimed(const OmWString& wstr)
{
  OmWString trimed;

  size_t subpos = wstr.find_first_not_of(__wspace_chr);
  size_t sublen = wstr.find_last_not_of(__wspace_chr) + 1;

  trimed.assign(wstr, subpos, sublen);

  return trimed;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isUrl(const wchar_t* url)
{
  std::match_results<const wchar_t*> matches;

  if(std::regex_match(url, matches, __url_reg)) {

    // matches :
    // 0) full match (almost never happen)
    // 1) http(s)://
    // 2) xxx.www.domain.tld
    // 3) :1234
    // 4) /path/
    // 5) file.ext
    // 6) ?x=1&y=2...

    // search for minimum required matches to have a full valid base URL
    if(matches[1].length() != 0) { //< http(s)://
      return (matches[2].length() != 0); //< xxx.www.domain.tld
    }
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isUrl(const OmWString& url)
{
  return Om_isUrl(url.c_str());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isFileUrl(const wchar_t* url)
{
  std::match_results<const wchar_t*> matches;

  if(std::regex_match(url, matches, __url_reg)) {

    // matches :
    // 0) full match (almost never happen)
    // 1) http(s)://
    // 2) xxx.www.domain.tld
    // 3) :1234
    // 4) /path/
    // 5) file.ext
    // 6) ?x=1&y=2...

    // search for minimum required matches to have a full valid URL
    // including path to file
    if(matches[1].length() != 0) { //< http(s)://
      if(matches[2].length() != 0) { //< xxx.www.domain.tld
        return (matches[5].length() != 0); //< file.ext
      }
    }
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isFileUrl(const OmWString& url)
{
  return Om_isFileUrl(url.c_str());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_hasIllegalUrlChar(const wchar_t* path)
{
  for(size_t i = 0; i < 17; ++i)
    if(wcschr(path, __illegal_url_chr[i]))
      return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_hasIllegalUrlChar(const OmWString& path)
{
  for(size_t i = 0; i < 17; ++i)
    if(path.find_first_of(__illegal_url_chr[i]) != OmWString::npos)
      return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_urlEscape(OmCString* esc, const OmWString& url)
{
  // convert to UTF-8
  int32_t mb_len = WideCharToMultiByte(CP_UTF8, 0, url.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if(mb_len > 0) {

    char* mb_buf = static_cast<char*>(Om_alloc(mb_len));

    WideCharToMultiByte(CP_UTF8, 0, url.c_str(), -1, mb_buf, mb_len, nullptr, nullptr);

    // set capacity for target string
    esc->clear();
    esc->reserve(mb_len * 2);

    mb_len--; //< reduce to string length without nullchar

    for(int32_t i = 0; i < mb_len; ++i) {

      uint8_t c = mb_buf[i];

      bool is_escaped = false;

      for(size_t k = 0; k < 15; ++k) { //< avoid check for ':' and '@'
        if(c == __illegal_url_chr[k]) {

          esc->push_back('%');
          esc->push_back(__hex_digit[(c >> 4) & 0x0F]);
          esc->push_back(__hex_digit[(c)      & 0x0F]);

          is_escaped = true;
          break;
        }
      }

      if(!is_escaped)
        esc->push_back(c);
    }

    Om_free(mb_buf);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_hasLegalSysChar(const wchar_t* name)
{
  if(!wcslen(name))
    return false;

  for(size_t i = 0; i < 8; ++i) // forbids all including back-slash
    if(wcschr(name, __illegal_win_chr[i]))
      return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_hasLegalSysChar(const OmWString& name)
{
  if(name.empty())
    return false;

  for(size_t i = 0; i < 8; ++i) // forbids all including back-slash
    if(name.find_first_of(__illegal_win_chr[i]) != OmWString::npos)
      return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_hasLegalPathChar(const wchar_t* path)
{
  if(!wcslen(path))
    return false;

  // check for illegal characters in path
  for(unsigned i = 0; i < 7; ++i)  // excluding back-slash
    if(wcschr(path, __illegal_win_chr[i]))
      return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_hasLegalPathChar(const OmWString& path)
{
  if(path.empty())
    return false;

  for(unsigned i = 0; i < 7; ++i)  // excluding back-slash
    if(path.find_first_of(__illegal_win_chr[i]) != OmWString::npos)
      return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_getFileExtPart(const OmWString& path) {
  size_t d = path.find_last_of(L'.') + 1;
  return path.substr(d, OmWString::npos);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_getFilePart(const OmWString& path){
  size_t s = path.find_last_of(L'\\') + 1;
  return path.substr(s, OmWString::npos);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_getDirPart(const OmWString& uri){
  size_t e = uri.find_last_of(L'\\');
  return uri.substr(0, e);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_getNamePart(const OmWString& uri){
  size_t s = uri.find_last_of(L'\\') + 1;
  size_t e = uri.find_last_of(L'.');
  return uri.substr(s, e-s);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_extensionMatches(const OmWString& file, const wchar_t* ext)
{
  size_t d = file.find_last_of(L'.') + 1;
  if(d > 0 && d != OmWString::npos) {

    size_t len = wcslen(ext);

    if(len != (file.size() - d))
      return false;

    // compare case-insensitive
    for(size_t i = 0; i < len; ++i) {
      if(towupper(file[d+i]) != towupper(ext[i]))
        return false;
    }

    return true;
  } else {
    return false;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_concatPaths(const OmWString& left, const OmWString& right) {

  OmWString result;

  if(left.empty()) {

    result.assign(right);

  } else {

    result.assign(left);

    if(left.back() != L'\\' && right.front() != L'\\')
      result.push_back(L'\\');

    result.append(right);
  }
  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_concatPaths(OmWString& conc, const OmWString& left, const OmWString& right) {

  if(left.empty()) {

    conc.assign(right);

  } else {

    conc.assign(left);

    if(left.back() != L'\\' && right.front() != L'\\')
      conc.push_back(L'\\');

    conc.append(right);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_concatPathsExt(const OmWString& left, const OmWString& right, const wchar_t* ext)
{
  OmWString result;
  if(left.empty()) {

    result.assign(right);

  } else {

    result.assign(left);

    if(left.back() != L'\\' && right.front() != L'\\')
      result.push_back(L'\\');

    result.append(right);

  }

  result.push_back(L'.');
  result.append(ext);

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_concatPathsExt(OmWString& conc, const OmWString& left, const OmWString& right, const wchar_t* ext)
{
  if(left.empty()) {

    conc.assign(right);

  } else {

    conc.assign(left);

    if(left.back() != L'\\' && right.front() != L'\\')
      conc.push_back(L'\\');

    conc.append(right);
  }

  conc.push_back(L'.');
  conc.append(ext);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isRootOfPath(const OmWString& root, const OmWString& path) {

  size_t l = root.size();

  if(l > path.size())
    return false;

  if(path.compare(0,l,root) == 0) {
    return (path[l] == L'\\'); //< verify this is a folder and not a file
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_getRelativePath(OmWString* rel, const OmWString& root, const OmWString& path) {

  size_t l = root.size();

  if(l > path.size())
    return false;

  if(path.compare(0,l,root) == 0) {
    if(path[l] == L'\\') { //< verify this is a folder
      l++;
      if(path.size() > l) {
        *rel = path.substr(l, -1);
        return true;
      }
    }
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_concatURLs(const OmWString& left, const OmWString& right) {

  OmWString result;
  if(left.empty()) {
    result = right;
  } else {
    result = left;
    if(left.back() != L'/' && right.front() != L'/') result += L"/";
    result += right;
  }
  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_concatURLs(OmWString& conc, const OmWString& left, const OmWString& right) {

  if(left.empty()) {
    conc = right;
  } else {
    conc = left;
    if(left.back() != L'/' && right.front() != L'/') conc += L"/";
    conc += right;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_formatSizeStr(uint64_t bytes, bool octet)
{
  wchar_t swp_buf[64];
  wchar_t unit = (octet) ? 'o' : 'B';

  double fbytes;
  OmWString result;

  if(bytes < 1024) { // 1 Ko
    if(octet) {
      swprintf(swp_buf, 64, L"%d Octet(s)", bytes);
    } else {
      swprintf(swp_buf, 64, L"%d Byte(s)", bytes);
    }
  } else if(bytes < 1024000) { // 1 Mo
    fbytes = (double)bytes / 1024.0;
    if(fbytes < 10.0) {
      swprintf(swp_buf, 64, L"%.1f Ki%lc", fbytes, unit);
    } else {
      swprintf(swp_buf, 64, L"%d Ki%lc", static_cast<int>(fbytes), unit);
    }
  } else if(bytes < 1024000000) { // 1 Go
    fbytes = (double)bytes / 1024000.0;
    swprintf(swp_buf, 64, L"%.1f Mi%lc", fbytes, unit);
  } else {
    fbytes = (double)bytes / 1024000000.0;
    swprintf(swp_buf, 64, L"%.1f Gi%lc", fbytes, unit);
  }

  result = swp_buf;
  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_formatSizeSysStr(OmWString* dest, int64_t bytes)
{
  wchar_t buf[32];

  if(StrFormatKBSizeW(bytes, buf, 32))
    dest->assign(buf);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_formatSizeSysStr(int64_t bytes)
{
  OmWString result;

  wchar_t buf[32];

  if(StrFormatKBSizeW(bytes, buf, 32))
    result.assign(buf);

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_strIsVersion(const OmWString& str)
{
  unsigned n = 0;
  unsigned j = 0;

  for(unsigned i = 0; i < str.size(); ++i) {

    // we accept wildcard
    if(str[i] > 47 && str[i] < 58)  { // 0123456789 || *

      if(j < 15) {
        ++j;
      } else {
        return false;
      }

    } else {

      if(str[i] == L'.' || str[i] == L'*') {
        if(j > 0) {
          n++; j = 0;
        }

        if(str[i] == L'*') {
          n++;
        }
      }

      if(n > 2)
        break;
    }
  }

  return (j > 0 || n > 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_escapeMarkdown(OmWString* dst, const OmWString& src)
{
  uint32_t new_cap = dst->size() + (src.size() * 2);

  if(dst->capacity() < new_cap)
    dst->reserve(new_cap);

  const wchar_t* s = src.data();

  while(*s != L'\0') {
    if(wcschr(L"*_#`", *s)) {
      dst->push_back(L'\\');
    }
    dst->push_back(*s++);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_escapeMarkdown(wchar_t* dst, const OmWString& src)
{
  size_t n = 0;
  const wchar_t* s = src.data();

  while(*s != L'\0') {
    if(wcschr(L"*_#`", *s)) {
      *dst++ = '\\'; n++;
    }
    *dst++ = *s++; n++;
  }

  *dst++ = '\0';

  return n;
}

/// \brief Convert to CRLF text
///
/// Create CRLF version of the given input text, if the input text is
/// already CRLF the text is simply copied.
///
/// \param[out] out_text : Pointer to string to receive CRLF text.
/// \param[in]  in_text  : String to convert to CRLF.
///
inline static void __to_crlf(OmWString* out_text, const OmWString& in_text)
{
  out_text->clear();
  out_text->reserve(in_text.size() * 2);

  for(size_t i = 0; i < in_text.size(); ++i) {

    if(in_text[i] == L'\n') {

      // check for preceding '\r', if not add it
      if(i < 1) {
        if(in_text[i - 1] != L'\r') {
          out_text->push_back(L'\r');
          out_text->push_back(L'\n');
          continue;
        }
      } else {
        out_text->push_back(L'\r');
        out_text->push_back(L'\n');
        continue;
      }
    }

    out_text->push_back(in_text[i]);
  }
}

/// \brief Convert to CRLF text
///
/// Create CRLF version of the given input text, if the input text is
/// already CRLF the text is simply copied.
///
/// \param[out] out_text : Pointer to string to receive CRLF text.
/// \param[in]  in_text  : String to convert to CRLF.
///
inline static void __to_crlf(OmCString* out_text, const OmCString& in_text)
{
  out_text->clear();
  out_text->reserve(in_text.size() * 2);

  for(size_t i = 0; i < in_text.size(); ++i) {

    if(in_text[i] == '\n') {

      // check for preceding '\r', if not add it
      if(i < 1) {
        if(in_text[i - 1] != '\r') {
          out_text->push_back('\r');
          out_text->push_back('\n');
          continue;
        }
      } else {
        out_text->push_back('\r');
        out_text->push_back('\n');
        continue;
      }
    }

    out_text->push_back(in_text[i]);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_toCRLF(OmWString* crlf_txt, const OmWString& lf_txt)
{
  __to_crlf(crlf_txt, lf_txt);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_toCRLF(const OmWString& lf_txt)
{
  OmWString crlf_txt;

  __to_crlf(&crlf_txt, lf_txt);

  return crlf_txt;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_toCRLF(OmCString* crlf_txt, const OmCString& lf_txt)
{
  __to_crlf(crlf_txt, lf_txt);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmCString Om_toCRLF(const OmCString& lf_txt)
{
  OmCString crlf_txt;

  __to_crlf(&crlf_txt, lf_txt);

  return crlf_txt;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_hasCRLF(const OmWString& str)
{
  for(size_t i = 0; i < str.size(); ++i) {
    if(str[i] == L'\n') {
      // check for receding '\r'
      if(i > 0 && str[i - 1] == L'\r')
        return true;
    }
  }
  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_hasCRLF(const OmCString& str)
{
  for(size_t i = 0; i < str.size(); ++i) {
    if(str[i] == '\n') {
      // check for receding '\r'
      if(i > 0 && str[i - 1] == '\r')
        return true;
    }
  }
  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_concatStrings(const OmWStringArray& strings, const OmWString& separator)
{
  OmWString concat;

  size_t stop = strings.size() - 1;

  size_t i;

  for(i = 0; i < stop; ++i) {
    concat += strings[i];
    concat += separator;
  }

  concat += strings[i];

  return concat;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_spacesToUnderscores(const OmWString& spaces)
{
  OmWString result;

  result.assign(spaces);
  std::replace(result.begin(), result.end(), ' ', '_');

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString Om_underscoresToSpaces(const OmWString& spaces)
{
  OmWString result;

  result.assign(spaces);
  std::replace(result.begin(), result.end(), '_', ' ');

  return result;
}
