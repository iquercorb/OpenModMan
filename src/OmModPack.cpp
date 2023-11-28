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

#include "OmArchive.h"
#include "OmXmlConf.h"

#include "OmUtilFs.h"
#include "OmUtilStr.h"
#include "OmUtilErr.h"
#include "OmUtilHsh.h"
#include "OmUtilPkg.h"
#include <ctime>

#include "OmModChan.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmModPack.h"

#define BACKUP_DATA_ROOT_DIR    L"root"
#define SAVEAS_THUMBN_NAME      L"thumbnail.png"
#define SAVEAS_README_NAME      L"readme.md"
#define SAVEAS_MODDEF_NAME      L"modpack.xml"

/// \brief Get package folder tree.
///
/// Get the full list, in recursive way, of items (subfolders and files)
/// contained in the specified folder.
///
/// \param[in]  ent_ls  : Pointer to Mod Entry vector object to be filled
/// \param[in]  orig    : Path to where to begin the inspection.
///
/// \return The filled buffer as const char
///
static void __parse_source_dir(OmModEntryArray* ent_ls, const OmWString& orig, const OmWString& from)
{
  OmWString item;
  OmWString root;

  OmModEntry_t entry;
  entry.cdid = -1;

  OmWString srch(orig);
  srch += L"\\*";

  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

      item = from + L"\\"; item += fd.cFileName;

      entry.path = item;

      if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        entry.attr = OM_MODENTRY_DIR;
        ent_ls->push_back(entry);
        // go deep in tree
        root = orig + L"\\"; root += fd.cFileName;
        __parse_source_dir(ent_ls, root, item);
      } else {
        entry.attr = 0;
        ent_ls->push_back(entry);
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPack::OmModPack() :
  _ModChan(nullptr),
  _hash(0),
  _has_src(false),
  _src_isdir(false),
  _has_bck(false),
  _bck_isdir(false),
  _error_backup(false),
  _error_restore(false),
  _error_apply(false),
  _op_backup(false),
  _op_restore(false),
  _op_apply(false),
  _op_progress(0)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPack::OmModPack(OmModChan* ModChan) :
  _ModChan(ModChan),
  _hash(0),
  _has_src(false),
  _src_isdir(false),
  _has_bck(false),
  _bck_isdir(false),
  _error_backup(false),
  _error_restore(false),
  _error_apply(false),
  _op_backup(false),
  _op_restore(false),
  _op_apply(false),
  _op_progress(0)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPack::~OmModPack()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPack::clearAll()
{
  // Source and Backup sides
  this->clearSource();
  this->clearBackup();

  // General properties
  this->_iden.clear();
  this->_hash = 0;
  this->_core.clear();
  this->_name.clear();
  this->_version.clear();

  this->_error_backup = false;
  this->_error_restore = false;
  this->_error_apply = false;
  this->_op_backup = false;
  this->_op_restore = false;
  this->_op_apply = false;
  this->_op_progress = 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPack::clearAnalytics()
{
  this->_error_backup = false;
  this->_error_restore = false;
  this->_error_apply = false;
  this->_op_backup = false;
  this->_op_restore = false;
  this->_op_apply = false;
  this->_op_progress = 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPack::clearSource()
{
  // Package source properties
  this->_has_src = false;
  this->_src_time = 0;
  this->_src_path.clear();
  this->_src_isdir = false;
  this->_src_root.clear();
  this->_src_entry.clear();
  this->_src_depend.clear();

  // Optional properties liked to source
  this->_category.clear();
  this->_description.clear();
  this->_description_time = 0;
  this->_thumbnail.clear();
  this->_thumbnail_time = 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::parseSource(const OmWString& path)
{
  this->clearSource();

  bool isdir = false;

  OmWString src_root, src_iden;

  if(Om_isDir(path)) {

    // Parse legacy/development Mod Package directory

    isdir = true;

    __parse_source_dir(&this->_src_entry, path, L"");

    src_root = path;

    src_iden = Om_getFilePart(path);

    this->loadDirDescription();

    this->loadDirThumbnail();

  } else if(Om_isFileZip(path)) {

    // Parse compressed file Mod Package

    OmArchive source_zip;

    if(!source_zip.read(path)) {
      this->_error(L"parseSource", Om_errLoad(L"archive file", path, source_zip.lastErrorStr()));
      return false;
    }

    // Try to find modern Mod Package XML definition file
    OmXmlConf source_cfg;

    int32_t zcd_idx;
    OmWString zcd_path;

    // Search in backward since XML definition file is usually the last added
    // in zip archive so appear at the end of Central Directory.
    unsigned i = source_zip.entryCount();
    while(i--) {

      source_zip.entryPath(i, zcd_path);

      if(Om_extensionMatches(zcd_path, OM_PKG_DEF_FILE_EXT) || Om_namesMatches(zcd_path, L"ModPack.xml")) {

        uint64_t data_len = source_zip.entrySize(i);

        char* data_buf = new(std::nothrow) char[data_len+1];
        if(!data_buf) {
          this->_error(L"parseSource", Om_errBadAlloc(L"definition file extraction", zcd_path));
          return false;
        }

        if(!source_zip.entrySave(i, data_buf)) {
          this->_error(L"parseSource", Om_errZipExtr(L"definition file", zcd_path, source_zip.lastErrorStr()));
          delete [] data_buf; return false;
        }

        data_buf[data_len] = '\0';

        if(!source_cfg.parse(Om_toUTF16(data_buf), OM_XMAGIC_PKG)) {
          this->_error(L"parseSource", Om_errParse(L"definition file", zcd_path, source_cfg.lastErrorStr()));
          delete [] data_buf; return false;
        }

        delete [] data_buf;

        break;
      }
    }

    // Either we found XML definition file, or we try to parse this Package the old fashion
    if(!source_cfg.empty()) {

      src_root = source_cfg.child(L"install").content();

      // search for <dependencies>
      if(source_cfg.hasChild(L"dependencies")) {

        OmXmlNodeArray xml_iden_ls;

        source_cfg.child(L"dependencies").children(xml_iden_ls, L"ident");

        for(size_t i = 0; i < xml_iden_ls.size(); ++i)
          this->_src_depend.push_back(xml_iden_ls[i].content());
      }

      // search for <category>
      if(source_cfg.hasChild(L"category")) {
        this->_category = source_cfg.child(L"category").content();
      }

      // search for <description>
      if(source_cfg.hasChild(L"description")) {
        Om_toCRLF(&this->_description, source_cfg.child(L"description").content());
      }

      // search for <picture>
      if(source_cfg.hasChild(L"picture")) {

        zcd_path = source_cfg.child(L"picture").content();

        zcd_idx = source_zip.entryLocate(zcd_path);

        if(zcd_idx >= 0) {

          uint64_t data_len = source_zip.entrySize(zcd_idx);

          uint8_t* data_buf = new(std::nothrow) uint8_t[data_len];
          if(data_buf) {

            if(source_zip.entrySave(zcd_idx, data_buf)) {
              if(!this->_thumbnail.loadThumbnail(data_buf, data_len, OM_MODPACK_THUMB_SIZE, OM_SIZE_FILL)) {
                this->_log(OM_LOG_WRN, L"parseSource", L"thumbnail image: "+this->_thumbnail.lastErrorStr());
              }
            } else {
              this->_log(OM_LOG_WRN, L"parseSource", Om_errZipExtr(L"thumbnail image",zcd_path,source_zip.lastErrorStr()));
            }
            delete [] data_buf;
          } else {
            this->_log(OM_LOG_WRN, L"parseSource", Om_errBadAlloc(L"thumbnail image extraction",zcd_path));
          }
        }
      }

    } else {

      // We are here with a zip file without Mod Pack XML definition, we try
      // to parse it the old fashion, we search for data root directory named
      // exactly the same as file name.

      bool has_root = false;

      src_root = Om_getNamePart(path);

      for(size_t i = 0; i < source_zip.entryCount(); ++i) {

        source_zip.entryPath(i, zcd_path);

        if(Om_isRootOfPath(src_root, zcd_path)) {
          has_root = true; break;
        }
      }

      if(!has_root) {
        this->_error(L"parseSource", L"\""+path+L"\": unknown or wrong Mod Pack architecture.");
        return false;
      }

      // search for a readme file to use as description
      zcd_idx = source_zip.entryLocate(L"readme.txt");
      if(zcd_idx >= 0) {

        uint64_t data_len = source_zip.entrySize(zcd_idx);

        char* data_buf = new(std::nothrow) char[data_len+1];
        if(data_buf) {
          if(source_zip.entrySave(zcd_idx, data_buf)) {
            data_buf[data_len] = '\0';
            this->_description = Om_toUTF16(data_buf);
          } else {
            this->_log(OM_LOG_WRN, L"parseSource", Om_errZipExtr(L"readme file", L"readme.txt", source_zip.lastErrorStr()));
          }
          delete [] data_buf;
        } else {
          this->_log(OM_LOG_WRN, L"parseSource", Om_errBadAlloc(L"readme file extraction", L"readme.txt"));
        }
      }
    }

    // Mod Pack appear valid, now we gather all Mod entries
    for(size_t i = 0; i < source_zip.entryCount(); ++i) {

      source_zip.entryPath(i, zcd_path);

      OmModEntry_t entry;

      if(Om_getRelativePath(&entry.path, src_root, zcd_path)) {

        entry.cdid = i;

        if(source_zip.entryIsDir(i)) {
          entry.attr = OM_MODENTRY_DIR;
        } else {
          entry.attr = 0;
        }



        this->_src_entry.push_back(entry);
      }
    }

    src_iden = Om_getNamePart(path);

  } else {
    this->_error(L"parseSource", L"unknown or wrong file type \""+path+L"\"");
    return false;
  }

  uint64_t src_hash = Om_getXXHash3(Om_getFilePart(path));

  // ultimately check against already parsed data from the Source
  if(this->_has_bck) {

    if(src_hash != this->_hash || this->_iden != src_iden) {
      this->_error(L"parseSource", L"parsed Source mismatch already parsed Backup \""+path+L"\"");
      return false;
    }

  } else {

    this->_hash = src_hash;

    this->_iden = src_iden;

    // parse other Mod common infos from identity
    OmWString vers_str;
    if(Om_parseModIdent(this->_iden, &this->_core, &vers_str, &this->_name))
      this->_version.parse(vers_str);
  }

  // All appear correct, now finalizing Mod Package setup
  this->_src_path = path;

  this->_src_isdir = isdir;

  this->_src_root = src_root;

  this->_src_home = Om_getDirPart(path);

  this->_src_time = Om_itemTime(path);

  this->_has_src = true;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::refreshSource()
{
  if(this->_has_src) {

    if(this->_src_isdir) {
      // we cannot rely on time for directory changes, we refresh in all cases
      this->parseSource(OmWString(this->_src_path));
      return true;
    } else {
      if(this->_src_time != Om_itemTime(this->_src_path))
        return this->parseSource(OmWString(this->_src_path));
    }
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::loadDirDescription()
{
  // build bas file path prototypes to search
  OmWString found_path, test_path[2];

  Om_concatPaths(test_path[0], this->_src_home, this->_iden);
  Om_concatPaths(test_path[1], this->_src_home, this->_core);

  bool found, changed = false;

  found = false;
  // Try text file with same identity, then core name
  for(unsigned i = 0; i < 2; ++i) {
    if(Om_isFile(found_path = test_path[i] + L".txt")) {
      found = true; break;
    }
  }

  if(found) {
    time_t new_time = Om_itemTime(found_path);
    if(new_time != this->_description_time) {
      Om_loadToUTF16(&this->_description, found_path);
      this->_description_time = new_time;
      changed = true;
    }
  } else {
    this->_description_time = 0;
    this->_description.clear();
  }

  return changed;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::loadDirThumbnail()
{
  // build bas file path prototypes to search
  OmWString found_path, test_path[2];

  Om_concatPaths(test_path[0], this->_src_home, this->_iden);
  Om_concatPaths(test_path[1], this->_src_home, this->_core);

  bool found, changed = false;

  // supported image extensions
  const wchar_t* img_ext[] = {L".jpg", L".png", L".gif", L".bmp"};

  found = false;
  // try image file with same identity, then core name
  for(unsigned i = 0; i < 4; ++i) {
    if(Om_isFile(found_path = test_path[0] + img_ext[i])) {
      found = true; break;
    }
    if(Om_isFile(found_path = test_path[1] + img_ext[i])) {
      found = true; break;
    }
  }

  if(found) {
    time_t new_time = Om_itemTime(found_path);
    if(new_time != this->_thumbnail_time) {
      this->_thumbnail.loadThumbnail(found_path, OM_MODPACK_THUMB_SIZE, OM_SIZE_FILL);
      this->_thumbnail_time = new_time;
      changed = true;
    }
  } else {
    this->_thumbnail_time = 0;
    this->_thumbnail.clear();
  }

  return changed;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPack::setThumbnail(const OmWString& path)
{
  this->_thumbnail.loadThumbnail(path, OM_MODPACK_THUMB_SIZE, OM_SIZE_FILL);
  this->_thumbnail_time = 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPack::clearThumbnail()
{
  this->_thumbnail.clear();
  this->_thumbnail_time = 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::hasDepend(const OmWString& ident) const
{
  // you don't like raw loops ? I LOVE row loops...
  for(size_t i = 0; i < this->_src_depend.size(); ++i)
    if(this->_src_depend[i] == ident)
      return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPack::clearBackup()
{
 this->_has_bck = false;
 this->_bck_path.clear();
 this->_bck_isdir = false;
 this->_bck_root.clear();
 this->_bck_entry.clear();
 this->_bck_overlap.clear();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::parseBackup(const OmWString& path)
{
  this->clearBackup();

  bool isdir = false;

  OmWString bck_root;

  OmXmlConf backup_cfg;

  if(Om_isDir(path)) {

    isdir = true;

    // Parse uncompressed backup raw directory
    OmWStringArray files_ls;
    Om_lsFile(&files_ls, path, true);

    for(size_t i = 0; i < files_ls.size(); ++i) {

      if(Om_extensionMatches(files_ls[i], OM_BCK_DEF_FILE_EXT) || Om_namesMatches(files_ls[i], L"ModBack.xml")) {

        if(!backup_cfg.load(files_ls[i], OM_XMAGIC_BCK)) {
          this->_error(L"parseBackup", Om_errZipExtr(L"definition file", files_ls[i], backup_cfg.lastErrorStr()));
          return false;
        }

        break;
      }
    }

    bck_root = path;

  } else if(Om_isFileZip(path)) {

    // Parse compressed file backup

    OmArchive backup_zip;

    // Open zip file
    if(!backup_zip.read(path)) {
      this->_error(L"parseBackup", Om_errLoad(L"archive file", path, backup_zip.lastErrorStr()));
      return false;
    }

    // Search in backward since XML definition file is usually the last added
    // in zip archive so appear at the end of Central Directory.
    unsigned i = backup_zip.entryCount();
    while(i--) {

      OmWString zcd_path;
      backup_zip.entryPath(i, zcd_path);

      if(Om_extensionMatches(zcd_path, OM_BCK_DEF_FILE_EXT) || Om_namesMatches(zcd_path, L"ModBack.xml")) {

        uint64_t data_len = backup_zip.entrySize(i);

        char* data_buf = new(std::nothrow) char[data_len+1];
        if(!data_buf) {
          this->_error(L"parseBackup", Om_errBadAlloc(L"definition file extraction", zcd_path));
          return false;
        }

        if(!backup_zip.entrySave(i, data_buf)) {
          delete [] data_buf;
          this->_error(L"parseBackup", Om_errZipExtr(L"definition file", zcd_path, backup_zip.lastErrorStr()));
          return false;
        }

        data_buf[data_len] = '\0';

        if(!backup_cfg.parse(Om_toUTF16(data_buf), OM_XMAGIC_BCK)) {
          delete [] data_buf;
          this->_error(L"parseBackup", Om_errParse(L"definition file", zcd_path, backup_cfg.lastErrorStr()));
          return false;
        }

        delete [] data_buf;

        break;
      }
    }
  } else {

    // No valid backup data here
    this->_error(L"parseBackup", L"unknown or wrong file type \""+path+L"\"");
    return false;
  }

  OmWString bck_iden;
  uint64_t bck_hash;

  if(!backup_cfg.empty()) {

    // verify the Backup definition have the proper nodes
    if(!backup_cfg.hasChild(L"backup") || !backup_cfg.hasChild(L"ident") || !backup_cfg.hasChild(L"hash")) {
      this->_error(L"parseBackup", L"definition file is missing essential nodes");
      return false;
    }

    bck_root = backup_cfg.child(L"backup").content();

    bck_iden = backup_cfg.child(L"ident").content();

    bck_hash = Om_strToUint64(backup_cfg.child(L"hash").content());


    OmXmlNodeArray xml_node_ls;

    // get list of installed files, they are listed in the
    // backup definition, either as <cpy> or <del> entries.
    backup_cfg.children(xml_node_ls, L"cpy");

    for(size_t i = 0; i < xml_node_ls.size(); ++i) {

      OmModEntry_t entry;
      entry.cdid = xml_node_ls[i].attrAsInt(L"cdi");
      entry.attr = (xml_node_ls[i].attrAsInt(L"dir") > 0) ? OM_MODENTRY_DIR : 0;
      entry.path = xml_node_ls[i].content();

      this->_bck_entry.push_back(entry);
    }

    backup_cfg.children(xml_node_ls, L"del");

    for(size_t i = 0; i < xml_node_ls.size(); ++i) {

      OmModEntry_t entry;
      entry.cdid = -1;
      entry.attr = OM_MODENTRY_DEL | (xml_node_ls[i].attrAsInt(L"dir") > 0) ? OM_MODENTRY_DIR : 0;
      entry.path = xml_node_ls[i].content();

      this->_bck_entry.push_back(entry);
    }

    // retrieve the backup overlap list, they are stored as a list of Hash values
    // corresponding to package file name
    if(backup_cfg.hasChild(L"overlap")) {

      OmXmlNodeArray xml_hash_ls;
      backup_cfg.child(L"overlap").children(xml_hash_ls, L"hash");

      for(size_t i = 0; i < xml_hash_ls.size(); ++i)
        this->_bck_overlap.push_back(Om_strToUint64(xml_hash_ls[i].content()));
    }

  } else {
    this->_error(L"parseBackup", L"definition not found in location \""+path+L"\"");
    return false;
  }

  // ultimately check against already parsed data from the Source
  if(this->_has_src) {

    if(bck_hash != this->_hash || this->_iden != bck_iden) {

      this->_error(L"parseBackup", L"parsed Backup mismatch already parsed Source \""+path+L"\"");
      return false;
    }

  } else {

    this->_hash = bck_hash;

    this->_iden = bck_iden;

    // parse other Mod common infos from identity
    OmWString vers_str;
    if(Om_parseModIdent(this->_iden, &this->_core, &vers_str, &this->_name))
      this->_version.parse(vers_str);
  }

  // All appear correct, now finalizing Mod Backup setup
  this->_bck_path = path;

  this->_bck_isdir = isdir;

  //this->_bck_home = Om_getDirPart(path);

  this->_bck_root = bck_root;

  this->_has_bck = true;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::backupHasEntry(const OmWString& path, int32_t attr) const
{
  for(size_t i = 0; i < this->_bck_entry.size(); ++i) {
    if(this->_bck_entry[i].attr == attr)
      if(this->_bck_entry[i].path == path)
        return true;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPack::getFootprint(OmModEntryArray* footprint) const
{
  if(!this->_ModChan) {
    //this->_error(L"getFootprint", L"no Mod Channel");
    return;
  }

  OmModEntry_t entry;
  entry.cdid = -1;

  OmWString tgt_file;

  for(size_t i = 0; i < this->_src_entry.size(); ++i) {

    Om_concatPaths(tgt_file, this->_ModChan->targetPath(), this->_src_entry[i].path);

    entry.path = this->_src_entry[i].path;
    entry.attr = this->_src_entry[i].attr;

    if(!Om_pathExists(tgt_file))
      entry.attr |= OM_MODENTRY_DEL;

    footprint->push_back(entry);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::hasOverlap(uint64_t hash) const
{
  // you don't like raw loops ? I LOVE row loops...
  for(size_t i = 0; i < this->_bck_overlap.size(); ++i)
    if(this->_bck_overlap[i] == hash)
      return true;

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::canOverlap(const OmModPack* other) const
{
  // you don't like raw loops ? I LOVE row loops...
  for(size_t i = 0; i < this->_src_entry.size(); ++i) {

    if(OM_HAS_BIT(this->_src_entry[i].attr, OM_MODENTRY_DIR)) //< we don't care directories
      continue;

    for(size_t j = 0; j < other->_src_entry.size(); ++j) {

      if(OM_HAS_BIT(other->_src_entry[j].attr, OM_MODENTRY_DIR)) //< we don't care directories
        continue;

      // same path mean overlap
      if(this->_src_entry[i].path == other->_src_entry[j].path)
        return true;
    }
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::canOverlap(const OmModEntryArray& footprint) const
{
  // you don't like raw loops ? I LOVE row loops...
  for(size_t i = 0; i < this->_src_entry.size(); ++i) {

    if(OM_HAS_BIT(this->_src_entry[i].attr, OM_MODENTRY_DIR)) //< we don't care directories
      continue;

    for(size_t j = 0; j < footprint.size(); ++j) {

      if(OM_HAS_BIT(footprint[j].attr, OM_MODENTRY_DIR)) //< we don't care directories
        continue;

      // same path mean overlap
      if(this->_src_entry[i].path == footprint[j].path)
        return true;
    }
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModPack::makeBackup(Om_progressCb progress_cb, void* user_ptr)
{
  if(this->_has_bck)
    return OM_RESULT_ABORT;

  if(!this->_ModChan) {
    this->_error(L"makeBackup", L"no Mod Channel");
    this->_op_backup = false; this->_error_backup = true;
    return OM_RESULT_ABORT;
  }

  if(!this->_has_src) {
    this->_error(L"makeBackup", L"Source data missing");
    this->_op_backup = false; this->_error_backup = true;
    return OM_RESULT_ABORT;
  }

  // start backup operation
  this->_op_backup = true; this->_error_backup = false;

  // initialize chrono
  clock_t time = clock();

  // initialize progression callback
  size_t progress_tot, progress_cur;

  if(progress_cb) {
    progress_tot = this->_src_entry.size() * 2;   //< backup + install
    progress_cur = 0;
    this->_op_progress =((double)progress_cur / progress_tot) * 100;
    if(!progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(this))) {
      return OM_RESULT_ABORT;
    }
  }

  OmArchive backup_zip;

  bool isdir = (this->_ModChan->backupCompMethod() < 0);

  OmWString bck_root;

  OmWString bck_name = Om_getFilePart(this->_src_path);

  OmWString bck_path = this->_ModChan->backupPath() + L"\\";

  if(isdir) {

    bck_path += bck_name;

    bck_root = bck_path + L"\\" + BACKUP_DATA_ROOT_DIR;

    int32_t result = Om_dirCreateRecursive(bck_root);
    if(result != 0) {
      this->_error(L"makeBackup", Om_errCreate(L"initial Backup directories", bck_root, result));
      this->_op_backup = false; this->_error_backup = true;
      return OM_RESULT_ERROR;
    }

  } else {

    bck_name += L"." OM_BCK_FILE_EXT;
    bck_path += bck_name;

    bck_root = BACKUP_DATA_ROOT_DIR;

    // initialize zip archive
    if(!backup_zip.write(bck_path, this->_ModChan->backupCompMethod(), this->_ModChan->backupCompLevel())) {
      this->_error(L"makeBackup", Om_errInit(L"Backup archive file", bck_path, backup_zip.lastErrorStr()));
      this->_op_backup = false; this->_error_backup = true;
      return OM_RESULT_ERROR;
    }
  }

  // initialize backup XML config
  OmXmlConf backup_cfg(OM_XMAGIC_BCK);

  backup_cfg.addChild(L"ident").setContent(this->_iden);
  backup_cfg.addChild(L"hash").setContent(Om_uint64ToStr(this->_hash));
  backup_cfg.addChild(L"backup").setContent(BACKUP_DATA_ROOT_DIR);

  bool has_error = false;
  bool has_abort = false;

  OmWString tgt_file, bck_file;
  OmXmlNode bck_node;

  for(size_t i = 0, z = 0; i < this->_src_entry.size(); ++i) {

    OmModEntry_t entry;
    entry.path = this->_src_entry[i].path;
    entry.attr = this->_src_entry[i].attr;
    entry.cdid = -1; //< invalid zip central-directory index
    entry.attr = 0;

    Om_concatPaths(tgt_file, this->_ModChan->targetPath(), entry.path);
    Om_concatPaths(bck_file, bck_root, entry.path);

    if(!Om_pathExists(tgt_file)) {

      // file or directory does not exists in Target, this is a added/created file
      // by the Mod that must be deleted at uninstall
      entry.attr |= OM_MODENTRY_DEL;

      bck_node = backup_cfg.addChild(L"del");
      bck_node.setContent(entry.path);
      bck_node.setAttr(L"cdi", (int)entry.cdid);
      bck_node.setAttr(L"dir", OM_HAS_BIT(entry.attr, OM_MODENTRY_DIR) ? 1 : 0 );

      this->_bck_entry.push_back(entry);

    } else {

      if(OM_HAS_BIT(entry.attr, OM_MODENTRY_DIR)) {

        if(this->_ModChan->backupEntryExists(entry.path, entry.attr)) {

          // directory was created by another Mod, in this case we add it as to be
          // deleted by this one too so we can delete unused shared folders if empty.
          entry.attr |= OM_MODENTRY_DEL;

          bck_node = backup_cfg.addChild(L"del");
          bck_node.setContent(entry.path);
          bck_node.setAttr(L"cdi", (int)entry.cdid);
          bck_node.setAttr(L"dir", 1);

          this->_bck_entry.push_back(entry);
        }

        if(isdir) {

          int32_t result = Om_dirCreate(bck_file);
          if(result != 0) {
            this->_error(L"makeBackup", Om_errCreate(L"directory in Backup", bck_file, result));
            has_error = true; break;
          }
        }

      } else {

        if(isdir) {

          // create required directory tree before moving file
          OmWString bck_tree = Om_getDirPart(bck_file);

          if(!Om_isDir(bck_tree)) {
            int32_t result = Om_dirCreateRecursive(bck_tree);
            if(result != 0) {
              this->_error(L"makeBackup", Om_errCreate(L"tree in Backup", bck_tree, result));
              has_error = true; break;
            }
          }

          // move file, hopping nothing goes wrong
          int32_t result = Om_fileMove(tgt_file, bck_file); //< risky
          if(result != 0) {
            this->_error(L"makeBackup", Om_errMove(L"Backup from Target file", tgt_file, result));
            has_error = true; break;
          }

        } else {

          // set zip central-directory index
          entry.cdid = z;

          // add zip entry
          if(!backup_zip.entryAdd(tgt_file, bck_file)) {
            this->_error(L"makeBackup", Om_errZipComp(L"Backup from Target file", tgt_file, backup_zip.lastErrorStr()));
            backup_zip.close(); has_error = true; break;
          }

          z++; //< increment zip central-directory index
        }

        bck_node = backup_cfg.addChild(L"cpy");
        bck_node.setContent(entry.path);
        bck_node.setAttr(L"cdi", (int)entry.cdid);
        bck_node.setAttr(L"dir", 0);

        this->_bck_entry.push_back(entry);
      }
    }

    // call progression callback
    if(progress_cb) {
      progress_cur++;
      this->_op_progress = ((double)progress_cur / progress_tot) * 100;
      if(!progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(this))) {
        this->_log(OM_LOG_WRN, L"makeBackup", L"process aborted by user.");
        has_abort = true; break;
      }
    }

    #ifdef DEBUG
    Sleep(50); //< for debug
    #endif
  }

  // Required data for potential undo
  this->_bck_path = bck_path;

  this->_bck_isdir = isdir;

  this->_bck_root = bck_root;

  // process aborted, either by user or encountered error
  if(has_abort || has_error) {
    this->_op_backup = false; this->_error_backup = has_error;
    return has_error ? OM_RESULT_ERROR : OM_RESULT_ABORT;
  }

  // retrieve overlapped Mod list and add to XML config
  OmUint64Array bck_overlap;

  this->_ModChan->findOverlapped(this, &bck_overlap);

  if(bck_overlap.size()) {

    OmXmlNode xml_overlap = backup_cfg.addChild(L"overlap");

    for(size_t i = 0; i < bck_overlap.size(); ++i)
      xml_overlap.addChild(L"hash").setContent(Om_uint64ToStr(bck_overlap[i]));

    this->_bck_overlap.assign(bck_overlap.begin(), bck_overlap.end());
  }

  if(isdir) {

    OmWString cfg_path = bck_path + L"\\ModBack.xml";

    if(!backup_cfg.save(cfg_path)) {
      this->_error(L"makeBackup", Om_errSave(L"definition file", cfg_path, backup_cfg.lastErrorStr()));
      has_error = true;
    }

  } else {

    // get backup definition XML data
    OmCString xml_data = backup_cfg.data();
    if(!backup_zip.entryAdd(xml_data.c_str(), xml_data.size(), L"ModBack.xml")) {
      this->_error(L"makeBackup", Om_errZipComp(L"definition file", L"ModBack.xml", backup_zip.lastErrorStr()));
      backup_zip.close(); has_error = true;
    }

    // finalize zip archive
    backup_zip.close();
  }

  // here is data to be restored, completed or not
  this->_has_bck = true;

  // end backup operation
  this->_op_backup = false; this->_error_backup = has_error;

  if(has_error) {
    return OM_RESULT_ERROR; //< must undo
  }

  // making report
  wchar_t done_str[32];
  swprintf(done_str, 32, L"done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);
  this->_log(OM_LOG_OK, L"makeBackup", done_str);


  return OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModPack::restoreData(Om_progressCb progress_cb, void* user_ptr, bool isundo)
{
  if(!this->_ModChan) {
    this->_error(L"restoreData", L"no Mod Channel.");
    this->_op_restore = false; this->_error_restore = true;
    return OM_RESULT_ABORT;
  }

  if(!this->_has_bck) {
    if(!isundo) this->_error(L"restoreData", L"Backup data missing.");
    this->_op_restore = false; this->_error_restore = true;
    return OM_RESULT_ABORT;
  }

  // start restore operation
  this->_op_restore = true; this->_error_restore = false;

  // initialize chrono
  clock_t time = clock();

  // initialize progression callback
  size_t progress_tot, progress_cur;
  if(progress_cb) {
    if(isundo) {
      progress_cur = this->_bck_entry.size();
      progress_tot = this->_bck_entry.size() * 2; //< item we must restore
    } else {
      progress_cur = 0;
      progress_tot = this->_bck_entry.size(); //< item we must restore
    }
    this->_op_progress =((double)progress_cur / progress_tot) * 100;
    progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(this));
  }

  OmArchive backup_zip;

  // verify we have data to restore
  if(this->_bck_isdir) {
    if(!Om_isDir(this->_bck_root)) {
      this->_error(L"restoreData", Om_errNotDir(L"Backup root directory", this->_bck_root));
      this->_op_restore = false; this->_error_restore = true;
      return OM_RESULT_ERROR;
    }
  } else {
    if(!backup_zip.read(this->_bck_path)) {
      this->_error(L"restoreData", Om_errLoad(L"Backup archive file", this->_bck_path, backup_zip.lastErrorStr()));
      this->_op_restore = false; this->_error_restore = true;
      return OM_RESULT_ERROR;
    }
  }

  bool has_error = false;

  // restore original files from Backup to Target
  for(size_t i = 0; i < this->_bck_entry.size(); ++i) {

    if(OM_HAS_BIT(this->_bck_entry[i].attr, OM_MODENTRY_DEL))
      continue;

    OmWString tgt_file, bck_file;
    Om_concatPaths(tgt_file, this->_ModChan->targetPath(), this->_bck_entry[i].path);

    if(this->_bck_isdir) {

      Om_concatPaths(bck_file, this->_bck_root, this->_bck_entry[i].path);

      // move file from backup to target, overwriting existing
      int32_t result = Om_fileMove(bck_file, tgt_file);
      if(result != 0) {
        this->_error(L"restoreData", Om_errMove(L"Backup to Target file", tgt_file, result));
        has_error = true;
      }

    } else {

      // extract from backup archive to target, overwriting existing
      if(!backup_zip.entrySave(this->_bck_entry[i].cdid, tgt_file)) { //< TODO: des erreur d'index ici, le cdid est incohérent... data perdue ? mal parsé ?
        this->_error(L"restoreData", Om_errZipExtr(L"Backup to Target file", this->_bck_entry[i].path, backup_zip.lastErrorStr()));
        has_error = true;
      }
    }

    // call progression callback
    if(progress_cb) {
      if(isundo) progress_cur--; else progress_cur++;
      this->_op_progress = ((double)progress_cur / progress_tot) * 100;
      progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(this));
    }

    #ifdef DEBUG
    Sleep(50); //< for debug
    #endif
  }

  // close backup archive file
  if(!this->_bck_isdir) backup_zip.close();

  // delete added files and/or folders Mod may have created in Target
  //
  // the "To Delete" entries are listed according tree hierarchy in depth-first
  // order, so we walk list in backward to have the proper deletion sequence.

  size_t i = this->_bck_entry.size();
  while(i--) {

    if(!OM_HAS_BIT(this->_bck_entry[i].attr, OM_MODENTRY_DEL))
      continue;

    OmWString tgt_file;
    Om_concatPaths(tgt_file, this->_ModChan->targetPath(), this->_bck_entry[i].path);

    // if undo the file may not be installed yet, we prevent warnings
    if(isundo && !Om_pathExists(tgt_file))
      continue;

    if(OM_HAS_BIT(this->_bck_entry[i].attr, OM_MODENTRY_DIR)) {

      // delete folder only if empty
      if(Om_isDirEmpty(tgt_file)) {

        int32_t result = Om_dirDelete(tgt_file);
        if(result != 0) {
          this->_error(L"restoreData", Om_errDelete(L"directory in Target", tgt_file, result));
          has_error = true;
        }
      }

    } else {

      int32_t result = Om_fileDelete(tgt_file);
      if(result != 0) {
        this->_error(L"restoreData", Om_errDelete(L"file in Target", tgt_file, result));
        has_error = true;
      }
    }

    // call progression callback
    if(progress_cb) {
      if(isundo) progress_cur--; else progress_cur++;
      this->_op_progress = ((double)progress_cur / progress_tot) * 100;
      progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(this));
    }

    #ifdef DEBUG
    Sleep(50); //< for debug
    #endif
  }

  // cleanup backup data
  if(this->_bck_isdir) {

    int32_t result = Om_dirDeleteRecursive(this->_bck_path);
    if(result != 0) {
      this->_error(L"restoreData", Om_errDelete(L"Backup directories", this->_bck_path, result));
      has_error = true;
    }

  } else {

    int32_t result = Om_fileDelete(this->_bck_path);
    if(result != 0) {
      this->_error(L"restoreData", Om_errDelete(L"Backup archive file", this->_bck_path, result));
      has_error = true;
    }

  }

  // revoke and clean Backup side of this instance
  this->clearBackup();

  // making report
  wchar_t done_str[32];
  swprintf(done_str, 32, L"done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);
  this->_log(OM_LOG_OK, L"restoreData", done_str);

  // end restore operation
  this->_op_restore = false; this->_error_restore = has_error;

  return has_error ? OM_RESULT_ERROR : OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModPack::applySource(Om_progressCb progress_cb, void* user_ptr)
{
  if(!this->_ModChan) {
    this->_error(L"applySource", L"no Mod Channel.");
    this->_op_apply = false; this->_error_apply = true;
    return OM_RESULT_ABORT;
  }

  // start install operation
  this->_op_apply = true; this->_error_apply = false;

  // initialize chrono
  clock_t time = clock();

  // initialize progression callback
  size_t progress_tot, progress_cur;
  if(progress_cb) {
    progress_tot = this->_src_entry.size() * 2;   //< item we must install
    progress_cur = this->_src_entry.size();       //< start at half the total, backup was the first half
    this->_op_progress =((double)progress_cur / progress_tot) * 100;
    if(!progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(this))) {
      return OM_RESULT_ABORT;
    }
  }

  OmArchive source_zip;

  // verify we have source to install
  if(this->_src_isdir) {
    if(!Om_isDir(this->_src_root)) {
      this->_error(L"applySource", Om_errNotDir(L"Source directory", this->_src_root));
      this->_op_apply = false; this->_error_apply = true;
      return OM_RESULT_ERROR;
    }
  } else {
    if(!source_zip.read(this->_src_path)) {
      this->_error(L"applySource", Om_errLoad(L"Source archive file", this->_src_path, source_zip.lastErrorStr()));
      this->_op_apply = false; this->_error_apply = true;
      return OM_RESULT_ERROR;
    }
  }

  bool has_error = false;
  bool has_abort = false;

  OmWString tgt_file, src_file;

  for(size_t i = 0; i < this->_src_entry.size(); ++i) {

    Om_concatPaths(tgt_file, this->_ModChan->targetPath(), this->_src_entry[i].path);

    if(OM_HAS_BIT(this->_src_entry[i].attr, OM_MODENTRY_DIR)) {

      // if directory does not exists in Target, create it
      if(!Om_isDir(tgt_file)) {
        int32_t result = Om_dirCreate(tgt_file);
        if(result != 0) {
          this->_error(L"applySource", Om_errCreate(L"directory in Target", tgt_file, result));
          has_error = true; break;
        }
      }

    } else {

      if(this->_src_isdir) {

        Om_concatPaths(src_file, this->_src_root, this->_src_entry[i].path);

        // Copy and overwrite
        int32_t result = Om_fileCopy(src_file, tgt_file, true);
        if(result != 0) {
          this->_error(L"applySource", Om_errCopy(L"Source file to Target", tgt_file, result));
          has_error = true; break;
        }

      } else {

        // extract to destination
        if(!source_zip.entrySave(this->_src_entry[i].cdid, tgt_file)) {
          this->_error(L"applySource", Om_errZipExtr(L"Source file to Target", tgt_file, source_zip.lastErrorStr()));
          has_error = true; break;
        }

      }

    }

    // call progression callback
    if(progress_cb) {
      progress_cur++;
      this->_op_progress = ((double)progress_cur / progress_tot) * 100;
      if(!progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(this))) {
        this->_log(OM_LOG_WRN, L"applySource", L"process aborted by user.");
        has_abort = true; break;
      }
    }

    #ifdef DEBUG
    Sleep(50); //< for debug
    #endif
  }

  // close zip file
  if(!this->_src_isdir) source_zip.close();

  // end install operation
  this->_op_apply = false; this->_error_apply = has_error;

  if(has_abort || has_error) {
    return has_error ? OM_RESULT_ERROR : OM_RESULT_ABORT; //< must undo
  }

  // making report
  wchar_t done_str[32];
  swprintf(done_str, 32, L"done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);
  this->_log(OM_LOG_OK, L"applySource", done_str);

  return OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmModPack::discardBackup()
{

  // initialize local timer
  clock_t time = clock();

  bool has_error = false;

  // cleanup backup data either zip file or sub-directory...
  if(this->_bck_isdir) {

    int32_t result = Om_moveToTrash(this->_bck_path);
    if(result != 0) {
      this->_error(L"discardBackup", Om_errDelete(L"Backup data directory", this->_bck_path, result));
      has_error = true;
    }

  } else {

    int32_t result = Om_moveToTrash(this->_bck_path);
    if(result != 0) {
      this->_error(L"discardBackup", Om_errDelete(L"Backup archive file", this->_bck_path, result));
      has_error = true;
    }

  }

  // revoke the backup side of this package
  this->clearBackup();

  // making report
  wchar_t done_str[32];
  swprintf(done_str, 32, L"done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);
  this->_log(OM_LOG_OK, L"discardBackup", done_str);

  return has_error ? OM_RESULT_ERROR : OM_RESULT_OK;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPack::saveAs(const OmWString& path, int32_t method, int32_t level, Om_progressCb progress_cb, void* user_ptr)
{
  // initialize local timer
  clock_t time = clock();

  if(!this->_has_src) {
    this->_error(L"saveAs", L"Source missing");
    return false;
  }

  OmArchive source_zip;

  // verify we have source to save as...
  if(this->_src_isdir) {
    if(!Om_isDir(this->_src_root)) {
      this->_error(L"saveAs", Om_errNotDir(L"Source directory", this->_src_root));
      return false;
    }
  } else {
    if(!source_zip.read(this->_src_path)) {
      this->_error(L"saveAs", Om_errLoad(L"Source archive file", this->_src_path, source_zip.lastErrorStr()));
      return false;
    }
  }

  OmArchive output_zip;

  // create temporary file name
  OmWString tmp_path(path); tmp_path.append(L".zc_tmp");

  // open output archive for writing
  if(!output_zip.write(tmp_path, method, level)) {
    this->_error(L"saveAs", Om_errInit(L"Output archive file", tmp_path, output_zip.lastErrorStr()));
    return false;
  }

  // some required constants strings
  const OmWString out_root(Om_getNamePart(path));

  // initialize the progression values
  size_t entry_tot, entry_cur = 0;
  if(progress_cb) {
    entry_tot = this->_src_entry.size();
    progress_cb(user_ptr, entry_tot, entry_cur, reinterpret_cast<uint64_t>(this));
  }

  bool has_error = false;
  bool has_abort = false;

  OmWString out_file;

  // transfer data from source to output zip
  for(size_t i = 0; i < this->_src_entry.size(); ++i) {

    // call progression callback
    if(progress_cb) {
      entry_cur++;
      if(!progress_cb(user_ptr, entry_tot, entry_cur, reinterpret_cast<uint64_t>(this))) {
        has_abort = true; break;
      }
    }

    // output file path (in zip)
    Om_concatPaths(out_file, out_root, this->_src_entry[i].path);

    if(OM_HAS_BIT(this->_src_entry[i].attr, OM_MODENTRY_DIR)) {

      // add folder to destination archive
      if(!output_zip.entryAdd(nullptr, 0, out_file)) {
        this->_error(L"saveAs", Om_errZipComp(L"Source directory to destination", out_file, output_zip.lastErrorStr()));
        has_error = true; break;
      }

    } else {

      if(this->_src_isdir) {

        // source file path
        OmWString src_file;
        Om_concatPaths(src_file, this->_src_root, this->_src_entry[i].path);

        if(!output_zip.entryAdd(src_file, out_file, nullptr, user_ptr)) {
          this->_error(L"saveAs", Om_errZipComp(L"Source file to destination", src_file, output_zip.lastErrorStr()));
          has_error = true; break;
        }

      } else {

        //transfers data from source to destination via memory buffer
        uint64_t data_len = source_zip.entrySize(this->_src_entry[i].cdid);
        uint8_t* data_buf = new(std::nothrow) uint8_t[data_len];

        if(!data_buf) {
          this->_error(L"saveAs", Om_errBadAlloc(L"Source file extraction", this->_src_entry[i].path));
          has_error = true; break;
        }

        if(!source_zip.entrySave(this->_src_entry[i].cdid, data_buf, nullptr, user_ptr)) {
          this->_error(L"saveAs", Om_errZipExtr(L"Source file", this->_src_entry[i].path, source_zip.lastErrorStr()));
          delete [] data_buf; has_error = true; break;
        }

        if(!output_zip.entryAdd(data_buf, data_len, out_file, nullptr, user_ptr)) {
          this->_error(L"saveAs", Om_errZipComp(L"Destination file", out_file, output_zip.lastErrorStr()));
          delete [] data_buf; has_error = true; break;
        }

        delete [] data_buf;
      }

    }

    #ifdef DEBUG
    Sleep(50); //< for debug
    #endif
  }

  // we do not need source archive anymore
  source_zip.close();

  if(has_error || has_abort) {
    output_zip.close();
    Om_fileDelete(tmp_path);
    return !has_error;
  }

  // create the Mod source definition
  OmXmlConf output_cfg(OM_XMAGIC_PKG);

  // defines data source root directory
  output_cfg.addChild(L"install").setContent(out_root);

  // add source dependencies
  if(this->_src_depend.size()) {

    OmXmlNode dependencies_node = output_cfg.addChild(L"dependencies");

    for(size_t i = 0; i < this->_src_depend.size(); ++i)
      dependencies_node.addChild(L"ident").setContent(this->_src_depend[i]);
  }

  // add category
  if(!this->_category.empty())
    output_cfg.addChild(L"category").setContent(this->_category);

  // add description
  if(!this->_description.empty())
    output_cfg.addChild(L"description").setContent(this->_description);

  // create readme file
  OmCString reamde;

  if(!this->_description.empty()) {

    // use provided description as readme
    Om_toUTF8(&reamde, this->_description);

  } else {

    // create readme text
    reamde.append("# Mod Pack for \"");
    reamde.append(Om_toUTF8(this->_name)); reamde.append("\"\r\n");
    reamde.append("\r\n"
    "This file was created using Open Mod Manager and is intended to be installed using\r\n"
    "Open Mod Manager or any compatible software.\r\n"
    "\r\n"
    "# Manual installation\r\n"
    "\r\n"
    "If you want to install this Mod manually, you can find Mod files into\r\n"
    "the \""); reamde.append(Om_toUTF8(out_root)); reamde.append("\" subdirectory\r\n"
    "of this archive.");
    reamde.append("\r\n"
    "Its content is respecting the target application/game tree and here is the list of\r\n"
    "files to be copied :\r\n"
    "\r\n");
    for(size_t i = 0; i < this->_src_entry.size(); ++i) {
      reamde.append(" - "); reamde.append(Om_toUTF8(this->_src_entry[i].path)); reamde.append("\r\n");
    }
    reamde.append("\r\n"
    "Once you made a backup of the original files, you can install Mod by extracting\r\n"
    "files to the proper target application/game directory, overwriting original files.\r\n"
    "\r\n"
    "# About Open Mod Manager\r\n"
    "\r\n"
    "For more information about Open Mod Manager and Mod Pack, please visit :\r\n"
    "\r\n   "); reamde.append(Om_toUTF8(OM_APP_URL));
  }

  // add readme file in destination archive
  if(!output_zip.entryAdd(reamde.c_str(), reamde.size(), SAVEAS_README_NAME, nullptr, user_ptr)) {
    this->_error(L"saveAs", Om_errZipComp(L"Readme file", SAVEAS_README_NAME, output_zip.lastErrorStr()));
    has_error = true;
  }

  // add picture reference (will be added to zip later)
  if(this->_thumbnail.valid()) {

    // add entry into definition
    output_cfg.addChild(L"picture").setContent(SAVEAS_THUMBN_NAME);

    // convert current thumbnail image to PNG data
    uint64_t data_len;
    uint8_t* data_buf = Om_imgEncodePng(&data_len, this->_thumbnail.data(),
                                        this->_thumbnail.width(),
                                        this->_thumbnail.height(), 4);
    if(data_buf) {

      // add PNG data as file in destination archive
      if(!output_zip.entryAdd(data_buf, data_len, SAVEAS_THUMBN_NAME, nullptr, user_ptr)) {
        this->_error(L"saveAs", Om_errZipComp(L"Thumbnail file", SAVEAS_THUMBN_NAME, output_zip.lastErrorStr()));
        has_error = true;
      }

      Om_free(data_buf);

    } else {
      this->_error(L"saveAs", Om_errBadAlloc(L"Thumbnail creation", SAVEAS_THUMBN_NAME));
      has_error = true;
    }
  }

  // finally add the definition file
  OmCString cfg_data = output_cfg.data();
  if(!output_zip.entryAdd(cfg_data.c_str(), cfg_data.size(), SAVEAS_MODDEF_NAME, nullptr, user_ptr)) {
    this->_error(L"saveAs", Om_errZipComp(L"Definition file", SAVEAS_MODDEF_NAME, output_zip.lastErrorStr()));
    has_error = true;
  }

  // close and finalize archive file
  output_zip.close();

  // rename archive file
  if(!has_error) {
    int32_t result = Om_fileMove(tmp_path, path);
    if(result != 0) {
      this->_error(L"saveAs", Om_errRename(L"Output archive file", tmp_path, result));
      has_error = true;
    }
  }

  if(has_error) {
    Om_fileDelete(tmp_path);
    return false;
  }

  // making report
  wchar_t done_str[32];
  swprintf(done_str, 32, L"done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);
  this->_log(OM_LOG_OK, L"saveAs", done_str);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPack::_log(unsigned level, const OmWString& origin,  const OmWString& detail)
{
  if(this->_ModChan) {
    OmWString root(L"ModPack["); root.append(this->_iden); root.append(L"].");
    this->_ModChan->escalateLog(level, root + origin, detail);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPack::_error(const OmWString& origin, const OmWString& detail)
{
  this->_lasterr = detail;
  this->_log(OM_LOG_ERR, origin, detail);
}
