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
#include "OmUtilHsh.h"
#include "OmUtilZip.h"
#include "OmUtilB64.h"

#include "OmModChan.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmNetRepo.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmNetRepo::OmNetRepo(OmModChan* ModChan) :
  _ModChan(ModChan),
  _query_result(OM_RESULT_UNKNOW)
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
void OmNetRepo::clear()
{
  this->_xml.clear();
  this->_uuid.clear();
  this->_title.clear();
  this->_downpath.clear();
  this->_base.clear();
  this->_name.clear();
  this->_path.clear();
  this->_reference_list.clear();
  this->_query_connect.clear();
  this->_query_result = OM_RESULT_UNKNOW;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::init(const OmWString& title)
{
  this->clear();

  this->_uuid = Om_genUUID();
  this->_title = title;
  this->_downpath = L"files/";

  // Initialize new Repository definition XML scheme
  this->_xml.init(OM_XMAGIC_REP);

  // set basis nodes
  this->_xml.addChild(L"uuid").setContent(this->_uuid);
  this->_xml.addChild(L"title").setContent(this->_title);
  this->_xml.addChild(L"downpath").setContent(this->_downpath);

  // references root node
  this->_xml.addChild(L"references").setAttr(L"count", 0);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::setCoordinates(const OmWString& base, const OmWString& name)
{
  // Build final URL to check if given setting is valid
  OmWString url;

  if(name.empty()) {

    // assume a raw URL is provided
    url = base;

    // check for valid URL (less restrictive) ie. "http://www.example.com"
    if(!Om_isUrl(url)) {
      this->_error(L"setCoordinates", Om_errParam(url, L"URL"));
      return false;
    }

  } else {

    // assume named coordinates is provided
    Om_concatURLs(url, base, name);
    url += L".xml";

    // check for valid URL to file ie. "http://www.example.com/toto/default.xml"
    if(!Om_isFileUrl(url)) {
      this->_error(L"setCoordinates", Om_errParam(url, L"file URL"));
      return false;
    }
  }

  this->_base = base;
  this->_name = name;
  this->_path = url;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::parse(const OmWString& data)
{
  // try to parse received data as repository
  if(!this->_xml.parse(data, OM_XMAGIC_REP)) {
    this->_error(L"parse", Om_errParse(L"Repository definition", this->_path, this->_xml.lastErrorStr()));
    return false;
  }

  if(!this->_xml.hasChild(L"uuid") || !this->_xml.hasChild(L"title") || !this->_xml.hasChild(L"downpath")) {
    this->_error(L"parse", Om_errParse(L"Repository definition", this->_path, L"basic nodes missing"));
    return false;
  }

  this->_uuid = this->_xml.child(L"uuid").content();
  this->_title = this->_xml.child(L"title").content();
  this->_downpath = this->_xml.child(L"downpath").content();

  // get list of Mod references
  this->_reference_list.clear();

  if(this->_xml.hasChild(L"references")) { //< this is the new preferred XML schema

    this->_xml.child(L"references").children(this->_reference_list, L"mod");

  } else if(this->_xml.hasChild(L"remotes")) { //< this is the old deprecated XML schema

    this->_xml.child(L"remotes").children(this->_reference_list, L"remote");

    // migrate to new XML shema
    for(size_t i = 0; i < this->_reference_list.size(); ++i) {

      // <picture>  --> <thumbnail>
      if(this->_reference_list[i].hasChild(L"picture")) {
        this->_reference_list[i].child(L"picture").setName(L"thumbnail");
      }

      // <remote>  --> <mod>
      this->_reference_list[i].setName(L"mod");
    }

    // <remotes>  --> <references>
    this->_xml.child(L"remotes").setName(L"references");

  } else {
    this->_log(OM_LOG_WRN, L"parse", L"repository does not provide any mod reference");
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmNetRepo::load(const OmWString& path)
{
  // clear instance
  this->clear();

  // set access path
  this->_path = path;

  OmWString data;

  // load data from file
  size_t len = Om_loadToUTF16(&data, this->_path);

  if(len == 0) {
    this->_error(L"load", Om_errOpen(L"repository definition", this->_path, L"file open error"));
    return OM_RESULT_ERROR_IO;
  }

  // try to parse
  if(!this->parse(data)) { //< automatically migrate
    return OM_RESULT_ERROR_PARSE;
  }

  return OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmNetRepo::save(const OmWString& path)
{
  if(!this->_xml.save(path)) {

    this->_error(L"save", Om_errSave(L"repository definition", path, this->_xml.lastErrorStr()));

    return OM_RESULT_ERROR;
  }

  this->_path = path;

  return OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::abortQuery()
{
  this->_query_connect.abortRequest();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmNetRepo::query()
{
  if(!this->_path.empty()) {

    // Notice to who consider rewrite this part asynchronous way :
    //
    // Since updating repositories in Mod Channel imply Libraries (Mods list)
    // manipulation, in case of multiple repositories, such operation need to be
    // performed sequentially to prevent potential conflicting between threads.
    // For this reason, this is easier and cleaner to run thread within
    // Mod Channel and keep Repository Query operation synchronous way.

    // send synchronous request
    OmCString response;

    this->_query_result = OM_RESULT_PENDING;
    this->_query_result = this->_query_connect.requestHttpGet(this->_path, &response);

    // if request succeed, try to parse response data
    if(this->_query_result == OM_RESULT_OK) {

      if(!this->parse(Om_toUTF16(response))) {
        this->_query_result = OM_RESULT_ERROR_PARSE;
        return this->_query_result;
      }

    } else {

      // if query was not aborted, this is definitely an error
      if(this->_query_result != OM_RESULT_ABORT) {
        this->_error(L"query", this->_query_connect.lastError());
      }
    }
  }

  return this->_query_result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::setTitle(const OmWString& title)
{
  this->_title = title;

  if(!this->_xml.valid())
    return;

  this->_xml.child(L"title").setContent(this->_title);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::setDownpath(const OmWString& downpath)
{
  this->_downpath = downpath;

  if(!this->_xml.valid())
    return;

  this->_xml.child(L"downpath").setContent(this->_downpath);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::hasReference(const OmWString& iden) const
{
  return (this->indexOfReference(iden) != -1);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmNetRepo::indexOfReference(const OmWString& iden) const
{
  for(size_t i = 0; i < this->_reference_list.size(); ++i) {
    if(this->_reference_list[i].attrAsString(L"ident") == iden)
      return i;
  }

  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::deleteReference(const OmWString& iden)
{
  // get references root node
  OmXmlNode references_node = this->_xml.child(L"references");

  // in case of invalid call
  if(references_node.empty())
    return false;

  // delete from mirror reference list first
  for(size_t i = 0; i < this->_reference_list.size(); ++i) {
    if(this->_reference_list[i].attrAsString(L"iden") == iden) {

      // remove from XML definition
      references_node.remChild(this->_reference_list[i]); //< FIXME : verify this work properly

      // remove from local array
      this->_reference_list.erase(this->_reference_list.begin() + i);

      return true;
    }
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::deleteReference(size_t index)
{
  if(index >= this->_reference_list.size())
    return;

  // get references root node
  OmXmlNode references_node = this->_xml.child(L"references");

  // in case of invalid call
  if(references_node.empty())
    return;

  // remove from XML definition
  references_node.remChild(this->_reference_list[index]); //< FIXME : verify this work properly

  // remove from local array
  this->_reference_list.erase(this->_reference_list.begin() + index);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmNetRepo::addReference(const OmModPack* ModPack)
{
    // get references root node
  OmXmlNode references_node = this->_xml.child(L"references");

  // in case of invalid call
  if(references_node.empty())
    return false;

  OmXmlNode modref;

  // search whether reference with same identity exists
  int32_t found = this->indexOfReference(ModPack->iden());

  if(found >= 0) {
    modref = this->_reference_list[found];
  } else {
    modref = references_node.addChild(L"mod");
    this->_reference_list.push_back(modref);
    modref.setAttr(L"ident", ModPack->iden());
  }

  // set or replace references bases values
  modref.setAttr(L"file", Om_getFilePart(ModPack->sourcePath()));
  modref.setAttr(L"bytes", Om_itemSize(ModPack->sourcePath()));
  modref.setAttr(L"xxhsum", Om_getXXHsum(ModPack->sourcePath())); //< use XXHash3 by default
  modref.setAttr(L"category", ModPack->category());

  // set or replace dependencies
  if(modref.hasChild(L"dependencies"))
    modref.remChild(L"dependencies");

  if(ModPack->dependCount()) {
    OmXmlNode dependencies_nodes = modref.addChild(L"dependencies");
    for(size_t i = 0; i < ModPack->dependCount(); ++i) {
      dependencies_nodes.addChild(L"ident").setContent(ModPack->getDependIden(i));
    }
  }

  // set, replace or delete thumbnail data
  if(ModPack->thumbnail().valid()) {
    if(!this->_save_thumbnail(modref, ModPack->thumbnail()))
      this->_log(OM_LOG_WRN, L"addReference", L"thumbnail JPEG encode failed");
  } else {
    if(modref.hasChild(L"thumbnail"))
      modref.remChild(L"thumbnail");
  }

  // set, replace or delete description data
  if(!ModPack->description().empty()) {
    if(!this->_save_description(modref, ModPack->description()))
      this->_log(OM_LOG_WRN, L"addReference", L"description deflate failed");
  } else {
    if(modref.hasChild(L"description"))
      modref.remChild(L"description");
  }

  return found;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::_save_thumbnail(OmXmlNode& modref, const OmImage& image)
{
  // Encode RGBA to JPEG
  uint64_t jpg_size;
  uint8_t* jpg_data = Om_imgEncodeJpg(&jpg_size, image.data(), image.width(), image.height(), 4, 7);

  if(!jpg_data)
    return false;

  // format jpeg to base64 encoded data URI
  OmWString data_uri;
  Om_encodeDataUri(data_uri, L"image/jpeg", L"", jpg_data, jpg_size);

  // set node content to data URI string
  if(modref.hasChild(L"thumbnail")) {
    modref.child(L"thumbnail").setContent(data_uri);
  } else {
    modref.addChild(L"thumbnail").setContent(data_uri);
  }

  // free allocated data
  Om_free(jpg_data);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::_save_description(OmXmlNode& modref, const OmWString& text)
{
  // convert to UTF-8
  OmCString utf8 = Om_toUTF8(text);

  // text buffer size with null char
  size_t txt_size = utf8.size() + 1;

  // compress data using defalte
  size_t dfl_size;
  uint8_t* dfl_data = Om_zDeflate(&dfl_size, reinterpret_cast<const uint8_t*>(utf8.c_str()), txt_size, 9);

  if(!dfl_data)
    return false;

  // encode raw data to data URI Base64
  OmWString data_uri;
  Om_encodeDataUri(data_uri, L"application/octet-stream", L"", dfl_data, dfl_size);

  // free deflated data
  Om_free(dfl_data);

  OmXmlNode description_node;

  // set node content to data URI string
  if(modref.hasChild(L"description")) {
    description_node = modref.child(L"description");
  } else {
    description_node = modref.addChild(L"description");
  }

  // set node data content
  description_node.setContent(data_uri);
  // set uncompressed text size
  description_node.setAttr(L"bytes", static_cast<int>(txt_size));

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::_log(unsigned level, const OmWString& origin,  const OmWString& detail) const
{
  OmWString root(L"NetRepo["); root.append(this->_path); root.append(L"].");
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
