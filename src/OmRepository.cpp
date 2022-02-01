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
bool OmRepository::init(const wstring& base, const wstring& name)
{
  // Build final URL to check if given setting is valid
  wstring url = base + L"/" + name + L".xml";

  // check for valid url
  if(!Om_isValidFileUrl(url)) {
    this->_error =  L"Invalid parameters: ";
    this->_error += L"\"" + url + L"\" is not a valid URL";
    this->log(1, L"Repository("+base+L"-"+name+L") Init", this->_error);
    return false;
  }

  this->_base = base;
  this->_name = name;
  this->_url = url;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmRepository::query()
{
  if(this->_url.empty())
    return false;

  this->clear();

  OmSocket sock;

  this->log(2, L"Repository("+this->_base+L"-"+this->_name+L") Query", L"HTTP GET \""+this->_url+L"\"");

  // Get remote XML repository data
  string data;
  if(!sock.httpGet(this->_url, data)) {
    this->_error = L"\""+this->_url+L"\" HTTP GET error: "+sock.lastErrorStr();
    this->log(1, L"Repository("+this->_base+L"-"+this->_name+L") Query", this->_error);
    this->clear();
    return false;
  }

  this->log(2, L"Repository("+this->_base+L"-"+this->_name+L") Query", to_wstring(data.size())+L" bytes received.");

  // try to parse received data as repository
  if(!this->_config.parse(Om_fromUtf8(data.c_str()), OMM_CFG_SIGN_REP)) {
    this->_error = Om_errParse(L"Repository definition", this->_url, this->_config.lastErrorStr());
    this->log(0, L"Repository("+this->_base+L"-"+this->_name+L") Query", this->_error);
    this->clear();
    return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_config.xml().hasChild(L"uuid")) {
    this->_error =  L"\"" + this->_url + L"\" invalid definition: <uuid> node missing.";
    log(0, L"Repository("+this->_base+L"-"+this->_name+L") Query", this->_error);
    this->clear();
    return false;
  }

  // check for the presence of <title> entry
  if(!this->_config.xml().hasChild(L"title")) {
    this->_error =  L"\"" + this->_url + L"\" invalid definition: <title> node missing.";
    log(0, L"Repository("+this->_base+L"-"+this->_name+L") Query", this->_error);
    this->clear();
    return false;
  }

  // check for the presence of <title> entry
  if(!this->_config.xml().hasChild(L"downpath")) {
    this->_error =  L"\"" + this->_url + L"\" invalid definition: <downpath> node missing.";
    log(0, L"Repository("+this->_base+L"-"+this->_name+L") Query", this->_error);
    this->clear();
    return false;
  }

  // check for the presence of <title> entry
  if(!this->_config.xml().hasChild(L"remotes")) {
    this->_error =  L"\"" + this->_url + L"\" invalid definition: <remotes> node missing.";
    log(0, L"Repository("+this->_base+L"-"+this->_name+L") Query", this->_error);
    this->clear();
    return false;
  }

  this->_uuid = this->_config.xml().child(L"uuid").content();
  this->_title = this->_config.xml().child(L"title").content();
  this->_downpath = this->_config.xml().child(L"downpath").content();

  this->log(2, L"Repository("+this->_title+L") Query", L"Successfully parsed.");

  this->_valid = true;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmRepository::rmtMerge(vector<OmRemote*>& rmt_ls)
{
  if(!this->_valid)
    return 0;

  // Get the package list XML node
  OmXmlNode xml_rmts = this->_config.xml().child(L"remotes");

  // Get all <remote> children
  std::vector<OmXmlNode> xml_rmt_ls;
  xml_rmts.children(xml_rmt_ls, L"remote");

  wstring ident;
  bool unique;
  unsigned n = 0;

  // search <remote> with specified identity
  OmRemote* pRmt;
  for(size_t i = 0; i < xml_rmt_ls.size(); ++i) {

    pRmt = new OmRemote(this->_location);

    if(pRmt->parse(this->_base, this->_downpath, xml_rmt_ls[i])) {

      unique = true;

      // check whether remote with same identity already exists in given list
      for(size_t j = 0; j < rmt_ls.size(); ++j) {
        if(pRmt->ident() == rmt_ls[j]->ident()) {
          unique = false;
          // we simply add download URL to existing
          rmt_ls[j]->urlAdd(pRmt->urlGet(0));
          n++;
          // TODO: we may do further checks here, by checksum and so and choose what to do...
        }
      }

      if(unique) {
        rmt_ls.push_back(pRmt);
        n++;
      }

    } else {
      this->_error =  L"<remote> #" + to_wstring(i);
      this->_error += L" parse failed: " + pRmt->lastError();
      log(1, L"Repository("+this->_title+L") Query", this->_error);
      delete pRmt;
    }
  }

  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmRepository::rmtCount()
{
  if(!this->_valid)
    return 0;

  // Get the remote package list XML node
  OmXmlNode xml_rmts = this->_config.xml().child(L"remotes");
  return xml_rmts.attrAsInt(L"count");
}
///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmRepository::rmtHas(const wstring& ident)
{
  if(!this->_valid)
    return false;

  // Get the remote package list XML node
  OmXmlNode xml_rmts = this->_config.xml().child(L"remotes");

  // Get all <remote> children
  std::vector<OmXmlNode> xml_rmt_ls;
  xml_rmts.children(xml_rmt_ls, L"remote");

  for(size_t i = 0; i < xml_rmt_ls.size(); ++i) {
    if(ident == xml_rmt_ls[i].attrAsString(L"ident"))
       return true;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmRepository::clear()
{
  this->_config.close();
  this->_uuid.clear();
  this->_title.clear();
  this->_downpath.clear();
  this->_valid = false;
  this->_error.clear();
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
