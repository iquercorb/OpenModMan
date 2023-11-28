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

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmVersion.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmVersion::OmVersion() :
  _str(), _maj(0), _min(0), _rev(0)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmVersion::OmVersion(const OmWString& vstr) :
  _str(), _maj(0), _min(0), _rev(0)
{
  wchar_t num[32];

  unsigned j = 0;
  for(size_t i = 0; i < vstr.size(); ++i) {

    if(vstr[i] > 47 && vstr[i] < 58) { // 0123456789

      if(j < 15) {
        num[j] = vstr[i]; ++j;
      } else {
        return;
      }

    } else {

      if(vstr[i] == L'.') {
        if(j > 0) {
          num[j] = 0;
          _str.push_back(num);
          j = 0;
        }
      }

      if(_str.size() > 2)
        break;
    }
  }

  if(j > 0) {
    num[j] = 0; _str.push_back(num);
  }

  if(_str.size() > 0) {
    _maj = wcstoul(_str[0].c_str(), nullptr, 10);
    if(_str.size() > 1) {
      _min = wcstoul(_str[1].c_str(), nullptr, 10);
      if(_str.size() > 2) {
        _rev = wcstoul(_str[2].c_str(), nullptr, 10);
      }
    }
    return;
  }

  return;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmVersion::OmVersion(const OmVersion& other) :
  _str(),
  _maj(other._maj),
  _min(other._min),
  _rev(other._rev)
{
  for(size_t i = 0; i < other._str.size(); ++i) {
    _str.push_back(other._str[i]);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmVersion::~OmVersion()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmVersion::parse(const OmWString& vstr)
{
  wchar_t num[32];

  _maj = 0;
  _min = 0;
  _rev = 0;

  _str.clear();

  unsigned j = 0;
  for(size_t i = 0; i < vstr.size(); ++i) {

    if(vstr[i] > 47 && vstr[i] < 58) { // 0123456789

      if(j < 15) {
        num[j] = vstr[i]; ++j;
      } else {
        return false;
      }

    } else {

      if(vstr[i] == L'.') {
        if(j > 0) {
          num[j] = 0;
          _str.push_back(num);
          j = 0;
        }
      }

      if(_str.size() > 2)
        break;
    }
  }

  if(j > 0) {
    num[j] = 0; _str.push_back(num);
  }

  if(_str.size() > 0) {
    _maj = wcstoul(_str[0].c_str(), nullptr, 10);
    if(_str.size() > 1) {
      _min = wcstoul(_str[1].c_str(), nullptr, 10);
      if(_str.size() > 2) {
        _rev = wcstoul(_str[2].c_str(), nullptr, 10);
      }
    }
    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmVersion::define(unsigned maj, unsigned min, unsigned rev)
{
  _maj = maj;
  _min = min;
  _rev = rev;

  _str.clear();

  wchar_t wcbuf[32];

  swprintf(wcbuf, 32, L"%u", _maj);
  _str.push_back(wcbuf);

  swprintf(wcbuf, 32, L"%u", _min);
  _str.push_back(wcbuf);

  if(_rev > 0) {
    swprintf(wcbuf, 32, L"%u", _rev);
    _str.push_back(wcbuf);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmVersion& OmVersion::operator=(const OmVersion& other)
{
  _maj = other._maj;
  _min = other._min;
  _rev = other._rev;

  for(size_t i = 0; i < other._str.size(); ++i) {
    _str.push_back(other._str[i]);
  }

  return *this;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmVersion& OmVersion::operator=(const OmWString& vstr)
{
  wchar_t digits[16];

  _maj = 0;
  _min = 0;
  _rev = 0;

  _str.clear();

  unsigned j = 0;
  for(size_t i = 0; i < vstr.size(); ++i) {

    if(vstr[i] > 47 && vstr[i] < 58) { // 0123456789

      if(j < 15) {
        digits[j] = vstr[i]; ++j;
      } else {
        return *this;
      }

    } else {

      if(vstr[i] == L'.') {
        if(j > 0) {
          digits[j] = 0;
          _str.push_back(digits);
          j = 0;
        }
      }

      if(_str.size() > 2)
        break;
    }
  }

  if(j > 0) {
    digits[j] = 0; _str.push_back(digits);
  }

  if(_str.size() > 0) {
    _maj = wcstoul(_str[0].c_str(), nullptr, 10);
    if(_str.size() > 1) {
      _min = wcstoul(_str[1].c_str(), nullptr, 10);
      if(_str.size() > 2) {
        _rev = wcstoul(_str[2].c_str(), nullptr, 10);
      }
    }
    return *this;
  }

  return *this;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString OmVersion::asString() const
{
  OmWString ret;

  if(_str.size() > 0) {
    ret = _str[0];
    if(_str.size() > 1) {
      ret += L"."; ret += _str[1];
      if(_str.size() > 2) {
        ret += L"."; ret += _str[2];
      }
    }
  } else {
    ret = L"N/A";
  }

  return ret;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmVersion::operator<(const OmVersion& other) const
{
  if(_maj < other._maj) {
    return true;
  } else if(_maj == other._maj) {
    if(_min < other._min) {
      return true;
    } else if(_min == other._min) {
      if(_rev < other._rev) {
        return true;
      } else {
        return false;
      }
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmVersion::operator>(const OmVersion& other) const
{
  if(_maj > other._maj) {
    return true;
  } else if(_maj == other._maj) {
    if(_min > other._min) {
      return true;
    } else if(_min == other._min) {
      if(_rev > other._rev) {
        return true;
      } else {
        return false;
      }
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmVersion::operator<=(const OmVersion& other) const
{
  if(_maj == other._maj && _min == other._min && _rev == other._rev) {
    return true;
  } else {
    if(_maj < other._maj) {
      return true;
    } else if(_maj == other._maj) {
      if(_min < other._min) {
        return true;
      } else if(_min == other._min) {
        if(_rev < other._rev) {
          return true;
        } else {
          return false;
        }
      }
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmVersion::operator>=(const OmVersion& other) const
{
  if(_maj == other._maj && _min == other._min && _rev == other._rev) {
    return true;
  } else {
    if(_maj > other._maj) {
      return true;
    } else if(_maj == other._maj) {
      if(_min > other._min) {
        return true;
      } else if(_min == other._min) {
        if(_rev > other._rev) {
          return true;
        } else {
          return false;
        }
      }
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmVersion::clear()
{
  _str.clear();
  _maj = 0;
  _min = 0;
  _rev = 0;
}
