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

#include "OmImage.h"

#include "OmModChan.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmNetRepo.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmNetRepo::OmNetRepo(OmModChan* ModChan) :
  _ModChan(ModChan),
  _query_result(OM_RESULT_UNKNOW),
  _query_respcode(0)
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
  this->_query_respcode = 0;
  this->_query_respdata.clear();
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
  // preliminary test on parameters
  OmWString url = Om_concatURLs(base, name);

  // check for valid URL (less restrictive) ie. "http://www.example.com/toto"
  if(!Om_isUrl(url)) {
    this->_error(L"setCoordinates", Om_errParam(url, L"URL"));
    return false;
  }

  this->_base = base;
  if(!name.empty())
    this->_name = name;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::parse(const OmWString& data)
{
  // try to parse received data as repository
  if(!this->_xml.parse(data, OM_XMAGIC_REP))
    return false;

  if(!this->_xml.hasChild(L"uuid") || !this->_xml.hasChild(L"title") || !this->_xml.hasChild(L"downpath"))
    return false;

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
  // Notice to who consider rewrite this part asynchronous way :
  //
  // Since updating repositories in Mod Channel imply Libraries (Mods list)
  // manipulation, in case of multiple repositories, such operation need to be
  // performed sequentially to prevent potential conflicting between threads.
  // For this reason, this is easier and cleaner to run thread within
  // Mod Channel and keep Repository Query operation synchronous way.

  // check for basic setup
  if(this->_base.empty() && this->_name.empty())
    return this->_query_result;

  // create list of URL to try
  OmWStringArray urls;

  if(this->_name.empty()) {
    urls.push_back(this->_base);
  } else {
    // we test repository coordinates with two possible extension
    urls.push_back(Om_concatURLs(this->_base, this->_name) + L"." OM_XML_DEF_EXT);
    urls.push_back(Om_concatURLs(this->_base, this->_name) + L".xml");
  }

  // send synchronous request
  OmXmlDoc parsexml;
  OmCString respdata;

  // the stuff bellow is used for error reporting in various test
  // situations such as properties and wizard dialogs in order to avoid
  // duplicate code (poor maintainability) and keep consistent behavior.
  this->_query_respdata.clear();
  this->_query_respcode = 0;
  this->_query_lasterr.clear();

  // the general query result
  this->_query_result = OM_RESULT_PENDING;

  for(size_t i = 0; i < urls.size(); ++i) {

    #ifdef DEBUG
    std::wcout << L"DEBUG => OmNetRepo::query : try url=" << urls[i] << L"\n";
    #endif // DEBUG

    OmResult result = this->_query_connect.requestHttpGet(urls[i], &respdata);

    if(result == OM_RESULT_OK) {

      // store HTTP response code
      this->_query_respdata = Om_toUTF16(respdata);
      this->_query_respcode = this->_query_connect.httpGetResponse();

      // we verify we received valid XML data
      if(!parsexml.parse(this->_query_respdata)) {
        this->_query_result = OM_RESULT_ERROR_PARSE;
        this->_query_lasterr = L"Received invalid data";
        return this->_query_result;
      }

      // try to parse the XML data as repository
      if(!this->parse(this->_query_respdata)) {
        this->_query_result = OM_RESULT_ERROR_PARSE;
        this->_query_lasterr = L"Invalid Repository XML";
        return this->_query_result;
      }

      this->_path = urls[i]; //< save the working URL in path
      this->_query_result = OM_RESULT_OK;
      return this->_query_result;

    } else {

      if(result == OM_RESULT_ABORT) {

        // operation aborted, we simply return

        this->_query_result = OM_RESULT_ABORT;

        return this->_query_result;

      } else {

        // since we try several URLs, at least one will result in 404 error, so to keep
        // consistent error reporting we ignore 404 error unless we are a the end of
        // list. If other error type occur, statistically this will be the same for both
        // URLs so we don't care if we store errors for the first or the second one

        if((this->_query_connect.httpGetResponse() != 404) || (i == (urls.size() - 1))) {

          // store data if any (should not)
          if(!respdata.empty())
            this->_query_respdata = Om_toUTF16(respdata);

          // store HTTP response code and error string
          this->_query_respcode = this->_query_connect.httpGetResponse();
          this->_query_lasterr = this->_query_connect.lastError();
        }

        this->_error(L"query", Om_errHttp(L"repository def", urls[i], this->_query_connect.lastError()));
      }
    }
  }

  // arriving here mean no URL succeed, this is a fail
  this->_query_result = OM_RESULT_ERROR;

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

  OmXmlNode ref_node;

  // search whether reference with same identity exists
  int32_t found = this->indexOfReference(ModPack->iden());

  if(found >= 0) {
    ref_node = this->_reference_list[found];
  } else {
    ref_node = references_node.addChild(L"mod");
    this->_reference_list.push_back(ref_node);
    ref_node.setAttr(L"ident", ModPack->iden());
  }

  // set or replace references bases values
  ref_node.setAttr(L"file", Om_getFilePart(ModPack->sourcePath()));
  ref_node.setAttr(L"bytes", Om_itemSize(ModPack->sourcePath()));
  ref_node.setAttr(L"xxhsum", Om_getXXHsum(ModPack->sourcePath())); //< use XXHash3 by default
  ref_node.setAttr(L"category", ModPack->category());

  // set or replace dependencies
  if(ref_node.hasChild(L"dependencies"))
    ref_node.remChild(L"dependencies");

  if(ModPack->dependCount()) {
    OmXmlNode dependencies_nodes = ref_node.addChild(L"dependencies");
    for(size_t i = 0; i < ModPack->dependCount(); ++i) {
      dependencies_nodes.addChild(L"ident").setContent(ModPack->getDependIden(i));
    }
  }

  // set, replace or delete thumbnail data
  if(ModPack->thumbnail().valid()) {
    if(!this->_save_thumbnail(ref_node, ModPack->thumbnail()))
      this->_log(OM_LOG_WRN, L"addReference", L"thumbnail JPEG encode failed");
  } else {
    if(ref_node.hasChild(L"thumbnail"))
      ref_node.remChild(L"thumbnail");
  }

  // set, replace or delete description data
  if(!ModPack->description().empty()) {
    if(!this->_save_description(ref_node, ModPack->description()))
      this->_log(OM_LOG_WRN, L"addReference", L"description deflate failed");
  } else {
    if(ref_node.hasChild(L"description"))
      ref_node.remChild(L"description");
  }

  return found;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::setReferenceUrl(size_t index, const OmWString& url)
{
  if(index >= this->_reference_list.size())
    return;

  OmXmlNode ref_node = this->_reference_list[index];

  if(url.empty()) {

    if(ref_node.hasChild(L"url"))
       ref_node.remChild(L"url");

  } else {

    if(ref_node.hasChild(L"url")) {
      ref_node.child(L"url").setContent(url);
    } else {
      ref_node.addChild(L"url").setContent(url);
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString OmNetRepo::getReferenceUrl(size_t index) const
{
  OmWString result;

  if(index >= this->_reference_list.size())
    return result;

  OmXmlNode ref_node = this->_reference_list[index];

  if(ref_node.hasChild(L"url"))
    result = ref_node.child(L"url").content();

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::referenceHasUrl(size_t index) const
{
  if(index >= this->_reference_list.size())
    return false;

  return this->_reference_list[index].hasChild(L"url");
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::setReferenceThumbnail(size_t index, const OmImage& image, uint8_t level)
{
  if(index >= this->_reference_list.size())
    return;

  OmXmlNode ref_node = this->_reference_list[index];

  if(!image.valid()) {

    if(ref_node.hasChild(L"thumbnail"))
       ref_node.remChild(L"thumbnail");

  } else {

    if(!this->_save_thumbnail(ref_node, image, level))
      this->_log(OM_LOG_WRN, L"setReferenceThumbnail", L"thumbnail JPEG encode failed");
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmImage OmNetRepo::getReferenceThumbnail(size_t index) const
{
  OmImage image;

  if(index >= this->_reference_list.size())
    return image;

  OmXmlNode ref_node = this->_reference_list[index];

  if(ref_node.hasChild(L"thumbnail")) {

    // decode the DataURI
    size_t jpg_size;
    OmWString mimetype, charset;
    uint8_t* jpg_data = Om_decodeDataUri(&jpg_size, mimetype, charset, ref_node.child(L"thumbnail").content());

    // load Jpeg image
    if(jpg_data) {
      image.load(jpg_data, jpg_size);
      Om_free(jpg_data);
    }
  }

  return image;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::referenceHasThumbnail(size_t index) const
{
  if(index >= this->_reference_list.size())
    return false;

  return this->_reference_list[index].hasChild(L"thumbnail");
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::setReferenceDescription(size_t index, const OmWString& text, uint8_t level)
{
  if(index >= this->_reference_list.size())
    return;

  OmXmlNode ref_node = this->_reference_list[index];

  if(text.empty()) {

    if(ref_node.hasChild(L"description"))
       ref_node.remChild(L"description");

  } else {

    if(!this->_save_description(ref_node, text, level))
      this->_log(OM_LOG_WRN, L"setReferenceDescription", L"description deflate failed");
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString OmNetRepo::getReferenceDescription(size_t index) const
{
  OmWString desc;

  if(index >= this->_reference_list.size())
    return desc;

  OmXmlNode ref_node = this->_reference_list[index];

  if(ref_node.hasChild(L"description")) {

    // decode the DataURI
    size_t dfl_size;
    OmWString mimetype, charset;
    uint8_t* dfl_data = Om_decodeDataUri(&dfl_size, mimetype, charset, ref_node.child(L"description").content());

    // decompress text
    if(dfl_data) {

      size_t txt_size = ref_node.child(L"description").attrAsInt(L"bytes");
      uint8_t* txt_data = Om_zInflate(dfl_data, dfl_size, txt_size);
      Om_free(dfl_data);

      if(txt_data) {
        Om_toUTF16(&desc, reinterpret_cast<char*>(txt_data));
        Om_free(txt_data);
      }
    }
  }

  return desc;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::referenceHasDescription(size_t index) const
{
  if(index >= this->_reference_list.size())
    return false;

  return this->_reference_list[index].hasChild(L"description");
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetRepo::getReferenceDepends(size_t index, OmWStringArray* depends) const
{
  if(!depends) return;
  depends->clear();

  if(index >= this->_reference_list.size())
    return;

  OmXmlNode ref_node = this->_reference_list[index];

  // check for dependencies
  if(ref_node.hasChild(L"dependencies")) {

    OmXmlNodeArray ident_nodes;
    ref_node.child(L"dependencies").children(ident_nodes, L"ident");

    for(size_t i = 0; i < ident_nodes.size(); ++i)
      depends->push_back(ident_nodes[i].content());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmNetRepo::getReferenceDependCount(size_t index) const
{
  if(index >= this->_reference_list.size())
    return 0;

  OmXmlNode ref_node = this->_reference_list[index];

  // check for dependencies
  if(ref_node.hasChild(L"dependencies"))
    return ref_node.child(L"dependencies").childCount(L"ident");

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::_save_thumbnail(OmXmlNode& modref, const OmImage& image, uint8_t level)
{
  // Encode RGBA to JPEG
  uint64_t jpg_size;
  uint8_t* jpg_data = Om_imgEncodeJpg(&jpg_size, image.data(), image.width(), image.height(), image.bpp(), level);

  if(!jpg_data)
    return false;

  // format jpeg to base64 encoded data URI
  OmWString data_uri;
  Om_encodeDataUri(data_uri, L"image/jpeg", L"", jpg_data, jpg_size);

  // free allocated data
  Om_free(jpg_data);

  // set node content to data URI string
  if(modref.hasChild(L"thumbnail")) {
    modref.child(L"thumbnail").setContent(data_uri);
  } else {
    modref.addChild(L"thumbnail").setContent(data_uri);
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetRepo::_save_description(OmXmlNode& modref, const OmWString& text, uint8_t level)
{
  // convert to UTF-8
  OmCString utf8 = Om_toUTF8(text);

  // text buffer size with null char
  size_t txt_size = utf8.size() + 1;

  // compress data using defalte
  size_t dfl_size;
  uint8_t* dfl_data = Om_zDeflate(&dfl_size, reinterpret_cast<const uint8_t*>(utf8.c_str()), txt_size, level);

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
