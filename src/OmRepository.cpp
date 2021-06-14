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

#include "OmRepository.h"
#include "OmLocation.h"
#include "OmSocket.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmRepository::OmRepository(OmLocation* pLoc) :
  _location(pLoc),
  _base(),
  _name(),
  _url(),
  _valid(false),
  _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmRepository::~OmRepository()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmRepository::define(const wstring& base, const wstring& name)
{
  // Build final URL to check if given setting is valid
  wstring url = base + L"/";
  url += name + L".xml";

  if(!Om_isValidUrl(url)) {
    this->_error =  L"Parameters make an invalid URL: ";
    this->_error += L"\"" + url + L"\"";
    this->log(1, L"Repository("+base+L"-"+name+L") Define", this->_error);
    return false;
  }

  this->_base = base;
  this->_name = name;

  this->_url = Om_toUtf8(url);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmRepository::update()
{
  if(!_url.size())
    return false;

  this->_valid = false;

  OmSocket sock;

  string rep_str;

  if(!sock.httpGet(this->_url, rep_str)) {
    this->_error =  L"\"" + Om_fromUtf8(this->_url.c_str());
    this->_error += L"\" HTTP GET error: ";
    this->_error += sock.lastErrorStr();
    this->log(1, L"Repository("+this->_base+L"-"+this->_name+L") Querry", this->_error);
    return false;
  }

  OmXmlDoc rep_xml;

  if(!rep_xml.parse(Om_fromUtf8(rep_str.c_str()))) {
    this->_error = L"Repository definition \""+Om_fromUtf8(this->_url.c_str())+L"\"";
    this->_error += OMM_STR_ERR_DEFOPEN(rep_xml.lastErrorStr());
    this->log(1, L"Repository("+this->_base+L"-"+this->_name+L") Querry", this->_error);
    return false;
  }

  // get current <package> child entries in definition file
  vector<OmXmlNode> pkgs_list;
  rep_xml.children(pkgs_list, L"package");

  this->_item.clear();

  OmRepoItem repo_item;

  for(size_t i = 0; i < pkgs_list.size(); ++i) {

    if(!pkgs_list[i].hasAttr(L"ident"))
      continue;

    if(!pkgs_list[i].hasAttr(L"href"))
      continue;

    repo_item.ident = pkgs_list[i].attrAsString(L"ident");
    repo_item.href = Om_toUtf8(pkgs_list[i].attrAsString(L"href"));

    if(pkgs_list[i].hasAttr(L"img")) {
      repo_item.img = Om_toUtf8(pkgs_list[i].attrAsString(L"img"));
    }

    if(pkgs_list[i].hasChild(L"desc")) {
      repo_item.desc = pkgs_list[i].content();
    }

    this->_item.push_back(repo_item);
  }

  this->_valid = true;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmRepository::log(unsigned level, const wstring& head, const wstring& detail)
{
  if(this->_location != nullptr) {

    wstring log_str = L"Location("; log_str.append(this->_location->title());
    log_str.append(L"):: "); log_str.append(head);

    this->_location->log(level, log_str, detail);
  }
}
