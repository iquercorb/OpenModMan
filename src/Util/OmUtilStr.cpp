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
#include "OmBase.h"           //< string, vector, Om_alloc, OMM_MAX_PATH, etc.
#include <regex>              //< wregex

#include "OmBaseWin.h"        //< WinAPI
#include <ShlWApi.h>          //< StrFromKBSizeW, etc.

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_strToUpper(wstring& str) {
  for(size_t i = 0; i < str.size(); ++i)
    str[i] = towupper(str[i]);
}

/// \brief UTF-8 decode
///
/// Convert the given multibyte string into wide char string assuming UTF-8
/// encoding.
///
/// This function si no longer used in current implementation but conserved
/// here in cas this is needed.
///
/// \param[in]  wstr    : Wide char string to receive conversion result.
/// \param[out] utf8    : Pointer to null-terminated Multibyte string to convert.
///
inline static void __utf8_decode(wstring& wstr, const char* utf8)
{
  wstr.clear();
  wstr.reserve(strlen(utf8));

  uint32_t u;
  unsigned char c;
  size_t p = 0;

  while(utf8[p] != 0) {

    c = utf8[p];

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

    if(((utf8[p] & 0xC0) != 0x80) && (u <= 0x10FFFF)) {
      if(u > 0xFFFF)  {
        wstr.push_back(static_cast<wchar_t>(0xD800 + (u >> 10)));
        wstr.push_back(static_cast<wchar_t>(0xDC00 + (u & 0x03FF)));
      } else if(u < 0xD800 || u >= 0xE000) {
        wstr.push_back(static_cast<wchar_t>(u));
      }
    }
  }
}


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
inline static void __utf8_encode(string& utf8, const wstring& wstr)
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

