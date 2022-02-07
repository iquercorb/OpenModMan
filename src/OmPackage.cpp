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

#include "OmZipFile.h"
#include "OmXmlDoc.h"
#include "OmManager.h"
#include "OmContext.h"
#include "OmLocation.h"

#include "OmImage.h"

#include "Util/OmUtilFs.h"
#include "Util/OmUtilStr.h"
#include "Util/OmUtilHsh.h"
#include "Util/OmUtilErr.h"
#include "Util/OmUtilPkg.h"

#include "3rdP/miniz/miniz.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmPackage.h"

#define PKG_BACKUP_DIR    L"root"
#define PKG_BACKUP_DEF    L"backup"
#define PKG_SOURCE_DIR    L"root"
#define PKG_SOURCE_DEF    L"package"


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
static void __get_folder_src_items(vector<OmPkgItem>* ls, const wstring& orig, const wstring& from)
{
  wstring item;
  wstring root;

  OmPkgItem pkg_item;

  wstring srch(orig); srch += L"\\*";
  WIN32_FIND_DATAW fd;
  HANDLE hnd = FindFirstFileW(srch.c_str(), &fd);
  if(hnd != INVALID_HANDLE_VALUE) {
    do {
      // skip this and parent folder
      if(!wcscmp(fd.cFileName, L".")) continue;
      if(!wcscmp(fd.cFileName, L"..")) continue;

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
        __get_folder_src_items(ls, root, item);

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
  _location(nullptr), _type(0), _ident(), _hash(0), _core(), _version(), _name(),
  _src(), _srcDir(), _srcTime(0), _srcItemLs(), _depLs(), _bck(), _bckDir(),
  _bckItemLs(), _ovrLs(), _category(), _desc(), _image(), _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmPackage::OmPackage(OmLocation* pLoc) :
  _location(pLoc), _type(0), _ident(), _hash(0), _core(), _version(),
  _name(), _src(), _srcDir(), _srcTime(0), _srcItemLs(), _depLs(), _bck(),
  _bckDir(), _bckItemLs(), _ovrLs(), _category(), _desc(), _image(), _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmPackage::~OmPackage()
{
  this->_src.clear();
  this->_bck.clear();
  this->_bckDir.clear();
  this->_srcDir.clear();
  this->_srcItemLs.clear();
  this->_bckItemLs.clear();
  this->_ovrLs.clear();
  this->_depLs.clear();
  this->_image.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::isBckOf(const wstring& path)
{
  return (Om_getXXHash3(Om_getFilePart(path)) == _hash);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::srcParse(const wstring& path)
{
  this->_src = path;

  // check if it is a file
  if(Om_isFile(this->_src)) {

    // get Package identity from file name without extension
    this->_ident = Om_getNamePart(this->_src);

    // this is a file, so check if it is a Zip file
    if(Om_isFileZip(this->_src)) {

      // we got a zip file, checks whether it is formated like we expect it
      OmZipFile src_zip;
      if(!src_zip.load(this->_src)) {
        this->_error = Om_errLoad(L"Package file", this->_src, src_zip.lastErrorStr());
        this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
        src_zip.close();
        this->srcClear();
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

        if(Om_extensionMatches(zcd_entry, OMM_PKG_DEF_FILE_EXT)) {
          // good candidate for Package definition, try to load it
          size_t s = src_zip.size(zcd_index);
          char* str_buf = new(std::nothrow) char[s+1];
          if(str_buf) {
            if(!src_zip.extract(zcd_index, str_buf, s)) {
              this->_error = Om_errZipInfl(L"Package definition", zcd_entry, src_zip.lastErrorStr());
              this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
              src_zip.close();
              delete [] str_buf;
              this->srcClear();
              return false;
            }
            str_buf[s] = 0;
            if(src_def.parse(Om_fromUtf8(str_buf), OMM_XMAGIC_PKG)) {
              has_def = true;
              delete [] str_buf;
              break;
            } else {
              this->_error = Om_errParse(L"Package definition", zcd_entry, src_def.lastErrorStr());
              this->log(1, L"Package("+this->_ident+L") Parse Source", this->_error);
            }
            delete [] str_buf;
          } else {
            this->_error = Om_errZipInfl(L"Package definition", zcd_entry, L"bad alloc");
            this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
            src_zip.close();
            this->srcClear();
            return false;
          }
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
          this->srcClear();
          return false;
        }
        // install directory name
        this->_srcDir = cfg_xml.child(L"install").content();
        // make sure src list is empty
        this->_srcItemLs.clear();
        // then we now must create the tree
        OmPkgItem pkg_item;
        pkg_item.dest = PKGITEM_DEST_NUL;
        // here we go to gather files to install
        for(unsigned i = 0; i < zcd_count; ++i) {
          src_zip.index(zcd_entry, i); //< get Central Directory Record entry
          // we check for all entry with the specified destination folder as
          // root, then we get only the relative path for the Package Item.
          if(Om_getRelativePath(pkg_item.path, this->_srcDir, zcd_entry)) {
            // we got one, lets add it to the package tree
            pkg_item.cdri = i; //< keep Zip CDR index
            pkg_item.type = src_zip.indexIsDir(i) ? PKGITEM_TYPE_D : PKGITEM_TYPE_F;
            // add source item and create parent folder items
            this->_srcItemAdd(pkg_item);
          }
        }
        // make sure dependency list is empty
        this->_depLs.clear();
        // check for Package dependencies
        if(cfg_xml.hasChild(L"dependencies")) {
          OmXmlNode dpnd_node = cfg_xml.child(L"dependencies");
          unsigned dpnd_count = dpnd_node.childCount(L"ident");
          for(unsigned i = 0; i < dpnd_count; ++i) {
            this->_depLs.push_back(dpnd_node.child(L"ident", i).content());
          }
        }
        // check for Package category
        if(cfg_xml.hasChild(L"category")) {
          this->_category = cfg_xml.child(L"category").content();
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
            size_t s = src_zip.size(zcrd_index);
            uint8_t* img_data = new(std::nothrow) uint8_t[s];
            if(img_data) {
              if(src_zip.extract(zcrd_index, img_data, s)) {
                // finally load picture data
                if(!this->_image.open(img_data, s, OMM_THUMB_SIZE)) {
                  this->_error = L"Snapshot file \""+pic_name+L"\"";
                  this->_error += L"cannot be loaded: "+_image.lastErrorStr();
                  this->log(1, L"Package("+this->_ident+L") Parse Source", this->_error);
                }
              } else {
                this->_error = Om_errZipInfl(L"Snapshot file", pic_name, src_zip.lastErrorStr());
                this->log(1, L"Package("+this->_ident+L") Parse Source", this->_error);
              }
              delete [] img_data;
            } else {
              this->_error = Om_errZipInfl(L"Snapshot file", pic_name, L"bad alloc");
              this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
            }
          } else {
            this->_error = L"Referenced Snapshot file \""+pic_name+L"\" was not found in archive.";
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
          this->_error =  L"Source archive file \""+this->_src+L"\" is not a valid Package: missing root folder.";
          this->log(1, L"Package("+this->_ident+L") Parse Source", this->_error);
          src_zip.close();
          this->srcClear();
          return false;
        }
        // make sure src list is empty
        this->_srcItemLs.clear();
        // then we now must create the tree
        OmPkgItem pkg_item;
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
            // add source item and create parent folder items
            this->_srcItemAdd(pkg_item);
          }
          // lookup for a readme file to get description
          if(Om_namesMatches(zcd_entry, L"readme.txt")) {
            size_t s = src_zip.size(i);
            char* str_buf = new(std::nothrow) char[s+1];
            if(str_buf) {
              if(src_zip.extract(i, str_buf, s)) {
                str_buf[s] = 0; //< add terminal null
                this->_desc = Om_fromUtf8(str_buf);
              } else {
                this->_error = Om_errZipInfl(L"Readme file", zcd_entry, src_zip.lastErrorStr());
                this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
              }
              delete[] str_buf; //< do not forget to delete buffer
            } else {
              this->_error = Om_errZipInfl(L"Readme file", zcd_entry, L"Bad alloc");
              this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
            }
          }
          // lookup for snapshot
          if(Om_namesMatches(zcd_entry, L"snapshot.jpg") ||
             Om_namesMatches(zcd_entry, L"snapshot.png") ||
             Om_namesMatches(zcd_entry, L"snapshot.bmp") ||
             Om_namesMatches(zcd_entry, L"snapshot.gif")) {
            size_t s = src_zip.size(i);
            uint8_t* img_data = new(std::nothrow) uint8_t[s];
            if(img_data) {
              if(src_zip.extract(i, img_data, s)) {
                if(!this->_image.open(img_data, s, OMM_THUMB_SIZE)) {
                  this->_error = L"Image file \""+zcd_entry+L"\"";
                  this->_error += L"cannot be loaded: "+this->_image.lastErrorStr();
                  this->log(1, L"Package("+this->_ident+L") Parse Source", this->_error);
                }
              } else {
                this->_error = Om_errZipInfl(L"Image file", zcd_entry, src_zip.lastErrorStr());
                this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
              }
              delete [] img_data; //< do not forget to delete buffer
            } else {
              this->_error = Om_errZipInfl(L"Image file", zcd_entry, L"Bad alloc");
              this->log(0, L"Package("+this->_ident+L") Parse Source", this->_error);
            }
          }
        }
      }

      src_zip.close();

      this->_type |= PKG_TYPE_ZIP;

    } else {

      // file but not a zip, skip this...
      this->srcClear();
      return false;
    }
  } else {
    // the source is not a file, maybe this is a folder
    if(Om_isDir(this->_src)) {

      // get Package identity from folder name full name
      this->_ident = Om_getFilePart(this->_src);

      // make sure src list is empty
      this->_srcItemLs.clear();

      // get Package item list from folder content (recursive function)
      __get_folder_src_items(&this->_srcItemLs, this->_src, L"");

    } else {

      // obviously something is wrong
      this->srcClear();
      return false;
    }
  }

  // parse raw name to get display name and potential version
  wstring vers;
  if(Om_parsePkgIdent(this->_name, this->_core, vers, this->_src, this->isZip(), true)) {
    this->_version.parse(vers);
  }

  // store source last write time
  this->_srcTime = Om_itemTime(this->_src);

  // create hash from file name
  this->_hash = Om_getXXHash3(Om_getFilePart(this->_src));

  // This package is now valid source to be installed
  this->_type |= PKG_TYPE_SRC;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::bckParse(const wstring& path)
{
  OmConfig bck_def;

  bool has_def = false;

  this->_bck = path;

  if(Om_isFileZip(this->_bck)) {

    OmZipFile bck_zip;

    // load the backup Zip file
    if(!bck_zip.load(this->_bck)) {
      this->_error = Om_errLoad(L"Backup archive file", this->_bck, bck_zip.lastErrorStr());
      this->log(0, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
      bck_zip.close();
      this->bckClear();
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

      if(Om_extensionMatches(zcd_entry, OMM_BCK_DEF_FILE_EXT)) {
        // good candidate for Package definition, try to load it
        size_t s = bck_zip.size(zcd_index);
        char* str_buf = new(std::nothrow) char[s+1];
        if(str_buf) {
          if(!bck_zip.extract(zcd_index, str_buf, s)) {
            this->_error = Om_errZipInfl(L"Backup definition", zcd_entry, bck_zip.lastErrorStr());
            this->log(0, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
            bck_zip.close();
            delete [] str_buf;
            this->bckClear();
            return false;
          }
          str_buf[s] = 0;
          if(bck_def.parse(Om_fromUtf8(str_buf), OMM_XMAGIC_BCK)) {
            has_def = true;
            delete [] str_buf;
            break;
          } else {
            this->_error = Om_errParse(L"Backup definition", zcd_entry, bck_def.lastErrorStr());
            this->log(1, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
          }
          delete [] str_buf;
        } else {
          this->_error = Om_errZipInfl(L"Backup definition", zcd_entry, L"Bad alloc");
          this->log(0, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
          bck_zip.close();
          this->bckClear();
          return false;
        }
      }
    }

    if(!has_def) {
      this->_error =  L"Invalid backup archive file: Backup definition missing.";
      this->log(1, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
      this->bckClear();
      return false;
    }

    // we do not need the zip file anymore
    bck_zip.close();

  } else {

    if(!Om_isDir(this->_bck)) {
      this->_error =  L"Found invalid backup: File is not a ZIP archive.";
      this->log(1, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
      this->bckClear();
      return false;
    }

    // search a Backup definition file within the sub-folder
    vector<wstring> ls;
    Om_lsFile(&ls, this->_bck, true);

    for(size_t i = 0; i < ls.size(); ++i) {
      if(Om_extensionMatches(ls[i], OMM_BCK_DEF_FILE_EXT)) {
        if(bck_def.open(ls[i], OMM_XMAGIC_BCK)) {
          has_def = true;
          break;
        } else {
          this->_error = Om_errParse(L"Backup definition", ls[i], bck_def.lastErrorStr());
          this->log(1, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
        }
      }
    }

    if(!has_def) {
      this->_error = L"Invalid backup: Definition file missing.";
      this->log(0, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
      this->bckClear();
      return false;
    }
  }

  OmXmlNode def_xml = bck_def.xml();

  // verify the Backup definition have the proper nodes
  if(def_xml.hasChild(L"backup")) {
    this->_bckDir = def_xml.child(L"backup").content();
  } else {
    this->_error = L"Invalid definition : <backup> node missing.";
    this->log(0, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
    this->bckClear();
    return false;
  }
  if(def_xml.hasChild(L"ident")) {
    this->_ident = def_xml.child(L"ident").content();
  } else {
    this->_error = L"Invalid definition : <ident> node missing.";
    this->log(0, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
    this->bckClear();
    return false;
  }
  if(def_xml.hasChild(L"hash")) {
    this->_hash = Om_toUint64(def_xml.child(L"hash").content());
  } else {
    this->_error = L"Invalid definition : <hash> node missing.";
    this->log(0, L"Package("+Om_getFilePart(this->_bck)+L") Parse Backup", this->_error);
    this->bckClear();
    return false;
  }

  // If name and version was not previously parsed as source, pars here using
  // the identity string saved in backup definition.
  if(!this->isType(PKG_TYPE_SRC)) {
    wstring vers;
    if(Om_parsePkgIdent(this->_name, this->_core, vers, this->_ident, false, true)) {
      this->_version.parse(vers);
    }
  }

  this->_bckItemLs.clear();
  OmPkgItem bck_item;
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
    this->_bckItemLs.push_back(bck_item);
  }
  item_count = def_xml.childCount(L"del");
  for(unsigned i = 0; i < item_count; ++i) {
    xml_item = def_xml.child(L"del", i);
    bck_item.path = xml_item.content();
    bck_item.type = (xml_item.attrAsInt(L"dir")>0)?PKGITEM_TYPE_D:PKGITEM_TYPE_F;
    bck_item.cdri = -1;
    bck_item.dest = PKGITEM_DEST_DEL;
    this->_bckItemLs.push_back(bck_item);
  }

  this->_ovrLs.clear();
  // retrieve the backup overlap list, they are stored as a list of Hash values
  // corresponding to package file name
  if(def_xml.hasChild(L"overlap")) {
    OmXmlNode xml_ovlap = def_xml.child(L"overlap");
    unsigned hash_count = xml_ovlap.childCount(L"hash");
    for(unsigned i = 0; i < hash_count; ++i) {
      xml_item = xml_ovlap.child(L"hash", i);
      this->_ovrLs.push_back(Om_toUint64(xml_item.content()));
    }
  }

  // All appear OK, we finalizes our package setup
  this->_type |= PKG_TYPE_BCK;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::srcClear()
{
  // remove the source flag
  this->_type &= ~PKG_TYPE_SRC;
  this->_type &= ~PKG_TYPE_ZIP;
  this->_depLs.clear();
  this->_src.clear();
  this->_srcDir.clear();
  this->_srcTime = 0;
  this->_srcItemLs.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::bckClear()
{
  // remove the backup flag
  this->_type &= ~PKG_TYPE_BCK;
  this->_ovrLs.clear();
  this->_bck.clear();
  this->_bckDir.clear();
  this->_bckItemLs.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::srcValid()
{
  if(this->_type & PKG_TYPE_SRC) {

    // this is brutal but safe, we simply re-parse the previously
    // parsed source to ensure everything is up to date
    if(!this->srcParse(this->_src)) {
      this->srcClear();
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
bool OmPackage::bckValid()
{
  if(this->_type & PKG_TYPE_BCK) {

    // this is brutal but safe, we simply re-parse the previously
    // parsed source to ensure everything is up to date
    if(!this->bckParse(this->_bck)) {
      this->bckClear();
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
bool OmPackage::uninst(Om_progressCb progress_cb, void* user_ptr)
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
  if(!this->bckValid()) {
    this->_error = L"Backup data should exist but was not found.";
    this->log(0, L"Package("+this->_ident+L") Uninstall", this->_error);
    return false;
  }

  // initialize the progression values
  size_t progress_cur, progress_tot;
  if(progress_cb) {
    progress_tot = this->_bckItemLs.size();
    progress_cur = 0;
    // it still time to abort
    if(!progress_cb(user_ptr, progress_tot, progress_cur, 0)) {
      this->log(1, L"Package("+this->_ident+L") Uninstall", L"Aborted.");
      return true;
    }
  }

  // restore backed files into destination tree
  if(!this->_restore(progress_cb, user_ptr)) {
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::install(unsigned zipLvl, Om_progressCb progress_cb, void* user_ptr)
{
  // cannot install without valid Location
  if(this->_location == nullptr) {
    this->_error = L"Package cannot be installed out of a valid Location.";
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
  if(!this->srcValid()) {
    this->_error = L"Source data should exist but was not found.";
    this->log(0, L"Package("+this->_ident+L") Install", this->_error);
    return false;
  }

  // initialize the progression values
  size_t progress_cur, progress_tot;
  if(progress_cb) {
    progress_tot = 2 * this->_srcItemLs.size();
    progress_cur = 0;
    // it still time to abort
    if(!progress_cb(user_ptr, progress_tot, progress_cur, 0)) {
      this->log(1, L"Package("+this->_ident+L") Install", L"Aborted.");
      return true;
    }
  }

  // Step 1 : Create backups of destination files overwritten by package
  if(!this->_backup(zipLvl, progress_cb, user_ptr)) {
    return false;
  }

  // Step 2 : Install package files into destination tree
  if(!this->_apply(progress_cb, user_ptr)) {
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::unbackup()
{
  // cannot install without valid Location
  if(this->_location == nullptr) {
    this->_error = L"Package cannot be uninstalled out of a valid Location.";
    this->log(0, L"Package("+this->_ident+L") Unbackup", this->_error);
    return false;
  }

  if(!(this->_type & PKG_TYPE_BCK)) {
    // I am sorry Dave, but there is no backup to restore
    this->_error = L"Backup data does not exists.";
    this->log(0, L"Package("+this->_ident+L") Unbackup", this->_error);
    return false;
  }

  // ultimate validity check before try to install
  if(!this->bckValid()) {
    this->_error = L"Backup data should exist but was not found.";
    this->log(0, L"Package("+this->_ident+L") Unbackup", this->_error);
    return false;
  }

  // restore backed files into destination tree
  if(!this->_discard()) {
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::bckItemHas(const wstring& path, OmPkgItemDest dest) const
{
  for(size_t i = 0; i < this->_bckItemLs.size(); ++i) {

    if(this->_bckItemLs[i].dest == dest) {
      if(this->_bckItemLs[i].path == path)
        return true;
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::footprint(vector<OmPkgItem>& footprint) const
{
  // simulate package installation to have backup item list
  wstring path;
  OmPkgItem  item;

  for(size_t i = 0; i < this->_srcItemLs.size(); ++i) {

    Om_concatPaths(path, this->_location->_dstDir, this->_srcItemLs[i].path);

    item.path = this->_srcItemLs[i].path;
    item.type = this->_srcItemLs[i].type;
    item.cdri = -1;

    item.dest = Om_pathExists(path) ? PKGITEM_DEST_CPY : PKGITEM_DEST_DEL;

    footprint.push_back(item);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::ovrTest(const vector<OmPkgItem>& footprint) const
{
  if(footprint.empty())
    return false;

  for(size_t i = 0; i < this->_srcItemLs.size(); ++i) {

    // we care only about files
    if(this->_srcItemLs[i].type != PKGITEM_TYPE_F)
      continue;

    for(size_t j = 0; j < footprint.size(); ++j) {

      // we care only about files
      if(footprint[j].type != PKGITEM_TYPE_F)
        continue;

      // same path mean overlap
      if(this->_srcItemLs[i].path == footprint[j].path) {
        return true;
      }
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::loadImage(const wstring& path, unsigned size)
{
  this->_image.clear();
  this->_image.open(path, size);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::clearImage()
{
  this->_image.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::save(const wstring& out_path, unsigned zipLvl, Om_progressCb progress_cb, void* user_ptr)
{
  // initialize local timer
  clock_t time = clock();

  if(!(this->_type & PKG_TYPE_ZIP)) {
    if(Om_isDir(this->_src)) {
      if(!Om_checkAccess(this->_src, OMM_ACCESS_DIR_READ)) {
        this->_error = Om_errReadAccess(L"Source folder", this->_src);
        this->log(0, L"Package("+out_path+L") Save", this->_error);
        return false;
      }
    } else {
      this->_error = Om_errIsDir(L"Source folder", this->_src);
      this->log(0, L"Package("+out_path+L") Save", this->_error);
      return false;
    }
  }
/*
  if(!this->_srcItemLs.size()) {
    this->_error = L"Package does not contain any data to be saved.";
    this->log(0, L"Package("+out_path+L") Save", this->_error);
    return false;
  }
*/
  // create package identity according destination path
  wstring pkg_ext = Om_getFileExtPart(out_path);
  wstring pkg_ident = Om_getNamePart(out_path);

  OmZipFile src_zip;

  int result;
  bool has_failed = false;
  bool has_aborted = false;

  // do we got a ZIP file or a Folder
  if(this->_type & PKG_TYPE_ZIP) {
    if(!src_zip.load(this->_src)) {
      this->_error = Om_errLoad(L"Input Package file", this->_src, src_zip.lastErrorStr());
      this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
      src_zip.close();
      return false;
    }
  }

  OmZipFile pkg_zip;

  // use temporary file name, in case source and destination are the same
  wstring pkg_tmp_path;
  Om_concatPaths(pkg_tmp_path, Om_getDirPart(out_path), pkg_ident);
  pkg_tmp_path += L".ztmp";

  // initialize zip archive
  if(!pkg_zip.init(pkg_tmp_path)) {
    this->_error = Om_errInit(L"Package (temp) file", out_path, pkg_zip.lastErrorStr());
    this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
    pkg_zip.close();
    Om_fileDelete(pkg_tmp_path);
    return false;
  }

  OmConfig pkg_def;
  pkg_def.init(OMM_XMAGIC_PKG);

  OmXmlNode  def_xml = pkg_def.xml();

  // defines the package source directory
  def_xml.addChild(L"install").setContent(pkg_ident);

  // initialize the progression values
  size_t progress_cur, progress_tot;
  if(progress_cb) {
    progress_tot = this->_srcItemLs.size();
    progress_cur = 0;
    progress_cb(user_ptr, progress_tot, progress_cur, 0);
  }

  wstring src_path, zcd_entry;

  uint8_t* file_data;
  size_t   file_size;

  for(size_t i = 0; i < this->_srcItemLs.size(); ++i) {

    // call progression callback
    if(progress_cb) {
      progress_cur++;
      if(!progress_cb(user_ptr, progress_tot, progress_cur, 0)) {
        has_aborted = true;
        break;
      }
    }
    #ifdef DEBUG
    Sleep(OMM_DEBUG_SLOW); //< for debug
    #endif

    // destination zip path, with mirror folder preceding
    Om_concatPaths(zcd_entry, pkg_ident, this->_srcItemLs[i].path);
    // check if we have a file or folder to install
    if(this->_srcItemLs[i].type == PKGITEM_TYPE_F) { //< this is a file
      // if zip Source, extract from zip to append to other
      if(this->_type & PKG_TYPE_ZIP) {
        // create new buffer to load source zip content
        if((file_size = src_zip.size(this->_srcItemLs[i].cdri)) != 0) {
          file_data = new(std::nothrow) uint8_t[file_size];
          if(file_data) {
            // extract source zip content to buffer
            if(!src_zip.extract(this->_srcItemLs[i].cdri, file_data, file_size)) {
              this->_error = Om_errZipInfl(L"Item (file)", this->_srcItemLs[i].path, src_zip.lastErrorStr());
              this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
              has_failed = true;
              delete [] file_data;
              break;
            }
            // append data to destination zip archive
            if(!pkg_zip.append(file_data, file_size, zcd_entry, zipLvl)) {
              this->_error = Om_errZipDefl(L"Item (file)", zcd_entry, pkg_zip.lastErrorStr());
              this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
              has_failed = true;
              delete [] file_data;
              break;
            }
            delete [] file_data;
          } else {
            this->_error = Om_errZipInfl(L"Item (file)", this->_srcItemLs[i].path, L"Bad alloc");
            this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
            has_failed = true;
            break;
          }
        }
      } else {
        // path to item in package sub-directory
        Om_concatPaths(src_path, this->_src, this->_srcItemLs[i].path);
        // add file to destination zip archive
        if(!pkg_zip.append(src_path, zcd_entry, zipLvl)) {
          this->_error = Om_errZipDefl(L"Item (file)", zcd_entry, pkg_zip.lastErrorStr());
          this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
          has_failed = true;
          break;
        }
      }
    } else {
      // add folder to destination zip archive
      if(!pkg_zip.append(nullptr, 0, zcd_entry, zipLvl)) {
        this->_error = Om_errZipDefl(L"Item (directory)", zcd_entry, pkg_zip.lastErrorStr());
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
    return false;
  }

  if(has_aborted) {
    pkg_zip.close();
    Om_fileDelete(pkg_tmp_path);
    return true;
  }

  // add category to source definition
  if(!this->_category.empty()) {
    def_xml.addChild(L"category").setContent(this->_category);
  }

  // add dependency to source definition
  if(this->_depLs.size()) {
    OmXmlNode xml_dpnd = def_xml.addChild(L"dependencies");
    for(size_t i = 0; i < this->_depLs.size(); ++i) {
      xml_dpnd.addChild(L"ident").setContent(this->_depLs[i]);
    }
  }

  // add description to source definition
  if(!this->_desc.empty()) {
    def_xml.addChild(L"description").setContent(this->_desc);
  }

  // add image to archive and source definition
  if(this->_image.valid()) {
    // check image type to create file name
    switch(this->_image.data_type())
    {
    case 1: zcd_entry = L"snapshot.bmp"; break;
    case 2: zcd_entry = L"snapshot.jpg"; break;
    case 3: zcd_entry = L"snapshot.png"; break;
    case 4: zcd_entry = L"snapshot.gif"; break;
    }
    // add image in zip archive
    if(!pkg_zip.append(this->_image.data(), this->_image.data_size(), zcd_entry, zipLvl)) {
      this->_error = Om_errZipDefl(L"Snapshot file", zcd_entry, pkg_zip.lastErrorStr());
      this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
      pkg_zip.close();
      Om_fileDelete(pkg_tmp_path);
      return false;
    }
    // add section to source definition
    def_xml.addChild(L"picture").setContent(zcd_entry);
  }

  // Compose and add a REAMDE.TXT file into archive
  string pkg_readme;

  if(!this->_desc.empty()) {
    Om_toAnsiCp(pkg_readme, this->_desc);
    pkg_readme += "\r\n"
    "\r\n"
    "-- END OF DESCRIPTION ---------------------------------------------------------"
    "\r\n"
    "\r\n";
  }

  pkg_readme += "Open Mod Manager Package file for \"";
  pkg_readme += Om_toUtf8(pkg_ident); pkg_readme += "\" Mod.\r\n"
  "\r\n"
  "This Mod Package was created using Open Mod Manager and is intended to be\r\n"
  "installed using Open Mod Manager or any other compatible software.\r\n\r\n"
  "If you want to install this Mod manually, you will find the Mod files into\r\n"
  "the following folder : \r\n"
  "\r\n  \""; pkg_readme += Om_toUtf8(pkg_ident);
  pkg_readme += "\"\r\n"
  "\r\n"
  "Its content is respecting the destination folder tree and includes files to\r\n"
  "be overwritten or added :\r\n"
  "\r\n";
  for(size_t i = 0; i < this->_srcItemLs.size(); ++i) {
    pkg_readme += "   ";
    pkg_readme += Om_toUtf8(this->_srcItemLs[i].path);
    pkg_readme += "\r\n";
  }
  pkg_readme += "\r\n"
  "Once you made a backup of the original files, you can install the Mod by\r\n"
  "extracting the content of the previously indicated folder into the\r\n"
  "proper application or game folder, overwriting original files.\r\n"
  "\r\n"
  "For more information about Open Mod Manager and Open Mod Packages, please\r\n"
  "visit :\r\n"
  "\r\n   "; pkg_readme += Om_toUtf8(OMM_APP_URL);

  // add the REAMDE.TXT file in zip archive
  if(!pkg_zip.append(pkg_readme.c_str(), pkg_readme.size(), L"README.TXT", zipLvl)) {
    this->_error = Om_errZipDefl(L"Readme file", L"README.TXT", pkg_zip.lastErrorStr());
    this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
    pkg_zip.close();
    Om_fileDelete(pkg_tmp_path);
    return false;
  }

  // compose source definition file name
  wstring pkg_def_name = PKG_SOURCE_DEF;
  pkg_def_name += L".";
  pkg_def_name += OMM_PKG_DEF_FILE_EXT;

  // get XML backup data
  string pkg_def_data = pkg_def.data();

  // add the definition file in zip archive
  if(!pkg_zip.append(pkg_def_data.c_str(), pkg_def_data.size(), pkg_def_name, zipLvl)) {
    this->_error = Om_errZipDefl(L"Definition file", pkg_def_name, pkg_zip.lastErrorStr());
    this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
    pkg_zip.close();
    Om_fileDelete(pkg_tmp_path);
    return false;
  }

  // finalize destination zip file
  pkg_zip.close();

  // compose the definitive package filename
  wstring pkg_path;
  Om_concatPaths(pkg_path, Om_getDirPart(out_path), pkg_ident);
  pkg_path += L"." + pkg_ext;

  // rename temporary file to its final name, this will replace
  // the original file if exists
  result = Om_fileMove(pkg_tmp_path, pkg_path);
  if(result != 0) {
    this->_error = Om_errRename(L"Package (temp) file", pkg_tmp_path, result);
    this->log(0, L"Package("+pkg_ident+L") Save", this->_error);
    Om_fileDelete(pkg_tmp_path);
    return false;
  }

  // making report
  wchar_t log_buf[32];
  swprintf(log_buf, 32, L"Done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);
  this->log(2, L"Package("+pkg_ident+L") Save", log_buf);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmPackage::clear()
{
  this->_type = 0;
  this->_hash = 0;
  this->_image.clear();
  this->_name.clear();
  this->_ident.clear();
  this->_src.clear();
  this->_srcDir.clear();
  this->_srcTime = 0;
  this->_srcItemLs.clear();
  this->_depLs.clear();
  this->_bck.clear();
  this->_bckDir.clear();
  this->_bckItemLs.clear();
  this->_ovrLs.clear();
  this->_category.clear();
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
void OmPackage::_srcItemAdd(const OmPkgItem& item) {

  if(item.type == PKGITEM_TYPE_F) {

    // we decompose the item path to add parent folder as source item
    // to properly and robustly track folder creation and deletion.

    bool exists;

    OmPkgItem parent;
    parent.type = PKGITEM_TYPE_D;
    parent.dest = PKGITEM_DEST_NUL;
    parent.cdri = -1;

    size_t s = 0;

    while((s = item.path.find(L'\\',s+1)) != wstring::npos) {

      parent.path = item.path.substr(0, s) + L"\\";

      // check whether directory item already exists
      exists = false;

      for(size_t i = 0; i < this->_srcItemLs.size(); ++i) {
        if(this->_srcItemLs[i].type == PKGITEM_TYPE_D) {
          if(this->_srcItemLs[i].path == parent.path) {
            exists = true;
            break;
          }
        }
      }

      if(!exists) {
        #ifdef DEBUG
        std::wcout << L"DEBUG => OmPackage::_srcItemAdd : add parent : " << parent.path << L"\n";
        #endif
        this->_srcItemLs.push_back(parent);
      }
    }
  }

  this->_srcItemLs.push_back(item);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::_backup(int zipLvl, Om_progressCb progress_cb, void* user_ptr)
{
  #ifdef DEBUG
  std::wcout << L"DEBUG => OmPackage::_backup\n";
  #endif

  // initialize local timer
  clock_t time = clock();

  // initialize progression callback
  size_t progress_tot, progress_cur;
  if(progress_cb) {
    progress_tot = this->_srcItemLs.size() * 2;   //< backup + install
    progress_cur = 0;
    if(!progress_cb(user_ptr, progress_tot, progress_cur, 0)) {
      return true;
    }
  }

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
  this->_bckDir = PKG_BACKUP_DIR;

  // path to backup root path, mainly used for sub-directory backup
  wstring bck_dir_path;

  // base file name for backup file or sub-directory
  wstring bck_file_name(Om_getFilePart(this->_src));

  if(is_zip) {
    // backup zip file name
    bck_file_name += L".";
    bck_file_name += OMM_BCK_FILE_EXT;
    // backup zip full path
    this->_bck = this->_location->_bckDir; //< Location Backup location
    this->_bck += L"\\";
    this->_bck += bck_file_name;

    // initialize zip archive
    if(!bck_zip.init(this->_bck)) {
      this->_error = Om_errInit(L"Archive file", this->_bck, bck_zip.lastErrorStr());
      this->log(0, L"Package("+this->_ident+L") Backup", this->_error);

      bck_zip.close();
      Om_fileDelete(this->_bck);
      this->bckClear();
      return false;
    }
  } else { // Sub-Directory backup
    // backup sub-directory full path
    this->_bck = this->_location->_bckDir; //< Location Backup location
    this->_bck += L"\\";
    this->_bck += bck_file_name;

    // create sub-directory into Backup location
    result = Om_dirCreate(this->_bck);
    if(result != 0) {
      this->_error = Om_errCreate(L"Main folder", this->_bck, result);
      this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
      this->bckClear();
      return false;
    }

    // compose the path to backup root path
    bck_dir_path = this->_bck + L"\\" + this->_bckDir;

    // create backup data root folder in Backup sub-directory
    result = Om_dirCreate(bck_dir_path);
    if(result != 0) {
      this->_error = Om_errCreate(L"Root folder", bck_dir_path, result);
      this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
      this->bckClear();
      return false;
    }
  }

  // retrieve the list of overlapped package
  vector<uint64_t> ovlap_list;
  this->_location->pkgFindOverlaps(ovlap_list, this);

  // compose the Backup definition file name
  wstring back_def_name = PKG_BACKUP_DEF;
  back_def_name += L".";
  back_def_name += OMM_BCK_DEF_FILE_EXT;

  OmConfig bck_def;
  if(is_zip) {
    bck_def.init(OMM_XMAGIC_BCK);
  } else {
    bck_def.init(this->_bck + L"\\" + back_def_name, OMM_XMAGIC_BCK);
  }

  OmXmlNode  def_xml = bck_def.xml();

  def_xml.addChild(L"ident").setContent(this->_ident);
  def_xml.addChild(L"hash").setContent(Om_toHexString(this->_hash));

  // define backup root directory
  def_xml.addChild(L"backup").setContent(this->_bckDir);

  // it should be already empty, but we want to be sure
  this->_bckItemLs.clear();

  // stuff for Backup item list generation
  OmPkgItem  bck_item;
  OmXmlNode  xml_item;
  wstring app_file, bck_file, zcd_entry, bck_tree;

  for(size_t i = 0, z = 0; i < this->_srcItemLs.size(); ++i) {

    // path to item in the destination tree
    Om_concatPaths(app_file, this->_location->_dstDir, this->_srcItemLs[i].path);
    // our future Backup item entry
    bck_item.path = this->_srcItemLs[i].path;
    bck_item.type = this->_srcItemLs[i].type;
    // check if item exists in destination tree
    if(Om_pathExists(app_file)) {
      // if item is a file, save to zip archive
      if(this->_srcItemLs[i].type == PKGITEM_TYPE_F) { //< this is a file
        // add a 'copy' entry into backup definition
        xml_item = def_xml.addChild(L"cpy");
        xml_item.setContent(this->_srcItemLs[i].path);
        xml_item.setAttr(L"dir", 0);
        xml_item.setAttr(L"cdi", (int)z); //< CDR index in zip
        if(is_zip) {
          // backup file path in zip archive
          Om_concatPaths(zcd_entry, this->_bckDir, this->_srcItemLs[i].path);
          // add file to zip archive
          if(!bck_zip.append(app_file, zcd_entry, zipLvl)) {
            this->_error = Om_errZipDefl(L"File", app_file, bck_zip.lastErrorStr());
            this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
            // notify process failed and break loop
            has_failed = true;
            break;
          }
        } else {
          // path of item in backup directory
          Om_concatPaths(bck_file, bck_dir_path, this->_srcItemLs[i].path);
          // create the folder tree if needed
          bck_tree = Om_getDirPart(bck_file);
          if(!Om_isDir(bck_tree)) {
            // in case the package source is a zip file, the zip may not
            // contain each sub-directories as individual CDR entries, so the
            // folder tree where to move files may not be previously created.
            result = Om_dirCreateRecursive(bck_tree);
            if(result != 0) {
              this->_error = Om_errCreate(L"Folder tree", bck_tree, result);
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
            this->_error = Om_errMove(L"File", app_file, result);
            this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
            // notify process failed and break loop
            has_failed = true;
            break;
          }
        }

        // this thing is now part of backup tree
        bck_item.cdri = z;
        bck_item.dest = PKGITEM_DEST_CPY;
        this->_bckItemLs.push_back(bck_item);

        z++; //< increment CDR index

      } else {

        // check whether folder was created by another package, in this case
        // we add it as to be deleted by this one too, this allow to
        // automatically delete unused shared folders when empty by any package
        if(this->_location->bckItemExists(bck_item.path, PKGITEM_DEST_DEL)) {
          #ifdef DEBUG
          std::wcout << L"DEBUG => OmPackage::_backup : shared temp dir: " << bck_item.path << L"\n";
          #endif
          // add a "To be deleted" entry into backup definition
          xml_item = def_xml.addChild(L"del");
          xml_item.setContent(this->_srcItemLs[i].path);
          xml_item.setAttr(L"dir", 1);
          // this thing is now part of backup tree
          bck_item.cdri = -1;
          bck_item.dest = PKGITEM_DEST_DEL;
          this->_bckItemLs.push_back(bck_item);
        }

        if(!is_zip) {
          // path of item in backup directory
          Om_concatPaths(bck_file, bck_dir_path, this->_srcItemLs[i].path);
          // we simply create the folder in backup directory
          result = Om_dirCreate(bck_file);
          if(result != 0) {
            this->_error = Om_errCreate(L"Folder", bck_file, result);
            this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
            // notify process failed and break loop
            has_failed = true;
            break;
          }
        }
      }

    } else { // item doesn't exists in destination, it should be deleted at restore
      // add a "To be deleted" entry into backup definition
      xml_item = def_xml.addChild(L"del");
      xml_item.setContent(this->_srcItemLs[i].path);
      xml_item.setAttr(L"dir", this->_srcItemLs[i].type == PKGITEM_TYPE_F ? 0 : 1 );

      // this thing is now part of backup tree
      bck_item.cdri = -1;
      bck_item.dest = PKGITEM_DEST_DEL;
      this->_bckItemLs.push_back(bck_item);
    }

    // call progression callback
    if(progress_cb) {
      progress_cur++;
      if(!progress_cb(user_ptr, progress_tot, progress_cur, 0)) {
        has_aborted = true;
        break;
      }
    }

    #ifdef DEBUG
    Sleep(OMM_DEBUG_SLOW); //< for debug
    #endif
  }

  // process abortion requested
  if(has_aborted) {
    this->log(1, L"Package("+this->_ident+L") Backup", L"Aborted.");
    if(is_zip) bck_zip.close(); //< make sure file is not longer used in order to delete it
    this->_restore(progress_cb, user_ptr, true); //< undo
    return true;
  }

  // this mean the backup process encountered error, we undo and return
  if(has_failed) {
    this->log(1, L"Package("+this->_ident+L") Backup", L"Failed.");
    if(is_zip) bck_zip.close(); //< make sure file is not longer used in order to delete it
    this->_restore(progress_cb, user_ptr, true); //< undo
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
      this->_error = Om_errZipDefl(L"Definition file", back_def_name, bck_zip.lastErrorStr());
      this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
      bck_zip.close();
      Om_fileDelete(this->_bck);
      return false;
    }
    // finalize and close zip archive
    bck_zip.close();
  } else {
    // save XML backup data
    if(!bck_def.save()) {
      Om_errSave(L"Backup definition", back_def_name, bck_def.lastErrorStr());
      this->log(0, L"Package("+this->_ident+L") Backup", this->_error);
      Om_dirDeleteRecursive(this->_bck);
      return false;
    }
  }

  // backup is done without error, we now can update the local overlap list
  if(ovlap_list.size()) {
    for(size_t i = 0; i < ovlap_list.size(); ++i) {
      this->_ovrLs.push_back(ovlap_list[i]);
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
bool OmPackage::_apply(Om_progressCb progress_cb, void* user_ptr)
{
  #ifdef DEBUG
  std::wcout << L"DEBUG => OmPackage::_apply\n";
  #endif

  // initialize local timer
  clock_t time = clock();

  // initialize progression callback
  size_t progress_tot, progress_cur;
  if(progress_cb) {
    progress_tot = this->_srcItemLs.size() * 2;   //< item we must install
    progress_cur = this->_srcItemLs.size();       //< start at half the total, backup was the first half
    if(!progress_cb(user_ptr, progress_tot, progress_cur, 0)) {
      return true;
    }
  }

  OmZipFile zip;
  int result;
  bool has_failed = false;
  bool has_aborted = false;

  wstring src_file, app_file;

  // do we got a Zip file or a legacy Folder
  if(this->_type & PKG_TYPE_ZIP) {
    if(!zip.load(this->_src)) {
      this->_error = Om_errLoad(L"Package file", this->_src, zip.lastErrorStr());
      this->log(0, L"Package("+this->_ident+L") Install", this->_error);
      zip.close();
      this->_restore(progress_cb, user_ptr, true); //< undo
      return false;
    }
  }

  for(size_t i = 0; i < this->_srcItemLs.size(); ++i) {

    // path to destination file to be overwritten
    Om_concatPaths(app_file, this->_location->_dstDir, this->_srcItemLs[i].path);
    // check if we have a file or folder to install
    if(this->_srcItemLs[i].type == PKGITEM_TYPE_F) { //< this is a file

      if(this->_type & PKG_TYPE_ZIP) {
        // extract to destination
        if(!zip.extract(this->_srcItemLs[i].cdri, app_file)) {
          this->_error = Om_errZipInfl(L"Package file", this->_srcItemLs[i].path, zip.lastErrorStr());
          this->log(0, L"Package("+this->_ident+L") Install", this->_error);
          has_failed = true;
          break;
        }
      } else {
        // path to item in package sub-directory
        Om_concatPaths(src_file, this->_src, this->_srcItemLs[i].path);
        // Copy and overwrite
        result = Om_fileCopy(src_file, app_file, true);
        if(result != 0) {
          this->_error = Om_errCopy(L"File", this->_srcItemLs[i].path, result);
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
          this->_error = Om_errCreate(L"Folder", this->_srcItemLs[i].path, result);
          this->log(0, L"Package("+this->_ident+L") Install", this->_error);
          has_failed = true;
          break;
        }
      }
    }
    // call progression callback
    if(progress_cb) {
      progress_cur++;
      if(!progress_cb(user_ptr, progress_tot, progress_cur, 0)) {
        has_aborted = true;
        break;
      }
    }
    #ifdef DEBUG
    Sleep(OMM_DEBUG_SLOW); //< for debug
    #endif
  }

  // process abortion requested
  if(has_aborted) {
    this->log(1, L"Package("+this->_ident+L") Install", L"Aborted.");
    if(this->_type & PKG_TYPE_ZIP) zip.close();
    this->_restore(progress_cb, user_ptr, true); //< undo
    return true;
  }

  // this mean the backup process encountered error, we undo and return
  if(has_failed) {
    this->log(1, L"Package("+this->_ident+L") Install", L"Failed.");
    if(this->_type & PKG_TYPE_ZIP) zip.close();
    this->_restore(progress_cb, user_ptr, true); //< undo
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
bool OmPackage::_restore(Om_progressCb progress_cb, void* user_ptr, bool undo)
{
  #ifdef DEBUG
  std::wcout << L"DEBUG => OmPackage::_restore\n";
  #endif

  // initialize local timer
  clock_t time = clock();

  // initialize progression callback
  size_t progress_tot, progress_cur;
  if(progress_cb) {
    if(undo) {
      progress_tot = this->_bckItemLs.size() * 2; //< item we must restore
      progress_cur = this->_bckItemLs.size(); //< start at half the total
    } else {
      progress_tot = this->_bckItemLs.size();   //< item we must restore
      progress_cur = 0;
    }
    if(!progress_cb(user_ptr, progress_tot, progress_cur, 0) && !undo) { //< do not abort if undo
      return true;
    }
  }

  int result;
  bool is_zip;

  if(Om_isFileZip(this->_bck)) {

    OmZipFile zip;
    if(!zip.load(this->_bck)) {
      this->_error = Om_errLoad(L"Backup archive file", this->_bck, zip.lastErrorStr());
      this->log(0, L"Package("+this->_ident+L") Restore", this->_error);
      return false;
    }

    is_zip = true; //< backup data is zip file

    wstring app_file;

    // first we restore genuine files from zip
    for(size_t i = 0; i < this->_bckItemLs.size(); ++i) {
      // we are interested only by backup file to copy
      if(this->_bckItemLs[i].dest == PKGITEM_DEST_CPY) {
        // path to installed file to be overwritten
        Om_concatPaths(app_file, this->_location->_dstDir, this->_bckItemLs[i].path);
        // extract from zip
        if(!zip.extract(this->_bckItemLs[i].cdri, app_file)) {
          this->_error = Om_errZipInfl(L"Backup file", this->_bckItemLs[i].path, zip.lastErrorStr());
          this->log(0, L"Package("+this->_ident+L") Restore", this->_error);
        }
        // call progression callback
        if(progress_cb) {
          if(undo) {
            progress_cur--; //< for undo we step backward
          } else {
            progress_cur++;
          }
          progress_cb(user_ptr, progress_tot, progress_cur, 0);
        }
        #ifdef DEBUG
        Sleep(OMM_DEBUG_SLOW); //< for debug
        #endif
      }
    }
    // we do not need this anymore
    zip.close();

  } else {

    if(!Om_isDir(this->_bck)) {
      this->_error =  L"Backup data \""+this->_bck+L"\"";
      this->_error += L" is neither a valid ZIP archive or directory.";
      this->log(0, L"Package("+this->_ident+L") Restore", this->_error);
      return false;
    }

    // compose the path to backup root path
    wstring bck_dir_path = this->_bck + L"\\" + this->_bckDir;

    if(!Om_isDir(bck_dir_path)) {
      this->_error = Om_errIsDir(L"Root folder", bck_dir_path);
      this->log(0, L"Package("+this->_ident+L") Restore", this->_error);
      return false;
    }

    is_zip = false; //< backup data is a sub-directory

    // all appear OK, we can proceed
    wstring bck_file, app_file;

    // first we restore genuine files from backup folder
    for(size_t i = 0; i < this->_bckItemLs.size(); ++i) {
      // we are interested only by backup file to copy
      if(this->_bckItemLs[i].dest == PKGITEM_DEST_CPY) {
        // path to file in the backup sub-directory
        Om_concatPaths(bck_file, bck_dir_path, this->_bckItemLs[i].path);
        // path to destination file to be overwritten
        Om_concatPaths(app_file, this->_location->_dstDir, this->_bckItemLs[i].path);
        // Move file from backup directory to destination, replacing destination
        result = Om_fileMove(bck_file, app_file);
        if(result != 0) {
          this->_error = Om_errMove(L"File", bck_file, result);
          this->log(0, L"Package("+this->_ident+L") Restore", this->_error);
        }
        // call progression callback
        if(progress_cb) {
          if(undo) {
            progress_cur--; //< for undo we step backward
          } else {
            progress_cur++;
          }
          progress_cb(user_ptr, progress_tot, progress_cur, 0);
        }
        #ifdef DEBUG
        Sleep(OMM_DEBUG_SLOW); //< for debug
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

  size_t n = this->_bckItemLs.size();
  while(n--) {
    // we are interested only by file to cleanup
    if(this->_bckItemLs[n].dest == PKGITEM_DEST_DEL) {
      // get destination file (to be deleted) path
      Om_concatPaths(del_file, this->_location->_dstDir, this->_bckItemLs[n].path);
      // in case we undo, the file may be not installed already, to prevent
      // useless warnings we test existing file/folder before trying to delete
      if(undo) {
        if(!Om_pathExists(del_file))
          continue;
      }
      // check whether this is a file or directory
      if(this->_bckItemLs[n].type == PKGITEM_TYPE_F) {
        result = Om_fileDelete(del_file);
        if(result != 0) {
          this->_error = Om_errDelete(L"File", del_file, result);
          this->log(1, L"Package("+this->_ident+L") Restore", this->_error);
        }
      } else {
        // we delete folder only if empty
        if(Om_isDirEmpty(del_file)) {
          result = Om_dirDelete(del_file);
          if(result != 0) {
            this->_error = Om_errDelete(L"Folder", del_file, result);
            this->log(1, L"Package("+this->_ident+L") Restore", this->_error);
          }
        }
      }
      // call progression callback
      if(progress_cb) {
        if(undo) {
          progress_cur--; //< for undo we step backward
        } else {
          progress_cur++;
        }
        progress_cb(user_ptr, progress_tot, progress_cur, 0);
      }
      #ifdef DEBUG
      Sleep(OMM_DEBUG_SLOW); //< for debug
      #endif
    }
  }

  // cleanup backup data either zip file or sub-directory...
  if(is_zip) {
    result = Om_fileDelete(this->_bck);
    if(result != 0) {
      this->_error = Om_errDelete(L"Backup archive file", this->_bck, result);
      this->log(0, L"Package("+this->_ident+L") Restore", this->_error);
    }
  } else {
    result = Om_dirDeleteRecursive(this->_bck);
    if(result != 0) {
      this->_error = Om_errDelete(L"Backup main folder", this->_bck, result);
      this->log(0, L"Package("+this->_ident+L") Restore", this->_error);
    }
  }

  // revoke the backup property of this package
  this->bckClear();

  // making report
  wchar_t log_buf[32];
  swprintf(log_buf, 32, L"Done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);

  log(2, L"Package("+this->_ident+L") Restore", log_buf);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmPackage::_discard()
{
  #ifdef DEBUG
  std::wcout << L"DEBUG => OmPackage::_discard\n";
  #endif

  // initialize local timer
  clock_t time = clock();

  bool is_zip = Om_isFileZip(this->_bck); //< backup data is zip file

  int result;

  // cleanup backup data either zip file or sub-directory...
  if(is_zip) {
    result = Om_moveToTrash(this->_bck);
    if(result != 0) {
      this->_error = Om_errDelete(L"Backup archive file", this->_bck, result);
      this->log(0, L"Package("+this->_ident+L") Unbackup", this->_error);
    }
  } else {
    result = Om_moveToTrash(this->_bck);
    if(result != 0) {
      this->_error = Om_errDelete(L"Backup main folder", this->_bck, result);
      this->log(0, L"Package("+this->_ident+L") Unbackup", this->_error);
    }
  }

  // revoke the backup property of this package
  this->bckClear();

  // making report
  wchar_t log_buf[32];
  swprintf(log_buf, 32, L"Done in %.2fs", (double)(clock()-time)/CLOCKS_PER_SEC);

  log(2, L"Package("+this->_ident+L") Unbackup", log_buf);

  return true;
}
