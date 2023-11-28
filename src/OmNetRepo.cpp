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
#include "OmBaseApp.h"
#include "OmUtilStr.h"
#include "OmUtilErr.h"
#include "OmModChan.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmNetRepo.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmNetRepo::OmNetRepo(OmModChan* ModChan) : _ModChan(ModChan),
  _qry_result(OM_RESULT_UNKNOW)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmNetRepo::~OmNetRepo()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::setup(const OmWString& base, const OmWString& name)
{
  // Build final URL to check if given setting is valid
  OmWString full(base); full.append(L"/"); full.append(name); full.append(L".xml");

  // check for valid URL
  if(!Om_isValidFileUrl(full)) {
    this->_error(L"setup", L"Supplied parameters cannot make a valid URL");
    return false;
  }

  this->_url_base.assign(base);
  this->_url_name.assign(name);
  this->_url_full.assign(full);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::setup(const OmWString& base, const OmWString& name, const OmWString& title)
{
  if(this->setup(base, name)) {

    this->_title.assign(title);

    return true;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::abortQuery()
{
  this->_connect.abortRequest();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmNetRepo::query()
{
  if(!this->_url_full.empty()) {

    // Notice to who consider rewrite this part asynchronous way :
    //
    // Since updating repositories in Mod Channel imply Libraries (Mods list)
    // manipulation, in case of multiple repositories, such operation need to be
    // performed sequentially to prevent potential conflicting between threads.
    // For this reason, this is easier and cleaner to run thread within
    // Mod Channel and keep Repository Query operation synchronous way.

    // send synchronous request
    OmCString response;

    this->_qry_result = OM_RESULT_PENDING;
    this->_qry_result = this->_connect.requestHttpGet(this->_url_full, &response);

    // if request succeed, try to parse response data
    if(this->_qry_result == OM_RESULT_OK) {

      // try to parse received data as repository
      if(!this->_xmlconf.parse(Om_toUTF16(response), OM_XMAGIC_REP)) {
        this->_error(L"query", Om_errParse(L"Repository definition", this->_url_full, this->_xmlconf.lastErrorStr()));
        this->_qry_result = OM_RESULT_ERROR_PARSE;
        return this->_qry_result;
      }

      if(!this->_xmlconf.hasChild(L"uuid") || !this->_xmlconf.hasChild(L"title") ||
         !this->_xmlconf.hasChild(L"downpath") || !this->_xmlconf.hasChild(L"remotes")) {

        this->_error(L"query", Om_errParse(L"Repository definition", this->_url_full, L"basic nodes missing"));
        this->_qry_result = OM_RESULT_ERROR_PARSE;
        return this->_qry_result;
      }

      this->_uuid.assign(this->_xmlconf.child(L"uuid").content());
      this->_title.assign(this->_xmlconf.child(L"title").content());
      this->_downpath.assign(this->_xmlconf.child(L"downpath").content());

      // get list of Mod references
      this->_xmlconf.child(L"remotes").children(this->_reference, L"remote");

    } else {

      // if query was not aborted, this is definitely an error
      if(this->_qry_result != OM_RESULT_ABORT) {
        this->_error(L"query", this->_connect.lastError());
      }
    }
  }

  return this->_qry_result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::hasReference(const OmWString& iden) const
{
  return (this->referenceIndex(iden) != -1);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmNetRepo::referenceIndex(const OmWString& iden) const
{
  for(size_t i = 0; i < this->_reference.size(); ++i) {
    if(this->_reference[i].attrAsString(L"iden") == iden)
      return i;
  }

  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::_log(unsigned level, const OmWString& origin,  const OmWString& detail) const
{
  OmWString root(L"ModRepo["); root.append(this->_url_full); root.append(L"].");
  if(this->_ModChan)
    this->_ModChan->escalateLog(level, root + origin, detail);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::_error(const OmWString& origin, const OmWString& detail)
{
  this->_lasterr = detail;
  this->_log(OM_LOG_ERR, origin, detail);
}
