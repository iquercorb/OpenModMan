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
#include "OmBase.h"
#include <algorithm>            //< std::sort

#include "OmBaseApp.h"

#include "OmArchive.h"          //< Archive compression methods / level

#include "OmModMan.h"

#include "OmUtilFs.h"
#include "OmUtilAlg.h"
#include "OmUtilHsh.h"
#include "OmUtilErr.h"
#include "OmUtilStr.h"

#include <commctrl.h>           //< ExtractIconW

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmModHub.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModHub::OmModHub(OmModMan* ModMan) :
  _ModMan(ModMan),
  _icon_handle(nullptr),
  _active_channel(-1),
  _setup_abort(false),
  _setup_hth(nullptr),
  _setup_hwo(nullptr),
  _setup_dones(0),
  _setup_percent(0),
  _setup_begin_cb(nullptr),
  _setup_progress_cb(nullptr),
  _setup_result_cb(nullptr),
  _setup_user_ptr(nullptr),
  _presets_quietmode(true)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModHub::~OmModHub()
{
  this->close();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::close()
{
  this->_xmlconf.clear();

  this->_path.clear();
  this->_home.clear();
  this->_uuid.clear();
  this->_title.clear();

  this->_icon_source.clear();
  if(this->_icon_handle)
    DestroyIcon(this->_icon_handle);
  this->_icon_handle = nullptr;

  for(size_t i = 0; i < this->_channel_list.size(); ++i)
    delete this->_channel_list[i];

  this->_channel_list.clear();

  this->_active_channel = -1;

  for(size_t i = 0; i < this->_preset_list.size(); ++i)
    delete this->_preset_list[i];

  this->_preset_list.clear();

  this->_setup_abort = false;
  Om_clearThread(this->_setup_hth, this->_setup_hwo);
  this->_setup_hth = nullptr;
  this->_setup_hwo = nullptr;
  this->_setup_dones = 0;
  this->_setup_percent = 0;
  this->_setup_begin_cb = nullptr;
  this->_setup_progress_cb = nullptr;
  this->_setup_result_cb = nullptr;
  this->_setup_user_ptr = nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::open(const OmWString& path)
{
  this->close();

  // Migrate old standard to new standard
  if(!this->_migrate(path)) {
    this->_error(L"open", L"Migration required but has failed (see logs more details)");
    return false;
  }

  // Check whether path is valid
  if(!Om_isDir(path)) {
    this->_error(L"open", Om_errNotDir(L"Mod Hub home", path));
    return false;
  }

  // Expected Mod Hub definition file path
  OmWString modhub_path;
  Om_concatPaths(modhub_path, path, OM_MODHUB_FILENAME);

  // Check whether definition file exists
  if(!Om_isFile(modhub_path)) {
    this->_error(L"open", L"Expected definition file (ModHub.xml) not found");
    return false;
  }

  // try to open and parse the XML file
  if(!this->_xmlconf.load(modhub_path, OM_XMAGIC_HUB)) {
    this->_error(L"open", Om_errParse(L"Definition file", L"ModHub.xml", this->_xmlconf.lastErrorStr()));
    return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_xmlconf.hasChild(L"uuid") || !this->_xmlconf.hasChild(L"title")) {
    this->_error(L"open", Om_errParse(L"Definition file", L"ModHub.xml", L"missing essential node(s)"));
    return false;
  }

  // right now this Mod Hub appear usable, even if it is empty
  this->_home = path;
  this->_path = modhub_path;
  this->_uuid = this->_xmlconf.child(L"uuid").content();
  this->_title = this->_xmlconf.child(L"title").content();

  // lookup for a icon
  if(this->_xmlconf.hasChild(L"icon")) {

    // we got a banner
    this->_icon_source = this->_xmlconf.child(L"icon").content();

    HICON hIc = ExtractIconW(nullptr, this->_icon_source.c_str(), 0);
    //HICON hIc = nullptr;
    //ExtractIconExW(this->_icon_source.c_str(), 0, &hIc, nullptr, 1); //< large icon

    if(hIc) {
      this->_icon_handle = hIc;
    } else {
      this->_log(OM_LOG_WRN, L"open", L"application icon extraction failed.");
    }
  }

  // we check for saved setup quiet mode option
  if(this->_xmlconf.hasChild(L"batches_quietmode")) {
    this->_presets_quietmode = this->_xmlconf.child(L"batches_quietmode").attrAsInt(L"enable");
  } else {
    this->setPresetQuietMode(this->_presets_quietmode); //< create default
  }

  // load Mod Channels for this Mod Hub
  OmWStringArray subdir;
  Om_lsDir(&subdir, this->_home, true);

  OmWString modchan_path;
  for(size_t i = 0; i < subdir.size(); ++i) {

    // Expected Mod Chan definition file path
    Om_concatPaths(modchan_path, subdir[i], OM_MODCHN_FILENAME);

    if(Om_isFile(modchan_path)) {

      OmModChan* ModChan = new OmModChan(this);

      if(ModChan->open(modchan_path)) {
        this->_channel_list.push_back(ModChan);
      } else {
        delete ModChan;
      }
    }
  }

  // expected Script library path
  OmWString presets_path = Om_concatPaths(this->_home, OM_MODHUB_MODPSET_DIR);

  // load Script for this Mod Hub
  OmWStringArray filename;
  Om_lsFileFiltered(&filename, presets_path, L"*.xml", true);

  for(size_t i = 0; i < filename.size(); ++i) {

    OmModPset* ModPset = new OmModPset(this);

    if(ModPset->open(filename[i])) {
      this->_preset_list.push_back(ModPset);
    } else {
      delete ModPset;
    }
  }

  // sort Mod Channel by index
  this->sortChannels();

  this->sortPresets();

  // the first location in list become the default active one
  if(this->_channel_list.size())
    this->selectChannel(0);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::setTitle(const OmWString& title)
{
  if(this->_xmlconf.valid()) {

    this->_title = title;

    if(this->_xmlconf.hasChild(L"title")) {
      this->_xmlconf.child(L"title").setContent(title);
    } else {
      this->_xmlconf.addChild(L"title").setContent(title);
    }

    this->_xmlconf.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::setIcon(const OmWString& path)
{
  if(this->_xmlconf.valid()) {

    // delete previous object
    if(this->_icon_handle)
      DestroyIcon(this->_icon_handle);

    this->_icon_handle = nullptr;

    // empty source path mean remove icon
    if(!path.empty()) {

      HICON hIcon = nullptr;

      if(Om_isFile(path))
        ExtractIconExW(path.c_str(), 0, &hIcon, nullptr, 1);

      if(hIcon) {

        this->_icon_source = path;
        this->_icon_handle = hIcon;

        if(this->_xmlconf.hasChild(L"icon")) {
          this->_xmlconf.child(L"icon").setContent(this->_icon_source);
        } else {
          this->_xmlconf.addChild(L"icon").setContent(this->_icon_source);
        }

      } else {
        this->_log(OM_LOG_WRN, L"setIcon", L"icon extraction failed");
      }
    }

    if(!this->_icon_handle) {

      if(this->_xmlconf.hasChild(L"icon")) {
        this->_xmlconf.remChild(this->_xmlconf.child(L"icon"));
      }
    }

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModChan* OmModHub::findChannel(const OmWString& uuid) const
{
  for(size_t i = 0; i < this->_channel_list.size(); ++i) {

    if(this->_channel_list[i]->uuid() == uuid)

      return this->_channel_list[i];
  }

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::_compare_chn_index(const OmModChan* a, const OmModChan* b)
{
  return (a->index() < b->index());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::sortChannels()
{
  sort(this->_channel_list.begin(), this->_channel_list.end(), OmModHub::_compare_chn_index);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::selectChannel(int32_t i)
{
  if(i >= 0) {

    if(i < (int)this->_channel_list.size()) {

      this->_active_channel = i;

    } else {

      return false;
    }

  } else {

    this->_active_channel = -1;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::selectChannel(const OmWString& uuid)
{
  for(size_t i = 0; i < this->_channel_list.size(); ++i) {

    if(this->_channel_list[i]->uuid() == uuid) {

      this->_active_channel = i;

      return true;
    }
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModChan* OmModHub::activeChannel() const
{
  if(this->_active_channel >= 0)
    return this->_channel_list[this->_active_channel];

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmModHub::indexOfChannel(const OmWString& uuid)
{
  for(size_t i = 0; i < this->_channel_list.size(); ++i)
    if(this->_channel_list[i]->uuid() == uuid)
      return i;

  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmModHub::indexOfChannel(const OmModChan* ModChan)
{
  for(size_t i = 0; i < this->_channel_list.size(); ++i)
    if(this->_channel_list[i] == ModChan)
      return i;

  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::createChannel(const OmWString& title, const OmWString& target, const OmWString& library, const OmWString& backup)
{
  // compose Mod Channel home path
  OmWString channel_home = Om_concatPaths(this->_home, title);

  // create Mod Channel sub-folder
  if(!Om_isDir(channel_home)) {
    int32_t result = Om_dirCreate(channel_home);
    if(result != 0) {
      this->_error(L"createChannel", Om_errCreate(L"Mod Channel home", channel_home, result));
      return false;
    }
  } else {
    this->_log(OM_LOG_WRN, L"createChannel", Om_errExists(L"Mod Channel home", channel_home));
  }

  // initialize new definition file
  OmXmlConf channel_def(OM_XMAGIC_CHN);

  // define uuid and title in definition file
  channel_def.addChild(L"uuid").setContent(Om_genUUID());
  channel_def.addChild(L"title").setContent(title);
  channel_def.child(L"title").setAttr(L"index", static_cast<int>(this->_channel_list.size()));

  channel_def.addChild(L"install").setContent(target);

  // checks whether we have custom Backup directory
  if(backup.empty()) {
    // Create the default backup sub-directory
    OmWString backup_path = Om_concatPaths(channel_home, OM_MODCHAN_BACKUP_DIR);
    Om_dirCreate(backup_path);
  } else {
    // add custom backup in definition
    channel_def.addChild(L"backup").setContent(backup);
  }

  // checks whether we have custom Library directory
  if(library.empty()) {
    // Create the default library sub-directory
    OmWString library_path = Om_concatPaths(channel_home, OM_MODCHAN_MODLIB_DIR);
    Om_dirCreate(library_path);
  } else {
    // add custom library in definition
    channel_def.addChild(L"library").setContent(library);
  }

  // compose Mod Channel definition file name
  OmWString channel_path = Om_concatPaths(channel_home, OM_MODCHN_FILENAME);

  // save and close definition file
  if(!channel_def.save(channel_path)) {
    this->_error(L"createChannel", Om_errSave(L"definition file", channel_path, channel_def.lastErrorStr()));
    return false;
  }

  // load the newly created Mod Channel
  OmModChan* ModChan = new OmModChan(this);
  ModChan->open(channel_path);
  this->_channel_list.push_back(ModChan);

  // sort channels and select the last one
  this->sortChannels();
  this->selectChannel(this->_channel_list.size() - 1);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::deleteChannel(size_t index)
{
  if(index >= this->_channel_list.size()) {
    this->_error(L"deleteChannel", L"index out of bound");
    return false;
  }

  OmModChan* ModChan = this->_channel_list[index];

  if(ModChan->hasBackupData()) {
    this->_error(L"deleteChannel", L"channel still have backup data to be restored");
    return false;
  }

  // keep Mod Channel paths
  OmWString channel_title = ModChan->title();
  OmWString channel_home = ModChan->home();
  OmWString channel_path = ModChan->path();

  // close Mod Channel
  ModChan->close();

  // remove the default backup folder
  OmWString bck_path = Om_concatPaths(channel_home, OM_MODCHAN_BACKUP_DIR);
  if(Om_isDir(bck_path)) {
    // this will fails if folder not empty, this is intended
    int32_t result = Om_dirDelete(bck_path);
    if(result != 0)
      this->_log(OM_LOG_WRN, L"deleteChannel", Om_errDelete(L"Backup directory", bck_path, result));
  }

  // remove the default Library folder
  OmWString lib_path = Om_concatPaths(channel_home, OM_MODCHAN_MODLIB_DIR);
  if(Om_isDir(lib_path)) {
    // this will fails if folder not empty, this is intended
    if(Om_isDirEmpty(lib_path)) {
      int32_t result = Om_dirDelete(lib_path);
      if(result != 0)
        this->_log(OM_LOG_WRN, L"deleteChannel", Om_errDelete(L"Library directory", lib_path, result));

    } else {
      this->_log(OM_LOG_WRN, L"deleteChannel", L"Non-empty Library directory was not deleted");
    }
  }

  bool has_error = false;

  // remove the definition file
  if(Om_isFile(channel_path)) {
    int32_t result = Om_fileDelete(channel_path);
    if(result != 0) {
      this->_error(L"deleteChannel", Om_errDelete(L"Definition file", channel_path, result));
      has_error = true;
    }
  }

  // check if location home directory is empty, if yes, we delete it
  if(Om_isDirEmpty(channel_home)) {
    int result = Om_dirDelete(channel_home);
    if(result != 0) {
      this->_error(L"deleteChannel", Om_errDelete(L"Home directory", channel_home, result));
      has_error = true; //< this is considered as a real error
    }
  } else {
    this->_log(OM_LOG_WRN, L"deleteChannel", L"Non-empty Mod Channel home directory was not deleted");
  }

  // delete object
  delete ModChan;

  // remove from list
  this->_channel_list.erase(this->_channel_list.begin() + index);

  // update locations order indexing
  for(size_t i = 0; i < this->_channel_list.size(); ++i)
    this->_channel_list[i]->setIndex(i);

  // sort Mod Channels by index
  this->sortChannels();
  this->selectChannel(0);

  return !has_error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::_compare_pst_index(const OmModPset* a, const OmModPset* b)
{
  return (a->index() < b->index());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::sortPresets()
{
  sort(this->_preset_list.begin(), this->_preset_list.end(), OmModHub::_compare_pst_index);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmModHub::indexOfPreset(const OmModPset* ModPset) const
{
  for(size_t i = 0; i < this->_preset_list.size(); ++i)
    if(ModPset == this->_preset_list[i])
      return i;

  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPset* OmModHub::createPreset(const OmWString& title)
{
  OmWString presets_dir = Om_concatPaths(this->_home, OM_MODHUB_MODPSET_DIR);

  // check whether Preset subdirectory exists
  if(!Om_isDir(presets_dir)) {

    // create Preset subdirectory
    int32_t result = Om_dirCreate(presets_dir);
    if(result != 0) {
      this->_error(L"createPreset", Om_errCreate(L"Presets directory", presets_dir, result));
      return nullptr;
    }
  }

  // Initialize new definition
  OmXmlConf preset_cfg;
  preset_cfg.init(OM_XMAGIC_SPT);

  // generate and set UUID
  preset_cfg.addChild(L"uuid").setContent(Om_genUUID());

  // Set <title> width index
  OmXmlNode xml_title = preset_cfg.addChild(L"title");
  xml_title.setContent(title);
  xml_title.setAttr(L"index", static_cast<int32_t>(this->_preset_list.size()));

  // create the <options> node
  preset_cfg.addChild(L"options").setAttr(L"installonly", 0);

  // compose path using title and context home
  OmWString preset_path = Om_concatPaths(presets_dir, title);
  preset_path += L".xml";

  // save definition file
  if(!preset_cfg.save(preset_path)) {
    this->_error(L"createPreset", Om_errSave(L"preset file", preset_path, preset_cfg.lastErrorStr()));
    return nullptr;
  }

  // Create new Mod Preset object
  OmModPset* ModPset = new OmModPset(this);

  if(!ModPset->open(preset_path)) {
    this->_error(L"createPreset", Om_errLoad(L"preset file", preset_path, ModPset->lastError()));
    return nullptr;
  }

  this->_preset_list.push_back(ModPset);

  // sort Batches by index
  this->sortPresets();

  return ModPset;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::deletePreset(size_t index)
{
  if(index >= this->_preset_list.size()) {
    this->_error(L"deletePreset", L"index out of bound");
    return false;
  }

  OmModPset* ModPset = this->_preset_list[index];

  // delete the definition file
  if(Om_isFile(ModPset->path())) {

    int32_t result = Om_fileDelete(ModPset->path());
    if(result != 0) {
      this->_error(L"deletePreset", Om_errDelete(L"preset file", ModPset->path(), result));
      return false;
    }
  }

  // delete batch object
  delete ModPset;

  // remove from list
  this->_preset_list.erase(this->_preset_list.begin() + index);

  // update batches order indexing
  for(size_t i = 0; i < this->_preset_list.size(); ++i) {
    this->_preset_list[i]->setIndex(i);
    this->_preset_list[i]->save();
  }

  // sort Batches by index
  this->sortPresets();

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::renamePreset(size_t index, const OmWString& title)
{
  if(index >= this->_preset_list.size()) {
    this->_error(L"renamePreset", L"index out of bound");
    return false;
  }

  OmModPset* ModPset = this->_preset_list[index];

  // change Preset title
  ModPset->setTitle(title);
  ModPset->save();

  // keep old Preset filename
  OmWString old_path = ModPset->path();

  // compose new Preset filename
  OmWString new_path = Om_concatPaths(Om_getDirPart(old_path), title);
  new_path += L".xml";

  bool has_error = false;

  // try to rename file
  ModPset->close();

  int32_t result = Om_fileMove(old_path, new_path);
  if(result != 0) {
    this->_error(L"rename", Om_errMove(L"Preset definition", old_path, result));
    new_path = old_path;
    has_error = true;
  }

  ModPset->open(new_path);

  return !has_error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::setPresetQuietMode(bool enable)
{
  this->_presets_quietmode = enable;

  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"batches_quietmode")) {
      this->_xmlconf.child(L"batches_quietmode").setAttr(L"enable", this->_presets_quietmode ? 1 : 0);
    } else {
      this->_xmlconf.addChild(L"batches_quietmode").setAttr(L"enable", this->_presets_quietmode ? 1 : 0);
    }

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::abortPresets()
{
  this->_setup_abort = true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::queuePresets(OmModPset* ModPset, Om_beginCb begin_cb, Om_progressCb progress_cb, Om_resultCb result_cb, void* user_ptr)
{
  if(this->_setup_queue.empty()) {
    /*
    // another operation is currently processing
    if(this->_locked_mod_library) {

      this->_log(OM_LOG_WRN, L"queueInstalls", L"local library is locked by another operation");

      if(result_cb)  // flush all results with abort
        for(size_t i = 0; i << selection.size(); ++i)
          result_cb(user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(selection[i]));

      return;
    }
    */

    this->_setup_begin_cb = begin_cb;
    this->_setup_progress_cb = progress_cb;
    this->_setup_result_cb = result_cb;
    this->_setup_user_ptr = user_ptr;

    // reset global progression parameters
    this->_setup_dones = 0;
    this->_setup_percent = 0;

  } else {

    // emit a warning in case a crazy client starts new download with
    // different parameters than current
    if(this->_setup_begin_cb != begin_cb ||
       this->_setup_result_cb != result_cb ||
       this->_setup_progress_cb != progress_cb ||
       this->_setup_user_ptr != user_ptr) {
      this->_log(OM_LOG_WRN, L"queuePresets", L"changing callbacks for a running thread is not allowed");
    }
  }
  /*
  // lock the local library to prevent concurrent array manipulation
  this->_locked_mod_library = true;
  */

  // reset abort flag
  this->_setup_abort = false;

  Om_push_backUnique(this->_setup_queue, ModPset);

  if(!this->_setup_hth) {

    // launch thread
    this->_setup_hth = Om_createThread(OmModHub::_setup_run_fn, this);
    this->_setup_hwo = Om_waitForThread(this->_setup_hth, OmModHub::_setup_end_fn, this);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmModHub::_setup_run_fn(void* ptr)
{
  OmModHub* self = static_cast<OmModHub*>(ptr);
  DWORD exit_code = 0;

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModHub::_setup_run_fn : enter\n";
  #endif // DEBUG

  OmPModPackArray installs, restores;

  while(self->_setup_queue.size()) {

    OmModPset* ModPset = self->_setup_queue.front();

    if(self->_setup_begin_cb)
      self->_setup_begin_cb(self->_setup_user_ptr, reinterpret_cast<uint64_t>(ModPset));

    // perform setup for each existing Mod Channel
    for(size_t i = 0; i < self->_channel_list.size(); ++i) {

      OmModChan* ModChan = self->_channel_list[i];

      installs.clear();
      restores.clear();

      ModPset->getSetupEntryList(ModChan, &installs);

      if(!ModPset->installOnly()) {

        // create the restores list
        for(size_t j = 0; j < ModChan->modpackCount(); ++j) {

          OmModPack* ModPack = ModChan->getModpack(j);

          if(!ModPack->hasBackup())
            continue;

          if(!Om_arrayContain(installs, ModPack))
            restores.push_back(ModPack);
        }
      }

      if(self->_setup_progress_cb) {

        size_t tot, cur;

        tot = 0; cur = restores.size();
        // we send restores fisrt
        for(size_t j = 0; j < restores.size(); ++j)
          self->_setup_progress_cb(self->_setup_user_ptr, tot, cur--, reinterpret_cast<uint64_t>(restores[j]));

        tot = installs.size(); cur = 0;
        // then send installs
        for(size_t j = 0; j < installs.size(); ++j)
          self->_setup_progress_cb(self->_setup_user_ptr, tot, ++cur, reinterpret_cast<uint64_t>(installs[j]));
      }

    }

    if(self->_setup_result_cb)
      self->_setup_result_cb(self->_setup_user_ptr, OM_RESULT_OK, reinterpret_cast<uint64_t>(ModPset));

    self->_setup_dones++;
    self->_setup_queue.pop_front();
  }

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModHub::_setup_run_fn : leave\n";
  #endif // DEBUG

  return exit_code;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmModHub::_setup_end_fn(void* ptr,uint8_t fired)
{
  OmModHub* self = static_cast<OmModHub*>(ptr);

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModHub::_setup_end_fn\n";
  #endif // DEBUG
/*
  // unlock the local library
  self->_locked_mod_library = false;
*/
  //DWORD exit_code = Om_threadExitCode(self->_install_hth);
  Om_clearThread(self->_setup_hth, self->_setup_hwo);

  self->_setup_dones = 0;
  self->_setup_percent = 0;

  self->_setup_hth = nullptr;
  self->_setup_hwo = nullptr;

  self->_setup_begin_cb = nullptr;
  self->_setup_progress_cb = nullptr;
  self->_setup_result_cb = nullptr;
  self->_setup_user_ptr = nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::_migrate(const OmWString& path)
{
  // Function to migrate Mod Hub structure and definitions files from old standard
  // to new standard
  OmXmlDoc xmldoc;
  OmXmlNode xmlnode;

  OmWStringArray filename;
  OmWStringArray to_delete;

  OmWString modhub_home = Om_isFile(path) ? Om_getDirPart(path) : path;

  // Check whether we got an OMC file, meaning migration is required
  Om_lsFileFiltered(&filename, modhub_home, L"*." OM_CTX_DEF_FILE_EXT, true);
  if(!filename.size())
    return true;

  OmWString presets_dir = Om_concatPaths(modhub_home, OM_MODHUB_MODPSET_DIR);

  // Here we go for Mod Hub set migration
  this->_log(OM_LOG_OK, L"_migrate", L"Found old fashion Mod Hub that must be migrated");

  // We first create the new 'OM_MODHUB_MODPSET_DIR' folder, this will also tell us
  // that Mod Hub directory is writable
  if(!Om_isDir(presets_dir)) {
    // Create "Preset" directory, fail silently
    int32_t result = Om_dirCreate(presets_dir);
    if(result != 0) {
      this->_error(L"_migrate", Om_errCreate(L"presets directory", presets_dir, result));
      return false;
    }
  }

  // We now migrate Preset files, xml 'magic' node is modified and file
  // saved into the new dedicated '_Script' folder

  // Search for Preset within Mod Hub home directory
  filename.clear();
  Om_lsFileFiltered(&filename, modhub_home, L"*." OM_BAT_DEF_FILE_EXT, true);

  // Modify and save each Preset file
  OmWString preset_path;
  OmXmlNodeArray location_nodes;
  for(size_t i = 0; i < filename.size(); ++i) {

    // Load definition and change the 'magic' node
    xmldoc.load(filename[i]);
    xmlnode = xmldoc.child(OM_XMAGIC_BAT);
    if(!xmlnode.empty()) xmlnode.setName(OM_XMAGIC_SPT);

    // Rename all Scripts <location> by <modchan>
    xmlnode.children(location_nodes, L"location");
    for(size_t i = 0; i < location_nodes.size(); ++i)
      location_nodes[i].setName(L"setup");

    // Save file with new name at new location
    preset_path = Om_concatPaths(presets_dir, Om_getNamePart(filename[i]));
    preset_path += L".xml";
    if(!xmldoc.save(preset_path)) {
      this->_error(L"_migrate", Om_errSave(L"preset file", preset_path, xmldoc.lastErrorStr()));
      return false;
    }

    xmldoc.clear();

    // add old file to be deleted
    to_delete.push_back(filename[i]);
  }

  // Now migrate Mod Channel(s)
  OmWStringArray subdir;
  Om_lsDir(&subdir, modhub_home, false);

  OmWString channel_home, channel_path;
  for(size_t i = 0; i < subdir.size(); ++i) {

    channel_home = Om_concatPaths(modhub_home, subdir[i]);

    // check for presence of old standard Mod Channel definition file
    filename.clear();
    Om_lsFileFiltered(&filename, channel_home, L"*." OM_LOC_DEF_FILE_EXT, true);

    // Parse the first file found
    if(filename.size()) {

      // Load definition and change the 'magic' node
      xmldoc.load(filename[0]);
      xmlnode = xmldoc.child(OM_XMAGIC_LOC);
      if(!xmlnode.empty()) xmlnode.setName(OM_XMAGIC_CHN);

      // migrate backup zip options to new standard with default values
      xmlnode = xmlnode.child(L"backup_comp");
      if(!xmlnode.empty()) {
        xmlnode.setAttr(L"method", (int)OM_METHOD_ZSTD);
        xmlnode.setAttr(L"level", (int)OM_LEVEL_FAST);
      }

      channel_path = Om_concatPaths(channel_home, OM_MODCHN_FILENAME);

      // Save file with new name
      if(!xmldoc.save(channel_path)) {
        this->_error(L"_migrate", Om_errSave(L"Mod Channel definition file", channel_path, xmldoc.lastErrorStr()));
        return false;
      }

      xmldoc.clear();

      // add old file to be deleted
      to_delete.push_back(filename[0]);
    }
  }

  // Finally migrate the Mod Hub definition file
  filename.clear();
  Om_lsFileFiltered(&filename, modhub_home, L"*." OM_CTX_DEF_FILE_EXT, true);

  if(filename.size()) {

    // Load definition and change the 'magic' node
    xmldoc.load(filename[0]);
    xmlnode = xmldoc.child(OM_XMAGIC_CTX);
    if(!xmlnode.empty()) xmlnode.setName(OM_XMAGIC_HUB);

    OmWString modhub_path = Om_concatPaths(modhub_home, OM_MODHUB_FILENAME);

    // Save file with new name
    if(!xmldoc.save(modhub_path)) {
      this->_error(L"_migrate", Om_errSave(L"Mod Hub definition file", modhub_path, xmldoc.lastErrorStr()));
      return false;
    }

    xmldoc.clear();

    // add old file to be deleted
    to_delete.push_back(filename[0]);
  }

  // Here we go for Mod Hub set migration
  this->_log(OM_LOG_OK, L"_migrate", L"Migration appear successful, cleaning old data.");

  for(size_t i = 0; i < to_delete.size(); ++i) {
    int32_t result = Om_fileDelete(to_delete[i]);
    if(result != 0) {
      this->_error(L"_migrate", Om_errDelete(L"old file", to_delete[i], result));
      return false;
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::_log(unsigned level, const OmWString& origin,  const OmWString& detail) const
{
  OmWString root(L"ModHub["); root.append(this->_title); root.append(L"].");
  this->_ModMan->escalateLog(level, root + origin, detail);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::_error(const OmWString& origin, const OmWString& detail)
{
  this->_lasterr = detail;
  this->_log(OM_LOG_ERR, origin, detail);
}
