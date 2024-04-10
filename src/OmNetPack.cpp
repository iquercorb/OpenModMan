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
  this->stopDownload();
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
  // create formated string
  Om_formatSizeSysStr(&this->_size_str, this->_size);

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

  // check for custom link
  if(ref_node.hasChild(L"url")) {

    // get custom link
    this->_cust_url = ref_node.child(L"url").content();

    // check whether the supplied custom link is a full URL
    if(Om_isUrl(this->_cust_url)) {
      // set download URL as supplied custom link
      this->_down_url = this->_cust_url;
    } else {
      // compose basic download URL with custom link
      Om_concatURLs(this->_down_url, this->_NetRepo->base(), this->_cust_url);
    }
  } else {

    // check whether the supplied global link is a full URL
    if(Om_isUrl(this->_NetRepo->downpath())) {
      // set download URL as supplied global link
      this->_down_url = this->_NetRepo->downpath();
    } else {
      // compose download URL from common default parameters
      Om_concatURLs(this->_down_url, this->_NetRepo->base(), this->_NetRepo->downpath());
    }
  }

  // if download link is not already a full URL to file, add file
  if(!Om_isFileUrl(this->_down_url)) {
    // finally add file to this URL
    Om_concatURLs(this->_down_url, this->_down_url, this->_file);
  }

  // add download URL to list
  this->_iden = ref_node.attrAsString(L"ident");
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

  // check for thumbnail
  if(ref_node.hasChild(L"thumbnail")) {

    // decode the DataURI
    size_t jpg_size;
    OmWString mimetype, charset;
    uint8_t* jpg_data = Om_decodeDataUri(&jpg_size, mimetype, charset, ref_node.child(L"thumbnail").content());

    // load Jpeg image
    if(jpg_data) {
      this->_thumbnail.loadThumbnail(jpg_data, jpg_size, OM_MODPACK_THUMB_SIZE, OM_SIZE_FILL);
      Om_free(jpg_data);
    } else {
      this->_log(OM_LOG_WRN, L"parseReference", L"thumbnail DataURI decoding error");
    }
  }

  if(ref_node.hasChild(L"description")) {

    OmXmlNode description_node = ref_node.child(L"description");

    if(description_node.hasAttr(L"bytes")) {

      // decode the DataURI
      size_t dfl_size;
      OmWString mimetype, charset;
      uint8_t* dfl_data = Om_decodeDataUri(&dfl_size, mimetype, charset, description_node.content());

      if(dfl_data) {

        size_t txt_size = description_node.attrAsInt(L"bytes");

        uint8_t* txt_data = Om_zInflate(dfl_data, dfl_size, txt_size);

        Om_free(dfl_data);

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
bool OmNetPack::refreshAnalytics()
{
  if(!this->_ModChan)
    return false;

  uint32_t new_stat = 0;
  uint32_t old_stat = 0;

  // set bits for old state to be compared to new state
  if(this->_has_part)       OM_ADD_BIT(old_stat, 0x01);
  if(this->_has_local)      OM_ADD_BIT(old_stat, 0x02);
  if(this->_has_misg_dep)   OM_ADD_BIT(old_stat, 0x04);
  if(this->_upgrade.size()) OM_ADD_BIT(old_stat, 0x08);
  if(this->_dngrade.size()) OM_ADD_BIT(old_stat, 0x10);

  // reset all values
  this->_has_part = false;
  this->_has_local = false;
  this->_has_misg_dep = false;
  this->_upgrade.clear();
  this->_dngrade.clear();

  //check for resumable download data
  if( Om_isFile(Om_concatPathsExt(this->_ModChan->libraryPath(), this->_file, L"dl_part"))) {
    this->_has_part = true;
    OM_ADD_BIT(new_stat, 0x01);
  }

  for(size_t i = 0; i < this->_ModChan->modpackCount(); ++i) {

    OmModPack* ModPack = this->_ModChan->getModpack(i);

    // we ignore directory sources
    if(ModPack->sourceIsDir())
      continue;

    // search for same core but different version
    if(this->_core == ModPack->core()) {

      if(this->_iden == ModPack->iden()) {

        this->_has_local = true;
        OM_ADD_BIT(new_stat, 0x02);

        // check for missing dependencies
        if(this->_ModChan->hasMissingDepend(ModPack)) {
          this->_has_misg_dep = true;
          OM_ADD_BIT(new_stat, 0x04);
        }

      } else {

        // check versions
        if(this->_version > ModPack->version()) {

          this->_upgrade.push_back(ModPack);
          OM_ADD_BIT(new_stat, 0x08);

        } else {

          this->_dngrade.push_back(ModPack);
          OM_ADD_BIT(new_stat, 0x10);
        }

      }
    }
  }

  return (new_stat != old_stat);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetPack::revokeDownload()
{
  if(!this->_ModChan)
    return;

  Om_fileDelete(Om_concatPathsExt(this->_ModChan->libraryPath(), this->_file, L"dl_part"));

  this->refreshAnalytics();
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
  this->_connect.abortRequest();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetPack::startDownload(Om_downloadCb download_cb, Om_resultCb result_cb, void* user_ptr, uint32_t rate)
{
  if(this->_connect.isPerforming()) {
    return true; //< avoid error
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

  // check for exception when download part is actually the completed download, in this case
  // we call result callback directly to prevent HTTP error 416
  if(Om_isFile(this->_dnl_temp)) {
     if(Om_itemSize(this->_dnl_temp) == this->_size) {
        OmNetPack::_dnl_download_fn(this, 100, 100, 0, 0L);
        OmNetPack::_dnl_result_fn(this, OM_RESULT_OK, 0L);
        return true;
     }
  }

  if(!this->_connect.requestHttpGet(this->_down_url, this->_dnl_temp, true, OmNetPack::_dnl_result_fn, OmNetPack::_dnl_download_fn, this, rate)) {
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
  if(this->_dnl_result != OM_RESULT_OK) {

    // either download is still processing, aborted (paused) or it has never started, in all
    // case this function should not be called at this stage

    this->_error(L"finalizeDownload", L"invalid call");
    return false;
  }

  // check whether something exists
  if(!Om_isFile(this->_dnl_temp)) {

    this->_error(L"finalizeDownload", L"Received invalid data (no data)");
    this->_has_error = true;

    return false;
  }

  // wait to get handle to file to prevent concurrency with directory notification
  // thread and prevent useless multiple notifications to be sent

  HANDLE hFile;
  uint8_t attempt = 20;
  while(attempt--) {
    hFile = CreateFileW(this->_dnl_temp.c_str(),
                        GENERIC_READ|GENERIC_WRITE|DELETE,
                        0, //< exclusive control
                        nullptr,OPEN_EXISTING,
                        0,nullptr);

    if(hFile != INVALID_HANDLE_VALUE) break; else Sleep(25);
  }

  // do we have read/write access problem on the file ?
  if(hFile == INVALID_HANDLE_VALUE) {

    this->_error(L"finalizeDownload", L"Unable to get proper downloaded data read/write access");
    this->_has_error = true;

    return false;
  }

  #ifdef DEBUG
  std::wcout << L"DEBUG => OmNetPack::finalizeDownload : Access File ("<< this->_dnl_temp << L")\n";
  #endif // DEBUG

  // check whether received data is a zip file
  if(!Om_isFileZip(hFile, false)) { //< perform a full-check to ensure file is valid

    Om_fileDelete(hFile);

    this->_error(L"finalizeDownload", L"Received invalid data (it is not a Mod package file)");
    this->_has_error = true;

    CloseHandle(hFile);

    return false;
  }

  // check for file size
  if(Om_fileSize(hFile) != this->_size) {

    Om_fileDelete(hFile);

    this->_error(L"finalizeDownload", L"Downloaded file size mismatch the reference");
    this->_has_error = true;

    CloseHandle(hFile);

    return false;
  }

  // compare checksum
  bool checksum_ok = false;

  if(this->_csum_is_md5) {
    checksum_ok = Om_cmpMD5sum(hFile, this->_csum);
  } else {
    checksum_ok = Om_cmpXXHsum(hFile, this->_csum);
  }

  if(checksum_ok) {

    int32_t result = Om_fileRename(hFile, this->_dnl_path, true);
    if(result != 0) {
      Om_fileDelete(hFile);
      this->_error(L"finalizeDownload", Om_errRename(L"Temporary file", this->_dnl_temp, result));
      this->_has_error = true;
    }

  } else {

    Om_fileDelete(hFile);

    this->_error(L"finalizeDownload", L"Downloaded file checksum mismatch the reference");
    this->_has_error = true;
  }

  CloseHandle(hFile);

  // We now wait until the Channel local Library updated so everything is
  // up to date for further operations, notably upgrade.
  attempt = 20;
  while(attempt--) {
    if(this->_ModChan->findModpack(this->iden(), true)) break; else Sleep(25);
  }

  return !this->_has_error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmNetPack::_dnl_download_fn(void* ptr, int64_t tot, int64_t cur, int64_t rate, uint64_t param)
{
  OM_UNUSED(param);

  OmNetPack* self = static_cast<OmNetPack*>(ptr);

  self->_dnl_percent = ((double)cur / tot) * 100.0;
  self->_dnl_remain = (double)(tot - cur) / rate;

  if(self->_cli_download_cb) {
    return self->_cli_download_cb(self->_cli_ptr, tot, cur, rate, reinterpret_cast<uint64_t>(self));
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmNetPack::_dnl_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OM_UNUSED(param);

  OmNetPack* self = static_cast<OmNetPack*>(ptr);

  self->_dnl_result = result;

  if(self->_dnl_result == OM_RESULT_ERROR) {

    // delete temporary file if nothing was download
    if(Om_itemSize(self->_dnl_temp) == 0)
      Om_fileDelete(self->_dnl_temp);

    self->_error(L"_dnl_result_fn", self->_connect.lastError());
    self->_has_error = true;
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
  OmModPack* this_ModPack = this->_ModChan->findModpack(this->iden(), true);
  if(!this_ModPack) {
    this->_error(L"upgradeReplace", L"corresponding Mod Pack object not found in Mod library");
    return OM_RESULT_ERROR;
  }

  bool has_error = false;

  #ifdef DEBUG
  // simulate slow progressing process
  for(size_t i = 0; i < 100; ++i) {
    OmNetPack::_upg_progress_fn(this, 100, i, 0);
    Sleep(25);
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
  this->_upgrade.clear();

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
  OM_UNUSED(param);

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
