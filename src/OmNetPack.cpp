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

#include "OmXmlConf.h"

#include "OmUtilStr.h"
#include "OmUtilErr.h"
#include "OmUtilHsh.h"
#include "OmUtilPkg.h"
#include "OmUtilB64.h"
#include "OmUtilZip.h"
#include "OmUtilFs.h"

#include "OmModHub.h"
#include "OmModChan.h"
#include "OmNetRepo.h"
#include "OmModPack.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmNetPack.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmNetPack::OmNetPack() :
  _ModChan(nullptr),
  _NetRepo(nullptr),
  _hash(0),
  _size(0),
  _csum_is_md5(false),
  _stat(PACK_UKN),
  _has_part(false),
  _has_local(false),
  _has_error(false),
  _has_misg_dep(false),
  _is_upgrading(false),
  _cli_ptr(nullptr),
  _cli_result_cb(nullptr),
  _cli_download_cb(nullptr),
  _cli_progress_cb(nullptr),
  _dnl_result(OM_RESULT_UNKNOW),
  _dnl_remain(0),
  _dnl_percent(0),
  _upg_percent(0)
{

}
///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmNetPack::OmNetPack(OmModChan* ModChan) :
  _ModChan(ModChan),
  _NetRepo(nullptr),
  _hash(0),
  _size(0),
  _csum_is_md5(false),
  _stat(PACK_UKN),
  _has_part(false),
  _has_local(false),
  _has_error(false),
  _has_misg_dep(false),
  _is_upgrading(false),
  _cli_ptr(nullptr),
  _cli_result_cb(nullptr),
  _cli_download_cb(nullptr),
  _cli_progress_cb(nullptr),
  _dnl_result(OM_RESULT_UNKNOW),
  _dnl_remain(0),
  _dnl_percent(0),
  _upg_percent(0)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmNetPack::~OmNetPack()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetPack::parseReference(OmNetRepo* NetRepo, size_t i)
{
  OmXmlNode ref_node = NetRepo->getReference(i);

  if(!ref_node.hasAttr(L"file") || !ref_node.hasAttr(L"bytes")|| !ref_node.hasAttr(L"ident")) {
    this->_error(L"parseReference", Om_errParse(L"Repository reference", L"<remote>", L"base attributes missing"));
    return false;
  }

  if(!ref_node.hasAttr(L"xxhsum")) {
    if(!ref_node.hasAttr(L"md5sum")) {
      this->_error(L"parseReference", Om_errParse(L"Repository reference", L"<remote>", L"checksum attribute missing"));
      return false;
    }
  }

  this->_NetRepo = NetRepo;

  this->_file.assign(ref_node.attrAsString(L"file"));
  this->_size = ref_node.attrAsUint64(L"bytes");

  if(ref_node.hasAttr(L"xxhsum")) {

    this->_csum_is_md5 = false;
    this->_csum.assign(ref_node.attrAsString(L"xxhsum"));

  } else if(ref_node.hasAttr(L"md5sum")) {

    this->_csum_is_md5 = true;
    this->_csum.assign(ref_node.attrAsString(L"md5sum"));
  }

  // check whether we found a partial download data for this instance
  if(!this->_ModChan) {
    // compose download temporary file name
    this->_has_part = Om_isFile(Om_concatPathsExt(this->_ModChan->libraryPath(), this->_file, L"dl_part"));
  }

  OmWString down_url;

  // check for custom URL or download path
  if(ref_node.hasChild(L"url")) {

    // get custom path
    OmWString cust_url(ref_node.child(L"url").content());

    // check whether the supplied custom path is a full URL
    if(Om_isValidUrl(cust_url)) {
      // set dwonload URL as supplied custom path
      down_url = cust_url;
    } else {
      // compose basic download URL with custom path
      Om_concatURLs(down_url, this->_NetRepo->urlBase(), cust_url);
    }
  } else {
    // compose download URL from common default parameters
    Om_concatURLs(down_url, this->_NetRepo->urlBase(), this->_NetRepo->downpath());
  }

  // if download path is not already a full URL to file, add file
  if(!Om_isValidFileUrl(down_url)) {
    // finally add file to this URL
    Om_concatURLs(down_url, down_url, this->_file);
  }

  // add download URL to list
  this->_url.assign(down_url);

  this->_iden.assign(ref_node.attrAsString(L"ident"));

  this->_hash = Om_getXXHash3(this->_file);

  // parse other Mod common infos from identity
  OmWString vers_str;
  if(Om_parseModIdent(this->_iden, &this->_core, &vers_str, &this->_name))
    this->_version.parse(vers_str);

  // check for category
  if(ref_node.hasAttr(L"category"))
    this->_category = ref_node.attrAsString(L"category");

  // check for dependencies
  if(ref_node.hasChild(L"dependencies")) {

    OmXmlNodeArray ident_nodes;
    ref_node.child(L"dependencies").children(ident_nodes, L"ident");

    for(unsigned i = 0; i < ident_nodes.size(); ++i) {
      this->_depend.push_back(ident_nodes[i].content());
    }
  }

  // check for entry snapshot
  if(ref_node.hasChild(L"picture")) {

    // decode the DataURI
    size_t jpg_size;
    OmWString mimetype, charset;
    uint8_t* jpg_data = Om_decodeDataUri(&jpg_size, mimetype, charset, ref_node.child(L"picture").content());

    // load Jpeg image
    if(jpg_data) {
      this->_thumbnail.loadThumbnail(jpg_data, jpg_size, OM_MODPACK_THUMB_SIZE, OM_SIZE_FILL);
    } else {
      this->_log(OM_LOG_WRN, L"parseReference", L"thumbnail DataURI decoding error");
    }
  }

  if(ref_node.hasChild(L"description")) {

    OmXmlNode description_node = ref_node.child(L"description");

    if(description_node.hasAttr(L"bytes")) {

      // decode the DataURI
      size_t zip_size;
      OmWString mimetype, charset;
      uint8_t* zip_data = Om_decodeDataUri(&zip_size, mimetype, charset, description_node.content());

      if(zip_data) {

        size_t txt_size = description_node.attrAsInt(L"bytes");

        uint8_t* txt_data = Om_zInflate(zip_data, zip_size, txt_size);

        Om_free(zip_data);

        if(txt_data) {

          this->_description = Om_toUTF16(reinterpret_cast<char*>(txt_data));

          Om_free(txt_data);
        } else {
          this->_log(OM_LOG_WRN, L"parseReference", L"description data zip inflate error");
        }
      } else {
        this->_log(OM_LOG_WRN, L"parseReference", L"description DataURI decoding error");
      }
    } else {
      this->_log(OM_LOG_WRN, L"parseReference", L"description 'bytes' attribute missing");
    }
  }

  this->_stat = PACK_UKN;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetPack::hasDepend(const OmWString& ident) const
{
  // you don't like raw loops ? I LOVE row loops...
  for(size_t i = 0; i < this->_depend.size(); ++i)
    if(this->_depend[i] == ident)
      return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetPack::refreshStatus()
{
  if(!this->_ModChan)
    return false;

  uint32_t new_stat = this->_stat;

  // remove relevant states, but not all of them to
  // keep states such as downloading or error
  OM_REM_BIT(new_stat, PACK_OLD);
  OM_REM_BIT(new_stat, PACK_DEP);

  //check for resumable download data
  this->_has_part = Om_isFile(Om_concatPathsExt(this->_ModChan->libraryPath(), this->_file, L"dl_part"));

  // by default the net pack is "NEW"
  OM_ADD_BIT(new_stat, PACK_NEW);

  // clear the replacement list
  this->_upgrade.clear();
  this->_dngrade.clear();

  // clear downloaded status
  this->_has_local = false;

  // clear missing dependencies status
  this->_has_misg_dep = false;

  for(size_t i = 0; i < this->_ModChan->modpackCount(); ++i) {

    OmModPack* ModPack = this->_ModChan->getModpack(i);

    // we ignore directory sources
    if(ModPack->sourceIsDir())
      continue;

    // search for same core but different version
    if(this->_core == ModPack->core()) {

      if(this->_iden == ModPack->iden()) {

        this->_has_local = true;

        // check for missing dependencies
        this->_has_misg_dep = this->_ModChan->hasMissingDepend(ModPack);

      } else {

        // check versions
        if(this->_version > ModPack->version()) {

          this->_upgrade.push_back(ModPack);

        } else {

          this->_dngrade.push_back(ModPack);
        }

      }
    }
  }

  if(this->_stat != new_stat) {

    this->_stat = new_stat;

    return true;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetPack::revokeDownload()
{
  if(!this->_ModChan)
    return;

  Om_fileDelete(Om_concatPathsExt(this->_ModChan->libraryPath(), this->_file, L"dl_part"));

  this->refreshStatus();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetPack::isDownloading() const
{
  return this->_connect.isPerforming();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetPack::stopDownload()
{
  if(this->_dnl_result == OM_RESULT_PENDING)
    this->_connect.abortRequest();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetPack::startDownload(Om_downloadCb download_cb, Om_resultCb result_cb, void* user_ptr)
{
  if(this->_dnl_result == OM_RESULT_PENDING) {
    this->_error(L"startDownload", L"download already processing");
    return false;
  }

  if(!this->_ModChan) {
    this->_error(L"startDownload", L"no Mod Channel");
    return false;
  }

  // remove error status bit
  this->_has_error = false;

  // set file path
  Om_concatPaths(this->_dnl_path, this->_ModChan->libraryPath(), this->_file);
  this->_dnl_temp = this->_dnl_path;
  this->_dnl_temp += L".dl_part";

  // set user defined parameters
  this->_cli_ptr = user_ptr;
  this->_cli_result_cb = result_cb;
  this->_cli_download_cb = download_cb;

  this->_dnl_percent = 0.0;

  if(!this->_connect.requestHttpGet(this->_url, this->_dnl_temp, true, OmNetPack::_dnl_result_fn, OmNetPack::_dnl_download_fn, this)) {
    this->_error(L"startDownload", this->_connect.lastError());
    this->_has_error = true;
    return false;
  }

  this->_dnl_result = OM_RESULT_PENDING;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetPack::finalizeDownload()
{
  if(this->_dnl_result != OM_RESULT_OK && this->_dnl_result != OM_RESULT_ERROR) {

    // either download is still processing or it has never started, in all
    // case this function should not be called at this stage

    this->_error(L"finalizeDownload", L"invalid call");
    return false;
  }

  if(this->_dnl_result == OM_RESULT_ERROR) {

    // download ended with error, we have nothing to do

    this->_error(L"finalizeDownload", this->_connect.lastError());
    return false;
  }

  // compare checksum
  bool checksum_ok = false;

  if(this->_csum_is_md5) {
    checksum_ok = Om_cmpMD5sum(this->_dnl_temp, this->_csum);
  } else {
    checksum_ok = Om_cmpXXHsum(this->_dnl_temp, this->_csum);
  }

  if(checksum_ok) {

    int32_t result = Om_fileMove(this->_dnl_temp, this->_dnl_path);
    if(result != 0) {
      Om_fileDelete(this->_dnl_temp);
      this->_error(L"finalizeDownload", Om_errRename(L"Temporary file", this->_dnl_temp, result));
      this->_has_error = true;
    }

  } else {

    Om_fileDelete(this->_dnl_temp);
    this->_error(L"finalizeDownload", L"downloaded data checksum mismatch the reference");
    this->_has_error = true;
  }

  this->refreshStatus();

  return !this->_has_error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetPack::_dnl_download_fn(void* ptr, int64_t tot, int64_t cur, int64_t rate, uint64_t data)
{
  OmNetPack* self = static_cast<OmNetPack*>(ptr);

  self->_dnl_percent = ((double)cur / tot) * 100;
  self->_dnl_remain = (double)(tot - cur) / rate;

  if(self->_cli_download_cb) {
    return self->_cli_download_cb(self->_cli_ptr, tot, cur, rate, reinterpret_cast<uint64_t>(self));
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetPack::_dnl_result_fn(void* ptr, OmResult result, uint64_t data)
{
  OmNetPack* self = static_cast<OmNetPack*>(ptr);

  OM_REM_BIT(self->_stat, PACK_DNL); //< no more in DNL state

  if(result == OM_RESULT_OK) {

    self->_dnl_result = OM_RESULT_OK;

  } else {

    if(result != OM_RESULT_ABORT)
      self->_has_error = true;

    self->_dnl_result = OM_RESULT_ERROR;
  }

  if(self->_cli_result_cb)
    self->_cli_result_cb(self->_cli_ptr, result, reinterpret_cast<uint64_t>(self));
}

///
///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmNetPack::upgradeReplace(Om_progressCb progress_cb, void* user_ptr)
{
  if(!this->_ModChan) {
    this->_error(L"upgradeReplace", L"no Mod Channel");
    return OM_RESULT_ABORT;
  }

  if(this->_upgrade.empty()) {
    this->_error(L"upgradeReplace", L"nothing to upgrade");
    return OM_RESULT_ABORT;
  }

  if(this->isDownloading() || this->_has_error  || this->_has_part) {
    this->_error(L"upgradeReplace", L"invalid call");
    return OM_RESULT_ABORT;
  }

  this->_upg_percent = 0;

  this->_is_upgrading = true;

  // setup client parameters
  this->_cli_ptr = user_ptr;
  this->_cli_progress_cb = progress_cb;

  // find Mod Pack corresponding to this Net Pack
  OmModPack* this_ModPack;

  // since this function is intended to be called right after download success
  // the refresh of Mod library may had not been occurred yet, so we give us
  // some attempts with little time to get a result
  int32_t attempt = 4;
  while(attempt--) {
    this_ModPack = this->_ModChan->findModpack(this->iden(), true);
    if(this_ModPack) break; else Sleep(100);
  }

  if(!this_ModPack) {
    this->_error(L"upgradeReplace", L"corresponding Mod Pack object not found in Mod library");
    return OM_RESULT_ERROR;
  }

  bool has_error = false;

  #ifdef DEBUG
  // simulate slow progressing process
  for(size_t i = 0; i < 100; ++i) {
    OmNetPack::_upg_progress_fn(this, 100, i, 0);
    Sleep(100);
  }
  #endif //DEBUG


  // Mod Pack lists for uninstall and install process
  OmPModPackArray selection, restores;
  OmWStringArray overlaps, depends;

  // to keep track of all uninstalled Mods to be re-installed
  OmUint64Array unins_hash;

  // 1. uninstall all replaced Mods before renaming/trashing them

  for(size_t i = 0; i < this->_upgrade.size(); ++i) {
    if(this->_upgrade[i]->hasBackup())
      selection.push_back(this->_upgrade[i]);
  }

  // prepare Mods uninstall and backups restoration
  this->_ModChan->prepareRestores(selection, &restores, &overlaps, &depends);

  // perform Mods uninstall and backups restoration
  for(size_t i = 0; i < restores.size(); ++i) {

    // keep hash of uinstalled package to be re-installed later
    unins_hash.push_back(restores[i]->hash());

    if(!restores[i]->restoreData(OmNetPack::_upg_progress_fn, this)) {
      this->_error(L"upgradeReplace", restores[i]->lastError());
      has_error = true;
    }
  }

  // 2. remove and replace all references in Mod Presets
  OmModHub* ModHub = this->_ModChan->ModHub();

  // remove package references from existing batches
  for(size_t i = 0; i < ModHub->presetCount(); ++i) {

    OmModPset* ModPset = ModHub->getPreset(i);

    bool had_entry = false;

    // remove all reference to deleted packages
    for(size_t j = 0; j < this->_upgrade.size(); ++j) {

      if(ModPset->deleteSetupEntry(this->_ModChan, this->_upgrade[j]->iden()))
        had_entry = true;
    }

    // old reference was found, add reference to the new version
    if(had_entry)
      ModPset->addSetupEntry(this->_ModChan, this_ModPack);
  }

  // 3. rename or move to trash replaced Mods, this will delete
  //    old Mod Pack from Library

  for(size_t i = 0; i < this->_upgrade.size(); ++i) {

    OmModPack* ModPack = this->_upgrade[i];

    // rename or trash package source
    if(!ModPack->hasSource())
      continue;

    // store data since object will be deleted after library update
    OmWString mod_path(ModPack->sourcePath());

    if(this->_ModChan->upgdRename()) {

      // rename with .old extension
      OmWString mod_old(mod_path); mod_old.append(L".old");

      int32_t result = Om_fileMove(mod_path, mod_old);
      if(result != 0) {
        this->_error(L"upgradeReplace", Om_errRename(L"replaced Mod archive", mod_path, result));
        has_error = true;
      }

    } else {
      // move to recycle bin
      int32_t result = Om_moveToTrash(mod_path);
      if(result != 0) {
        this->_error(L"upgradeReplace", Om_errMove(L"replaced Mod archive", mod_path, result));
        has_error = true;
      }
    }
  }

  // as Mods are not downloaded sequentially and because dependencies are
  // not handled at download end, it may happen that a dependencies mods
  // are available for install before the Mod they depend on, which corrupt
  // installation process.
  //
  // Until a proper download/upgrade dependency handling mechanism was
  // found and implemented, the following piece of code stay disabled,
  // leaving Mods uninstalled ready for manual install.

  /*
  // 4. install the new Mod if replaced ones was installed

  if(!selection.empty()) { //< this mean old mod was installed

    OmPModPackArray installs;
    OmWStringArray missings;

    selection.clear(); //< our mods selection

    // add previously uninstalled packages
    for(size_t i = 0; i < unins_hash.size(); ++i) {

      OmModPack* ModPack = this->_ModChan->findModpack(unins_hash[i], true);

      if(ModPack)
        selection.push_back(ModPack);
    }

    // add this instance corresponding Mod Pack
    selection.push_back(this_ModPack);

    // clear all our processing lists
    overlaps.clear(); depends.clear();

    // prepare for installation
    this->_ModChan->prepareInstalls(selection, &installs, &overlaps, &depends, &missings);

    for(size_t i = 0; i < installs.size(); ++i) {
      if(!installs[i]->applySource(OmNetPack::_upg_progress_fn, this)) {
        this->_error(L"upgradeReplace", installs[i]->lastError());
        has_error = true;
      }
    }
  }
  */

  this->_upg_percent = 0;

  // clear the replaced list, pointers are now invalid
  this->refreshStatus();

  // reset client parameters
  this->_cli_ptr = nullptr;
  this->_cli_progress_cb = nullptr;

  this->_is_upgrading = false;

  return has_error ? OM_RESULT_ERROR : OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetPack::_upg_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmNetPack* self = static_cast<OmNetPack*>(ptr);

  self->_upg_percent = ((double)cur / tot) * 100;

  if(self->_cli_progress_cb) {
    return self->_cli_progress_cb(self->_cli_ptr, tot, cur, reinterpret_cast<uint64_t>(self));
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetPack::_log(unsigned level, const OmWString& origin,  const OmWString& detail)
{
  if(this->_ModChan) {
    OmWString root(L"NetPack["); root.append(this->_iden); root.append(L"].");
    this->_ModChan->escalateLog(level, root + origin, detail);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetPack::_error(const OmWString& origin, const OmWString& detail)
{
  this->_lasterr = detail;
  this->_log(OM_LOG_ERR, origin, detail);
}
