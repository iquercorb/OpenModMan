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
#include <algorithm>          //< std::replace

#include "Util/OmUtilStr.h"   //< Om_strIsVersion

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_parsePkgIdent(wstring& name, wstring& core, wstring& vers, const wstring& filename, bool isfile, bool us2spc)
{
  wstring ident;

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
  if(v_pos > 0 && v_pos != wstring::npos) {
    // verify the V letter is preceded by a common separator
    wchar_t wc = ident[v_pos - 1];
    if(wc == L' ' || wc == L'_' || wc == L'-') {
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
    // we extract the substring from the beginning to the version substring
    core = ident.substr(0, v_pos);
  } else {
    vers.clear();
    core = ident;
  }

  // if the last character of core is an underscore or a space, we crop it
  if(core.back() == L' ' || core.back() == L'_') {
    core.pop_back();
  }

  // copy parsed core to display name
  name = core;

  // replace all underscores by spaces in name
  if(us2spc) {
    std::replace(name.begin(), name.end(), L'_', L' ');
  }

  return has_version;
}
