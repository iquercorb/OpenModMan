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
#include <algorithm>          //< std::replace

#include "OmUtilStr.h"   //< Om_strIsVersion

static inline bool __parseModIdent(const OmWString& ident, OmWString* name, OmWString* core, OmWString* vers)
{
  // parse raw name to get display name and potential version
  bool has_version = false;
  // we search a version part in the name, this must be the letter V preceded
  // by a common separator, like space, minus or underscore character, followed
  // by a number
  size_t v_pos = ident.find_last_of(L"vV");
  if(v_pos > 0 && v_pos != OmWString::npos) {
    // verify the V letter is preceded by a common separator
    wchar_t wc = ident[v_pos - 1];
    if(wc == L' ' || wc == L'_' || wc == L'-' || wc == L'.') {
      // verify the V letter is followed by a number
      wc = ident[v_pos + 1];
      if(wc > 0x29 && wc < 0x40) { // 0123456789
        // get the substring from v char to the end of string
        *vers = ident.substr(v_pos+1, -1);
        has_version = Om_strIsVersion(*vers);
      }
    }
  }

  if(has_version) {
    // we extract the substring from the beginning to version delimiter
    *core = ident.substr(0, v_pos - 1);
  } else {
    vers->clear();
    *core = ident;
  }

  // copy parsed core to display name
  *name = *core;

  // replace all underscores by spaces in name
  std::replace(name->begin(), name->end(), L'_', L' ');

  return has_version;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_parseModFilename(const OmWString& filename, OmWString* iden, OmWString* core, OmWString* vers, OmWString* name)
{
  *iden = Om_getNamePart(filename);

  return __parseModIdent(*iden, name, core, vers);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_parseModDirname(const OmWString& dirname, OmWString* iden, OmWString* core, OmWString* vers, OmWString* name)
{
  *iden = Om_getFilePart(dirname);

  return __parseModIdent(*iden, name, core, vers);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_parseModIdent(const OmWString& iden, OmWString* core, OmWString* vers, OmWString* name)
{
  return __parseModIdent(iden, name, core, vers);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_parsePkgIdent(OmWString& name, OmWString& core, OmWString& vers, const OmWString& filename, bool isfile, bool us2spc)
{
  OmWString ident;

  // Get the proper part of the full filename
  if(isfile) {
    // get file name without file extension
    ident = Om_getNamePart(filename);
  } else {
    // this extract the folder name from the full path
    ident = Om_getFilePart(filename);
  }

  // parse raw name to get display name and potential version
  bool has_version = false;
  // we search a version part in the name, this must be the letter V preceded
  // by a common separator, like space, minus or underscore character, followed
  // by a number
  size_t v_pos = ident.find_last_of(L"vV");
  if(v_pos > 0 && v_pos != OmWString::npos) {
    // verify the V letter is preceded by a common separator
    wchar_t wc = ident[v_pos - 1];
    if(wc == L' ' || wc == L'_' || wc == L'-' || wc == L'.') {
      // verify the V letter is followed by a number
      wc = ident[v_pos + 1];
      if(wc > 0x29 && wc < 0x40) { // 0123456789
        // get the substring from v char to the end of string
        vers = ident.substr(v_pos+1, -1);
        has_version = Om_strIsVersion(vers);
      }
    }
  }

  if(has_version) {
    // we extract the substring from the beginning to version delimiter
    core = ident.substr(0, v_pos - 1);
  } else {
    vers.clear();
    core = ident;
  }

  // copy parsed core to display name
  name = core;

  // replace all underscores by spaces in name
  if(us2spc) {
    std::replace(name.begin(), name.end(), L'_', L' ');
  }

  return has_version;
}
