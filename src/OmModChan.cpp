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
#include <algorithm>            //< std::find

#include "OmBaseApp.h"

#include "OmUtilFs.h"
#include "OmUtilHsh.h"
#include "OmUtilErr.h"
#include "OmUtilStr.h"
#include "OmUtilAlg.h"
#include "OmUtilPkg.h"

#include "OmArchive.h"          //< Archive compression methods / level

#include "OmModMan.h"
#include "OmModHub.h"
#include "OmNetRepo.h"

#include "OmModPack.h"
#include "OmNetPack.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmModChan.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModChan::OmModChan(OmModHub* ModHub) :
  _Modhub(ModHub),
  _index(0),
  _cust_library_path(false),
  _cust_backup_path(false),
  _modpack_list_sort(OM_SORT_NAME),
  _netpack_list_sort(OM_SORT_NAME),
  _modpack_notify_cb(nullptr),
  _modpack_notify_ptr(nullptr),
  _netpack_notify_cb(nullptr),
  _netpack_notify_ptr(nullptr),
  _locked_mod_library(false),
  _locked_net_library(false),
  _modops_abort(false),
  _modops_hth(nullptr),
  _modops_hwo(nullptr),
  _modops_dones(0),
  _modops_percent(0),
  _modops_begin_cb(nullptr),
  _modops_progress_cb(nullptr),
  _modops_result_cb(nullptr),
  _modops_notify_cb(nullptr),
  _modops_user_ptr(nullptr),
  _download_abort(false),
  _download_dones(0),
  _download_percent(0),
  _download_start_hth(nullptr),
  _download_start_hwo(nullptr),
  _download_begin_cb(nullptr),
  _download_download_cb(nullptr),
  _download_result_cb(nullptr),
  _download_notify_cb(nullptr),
  _download_user_ptr(nullptr),
  _supersed_abort(false),
  _supersed_hth(nullptr),
  _supersed_hwo(nullptr),
  _supersed_dones(0),
  _supersed_percent(0),
  _supersed_begin_cb(nullptr),
  _supersed_progress_cb(nullptr),
  _supersed_result_cb(nullptr),
  _supersed_notify_cb(nullptr),
  _supersed_user_ptr(nullptr),
  _query_abort(false),
  _query_hth(nullptr),
  _query_hwo(nullptr),
  _query_dones(0),
  _query_percent(0),
  _query_begin_cb(nullptr),
  _query_result_cb(nullptr),
  _query_notify_cb(nullptr),
  _query_user_ptr(nullptr),
  _library_devmode(true),
  _library_showhidden(false),
  _library_cleanunins(false),
  _warn_overlaps(true),
  _warn_extra_inst(true),
  _backup_method(OM_METHOD_ZSTD),
  _backup_level(OM_LEVEL_FAST),
  _backup_overlap(false),
  _warn_extra_unin(true),
  _warn_extra_dnld(true),
  _warn_miss_deps(true),
  _warn_miss_dnld(true),
  _warn_upgd_brk_deps(true),
  _upgd_rename(false),
  _down_max_rate(0),
  _down_max_thread(0),
  _layout_repositories_span(70)
{
  // set parameters for library monitor
  this->_monitor.setCallback(OmModChan::_monitor_notify_fn, this);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModChan::~OmModChan()
{
  this->close();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::close()
{
  // stop library monitoring
  this->_monitor.stopMonitor();

  // stop and clear ModOps thread
  if(this->_modops_hth) {
    this->_modops_abort = true;
    WaitForSingleObject(this->_modops_hth, 1000);
  }

  Om_threadClear(this->_modops_hth, this->_modops_hwo);
  this->_modops_hth = nullptr;
  this->_modops_hwo = nullptr;

  // stop and clear Supersed thread
  if(this->_supersed_hth) {
    this->_supersed_abort = true;
    WaitForSingleObject(this->_supersed_hth, 1000);
  }
  Om_threadClear(this->_supersed_hth, this->_supersed_hwo);
  this->_supersed_hth = nullptr;
  this->_supersed_hwo = nullptr;

  // stop and clear Queries thread
  if(this->_query_hth) {
    this->_query_abort = true;
    WaitForSingleObject(this->_query_hth, 1000);
  }
  Om_threadClear(this->_query_hth, this->_query_hwo);
  this->_query_hth = nullptr;
  this->_query_hwo = nullptr;

  // stop all downloads
  if(!this->_download_queue.empty()) {
    // FIXME : Je ne sais pas si ce truc fonctionne
    this->stopDownloads();
    while(this->_download_queue.size())
      Sleep(50);
  }

  this->_lasterr.clear();

  this->_xml.clear();

  this->_path.clear();
  this->_home.clear();
  this->_uuid.clear();
  this->_title.clear();
  this->_index = 0;
  this->_target_path.clear();
  this->_library_path.clear();
  this->_backup_path.clear();

  this->clearModLibrary();
  this->_modpack_list_sort = OM_SORT_NAME;
  this->clearNetLibrary();
  this->_netpack_list_sort = OM_SORT_NAME;

  for(size_t i = 0; i < this->_repository_list.size(); ++i)
    delete this->_repository_list[i];
  this->_repository_list.clear();

  this->_modpack_notify_cb = nullptr;
  this->_modpack_notify_ptr = nullptr;

  this->_netpack_notify_cb = nullptr;
  this->_netpack_notify_ptr = nullptr;

  this->_locked_mod_library = false;
  this->_locked_net_library = false;

  this->_modops_abort = false;
  this->_modops_dones = 0;
  this->_modops_percent = 0;
  this->_modops_queue.clear();
  this->_modops_begin_cb = nullptr;
  this->_modops_progress_cb = nullptr;
  this->_modops_result_cb = nullptr;
  this->_modops_notify_cb = nullptr;
  this->_modops_user_ptr = nullptr;

  this->_download_abort = false;
  this->_download_dones = 0;
  this->_download_percent = 0;
  this->_download_start_hth = nullptr;
  this->_download_start_hwo = nullptr;
  this->_download_queue.clear();
  this->_download_array.clear();
  this->_download_begin_cb = nullptr;
  this->_download_download_cb = nullptr;
  this->_download_result_cb = nullptr;
  this->_download_notify_cb = nullptr;
  this->_download_user_ptr = nullptr;

  this->_supersed_abort = false;
  this->_supersed_dones = 0;
  this->_supersed_percent = 0;
  this->_supersed_queue.clear();
  this->_supersed_begin_cb = nullptr;
  this->_supersed_progress_cb = nullptr;
  this->_supersed_result_cb = nullptr;
  this->_supersed_notify_cb = nullptr;
  this->_supersed_user_ptr = nullptr;

  this->_query_abort = false;
  this->_query_queue.clear();
  this->_query_dones = 0;
  this->_query_percent = 0;
  this->_query_begin_cb = nullptr;
  this->_query_result_cb = nullptr;
  this->_query_notify_cb = nullptr;
  this->_query_user_ptr = nullptr;

  this->_cust_library_path = false;
  this->_library_devmode = true;
  this->_library_showhidden = false;
  this->_library_cleanunins = false;
  this->_warn_overlaps = true;
  this->_warn_extra_inst = true;
  this->_cust_backup_path = false;
  this->_backup_method = OM_METHOD_ZSTD;
  this->_backup_level = OM_LEVEL_FAST;
  this->_warn_extra_unin = true;
  this->_warn_extra_dnld = true;
  this->_warn_miss_deps = true;
  this->_warn_miss_dnld = true;
  this->_warn_upgd_brk_deps = true;
  this->_upgd_rename = false;
  this->_down_max_rate = 0;
  this->_down_max_thread = 0;
  this->_layout_repositories_span = 70;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::open(const OmWString& path)
{
  this->close();

  // try to open and parse the XML file
  if(!this->_xml.load(path, OM_XMAGIC_CHN)) {
    this->_error(L"open", L"XML definition parse error.");
    this->close(); return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_xml.hasChild(L"uuid") || !this->_xml.hasChild(L"title")) {
    this->_error(L"open", L"invalid XML definition: basic nodes missing.");
    this->close(); return false;
  }

  this->_path = path;

  this->_home = Om_getDirPart(this->_path);

  this->_uuid = this->_xml.child(L"uuid").content();

  this->_title = this->_xml.child(L"title").content();

  this->_index = this->_xml.child(L"title").attrAsInt(L"index");

  // check for the presence of <install> entry
  if(this->_xml.hasChild(L"install")) {

    this->_target_path = this->_xml.child(L"install").content();
    if(!Om_isDir(this->_target_path)) {
      this->_log(OM_LOG_WRN, L"open", Om_errNotDir(L"Target directory", this->_target_path));
    }

  } else {
    this->_error(L"open", L"Invalid XML definition: <install> node missing.");
    this->close(); return false;
  }

  // check for the presence of <library> entry for custom Library path
  if(this->_xml.hasChild(L"library")) {

    this->_cust_library_path = true;
    this->_library_path = this->_xml.child(L"library").content();

    if(!Om_isDir(this->_library_path)) {
      this->_log(OM_LOG_WRN, L"open", Om_errNotDir(L"custom Library directory", this->_library_path));
    }

  } else {

    // set default Mod library path
    this->_library_path = this->_home + OM_MODCHAN_MODLIB_DIR;

    if(!Om_isDir(this->_library_path)) {
      int32_t result = Om_dirCreate(this->_library_path);
      if(result != 0) {
        this->_error(L"open", Om_errCreate(L"Library directory", this->_library_path, result));
        this->close();  return false;
      }
    }
  }


  // check for the presence of <backup> entry for custom Backup path
  if(this->_xml.hasChild(L"backup")) {

    this->_cust_backup_path = true;
    this->_backup_path = this->_xml.child(L"backup").content();

    if(!Om_isDir(this->_backup_path)) {
      this->_log(OM_LOG_WRN, L"open", Om_errNotDir(L"Custom Backup directory", this->_backup_path));
    }

  } else {

    // set default Backup data path
    this->_backup_path = this->_home + OM_MODCHAN_BACKUP_DIR;

    if(!Om_isDir(this->_backup_path)) {
      int32_t result = Om_dirCreate(this->_backup_path);
      if(result != 0) {
        this->_error(L"open", Om_errCreate(L"Backup directory", this->_backup_path, result));
        this->close(); return false;
      }
    }
  }

  if(this->_xml.hasChild(L"backup_comp")) {

    // set need for XML saving
    bool has_changes = false;

    this->_backup_level = this->_xml.child(L"backup_comp").attrAsInt(L"level");

    // create default <method> if not exists
    if(!this->_xml.child(L"backup_comp").hasAttr(L"method")) {

      if(this->_backup_level < 0) {

        this->_xml.child(L"backup_comp").setAttr(L"method", -1);

      } else {

        this->_xml.child(L"backup_comp").setAttr(L"method", OM_METHOD_ZSTD);
      }

      has_changes = true;
    }

    // ensure consistent level values
    if(this->_backup_level > OM_LEVEL_BEST) {
      this->_backup_level = OM_LEVEL_BEST;
      has_changes = true;
    }

    if(this->_backup_level < OM_LEVEL_NONE) {
      this->_backup_level = OM_LEVEL_NONE;
      has_changes = true;
    }

    this->_backup_method = this->_xml.child(L"backup_comp").attrAsInt(L"method");

    // ensure consistent method values
    if(this->_backup_method > 95) {
      this->_backup_method = OM_METHOD_ZSTD;
      has_changes = true;
    }

    if(has_changes)
      // reset to consistent values
      this->setBackupComp(this->_backup_method, this->_backup_level);

  } else {
    // create default values
    this->setBackupComp(this->_backup_method, this->_backup_level);
  }

  if(this->_xml.hasChild(L"backup_overlap")) {
    this->_backup_overlap = this->_xml.child(L"backup_overlap").attrAsInt(L"enable");
  } else {
    // create default values
    this->setBackupOverlap(this->_backup_overlap); //< create default
  }

  if(this->_xml.hasChild(L"library_sort")) {
    this->_modpack_list_sort = this->_xml.child(L"library_sort").attrAsInt(L"sort");
  } else {
    // create default values
    this->_xml.addChild(L"library_sort").setAttr(L"sort", (int)this->_modpack_list_sort);
    this->_xml.save();
  }

  if(this->_xml.hasChild(L"library_devmode")) {
    this->_library_devmode = this->_xml.child(L"library_devmode").attrAsInt(L"enable");
  } else {
    // create default values
    this->setLibraryDevmod(this->_library_devmode);
  }

  if(this->_xml.hasChild(L"library_showhidden")) {
    this->_library_showhidden = this->_xml.child(L"library_showhidden").attrAsInt(L"enable");
  } else {
    // create default values
    this->setLibraryShowhidden(this->_library_showhidden); //< create default
  }

  if(this->_xml.hasChild(L"library_cleanunins")) {
    this->_library_cleanunins = this->_xml.child(L"library_cleanunins").attrAsInt(L"enable");
  } else {
    // create default values
    this->setLibraryCleanUnins(this->_library_cleanunins); //< create default
  }

  if(this->_xml.hasChild(L"remotes_sort")) {
    this->_netpack_list_sort = this->_xml.child(L"remotes_sort").attrAsInt(L"sort");
  } else {
    // create default values
    this->_xml.addChild(L"remotes_sort").setAttr(L"sort", (int)this->_netpack_list_sort);
    this->_xml.save();
  }

  // Check warnings options
  if(this->_xml.hasChild(L"warn_options")) {

    OmXmlNode warn_options_node = this->_xml.child(L"warn_options");

    if(warn_options_node.hasChild(L"warn_overlaps")) {
      this->_warn_overlaps = warn_options_node.child(L"warn_overlaps").attrAsInt(L"enable");
    } else {
      this->setWarnOverlaps(this->_warn_overlaps);
    }

    if(warn_options_node.hasChild(L"warn_extra_inst")) {
      this->_warn_extra_inst = warn_options_node.child(L"warn_extra_inst").attrAsInt(L"enable");
    } else {
      this->setWarnExtraInst(this->_warn_extra_inst);
    }

    if(warn_options_node.hasChild(L"warn_miss_deps")) {
      this->_warn_miss_deps = warn_options_node.child(L"warn_miss_deps").attrAsInt(L"enable");
    } else {
      this->setWarnMissDeps(this->_warn_miss_deps);
    }

    if(warn_options_node.hasChild(L"warn_extra_unin")) {
      this->_warn_extra_unin = warn_options_node.child(L"warn_extra_unin").attrAsInt(L"enable");
    } else {
      this->setWarnExtraUnin(this->_warn_extra_unin);
    }

  } else {

    // create default
    this->_xml.addChild(L"warn_options");
    this->setWarnOverlaps(this->_warn_overlaps);
    this->setWarnExtraInst(this->_warn_extra_inst);
    this->setWarnMissDeps(this->_warn_miss_deps);
    this->setWarnExtraUnin(this->_warn_extra_unin);
  }

  // get 'network' parameters (repositories and options)
  if(this->_xml.hasChild(L"network")) {

    OmXmlNode network_node = this->_xml.child(L"network");

    if(network_node.hasAttr(L"upgd_rename")) {
      this->_upgd_rename = network_node.attrAsInt(L"upgd_rename");
    } else {
      this->_upgd_rename = false;
    }

    OmXmlNodeArray repository_nodes;
    network_node.children(repository_nodes, L"repository");

    // Get network repository list
    OmWString base, name;

    for(size_t i = 0; i < repository_nodes.size(); ++i) {

      base = repository_nodes[i].attrAsString(L"base");
      name = repository_nodes[i].attrAsString(L"name");

      OmNetRepo* NetRepo = new OmNetRepo(this);

      if(!NetRepo->setCoordinates(base, name)) {
        delete NetRepo;
        NetRepo = nullptr;
      }

      if(NetRepo) {

        if(repository_nodes[i].hasAttr(L"title"))
          NetRepo->setTempTitle(repository_nodes[i].attrAsString(L"title"));

        this->_repository_list.push_back(NetRepo);
      }

    }

    if(network_node.hasChild(L"warn_extra_dnld")) {
      this->_warn_extra_dnld = network_node.child(L"warn_extra_dnld").attrAsInt(L"enable");
    } else {
      this->setWarnExtraDnld(this->_warn_extra_dnld);
    }

    if(network_node.hasChild(L"warn_miss_dnld")) {
      this->_warn_miss_dnld = network_node.child(L"warn_miss_dnld").attrAsInt(L"enable");
    } else {
      this->setWarnMissDnld(this->_warn_miss_dnld);
    }

    if(network_node.hasChild(L"warn_upgd_brk_deps")) {
      this->_warn_upgd_brk_deps = network_node.child(L"warn_upgd_brk_deps").attrAsInt(L"enable");
    } else {
      this->setWarnUpgdBrkDeps(this->_warn_upgd_brk_deps);
    }

    if(network_node.hasChild(L"down_limits")) {
      this->_down_max_rate = network_node.child(L"down_limits").attrAsInt(L"rate");
      this->_down_max_thread = network_node.child(L"down_limits").attrAsInt(L"thread");
    } else {
      this->setDownLimits(this->_down_max_rate, this->_down_max_thread);
    }

  } else {
    // create default
    this->_xml.addChild(L"network");
    this->setWarnExtraDnld(this->_warn_extra_dnld);
    this->setWarnMissDnld(this->_warn_miss_dnld);
    this->setWarnUpgdBrkDeps(this->_warn_upgd_brk_deps);
    this->setDownLimits(this->_down_max_rate, this->_down_max_thread);
  }

  // get 'layout' parameters
  if(this->_xml.hasChild(L"layout")) {
    OmXmlNode layout_node = this->_xml.child(L"layout");
    this->_layout_repositories_span = layout_node.child(L"repositories").attrAsInt(L"span");
  } else {
    // create default
    this->_xml.addChild(L"layout");
    this->setLayoutRepositoriesSpan(this->_layout_repositories_span);
  }

  // get 'tools' parameters
  if(this->_xml.hasChild(L"tools")) {

    OmXmlNode tools_node = this->_xml.child(L"tools");

    if(tools_node.hasChild(L"sources_path")) {
      this->_mods_sources_path = tools_node.child(L"sources_path").content();
    } else {
      this->setModsSourcesPath(L"");
    }

  } else {
    // create default
    this->_xml.addChild(L"tools");
    this->setModsSourcesPath(L"");
  }

  this->_log(OM_LOG_OK, L"open", L"OK");

  // Load library
  this->reloadModLibrary();

  // start library monitoring
  if(this->accessesLibrary(OM_ACCESS_DIR_READ))
    this->_monitor.startMonitor(this->_library_path);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModChan::rename(const OmWString& title)
{
  if(this->_locked_mod_library) {
    this->_error(L"setCustBackupPath", L"Mod library operation is currently processing");
    return OM_RESULT_ABORT;
  }

  // 1. change title in configuration
  this->setTitle(title);

  // store old parameter since we close the instance
  OmWString old_home = this->_home;

  // Close Mod Channel to safe rename and reload it after
  this->close();

  bool has_error = false;

  // compose new Mod Channel home folder
  OmWString new_home;
  Om_concatPaths(new_home, this->_Modhub->home(), title);

  // Rename Mod Channel home folder
  int32_t result = Om_fileMove(old_home, new_home);
  if(result != 0) {
    this->_error(L"rename", Om_errRename(L"Home directory", old_home, result));
    new_home = old_home;
    has_error = true;
  }

  // new Path to XML def
  OmWString new_path;
  Om_concatPaths(new_path, new_home, OM_MODCHN_FILENAME);

  // Reload location
  this->open(new_path);

  if(!has_error) {
    this->_log(OM_LOG_OK, L"renameHome", L"successfully renamed to \""+new_home+L"\"");
  }

  return has_error ? OM_RESULT_ERROR : OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::accessesTarget(uint32_t mask)
{
  // checks whether directory exists
  if(!Om_isDir(this->_target_path)) {
    this->_error(L"accessesTarget", Om_errNotDir(L"target directory", this->_target_path));
    return false;
  }

  // checks for proper permissions on folder
  if(!Om_checkAccess(this->_target_path, mask)) {

    // Check for special case of network folder
    if(Om_pathIsNetwork(this->_target_path.c_str())) {

      this->_log(OM_LOG_WRN, L"accessesTarget", L"Path is a shared network directory and "
                 "permissions cannot be properly evaluated");

    } else {
      // compose proper error according access mask
      if(OM_HAS_BIT(mask, OM_ACCESS_DIR_WRITE)) {
        this->_error(L"accessesTarget", Om_errWriteAccess(L"target directory", this->_target_path));
      } else {
        this->_error(L"accessesTarget", Om_errReadAccess(L"target directory", this->_target_path));
      }
      return false;
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::accessesLibrary(uint32_t mask)
{
  // checks whether directory exists
  if(!Om_isDir(this->_library_path)) {
    if(this->_cust_library_path) {
      this->_error(L"accessesLibrary", Om_errNotDir(L"library directory", this->_library_path));
      return false;
    } else {
      // try to create it
      int32_t result = Om_dirCreate(this->_library_path);
      if(result != 0) {
        this->_error(L"accessesLibrary", Om_errCreate(L"library directory", this->_library_path, result));
        return false;
      }
    }
  }

  // checks for proper permissions on folder
  if(!Om_checkAccess(this->_library_path, mask)) {

    // Check for special case of network folder
    if(Om_pathIsNetwork(this->_library_path.c_str())) {

      this->_log(OM_LOG_WRN, L"accessesLibrary", L"Path is a shared network directory and "
                 "permissions cannot be properly evaluated");

    } else {
      // compose proper error according access mask
      if(OM_HAS_BIT(mask, OM_ACCESS_DIR_WRITE)) {
        this->_error(L"accessesLibrary", Om_errWriteAccess(L"library directory", this->_library_path));
      } else {
        this->_error(L"accessesLibrary", Om_errReadAccess(L"library directory", this->_library_path));
      }
      return false;
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::accessesBackup(uint32_t mask)
{
  // checks whether directory exists
  if(!Om_isDir(this->_backup_path)) {
    if(this->_cust_backup_path) {
      this->_error(L"accessesBackup", Om_errNotDir(L"backup directory", this->_backup_path));
      return false;
    } else {
      // try to create it
      int32_t result = Om_dirCreate(this->_backup_path);
      if(result != 0) {
        this->_error(L"accessesBackup", Om_errCreate(L"backup directory", this->_backup_path, result));
        return false;
      }
    }
  }

    // checks for proper permissions on folder
  if(!Om_checkAccess(this->_backup_path, mask)) {

    // Check for special case of network folder
    if(Om_pathIsNetwork(this->_backup_path.c_str())) {

      this->_log(OM_LOG_WRN, L"accessesBackup", L"Path is a shared network directory and "
                 "permissions cannot be properly evaluated");

    } else {
      // compose proper error according access mask
      if(OM_HAS_BIT(mask, OM_ACCESS_DIR_WRITE)) {
        this->_error(L"accessesBackup", Om_errWriteAccess(L"backup directory", this->_backup_path));
      } else {
        this->_error(L"accessesBackup", Om_errReadAccess(L"backup directory", this->_backup_path));
      }
      return false;
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::notifyModLibraryStart(Om_notifyCb notify_cb, void* user_ptr)
{
  this->_modpack_notify_cb = notify_cb;
  this->_modpack_notify_ptr = user_ptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::notifyModLibraryStop()
{
  this->_modpack_notify_cb = nullptr;
  this->_modpack_notify_ptr = nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::notifyNetLibraryStart(Om_notifyCb notify_cb, void* user_ptr)
{
  this->_netpack_notify_cb = notify_cb;
  this->_netpack_notify_ptr = user_ptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::notifyNetLibraryStop()
{
  this->_netpack_notify_cb = nullptr;
  this->_netpack_notify_ptr = nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_monitor_notify_fn(void* ptr, OmNotify notify, uint64_t param)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  OmWString mod_path = reinterpret_cast<wchar_t*>(param);

  // ignore hidden file except if required
  if(!self->_library_showhidden && Om_isHidden(mod_path))
      return;

  // ignore directories except in dev mode
  if(!self->_library_devmode && Om_isDir(mod_path))
      return;

  // get name hash
  uint64_t name_hash = Om_getXXHash3(Om_getFilePart(mod_path));

  // Forwarded notification type
  OmNotify fw_notify = OM_NOTIFY_UNDEFINED;

  // search for Mod Pack object
  OmModPack* ModPack = self->findModpack(name_hash);

  if(notify == OM_NOTIFY_ALTERED) {

    // refresh Mod Pack source
    if(ModPack) {
      ModPack->refreshSource();
      // forward alternation notification
      fw_notify = OM_NOTIFY_ALTERED;
    }

    #ifdef DEBUG
    if(fw_notify)
      std::cout << "DEBUG => OmModChan::_monitor_notify_fn ~=\n";
    #endif
  }

  if(notify == OM_NOTIFY_CREATED) {

    // filter by directory / file extension
    if(Om_isDir(mod_path) ||
       Om_extensionMatches(mod_path, L"zip") ||
       Om_extensionMatches(mod_path, OM_PKG_FILE_EXT)) {

      // check whether this Mod Source matches an existing Backup
      if(ModPack) {
        ModPack->parseSource(mod_path);
        // forward alternation notification
        fw_notify = OM_NOTIFY_ALTERED;
      } else {
        // no Backup found for this Mod Source, adding new
        ModPack = new OmModPack(self);
        if(ModPack->parseSource(mod_path)) {
          self->_modpack_list.push_back(ModPack);
          // forward creation notification
          fw_notify = OM_NOTIFY_CREATED;
        } else {
          delete ModPack;
        }
      }

      #ifdef DEBUG
      if(fw_notify)
        std::cout << "DEBUG => OmModChan::_monitor_notify_fn ++\n";
      #endif
    }
  }

  if(notify == OM_NOTIFY_DELETED) {

    // delete or modify Mod Pack
    if(ModPack) {
      // check whether Mod Pack has backup data (is installed)
      if(ModPack->hasBackup()) {
        // Clear Mod Pack source side and
        ModPack->clearSource();
        // forward alternation notification
        fw_notify = OM_NOTIFY_ALTERED;
      } else {
        // Remove Mod Pack from Mod Library
        int32_t p = self->indexOfModpack(ModPack);
        self->_modpack_list.erase(self->_modpack_list.begin() + p);
        delete ModPack;
        // forward deletion notification
        fw_notify = OM_NOTIFY_DELETED;
      }
    }

    #ifdef DEBUG
    if(fw_notify)
      std::cout << "DEBUG => OmModChan::_monitor_notify_fn --\n";
    #endif
  }

  if(fw_notify) {

    // if an element was added to list we need to sort again
    if(fw_notify == OM_NOTIFY_CREATED) {

      self->sortModLibrary(); //< this will send rebuild notification

    } else {

      // this is a simple alteration we can optimize changes
      if(self->_modpack_notify_cb)
        self->_modpack_notify_cb(self->_modpack_notify_ptr, fw_notify, name_hash);
    }

    // As changes in local library may change status in Network library
    // we refresh Network library
    self->refreshNetLibrary();

  } else {

    // at this point, if not changes was made this mean the file is not a
    // Mod Pack, so we check whether this is an image or text file used
    // as thumbnail or description for a dev Mod directory
    if(self->_library_devmode) {

      // get presumed mod 'identity' from file name
      OmWString iden = Om_getNamePart(mod_path);

      for(size_t p = 0; p < self->_modpack_list.size(); ++p) {

        ModPack = self->_modpack_list[p];

        // we ignore non directory Mods
        if(!ModPack->sourceIsDir())
          continue;

        // we compare against Mod core name (without version)
        if(Om_namesMatches(iden, ModPack->core())) {

          ModPack->loadDirDescription();
          ModPack->loadDirThumbnail();

          // forward alteration notification
          if(self->_modpack_notify_cb)
            self->_modpack_notify_cb(self->_modpack_notify_ptr, OM_NOTIFY_ALTERED, ModPack->hash());

          #ifdef DEBUG
          std::cout << "DEBUG => OmModChan::_monitor_notify_fn //\n";
          #endif
        }
      }


    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::clearModLibrary()
{
  if(!this->_modpack_list.empty()) {

    for(size_t i = 0; i < this->_modpack_list.size(); ++i)
      delete this->_modpack_list[i];

    this->_modpack_list.clear();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::reloadModLibrary()
{
  // clear current library
  if(!this->_modpack_list.empty()) {

    for(size_t i = 0; i < this->_modpack_list.size(); ++i)
      delete this->_modpack_list[i];

    this->_modpack_list.clear();
  }

  if(!this->accessesLibrary(OM_ACCESS_DIR_READ)) { // check for read access
    #ifdef DEBUG
    std::cout << "DEBUG => OmModChan::reloadModLibrary X\n";
    #endif
    return;
  }

  OmWStringArray paths;

  // get Backup directory content
  Om_lsFileFiltered(&paths, this->_backup_path, L"*.zip", true, true);
  Om_lsFileFiltered(&paths, this->_backup_path, L"*." OM_BCK_FILE_EXT, true, true);
  Om_lsDir(&paths, this->_backup_path, true, true);

  // add all available and valid Backups
  for(size_t i = 0; i < paths.size(); ++i) {

    OmModPack* ModPack = new OmModPack(this);

    if(ModPack->parseBackup(paths[i])) {
      this->_modpack_list.push_back(ModPack);
    } else {
      delete ModPack;
    }

  }

  // get Library directory content
  paths.clear();
  Om_lsFileFiltered(&paths, this->_library_path, L"*.zip", true, this->_library_showhidden);
  Om_lsFileFiltered(&paths, this->_library_path, L"*." OM_PKG_FILE_EXT, true, this->_library_showhidden);
  if(this->_library_devmode)
    Om_lsDir(&paths, this->_library_path, true, this->_library_showhidden);

  // Link Sources to matching Backup, or add new Sources
  for(size_t i = 0; i < paths.size(); ++i) {

    uint64_t name_hash = Om_getXXHash3(Om_getFilePart(paths[i]));

    bool found = false;

    // check whether this Mod Source matches an existing Backup
    for(size_t p = 0; p < this->_modpack_list.size(); p++) {
      if(name_hash == this->_modpack_list[p]->hash()) {
        this->_modpack_list[p]->parseSource(paths[i]);
        found = true; break;
      }
    }

    // no Backup found for this Mod Source, adding new
    if(!found) {
      OmModPack* ModPack = new OmModPack(this);
      if(ModPack->parseSource(paths[i])) {
        this->_modpack_list.push_back(ModPack);
      } else {
        delete ModPack;
      }
    }
  }

  // sort library
  this->sortModLibrary(); //< this will send rebuild notification

  // refresh Mod Packs analytical parameters
  this->refreshModLibrary();

  // as changes in local library may change status in Network library
  // we also refresh Network library
  this->refreshNetLibrary();

  #ifdef DEBUG
  std::cout << "DEBUG => OmModChan::reloadModLibrary\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::refreshModLibrary()
{
  bool has_change = false;

  for(size_t i = 0; i < this->_modpack_list.size(); ++i) {

    // refresh Net Pack status
    if(this->_modpack_list[i]->refreshAnalytics()) {

      // notify changes
      if(this->_modpack_notify_cb)
        this->_modpack_notify_cb(this->_modpack_notify_ptr, OM_NOTIFY_ALTERED, this->_modpack_list[i]->hash());

      has_change = true;
    }
  }

  #ifdef DEBUG
  std::cout << "DEBUG => OmModChan::refreshModLibrary " << (has_change ? "~=" : "==") << "\n";
  #endif

  return has_change;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::ghostbusterModLibrary()
{
  bool has_change = false;

  // search for ghost packages
  for(size_t p = 0; p < this->_modpack_list.size(); ++p) {

    if(!this->_modpack_list[p]->hasBackup() && !this->_modpack_list[p]->hasSource()) {

      // The Package has no Backup and Source is no longer
      // available, so this is a ghost, we have to remove it
      OmModPack* ModPack = this->_modpack_list[p];

      // send library changes notifications
      if(this->_modpack_notify_cb)
        this->_modpack_notify_cb(this->_modpack_notify_ptr, OM_NOTIFY_DELETED, ModPack->hash());

      // delete object
      delete ModPack;

      // remove from list
      this->_modpack_list.erase(this->_modpack_list.begin() + p); --p;

      has_change = true;
    }
  }

  #ifdef DEBUG
  std::cout << "DEBUG => OmModChan::ghostbusterModLibrary " << (has_change ? "+-" : "==") << "\n";
  #endif

  return has_change;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::ghostbusterModPack(const OmModPack* ModPack)
{
  if(!ModPack->hasBackup() && !ModPack->hasSource()) {

    int32_t p = this->indexOfModpack(ModPack);

    if(p != 0) {

      // send library changes notifications
      if(this->_modpack_notify_cb)
        this->_modpack_notify_cb(this->_modpack_notify_ptr, OM_NOTIFY_DELETED, ModPack->hash());

      // delete object
      delete ModPack;

      // remove from list
      this->_modpack_list.erase(this->_modpack_list.begin() + p);

      return true;
    }
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModChan::addToLibrary(const OmWStringArray& files, Om_progressCb progress_cb, void* user_ptr)
{
  if(!this->_xml.valid())
    return OM_RESULT_ABORT;

  bool has_error = false;

  // create filtered list
  OmWStringArray src_path;

  for(size_t i = 0; i < files.size(); ++i)
    if(Om_extensionMatches(files[i], OM_PKG_FILE_EXT) || Om_extensionMatches(files[i], L"zip"))
      src_path.push_back(files[i]);

  // copy files
  OmWString dst_path;

  for(size_t i = 0; i < src_path.size(); ++i) {

    // call progress callback
    if(progress_cb)
      if(!progress_cb(user_ptr, src_path.size(), i, reinterpret_cast<uint64_t>(src_path[i].c_str())))
        return OM_RESULT_ABORT;

    // Compose destination path
    Om_concatPaths(dst_path, this->_library_path, Om_getFilePart(src_path[i]));

    int32_t result = Om_fileCopy(src_path[i], dst_path, true);
    if(result != 0) {
      this->_error(L"addToLibrary", Om_errCopy(L"Mod file", src_path[i], result));
      has_error = true;
    }

    #ifdef DEBUG
    Sleep(100);
    #endif // DEBUG
  }

  return has_error ? OM_RESULT_ERROR : OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModChan::importToLibrary(const OmWStringArray& paths, Om_progressCb progress_cb, Om_progressCb compress_cb, void* user_ptr)
{
  if(!this->_xml.valid())
    return OM_RESULT_ABORT;

  bool has_error = false;
  bool has_abort = false;

  OmModPack* ModPack = new OmModPack(this);

  OmWString filename;
  OmWString dst_path;

  for(size_t i = 0; i < paths.size(); ++i) {

    // call progress callback
    if(progress_cb)
      if(!progress_cb(user_ptr, paths.size(), i, reinterpret_cast<uint64_t>(paths[i].c_str()))) {
        has_abort = true; break;
      }

    // parse directory as Mod-package
    if(!ModPack->parseSource(paths[i])) {
      this->_error(L"importToLibrary", Om_errParse(L"Mod directory", Om_getFilePart(paths[i]), ModPack->lastError()));
      has_error = true;
      continue;
    }

    // compose filename
    filename = Om_spacesToUnderscores(ModPack->name());

    if(ModPack->version().valid()) {
      filename += L"_v";
      filename += ModPack->version().asString();
    }

    filename += L"." OM_PKG_FILE_EXT;

    // compose final destination path
    Om_concatPaths(dst_path, this->libraryPath(), filename);

    // create Mod-Package from directory
    OmResult result = ModPack->saveAs(dst_path, OM_METHOD_ZSTD, OM_LEVEL_SLOW, nullptr, compress_cb, user_ptr);
    if(result != OM_RESULT_OK) {
      if(result == OM_RESULT_ABORT) {
        has_abort = true; break;
      } else {
        has_error = true;
      }
    }

    #ifdef DEBUG
    Sleep(100);
    #endif // DEBUG
  }

  delete ModPack;

  if(has_abort || has_error)
    return has_error ? OM_RESULT_ERROR : OM_RESULT_ABORT;

  return OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmModChan::indexOfModpack(const OmModPack* ModPack) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i) {
    if(this->_modpack_list[i] == ModPack)
      return i;
  }

  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmModChan::indexOfModpack(uint64_t hash) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i) {
    if(this->_modpack_list[i]->hash() == hash)
      return i;
  }

  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPack* OmModChan::findModpack(uint64_t hash, bool nodir) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i) {

    if(nodir && this->_modpack_list[i]->sourceIsDir())
      continue;

    if(this->_modpack_list[i]->hash() == hash)
      return this->_modpack_list[i];
  }

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPack* OmModChan::findModpack(const OmWString& iden, bool nodir) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i) {

    if(nodir && this->_modpack_list[i]->sourceIsDir())
      continue;

    if(this->_modpack_list[i]->iden() == iden)
      return this->_modpack_list[i];
  }

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_compare_mod_name(const OmModPack* a, const OmModPack* b)
{
  // test against the shorter string
  size_t l = a->iden().size() < b->iden().size() ? b->iden().size() : a->iden().size();

  const wchar_t* a_srt = a->iden().c_str();
  const wchar_t* b_str = b->iden().c_str();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {

    wchar_t a_chr = towupper(a_srt[i]);
    wchar_t b_chr = towupper(b_str[i]);

    if(a_chr != b_chr) {
      if(a_chr < b_chr) {
        return true;
      } else {
        return false;
      }
    }
  }

  // strings are equals in tester portion, sort by string size
  if(a->iden().size() != b->iden().size())
    if(a->iden().size() < b->iden().size())
      return true;

  // strings are strictly equals, we sort by "IsZip" status
  if(!a->sourceIsDir() && b->sourceIsDir())
    return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_compare_mod_stat(const OmModPack* a, const OmModPack* b)
{
  if(a->hasBackup() == b->hasBackup()) {
    return OmModChan::_compare_mod_name(a, b);
  } else {
    return (a->hasBackup() && !b->hasBackup());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_compare_mod_vers(const OmModPack* a, const OmModPack* b)
{
  if(a->version() == b->version()) {
    return OmModChan::_compare_mod_name(a, b);
  } else {
    return (a->version() < b->version());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_compare_mod_cate(const OmModPack* a, const OmModPack* b)
{
  // test against the shorter string
  size_t l = a->category().size() < b->category().size() ? b->category().size() : a->category().size();

  const wchar_t* a_srt = a->category().c_str();
  const wchar_t* b_str = b->category().c_str();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {

    wchar_t a_chr = towupper(a_srt[i]);
    wchar_t b_chr = towupper(b_str[i]);

    if(a_chr != b_chr) {
      if(a_chr < b_chr) {
        return true;
      } else {
        return false;
      }
    }
  }

  // strings are equals in tester portion, sort by string size
  if(a->category().size() != b->category().size())
    if(a->category().size() < b->category().size())
      return true;

  // strings are strictly equals, we sort by name
  return OmModChan::_compare_mod_name(a, b);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::sortModLibrary()
{
  bool(*compare_func)(const OmModPack*,const OmModPack*) = nullptr;

  if(OM_HAS_BIT(this->_modpack_list_sort,OM_SORT_STAT)) compare_func = OmModChan::_compare_mod_stat;
  if(OM_HAS_BIT(this->_modpack_list_sort,OM_SORT_NAME)) compare_func = OmModChan::_compare_mod_name;
  if(OM_HAS_BIT(this->_modpack_list_sort,OM_SORT_VERS)) compare_func = OmModChan::_compare_mod_vers;
  if(OM_HAS_BIT(this->_modpack_list_sort,OM_SORT_CATE)) compare_func = OmModChan::_compare_mod_cate;

  if(compare_func)
    std::sort(this->_modpack_list.begin(), this->_modpack_list.end(), compare_func);

  // check whether we need a normal or reverse sorting
  if(OM_HAS_BIT(this->_modpack_list_sort,OM_SORT_INVT)) {
    std::reverse(this->_modpack_list.begin(), this->_modpack_list.end());
  }

  if(this->_modpack_notify_cb)
    this->_modpack_notify_cb(this->_modpack_notify_ptr, OM_NOTIFY_REBUILD, 0);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setModLibrarySort(OmSort sorting)
{
  if(!this->_xml.valid())
    return;

  // if requested sorting is same as current, reverse order
  if(OM_HAS_BIT(this->_modpack_list_sort, sorting)) {

    OM_TOG_BIT(this->_modpack_list_sort, OM_SORT_INVT);

  } else {

    this->_modpack_list_sort = sorting;
  }

  // save the current sorting
  if(this->_xml.hasChild(L"library_sort")) {
    this->_xml.child(L"library_sort").setAttr(L"sort", this->_modpack_list_sort);
  } else {
    this->_xml.addChild(L"library_sort").setAttr(L"sort", this->_modpack_list_sort);
  }

  this->_xml.save();

  this->sortModLibrary(); //< this will send rebuild notification
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::isOverlapped(size_t index) const
{
  uint64_t mod_hash = this->_modpack_list[index]->hash();

  for(size_t i = 0; i < this->_modpack_list.size(); ++i)
    if(this->_modpack_list[i]->hasOverlap(mod_hash))
      return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::isOverlapped(const OmModPack* ModPack) const
{
  uint64_t mod_hash = ModPack->hash();

  for(size_t i = 0; i < this->_modpack_list.size(); ++i)
    if(this->_modpack_list[i]->hasOverlap(mod_hash))
      return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::findOverlaps(const OmModPack* ModPack, OmUint64Array* overlaps) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i)
    if(this->_modpack_list[i]->hasBackup())
      if(ModPack->canOverlap(this->_modpack_list[i]))
        overlaps->push_back(this->_modpack_list[i]->hash());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::findOverlaps(const OmModPack* ModPack, OmPModPackArray* overlaps) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i)
    if(this->_modpack_list[i]->hasBackup())
      if(ModPack->canOverlap(this->_modpack_list[i]))
        overlaps->push_back(this->_modpack_list[i]);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::getOverlaps(const OmModPack* ModPack, OmPModPackArray* overlaps) const
{
  for(size_t i = 0; i < ModPack->overlapCount(); ++i)
    for(size_t j = 0; j < this->_modpack_list.size(); ++j)
      if(ModPack->getOverlapHash(i) == this->_modpack_list[j]->hash()) {
        overlaps->push_back(this->_modpack_list[j]); break;
      }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::backupEntryExists(const OmWString& path, int32_t attr) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i)
    if(this->_modpack_list[i]->hasBackup())
      if(this->_modpack_list[i]->backupHasEntry(path, attr))
        return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPack* OmModChan::findModDepend(const OmWString& filter, bool installed) const
{
  // parsed identity filter
  OmWString core, vers;

  if(Om_parseModIdent(filter, &core, &vers, nullptr)) {

    // gather candidates
    OmPModPackArray candidates;

    for(size_t i = 0; i < this->_modpack_list.size(); ++i) {

      if(installed && !this->_modpack_list[i]->hasBackup())
        continue;

      if(this->_modpack_list[i]->core() != core)
        continue;

      if(!this->_modpack_list[i]->version().match(vers))
        continue;

      candidates.push_back(this->_modpack_list[i]);
    }

    if(candidates.size()) {

      // more than one candidate, sort by version
      if(candidates.size() > 1)
        std::sort(candidates.begin(), candidates.end(), _compare_mod_vers);

      return candidates[candidates.size()-1];
    }

  } else {

    for(size_t i = 0; i < this->_modpack_list.size(); ++i) {

      if(installed && !this->_modpack_list[i]->hasBackup())
        continue;

      if(this->_modpack_list[i]->iden() == core)
        return this->_modpack_list[i];
    }
  }

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::isDependency(const OmModPack* ModPack, bool installed) const
{
  if(installed) {

    for(size_t i = 0; i < this->_modpack_list.size(); ++i)
      if(this->_modpack_list[i]->hasBackup() && ModPack != this->_modpack_list[i])
        //if(this->_modpack_list[i]->hasDepend(ModPack->iden()))
        if(this->_modpack_list[i]->matchDepend(ModPack))
          return true;

  } else {

    for(size_t i = 0; i < this->_modpack_list.size(); ++i)
      if(ModPack != this->_modpack_list[i])
        //if(this->_modpack_list[i]->hasDepend(ModPack->iden()))
        if(this->_modpack_list[i]->matchDepend(ModPack))
          return true;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::hasMissingDepend(const OmModPack* ModPack) const
{
  for(size_t i = 0; i < ModPack->dependCount(); ++i) {

    bool is_missing = true;
/*
    for(size_t j = 0; j < this->_modpack_list.size(); ++j) {

      // ignore directory Mods
      if(this->_modpack_list[j]->sourceIsDir())
        continue;

      if(ModPack->getDependIden(i) == this->_modpack_list[j]->iden()) {

        // recursively check
        if(this->hasMissingDepend(this->_modpack_list[j]))
          return true;

        is_missing = false;

        break;
      }
    }
*/
    OmModPack* DepMod = this->findModDepend(ModPack->getDependIden(i));

    if(DepMod && !DepMod->sourceIsDir()) {

      // recursively check
      if(this->hasMissingDepend(DepMod)) {

        is_missing = false;
        break;
      }
    }

    if(is_missing)
      return true;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::hasBrokenDepend(const OmModPack* ModPack) const
{
  for(size_t i = 0; i < ModPack->dependCount(); ++i) {

    bool is_broken = true;
/*
    for(size_t j = 0; j < this->_modpack_list.size(); ++j) {

      // ignore directory Mods
      if(this->_modpack_list[j]->sourceIsDir())
        continue;

      if(ModPack->getDependIden(i) == this->_modpack_list[j]->iden()) {

        // check whether Mod is installed
        if(!this->_modpack_list[j]->hasBackup())
          return true;

        // recursively check
        if(this->hasBrokenDepend(this->_modpack_list[j]))
          return true;

        is_broken = false;

        break;
      }
    }
*/
    OmModPack* DepMod = this->findModDepend(ModPack->getDependIden(i), true);

    // ignore directory Mods
    if(DepMod && !DepMod->sourceIsDir()) {

      // check whether Mod is installed
      if(!DepMod->hasBackup())
        return true;

      // recursively check
      if(this->hasBrokenDepend(DepMod))
        return true;

      is_broken = false;

      break;
    }

    if(is_broken)
      return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_get_modops_depends(const OmModPack* ModPack, OmPModPackArray* depends, OmWStringArray* missings) const
{
  for(size_t i = 0; i < ModPack->dependCount(); ++i) {

    bool is_missing = true;
    /*
    for(size_t j = 0; j < this->_modpack_list.size(); ++j) {

      // rely only on packages
      if(this->_modpack_list[j]->sourceIsDir())
        continue;


      if(ModPack->getDependIden(i) == this->_modpack_list[j]->iden()) {

        this->_get_modops_depends(this->_modpack_list[j], depends, missings);

        // we add to list only if unique and not already installed, this allow
        // us to get a consistent dependency list for a bunch of package by
        // calling this function for each package without clearing the list
        if(!this->_modpack_list[j]->hasBackup())
          Om_push_backUnique(*depends, this->_modpack_list[j]);

        is_missing = false;
        break;
      }
    }
    */

    OmModPack* DepMod = this->findModDepend(ModPack->getDependIden(i));

    if(DepMod && !DepMod->sourceIsDir()) {

      this->_get_modops_depends(DepMod, depends, missings);

      // we add to list only if unique and not already installed, this allow
      // us to get a consistent dependency list for a bunch of package by
      // calling this function for each package without clearing the list
      if(!DepMod->hasBackup())
        Om_push_backUnique(*depends, DepMod);

      is_missing = false;
      break;
    }

    if(is_missing)
      Om_push_backUnique(*missings, ModPack->getDependIden(i));
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::prepareInstalls(const OmPModPackArray& selection, OmPModPackArray* installs, OmWStringArray* overlaps, OmWStringArray* depends, OmWStringArray* missings, OmWStringArray* conflicts) const
{
  // Force refresh Mod content
  for(size_t i = 0; i < selection.size(); ++i)
    selection[i]->refreshSource();

  // gather dependencies and create missing lists
  OmPModPackArray found_depends;
  for(size_t i = 0; i < selection.size(); ++i)
    this->_get_modops_depends(selection[i], &found_depends, missings);

  // create the extra install list
  for(size_t i = 0; i < found_depends.size(); ++i) {
    if(!Om_arrayContain(selection, found_depends[i])) {
      // add to installs list
      installs->push_back(found_depends[i]);
      // add to extra depends list
      depends->push_back(found_depends[i]->iden());
    }
  }

  // compose the final install list
  for(size_t i = 0; i < selection.size(); ++i)
    Om_push_backUnique(*installs, selection.at(i));

  // get installation footprint of Mods to be installed
  std::vector<OmModEntryArray> footprintArray;
  OmModEntryArray footprint;

  // get overlaps list against Mods selection (to be installed) on itself
  for(size_t i = 0; i < installs->size(); ++i) {
    // test overlapping against Mods to be installed
    for(size_t j = 0; j < footprintArray.size(); ++j) {
      if(installs->at(i)->canOverlap(footprintArray[j])) {
        overlaps->push_back(installs->at(j)->iden());

        // If channel is in No-Overlapping mode, store conflicting Mods
        if(!this->_backup_overlap) {
          Om_push_backUnique(*conflicts, installs->at(i)->iden());
          Om_push_backUnique(*conflicts, installs->at(j)->iden());
        }
      }
    }

    // create installation footprint of package
    footprint.clear();
    installs->at(i)->getFootprint(&footprint);
    footprintArray.push_back(footprint);
  }

  // get overlaps list against already installed Mods
  for(size_t i = 0; i < installs->size(); ++i) {

    // test overlapping against installed Mods
    for(size_t j = 0; j < this->_modpack_list.size(); ++j) {
      if(this->_modpack_list[j]->hasBackup() || this->_modpack_list[j]->isApplying()) {
        if(installs->at(i)->canOverlap(this->_modpack_list[j])) {
          overlaps->push_back(this->_modpack_list[j]->iden());

          // If channel is in No-Overlapping mode, insert Overlapped Mod in
          // the install list, it will be uninstalled during modops process
          if(!this->_backup_overlap) {
            installs->insert(installs->begin() + i, this->_modpack_list[j]);
            i++;
          }
        }
      }
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_get_backup_relations(const OmModPack* ModPack, OmPModPackArray* relations, OmWStringArray* overlappers, OmWStringArray* dependents) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i) {

    // search only among installed packages and ignore auto-reference
    if(!this->_modpack_list[i]->hasBackup() || this->_modpack_list[i] == ModPack)
      continue;

    // check both if Mod is overlapping and/or depend on
    // the currently specified one
    bool is_overlapper = this->_modpack_list[i]->hasOverlap(ModPack->hash());
    bool is_dependent = this->_modpack_list[i]->hasDepend(ModPack->iden());

    if(is_overlapper || is_dependent) {

      // we go for recursive search to get a properly sorted list of
      // packages in depth-first search order.
      this->_get_backup_relations(this->_modpack_list[i], relations, overlappers, dependents);

      // we now add to the proper lists
      if(is_overlapper)
        Om_push_backUnique(*overlappers, this->_modpack_list[i]->iden());

      if(is_dependent)
        Om_push_backUnique(*dependents, this->_modpack_list[i]->iden());

      // finally add to main list
      Om_push_backUnique(*relations, this->_modpack_list[i]);
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::prepareRestores(const OmPModPackArray& selection, OmPModPackArray* restores, OmWStringArray* overlappers, OmWStringArray* dependents) const
{
  // get overlapping packages list to be uninstalled before selection
  for(size_t i = 0; i < selection.size(); ++i) {

    // this is the only call we do, but the function is doubly recursive and
    // can lead to huge complexity depending the actual state of package installation
    // dependencies and overlapping...
    this->_get_backup_relations(selection[i], restores, overlappers, dependents);

    // add selection
    Om_push_backUnique(*restores, selection[i]);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_get_cleaning_depends(const OmModPack* ModPack, const OmPModPackArray& selection, OmPModPackArray* depends) const
{
  // recursively found all installed (that have backup) dependencies Mods for the
  // specified Mod, then for each, verify if unused and can be uninstalled along
  // the first specified Mod

  for(size_t i = 0; i < this->_modpack_list.size(); ++i) {

    // search only among installed Mods
    if(!this->_modpack_list[i]->hasBackup() || Om_arrayContain(selection, this->_modpack_list[i]))
      continue;

    if(ModPack->hasDepend(this->_modpack_list[i]->iden())) {

      bool is_breaking = false;

      // check whether this dependency Mod can be restored/uninstalled
      // without breaking sibling dependency install
      for(size_t j = 0; j < this->_modpack_list[i]->dependCount(); ++j) {

        // try to find this dependency in Mod Library
        OmModPack* Sibling = this->findModpack(this->_modpack_list[i]->getDependIden(j));

        // If Mod is found, check whether it is installed, meaning we cannot
        // restore this dependency yet
        if(Sibling && Sibling != ModPack && Sibling->hasBackup()) {
          is_breaking = true; break;
        }
      }

      if(!is_breaking) {

        // check recursively, this give depth-first sorted list
        this->_get_cleaning_depends(this->_modpack_list[i], selection, depends);

        // add only if unique
        Om_push_backUnique(*depends, this->_modpack_list[i]);
      }
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_get_cleaning_relations(const OmModPack* ModPack, const OmPModPackArray& selection, OmPModPackArray* relations, OmWStringArray* overlappers) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i) {

    // search only among installed packages and ignore auto-reference
    if(!this->_modpack_list[i]->hasBackup() || Om_arrayContain(selection, this->_modpack_list[i]))
      continue;

    // check both if Mod is overlapping and/or depend on
    // the currently specified one
    bool is_overlapper = this->_modpack_list[i]->hasOverlap(ModPack->hash());

    if(is_overlapper) {

      // we go for recursive search to get a properly sorted list of
      // packages in depth-first search order.
      this->_get_cleaning_relations(this->_modpack_list[i], selection, relations, overlappers);

      // we now add to the proper lists
      if(is_overlapper)
        Om_push_backUnique(*overlappers, this->_modpack_list[i]->iden());

      // finally add to main list
      Om_push_backUnique(*relations, this->_modpack_list[i]);
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::prepareCleaning(const OmPModPackArray& selection, OmPModPackArray* restores, OmWStringArray* depends, OmWStringArray* overlappers, OmWStringArray* dependents) const
{
  // get list of extra dependencies that can be cleaned with selection
  OmPModPackArray found_depends;
  for(size_t i = 0; i < selection.size(); ++i) {

    this->_get_cleaning_depends(selection[i], selection, &found_depends);
  }

  // get overlappers Mods of the found unused dependencies
  for(size_t i = 0; i < found_depends.size(); ++i) {

    // get overlappers packages
    this->_get_cleaning_relations(found_depends[i], selection, restores, overlappers);

    if(!Om_arrayContain(selection, found_depends[i])) {

      // add to restores list
      restores->push_back(found_depends[i]);

      // add to extra depends list
      depends->push_back(found_depends[i]->iden());
    }
  }

  // get overlapping and dependents Mods of the selection
  for(size_t i = 0; i < selection.size(); ++i) {

    // get overlappers and dependents
    this->_get_backup_relations(selection[i], restores, overlappers, dependents);

    // add to restores list
    restores->push_back(selection[i]);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::abortModOps()
{
  if(this->_modops_queue.size())
    this->_modops_abort = true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::queueModOps(const OmPModPackArray& selection, Om_beginCb begin_cb, Om_progressCb progress_cb, Om_resultCb result_cb, Om_notifyCb notify_cb, void* user_ptr)
{
  if(this->_modops_queue.empty()) {

    // another operation is currently processing
    if(this->_locked_mod_library) {

      this->_log(OM_LOG_WRN, L"queueModOps", L"local library is locked by another operation");

      if(result_cb)  // flush all results with abort
        for(size_t i = 0; i << selection.size(); ++i)
          result_cb(user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(selection[i]));

      return;
    }

    this->_modops_begin_cb = begin_cb;
    this->_modops_progress_cb = progress_cb;
    this->_modops_result_cb = result_cb;
    this->_modops_notify_cb = notify_cb;
    this->_modops_user_ptr = user_ptr;

    // reset global progression parameters
    this->_modops_dones = 0;
    this->_modops_percent = 0;

  } else {

    // emit a warning in case a crazy client starts new download with
    // different parameters than current
    if(this->_modops_begin_cb != begin_cb ||
       this->_modops_progress_cb != progress_cb ||
       this->_modops_result_cb != result_cb ||
       this->_modops_notify_cb != notify_cb ||
       this->_modops_user_ptr != user_ptr) {
      this->_log(OM_LOG_WRN, L"queueModOps", L"changing callbacks for a running thread is not allowed");
    }
  }

  // lock the local library to prevent concurrent array manipulation
  this->_locked_mod_library = true;

  // reset abort flag
  this->_modops_abort = false;

  for(size_t i = 0; i < selection.size(); ++i)
    Om_push_backUnique(this->_modops_queue, selection[i]);

  if(!this->_modops_hth) {

    // launch thread
    this->_modops_hth = Om_threadCreate(OmModChan::_modops_run_fn, this);
    this->_modops_hwo = Om_threadWaitEnd(this->_modops_hth, OmModChan::_modops_end_fn, this);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModChan::execModOps(const OmPModPackArray& selection, Om_beginCb begin_cb, Om_progressCb progress_cb, Om_resultCb result_cb, void* user_ptr)
{
  bool invalid_call = false;

  // install queue not empty processing
  if(!this->_modops_queue.empty()) {
    this->_log(OM_LOG_WRN, L"execModOps", L"install queue is not empty");
    invalid_call = true;
  }

  // another operation is currently processing
  if(this->_locked_mod_library) {
    this->_log(OM_LOG_WRN, L"execModOps", L"local library is locked by another operation");
    invalid_call = true;
  }

  if(invalid_call) {
    if(result_cb)  // flush all results with abort
      for(size_t i = 0; i << selection.size(); ++i)
        result_cb(user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(selection[i]));

    return OM_RESULT_ABORT;
  }

  this->_modops_begin_cb = begin_cb;
  this->_modops_progress_cb = progress_cb;
  this->_modops_result_cb = result_cb;
  this->_modops_notify_cb = nullptr;
  this->_modops_user_ptr = user_ptr;

  // reset global progression parameters
  this->_modops_dones = 0;
  this->_modops_percent = 0;

  // lock the local library to prevent concurrent array manipulation
  this->_locked_mod_library = true;

  // reset abort flag
  this->_modops_abort = false;

  for(size_t i = 0; i < selection.size(); ++i)
    Om_push_backUnique(this->_modops_queue, selection[i]);

  // run install process without thread
  OmResult result = static_cast<OmResult>(OmModChan::_modops_run_fn(this));
  OmModChan::_modops_end_fn(this, 0);

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmModChan::_modops_run_fn(void* ptr)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  DWORD exit_code = OM_RESULT_OK;

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_modops_run_fn : enter\n";
  #endif // DEBUG

  while(self->_modops_queue.size()) {

    OmModPack* ModPack = self->_modops_queue.front();

    if(self->_modops_abort) {

      // flush all queue with abort result

      if(self->_modops_result_cb)
        self->_modops_result_cb(self->_modops_user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(ModPack));

      self->_modops_queue.pop_front();

      continue;
    }

    // call client result callback so it can perform proper operations
    if(self->_modops_begin_cb)
      self->_modops_begin_cb(self->_modops_user_ptr, reinterpret_cast<uint64_t>(ModPack));

    OmResult result;

    if(ModPack->hasBackup()) {

      // This is a Restore operation
      result = ModPack->restoreData(OmModChan::_modops_progress_fn, self);

      // refresh Mod Packs analytical parameters
      self->refreshModLibrary();

      // call client result callback so it can perform proper operations
      if(self->_modops_result_cb)
        self->_modops_result_cb(self->_modops_user_ptr, result, reinterpret_cast<uint64_t>(ModPack));

      // check whether mod pack is a ghost to be removed
      self->ghostbusterModPack(ModPack);

      if(result != OM_RESULT_OK) {
        exit_code = result;
        if(result == OM_RESULT_ABORT)
          self->_modops_abort = true;
      }

    } else {

      // This is an Install operation
      result = ModPack->makeBackup(OmModChan::_modops_progress_fn, self);
      if(result == OM_RESULT_OK)
        result = ModPack->applySource(OmModChan::_modops_progress_fn, self);

      // refresh Mod Packs analytical parameters
      self->refreshModLibrary();

      // call client result callback so it can perform proper operations
      if(self->_modops_result_cb)
        self->_modops_result_cb(self->_modops_user_ptr, result, reinterpret_cast<uint64_t>(ModPack));

      if(result != OM_RESULT_OK) {

        exit_code = result;

        // restore any stored Backup data
        ModPack->restoreData(OmModChan::_modops_progress_fn, self, true);

        // reset progression status
        if(self->_modops_progress_cb)
          self->_modops_progress_cb(self->_modops_user_ptr, 0, 0, reinterpret_cast<uint64_t>(ModPack));

        if(result == OM_RESULT_ABORT)
          self->_modops_abort = true;
      }

    }

    self->_modops_dones++;
    self->_modops_queue.pop_front();
  }

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_modops_run_fn : leave\n";
  #endif // DEBUG

  return exit_code;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_modops_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  // compute global queue progress percentage
  double queue_percents = self->_modops_dones * 100;
  for(size_t i = 0; i < self->_modops_queue.size(); ++i)
    queue_percents += self->_modops_queue[i]->operationProgress();

  self->_modops_percent = queue_percents / (self->_modops_dones + self->_modops_queue.size());

  if(self->_modops_progress_cb)
    if(!self->_modops_progress_cb(self->_modops_user_ptr, tot, cur, param))
      self->abortModOps();

  return !self->_modops_abort;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmModChan::_modops_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  OmModChan* self = static_cast<OmModChan*>(ptr);

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_modops_end_fn\n";
  #endif // DEBUG

  // clean potential ghost packages, that may appear after restores
  self->ghostbusterModLibrary();

  // unlock the local library
  self->_locked_mod_library = false;

  //DWORD exit_code = Om_threadExitCode(self->_modops_hth);
  Om_threadClear(self->_modops_hth, self->_modops_hwo);

  // call notify callback
  if(self->_modops_notify_cb)
    self->_modops_notify_cb(self->_modops_user_ptr, OM_NOTIFY_ENDED, reinterpret_cast<uint64_t>(self));

  self->_modops_dones = 0;
  self->_modops_percent = 0;

  self->_modops_hth = nullptr;
  self->_modops_hwo = nullptr;

  self->_modops_user_ptr = nullptr;
  self->_modops_begin_cb = nullptr;
  self->_modops_progress_cb = nullptr;
  self->_modops_result_cb = nullptr;
  self->_modops_notify_cb = nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::discardBackups(const OmPModPackArray& selection, Om_progressCb progress_cb, void* user_ptr)
{
  bool has_error = false;

  for(size_t i = 0; i < selection.size(); ++i) {

    OmModPack* ModPack = selection[i];

    if(ModPack->hasBackup()) {
      if(ModPack->discardBackup() == OM_RESULT_ERROR)
        has_error = true;
    }

    if(progress_cb)
      progress_cb(user_ptr, selection.size(), i, reinterpret_cast<uint64_t>(selection[i]));
  }

  // refresh Mod Packs analytical parameters
  this->refreshModLibrary();

  return !has_error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::hasBackupData()
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i) {
    if(this->_modpack_list[i]->hasBackup())
      return true;
  }
  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::clearNetLibrary()
{
  for(size_t i = 0; i < this->_netpack_list.size(); ++i)
    delete this->_netpack_list[i];

  this->_netpack_list.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::refreshNetLibrary()
{
  bool has_change = false;

  for(size_t i = 0; i < this->_netpack_list.size(); ++i) {

    // refresh Net Pack status
    if(this->_netpack_list[i]->refreshAnalytics()) {

      // notify changes
      if(this->_netpack_notify_cb)
        this->_netpack_notify_cb(this->_netpack_notify_ptr, OM_NOTIFY_ALTERED, this->_netpack_list[i]->hash());

      has_change = true;
    }
  }

  #ifdef DEBUG
  std::cout << "DEBUG => OmModChan::refreshNetLibrary " << (has_change ? "~=" : "==") << "\n";
  #endif

  return has_change;
}
///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmNetPack* OmModChan::findNetpack(uint64_t hash) const
{
  for(size_t i = 0; i < this->_netpack_list.size(); ++i)
    if(this->_netpack_list[i]->hash() == hash)
      return this->_netpack_list[i];

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmNetPack* OmModChan::findNetpack(const OmWString& iden) const
{
  for(size_t i = 0; i < this->_netpack_list.size(); ++i)
    if(this->_netpack_list[i]->iden() == iden)
      return this->_netpack_list[i];

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmModChan::indexOfNetpack(const OmNetPack* NetPack) const
{
  for(size_t i = 0; i < this->_netpack_list.size(); ++i)
    if(this->_netpack_list[i] == NetPack)
      return i;

  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_get_missing_depends(const OmModPack* ModPack, OmWStringArray* missings) const
{
  for(size_t i = 0; i < ModPack->dependCount(); ++i) {

    bool is_missing = true;
/*
    for(size_t m = 0; m < this->_modpack_list.size(); ++m) {

      // ignore directory Mods
      if(this->_modpack_list[m]->sourceIsDir())
        continue;

      if(ModPack->getDependIden(i) == this->_modpack_list[m]->iden()) {

        this->_get_missing_depends(this->_modpack_list[m], missings);

        is_missing = false;
        break;
      }
    }
*/

    OmModPack* DepMod = this->findModDepend(ModPack->getDependIden(i));

    if(DepMod && !DepMod->sourceIsDir()) {

      this->_get_missing_depends(DepMod, missings);

      is_missing = false;

      break;
    }

    if(is_missing)
      Om_push_backUnique(*missings, ModPack->getDependIden(i));
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmNetPack* OmModChan::findNetDepend(const OmWString& filter) const
{
  // parsed identity filter
  OmWString core, vers;

  if(Om_parseModIdent(filter, &core, &vers, nullptr)) {

    // gather candidates
    OmPNetPackArray candidates;

    for(size_t i = 0; i < this->_netpack_list.size(); ++i) {

      if(this->_netpack_list[i]->core() != core)
        continue;

      if(!this->_netpack_list[i]->version().match(vers))
        continue;

      candidates.push_back(this->_netpack_list[i]);
    }

    if(candidates.size()) {

      // more than one candidate, sort by version
      if(candidates.size() > 1)
        std::sort(candidates.begin(), candidates.end(), _compare_net_vers);

      return candidates[candidates.size()-1];
    }

  } else {

    for(size_t i = 0; i < this->_netpack_list.size(); ++i) {

      if(this->_netpack_list[i]->iden() == core)
        return this->_netpack_list[i];
    }
  }

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_get_source_downloads(const OmModPack* ModPack, OmPNetPackArray* downloads, OmWStringArray* missings) const
{
  // get all missing dependencies for this package
  OmWStringArray missing_depends;

  this->_get_missing_depends(ModPack, &missing_depends);

  if(missing_depends.empty())
    return;

  // try to found packages in the network library list
  for(size_t i = 0; i < missing_depends.size(); ++i) {

    bool is_missing = true;

    /*
    for(size_t n = 0; n < this->_netpack_list.size(); ++n) {

      if(missing_depends.at(i) == this->_netpack_list[n]->iden()) {

        // add Mod dependencies
        this->getDepends(this->_netpack_list[n], downloads, missings);

        // add Mod itslef
        Om_push_backUnique(*downloads, this->_netpack_list[n]);

        is_missing = false;
      }
    }
    */

    OmNetPack* DepNet = this->findNetDepend(missing_depends.at(i));

    if(DepNet) {

      // add Mod dependencies
      this->getDepends(DepNet, downloads, missings);

      // add Mod itslef
      Om_push_backUnique(*downloads, DepNet);

      is_missing = false;
    }

    if(is_missing)
      Om_push_backUnique(*missings, missing_depends.at(i));
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::getDepends(const OmNetPack* NetPack, OmPNetPackArray* depends, OmWStringArray* missings) const
{
  // first check required dependency in local library to
  // gather all missing dependencies, this will add dependencies
  // to found in the remote package list
  for(size_t i = 0; i < NetPack->dependCount(); ++i) {

    bool in_library = false;

    // first check whether required dependency is in package library
    /*
    for(size_t m = 0; m < this->_modpack_list.size(); ++m) {

      // ignore directories
      if(this->_modpack_list[m]->sourceIsDir())
        continue;

      // if we found package, we must verify its dependencies list
      if(NetPack->getDependIden(i) == this->_modpack_list[m]->iden()) {

        // get all available dependencies for this Mod
        this->_get_source_downloads(this->_modpack_list[m], depends, missings);

        in_library = true;

        break;
      }
    }
    */

    OmModPack* DepMod = this->findModDepend(NetPack->getDependIden(i));

    if(DepMod && !DepMod->sourceIsDir()) {

      // get all available dependencies for this Mod
      this->_get_source_downloads(DepMod, depends, missings);

      in_library = true;
    }

    if(in_library) //< skip if already in library
      continue;

    bool is_missing = true;

    /*
    for(size_t n = 0; n < this->_netpack_list.size(); ++n) {

      if(NetPack->getDependIden(i) == this->_netpack_list[n]->iden()) {

        // add Mod dependencies
        this->getDepends(this->_netpack_list[n], depends, missings);

        // add Mod itslef
        Om_push_backUnique(*depends, this->_netpack_list[n]);

        is_missing = false;
        break;
      }
    }
    */

    OmNetPack* DepNet = this->findNetDepend(NetPack->getDependIden(i));

    if(DepNet) {

      // add Mod dependencies
      this->getDepends(DepNet, depends, missings);

      is_missing = false;
    }

    if(is_missing)
      Om_push_backUnique(*missings, NetPack->getDependIden(i));
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_get_replace_breaking(const OmNetPack* NetPack, OmWStringArray* breaking) const
{
  for(size_t i = 0; i < NetPack->upgradableCount(); ++i) {
    if(this->isDependency(NetPack->getUpgradable(i)))
      Om_push_backUnique(*breaking, NetPack->getUpgradable(i)->iden());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::prepareDownloads(const OmPNetPackArray& selection, OmPNetPackArray* downloads, OmWStringArray* depends, OmWStringArray* missings, OmWStringArray* breaking) const
{
  // gather dependencies and create missing lists
  OmPNetPackArray found_depends;
  for(size_t i = 0; i < selection.size(); ++i)
    this->getDepends(selection[i], &found_depends, missings);

  // add extra dependencies to be downloaded
  for(size_t i = 0; i < found_depends.size(); ++i) {

    // check for required old packages that this one may supersedes
    this->_get_replace_breaking(found_depends[i], breaking);

    // add to final download list
    Om_push_backUnique(*downloads, selection, found_depends[i]);

    // add extra depends identity to list
    Om_push_backUnique(*depends, found_depends[i]->iden());
  }

  // compose the final download list
  for(size_t i = 0; i < selection.size(); ++i) {

    // add only if not already in install list
    if(std::find(downloads->begin(), downloads->end(), selection[i]) == downloads->end()) {

      // check for required old packages that this one may supersedes
      this->_get_replace_breaking(selection[i], breaking);

      downloads->push_back(selection[i]);
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::startDownloads(const OmPNetPackArray& selection, Om_beginCb begin_cb, Om_downloadCb download_cb, Om_resultCb result_cb, Om_notifyCb notify_cb, void* user_ptr)
{
  if(this->_download_array.empty()) {

    // another operation is currently processing
    if(this->_locked_net_library) {

      this->_log(OM_LOG_WRN, L"startDownloads", L"network library is locked by another operation");

      if(result_cb) // flush all with abort result
        for(size_t i = 0; i < selection.size(); ++i)
          result_cb(user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(selection[i]));

      return;
    }

    this->_download_user_ptr = user_ptr;
    this->_download_begin_cb = begin_cb;
    this->_download_download_cb = download_cb;
    this->_download_result_cb = result_cb;
    this->_download_notify_cb = notify_cb;

    // reset global progression
    this->_download_dones = 0;
    this->_download_percent = 0;

  } else {

    // emit a warning in case a crazy client starts new download with
    // different parameters than current
    if(this->_download_download_cb != download_cb ||
       this->_download_begin_cb != begin_cb ||
       this->_download_result_cb != result_cb ||
       this->_download_notify_cb != notify_cb ||
       this->_download_user_ptr != user_ptr) {
      this->_log(OM_LOG_WRN, L"startDownloads", L"subsequent downloads with different parameters");
    }
  }

  this->_locked_net_library = true;

  this->_download_abort = false;

  // add to queue
  for(size_t i = 0; i < selection.size(); ++i)
    Om_push_backUnique(this->_download_queue, selection[i]);

  // starts queued downloads (according current limits)
  this->_download_srart_queued();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::stopDownloads()
{
  // flush download queue
  while(this->_download_queue.size()) {

    OmNetPack* NetPack = this->_download_queue.front();

    if(this->_download_result_cb) // call result callback with error
      this->_download_result_cb(this->_download_user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(NetPack));

    this->_download_queue.pop_front();
  }

  // start sequential stops of running downloads
  if(this->_download_array.size()) {
    // we abort all processing downloads
    this->_download_abort = true;

    // if abort request was fired, we must stop downloads sequentially to
    // prevent callback concurrent calls that mess up all process, so we
    // only stop the last started download, they will be aborted in cascade
    // through the result callback
    this->_download_array.back()->stopDownload();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::stopDownload(size_t index)
{
  this->_netpack_list[index]->stopDownload();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_download_srart_queued()
{
  // prevent simultaneous startings
  if(!this->_download_start_hth) {
    this->_download_start_hth = Om_threadCreate(OmModChan::_download_start_run_fn, this);
    this->_download_start_hwo = Om_threadWaitEnd(this->_download_start_hth, OmModChan::_download_start_end_fn, this);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmModChan::_download_start_run_fn(void* ptr)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  while(self->_download_queue.size()) {

    if(self->_down_max_thread > 0) {
      if(self->_download_array.size() >= self->_down_max_thread) {
        Sleep(250);
        continue;
      }
    }

    OmNetPack* NetPack = self->_download_queue.front();

    // add download to stack
    Om_push_backUnique(self->_download_array, NetPack);

    if(self->_download_begin_cb)
      self->_download_begin_cb(self->_download_user_ptr, reinterpret_cast<uint64_t>(NetPack));

    // start download
    if(!NetPack->startDownload(OmModChan::_download_download_fn, OmModChan::_download_result_fn, self, self->_down_max_rate)) {

      if(self->_download_result_cb) // call result callback with error
        self->_download_result_cb(self->_download_user_ptr, OM_RESULT_ERROR, reinterpret_cast<uint64_t>(NetPack));
    }

    self->_download_queue.pop_front();

    // wait a bit to prevent flood
    Sleep(250);
  }

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmModChan::_download_start_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  OmModChan* self = static_cast<OmModChan*>(ptr);

  // unlock the local library
  self->_locked_mod_library = false;

  Om_threadClear(self->_download_start_hth, self->_download_start_hwo);

  self->_download_start_hth = nullptr;
  self->_download_start_hwo = nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_download_download_fn(void* ptr, int64_t tot, int64_t cur, int64_t rate, uint64_t param)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  // update global progress
  double queue_percents = self->_download_dones * 100;
  for(size_t i = 0; i < self->_download_array.size(); ++i)
    queue_percents += self->_download_array[i]->downloadProgress();

  self->_download_percent = queue_percents / (self->_download_dones + self->_download_array.size() + self->_download_queue.size());

  if(self->_download_download_cb)
    if(!self->_download_download_cb(self->_download_user_ptr, tot, cur, rate, param))
      self->stopDownloads();

  return !self->_download_abort;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_download_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  // the final result, including download close operation
  OmResult final_result;

  // finalize the download, this verify checksum and rename
  // temporary partial download file to regular file name
  if(result == OM_RESULT_OK) {

    if(NetPack->finalizeDownload()) {
      final_result = OM_RESULT_OK;
    } else {
      final_result = OM_RESULT_ERROR;
    }

  } else {

    // this is probably an abort
    final_result = result;
  }

  // update status and send propers notifications
  self->refreshNetLibrary();

  // remove download from stack
  Om_eraseValue(self->_download_array, NetPack);

  // increase download done count
  self->_download_dones++;

  // call client callback
  if(self->_download_result_cb)
    self->_download_result_cb(self->_download_user_ptr, final_result, param);

  if(self->_download_array.size()) {

    // if abort request was fired, we must stop downloads sequentially to
    // prevent callback concurrent calls that mess up all process
    if(self->_download_abort) {
      self->_download_array.back()->stopDownload();
    }

  } else {

    self->_locked_net_library = false;

    // call notify callback
    if(self->_download_notify_cb)
      self->_download_notify_cb(self->_download_user_ptr, OM_NOTIFY_ENDED, reinterpret_cast<uint64_t>(self));

    self->_download_dones = 0;
    self->_download_percent = 0;

    self->_download_user_ptr = nullptr;
    self->_download_begin_cb = nullptr;
    self->_download_download_cb = nullptr;
    self->_download_result_cb = nullptr;
    self->_download_notify_cb = nullptr;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::queueSupersede(const OmPNetPackArray& selection, Om_beginCb begin_cb, Om_progressCb progress_cb, Om_resultCb result_cb, Om_notifyCb notify_cb, void* user_ptr)
{
  if(this->_supersed_queue.empty()) {

    // another operation is currently processing
    if(this->_locked_mod_library) {

      this->_log(OM_LOG_WRN, L"queueSupersede", L"local library is locked by another operation");

      if(result_cb)  // flush all results with abort
        for(size_t i = 0; i << selection.size(); ++i)
          result_cb(user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(selection[i]));

      return;
    }

    this->_supersed_begin_cb = begin_cb;
    this->_supersed_progress_cb = progress_cb;
    this->_supersed_result_cb = result_cb;
    this->_supersed_notify_cb = notify_cb;
    this->_supersed_user_ptr = user_ptr;

    // reset global progress
    this->_supersed_dones = 0;
    this->_supersed_percent = 0;

  } else {

    // emit a warning in case a crazy client starts new download with
    // different parameters than current
    if(this->_supersed_begin_cb != begin_cb ||
       this->_supersed_progress_cb != progress_cb ||
       this->_supersed_result_cb != result_cb ||
       this->_supersed_notify_cb != notify_cb ||
       this->_supersed_user_ptr != user_ptr) {
      this->_log(OM_LOG_WRN, L"queueSupersede", L"subsequent supersede with different parameters");
    }
  }

  // lock the local library to prevent concurrent array manipulation
  this->_locked_mod_library = true;

  // reset abort flag
  this->_supersed_abort = false;

  for(size_t i = 0; i < selection.size(); ++i)
    Om_push_backUnique(this->_supersed_queue, selection[i]);

  if(!this->_supersed_hth) {

    // launch thread
    this->_supersed_hth = Om_threadCreate(OmModChan::_supersed_run_fn, this);
    this->_supersed_hwo = Om_threadWaitEnd(this->_supersed_hth, OmModChan::_supersed_end_fn, this);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::abortSupersede()
{
  if(this->_supersed_queue.size())
    this->_supersed_abort = true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmModChan::_supersed_run_fn(void* ptr)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  DWORD exit_code = 0;

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_supersed_run_fn : enter\n";
  #endif // DEBUG

  while(self->_supersed_queue.size()) {

    OmNetPack* NetPack = self->_supersed_queue.front();

    if(self->_supersed_abort) {

      // flush all queue with abort

      if(self->_supersed_result_cb)
        self->_supersed_result_cb(self->_supersed_user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(NetPack));

      self->_supersed_queue.pop_front();

      continue;
    }

    // call client begin callback so it can perform proper operations
    if(self->_supersed_begin_cb)
      self->_supersed_begin_cb(self->_supersed_user_ptr, reinterpret_cast<uint64_t>(NetPack));

    OmResult result = NetPack->supersede(OmModChan::_supersed_progress_fn, self);

    // update status and send propers notifications
    self->refreshNetLibrary();

    if(result == OM_RESULT_ERROR)
      exit_code = 1;

    // call client result callback so it can perform proper operations
    if(self->_supersed_result_cb)
      self->_supersed_result_cb(self->_supersed_user_ptr, result, reinterpret_cast<uint64_t>(NetPack));

    self->_supersed_dones++;

    self->_supersed_queue.pop_front();
  }

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_supersed_run_fn : leave\n";
  #endif // DEBUG

  return exit_code;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_supersed_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  // update global progress
  double queue_percents = self->_supersed_dones * 100;
  for(size_t i = 0; i < self->_supersed_queue.size(); ++i)
    queue_percents += self->_supersed_queue[i]->supersedeProgress();

  self->_supersed_percent = queue_percents / (self->_supersed_dones + self->_supersed_queue.size());

  if(self->_supersed_progress_cb)
    if(!self->_supersed_progress_cb(self->_supersed_user_ptr, tot, cur, param))
      self->abortSupersede();

  return !self->_supersed_abort;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmModChan::_supersed_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_supersed_end_fn\n";
  #endif // DEBUG

  OmModChan* self = static_cast<OmModChan*>(ptr);

  // unlock the local library
  self->_locked_mod_library = false;

  //DWORD exit_code = Om_threadExitCode(self->_supersed_hth);
  Om_threadClear(self->_supersed_hth, self->_supersed_hwo);

  self->_supersed_hth = nullptr;
  self->_supersed_hwo = nullptr;

  // call notify callback
  if(self->_supersed_notify_cb)
    self->_supersed_notify_cb(self->_supersed_user_ptr, OM_NOTIFY_ENDED, reinterpret_cast<uint64_t>(self));

  self->_supersed_dones = 0;
  self->_supersed_percent = 0;

  self->_supersed_user_ptr = nullptr;
  self->_supersed_progress_cb = nullptr;
  self->_supersed_result_cb = nullptr;
  self->_supersed_notify_cb = nullptr;
  self->_supersed_begin_cb = nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_compare_net_name(const OmNetPack* a, const OmNetPack* b)
{
  // test against the shorter string
  size_t l = a->iden().size() < b->iden().size() ? b->iden().size() : a->iden().size();

  const wchar_t* a_srt = a->iden().c_str();
  const wchar_t* b_str = b->iden().c_str();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {

    wchar_t a_chr = towupper(a_srt[i]);
    wchar_t b_chr = towupper(b_str[i]);

    if(a_chr != b_chr) {
      if(a_chr < b_chr) {
        return true;
      } else {
        return false;
      }
    }
  }

  // strings are equals in tester portion, sort by string size
  if(a->iden().size() != b->iden().size())
    if(a->iden().size() < b->iden().size())
      return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_compare_net_stat(const OmNetPack* a, const OmNetPack* b)
{
  uint16_t a_score = 0;
  uint16_t b_score = 0;

  // select proper status icon
  if(a->hasError()) {
    a_score = 1;
  } else if(a->hasLocal()) {
    if(a->isSuperseding()) {
      a_score = 1;
    } else if(a->hasMissingDepend()) {
      a_score = 7;
    } else {
      a_score = 8;
    }
  } else if(a->isDownloading()) {
    a_score = 6;
  } else {
    if(a->isResumable()) {
      a_score = 5;
    } else if(a->upgradableCount()) {
      a_score = 3;
    } else if(a->downgradableCount()) {
      a_score = 2;
    } else {
      a_score = 4; // new
    }
  }

  // select proper status icon
  if(b->hasError()) {
    b_score = 1;
  } else if(b->hasLocal()) {
    if(b->isSuperseding()) {
      b_score = 1;
    } else if(b->hasMissingDepend()) {
      b_score = 7;
    } else {
      b_score = 8;
    }
  } else if(b->isDownloading()) {
    b_score = 6;
  } else {
    if(b->isResumable()) {
      b_score = 5;
    } else if(b->upgradableCount()) {
      b_score = 3;
    } else if(b->downgradableCount()) {
      b_score = 2;
    } else {
      b_score = 4; // new
    }
  }

  if(a_score == b_score) {
    return OmModChan::_compare_net_name(a, b);
  } else {
    return (a_score < b_score);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_compare_net_vers(const OmNetPack* a, const OmNetPack* b)
{
  if(a->version() == b->version()) {
    return OmModChan::_compare_net_name(a, b);
  } else {
    return (a->version() < b->version());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_compare_net_cate(const OmNetPack* a, const OmNetPack* b)
{
  // test against the shorter string
  size_t l = a->category().size() < b->category().size() ? b->category().size() : a->category().size();

  const wchar_t* a_srt = a->category().c_str();
  const wchar_t* b_str = b->category().c_str();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {

    wchar_t a_chr = towupper(a_srt[i]);
    wchar_t b_chr = towupper(b_str[i]);

    if(a_chr != b_chr) {
      if(a_chr < b_chr) {
        return true;
      } else {
        return false;
      }
    }
  }

  // strings are equals in tester portion, sort by string size
  if(a->category().size() != b->category().size())
    if(a->category().size() < b->category().size())
      return true;

  // strings are strictly equals, we sort by name
  return OmModChan::_compare_net_name(a, b);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_compare_net_size(const OmNetPack* a, const OmNetPack* b)
{
  if(a->fileSize() == b->fileSize()) {
    return OmModChan::_compare_net_name(a, b);
  } else {
    return (a->fileSize() < b->fileSize());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::sortNetLibrary()
{
  bool(*compare_func)(const OmNetPack*,const OmNetPack*) = nullptr;

  if(OM_HAS_BIT(this->_netpack_list_sort,OM_SORT_STAT)) compare_func = OmModChan::_compare_net_stat;
  if(OM_HAS_BIT(this->_netpack_list_sort,OM_SORT_NAME)) compare_func = OmModChan::_compare_net_name;
  if(OM_HAS_BIT(this->_netpack_list_sort,OM_SORT_VERS)) compare_func = OmModChan::_compare_net_vers;
  if(OM_HAS_BIT(this->_netpack_list_sort,OM_SORT_CATE)) compare_func = OmModChan::_compare_net_cate;
  if(OM_HAS_BIT(this->_netpack_list_sort,OM_SORT_SIZE)) compare_func = OmModChan::_compare_net_size;

  if(compare_func)
    std::sort(this->_netpack_list.begin(), this->_netpack_list.end(), compare_func);

  // check whether we need a normal or reverse sorting
  if(OM_HAS_BIT(this->_netpack_list_sort,OM_SORT_INVT)) {
    std::reverse(this->_netpack_list.begin(), this->_netpack_list.end());
  }

  if(this->_netpack_notify_cb)
    this->_netpack_notify_cb(this->_netpack_notify_ptr, OM_NOTIFY_REBUILD, 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setNetLibrarySort(OmSort sorting)
{
  if(!this->_xml.valid())
    return;

  // if requested sorting is same as current, reverse order

  if(OM_HAS_BIT(this->_netpack_list_sort, sorting)) {

    OM_TOG_BIT(this->_netpack_list_sort, OM_SORT_INVT);

  } else {

    this->_netpack_list_sort = sorting;
  }

  // save the current sorting
  if(this->_xml.hasChild(L"remotes_sort")) {
    this->_xml.child(L"remotes_sort").setAttr(L"sort", this->_netpack_list_sort);
  } else {
    this->_xml.addChild(L"remotes_sort").setAttr(L"sort", this->_netpack_list_sort);
  }

  this->_xml.save();

  this->sortNetLibrary(); //< this will send rebuild notification
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmModChan::indexOfRepository(OmNetRepo* NetRepo) const
{
  for(size_t i = 0; i < this->_repository_list.size(); ++i)
    if(this->_repository_list[i] == NetRepo)
      return i;

  return -1;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::addRepository(const OmWString& base, const OmWString& name)
{
  if(!this->_xml.valid())
    return false;

  // get or create <network> node where repositories are listed
  OmXmlNode network_node;

  if(!this->_xml.hasChild(L"network")) {
    network_node = this->_xml.addChild(L"network");
  } else {
    network_node = this->_xml.child(L"network");
  }

  // check whether repository already exists
  OmXmlNodeArray repository_nodes;
  network_node.children(repository_nodes, L"repository");

  for(size_t i = 0; i < repository_nodes.size(); ++i) {
    if(base == repository_nodes[i].attrAsString(L"base")) {
      if(name == repository_nodes[i].attrAsString(L"name")) {
        this->_error(L"addRepository", L"Repository with same parameters already exists");
        return false;
      }
    }
  }

  // add repository entry in definition
  OmXmlNode repository_node = network_node.addChild(L"repository");

  repository_node.setAttr(L"base", base);
  repository_node.setAttr(L"name", name);

  // Save configuration
  this->_xml.save();

  // add repository in local list
  OmNetRepo* ModRepo = new OmNetRepo(this);

  // set repository parameters
  if(!ModRepo->setCoordinates(base, name)) {
    this->_error(L"addRepository", ModRepo->lastError());
    delete ModRepo; return false;
  }

  // add to list
  this->_repository_list.push_back(ModRepo);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::removeRepository(size_t index)
{
  if(index >= this->_repository_list.size())
    return;

  // Remove repository entry from configuration
  if(!this->_xml.valid())
    return;

  OmNetRepo* NetRepo = this->_repository_list[index];

  // get <network> node

  if(this->_xml.hasChild(L"network")) {

    OmXmlNodeArray repository_nodes;
    this->_xml.child(L"network").children(repository_nodes, L"repository");

    for(size_t i = 0; i < repository_nodes.size(); ++i) {
      if(NetRepo->base() == repository_nodes[i].attrAsString(L"base")) {
        if(NetRepo->name() == repository_nodes[i].attrAsString(L"name")) {
          this->_xml.child(L"network").remChild(repository_nodes[i]);
          break;
        }
      }
    }
  }

  // save configuration
  this->_xml.save();

  // remove all Remote packages related to this Repository
  size_t i = this->_netpack_list.size();
  while(i--) {
    if(this->_netpack_list[i]->NetRepo() == NetRepo) {

      // notify delete
      if(this->_netpack_notify_cb)
        this->_netpack_notify_cb(this->_netpack_notify_ptr, OM_NOTIFY_DELETED, this->_netpack_list[i]->hash());

      delete this->_netpack_list[i];

      this->_netpack_list.erase(this->_netpack_list.begin() + i);
    }
  }

  // delete object and remove it from local list
  delete NetRepo;
  this->_repository_list.erase(this->_repository_list.begin() + index);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::abortQueries()
{
  if(this->_query_queue.size()) {

    this->_query_abort = true;

    for(size_t i = 0; i < this->_repository_list.size(); ++i) {
      this->_repository_list[i]->abortQuery();
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::queueQueries(const OmPNetRepoArray& selection, Om_beginCb begin_cb, Om_resultCb result_cb, Om_notifyCb notify_cb, void* user_ptr)
{
  if(this->_query_queue.empty()) {

    // another operation is currently processing
    if(this->_locked_net_library) {

      this->_log(OM_LOG_WRN, L"queueQueries", L"network library is locked by another operation");

      if(result_cb) // flush all with abort result
        for(size_t i = 0; i < selection.size(); ++i)
          result_cb(user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(selection[i]));

      return;
    }

    this->_query_user_ptr = user_ptr;
    this->_query_begin_cb = begin_cb;
    this->_query_result_cb = result_cb;
    this->_query_notify_cb = notify_cb;

    // reset global progress
    this->_query_dones = 0;
    this->_query_percent = 0;

  } else {
    // emit a warning in case a crazy client starts new download with
    // different parameters than current
    if(this->_query_begin_cb != begin_cb ||
       this->_query_result_cb != result_cb ||
       this->_query_notify_cb != notify_cb ||
       this->_query_user_ptr != user_ptr ) {
      this->_log(OM_LOG_WRN, L"queueQueries", L"subsequent query with different parameters");
    }
  }

  // lock the network library to prevent concurrent array manipulation
  this->_locked_net_library = true;

  for(size_t i = 0; i < selection.size(); ++i)
    this->_query_queue.push_back(selection[i]);

  // reset abort stat
  this->_query_abort = false;

  // start new thread
  if(!this->_query_hth) {

    // launch thread
    this->_query_hth = Om_threadCreate(OmModChan::_query_run_fn, this);
    this->_query_hwo = Om_threadWaitEnd(this->_query_hth, OmModChan::_query_end_fn, this);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmModChan::_query_run_fn(void* ptr)
{
  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_query_run : enter\n";
  #endif // DEBUG

  OmModChan* self = static_cast<OmModChan*>(ptr);

  DWORD exit_code = 0;

  while(self->_query_queue.size()) {

    OmNetRepo* NetRepo = self->_query_queue.front();

    // FIXME: Je ne pense que cette partie est inutile, le verifier
    if(self->_query_abort) {

      // update queue progress before sending result
      self->_query_dones = 0; self->_query_percent = 0;

      // flush all queue with abort result

      if(self->_query_result_cb)
        self->_query_result_cb(self->_query_user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(NetRepo));

      self->_query_queue.pop_front();

      continue;
    }

    if(self->_query_begin_cb)
      self->_query_begin_cb(self->_query_user_ptr, reinterpret_cast<uint64_t>(NetRepo));

    OmResult result = NetRepo->query();

    if(result == OM_RESULT_OK) {

      // update repository title if possible
      if(!NetRepo->title().empty()) {

        OmXmlNodeArray repository_nodes;
        self->_xml.child(L"network").children(repository_nodes, L"repository");

        for(size_t i = 0; i < repository_nodes.size(); ++i) {
          if(repository_nodes[i].attrAsString(L"base") == NetRepo->base()) {
            if(repository_nodes[i].attrAsString(L"name") == NetRepo->name()) {
              repository_nodes[i].setAttr(L"title", NetRepo->title()); break;
            }
          }
        }

        self->_xml.save();
      }

      // Add or Merge Repository referenced Mods to list

      // 1. remove / clear reference that previously belong this Repository
      size_t net_size = self->_netpack_list.size();
      while(net_size--) {
        if(self->_netpack_list[net_size]->NetRepo() == NetRepo) {
          delete self->_netpack_list[net_size];
          self->_netpack_list.erase(self->_netpack_list.begin() + net_size);
        }
      }

      // 2. parse and add referenced Mods in lists
      for(size_t r = 0; r < NetRepo->referenceCount(); ++r) {

        OmNetPack* NetPack = new OmNetPack(self);

        if(NetPack->parseReference(NetRepo, r)) {

          // we want to be sure Net Pack is unique in list
          bool is_unique = true;

          for(size_t j = 0; j < self->_netpack_list.size(); ++j) {

            if(self->_netpack_list[j]->iden() == NetPack->iden()) {
              delete self->_netpack_list[j]; //< remove previous
              self->_netpack_list[j] = NetPack; //< replace object
              is_unique = false; break;
            }
          }

          if(is_unique)
            self->_netpack_list.push_back(NetPack);

        } else {

          self->_log(OM_LOG_WRN, L"queryNetRepository", NetPack->lastError());
          delete NetPack;
        }
      }

      self->sortNetLibrary(); //< this will send rebuild notification

      self->refreshNetLibrary();

    }

    // update queue progress before sending result
    self->_query_dones++;
    self->_query_percent = static_cast<double>(self->_query_dones * 100) / (self->_query_dones + self->_query_queue.size());

    if(self->_query_result_cb)
      self->_query_result_cb(self->_query_user_ptr, result, reinterpret_cast<uint64_t>(NetRepo));

    self->_query_queue.pop_front();
  }

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_query_run : leave\n";
  #endif // DEBUG

  return exit_code;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmModChan::_query_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  OmModChan* self = static_cast<OmModChan*>(ptr);

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_query_end\n";
  #endif // DEBUG

  // unlock the network library
  self->_locked_net_library = false;

  //DWORD exit_code = Om_threadExitCode(self->_query_hth);
  Om_threadClear(self->_query_hth, self->_query_hwo);

  self->_query_hth = nullptr;
  self->_query_hwo = nullptr;

  self->_query_queue.clear();

  // call notify callback
  if(self->_query_notify_cb)
    self->_query_notify_cb(self->_query_user_ptr, OM_NOTIFY_ENDED, reinterpret_cast<uint64_t>(self));

  self->_query_dones = 0;
  self->_query_percent = 0;

  self->_query_begin_cb = nullptr;
  self->_query_result_cb = nullptr;
  self->_query_notify_cb = nullptr;
  self->_query_user_ptr = nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setTitle(const OmWString& title)
{
  if(!this->_xml.valid())
    return;

  this->_title = title;

  if(this->_xml.hasChild(L"title")) {
    this->_xml.child(L"title").setContent(title);
  } else {
    this->_xml.addChild(L"title").setContent(title);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setIndex(unsigned index)
{
  if(!this->_xml.valid())
    return;

  this->_index = index;

  if(this->_xml.hasChild(L"title")) {
    this->_xml.child(L"title").setAttr(L"index", static_cast<int>(index));
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModChan::setTargetPath(const OmWString& path)
{
  if(this->_locked_mod_library) {
    this->_error(L"setTargetPath", L"Mod library operation is currently processing");
    return OM_RESULT_ABORT;
  }

  if(!this->_xml.valid()) {
    this->_error(L"setTargetPath", L"channel is not valid");
    return OM_RESULT_ABORT;
  }

  if(this->hasBackupData()) {
    this->_error(L"setTargetPath", L"one or more Mods are currently installed");
    return OM_RESULT_ABORT;
  }

  if(!Om_isDir(path)) {
    this->_error(L"setTargetPath", Om_errNotDir(L"directory", path));
    return OM_RESULT_ERROR;
  }

  this->_target_path = path;

  if(this->_xml.hasChild(L"install")) {
    this->_xml.child(L"install").setContent(path);
  } else {
    this->_xml.addChild(L"install").setContent(path);
  }

  this->_xml.save();

  return OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModChan::setCustLibraryPath(const OmWString& path)
{
  if(this->_locked_mod_library) {
    this->_error(L"setCustLibraryPath", L"Mod library operation is currently processing");
    return OM_RESULT_ABORT;
  }

  if(!this->_xml.valid()) {
    this->_error(L"setCustLibraryPath", L"channel is not valid");
    return OM_RESULT_ABORT;
  }

  if(!Om_isDir(path)) {
    this->_error(L"setCustLibraryPath", Om_errNotDir(L"directory", path));
    return OM_RESULT_ERROR;
  }

  // stop monitoring
  this->_monitor.stopMonitor();

  this->_library_path = path;

  // notify we use a custom Library path
  this->_cust_library_path = true;

  if(this->_xml.hasChild(L"library")) {
    this->_xml.child(L"library").setContent(path);
  } else {
    this->_xml.addChild(L"library").setContent(path);
  }

  this->_xml.save();

  // reload library
  this->reloadModLibrary();

  // restart monitoring for the new directory
  if(this->accessesLibrary(OM_ACCESS_DIR_READ))
    this->_monitor.startMonitor(this->_library_path);

  return OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModChan::setDefLibraryPath()
{
  if(this->_locked_mod_library) {
    this->_error(L"setDefLibraryPath", L"Mod library operation is currently processing");
    return OM_RESULT_ABORT;
  }

  if(!this->_xml.valid()) {
    this->_error(L"setDefLibraryPath", L"channel is not valid");
    return OM_RESULT_ABORT;
  }

  // stop monitoring
  this->_monitor.stopMonitor();

  this->_library_path = this->_home + OM_MODCHAN_MODLIB_DIR;

  // notify we use default settings
  this->_cust_library_path = false;

  if(this->_xml.hasChild(L"library"))
    this->_xml.remChild(L"library");

  this->_xml.save();

  // reload library
  this->reloadModLibrary();

  // restart monitoring for the new directory
  if(this->accessesLibrary(OM_ACCESS_DIR_READ))
    this->_monitor.startMonitor(this->_library_path);

  return OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModChan::setCustBackupPath(const OmWString& path)
{
  if(this->_locked_mod_library) {
    this->_error(L"setCustBackupPath", L"Mod library operation is currently processing");
    return OM_RESULT_ABORT;
  }

  if(!this->_xml.valid()) {
    this->_error(L"setCustBackupPath", L"channel is not valid");
    return OM_RESULT_ABORT;
  }

  if(!Om_isDir(path)) {
    this->_error(L"setCustBackupPath", Om_errNotDir(L"directory", path));
    return OM_RESULT_ERROR;
  }

  bool has_error = false;

  if(!Om_namesMatches(this->_backup_path, path)) {

    // move content from old to new backup directory
    OmWStringArray items;
    Om_lsAll(&items, this->_backup_path);

    OmWString src_path, dst_path;

    for(size_t i = 0; i < items.size(); ++i) {

      Om_concatPaths(src_path, this->_backup_path, items[i]);
      Om_concatPaths(dst_path, path, items[i]);

      int32_t result = Om_fileMove(src_path, dst_path);
      if(result != 0) {
        this->_error(L"setCustBackupPath", Om_errMove(L"Backup element", src_path, result));
        has_error = true;
      }
    }
  }

  // change backup path
  this->_backup_path = path;

  // notify we use a custom Library path
  this->_cust_backup_path = true;

  if(this->_xml.hasChild(L"backup")) {
    this->_xml.child(L"backup").setContent(path);
  } else {
    this->_xml.addChild(L"backup").setContent(path);
  }

  this->_xml.save();

  // reload library content
  this->reloadModLibrary();

  return has_error ? OM_RESULT_ERROR : OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModChan::setDefBackupPath()
{
  if(this->_locked_mod_library) {
    this->_error(L"setDefBackupPath", L"Mod library operation is currently processing");
    return OM_RESULT_ABORT;
  }

  if(!this->_xml.valid()) {
    this->_error(L"setCustBackupPath", L"channel is not valid");
    return OM_RESULT_ABORT;
  }

  bool has_error = false;

  OmWString default_path;
  Om_concatPaths(default_path, this->_home, OM_MODCHAN_BACKUP_DIR);

  if(!Om_namesMatches(this->_backup_path, default_path)) {

    // move content from old to new backup directory
    OmWStringArray items;
    Om_lsAll(&items, this->_backup_path);

    OmWString src_path, dst_path;

    for(size_t i = 0; i < items.size(); ++i) {

      Om_concatPaths(src_path, this->_backup_path, items[i]);
      Om_concatPaths(dst_path, default_path, items[i]);

      int32_t result = Om_fileMove(src_path, dst_path);
      if(result != 0) {
        this->_error(L"setCustBackupPath", Om_errMove(L"Backup element", src_path, result));
        has_error = true;
      }
    }
  }

  this->_backup_path = default_path;

  // notify we use default settings
  this->_cust_backup_path = false;

  if(this->_xml.hasChild(L"backup"))
    this->_xml.remChild(L"backup");

  this->_xml.save();

  // reload library content
  this->reloadModLibrary();

  return has_error ? OM_RESULT_ERROR : OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setLibraryDevmod(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_library_devmode = enable;

  if(this->_xml.hasChild(L"library_devmode")) {
    this->_xml.child(L"library_devmode").setAttr(L"enable", this->_library_devmode ? 1 : 0);
  } else {
    this->_xml.addChild(L"library_devmode").setAttr(L"enable", this->_library_devmode ? 1 : 0);
  }

  this->_xml.save();

  // refresh library content
  this->reloadModLibrary();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setLibraryShowhidden(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_library_showhidden = enable;

  if(this->_xml.hasChild(L"library_showhidden")) {
    this->_xml.child(L"library_showhidden").setAttr(L"enable", this->_library_showhidden ? 1 : 0);
  } else {
    this->_xml.addChild(L"library_showhidden").setAttr(L"enable", this->_library_showhidden ? 1 : 0);
  }

  this->_xml.save();

  // refresh library content
  this->reloadModLibrary();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setLibraryCleanUnins(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_library_cleanunins = enable;

  if(this->_xml.hasChild(L"library_cleanunins")) {
    this->_xml.child(L"library_cleanunins").setAttr(L"enable", this->_library_cleanunins ? 1 : 0);
  } else {
    this->_xml.addChild(L"library_cleanunins").setAttr(L"enable", this->_library_cleanunins ? 1 : 0);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setBackupOverlap(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_backup_overlap = enable;

  if(this->_xml.hasChild(L"backup_overlap")) {
    this->_xml.child(L"backup_overlap").setAttr(L"enable", this->_backup_overlap ? 1 : 0);
  } else {
    this->_xml.addChild(L"backup_overlap").setAttr(L"enable", this->_backup_overlap ? 1 : 0);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnOverlaps(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_warn_overlaps = enable;

  OmXmlNode warn_options_node;

  if(this->_xml.hasChild(L"warn_options")) {
    warn_options_node = this->_xml.child(L"warn_options");
  } else {
    warn_options_node = this->_xml.addChild(L"warn_options");
  }

  if(warn_options_node.hasChild(L"warn_overlaps")) {
    warn_options_node.child(L"warn_overlaps").setAttr(L"enable", this->_warn_overlaps ? 1 : 0);
  } else {
    warn_options_node.addChild(L"warn_overlaps").setAttr(L"enable", this->_warn_overlaps ? 1 : 0);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnExtraInst(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_warn_extra_inst = enable;

  OmXmlNode warn_options_node;

  if(this->_xml.hasChild(L"warn_options")) {
    warn_options_node = this->_xml.child(L"warn_options");
  } else {
    warn_options_node = this->_xml.addChild(L"warn_options");
  }

  if(warn_options_node.hasChild(L"warn_extra_inst")) {
    warn_options_node.child(L"warn_extra_inst").setAttr(L"enable", this->_warn_extra_inst ? 1 : 0);
  } else {
    warn_options_node.addChild(L"warn_extra_inst").setAttr(L"enable", this->_warn_extra_inst ? 1 : 0);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnMissDeps(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_warn_miss_deps = enable;

  OmXmlNode warn_options_node;

  if(this->_xml.hasChild(L"warn_options")) {
    warn_options_node = this->_xml.child(L"warn_options");
  } else {
    warn_options_node = this->_xml.addChild(L"warn_options");
  }

  if(warn_options_node.hasChild(L"warn_miss_deps")) {
    warn_options_node.child(L"warn_miss_deps").setAttr(L"enable", this->_warn_miss_deps ? 1 : 0);
  } else {
    warn_options_node.addChild(L"warn_miss_deps").setAttr(L"enable", this->_warn_miss_deps ? 1 : 0);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnExtraUnin(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_warn_extra_unin = enable;

  OmXmlNode warn_options_node;

  if(this->_xml.hasChild(L"warn_options")) {
    warn_options_node = this->_xml.child(L"warn_options");
  } else {
    warn_options_node = this->_xml.addChild(L"warn_options");
  }

  if(warn_options_node.hasChild(L"warn_extra_unin")) {
    warn_options_node.child(L"warn_extra_unin").setAttr(L"enable", this->_warn_extra_unin ? 1 : 0);
  } else {
    warn_options_node.addChild(L"warn_extra_unin").setAttr(L"enable", this->_warn_extra_unin ? 1 : 0);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setBackupComp(int32_t method, int32_t level)
{
  if(!this->_xml.valid())
    return;

  this->_backup_method = method;
  this->_backup_level = level;

  OmXmlNode backup_comp_node;

  if(this->_xml.hasChild(L"backup_comp")) {
    backup_comp_node = this->_xml.child(L"backup_comp");
  } else {
    backup_comp_node = this->_xml.addChild(L"backup_comp");
  }

  backup_comp_node.setAttr(L"method", (int)method);
  backup_comp_node.setAttr(L"level", (int)level);

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setUpgdRename(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_upgd_rename = enable;

  OmXmlNode network_node;

  if(!this->_xml.hasChild(L"network")) {
    network_node = this->_xml.addChild(L"network");
  } else {
    network_node = this->_xml.child(L"network");
  }

  network_node.setAttr(L"upgd_rename", static_cast<int>(enable ? 1 : 0));

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnExtraDnld(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_warn_extra_dnld = enable;

  OmXmlNode network_node;

  if(this->_xml.hasChild(L"network")) {
    network_node = this->_xml.child(L"network");
  } else {
    network_node = this->_xml.addChild(L"network");
  }

  if(network_node.hasChild(L"warn_extra_dnld")) {
    network_node.child(L"warn_extra_dnld").setAttr(L"enable", this->_warn_extra_dnld ? 1 : 0);
  } else {
    network_node.addChild(L"warn_extra_dnld").setAttr(L"enable", this->_warn_extra_dnld ? 1 : 0);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnMissDnld(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_warn_miss_dnld = enable;

  OmXmlNode network_node;

  if(this->_xml.hasChild(L"network")) {
    network_node = this->_xml.child(L"network");
  } else {
    network_node = this->_xml.addChild(L"network");
  }

  if(network_node.hasChild(L"warn_miss_dnld")) {
    network_node.child(L"warn_miss_dnld").setAttr(L"enable", this->_warn_miss_dnld ? 1 : 0);
  } else {
    network_node.addChild(L"warn_miss_dnld").setAttr(L"enable", this->_warn_miss_dnld ? 1 : 0);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnUpgdBrkDeps(bool enable)
{
  if(!this->_xml.valid())
    return;

  this->_warn_upgd_brk_deps = enable;

  OmXmlNode network_node;

  if(this->_xml.hasChild(L"network")) {
    network_node = this->_xml.child(L"network");
  } else {
    network_node = this->_xml.addChild(L"network");
  }

  if(network_node.hasChild(L"warn_upgd_brk_deps")) {
    network_node.child(L"warn_upgd_brk_deps").setAttr(L"enable", this->_warn_upgd_brk_deps ? 1 : 0);
  } else {
    network_node.addChild(L"warn_upgd_brk_deps").setAttr(L"enable", this->_warn_upgd_brk_deps ? 1 : 0);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setDownLimits(uint32_t rate, uint32_t thread)
{
  if(!this->_xml.valid())
    return;

  this->_down_max_rate = rate;
  this->_down_max_thread = thread;


  OmXmlNode network_node;

  if(this->_xml.hasChild(L"network")) {
    network_node = this->_xml.child(L"network");
  } else {
    network_node = this->_xml.addChild(L"network");
  }

  OmXmlNode limits_node;

  if(network_node.hasChild(L"down_limits")) {
    limits_node = network_node.child(L"down_limits");
  } else {
    limits_node = network_node.addChild(L"down_limits");
  }

  limits_node.setAttr(L"rate", static_cast<int>(this->_down_max_rate));
  limits_node.setAttr(L"thread", static_cast<int>(this->_down_max_thread));

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setModsSourcesPath(const OmWString& path)
{
  if(!this->_xml.valid())
    return;

  this->_mods_sources_path = path;

  OmXmlNode tools_node;

  if(this->_xml.hasChild(L"tools")) {
    tools_node = this->_xml.child(L"tools");
  } else {
    tools_node = this->_xml.addChild(L"tools");
  }

  OmXmlNode sources_path_node;

  if(tools_node.hasChild(L"sources_path")) {
    sources_path_node = tools_node.child(L"sources_path");
  } else {
    sources_path_node = tools_node.addChild(L"sources_path");
  }

  sources_path_node.setContent(path);

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setLayoutRepositoriesSpan(int span)
{
  if(!this->_xml.valid())
    return;

  this->_layout_repositories_span = span;

  OmXmlNode layout_node;

  if(this->_xml.hasChild(L"layout")) {
    layout_node = this->_xml.child(L"layout");
  } else {
    layout_node = this->_xml.addChild(L"layout");
  }

  if(layout_node.hasChild(L"repositories")) {
    layout_node.child(L"repositories").setAttr(L"span", span);
  } else {
    layout_node.addChild(L"repositories").setAttr(L"span", span);
  }

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_log(unsigned level, const OmWString& origin,  const OmWString& detail) const
{
  OmWString root(L"ModChan["); root.append(this->_title); root.append(L"].");
  this->_Modhub->escalateLog(level, root + origin, detail);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_error(const OmWString& origin, const OmWString& detail)
{
  this->_lasterr = detail;
  this->_log(OM_LOG_ERR, origin, detail);
}
