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

#include "OmConfig.h"
#include <sstream>

#define OMM_CONFIG_PENDING     0
#define OMM_CONFIG_VALID       1
#define OMM_CONFIG_ERR_XDOC    -1
#define OMM_CONFIG_ERR_SIGN    -2

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmConfig::OmConfig() :
  _status(OMM_CONFIG_PENDING),
  _file(),
  _xmlDoc(),
  _cfgRoot()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmConfig::~OmConfig()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmConfig::valid() const {
  return (_status == OMM_CONFIG_VALID);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmConfig::init(const wstring& sign)
{
  close();

  _cfgRoot = _xmlDoc.addChild(sign);
  _status = OMM_CONFIG_VALID;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmConfig::init(const wstring& path, const wstring& sign)
{
  close();

  _file = path;

  _cfgRoot = _xmlDoc.addChild(sign);
  _status = OMM_CONFIG_VALID;

  // try to save file
  save();

  return (_status == OMM_CONFIG_VALID);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmConfig::open(const wstring& path, const wstring& sign)
{
  close();

  _file = path;

  if(!_xmlDoc.load(_file)) {
    _status = OMM_CONFIG_ERR_XDOC;
    return false;
  }

  if(_xmlDoc.hasChild(sign)) {
    _cfgRoot = _xmlDoc.child(sign);
    _status = OMM_CONFIG_VALID;
  } else {
    _status = OMM_CONFIG_ERR_SIGN;
    return false;
  }

  return true;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmConfig::parse(const wstring& xml, const wstring& sign)
{
  close();

  if(!_xmlDoc.parse(xml)) {
    _status = OMM_CONFIG_ERR_XDOC;
    return false;
  }

  if(_xmlDoc.hasChild(sign)) {
    _cfgRoot = _xmlDoc.child(sign);
    _status = OMM_CONFIG_VALID;
  } else {
    _status = OMM_CONFIG_ERR_SIGN;
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmConfig::save()
{
  if(_status == OMM_CONFIG_VALID) {

    if(_xmlDoc.save(_file)) {
      return true;
    } else {
      _status = OMM_CONFIG_ERR_XDOC;
      return false;
    }
  }

  _status = OMM_CONFIG_ERR_SIGN;
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmConfig::save(const wstring& path)
{
  if(_status == OMM_CONFIG_VALID) {

    if(_xmlDoc.save(path)) {
      _file = path;
      return true;
    } else {
      _status = OMM_CONFIG_ERR_XDOC;
      return false;
    }
  }

  _status = OMM_CONFIG_ERR_SIGN;
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
string OmConfig::data()
{
  string xml = "<?xml version=\"1.0\"?>\r\n";

  if(_status == OMM_CONFIG_VALID) {
    xml += _xmlDoc.data();
  }

  return xml;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmConfig::close(bool save)
{
  if(save)
    this->save();

  _xmlDoc.clear();

  _cfgRoot.clear();

  _file.clear();

  _status = OMM_CONFIG_PENDING;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring OmConfig::lastErrorStr() const
{
  switch(_status)
  {
  case OMM_CONFIG_ERR_XDOC:
    return _xmlDoc.lastErrorStr();
    break;
  case OMM_CONFIG_ERR_SIGN:
    return L"Wrong definition signature.";
    break;
  default:
    return L"";
    break;
  }
}