/// \brief Multibyte Decode
///
/// Static inlined function to convert the given multibyte string into wide
/// char string assuming the specified encoding.
///
/// This function use the WinAPI MultiByteToWideChar implementation witch is
/// currently the known fastest way.
///
/// \param[in]  cp      : Code page to use in performing the conversion.
/// \param[in]  wstr    : Wide char string to receive conversion result.
/// \param[out] utf8    : Pointer to null-terminated Multibyte string to convert.
///
inline static size_t __multibyte_decode(UINT cp, wstring& wstr, const char* utf8)
{
  int n = MultiByteToWideChar(cp, 0, utf8, -1, nullptr, 0);

  if(n > 0) {
    wstr.resize(n - 1);
    // NOTICE: here bellow, the string object is used as C char buffer, in
    // theory this is not allowed since std::string is not required to store
    // its contents contiguously in memory.
    //
    // HOWEVER, in practice, there is no know situation where std::string does
    // not store its content contiguous, so, this should work anyway.
    //
    // If some problem emerge from this function, change this implementation for
    // a more regular approach.
    return static_cast<size_t>(MultiByteToWideChar(cp, 0, utf8, -1, &wstr[0], n));
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
inline static size_t __multibyte_encode(UINT cp, string& utf8, const wchar_t* wstr)
{
  BOOL pBool;
  int n = WideCharToMultiByte(cp, 0, wstr, -1, nullptr, 0, nullptr, &pBool);
  if(n > 0) {
    utf8.resize(n - 1);
    // NOTICE: here bellow, the string object is used as C char buffer, in
    // theory this is not allowed since std::string is not required to store
    // its contents contiguously in memory.
    //
    // HOWEVER, in practice, there is no know situation where std::string does
    // not store its content contiguous, so, this should work anyway.
    //
    // If some problem emerge from this function, change this implementation for
    // a more regular approach.
    return static_cast<size_t>(WideCharToMultiByte(cp, 0, wstr, -1, &utf8[0], n, nullptr, &pBool));
  }
  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_fromUtf8(const char* utf8)
{
  wstring result;
  __multibyte_decode(CP_UTF8, result, utf8);
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_fromUtf8(wstring& wstr, const char* utf8)
{
  return __multibyte_decode(CP_UTF8, wstr, utf8);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toUtf8(char* utf8, size_t len, const wstring& wstr)
{
  // The WinAPI implementation is the fastest one at this time
  BOOL pBool;
  int n = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8, len, nullptr, &pBool);
  return (n > 0) ? static_cast<size_t>(n) : 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
string Om_toUtf8(const wstring& wstr)
{
  string result;
  __multibyte_encode(CP_UTF8, result, wstr.c_str());
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toUtf8(string& utf8, const wstring& wstr)
{
  return __multibyte_encode(CP_UTF8, utf8, wstr.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toAnsiCp(char* ansi, size_t len, const wstring& wstr)
{
  // The WinAPI implementation is the fastest one at this time
  BOOL pBool;
  int n = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, ansi, len, nullptr, &pBool);
  return (n > 0) ? static_cast<size_t>(n) : 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toAnsiCp(string& ansi, const wstring& wstr)
{
  return __multibyte_encode(CP_ACP, ansi, wstr.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_fromAnsiCp(wstring& wstr, const char* ansi)
{
  return __multibyte_decode(CP_ACP, wstr, ansi);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toZipCDR(char* cdr, size_t len, const wstring& wstr)
{
  // The WinAPI implementation is the fastest one at this time
  BOOL pBool;
  int n = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, cdr, len, nullptr, &pBool);

  for(size_t i = 0; cdr[i] != 0; ++i) {
    if(cdr[i] == '\\') cdr[i] = '/';
  }

  return (n > 0) ? static_cast<size_t>(n) : 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_toZipCDR(string& cdr, const wstring& wstr)
{
  __multibyte_encode(CP_UTF8, cdr, wstr.c_str());

  for(size_t i = 0; i < cdr.size(); ++i) {
    if(cdr[i] == '\\') cdr[i] = '/';
  }

  return cdr.size();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t Om_fromZipCDR(wstring& wstr, const char* cdr)
{
  __multibyte_decode(CP_UTF8, wstr, cdr);

  for(size_t i = 0; i < wstr.size(); ++i) {
    if(wstr[i] == L'/') wstr[i] = L'\\';
  }

  return wstr.size();
}

/*
/// \brief Sort comparison function
///
/// Comparison callback function for std::sort() to sort strings in
/// alphabetical order
///
/// \param[in]  a   : left string to compare.
/// \param[in]  b   : right string to compare.
///
/// \return True if a is "before" b, false otherwise
///
static bool __sortStrings_Func(const wstring& a, const wstring& b)
{
  // get size of the shorter string to compare
  size_t l = a.size() > b.size() ? b.size() : a.size();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {
    if(towupper(a[i]) != towupper(b[i])) {
      if(towupper(a[i]) < towupper(b[i])) {
        return true;
      } else {
        return false;
      }
    }
  }

  // if function does not returned at this stage, this mean strings
  // are equals in the tested portion, so the longer one is after
  if(a.size() < b.size())
    return true;

  // if the two strings are strictly equals, then, we don't care
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_sortStrings(vector<wstring>* strings)
{
  sort(strings->begin(), strings->end(), __sortStrings_Func);
}
*/

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_namesMatches(const wstring& left, const wstring& right)
{
  if(left.size() != right.size())
    return false;

  for(size_t i = 0; i < left.size(); ++i) {
    if(towupper(left[i]) != towupper(right[i]))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_namesMatches(const wstring& left, const wchar_t* right)
{
  if(left.size() != wcslen(right))
    return false;

  for(size_t i = 0; i < left.size(); ++i) {
    if(towupper(left[i]) != towupper(right[i]))
      return false;
  }

  return true;
}

/// \brief Illegal Windows characters
///
/// List of forbidden characters to test validity of file name or path.
///
static const wchar_t __illegal_win_chr[] = L"/*?\"<>|\\";

/// \brief URL Regex pattern
///
/// Regular expression pattern for URL.
///
static const std::wregex __url_reg(LR"(^(https?:\/\/)([\da-z\.-]+)(:[\d]+)?([\/\.\w%-]*\/)([\w.%-]*)?(\?[\w%-=&]+)?)");

/// \brief Illegal URL path characters
///
/// List of forbidden characters to test validity of URL path.
///
static const wchar_t __illegal_url_chr[] = L"#\"<>|\\{}^[]`+:@$";

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidUrl(const wchar_t* url)
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
bool Om_isValidUrl(const wstring& url)
{
  return Om_isValidUrl(url.c_str());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidFileUrl(const wchar_t* url)
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
bool Om_isValidFileUrl(const wstring& url)
{
  return Om_isValidFileUrl(url.c_str());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidUrlPath(const wchar_t* path)
{
  if(!wcslen(path))
    return false;

  for(unsigned i = 0; i < 16; ++i) // forbids all including back-slash
    if(wcschr(path, __illegal_url_chr[i]))
      return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidUrlPath(const wstring& path)
{
  if(path.empty())
    return false;

  for(unsigned i = 0; i < 16; ++i) // forbids all including back-slash
    if(path.find_first_of(__illegal_url_chr[i]) != wstring::npos)
      return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidName(const wchar_t* name)
{
  if(!wcslen(name))
    return false;

  for(unsigned i = 0; i < 8; ++i) // forbids all including back-slash
    if(wcschr(name, __illegal_win_chr[i]))
      return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidName(const wstring& name)
{
  if(name.empty())
    return false;

  for(unsigned i = 0; i < 8; ++i) // forbids all including back-slash
    if(name.find_first_of(__illegal_win_chr[i]) != wstring::npos)
      return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isValidPath(const wchar_t* path)
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
bool Om_isValidPath(const wstring& path)
{
  if(path.empty())
    return false;

  for(unsigned i = 0; i < 7; ++i)  // excluding back-slash
    if(path.find_first_of(__illegal_win_chr[i]) != wstring::npos)
      return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_getFileExtPart(const wstring& path) {
  size_t d = path.find_last_of(L'.') + 1;
  return path.substr(d, wstring::npos);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_getFilePart(const wstring& path){
  size_t s = path.find_last_of(L'\\') + 1;
  return path.substr(s, wstring::npos);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_getDirPart(const wstring& uri){
  size_t e = uri.find_last_of(L'\\');
  return uri.substr(0, e);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_getNamePart(const wstring& uri){
  size_t s = uri.find_last_of(L'\\') + 1;
  size_t e = uri.find_last_of(L'.');
  return uri.substr(s, e-s);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_extensionMatches(const wstring& file, const wchar_t* ext)
{
  size_t d = file.find_last_of(L'.') + 1;
  if(d > 0 && d != wstring::npos) {

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
wstring Om_concatPaths(const wstring& left, const wstring& right) {

  wstring result;
  if(left.empty()) {
    result = right;
  } else {
    result = left;
    if(left.back() != L'\\' && right.front() != L'\\') result += L"\\";
    result += right;
  }
  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_concatPaths(wstring& conc, const wstring& left, const wstring& right) {

  if(left.empty()) {
    conc = right;
  } else {
    conc = left;
    if(left.back() != L'\\' && right.front() != L'\\') conc += L"\\";
    conc += right;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_isRootOfPath(const wstring& root, const wstring& path) {

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
bool Om_getRelativePath(wstring& rel, const wstring& root, const wstring& path) {

  size_t l = root.size();

  if(l > path.size())
    return false;

  if(path.compare(0,l,root) == 0) {
    if(path[l] == L'\\') { //< verify this is a folder
      l++;
      if(path.size() > l) {
        rel = path.substr(l, -1);
        return true;
      }
    }
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring Om_concatURLs(const wstring& left, const wstring& right) {

  wstring result;
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
void Om_concatURLs(wstring& conc, const wstring& left, const wstring& right) {

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
wstring Om_formatSizeStr(size_t bytes, bool octet)
{
  wchar_t swp_buf[64];
  wchar_t unit = (octet) ? 'o' : 'B';

  double fbytes;
  wstring result;

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
wstring Om_formatSizeSysStr(size_t bytes, bool kbytes)
{
  wchar_t buf[64];

  if(kbytes) {
    StrFormatKBSizeW(bytes, buf, 64);
  } else {
    StrFormatByteSizeW(bytes, buf, 64);
  }

  return wstring(buf);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_strIsVersion(const wstring& str)
{
  unsigned n = 0;
  unsigned j = 0;

  for(unsigned i = 0; i < str.size(); ++i) {

    if(str[i] > 47 && str[i] < 58) { // 0123456789

      if(j < 15) {
        ++j;
      } else {
        return false;
      }

    } else {

      if(str[i] == L'.') {
        if(j > 0) {
          n++; j = 0;
        }
      }

      if(n > 2)
        break;
    }
  }

  return (j > 0 || n > 0);
}
