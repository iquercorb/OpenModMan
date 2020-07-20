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

#include "thirdparty/miniz/miniz.h"
#include "OmZipFile.h"
#include "OmXmlDoc.h"
#include "OmManager.h"
#include "OmContext.h"
#include "OmLocation.h"
#include "OmPackage.h"
#include <time.h>


#define PKG_BACKUP_DIR    L"backup_dir"
#define PKG_BACKUP_DEF    L"backup_def"
#define PKG_SOURCE_DIR    L"source_dir"
#define PKG_SOURCE_DEF    L"source_def"


/// \brief Get package folder tree.
///
/// Get the full list, in recursive way, of items (subfolders and files)
/// contained in the specified folder.
///
/// \param[in]  ls      : Pointer to array to get list of entries.
/// \param[in]  orig    : Path to where to begin the inspection.
/// \param[in]  from    : Relative path from orig to build paths in recursive way.
///
/// \return The filled buffer as const char
///
static void __OmPackage_getItemsFromDir(vector<OmPackageItem>* ls, const wstring& orig, const wstring& from)
{
  wstring item;
  wstring root;

  wstring srch(orig); srch += L"\\*";
  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

      OmPackageItem pkg_item;
      pkg_item.cdri = -1;
      pkg_item.dest = PKGITEM_DEST_NUL;

      if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if(!from.size()) {
          item = fd.cFileName;
        } else {
          Om_concatPaths(item, from, fd.cFileName);
        }
        Om_concatPaths(root, orig, fd.cFileName);

        pkg_item.type = PKGITEM_TYPE_D;
        // add final backslash to stay consistent with zip CDR conventions
        pkg_item.path = item + L"\\";
        ls->push_back(pkg_item);

        // go deep in hierarchy
        __OmPackage_getItemsFromDir(ls, root, item);

      } else {
        Om_concatPaths(item, from, fd.cFileName);

        pkg_item.type = PKGITEM_TYPE_F;
        pkg_item.path = item;
        ls->push_back(pkg_item);
      }
    } while(FindNextFileW(hnd, &fd));
  }
  FindClose(hnd);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmPackage::OmPackage() :
  _type(0),
  _ident(),
  _hash(0),
  _name(),
  _source(),
  _sourceDir(),
  _sourceItem(),
  _depends(),
  _backup(),
  _backupDir(),
  _backupItem(),
  _overlap(),
  _desc(),
  _version(),
  _picture(nullptr),
  _location(nullptr),
  _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmPackage::OmPackage(OmLocation* location) :
  _type(0),
  _ident(),
  _hash(0),
  _name(),
  _source(),
  _sourceDir(),
  _sourceItem(),
  _depends(),
  _backup(),
  _backupDir(),
  _backupItem(),
  _overlap(),
  _desc(),
  _version(),
  _picture(nullptr),
  _location(location),
  _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmPackage::~OmPackage()
{
  if(this->_picture) DeleteObject(this->_picture);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::sourceParse(const wstring& path)
{
  this->_source = path;

  // check if it is a file
  if(Om_isFile(this->_source)) {

    // get Package identity from file name without extension
    this->_ident = Om_getNamePart(this->_source);

    // this is a file, so check if it is a Zip file
    if(Om_isFileZip(this->_source)) {

      // we got a zip file, checks whether it is formated like we expect it
      OmZipFile src_zip;
      if(!src_zip.load(this->_source)) {
        // bad news, I can't even load it...
        this->_error = L"Source ZIP archive \""+this->_source+L"\"";
        this->_error += OMM_STR_ERR_ZIPOPEN(src_zip.lastErrorStr());
        this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
        src_zip.close();
        this->sourceClear();
        return false;
      }

      bool has_def = false;
      OmConfig src_def;

      // check for a an .omp definition file in the zip archive
      wstring zcd_entry;
      unsigned zcd_count = src_zip.indexCount(); //< count of Central Directory Records

      // we search beginning from the last entry because the definition file is
      // almost all the time the very last one added in in the archive
      unsigned zcd_index = zcd_count;
      while(zcd_index--) {

        src_zip.index(zcd_entry, zcd_index); //< get Central Directory Record entry

        if(Om_extensionMatches(zcd_entry, OMM_PKG_FILE_EXT)) {
          // good candidate for Package definition, try to load it
          char* cbuf = nullptr;
          size_t s = src_zip.size(zcd_index);
          try {
            cbuf = new char[s+1];
          } catch (std::bad_alloc& ba) {
            this->_error = L"Definition file \""+zcd_entry+L"\"";
            this->_error += OMM_STR_ERR_ZIPINFL(Om_fromUtf8(ba.what()));
            this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
            src_zip.close();
            this->sourceClear();
            return false;
          }
          if(!src_zip.extract(zcd_index, cbuf, s)) {
            this->_error = L"Definition file \""+zcd_entry+L"\"";
            this->_error += OMM_STR_ERR_ZIPINFL(src_zip.lastErrorStr());
            this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
            src_zip.close();
            delete [] cbuf;
            this->sourceClear();
            return false;
          }
          cbuf[s] = 0;
          if(src_def.parse(Om_fromUtf8(cbuf), OMM_CFG_SIGN_PKG)) {
            has_def = true;
            delete [] cbuf;
            break;
          } else {
            this->_error = L"Source definition \""+zcd_entry+L"\"";
            this->_error += OMM_STR_ERR_DEFOPEN(src_def.lastErrorStr());
            this->log(1, L"Package("+this->_ident+L") Parse Source", this->_error);
          }
          delete [] cbuf;
        }
      }

      if(has_def) {
        // definition file was found we retrieve Package infos from it
        OmXmlNode cfg_xml = src_def.xml();
        // search for <install> node in definition
        if(!cfg_xml.hasChild(L"install")) {
          this->_error = L"Invalid definition : <install> node missing.";
          this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
          src_zip.close();
          this->sourceClear();
          return false;
        }
        // install directory name
        this->_sourceDir = cfg_xml.child(L"install").content();
        // make sure src list is empty
        this->_sourceItem.clear();
        // then we now must create the tree
        OmPackageItem pkg_item;
        pkg_item.dest = PKGITEM_DEST_NUL;
        // here we go to gather files to install
        for(unsigned i = 0; i < zcd_count; ++i) {
          src_zip.index(zcd_entry, i); //< get Central Directory Record entry

          // we check for all entry with the specified destination folder as
          // root, then we get only the relative path for the Package Item.
          if(Om_getRelativePath(pkg_item.path, this->_sourceDir, zcd_entry)) {
            // we got one, lets add it to the package tree
            pkg_item.cdri = i; //< keep Zip CDR index
            pkg_item.type = src_zip.indexIsDir(i) ? PKGITEM_TYPE_D : PKGITEM_TYPE_F;
            this->_sourceItem.push_back(pkg_item);
          }
        }
        // make sure dependency list is empty
        this->_depends.clear();
        // check for Package dependencies
        if(cfg_xml.hasChild(L"dependencies")) {
          OmXmlNode dpnd_node = cfg_xml.child(L"dependencies");
          unsigned dpnd_count = dpnd_node.childCount(L"ident");
          for(unsigned i = 0; i < dpnd_count; ++i) {
            this->_depends.push_back(dpnd_node.child(L"ident", i).content());
          }
        }
        // check for Package description
        if(cfg_xml.hasChild(L"description")) {
          this->_desc = cfg_xml.child(L"description").content();
          // the XML store text with simple LF as new line, so we need to
          // replace back all simple LF into CRLF
          size_t lf = this->_desc.find(L'\n'); // get first occurrence
          while(lf != std::wstring::npos) {
            this->_desc.replace(lf, 1, L"\r\n"); //< replace with CRLF
            lf = this->_desc.find(L'\n', lf + 2); //< get next occurrence
          }
        }
        // check for Package picture
        if(cfg_xml.hasChild(L"picture")) {
          wstring pic_name = cfg_xml.child(L"picture").content();
          int zcrd_index = src_zip.locate(pic_name);
          if(zcrd_index >= 0) {
            uint8_t* data = nullptr;
            size_t s = src_zip.size(zcrd_index);
            try {
              data = new uint8_t[s];
            } catch (std::bad_alloc& ba) {
              this->_error = L"Picture file \""+pic_name+L"\"";
              this->_error += OMM_STR_ERR_ZIPINFL(Om_fromUtf8(ba.what()));
              this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
            }
            if(data != nullptr) {
              if(src_zip.extract(zcrd_index, data, s)) {
                // finally load picture data
                this->_picture = (HBITMAP)Om_loadBitmap(data, s, 0, 0, true);
              } else {
                this->_error = L"Picture file \""+pic_name+L"\"";
                this->_error += OMM_STR_ERR_ZIPINFL(src_zip.lastErrorStr());
                this->log(1, L"Package("+this->_ident+L") Parse Source", this->_error);
              }
              delete [] data;
            }
          } else {
            this->_error =  L"Referenced picture file \""+pic_name+L"\"";
            this->_error += L" was not found in ZIP archive.";
            this->log(1, L"Package("+this->_ident+L") Parse Source", this->_error);
          }
        }
      } else {

        // no Package definition found in archive, we try the legacy way to
        // parse the Package. The rule is to have a folder with the same than
        // the zip file itself at the root of the zip file,
        // lets call it the "mirror folder": PackageName.zip => PackageName/

        bool has_mirror = false;
        for(unsigned i = 0; i < zcd_count; ++i) {
          src_zip.index(zcd_entry, i);
          if(Om_isRootOfPath(this->_ident, zcd_entry)) {
            has_mirror = true;
            break;
          }
        }
        if(!has_mirror) {
          // bad luck, this one is not is not a good one
          this->_error =  L"Source ZIP archive \""+this->_source+L"\"";
          this->_error += L" is not a valid Package: mirror folder missing.";
          this->log(1, L"Package("+this->_ident+L") Parse Source", this->_error);
          src_zip.close();
          this->sourceClear();
          return false;
        }
        // make sure src list is empty
        this->_sourceItem.clear();
        // then we now must create the tree
        OmPackageItem pkg_item;
        pkg_item.dest = PKGITEM_DEST_NUL;
        // we check ALL zip CDR entries at once to get all what we can
        for(unsigned i = 0; i < zcd_count; ++i) {

          src_zip.index(zcd_entry, i); //< get Central Directory Record entry

          // we check for all entry with the package identity folder as
          // root, then we get only the relative path for the Package Item.
          if(Om_getRelativePath(pkg_item.path, this->_ident, zcd_entry)) {
            // we got one, lets add it to the package tree
            pkg_item.cdri = i; //< keep Zip CDR index
            pkg_item.type = src_zip.indexIsDir(i) ? PKGITEM_TYPE_D : PKGITEM_TYPE_F;
            this->_sourceItem.push_back(pkg_item);
          }
          // lookup for a readme file to get description
          if(Om_namesMatches(zcd_entry, L"readme.txt")) {
            char* cbuf = nullptr;
            size_t s = src_zip.size(i);
            try {
              cbuf = new char[s+1];
            } catch (std::bad_alloc& ba) {
              this->_error = L"Readme file \""+zcd_entry+L"\"";
              this->_error += OMM_STR_ERR_ZIPINFL(Om_fromUtf8(ba.what()));
              this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
            }
            if(cbuf != nullptr) {
              if(src_zip.extract(i, cbuf, s)) {
                cbuf[s] = 0; //< add terminal null
                this->_desc = Om_fromUtf8(cbuf);
              } else {
                this->_error = L"Readme file \""+zcd_entry+L"\"";
                this->_error += OMM_STR_ERR_ZIPINFL(src_zip.lastErrorStr());
                this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
              }
              delete[] cbuf; //< do not forget to delete buffer
            }
          }
          // lookup for snapshot
          if(Om_namesMatches(zcd_entry, L"picture.jpg") ||
             Om_namesMatches(zcd_entry, L"picture.png") ||
             Om_namesMatches(zcd_entry, L"picture.bmp") ||
             Om_namesMatches(zcd_entry, L"picture.gif")) {

            uint8_t* data = nullptr;
            size_t s = src_zip.size(i);
            try {
              data = new uint8_t[s];
            } catch (std::bad_alloc& ba) {
              this->_error = L"Picture file \""+zcd_entry+L"\"";
              this->_error += OMM_STR_ERR_ZIPINFL(Om_fromUtf8(ba.what()));
              this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
            }
            if(data != nullptr) {
              if(src_zip.extract(i, data, s)) {
                this->_picture = (HBITMAP)Om_loadBitmap(data, s, 0, 0, true);
              } else {
                this->_error = L"Picture file \""+zcd_entry+L"\"";
                this->_error += OMM_STR_ERR_ZIPINFL(src_zip.lastErrorStr());
                this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
              }
              delete [] data; //< do not forget to delete buffer
            }
          }
        }
      }

      src_zip.close();

      this->_type |= PKG_TYPE_ZIP;

    } else {

      // file but not a zip, skip this...
      this->sourceClear();
      return false;
    }
  } else {
    // the source is not a file, maybe this is a folder
    if(Om_isDir(this->_source)) {

      // get Package identity from folder name full name
      this->_ident = Om_getFilePart(this->_source);

      // make sure src list is empty
      this->_sourceItem.clear();

      // get Package item list from folder content (recursive function)
      __OmPackage_getItemsFromDir(&this->_sourceItem, this->_source, L"");

    } else {

      // obviously something is wrong
      this->sourceClear();
      return false;
    }
  }

  // parse raw name to get display name and potential version
  wstring vers;
  if(Om_parsePkgIdent(this->_name, vers, this->_source, this->isType(PKG_TYPE_ZIP), true)) {
    this->_version.parse(vers);
  }

  // create hash from file name
  this->_hash = Om_getXXHash3(Om_getFilePart(this->_source));

  // This package is now valid source to be installed
  this->_type |= PKG_TYPE_SRC;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::backupParse(const wstring& path)
{
  OmConfig bck_def;

  bool has_def = false;

  this->_backup = path;

  if(Om_isFileZip(this->_backup)) {

    OmZipFile bck_zip;

    // load the backup Zip file
    if(!bck_zip.load(this->_backup)) {
      this->_error = L"Backup ZIP archive \""+this->_source+L"\"";
      this->_error += OMM_STR_ERR_ZIPOPEN(bck_zip.lastErrorStr());
      this->log(0, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
      bck_zip.close();
      this->backupClear();
      return false;
    }

    // check for a an .omb definition file in the zip archive
    wstring zcd_entry;
    unsigned zcd_count = bck_zip.indexCount(); //< count of Central Directory Records

    // we search beginning from the last entry because the definition file is
    // almost all the time the very last one added in in the archive
    unsigned zcd_index = zcd_count;
    while(zcd_index--) {

      bck_zip.index(zcd_entry, zcd_index); //< get Central Directory Record entry

      if(Om_extensionMatches(zcd_entry, OMM_BCK_FILE_EXT)) {
        // good candidate for Package definition, try to load it
        char* cbuf = nullptr;
        size_t s = bck_zip.size(zcd_index);
        try {
          cbuf = new char[s+1];
        } catch (std::bad_alloc& ba) {
          this->_error = L"Definition file \""+zcd_entry+L"\"";
          this->_error += OMM_STR_ERR_ZIPINFL(Om_fromUtf8(ba.what()));
          this->log(0, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
          bck_zip.close();
          this->backupClear();
          return false;
        }
        if(!bck_zip.extract(zcd_index, cbuf, s)) {
          this->_error = L"Picture file \""+zcd_entry+L"\"";
          this->_error += OMM_STR_ERR_ZIPINFL(bck_zip.lastErrorStr());
          this->log(0, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
          bck_zip.close();
          delete [] cbuf;
          this->backupClear();
          return false;
        }
        cbuf[s] = 0;
        if(bck_def.parse(Om_fromUtf8(cbuf), OMM_CFG_SIGN_BCK)) {
          has_def = true;
          delete [] cbuf;
          break;
        } else {
          this->_error = L"Source definition \""+zcd_entry+L"\"";
          this->_error += OMM_STR_ERR_DEFOPEN(bck_def.lastErrorStr());
          this->log(1, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
        }
        delete [] cbuf;
      }
    }

    if(!has_def) {
      this->_error =  L"Invalid backup data ZIP archive: Definition file missing.";
      this->log(1, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
      this->backupClear();
      return false;
    }

    // we do not need the zip file anymore
    bck_zip.close();

  } else {

    if(!Om_isDir(this->_backup)) {
      this->_error =  L"Found Invalid backup data: File is not a ZIP archive.";
      this->log(1, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
      this->backupClear();
      return false;
    }

    // search a Backup definition file within the sub-folder
    vector<wstring> ls;
    Om_lsFile(&ls, this->_backup, true);

    for(size_t i = 0; i < ls.size(); ++i) {
      if(Om_extensionMatches(ls[i], OMM_BCK_FILE_EXT)) {
        if(bck_def.open(ls[i], OMM_CFG_SIGN_BCK)) {
          has_def = true;
          break;
        } else {
          this->_error = L"Backup definition \""+ls[i]+L"\"";
          this->_error += OMM_STR_ERR_DEFOPEN(bck_def.lastErrorStr());
          this->log(1, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
        }
      }
    }

    if(!has_def) {
      this->_error = L"Invalid backup data: Definition file missing.";
      this->log(0, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
      this->backupClear();
      return false;
    }
  }

  OmXmlNode def_xml = bck_def.xml();

  // verify the Backup definition have the proper nodes
  if(def_xml.hasChild(L"backup")) {
    this->_backupDir = def_xml.child(L"backup").content();
  } else {
    this->_error = L"Invalid definition : <backup> node missing.";
    this->log(0, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
    this->backupClear();
    return false;
  }
  if(def_xml.hasChild(L"ident")) {
    this->_ident = def_xml.child(L"ident").content();
  } else {
    this->_error = L"Invalid definition : <ident> node missing.";
    this->log(0, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
    this->backupClear();
    return false;
  }
  if(def_xml.hasChild(L"hash")) {
    this->_hash = Om_toUint64(def_xml.child(L"hash").content());
  } else {
    this->_error = L"Invalid definition : <hash> node missing.";
    this->log(0, L"Package("+Om_getFilePart(this->_backup)+L") Parse Backup", this->_error);
    this->backupClear();
    return false;
  }

  // If name and version was not previously parsed as source, pars here using
  // the identity string saved in backup definition.
  if(!this->isType(PKG_TYPE_SRC)) {
    wstring vers;
    if(Om_parsePkgIdent(this->_name, vers, this->_ident, false, true)) {
      this->_version.parse(vers);
    }
  }

  this->_backupItem.clear();
  OmPackageItem bck_item;
  OmXmlNode xml_item;

  // get list of installed files, they are listed in the
  // backup definition, either as <cpy> or <del> entries.
  unsigned item_count = def_xml.childCount(L"cpy");
  for(unsigned i = 0; i < item_count; ++i) {
    xml_item = def_xml.child(L"cpy", i);
    bck_item.path = xml_item.content();
    bck_item.type = (xml_item.attrAsInt(L"dir")>0)?PKGITEM_TYPE_D:PKGITEM_TYPE_F;
    bck_item.cdri = xml_item.attrAsInt(L"cdi");
    bck_item.dest = PKGITEM_DEST_CPY;
    this->_backupItem.push_back(bck_item);
  }
  item_count = def_xml.childCount(L"del");
  for(unsigned i = 0; i < item_count; ++i) {
    xml_item = def_xml.child(L"del", i);
    bck_item.path = xml_item.content();
    bck_item.type = (xml_item.attrAsInt(L"dir")>0)?PKGITEM_TYPE_D:PKGITEM_TYPE_F;
    bck_item.cdri = -1;
    bck_item.dest = PKGITEM_DEST_DEL;
    this->_backupItem.push_back(bck_item);
  }

  this->_overlap.clear();
  // retrieve the backup overlap list, they are stored as a list of Hash values
  // corresponding to package file name
  if(def_xml.hasChild(L"overlap")) {
    OmXmlNode xml_ovlap = def_xml.child(L"overlap");
    unsigned hash_count = xml_ovlap.childCount(L"hash");
    for(unsigned i = 0; i < hash_count; ++i) {
      xml_item = xml_ovlap.child(L"hash", i);
      this->_overlap.push_back(Om_toUint64(xml_item.content()));
    }
  }

  // All appear OK, we finalizes our package setup
  this->_type |= PKG_TYPE_BCK;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::sourceClear()
{
  // remove the source flag
  this->_type &= ~PKG_TYPE_SRC;
  this->_type &= ~PKG_TYPE_ZIP;
  this->_depends.clear();
  this->_source.clear();
  this->_sourceDir.clear();
  this->_sourceItem.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::backupClear()
{
  // remove the backup flag
  this->_type &= ~PKG_TYPE_BCK;
  this->_overlap.clear();
  this->_backup.clear();
  this->_backupDir.clear();
  this->_backupItem.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::sourceValid()
{
  if(this->_type & PKG_TYPE_SRC) {

    // this is brutal but safe, we simply re-parse the previously
    // parsed source to ensure everything is up to date
    if(!this->sourceParse(this->_source)) {
      this->sourceClear();
      return false;
    }
  } else {
    // the Package was not even previously a source
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::backupValid()
{
  if(this->_type & PKG_TYPE_BCK) {

    // this is brutal but safe, we simply re-parse the previously
    // parsed source to ensure everything is up to date
    if(!this->backupParse(this->_backup)) {
      this->backupClear();
      return false;
    }
  } else {
    // the Package was not even previously a source
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::uninst(HWND hPb, const bool *pAbort)
{
  // cannot install without valid Location
  if(this->_location == nullptr) {
    this->_error = L"Package cannot be uninstalled out of a valid Location.";
    this->log(0, L"Package("+this->_ident+L") Uninstall", this->_error);
    return false;
  }

  if(!(this->_type & PKG_TYPE_BCK)) {
    // I am sorry Dave, but there is no backup to restore
    this->_error = L"Backup data does not exists.";
    this->log(0, L"Package("+this->_ident+L") Uninstall", this->_error);
    return false;
  }

  // ultimate validity check before try to install
  if(!this->backupValid()) {
    this->_error = L"Backup data should exist but was not found.";
    this->log(0, L"Package("+this->_ident+L") Uninstall", this->_error);
    return false;
  }

  // initialize the progress bar
  if(hPb) {
    SendMessageW(hPb, PBM_SETRANGE, 0, MAKELPARAM(0, this->_backupItem.size()));
    SendMessageW(hPb, PBM_SETSTEP, 1, 0);
    SendMessageW(hPb, PBM_SETPOS, 0, 0);
  }

  // it still time to abort
  if(pAbort) {
    if(*pAbort) {
      this->log(1, L"Package("+this->_ident+L") Uninstall", L"Aborted.");
      return true;
    }
  }

  // restore backed files into destination tree
  if(!this->_doUninst(hPb)) {
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::install(unsigned zipLvl, HWND hPb, const bool *pAbort)
{
  // cannot install without valid Location
  if(this->_location == nullptr) {
    this->_error = L"Package cannot be installed our of a valid Location.";
    this->log(0, L"Package("+this->_ident+L") Install", this->_error);
    return false;
  }

  // is there really something to install ?
  if(!(this->_type & PKG_TYPE_SRC)) {
    this->_error = L"Source data does not exist.";
    this->log(1, L"Package("+this->_ident+L") Install", this->_error);
    return false;
  }

  // ultimate validity check before try to install
  if(!this->sourceValid()) {
    this->_error = L"Source data should exist but was not found.";
    this->log(0, L"Package("+this->_ident+L") Install", this->_error);
    return false;
  }

  // initialize the progress bar
  if(hPb) {
    SendMessageW(hPb, PBM_SETRANGE, 0, MAKELPARAM(0, 2*this->_sourceItem.size()));
    SendMessageW(hPb, PBM_SETSTEP, 1, 0);
    SendMessageW(hPb, PBM_SETPOS, 0, 0);
  }

  // it still time to abort
  if(pAbort) {
    if(*pAbort) {
      this->log(1, L"Package("+this->_ident+L") Install", L"Aborted.");
      return true;
    }
  }

  // Step 1 : Create backups of destination files overwritten by package
  if(!this->_doBackup(zipLvl, hPb, pAbort)) {
    return false;
  }

  if(pAbort) {
    if(*pAbort) {
      return true;
    }
  }

  // Step 2 : Install package files into destination tree
  if(!this->_doInstall(hPb, pAbort)) {
    return false;
  }

  if(pAbort) {
    if(*pAbort) {
      return true;
    }
  }

  return true;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::couldOverlap(const OmPackage* other) const
{
  if(other->_backupItem.size()) {

    for(size_t i = 0; i < _sourceItem.size(); ++i) {

      for(size_t j = 0; j < other->_backupItem.size(); ++j) {

        // compare only if both are file or folder
        if(_sourceItem[i].type != other->_backupItem[j].type)
          continue;

        // Directories to be cleaned need to be empty first, so we also
        // lookup for created directories overlap
        if(other->_backupItem[j].type == PKGITEM_TYPE_D) {
          // compare only if other entry is "To Delete", meaning this was a
          // created directory, to be deleted at restore
          if(other->_backupItem[j].dest != PKGITEM_DEST_DEL) {
            continue;
          }
        }

        // same path mean overlap
        if(_sourceItem[i].path == other->_backupItem[j].path) {
          return true;
        }
      }
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::setPicture(HBITMAP hBmp)
{
  if(hBmp != nullptr) {

    this->_picture = static_cast<HBITMAP>(CopyImage(hBmp,IMAGE_BITMAP,0,0,0));

  } else {

    if(this->_picture != nullptr)
      DeleteObject(this->_picture);

    this->_picture = nullptr;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::save(const wstring& path, unsigned zipLvl, HWND hPb, HWND hSc, const bool *pAbort)
{
  if(!(this->_type & PKG_TYPE_ZIP)) {
    if(Om_isDir(this->_source)) {
      if(!Om_checkAccess(this->_source, OMM_ACCESS_DIR_READ)) {
        this->_error = L"Source folder \""+this->_source+L"\"";
        this->_error += OMM_STR_ERR_ACCESS_R;
        this->log(0, L"Package("+path+L") Save", this->_error);
        return false;
      }
    } else {
      this->_error = L"Source folder \""+this->_source+L"\"";
      this->_error += OMM_STR_ERR_ISDIR;
      this->log(0, L"Package("+path+L") Save", this->_error);
      return false;
    }
  }

  if(!this->_sourceItem.size()) {
    this->_error = L"Package does not contain any data to be saved.";
    this->log(0, L"Package("+path+L") Save", this->_error);
    return false;
  }

  // create package identity according destination path
  wstring pkg_ident = Om_getNamePart(path);

  OmZipFile src_zip;

  int result;
  bool has_failed = false;
  bool has_aborted = false;

  // do we got a Zip file or a legacy Folder
  if(this->_type & PKG_TYPE_ZIP) {
    if(!src_zip.load(this->_source)) {
      this->_error = L"Source ZIP archive \""+this->_source+L"\"";
      this->_error += OMM_STR_ERR_ZIPOPEN(src_zip.lastErrorStr());
      this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
      src_zip.close();
      return false;
    }
  }

  OmZipFile pkg_zip;

  // use temporary file name, in case source and destination are the same
  wstring pkg_tmp_path;
  Om_concatPaths(pkg_tmp_path, Om_getDirPart(path), pkg_ident);
  pkg_tmp_path += L".ztmp";

  // initialize zip archive
  if(!pkg_zip.init(pkg_tmp_path)) {
    this->_error = L"Destination ZIP archive \""+path+L"\"";
    this->_error += OMM_STR_ERR_ZIPINIT(pkg_zip.lastErrorStr());
    this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
    pkg_zip.close();
    Om_fileDelete(pkg_tmp_path);
    return false;
  }

  OmConfig pkg_def;
  pkg_def.init(OMM_CFG_SIGN_PKG);

  OmXmlNode  def_xml = pkg_def.xml();

  // defines the package source directory
  def_xml.addChild(L"install").setContent(pkg_ident);

  // initialize the progress bar
  if(hPb) {
    SendMessageW(hPb, PBM_SETRANGE, 0, MAKELPARAM(0, this->_sourceItem.size()));
    SendMessageW(hPb, PBM_SETSTEP, 1, 0);
    SendMessageW(hPb, PBM_SETPOS, 0, 0);
  }

  wstring src_path, zcd_entry;

  uint8_t*  data;
  size_t    s;

  for(size_t i = 0; i < this->_sourceItem.size(); ++i) {

    // check for abort request
    if(pAbort) {
      if(*pAbort) {
        has_aborted = true;
        break;
      }
    }

    // update description
    if(hSc) SetWindowTextW(hSc, this->_sourceItem[i].path.c_str());

    // step progress bar
    if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);
    #ifdef DEBUG_SLOW
    Sleep(DEBUG_SLOW); //< for debug
    #endif

    // destination zip path, with mirror folder preceding
    Om_concatPaths(zcd_entry, pkg_ident, this->_sourceItem[i].path);
    // check if we have a file or folder to install
    if(this->_sourceItem[i].type == PKGITEM_TYPE_F) { //< this is a file
      // if zip Source, extract from zip to append to other
      if(this->_type & PKG_TYPE_ZIP) {
        // create new buffer to load source zip content
        s = src_zip.size(this->_sourceItem[i].cdri);
        if(s) {
          try {
            data = new uint8_t[s];
          } catch (std::bad_alloc& ba) {
            this->_error = L"Source file \""+this->_sourceItem[i].path+L"\"";
            this->_error += OMM_STR_ERR_ZIPINFL(Om_fromUtf8(ba.what()));
            this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
            has_failed = true;
            break;
          }
          // extract source zip content to buffer
          if(!src_zip.extract(this->_sourceItem[i].cdri, data, s)) {
            this->_error = L"Source file \""+this->_sourceItem[i].path+L"\"";
            this->_error += OMM_STR_ERR_ZIPINFL(src_zip.lastErrorStr());
            this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
            has_failed = true;
            delete [] data;
            break;
          }
          // append data to destination zip archive
          if(!pkg_zip.append(data, s, zcd_entry, zipLvl)) {
            this->_error = L"Package file \""+zcd_entry+L"\"";
            this->_error += OMM_STR_ERR_ZIPDEFL(pkg_zip.lastErrorStr());
            this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
            has_failed = true;
            delete [] data;
            break;
          }
          delete [] data;
        }
      } else {
        // path to item in package sub-directory
        Om_concatPaths(src_path, this->_source, this->_sourceItem[i].path);
        // add file to destination zip archive
        if(!pkg_zip.append(src_path, zcd_entry, zipLvl)) {
          this->_error = L"Package file \""+zcd_entry+L"\"";
          this->_error += OMM_STR_ERR_ZIPDEFL(pkg_zip.lastErrorStr());
          this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
          has_failed = true;
          break;
        }
      }
    } else {
      // add folder to destination zip archive
      if(!pkg_zip.append(nullptr, 0, zcd_entry, zipLvl)) {
        this->_error = L"Package subfolder \""+zcd_entry+L"\"";
        this->_error += OMM_STR_ERR_ZIPDEFL(pkg_zip.lastErrorStr());
        this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
        has_failed = true;
        break;
      }
    }
  }

  // do not need the source zip anymore
  src_zip.close();

  if(has_failed) {
    pkg_zip.close();
    Om_fileDelete(pkg_tmp_path);
    // update description
    if(hSc) SetWindowTextW(hSc, L"Process failed");
    return false;
  }

  if(has_aborted) {
    pkg_zip.close();
    Om_fileDelete(pkg_tmp_path);
    // update description
    if(hSc) SetWindowTextW(hSc, L"Process aborted");
    return true;
  }

  // add dependency to source definition
  if(this->_depends.size()) {
    OmXmlNode xml_dpnd = def_xml.addChild(L"dependencies");
    for(size_t i = 0; i < this->_depends.size(); ++i) {
      xml_dpnd.addChild(L"ident").setContent(this->_depends[i]);
    }
  }

  // add description to source definition
  if(!this->_desc.empty()) {
    def_xml.addChild(L"description").setContent(this->_desc);
  }

  // add picture to archive and source definition
  if(this->_picture != nullptr) {

    // create a PNG image from raw pixel data
    size_t png_size = 0;
    void* png_data = Om_getPngData(this->_picture, &png_size);

    // add picture as PNG file in zip archive
    if(!pkg_zip.append(png_data, png_size, L"source_pic.png", zipLvl)) {
      this->_error = L"Picture file \"source_pic.png\"";
      this->_error += OMM_STR_ERR_ZIPDEFL(pkg_zip.lastErrorStr());
      this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
      pkg_zip.close();
      Om_fileDelete(pkg_tmp_path);
      return false;
    }

    // release png data
    mz_free(png_data);

    // add section to source definition
    def_xml.addChild(L"picture").setContent(L"source_pic.png");
  }

  // add a REAMDE.TXT file into archive
  string pkg_readme = "Open Mod Manager Package file for \"";
  pkg_readme += Om_toUtf8(pkg_ident);
  pkg_readme += "\" Mod.\r\n\r\n"
  "This Mod Package was created using Open Mod Manager and is intended to be\r\n"
  "installed using Open Mod Manager or any other compatible software.\r\n\r\n"
  "If you want to install this Mod manually, you will find the Mod files into\r\n"
  "the following folder : \r\n\r\n  \"";
  pkg_readme += Om_toUtf8(pkg_ident);
  pkg_readme += "\"\r\n\r\n"
  "Its content is respecting the destination folder tree and includes files to\r\n"
  "be overwritten or added :\r\n"
  "\r\n";
  for(size_t i = 0; i < this->_sourceItem.size(); ++i) {
    pkg_readme += "   ";
    pkg_readme += Om_toUtf8(this->_sourceItem[i].path);
    pkg_readme += "\r\n";
  }
  pkg_readme += "\r\n"
  "Once you made a backup of the original files, you can install the Mod by\r\n"
  "extracting the content of the previously indicated folder into the\r\n"
  "application/game installation folder, overwriting original files.\r\n\r\n"

  // TODO: update URL here...
  "For more information about Open Mod Manager and Open Mod Packages, please\r\n"
  "visit :\r\n\r\n   https://...";

  // add the REAMDE.TXT file in zip archive
  if(!pkg_zip.append(pkg_readme.c_str(), pkg_readme.size(), L"README.TXT", zipLvl)) {
    this->_error = L"Package README \"README.TXT\"";
    this->_error += OMM_STR_ERR_ZIPDEFL(pkg_zip.lastErrorStr());
    this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
    pkg_zip.close();
    Om_fileDelete(pkg_tmp_path);
    return false;
  }

  // compose source definition file name
  wstring pkg_def_name = PKG_SOURCE_DEF;
  pkg_def_name += L".";
  pkg_def_name += OMM_PKG_FILE_EXT;

  // get XML backup data
  string pkg_def_data = pkg_def.data();

  // add the definition file in zip archive
  if(!pkg_zip.append(pkg_def_data.c_str(), pkg_def_data.size(), pkg_def_name, zipLvl)) {
    this->_error = L"Definition file \""+pkg_def_name+L"\"";
    this->_error += OMM_STR_ERR_ZIPDEFL(pkg_zip.lastErrorStr());
    this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
    pkg_zip.close();
    Om_fileDelete(pkg_tmp_path);
    return false;
  }

  // finalize destination zip file
  pkg_zip.close();

  // compose the definitive package filename
  wstring pkg_path;
  Om_concatPaths(pkg_path, Om_getDirPart(path), pkg_ident);
  pkg_path += L".zip";

  // in case file already exists, we delete it
  if(Om_isFile(pkg_path)) {
    result = Om_fileDelete(pkg_path);
    if(result != 0) {
      this->_error =  L"File to overwrite \""+pkg_path+L"\"";
      this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
      this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
      return false;
    }
  }
  // rename temporary file to its final name
  result = Om_fileMove(pkg_tmp_path, pkg_path);
  if(result != 0) {
    this->_error =  L"Temporary file name \""+pkg_tmp_path+L"\"";
    this->_error += OMM_STR_ERR_RENAME(Om_getErrorStr(result));
    this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::clear()
{
  this->_type = 0;
  this->_hash = 0;
  if(this->_picture) DeleteObject(this->_picture);
  this->_picture = nullptr;
  this->_name.clear();
  this->_ident.clear();
  this->_source.clear();
  this->_sourceDir.clear();
  this->_sourceItem.clear();
  this->_depends.clear();
  this->_backup.clear();
  this->_backupDir.clear();
  this->_backupItem.clear();
  this->_overlap.clear();
  this->_desc.clear();
  this->_version.define(0,0,0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::log(unsigned level, const wstring& head, const wstring& detail)
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
bool OmPackage::_doBackup(int zipLvl, HWND hPb, const bool *pAbort)
{
  // initialize local timer
  clock_t time = clock();

  // a backup already exists ?
  if(this->_type & PKG_TYPE_BCK) {
    this->_error = L"Backup data already exists.";
    this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
    return false;
  }

  OmZipFile bck_zip;
  int result;
  bool has_failed = false;
  bool has_aborted = false;

  // negative compression level mean sub-directory instead of zip file
  bool is_zip = (zipLvl >= 0) ? true : false;

  // define the backup root dir
  this->_backupDir = PKG_BACKUP_DIR;

  // path to backup root path, mainly used for sub-directory backup
  wstring bck_dir_path;

  // base file name for backup file or sub-directory
  wstring bck_file_name(Om_getFilePart(this->_source));

  if(is_zip) {
    // backup zip file name
    bck_file_name += L".zip";
    // backup zip full path
    this->_backup = this->_location->_backupDir; //< Location Backup location
    this->_backup += L"\\";
    this->_backup += bck_file_name;

    // initialize zip archive
    if(!bck_zip.init(this->_backup)) {
      this->_error =  L"ZIP archive \""+this->_backup+L"\"";
      this->_error += OMM_STR_ERR_ZIPINIT(bck_zip.lastErrorStr());
      this->log(0, L"Package("+this->_ident+L") Backup", this->_error);

      bck_zip.close();
      Om_fileDelete(this->_backup);
      this->backupClear();
      return false;
    }
  } else { // Sub-Directory backup
    // backup sub-directory full path
    this->_backup = this->_location->_backupDir; //< Location Backup location
    this->_backup += L"\\";
    this->_backup += bck_file_name;

    // create sub-directory into Backup location
    result = Om_dirCreate(this->_backup);
    if(result != 0) {
      this->_error =  L"Backup main directory \""+this->_backup+L"\"";
      this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
      this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
      this->backupClear();
      return false;
    }

    // compose the path to backup root path
    bck_dir_path = this->_backup + L"\\" + this->_backupDir;

    // create backup data root folder in Backup sub-directory
    result = Om_dirCreate(bck_dir_path);
    if(result != 0) {
      this->_error =  L"Backup data subfolder \""+bck_dir_path+L"\"";
      this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
      this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
      this->backupClear();
      return false;
    }
  }

  // retrieve the list of overlapped package
  vector<uint64_t> ovlap_list;
  this->_location->getInstOwList(ovlap_list, this);

  // compose the Backup definition file name
  wstring back_def_name = PKG_BACKUP_DEF;
  back_def_name += L".";
  back_def_name += OMM_BCK_FILE_EXT;

  OmConfig bck_def;
  if(is_zip) {
    bck_def.init(OMM_CFG_SIGN_BCK);
  } else {
    bck_def.init(this->_backup + L"\\" + back_def_name, OMM_CFG_SIGN_BCK);
  }

  OmXmlNode  def_xml = bck_def.xml();

  def_xml.addChild(L"ident").setContent(this->_ident);
  def_xml.addChild(L"hash").setContent(Om_toHexString(this->_hash));

  // define backup root directory
  def_xml.addChild(L"backup").setContent(this->_backupDir);

  // it should be already empty, but we want to be sure
  this->_backupItem.clear();

  // stuff for Backup item list generation
  OmPackageItem  bck_item;
  OmXmlNode  xml_item;
  wstring app_file, bck_file, zcd_entry, bck_tree;

  for(size_t i = 0, z = 0; i < this->_sourceItem.size(); ++i) {
    // check for abort request
    if(pAbort) {
      if(*pAbort) {
        has_aborted = true;
        break;
      }
    }
    // path to item in the destination tree
    Om_concatPaths(app_file, this->_location->_installDir, this->_sourceItem[i].path);
    // our future Backup item entry
    bck_item.path = this->_sourceItem[i].path;
    bck_item.type = this->_sourceItem[i].type;
    // check if item exists in destination tree
    if(Om_pathExists(app_file)) {
      // if item is a file, save to zip archive
      if(this->_sourceItem[i].type == PKGITEM_TYPE_F) { //< this is a file
        // add a 'copy' entry into backup definition
        xml_item = def_xml.addChild(L"cpy");
        xml_item.setAttr(L"dir", L"0");
        xml_item.setAttr(L"cdi", (int)z); //< CDR index in zip
        if(is_zip) {
          // backup file path in zip archive
          Om_concatPaths(zcd_entry, this->_backupDir, this->_sourceItem[i].path);
          // add file to zip archive
          if(!bck_zip.append(app_file, zcd_entry, zipLvl)) {
            this->_error = L"Original file \""+app_file+L"\"";
            this->_error += OMM_STR_ERR_ZIPDEFL(bck_zip.lastErrorStr());
            this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
            // notify process failed and break loop
            has_failed = true;
            break;
          }
        } else {
          // path of item in backup directory
          Om_concatPaths(bck_file, bck_dir_path, this->_sourceItem[i].path);
          // create the folder tree if needed
          bck_tree = Om_getDirPart(bck_file);
          if(!Om_isDir(bck_tree)) {
            // in case the package source is a zip file, the zip may not
            // contain each sub-directories as individual CDR entries, so the
            // folder tree where to move files may not be previously created.
            result = Om_dirCreateRecursive(bck_tree);
            if(result != 0) {
              this->_error = L"Replicated subfolder \""+bck_tree+L"\"";
              this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
              this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
              // notify process failed and break loop
              has_failed = true;
              break;
            }
          }
          // copy file to backup directory
          //int result = Om_fileCopy(app_file, bck_file, true); //< slow
          result = Om_fileMove(app_file, bck_file); //< risky
          if(result != 0) {
            this->_error = L"Original file \""+app_file+L"\"";
            this->_error += OMM_STR_ERR_MOVE(Om_getErrorStr(result));
            this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
            // notify process failed and break loop
            has_failed = true;
            break;
          }
        }
        // this thing is now part of backup tree
        bck_item.cdri = z;
        bck_item.dest = PKGITEM_DEST_CPY;
        this->_backupItem.push_back(bck_item);
        z++; //< increment CDR index
      } else {
        if(!is_zip) {
          // path of item in backup directory
          Om_concatPaths(bck_file, bck_dir_path, this->_sourceItem[i].path);
          // we simply create the folder in backup directory
          result = Om_dirCreate(bck_file);
          if(result != 0) {
            this->_error = L"Replicated subfolder \""+bck_file+L"\"";
            this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
            this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
            // notify process failed and break loop
            has_failed = true;
            break;
          }
        }
        // step progress bar
        if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);
        #ifdef DEBUG_SLOW
        Sleep(DEBUG_SLOW); //< for debug
        #endif
        // do not care about folders
        continue;
      }
    } else { // item doesn't exists in destination, it should be deleted at restore
      // adds a "To be deleted" entry into backup definition
      xml_item = def_xml.addChild(L"del");
      if(this->_sourceItem[i].type == PKGITEM_TYPE_F) { //< this is a file
        xml_item.setAttr(L"dir", L"0");
      } else {
        xml_item.setAttr(L"dir", L"1");
      }
      // this thing is now part of backup tree
      bck_item.cdri = -1;
      bck_item.dest = PKGITEM_DEST_DEL;
      this->_backupItem.push_back(bck_item);
    }
    // set destination path of this entry in backup definition
    xml_item.setContent(this->_sourceItem[i].path);
    // step progress bar
    if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);
    #ifdef DEBUG_SLOW
    Sleep(DEBUG_SLOW); //< for debug
    #endif
  }

  // it still time to abort
  if(pAbort) {
    if(*pAbort) has_aborted = true;
  }

  // process abortion requested
  if(has_aborted) {
    this->log(1, L"Package("+this->_ident+L") Backup", L"Aborted.");
    if(is_zip) bck_zip.close(); //< make sure file is not longer used in order to delete it
    this->_undoInstall(hPb);
    return true;
  }

  // this mean the backup process encountered error, we undo and return
  if(has_failed) {
    this->log(1, L"Package("+this->_ident+L") Backup", L"Failed.");
    if(is_zip) bck_zip.close(); //< make sure file is not longer used in order to delete it
    this->_undoInstall(hPb);
    return false;
  }

  // we add an overlap list
  if(ovlap_list.size()) {
    OmXmlNode xml_ovlap = def_xml.addChild(L"overlap");
    for(size_t i = 0; i < ovlap_list.size(); ++i) {
      xml_ovlap.addChild(L"hash").setContent(Om_toHexString(ovlap_list[i]));
    }
  }

  if(is_zip) {
    // get backup definition XML data
    string bck_def_data = bck_def.data();
    // add definition file in zip archive
    if(!bck_zip.append(bck_def_data.c_str(), bck_def_data.size(), back_def_name, zipLvl)) {
      this->_error =  L"Definition file";
      this->_error += OMM_STR_ERR_ZIPDEFL(bck_zip.lastErrorStr());
      this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
      bck_zip.close();
      Om_fileDelete(this->_backup);
      return false;
    }
    // finalize and close zip archive
    bck_zip.close();
  } else {
    // save XML backup data
    if(!bck_def.save()) {
      this->_error = L"Definition file";
      this->_error += OMM_STR_ERR_DEFSAVE(bck_def.lastErrorStr());
      this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
      Om_dirDeleteRecursive(this->_backup);
      return false;
    }
  }

  // backup is done without error, we now can update the local overlap list
  if(ovlap_list.size()) {
    for(size_t i = 0; i < ovlap_list.size(); ++i) {
      this->_overlap.push_back(ovlap_list[i]);
    }
  }

  // we finally update this package as backup
  this->_type |= PKG_TYPE_BCK;

  // making report
  wchar_t log_buf[32];
  swprintf(log_buf, 32, L"Done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);
  this->log(2, L"Package("+this->_ident+L") Backup", log_buf);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::_doInstall(HWND hPb, const bool *pAbort)
{
  // initialize local timer
  clock_t time = clock();

  OmZipFile zip;
  int result;
  bool has_failed = false;
  bool has_aborted = false;

  wstring src_file, app_file;

  // do we got a Zip file or a legacy Folder
  if(this->_type & PKG_TYPE_ZIP) {
    if(!zip.load(this->_source)) {
      this->_error = L"Source ZIP archive \""+this->_source+L"\"";
      this->_error += OMM_STR_ERR_ZIPOPEN(zip.lastErrorStr());
      this->log(0, L"Package("+this->_ident+L") Install", this->_error);
      zip.close();
      this->_undoInstall(hPb); //< automatically uninstall the package
      return false;
    }
  }

  for(size_t i = 0; i < this->_sourceItem.size(); ++i) {
    // check for abort request
    if(pAbort) {
      if(*pAbort) {
        has_aborted = true;
        break;
      }
    }
    // path to destination file to be overwritten
    Om_concatPaths(app_file, this->_location->_installDir, this->_sourceItem[i].path);
    // check if we have a file or folder to install
    if(this->_sourceItem[i].type == PKGITEM_TYPE_F) { //< this is a file

      if(this->_type & PKG_TYPE_ZIP) {
        // extract to destination
        if(!zip.extract(this->_sourceItem[i].cdri, app_file)) {
          this->_error = L"Source file \""+this->_sourceItem[i].path+L"\"";
          this->_error += OMM_STR_ERR_ZIPINFL(zip.lastErrorStr());
          this->log(0, L"Package("+this->_ident+L") Install", this->_error);
          has_failed = true;
          break;
        }
      } else {
        // path to item in package sub-directory
        Om_concatPaths(src_file, this->_source, this->_sourceItem[i].path);
        // Copy this, and overwrite please...
        result = Om_fileCopy(src_file, app_file, true);
        if(result != 0) {
          this->_error = L"Source file \""+this->_sourceItem[i].path+L"\"";
          this->_error += OMM_STR_ERR_COPY(Om_getErrorStr(result));
          this->log(0, L"Package("+this->_ident+L") Install", this->_error);
          has_failed = true;
          break;
        }
      }

    } else {
      // we got a folder, we check if it exists in destination
      if(!Om_isDir(app_file)) {
        // we have to create this one
        result = Om_dirCreate(app_file);
        if(result != 0) {
          this->_error = L"Source subfolder \""+this->_sourceItem[i].path+L"\"";
          this->_error += OMM_STR_ERR_CREATE(Om_getErrorStr(result));
          this->log(0, L"Package("+this->_ident+L") Install", this->_error);
          has_failed = true;
          break;
        }
      }
    }
    // step progress bar
    if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);
    #ifdef DEBUG_SLOW
    Sleep(DEBUG_SLOW); //< for debug
    #endif
  }

  // it still time to abort
  if(pAbort) {
    if(*pAbort) has_aborted = true;
  }

  // process abortion requested
  if(has_aborted) {
    this->log(1, L"Package("+this->_ident+L") Install", L"Aborted.");
    if(this->_type & PKG_TYPE_ZIP) zip.close();
    this->_undoInstall(hPb);
    return true;
  }

  // this mean the backup process encountered error, we undo and return
  if(has_failed) {
    this->log(1, L"Package("+this->_ident+L") Install", L"Failed.");
    if(this->_type & PKG_TYPE_ZIP) zip.close();
    this->_undoInstall(hPb);
    return false;
  }

  // making report
  wchar_t log_buf[32];
  swprintf(log_buf, 32, L"Done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);
  this->log(2, L"Package("+this->_ident+L") Install", log_buf);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::_doUninst(HWND hPb)
{
  // initialize local timer
  clock_t time = clock();

  int result;
  bool is_zip;

  if(Om_isFileZip(this->_backup)) {

    OmZipFile zip;
    if(!zip.load(this->_backup)) {
      this->_error = L"Backup ZIP archive \""+this->_backup+L"\"";
      this->_error += OMM_STR_ERR_ZIPOPEN(zip.lastErrorStr());
      this->log(0, L"Package("+this->_ident+L") Uninstall", this->_error);
      return false;
    }

    is_zip = true; //< backup data is zip file

    wstring app_file;

    // first we restore genuine files from zip
    for(size_t i = 0; i < this->_backupItem.size(); ++i) {
      // we are interested only by backup file to copy
      if(this->_backupItem[i].dest == PKGITEM_DEST_CPY) {
        // path to installed file to be overwritten
        Om_concatPaths(app_file, this->_location->_installDir, this->_backupItem[i].path);
        // extract from zip
        if(!zip.extract(this->_backupItem[i].cdri, app_file)) {
          this->_error = L"Backup file \""+this->_backupItem[i].path+L"\"";
          this->_error += OMM_STR_ERR_ZIPINFL(zip.lastErrorStr());
          this->log(0, L"Package("+this->_ident+L") Uninstall", this->_error);
        }
        // step progress bar
        if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);
        #ifdef DEBUG_SLOW
        Sleep(DEBUG_SLOW); //< for debug
        #endif
      }
    }
    // we do not need this anymore
    zip.close();

  } else {

    if(!Om_isDir(this->_backup)) {
      this->_error =  L"Backup data \""+this->_backup+L"\"";
      this->_error += L" is neither a valid ZIP archive or directory.";
      this->log(0, L"Package("+this->_ident+L") Uninstall", this->_error);
      return false;
    }

    // compose the path to backup root path
    wstring bck_dir_path = this->_backup + L"\\" + this->_backupDir;

    if(!Om_isDir(bck_dir_path)) {
      this->_error = L"Backup data subfolder \""+bck_dir_path+L"\"";
      this->_error += OMM_STR_ERR_ISDIR;
      this->log(0, L"Package("+this->_ident+L") Uninstall", this->_error);
      return false;
    }

    is_zip = false; //< backup data is a sub-directory

    // all appear OK, we can proceed
    wstring bck_file, app_file;

    // first we restore genuine files from zip
    for(size_t i = 0; i < this->_backupItem.size(); ++i) {
      // we are interested only by backup file to copy
      if(this->_backupItem[i].dest == PKGITEM_DEST_CPY) {
        // path to file in the backup sub-directory
        Om_concatPaths(bck_file, bck_dir_path, this->_backupItem[i].path);
        // path to destination file to be overwritten
        Om_concatPaths(app_file, this->_location->_installDir, this->_backupItem[i].path);
        // move file from backup sub-directory

        // Move file from backup directory to destination, notice that the
        // Om_fileDelete + Om_fileMove method is way faster than the
        // Om_fileCopy method
        result = Om_fileDelete(app_file);
        if(result == 0) {
          result = Om_fileMove(bck_file, app_file);
          if(result != 0) {
            this->_error = L"Backup file \""+bck_file+L"\"";
            this->_error += OMM_STR_ERR_MOVE(Om_getErrorStr(result));
            this->log(0, L"Package("+this->_ident+L") Uinstall", this->_error);
          }
        } else {
          this->_error = L"Installed file \""+app_file+L"\"";
          this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
          this->log(0, L"Package("+this->_ident+L") Uinstall", this->_error);
        }
        // step progress bar
        if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);
        #ifdef DEBUG_SLOW
        Sleep(DEBUG_SLOW); //< for debug
        #endif
      }
    }
  }

  // Now delete the added (not modified) files and/or folders by the Package
  // installation.
  //
  // the <del> entries are listed in the depth-first order, so, reading list
  // in backward give us the perfect order to delete elements: from "leaves"
  // to "root".
  // Otherwise, we may  have to request to delete folders before their
  // contents, which simply does not work.

  wstring del_file;

  size_t n = this->_backupItem.size();
  while(n--) {
    // we are interested only by file to cleanup
    if(this->_backupItem[n].dest == PKGITEM_DEST_DEL) {
      // get destination file (to be deleted) path
      Om_concatPaths(del_file, this->_location->_installDir, this->_backupItem[n].path);
      // check whether this is a file or directory
      if(this->_backupItem[n].type == PKGITEM_TYPE_F) {
        result = Om_fileDelete(del_file);
        if(result != 0) {
          this->_error = L"Installed file \""+del_file+L"\"";
          this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
          this->log(1, L"Package("+this->_ident+L") Uinstall", this->_error);
        }
      } else {
        result = Om_dirDelete(del_file);
        if(result != 0) {
          this->_error = L"Installed subfolder \""+del_file+L"\"";
          this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
          this->log(1, L"Package("+this->_ident+L") Uinstall", this->_error);
        }
      }
      // step progress bar
      if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);
      #ifdef DEBUG_SLOW
      Sleep(DEBUG_SLOW); //< for debug
      #endif
    }
  }

  // cleanup backup data either zip file or sub-directory...
  if(is_zip) {
    result = Om_fileDelete(this->_backup);
    if(result != 0) {
      this->_error = L"Backup ZIP archive \""+this->_backup+L"\"";
      this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
      this->log(0, L"Package("+this->_ident+L") Uinstall", this->_error);
    }
  } else {
    result = Om_dirDeleteRecursive(this->_backup);
    if(result != 0) {
      this->_error = L"Backup main directory \""+this->_backup+L"\"";
      this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
      this->log(0, L"Package("+this->_ident+L") Uinstall", this->_error);
    }
  }

  // revoke the backup property of this package
  this->backupClear();

  // making report
  wchar_t log_buf[32];
  swprintf(log_buf, 32, L"Done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);

  log(2, L"Package("+this->_ident+L") Uinstall", log_buf);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::_undoInstall(HWND hPb)
{
  // initialize local timer
  clock_t time = clock();

  // This function restores partial unfinished backup data, avoiding checks
  // for valid package backup entry
  //
  // Its main purpose is to be called after a failed sub-directory backup process
  // to move back the destination genuine file which was moved into the backup
  // sub-directory right before a package installation
  //
  // as the problem does not concern the zipped backup method, this function only
  // restore files from sub-directory backup, and simply cleanup the zipped backup
  // if it exists.

  int result;

  if(Om_isDir(this->_backup)) {

    // compose the path to backup root path
    wstring bck_dir_path = this->_backup + L"\\" + this->_backupDir;

    wstring bck_file, app_file, del_file;

    // first we restore genuine files from zip
    for(size_t i = 0; i < this->_backupItem.size(); ++i) {
      // we are interested only by backup file to copy
      if(this->_backupItem[i].dest == PKGITEM_DEST_CPY) {
        // path to file in the backup sub-directory
        Om_concatPaths(bck_file, bck_dir_path, this->_backupItem[i].path);
        // path to file in the destination to be restored
        Om_concatPaths(app_file, this->_location->_installDir, this->_backupItem[i].path);
        // check whether we need to delete an already installed file, it should
        // be an non-genuine destination file, moved here during an aborted
        // install process
        if(Om_isFile(app_file)) {
          result = Om_fileDelete(app_file);
          if(result != 0) {
            this->_error = L"Installed file \""+app_file+L"\"";
            this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
            this->log(0, L"Package("+this->_ident+L") Undo", this->_error);
          }
        }
        // move the backed file to the destination
        result = Om_fileMove(bck_file, app_file);
        if(result != 0) {
          this->_error = L"Bakcup file \""+bck_file+L"\"";
          this->_error += OMM_STR_ERR_MOVE(Om_getErrorStr(result));
          this->log(1, L"Package("+this->_ident+L") Undo", this->_error);
        }
        // step the progress bar backward
        if(hPb) {
          int p = SendMessageW(hPb, PBM_GETPOS, 0, 0);
          SendMessageW(hPb, PBM_SETPOS, p-1, 0);
        }
        #ifdef DEBUG_SLOW
        Sleep(DEBUG_SLOW); //< for debug
        #endif
      }
    }

    // Now delete the added (not modified) files and/or folders by the Package
    // installation.
    //
    // the <del> entries are listed in the depth-first order, so, reading list
    // in backward give us the perfect order to delete elements: from "leaves"
    // to "root".
    // Otherwise, we may  have to request to delete folders before their
    // contents, which simply does not work.

    size_t n = this->_backupItem.size();
    while(n--) {
      // we are interested only by file to cleanup
      if(this->_backupItem[n].dest == PKGITEM_DEST_DEL) {
        // get destination file (to be deleted) path
        Om_concatPaths(del_file, this->_location->_installDir, this->_backupItem[n].path);
        // check whether this is a file or directory
        if(this->_backupItem[n].type == PKGITEM_TYPE_F) {
          if(Om_isFile(del_file)) {
            result = Om_fileDelete(del_file);
            if(result != 0) {
              this->_error = L"Installed file \""+del_file+L"\"";
              this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
              this->log(1, L"Package("+this->_ident+L") Undo", this->_error);
            }
          }
        } else {
          if(Om_isDir(del_file)) {
            result = Om_dirDelete(del_file);
            if(result != 0) {
              this->_error = L"Installed subfolder \""+del_file+L"\"";
              this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
              this->log(1, L"Package("+this->_ident+L") Undo", this->_error);
            }
          }
        }
        // step the progress bar backward
        if(hPb) {
          int p = SendMessageW(hPb, PBM_GETPOS, 0, 0);
          SendMessageW(hPb, PBM_SETPOS, p-1, 0);
        }
        #ifdef DEBUG_SLOW
        Sleep(DEBUG_SLOW); //< for debug
        #endif
      }
    }
    // cleanup backup sub-directory
    result = Om_dirDeleteRecursive(this->_backup);
    if(result != 0) {
      this->_error = L"Backup main direcotry \""+this->_backup+L"\"";
      this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
      this->log(0, L"Package("+this->_ident+L") Undo", this->_error);
    }

  } else {
    // cleanup backup zip file if exists
    if(Om_isFileZip(this->_backup)) {
      result = Om_fileDelete(this->_backup);
      if(result != 0) {
        this->_error = L"Backup ZIP archive \""+this->_backup+L"\"";
        this->_error += OMM_STR_ERR_DELETE(Om_getErrorStr(result));
        this->log(0, L"Package("+this->_ident+L") Undo", this->_error);
      }
    }
  }

  // revoke the backup property of this package
  this->backupClear();

  // making report
  wchar_t log_buf[32];
  swprintf(log_buf, 32, L"Done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);

  log(2, L"Package("+this->_ident+L") Undo", log_buf);
}
