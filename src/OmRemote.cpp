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
#include "OmRemote.h"
#include "OmManager.h"
#include "OmContext.h"
#include "OmLocation.h"
#include "OmPackage.h"
#include "OmImage.h"
#include "OmSocket.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmRemote::OmRemote() :
  _location(nullptr),
  _url(),
  _file(),
  _bytes(0),
  _checksum(),
  _state(0),
  _ident(),
  _hash(0),
  _core(),
  _version(),
  _name(),
  _depLs(),
  _desc(),
  _image(),
  _error(),
  _downl_file(),
  _downl_path(),
  _downl_temp(),
  _downl_spsd(false),
  _downl_user_download(nullptr),
  _downl_user_ptr(nullptr),
  _downl_hth(nullptr),
  _downl_percent(0)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmRemote::OmRemote(OmLocation* pLoc) :
  _location(pLoc),
  _url(),
  _file(),
  _bytes(0),
  _checksum(),
  _state(0),
  _ident(),
  _hash(0),
  _core(),
  _version(),
  _name(),
  _depLs(),
  _desc(),
  _image(),
  _error(),
  _downl_file(),
  _downl_path(),
  _downl_temp(),
  _downl_spsd(false),
  _downl_user_download(nullptr),
  _downl_user_ptr(nullptr),
  _downl_hth(nullptr),
  _downl_percent(0)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmRemote::~OmRemote()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmRemote::parse(const wstring& url, OmXmlNode entry)
{
  // reset this instance
  this->clear();

  // check for propers attributes
  if(!entry.hasAttr(L"file")) {
    this->_error = L"Invalid definition : 'file' attribute missing.";
    this->log(0, L"Remote(<anonymous>) Parse", this->_error);
    return false;
  }

  // check for propers attributes
  if(!entry.hasAttr(L"bytes")) {
    this->_error = L"Invalid definition : 'bytes' attribute missing.";
    this->log(0, L"Remote(<anonymous>) Parse", this->_error);
    return false;
  }

  // check for propers attributes
  if(!entry.hasAttr(L"checksum")) {
    this->_error = L"Invalid definition : 'checksum' attribute missing.";
    this->log(0, L"Remote(<anonymous>) Parse", this->_error);
    return false;
  }

  // check for propers attributes
  if(!entry.hasAttr(L"ident")) {
    this->_error = L"Invalid definition : 'ident' attribute missing.";
    this->log(0, L"Remote(<anonymous>) Parse", this->_error);
    return false;
  }

  // get file informations
  this->_file = entry.attrAsString(L"file");
  this->_bytes = entry.attrAsInt(L"bytes");
  this->_checksum = entry.attrAsString(L"checksum");

  // get packages informations
  this->_ident = entry.attrAsString(L"ident");

  // create hash from file name
  this->_hash = Om_getXXHash3(this->_file);

  // parse filename to get display name and potential version
  wstring vers;
  if(Om_parsePkgIdent(this->_name, this->_core, vers, this->_file, true, true)) {
    this->_version.parse(vers);
  }

  OmXmlNode xml_node;

  // check custom URL
  if(entry.hasChild(L"url")) {
    this->_url.push_back(entry.child(L"url").content());
  } else {
    // compose download url from repository file path
    this->_url.push_back(url + this->_file);
  }

  // check for dependencies
  if(entry.hasChild(L"dependencies")) {

    xml_node = entry.child(L"dependencies");
    std::vector<OmXmlNode> xml_ls;
    xml_node.children(xml_ls, L"ident");

    for(unsigned i = 0; i < xml_ls.size(); ++i) {
      this->_depLs.push_back(xml_ls[i].content());
    }
  }

  wstring tmp_str1, tmp_str2;

  // check for entry snapshot
  if(entry.hasChild(L"picture")) {
    xml_node = entry.child(L"picture");

    // decode the DataURI
    size_t jpg_size;
    uint8_t* jpg = Om_decodeDataUri(&jpg_size, tmp_str1, tmp_str2, xml_node.content());

    // load Jpeg image
    if(jpg) {
      if(!this->_image.open(jpg, jpg_size, OMM_PKG_THMB_SIZE)) {
        this->_image.clear();
      }
    }
  }

  // check for entry description
  if(entry.hasChild(L"description")) {
    xml_node = entry.child(L"description");

    if(xml_node.hasAttr(L"bytes")) {

      // decode the DataURI
      size_t zip_size;
      uint8_t* zip = Om_decodeDataUri(&zip_size, tmp_str1, tmp_str2, xml_node.content());

      if(zip) {
        // get string original size
        size_t txt_size = xml_node.attrAsInt(L"bytes");
        // decompress data
        uint8_t* txt = Om_zInflate(zip, zip_size, txt_size);
        Om_free(zip);

        if(txt) this->_desc = Om_fromUtf8(reinterpret_cast<char*>(txt));
      }
    } else {
      this->_error = L"Definition error : 'bytes' attribute missing for <description>.";
      this->log(1, L"Remote("+this->_ident+L") Parse", this->_error);
    }
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmRemote::urlAdd(const wstring& url)
{
  if(Om_isValidUrl(url)) {
    if(find(this->_url.begin(), this->_url.end(), url) == this->_url.end())
      this->_url.push_back(url);
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmRemote::download(const wstring& path, bool supersedes, Om_downloadCb download_cb, void* user_ptr)
{
  if(this->_downl_hth) {
    this->_error =  L"Download thread already running";
    this->log(1, L"Remote("+this->_ident+L") Download", this->_error);
    return false;
  }

  // remove error status
  this->_state &= ~RMT_STATE_ERR;

  this->_downl_spsd = supersedes;

  // set file path
  this->_downl_path = path + L"\\" + this->_file;
  this->_downl_temp = this->_downl_path + L".dl_temp";

  // Open file for writing
  if((this->_downl_file = _wfopen(this->_downl_temp.c_str(), L"wb")) == nullptr) {
    this->_error =  L"Temporary file \""+this->_downl_temp+L"\" creation failed: ";
    this->_error += L"Unable to open file for writing.";
    this->log(0, L"Remote("+this->_ident+L") Download", this->_error);
    return false;
  }

  // set user defined callback and pointer
  this->_downl_user_download = download_cb;
  this->_downl_user_ptr = user_ptr;

  // launch new download thread
  DWORD dwId;
  this->_downl_hth = CreateThread(nullptr, 0, this->_downl_fth, this, 0, &dwId);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmRemote::clear()
{
  this->_hash = 0;
  this->_url.clear();
  this->_image.clear();
  this->_name.clear();
  this->_ident.clear();
  this->_depLs.clear();
  this->_desc.clear();
  this->_version.define(0,0,0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmRemote::log(unsigned level, const wstring& head, const wstring& detail)
{
  if(this->_location != nullptr) {

    wstring log_str = L"Location("; log_str.append(this->_location->title());
    log_str.append(L"):: "); log_str.append(head);

    this->_location->log(level, log_str, detail);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmRemote::_downl_fth(void* ptr)
{
  OmRemote* self = static_cast<OmRemote*>(ptr);

  OmSocket sock;

  DWORD exitCode = 0;

  self->_state |= RMT_STATE_DNL;

  self->log(2, L"Remote("+self->_ident+L") Download", L"Get file at \""+self->_url[0]+L"\".");

  if(!sock.httpGet(self->_url[0], self->_downl_file, &self->_downl_download, self)) {

    exitCode = sock.lastError(); //< curl error code
    self->_error = L"Download error \""+self->_url[0]+L"\" ";
    self->_error += sock.lastErrorStr();

    if(exitCode == 42) { //< CURLE_ABORTED_BY_CALLBACK
      self->log(1, L"Remote("+self->_ident+L") Download", self->_error);
    } else {
      self->log(0, L"Remote("+self->_ident+L") Download", self->_error);
      self->_state |= RMT_STATE_ERR;
    }
  }

  // close file
  fclose(self->_downl_file);

  if(exitCode == 0) {
    // compare checksum
    if(Om_cmpChecksum(self->_downl_temp, self->_checksum)) {
      int result = Om_fileMove(self->_downl_temp, self->_downl_path);
      if(result == 0) {
        self->_state &= ~RMT_STATE_NEW; //< now in library
      } else {
        self->_error = Om_errRename(L"Temporary file", self->_downl_temp, result);
        self->log(0, L"Remote("+self->_ident+L") Download", self->_error);
        Om_fileDelete(self->_downl_temp);
        self->_state |= RMT_STATE_ERR;
      }
    } else {
      self->_error =  L"Downloaded file checksum mismatch the reference";
      self->log(0, L"Remote("+self->_ident+L") Download", self->_error);
      Om_fileDelete(self->_downl_temp);
      self->_state |= RMT_STATE_ERR;
    }

    if(!self->isState(RMT_STATE_ERR)) {

      // some log outpute
      wstring log =  L"File \""+self->_downl_path+L"\" successfully downloaded.";
      self->log(2, L"Remote("+self->_ident+L") Download", log);

      // now move to trash superseded packages
      if(self->_downl_spsd) {

        OmPackage* pPkg;

        bool rename = self->_location->upgdRename();

        for(size_t i = 0; i < self->_supLs.size(); ++i) {
          pPkg = self->_supLs[i];
          if(pPkg->hasSrc()) {
            if(rename) {
              // rename source with .old extension
              Om_fileMove(pPkg->srcPath(), pPkg->srcPath() + L".old");
            } else {
              // move source to recycle bin
              Om_moveToTrash(pPkg->srcPath());
            }
          }
        }
      }
    }

  } else {
    Om_fileDelete(self->_downl_temp);
  }

  // last call to callback so it can check for download result
  if(self->_downl_user_download) {
    self->_downl_user_download(self->_downl_user_ptr, 0.0, 0.0, -1.0, self->_hash);
  }

  self->_state &= ~RMT_STATE_DNL;

  self->_downl_temp.clear();
  self->_downl_path.clear();
  self->_downl_user_download = nullptr;
  self->_downl_user_ptr = nullptr;

  // not pretty but no choice
  self->_downl_hth = nullptr;

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmRemote::_downl_download(void* ptr, double tot, double cur, double rate, uint64_t data)
{
  OmRemote* self = static_cast<OmRemote*>(ptr);

  self->_downl_percent = (cur / tot) * 100.0;

  if(self->_downl_user_download) {
    return self->_downl_user_download(self->_downl_user_ptr, tot, cur, rate, self->_hash);
  }

  return true;
}
