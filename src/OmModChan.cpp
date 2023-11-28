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
  _valid(false),
  _modpack_list_sort(OM_SORT_NAME),
  _netpack_list_sort(OM_SORT_NAME),
  _locked_mod_library(false),
  _locked_net_library(false),
  _install_abort(false),
  _install_hth(nullptr),
  _install_hwo(nullptr),
  _install_dones(0),
  _install_percent(0),
  _install_begin_cb(nullptr),
  _install_progress_cb(nullptr),
  _install_result_cb(nullptr),
  _install_user_ptr(nullptr),
  _download_abort(false),
  _download_dones(0),
  _download_percent(0),
  _download_download_cb(nullptr),
  _download_result_cb(nullptr),
  _download_user_ptr(nullptr),
  _upgrade_abort(false),
  _upgrade_hth(nullptr),
  _upgrade_hwo(nullptr),
  _upgrade_dones(0),
  _upgrade_percent(0),
  _upgrade_begin_cb(nullptr),
  _upgrade_progress_cb(nullptr),
  _upgrade_result_cb(nullptr),
  _upgrade_user_ptr(nullptr),
  _query_abort(false),
  _query_hth(nullptr),
  _query_hwo(nullptr),
  _query_dones(0),
  _query_percent(0),
  _query_begin_cb(nullptr),
  _query_result_cb(nullptr),
  _query_user_ptr(nullptr),
  _library_path_is_cust(false),
  _library_devmode(true),
  _library_showhidden(false),
  _warn_overlaps(true),
  _warn_extra_inst(true),
  _backup_path_is_cust(false),
  _backup_comp_method(OM_METHOD_ZSTD),
  _backup_comp_level(OM_LEVEL_FAST),
  _warn_extra_unin(true),
  _warn_extra_dnld(true),
  _warn_miss_deps(true),
  _warn_miss_dnld(true),
  _warn_upgd_brk_deps(true),
  _upgd_rename(false)
{

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
  this->_lasterr.clear();

  this->_connect.clear();
  this->_xmlconf.clear();

  this->_path.clear();
  this->_home.clear();
  this->_uuid.clear();
  this->_title.clear();
  this->_index = 0;
  this->_valid = false;
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

  this->_locked_mod_library = false;
  this->_locked_net_library = false;

  this->_install_abort = false;
  Om_clearThread(this->_install_hth, this->_install_hwo);
  this->_install_hth = nullptr;
  this->_install_hwo = nullptr;
  this->_install_dones = 0;
  this->_install_percent = 0;
  this->_install_queue.clear();
  this->_install_begin_cb = nullptr;
  this->_install_progress_cb = nullptr;
  this->_install_result_cb = nullptr;
  this->_install_user_ptr = nullptr;

  this->_download_abort = false;
  this->_download_dones = 0;
  this->_download_percent = 0;
  this->_download_queue.clear();
  this->_download_download_cb = nullptr;
  this->_download_result_cb = nullptr;
  this->_download_user_ptr = nullptr;

  this->_upgrade_abort = false;
  Om_clearThread(this->_upgrade_hth, this->_upgrade_hwo);
  this->_upgrade_hth = nullptr;
  this->_upgrade_hwo = nullptr;
  this->_upgrade_dones = 0;
  this->_upgrade_percent = 0;
  this->_upgrade_queue.clear();
  this->_upgrade_begin_cb = nullptr;
  this->_upgrade_progress_cb = nullptr;
  this->_upgrade_result_cb = nullptr;
  this->_upgrade_user_ptr = nullptr;

  this->_query_abort = false;
  Om_clearThread(this->_query_hth, this->_query_hwo);
  this->_query_hth = nullptr;
  this->_query_hwo = nullptr;
  this->_query_queue.clear();
  this->_query_dones = 0;
  this->_query_percent = 0;
  this->_query_begin_cb = nullptr;
  this->_query_result_cb = nullptr;
  this->_query_user_ptr = nullptr;

  this->_library_path_is_cust = false;
  this->_library_devmode = true;
  this->_library_showhidden = false;
  this->_warn_overlaps = true;
  this->_warn_extra_inst = true;
  this->_backup_path_is_cust = false;
  this->_backup_comp_method = OM_METHOD_ZSTD;
  this->_backup_comp_level = OM_LEVEL_FAST;
  this->_warn_extra_unin = true;
  this->_warn_extra_dnld = true;
  this->_warn_miss_deps = true;
  this->_warn_miss_dnld = true;
  this->_warn_upgd_brk_deps = true;
  this->_upgd_rename = false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::open(const OmWString& path)
{
  this->close();

  // try to open and parse the XML file
  if(!this->_xmlconf.load(path, OM_XMAGIC_CHN)) {
    this->_error(L"open", L"XML definition parse error.");
    this->close(); return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_xmlconf.hasChild(L"uuid") || !this->_xmlconf.hasChild(L"title")) {
    this->_error(L"open", L"invalid XML definition: basic nodes missing.");
    this->close(); return false;
  }

  this->_path = path;

  this->_home = Om_getDirPart(this->_path);

  this->_uuid = this->_xmlconf.child(L"uuid").content();

  this->_title = this->_xmlconf.child(L"title").content();

  this->_index = this->_xmlconf.child(L"title").attrAsInt(L"index");

  // check for the presence of <install> entry
  if(this->_xmlconf.hasChild(L"install")) {

    this->_target_path = this->_xmlconf.child(L"install").content();
    if(!Om_isDir(this->_target_path)) {
      this->_log(OM_LOG_WRN, L"open", Om_errNotDir(L"Target directory", this->_target_path));
    }

  } else {
    this->_error(L"open", L"Invalid XML definition: <install> node missing.");
    this->close(); return false;
  }

  // check for the presence of <library> entry for custom Library path
  if(this->_xmlconf.hasChild(L"library")) {

    this->_library_path_is_cust = true;
    this->_library_path = this->_xmlconf.child(L"library").content();

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
  if(this->_xmlconf.hasChild(L"backup")) {

    this->_backup_path_is_cust = true;
    this->_backup_path = this->_xmlconf.child(L"backup").content();

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

  if(this->_xmlconf.hasChild(L"backup_comp")) {

    this->_backup_comp_method = this->_xmlconf.child(L"backup_comp").attrAsInt(L"method");

    if(this->_backup_comp_method > 95 || this->_backup_comp_method < 0)
      this->_backup_comp_method = -1;

    this->_backup_comp_level = this->_xmlconf.child(L"backup_comp").attrAsInt(L"level");

    if(this->_backup_comp_level > 9)
      this->_backup_comp_level = 0;

  } else {
    // create default values
    this->setBackupComp(this->_backup_comp_method, this->_backup_comp_level);
  }

  if(this->_xmlconf.hasChild(L"library_sort")) {
    this->_modpack_list_sort = this->_xmlconf.child(L"library_sort").attrAsInt(L"sort");
  } else {
    // create default values
    // TODO
  }

  if(this->_xmlconf.hasChild(L"library_devmode")) {
    this->_library_devmode = this->_xmlconf.child(L"library_devmode").attrAsInt(L"enable");
  } else {
    // create default values
    this->setLibraryDevmod(this->_library_devmode);
  }

  if(this->_xmlconf.hasChild(L"library_showhidden")) {
    this->_library_showhidden = this->_xmlconf.child(L"library_showhidden").attrAsInt(L"enable");
  } else {
    // create default values
    this->setLibraryShowhidden(this->_library_showhidden); //< create default
  }

  if(this->_xmlconf.hasChild(L"remotes_sort")) {
    this->_modpack_list_sort = this->_xmlconf.child(L"remotes_sort").attrAsInt(L"sort");
  } else {
    // create default values
    // TODO
  }

  // Check warnings options
  if(this->_xmlconf.hasChild(L"warn_options")) {

    OmXmlNode xml_wrn = this->_xmlconf.child(L"warn_options");

    if(xml_wrn.hasChild(L"warn_overlaps")) {
      this->_warn_overlaps = xml_wrn.child(L"warn_overlaps").attrAsInt(L"enable");
    } else {
      this->setWarnOverlaps(this->_warn_overlaps);
    }

    if(xml_wrn.hasChild(L"warn_extra_inst")) {
      this->_warn_extra_inst = xml_wrn.child(L"warn_extra_inst").attrAsInt(L"enable");
    } else {
      this->setWarnExtraInst(this->_warn_extra_inst);
    }

    if(xml_wrn.hasChild(L"warn_miss_deps")) {
      this->_warn_miss_deps = xml_wrn.child(L"warn_miss_deps").attrAsInt(L"enable");
    } else {
      this->setWarnMissDeps(this->_warn_miss_deps);
    }

    if(xml_wrn.hasChild(L"warn_extra_unin")) {
      this->_warn_extra_unin = xml_wrn.child(L"warn_extra_unin").attrAsInt(L"enable");
    } else {
      this->setWarnExtraUnin(this->_warn_extra_unin);
    }

  } else {

    // create default
    this->_xmlconf.addChild(L"warn_options");
    this->setWarnOverlaps(this->_warn_overlaps);
    this->setWarnExtraInst(this->_warn_extra_inst);
    this->setWarnMissDeps(this->_warn_miss_deps);
    this->setWarnExtraUnin(this->_warn_extra_unin);
  }

  // Get network repository list
  if(this->_xmlconf.hasChild(L"network")) {

    OmXmlNode xml_net = this->_xmlconf.child(L"network");

    if(xml_net.hasAttr(L"upgd_rename")) {
      this->_upgd_rename = xml_net.attrAsInt(L"upgd_rename");
    } else {
      this->_upgd_rename = false;
    }

    OmXmlNodeArray xml_rep_list;
    xml_net.children(xml_rep_list, L"repository");

    OmWString base, name;

    for(size_t i = 0; i < xml_rep_list.size(); ++i) {

      base = xml_rep_list[i].attrAsString(L"base");
      name = xml_rep_list[i].attrAsString(L"name");

      OmNetRepo* ModRepo = new OmNetRepo(this);

      if(xml_rep_list[i].hasAttr(L"title")) {

        if(!ModRepo->setup(base, name, xml_rep_list[i].attrAsString(L"title"))) {
          delete ModRepo; ModRepo = nullptr;
        }

      } else {

        if(!ModRepo->setup(base, name)) {
          delete ModRepo; ModRepo = nullptr;
        }

      }

      if(ModRepo)
        this->_repository_list.push_back(ModRepo);

    }

    if(xml_net.hasChild(L"warn_extra_dnld")) {
      this->_warn_extra_dnld = xml_net.child(L"warn_extra_dnld").attrAsInt(L"enable");
    } else {
      this->setWarnExtraDnld(this->_warn_extra_dnld);
    }

    if(xml_net.hasChild(L"warn_miss_dnld")) {
      this->_warn_miss_dnld = xml_net.child(L"warn_miss_dnld").attrAsInt(L"enable");
    } else {
      this->setWarnMissDnld(this->_warn_miss_dnld);
    }

    if(xml_net.hasChild(L"warn_upgd_brk_deps")) {
      this->_warn_upgd_brk_deps = xml_net.child(L"warn_upgd_brk_deps").attrAsInt(L"enable");
    } else {
      this->setWarnUpgdBrkDeps(this->_warn_upgd_brk_deps);
    }

  } else {
    // create default
    this->_xmlconf.addChild(L"network");
    this->setWarnExtraDnld(this->_warn_extra_dnld);
    this->setWarnMissDnld(this->_warn_miss_dnld);
    this->setWarnUpgdBrkDeps(this->_warn_upgd_brk_deps);
  }

  this->_log(OM_LOG_OK, L"open", L"succeed");

  // this location is OK and ready
  this->_valid = true;

  // Refresh library
  this->refreshModLibrary();

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::rename(const OmWString& title)
{
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

  return !has_error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::refreshLibraries()
{
  bool has_change = false;

  if(this->refreshModLibrary())
    has_change = true;

  if(this->refreshNetLibrary())
    has_change = true;

  return has_change;

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::clearModLibrary()
{
  if(!this->_modpack_list.empty()) {

    for(size_t i = 0; i < this->_modpack_list.size(); ++i)
      delete this->_modpack_list[i];

    this->_modpack_list.clear();

    return true;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::refreshModLibrary()
{
  if(!this->libraryDirAccess(false)) { // check for read access
    #ifdef DEBUG
    std::cout << "DEBUG => OmModChan::refreshModLibrary X\n";
    #endif
    return this->clearModLibrary();
  }

  uint64_t mod_hash;
  OmWStringArray path_ls;
  OmModPack* ModPack;


  // track list changes
  bool found, has_change = false;

  // our package list is not empty, we will check for added or removed item
  if(this->_modpack_list.size()) {

    // get content of the package Library folder
    Om_lsFileFiltered(&path_ls, this->_library_path, L"*.zip", true, this->_library_showhidden);
    Om_lsFileFiltered(&path_ls, this->_library_path, L"*." OM_PKG_FILE_EXT, true, this->_library_showhidden);
    if(this->_library_devmode)
      Om_lsDir(&path_ls, this->_library_path, true, this->_library_showhidden);

    // search for unavailable Sources or ghosts package
    for(size_t p = 0; p < this->_modpack_list.size(); ++p) {

      found = false;

      // check whether Mod Source still exists in Library directory
      if(this->_modpack_list[p]->hasSource()) {
        for(size_t i = 0; i < path_ls.size(); ++i) {
          if(this->_modpack_list[p]->sourcePath() == path_ls[i]) { //< compare Source paths
            found = true; break;
          }
        }
      }

      if(found) {
        // refresh source
        this->_modpack_list[p]->refreshSource();
      } else {
        // check whether this Mod is installed
        if(this->_modpack_list[p]->hasBackup()) {
          // keep Mod in library but remove its source side
          this->_modpack_list[p]->clearSource();
        } else {
          // this is a "ghost", delete Mod from library
          delete this->_modpack_list[p];
          this->_modpack_list.erase(this->_modpack_list.begin()+p); --p;
        }
        has_change = true;
      }
    }

    // Search for new Sources
    for(size_t i = 0; i < path_ls.size(); ++i) {

      found = false;

      mod_hash = Om_getXXHash3(Om_getFilePart(path_ls[i]));

      for(size_t p = 0; p < this->_modpack_list.size(); ++p) {

        if(this->_modpack_list[p]->hasSource()) {
          if(path_ls[i] == this->_modpack_list[p]->sourcePath()) {
            found = true; break;
          }
        } else {
          // checks whether Hash values matches
          if(mod_hash == this->_modpack_list[p]->hash()) {
            // this Package Source obviously matches to a currently
            // installed one, since we got a Package with the same Hash but
            // Source is missing, so we add the Source to this Package Backup
            this->_modpack_list[p]->parseSource(path_ls[i]);
            has_change = true;
            found = true; break;
          }
        }
      }
      // This is a new Package Source
      if(!found) {
        ModPack = new OmModPack(this);
        if(ModPack->parseSource(path_ls[i])) {
          has_change = true;
          this->_modpack_list.push_back(ModPack);
        } else {
          delete ModPack;
        }
      }
    }

  } else {

    has_change = true;

    // get Backup directory content
    Om_lsFileFiltered(&path_ls, this->_backup_path, L"*.zip", true, true);
    Om_lsFileFiltered(&path_ls, this->_backup_path, L"*." OM_BCK_FILE_EXT, true, true);
    Om_lsDir(&path_ls, this->_backup_path, true, true);

    // add all available and valid Backups
    for(size_t i = 0; i < path_ls.size(); ++i) {

      ModPack = new OmModPack(this);

      if(ModPack->parseBackup(path_ls[i])) {
        this->_modpack_list.push_back(ModPack);
      } else {
        delete ModPack;
      }

    }

    // get Library directory content
    path_ls.clear();
    Om_lsFileFiltered(&path_ls, this->_library_path, L"*.zip", true, this->_library_showhidden);
    Om_lsFileFiltered(&path_ls, this->_library_path, L"*." OM_PKG_FILE_EXT, true, this->_library_showhidden);
    if(this->_library_devmode)
      Om_lsDir(&path_ls, this->_library_path, true, this->_library_showhidden);

    // Link Sources to matching Backup, or add new Sources
    for(size_t i = 0; i < path_ls.size(); ++i) {

      mod_hash = Om_getXXHash3(Om_getFilePart(path_ls[i]));

      found = false;

      // check whether this Mod Source matches an existing Backup
      for(size_t p = 0; p < this->_modpack_list.size(); p++) {
        if(mod_hash == this->_modpack_list[p]->hash()) {
          this->_modpack_list[p]->parseSource(path_ls[i]);
          found = true; break;
        }
      }

      // no Backup found for this Mod Source, adding new
      if(!found) {
        ModPack = new OmModPack(this);
        if(ModPack->parseSource(path_ls[i])) {
          this->_modpack_list.push_back(ModPack);
        } else {
          delete ModPack;
        }
      }
    }
  }

  if(has_change)
    this->sortModLibrary();

  #ifdef DEBUG
  std::cout << "DEBUG => OmModChan::refreshModLibrary " << (has_change ? "+-" : "==") << "\n";
  #endif

  return has_change;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::ghostbusterModLibrary()
{
  OmModPack* ModPack;
  bool has_change = false;

  // search for ghost packages
  for(size_t p = 0; p < this->_modpack_list.size(); ++p) {

    if(!this->_modpack_list[p]->hasBackup() && !this->_modpack_list[p]->hasSource()) {
      // The Package has no Backup and Source is no longer available
      // this is a ghost, we have to remove it
      has_change = true;
      ModPack = this->_modpack_list[p];
      this->_modpack_list.erase(this->_modpack_list.begin()+p); --p;
      delete ModPack;
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
  if(a->hasBackup() && b->hasBackup()) {
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
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setModLibrarySort(OmSort sorting)
{
  // if requested sorting is same as current, reverse order
  if(OM_HAS_BIT(this->_modpack_list_sort, sorting)) {

    OM_TOG_BIT(this->_modpack_list_sort, OM_SORT_INVT);

  } else {

    this->_modpack_list_sort = sorting;
  }

  // save the current sorting
  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"library_sort")) {

      this->_xmlconf.child(L"library_sort").setAttr(L"sort", this->_modpack_list_sort);

    } else {

      this->_xmlconf.addChild(L"library_sort").setAttr(L"sort", this->_modpack_list_sort);
    }

    this->_xmlconf.save();
  }

  this->sortModLibrary();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::isOverlapped(size_t i) const
{
  uint64_t mod_hash = this->_modpack_list[i]->hash();

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
void OmModChan::findOverlapped(const OmModPack* ModPack, OmUint64Array* overlaps) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i)
    if(this->_modpack_list[i]->hasBackup())
      if(ModPack->canOverlap(this->_modpack_list[i]))
        overlaps->push_back(this->_modpack_list[i]->hash());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::findOverlapped(const OmModPack* ModPack, OmPModPackArray* overlaps) const
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
bool OmModChan::isDependency(const OmModPack* ModPack) const
{
  for(size_t i = 0; i < this->_modpack_list.size(); ++i)
    if(ModPack != this->_modpack_list[i])
      if(this->_modpack_list[i]->hasDepend(ModPack->iden()))
        return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::hasMissingDepend(const OmModPack* ModPack) const
{

  for(size_t i = 0; i < ModPack->dependCount(); ++i) {

    bool is_missing = true;

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

    if(is_missing)
      return true;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::_get_install_depends(const OmModPack* ModPack, OmPModPackArray* depends, OmWStringArray* missings) const
{
  for(size_t i = 0; i < ModPack->dependCount(); ++i) {

    bool missing = true;

    for(size_t j = 0; j < this->_modpack_list.size(); ++j) {

      // rely only on packages
      if(this->_modpack_list[j]->sourceIsDir())
        continue;

      if(ModPack->getDependIden(i) == this->_modpack_list[j]->iden()) {

        this->_get_install_depends(this->_modpack_list[j], depends, missings);

        // we add to list only if unique and not already installed, this allow
        // us to get a consistent dependency list for a bunch of package by
        // calling this function for each package without clearing the list
        if(!this->_modpack_list[j]->hasBackup())
          Om_push_backUnique(*depends, this->_modpack_list[j]);

        missing = false;
        break;
      }
    }

    if(missing)
      Om_push_backUnique(*missings, ModPack->getDependIden(i));
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::prepareInstalls(const OmPModPackArray& selection, OmPModPackArray* installs, OmWStringArray* overlaps, OmWStringArray* depends, OmWStringArray* missings) const
{
  // gather dependencies and create missing lists
  OmPModPackArray found_depends;
  for(size_t i = 0; i < selection.size(); ++i)
    this->_get_install_depends(selection[i], &found_depends, missings);

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

  // get overlaps list including simulated installation
  for(size_t i = 0; i < installs->size(); ++i) {

    // test overlapping against installed Mods
    for(size_t j = 0; j < this->_modpack_list.size(); ++j) {
      if(this->_modpack_list[j]->hasBackup()) {
        if(installs->at(i)->canOverlap(this->_modpack_list[j]))
          overlaps->push_back(this->_modpack_list[j]->iden());
      }
    }

    // test overlapping against Mods to be installed
    for(size_t j = 0; j < footprintArray.size(); ++j) {
      if(installs->at(i)->canOverlap(footprintArray[j]))
        overlaps->push_back(installs->at(j)->iden());
    }

    // create installation footprint of package
    footprint.clear();
    installs->at(i)->getFootprint(&footprint);
    footprintArray.push_back(footprint);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::queueInstalls(const OmPModPackArray& selection, Om_beginCb begin_cb, Om_progressCb progress_cb, Om_resultCb result_cb, void* user_ptr)
{
  if(this->_install_queue.empty()) {

    // another operation is currently processing
    if(this->_locked_mod_library) {

      this->_log(OM_LOG_WRN, L"queueInstalls", L"local library is locked by another operation");

      if(result_cb)  // flush all results with abort
        for(size_t i = 0; i << selection.size(); ++i)
          result_cb(user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(selection[i]));

      return;
    }

    this->_install_begin_cb = begin_cb;
    this->_install_progress_cb = progress_cb;
    this->_install_result_cb = result_cb;
    this->_install_user_ptr = user_ptr;

    // reset global progression parameters
    this->_install_dones = 0;
    this->_install_percent = 0;

  } else {

    // emit a warning in case a crazy client starts new download with
    // different parameters than current
    if(this->_install_begin_cb != begin_cb ||
       this->_install_progress_cb != progress_cb ||
       this->_install_result_cb != result_cb ||
       this->_install_user_ptr != user_ptr) {
      this->_log(OM_LOG_WRN, L"queueInstalls", L"changing callbacks for a running thread is not allowed");
    }
  }

  // lock the local library to prevent concurrent array manipulation
  this->_locked_mod_library = true;

  // reset abort flag
  this->_install_abort = false;

  for(size_t i = 0; i < selection.size(); ++i)
    Om_push_backUnique(this->_install_queue, selection[i]);

  if(!this->_install_hth) {

    // launch thread
    this->_install_hth = Om_createThread(OmModChan::_install_run_fn, this);
    this->_install_hwo = Om_waitForThread(this->_install_hth, OmModChan::_install_end_fn, this);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::execInstalls(const OmPModPackArray& selection, Om_beginCb begin_cb, Om_progressCb progress_cb, Om_resultCb result_cb, void* user_ptr)
{
  bool invalid_call = false;

  // install queue not empty processing
  if(!this->_install_queue.empty()) {
    this->_log(OM_LOG_WRN, L"execInstalls", L"install queue is not empty");
    invalid_call = true;
  }

  // another operation is currently processing
  if(this->_locked_mod_library) {
    this->_log(OM_LOG_WRN, L"execInstalls", L"local library is locked by another operation");
    invalid_call = true;
  }

  if(invalid_call) {
    if(result_cb)  // flush all results with abort
      for(size_t i = 0; i << selection.size(); ++i)
        result_cb(user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(selection[i]));

    return;
  }

  this->_install_begin_cb = begin_cb;
  this->_install_progress_cb = progress_cb;
  this->_install_result_cb = result_cb;
  this->_install_user_ptr = user_ptr;

  // reset global progression parameters
  this->_install_dones = 0;
  this->_install_percent = 0;

  // lock the local library to prevent concurrent array manipulation
  this->_locked_mod_library = true;

  // reset abort flag
  this->_install_abort = false;

  for(size_t i = 0; i < selection.size(); ++i)
    Om_push_backUnique(this->_install_queue, selection[i]);

  // run install process without thread
  OmModChan::_install_run_fn(this);
  OmModChan::_install_end_fn(this, 0);
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
void OmModChan::abortInstalls()
{
  this->_install_abort = true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmModChan::_install_run_fn(void* ptr)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  DWORD exit_code = 0;

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_install_run_fn : enter\n";
  #endif // DEBUG

  while(self->_install_queue.size()) {

    OmModPack* ModPack = self->_install_queue.front();

    if(self->_install_abort) {

      // flush all queue with abort result

      if(self->_install_result_cb)
        self->_install_result_cb(self->_install_user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(ModPack));

      self->_install_queue.pop_front();

      continue;
    }

    // call client result callback so it can perform proper operations
    if(self->_install_begin_cb)
      self->_install_begin_cb(self->_install_user_ptr, reinterpret_cast<uint64_t>(ModPack));

    OmResult result;

    if(ModPack->hasBackup()) {

      // This is a Restore operation
      result = ModPack->restoreData(OmModChan::_install_progress_fn, self);

      // call client result callback so it can perform proper operations
      if(self->_install_result_cb)
        self->_install_result_cb(self->_install_user_ptr, result, reinterpret_cast<uint64_t>(ModPack));

      if(result == OM_RESULT_ERROR)
        exit_code = 1;

    } else {

      // This is an Install operation
      result = ModPack->makeBackup(OmModChan::_install_progress_fn, self);
      if(result == OM_RESULT_OK)
        result = ModPack->applySource(OmModChan::_install_progress_fn, self);

      // call client result callback so it can perform proper operations
      if(self->_install_result_cb)
        self->_install_result_cb(self->_install_user_ptr, result, reinterpret_cast<uint64_t>(ModPack));

      if(result != OM_RESULT_OK) {

        // restore any stored Backup data
        ModPack->restoreData(OmModChan::_install_progress_fn, self, true);

        // reset progression status
        if(self->_install_progress_cb)
          self->_install_progress_cb(self->_install_user_ptr, 0, 0, reinterpret_cast<uint64_t>(ModPack));

        if(result == OM_RESULT_ERROR)
          exit_code = 1;
      }

    }

    self->_install_dones++;
    self->_install_queue.pop_front();
  }

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_install_run_fn : leave\n";
  #endif // DEBUG

  return exit_code;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_install_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  // compute global queue progress percentage
  double queue_percents = self->_install_dones * 100;
  for(size_t i = 0; i < self->_install_queue.size(); ++i)
    queue_percents += self->_install_queue[i]->operationProgress();

  self->_install_percent = queue_percents / (self->_install_dones + self->_install_queue.size());

  bool keep_on;

  if(self->_install_progress_cb) {
    keep_on = self->_install_progress_cb(self->_install_user_ptr, tot, cur, param);
  } else {
    keep_on = true;
  }

  return self->_install_abort ? false : keep_on;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmModChan::_install_end_fn(void* ptr, uint8_t fired)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_install_end_fn\n";
  #endif // DEBUG

  // unlock the local library
  self->_locked_mod_library = false;

  //DWORD exit_code = Om_threadExitCode(self->_install_hth);
  Om_clearThread(self->_install_hth, self->_install_hwo);

  self->_install_dones = 0;
  self->_install_percent = 0;

  self->_install_hth = nullptr;
  self->_install_hwo = nullptr;

  self->_install_user_ptr = nullptr;
  self->_install_progress_cb = nullptr;
  self->_install_result_cb = nullptr;
  self->_install_begin_cb = nullptr;
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
bool OmModChan::purgeBackupData(Om_progressCb progress_cb, void* user_ptr)
{
  // TODO: Rrefaire tout ça en utilisant la fille d'attente dans un thread

  // checks for access to backup folder
  if(!this->backupDirAccess(true)) { //< check for read and write
    this->_error(L"purgeBackupData", L"Backup folder access error.");
    return false;
  }
  // checks for access to Target path
  if(!this->targetDirAccess(true)) { //< check for read and write
    this->_error(L"purgeBackupData", L"Target path access error.");
    return false;
  }

  // get list of all installed packages
  OmPModPackArray selection; //< our select list

  for(size_t i = 0; i < this->_modpack_list.size(); ++i)
    if(this->_modpack_list[i]->hasBackup())
      selection.push_back(this->_modpack_list[i]);

  // if no package installed, nothing to purge
  if(selection.empty())
    return true;

  // initialize progression
  size_t progress_tot, progress_cur;
  if(progress_cb) {
    progress_tot = selection.size();
    progress_cur = 0;
    if(!progress_cb(user_ptr, progress_tot, progress_cur, 0))
      return true;
  }

  // even if we uninstall all packages, we need to get a sorted list
  // so we prepare with all overlaps and dependencies checking
  OmPModPackArray restores;
  OmWStringArray overlappings, dependents;

  // prepare packages uninstall and backups restoration
  this->prepareRestores(selection, &restores, &overlappings, &dependents);

  bool has_error = false;

  unsigned n = 0;

  // here we go for uninstall all packages
  for(size_t i = 0; i < restores.size(); ++i) {

    // call progression callback
    if(progress_cb) {
      progress_cur++;
      if(!progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(restores[i]->iden().c_str())))
        break;
    }

    // uninstall package
    if(restores[i]->restoreData()) {
      n++; //< increase counter
    } else {
      this->_error(L"purgeBackupData", restores[i]->lastError());
      has_error = true;
    }

    #ifdef DEBUG
    Sleep(50); //< for debug
    #endif
  }

  // refresh library
  this->refreshModLibrary();

  return !has_error;
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
    if(this->_netpack_list[i]->refreshStatus())
      has_change = true;
  }

  if(has_change)
    this->sortNetLibrary();

  #ifdef DEBUG
  std::cout << "DEBUG => OmModChan::refreshNetLibrary " << (has_change ? "+-" : "==") << "\n";
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

    for(size_t m = 0; m < this->_modpack_list.size(); ++m) {

      // ignore directory Mods
      if(this->_modpack_list[m]->sourceIsDir())
        continue;

      if(ModPack->getDependIden(i) == this->_modpack_list[m]->iden()) {

        this->_get_missing_depends(ModPack, missings);

        is_missing = false;
        break;
      }
    }

    if(is_missing)
      Om_push_backUnique(*missings, ModPack->getDependIden(i));
  }
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

    for(size_t n = 0; n < this->_netpack_list.size(); ++n) {

      if(missing_depends.at(i) == this->_netpack_list[n]->iden()) {

        // add Mod dependencies
        this->getNetpackDepends(this->_netpack_list[n], downloads, missings);

        // add Mod itslef
        Om_push_backUnique(*downloads, this->_netpack_list[n]);

        is_missing = false;
      }
    }

    if(is_missing)
      Om_push_backUnique(*missings, missing_depends.at(i));
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::getNetpackDepends(const OmNetPack* NetPack, OmPNetPackArray* depends, OmWStringArray* missings) const
{
  // first check required dependency in local library to
  // gather all missing dependencies, this will add dependencies
  // to found in the remote package list
  for(size_t i = 0; i < NetPack->dependCount(); ++i) {

    bool in_library = false;

    // first check whether required dependency is in package library
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

    if(in_library) //< skip if already in library
      continue;

    bool is_missing = true;

    for(size_t n = 0; n < this->_netpack_list.size(); ++n) {

      if(NetPack->getDependIden(i) == this->_netpack_list[n]->iden()) {

        // add Mod dependencies
        this->getNetpackDepends(this->_netpack_list[n], depends, missings);

        // add Mod itslef
        Om_push_backUnique(*depends, this->_netpack_list[n]);

        is_missing = false;
        break;
      }
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
    this->getNetpackDepends(selection[i], &found_depends, missings);

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
void OmModChan::queueDownloads(const OmPNetPackArray& selection, Om_downloadCb download_cb, Om_resultCb result_cb, void* user_ptr)
{
  if(this->_download_queue.empty()) {

    // another operation is currently processing
    if(this->_locked_net_library) {

      this->_log(OM_LOG_WRN, L"startDownloads", L"network library is locked by another operation");

      if(result_cb) // flush all with abort result
        for(size_t i = 0; i < selection.size(); ++i)
          result_cb(user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(selection[i]));

      return;
    }

    this->_download_user_ptr = user_ptr;
    this->_download_download_cb = download_cb;
    this->_download_result_cb = result_cb;

    // reset global progression
    this->_download_dones = 0;
    this->_download_percent = 0;

  } else {

    // emit a warning in case a crazy client starts new download with
    // different parameters than current
    if(this->_download_download_cb != download_cb ||
       this->_download_result_cb != result_cb ||
       this->_download_user_ptr != user_ptr) {
      this->_log(OM_LOG_WRN, L"startDownloads", L"subsequent downloads with different parameters");
    }
  }

  this->_locked_net_library = true;

  this->_download_abort = false;

  for(size_t i = 0; i < selection.size(); ++i) {

    // add download to stack
    Om_push_backUnique(this->_download_queue, selection[i]);

    // start download
    if(!selection[i]->startDownload(OmModChan::_download_download_fn, OmModChan::_download_result_fn, this)) {

      if(result_cb) // call result callback with error
        result_cb(user_ptr, OM_RESULT_ERROR, reinterpret_cast<uint64_t>(selection[i]));
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::stopDownloads()
{
  // we abort all processing downloads
  this->_download_abort = true;

  // if abort request was fired, we must stop downloads sequentially to
  // prevent callback concurrent calls that mess up all process, so we
  // only stop the last started download, they will be aborted in cascade
  // through the result callback
  this->_download_queue.back()->stopDownload();
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
bool OmModChan::_download_download_fn(void* ptr, int64_t tot, int64_t cur, int64_t rate, uint64_t param)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  if(self->_download_download_cb)
    return self->_download_download_cb(self->_download_user_ptr, tot, cur, rate, param);

  return true;
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

  // remove download from stack
  Om_eraseValue(self->_download_queue, NetPack);

  // increase download done count
  self->_download_dones++;

  // update global progress
  double queue_percents = self->_download_dones * 100;
  for(size_t i = 0; i < self->_download_queue.size(); ++i)
    queue_percents += self->_download_queue[i]->downloadProgress();

  self->_download_percent = queue_percents / (self->_download_dones + self->_download_queue.size());

  // call client callback
  if(self->_download_result_cb)
    self->_download_result_cb(self->_download_user_ptr, final_result, param);

  if(self->_download_queue.size()) {

    // if abort request was fired, we must stop downloads sequentially to
    // prevent callback concurrent calls that mess up all process
    if(self->_download_abort) {
      self->_download_queue.back()->stopDownload();
    }
  } else {

    self->_locked_net_library = false;

    self->_download_dones = 0;
    self->_download_percent = 0;

    self->_download_user_ptr = nullptr;
    self->_download_download_cb = nullptr;
    self->_download_result_cb = nullptr;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::queueUpgrades(const OmPNetPackArray& selection, Om_beginCb begin_cb, Om_progressCb progress_cb, Om_resultCb result_cb, void* user_ptr)
{
  if(this->_upgrade_queue.empty()) {

    // another operation is currently processing
    if(this->_locked_mod_library) {

      this->_log(OM_LOG_WRN, L"launchUpgrades", L"local library is locked by another operation");

      if(result_cb)  // flush all results with abort
        for(size_t i = 0; i << selection.size(); ++i)
          result_cb(user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(selection[i]));

      return;
    }

    this->_upgrade_begin_cb = begin_cb;
    this->_upgrade_progress_cb = progress_cb;
    this->_upgrade_result_cb = result_cb;
    this->_upgrade_user_ptr = user_ptr;

    // reset global progress
    this->_upgrade_dones = 0;
    this->_upgrade_percent = 0;

  } else {

    // emit a warning in case a crazy client starts new download with
    // different parameters than current
    if(this->_upgrade_begin_cb != begin_cb ||
       this->_upgrade_progress_cb != progress_cb ||
       this->_upgrade_result_cb != result_cb ||
       this->_upgrade_user_ptr != user_ptr) {
      this->_log(OM_LOG_WRN, L"launchUpgrades", L"subsequent upgrade with different parameters");
    }
  }

  // lock the local library to prevent concurrent array manipulation
  this->_locked_mod_library = true;

  // reset abort flag
  this->_upgrade_abort = false;

  for(size_t i = 0; i < selection.size(); ++i)
    Om_push_backUnique(this->_upgrade_queue, selection[i]);

  if(!this->_upgrade_hth) {

    // launch thread
    this->_upgrade_hth = Om_createThread(OmModChan::_upgrade_run_fn, this);
    this->_upgrade_hwo = Om_waitForThread(this->_upgrade_hth, OmModChan::_upgrade_end_fn, this);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::abortUpgrades()
{
  this->_upgrade_abort = true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmModChan::_upgrade_run_fn(void* ptr)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  DWORD exit_code = 0;

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_upgrade_run_fn : enter\n";
  #endif // DEBUG

  while(self->_upgrade_queue.size()) {

    OmNetPack* NetPack = self->_upgrade_queue.front();

    if(self->_upgrade_abort) {

      // flush all queue with abort

      if(self->_upgrade_result_cb)
        self->_upgrade_result_cb(self->_upgrade_user_ptr, OM_RESULT_ABORT, reinterpret_cast<uint64_t>(NetPack));

      self->_upgrade_queue.pop_front();

      continue;
    }

    // call client result callback so it can perform proper operations
    if(self->_upgrade_begin_cb)
      self->_upgrade_begin_cb(self->_upgrade_user_ptr, reinterpret_cast<uint64_t>(NetPack));

    OmResult result = NetPack->upgradeReplace(OmModChan::_upgrade_progress_fn, self);

    if(result == OM_RESULT_ERROR)
      exit_code = 1;

    // call client result callback so it can perform proper operations
    if(self->_upgrade_result_cb)
      self->_upgrade_result_cb(self->_upgrade_user_ptr, result, reinterpret_cast<uint64_t>(NetPack));

    self->_upgrade_dones++;

    self->_upgrade_queue.pop_front();
  }

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_upgrade_run_fn : leave\n";
  #endif // DEBUG

  return exit_code;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::_upgrade_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmModChan* self = static_cast<OmModChan*>(ptr);

  // update global progress
  double queue_percents = self->_upgrade_dones * 100;
  for(size_t i = 0; i < self->_upgrade_queue.size(); ++i)
    queue_percents += self->_upgrade_queue[i]->upgradeProgress();

  self->_upgrade_percent = queue_percents / (self->_upgrade_dones + self->_upgrade_queue.size());

  bool keep_on;

  if(self->_upgrade_progress_cb) {
    keep_on = self->_upgrade_progress_cb(self->_upgrade_user_ptr, tot, cur, param);
  } else {
    keep_on = true;
  }

  return self->_upgrade_abort ? false : keep_on;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmModChan::_upgrade_end_fn(void* ptr, uint8_t fired)
{
  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_upgrade_end_fn\n";
  #endif // DEBUG

  OmModChan* self = static_cast<OmModChan*>(ptr);

  // unlock the local library
  self->_locked_mod_library = false;

  //DWORD exit_code = Om_threadExitCode(self->_upgrade_hth);
  Om_clearThread(self->_upgrade_hth, self->_upgrade_hwo);

  self->_upgrade_hth = nullptr;
  self->_upgrade_hwo = nullptr;

  self->_upgrade_dones = 0;
  self->_upgrade_percent = 0;

  self->_upgrade_user_ptr = nullptr;
  self->_upgrade_progress_cb = nullptr;
  self->_upgrade_result_cb = nullptr;
  self->_upgrade_begin_cb = nullptr;
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
  if(a->status() == b->status()) {
    return OmModChan::_compare_net_name(a, b);
  } else {
    if(a->hasStatus(PACK_NEW) == b->hasStatus(PACK_NEW)) {
      return (a->status() < b->status());
    } else {
      return (!a->hasStatus(PACK_NEW) && b->hasStatus(PACK_NEW));
    }
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
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setNetLibrarySort(OmSort sorting)
{
  // if requested sorting is same as current, reverse order

  if(OM_HAS_BIT(this->_netpack_list_sort, sorting)) {

    OM_TOG_BIT(this->_netpack_list_sort, OM_SORT_INVT);

  } else {

    this->_netpack_list_sort = sorting;
  }

  // save the current sorting
  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"remotes_sort")) {
      this->_xmlconf.child(L"remotes_sort").setAttr(L"sort", this->_netpack_list_sort);
    } else {
      this->_xmlconf.addChild(L"remotes_sort").setAttr(L"sort", this->_netpack_list_sort);
    }

    this->_xmlconf.save();
  }

  this->sortNetLibrary();
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
  if(this->_xmlconf.valid()) {

    // get or create <network> node where repositories are listed
    OmXmlNode network_node;

    if(!this->_xmlconf.hasChild(L"network")) {
      network_node = this->_xmlconf.addChild(L"network");
    } else {
      network_node = this->_xmlconf.child(L"network");
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
    this->_xmlconf.save();

    // add repository in local list
    OmNetRepo* ModRepo = new OmNetRepo(this);

    // set repository parameters
    if(!ModRepo->setup(base, name)) {
      this->_error(L"addRepository", ModRepo->lastError());
      delete ModRepo; return false;
    }

    // add to list
    this->_repository_list.push_back(ModRepo);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::removeRepository(size_t i)
{
  if(i >= this->_repository_list.size())
    return;

  // Remove repository entry from configuration
  if(this->_xmlconf.valid()) {

    OmNetRepo* NetRepo = this->_repository_list[i];

    // get <network> node
    OmXmlNode network_node = this->_xmlconf.child(L"network");
    if(!network_node.empty()) {

      // retrieve proper <repository> reference then remove node
      OmXmlNodeArray repository_nodes;
      network_node.children(repository_nodes, L"repository");

      for(size_t k = 0; k < repository_nodes.size(); ++k) {

        if(NetRepo->urlBase() == repository_nodes[k].attrAsString(L"base")) {

          if(NetRepo->urlName() == repository_nodes[k].attrAsString(L"name")) {

            network_node.remChild(repository_nodes[k]); break;
          }
        }
      }
    }

    // save configuration
    this->_xmlconf.save();

    // remove all Remote packages related to this Repository
    size_t i = this->_repository_list.size();

    while(i--) {

      if(this->_netpack_list[i]->NetRepo() == NetRepo) {

        delete this->_netpack_list[i];

        this->_netpack_list.erase(this->_netpack_list.begin() + i);
      }
    }

    // delete object and remove it from local list
    delete NetRepo;
    this->_repository_list.erase(this->_repository_list.begin() + i);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::abortQueries()
{
  this->_query_abort = true;

  for(size_t i = 0; i < this->_repository_list.size(); ++i) {
    this->_repository_list[i]->abortQuery();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::queueQueries(const OmPNetRepoArray& selection, Om_beginCb begin_cb, Om_resultCb result_cb, void* user_ptr)
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

    // reset global progress
    this->_query_dones = 0;
    this->_query_percent = 0;

  } else {
    // emit a warning in case a crazy client starts new download with
    // different parameters than current
    if(this->_query_begin_cb != begin_cb || this->_query_result_cb != result_cb || this->_query_user_ptr != user_ptr) {
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
    this->_query_hth = Om_createThread(OmModChan::_query_run_fn, this);
    this->_query_hwo = Om_waitForThread(this->_query_hth, OmModChan::_query_end_fn, this);
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
        self->_xmlconf.child(L"network").children(repository_nodes, L"repository");

        for(size_t i = 0; i < repository_nodes.size(); ++i) {
          if(repository_nodes[i].attrAsString(L"base") == NetRepo->urlBase()) {
            if(repository_nodes[i].attrAsString(L"name") == NetRepo->urlName()) {
              repository_nodes[i].setAttr(L"title", NetRepo->title()); break;
            }
          }
        }

        self->_xmlconf.save();
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
  OmModChan* self = static_cast<OmModChan*>(ptr);

  #ifdef DEBUG
  std::wcout << "DEBUG => OmModChan::_query_end\n";
  #endif // DEBUG

  // unlock the network library
  self->_locked_net_library = false;

  //DWORD exit_code = Om_threadExitCode(self->_query_hth);
  Om_clearThread(self->_query_hth, self->_query_hwo);

  self->_query_hth = nullptr;
  self->_query_hwo = nullptr;

  self->_query_dones = 0;
  self->_query_percent = 0;

  self->_query_begin_cb = nullptr;
  self->_query_result_cb = nullptr;
  self->_query_user_ptr = nullptr;

  self->_query_queue.clear();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setLibraryDevmod(bool enable)
{
  this->_library_devmode = enable;

  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"library_devmode")) {
      this->_xmlconf.child(L"library_devmode").setAttr(L"enable", this->_library_devmode ? 1 : 0);
    } else {
      this->_xmlconf.addChild(L"library_devmode").setAttr(L"enable", this->_library_devmode ? 1 : 0);
    }

    this->_xmlconf.save();
  }

  // refresh all library for all locations
  this->clearModLibrary();
  this->refreshModLibrary();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setLibraryShowhidden(bool enable)
{
  this->_library_showhidden = enable;

  if(this->_xmlconf.valid()) {

    if(this->_xmlconf.hasChild(L"library_showhidden")) {
      this->_xmlconf.child(L"library_showhidden").setAttr(L"enable", this->_library_showhidden ? 1 : 0);
    } else {
      this->_xmlconf.addChild(L"library_showhidden").setAttr(L"enable", this->_library_showhidden ? 1 : 0);
    }

    this->_xmlconf.save();
  }

  // refresh all library for all locations
  this->clearModLibrary();
  this->refreshModLibrary();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnOverlaps(bool enable)
{
  this->_warn_overlaps = enable;

  if(this->_xmlconf.valid()) {

    OmXmlNode warn_options_node;

    if(this->_xmlconf.hasChild(L"warn_options")) {
      warn_options_node = this->_xmlconf.child(L"warn_options");
    } else {
      warn_options_node = this->_xmlconf.addChild(L"warn_options");
    }

    if(warn_options_node.hasChild(L"warn_overlaps")) {
      warn_options_node.child(L"warn_overlaps").setAttr(L"enable", this->_warn_overlaps ? 1 : 0);
    } else {
      warn_options_node.addChild(L"warn_overlaps").setAttr(L"enable", this->_warn_overlaps ? 1 : 0);
    }

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnExtraInst(bool enable)
{
  this->_warn_extra_inst = enable;

  if(this->_xmlconf.valid()) {

    OmXmlNode warn_options_node;

    if(this->_xmlconf.hasChild(L"warn_options")) {
      warn_options_node = this->_xmlconf.child(L"warn_options");
    } else {
      warn_options_node = this->_xmlconf.addChild(L"warn_options");
    }

    if(warn_options_node.hasChild(L"warn_extra_inst")) {
      warn_options_node.child(L"warn_extra_inst").setAttr(L"enable", this->_warn_extra_inst ? 1 : 0);
    } else {
      warn_options_node.addChild(L"warn_extra_inst").setAttr(L"enable", this->_warn_extra_inst ? 1 : 0);
    }

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnMissDeps(bool enable)
{
  this->_warn_miss_deps = enable;

  if(this->_xmlconf.valid()) {

    OmXmlNode warn_options_node;

    if(this->_xmlconf.hasChild(L"warn_options")) {
      warn_options_node = this->_xmlconf.child(L"warn_options");
    } else {
      warn_options_node = this->_xmlconf.addChild(L"warn_options");
    }

    if(warn_options_node.hasChild(L"warn_miss_deps")) {
      warn_options_node.child(L"warn_miss_deps").setAttr(L"enable", this->_warn_miss_deps ? 1 : 0);
    } else {
      warn_options_node.addChild(L"warn_miss_deps").setAttr(L"enable", this->_warn_miss_deps ? 1 : 0);
    }

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnExtraUnin(bool enable)
{
  this->_warn_extra_unin = enable;

  if(this->_xmlconf.valid()) {

    OmXmlNode warn_options_node;

    if(this->_xmlconf.hasChild(L"warn_options")) {
      warn_options_node = this->_xmlconf.child(L"warn_options");
    } else {
      warn_options_node = this->_xmlconf.addChild(L"warn_options");
    }

    if(warn_options_node.hasChild(L"warn_extra_unin")) {
      warn_options_node.child(L"warn_extra_unin").setAttr(L"enable", this->_warn_extra_unin ? 1 : 0);
    } else {
      warn_options_node.addChild(L"warn_extra_unin").setAttr(L"enable", this->_warn_extra_unin ? 1 : 0);
    }

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::targetDirAccess(bool rw)
{
  bool access_ok = true;

  // checks whether folder exists
  if(Om_isDir(this->_target_path)) {

    // checks for proper permissions on folder
    if(Om_checkAccess(this->_target_path, OM_ACCESS_DIR_READ)) {
      if(rw) { //< check for writing access
        if(!Om_checkAccess(this->_target_path, OM_ACCESS_DIR_WRITE)) {
          this->_error(L"targetDirAccess", Om_errWriteAccess(L"Mod Target", this->_target_path));
          access_ok = false;
        }
      }
    } else {
      this->_error(L"targetDirAccess", Om_errReadAccess(L"Mod Target", this->_target_path));
      access_ok = false;
    }
  } else {
    this->_error(L"targetDirAccess", Om_errNotDir(L"Mod Target", this->_target_path));
    access_ok = false;
  }

  if(!access_ok) {

    // Check for special case of network folder
    if(Om_pathIsNetwork(this->_target_path)) {

      // the Om_checkAccess (AccessCheck) method does not properly handle
      // permissions for network share that are not public/guest allowed.

      // Proper permissions check would require to implement domain-user login
      // using LogonUser and ImpersonateLoggedOnUser functions to test against
      // the given network folder.

      // To avoid wrong permissions issues, we bypass check but emit a warning
      // in log

      this->_log(OM_LOG_WRN, L"targetDirAccess",
                L"Access denied ignored because \""+this->_target_path+L"\" is a "
                "network folder and permissions may not be properly evaluated; "
                "Please be aware of this in case of file write or read error.");
      return true;
    } else {
      return false;
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::libraryDirAccess(bool rw)
{
  bool access_ok = true;

  // checks whether folder exists
  if(Om_isDir(this->_library_path)) {
    // checks for proper permissions on folder
    if(Om_checkAccess(this->_library_path, OM_ACCESS_DIR_READ)) {
      if(rw) { //< check for writing access
        if(!Om_checkAccess(this->_library_path, OM_ACCESS_DIR_WRITE)) {
          this->_error(L"libraryDirAccess", Om_errWriteAccess(L"Mod Library", this->_library_path));
          access_ok = false;
        }
      }
    } else {
      this->_error(L"libraryDirAccess", Om_errReadAccess(L"Mod Library", this->_library_path));
      access_ok = false;
    }
  } else {
    if(this->_library_path_is_cust) {
      this->_error(L"libraryDirAccess", Om_errNotDir(L"Mod Library", this->_library_path));
      access_ok = false;
    } else {
      // try to create it
      int result = Om_dirCreate(this->_library_path);
      if(result != 0) {
        this->_error(L"libraryDirAccess", Om_errCreate(L"Mod Library", this->_library_path, result));
        access_ok = false;
      }
    }
  }

  if(!access_ok) {

    // Check for special case of network folder
    if(Om_pathIsNetwork(this->_library_path.c_str())) {

      // the Om_checkAccess (AccessCheck) method does not properly handle
      // permissions for network share that are not public/guest allowed.

      // Proper permissions check would require to implement domain-user login
      // using LogonUser and ImpersonateLoggedOnUser functions to test against
      // the given network folder.

      // To avoid wrong permissions issues, we bypass check but emit a warning
      // in log

      this->_log(OM_LOG_WRN, L"libraryDirAccess",
                L"Access denied ignored because \""+this->_library_path+L"\" is a "
                "network folder and permissions may not be properly evaluated; "
                "Please be aware of this in case of file write or read error.");
      return true;
    } else {
      return false;
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::backupDirAccess(bool rw)
{
  bool access_ok = true;

  // checks whether folder exists
  if(Om_isDir(this->_backup_path)) {
    // checks for proper permissions on folder
    if(Om_checkAccess(this->_backup_path, OM_ACCESS_DIR_READ)) {
      if(rw) { //< check for writing access
        if(!Om_checkAccess(this->_backup_path, OM_ACCESS_DIR_WRITE)) {
          this->_error(L"backupDirAccess", Om_errWriteAccess(L"Mod Library", this->_backup_path));
          access_ok = false;
        }
      }
    } else {
      this->_error(L"backupDirAccess", Om_errReadAccess(L"Mod Library", this->_backup_path));
      access_ok = false;
    }
  } else {
    if(this->_backup_path_is_cust) {
      this->_error(L"backupDirAccess", Om_errNotDir(L"Mod Library", this->_backup_path));
      access_ok = false;
    } else {
      // try to create it
      int result = Om_dirCreate(this->_backup_path);
      if(result != 0) {
        this->_error(L"backupDirAccess", Om_errCreate(L"Mod Library", this->_backup_path, result));
        access_ok = false;
      }
    }
  }

  if(!access_ok) {

    // Check for special case of network folder
    if(Om_pathIsNetwork(this->_backup_path.c_str())) {

      // the Om_checkAccess (AccessCheck) method does not properly handle
      // permissions for network share that are not public/guest allowed.

      // Proper permissions check would require to implement domain-user login
      // using LogonUser and ImpersonateLoggedOnUser functions to test against
      // the given network folder.

      // To avoid wrong permissions issues, we bypass check but emit a warning
      // in log

      this->_log(OM_LOG_WRN, L"backupDirAccess",
                L"Access denied ignored because \""+this->_backup_path+L"\" is a "
                "network folder and permissions may not be properly evaluated; "
                "Please be aware of this in case of file write or read error.");
      return true;
    } else {
      return false;
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setTitle(const OmWString& title)
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
void OmModChan::setIndex(unsigned index)
{
  if(this->_xmlconf.valid()) {

    this->_index = index;

    if(this->_xmlconf.hasChild(L"title")) {
      this->_xmlconf.child(L"title").setAttr(L"index", static_cast<int>(index));
    }

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setDstDir(const OmWString& path)
{
  if(this->_xmlconf.valid()) {

    this->_target_path = path;

    if(this->_xmlconf.hasChild(L"install")) {
      this->_xmlconf.child(L"install").setContent(path);
    } else {
      this->_xmlconf.addChild(L"install").setContent(path);
    }

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setCustomLibraryDir(const OmWString& path)
{
  if(this->_xmlconf.valid()) {

    this->_library_path = path;
    // notify we use a custom Library path
    this->_library_path_is_cust = true;

    if(this->_xmlconf.hasChild(L"library")) {
      this->_xmlconf.child(L"library").setContent(path);
    } else {
      this->_xmlconf.addChild(L"library").setContent(path);
    }

    this->_xmlconf.save();
  }

  this->clearModLibrary();
  this->refreshModLibrary();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setDefaultLibraryDir()
{
  if(this->_xmlconf.valid()) {

    this->_library_path = this->_home + OM_MODCHAN_MODLIB_DIR;
    // notify we use default settings
    this->_library_path_is_cust = false;

    if(this->_xmlconf.hasChild(L"library"))
      this->_xmlconf.remChild(L"library");

    this->_xmlconf.save();
  }

  this->clearModLibrary();
  this->refreshModLibrary();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::setCustomBackupDir(const OmWString& path)
{
  if(this->_xmlconf.valid()) {

    bool has_error = false;

    // clear the mod library
    this->clearModLibrary();

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
          this->_error(L"setCustomBackupDir", Om_errMove(L"Backup element", src_path, result));
          has_error = true;
        }
      }
    }

    // change backup path
    this->_backup_path = path;

    // notify we use a custom Library path
    this->_backup_path_is_cust = true;

    if(this->_xmlconf.hasChild(L"backup")) {
      this->_xmlconf.child(L"backup").setContent(path);
    } else {
      this->_xmlconf.addChild(L"backup").setContent(path);
    }

    this->_xmlconf.save();

    // rebuild Mod library
    this->refreshModLibrary();

    return has_error;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModChan::setDefaultBackupDir()
{
  if(this->_xmlconf.valid()) {

    bool has_error = false;

    this->clearModLibrary();

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
          this->_error(L"setCustomBackupDir", Om_errMove(L"Backup element", src_path, result));
          has_error = true;
        }
      }
    }

    this->_backup_path = default_path;

    // notify we use default settings
    this->_backup_path_is_cust = false;

    if(this->_xmlconf.hasChild(L"backup"))
      this->_xmlconf.remChild(L"backup");

    this->_xmlconf.save();

    this->refreshModLibrary();

    return has_error;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setBackupComp(int32_t method, int32_t level)
{
  if(this->_xmlconf.valid()) {

    this->_backup_comp_method = method;
    this->_backup_comp_level = level;

    OmXmlNode backup_comp_node;

    if(this->_xmlconf.hasChild(L"backup_comp")) {
      backup_comp_node = this->_xmlconf.child(L"backup_comp");
    } else {
      backup_comp_node = this->_xmlconf.addChild(L"backup_comp");
    }

    backup_comp_node.setAttr(L"method", (int)method);
    backup_comp_node.setAttr(L"level", (int)level);

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setUpgdRename(bool enable)
{
  if(this->_xmlconf.valid()) {

    this->_upgd_rename = enable;

    OmXmlNode network_node;

    if(!this->_xmlconf.hasChild(L"network")) {
      network_node = this->_xmlconf.addChild(L"network");
    } else {
      network_node = this->_xmlconf.child(L"network");
    }

    network_node.setAttr(L"upgd_rename", static_cast<int>(enable ? 1 : 0));

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnExtraDnld(bool enable)
{
  this->_warn_extra_dnld = enable;

  if(this->_xmlconf.valid()) {

    OmXmlNode network_node;

    if(this->_xmlconf.hasChild(L"network")) {
      network_node = this->_xmlconf.child(L"network");
    } else {
      network_node = this->_xmlconf.addChild(L"network");
    }

    if(network_node.hasChild(L"warn_extra_dnld")) {
      network_node.child(L"warn_extra_dnld").setAttr(L"enable", this->_warn_extra_dnld ? 1 : 0);
    } else {
      network_node.addChild(L"warn_extra_dnld").setAttr(L"enable", this->_warn_extra_dnld ? 1 : 0);
    }

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnMissDnld(bool enable)
{
  this->_warn_miss_dnld = enable;

  if(this->_xmlconf.valid()) {

    OmXmlNode network_node;

    if(this->_xmlconf.hasChild(L"network")) {
      network_node = this->_xmlconf.child(L"network");
    } else {
      network_node = this->_xmlconf.addChild(L"network");
    }

    if(network_node.hasChild(L"warn_miss_dnld")) {
      network_node.child(L"warn_miss_dnld").setAttr(L"enable", this->_warn_miss_dnld ? 1 : 0);
    } else {
      network_node.addChild(L"warn_miss_dnld").setAttr(L"enable", this->_warn_miss_dnld ? 1 : 0);
    }

    this->_xmlconf.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModChan::setWarnUpgdBrkDeps(bool enable)
{
  this->_warn_upgd_brk_deps = enable;

  if(this->_xmlconf.valid()) {

    OmXmlNode network_node;

    if(this->_xmlconf.hasChild(L"network")) {
      network_node = this->_xmlconf.child(L"network");
    } else {
      network_node = this->_xmlconf.addChild(L"network");
    }

    if(network_node.hasChild(L"warn_upgd_brk_deps")) {
      network_node.child(L"warn_upgd_brk_deps").setAttr(L"enable", this->_warn_upgd_brk_deps ? 1 : 0);
    } else {
      network_node.addChild(L"warn_upgd_brk_deps").setAttr(L"enable", this->_warn_upgd_brk_deps ? 1 : 0);
    }

    this->_xmlconf.save();
  }
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
