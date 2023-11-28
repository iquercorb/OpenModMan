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

#include "OmBaseWin.h"
#include <ShlObj.h>

#include "OmBaseUi.h"

#include "OmBaseApp.h"

#include "OmModMan.h"
#include "OmModHub.h"
#include "OmModPack.h"

#include "OmUiProgress.h"

#include "OmUtilFs.h"
#include "OmUtilDlg.h"
#include "OmUtilStr.h"
#include "OmUtilImg.h"
#include "OmUtilHsh.h"
#include "OmUtilB64.h"
#include "OmUtilZip.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiToolRep.h"


#define REPO_DEFAULT_DOWLOAD  L"files/"

/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the _addDir_fth
/// thread finished his job.
///
#define UWM_ADDENTRIES_DONE   (WM_APP+1)

/// \brief Save repository remote package snapshot
///
/// Function to create and store (as Data URI) repository remote package snapshot
/// from the given image object. The image is:
///  1 - loaded as raw RGB(A) data.
///  2 - converted to its thumbnail version (128 x 128 pixels).
///  3 - encoded to JPEG image data.
///  3 - the result is encoded to Base64 then assembled in Data URI scheme.
///
/// \param[in]  xml_pic : <picture> child node of a <package> XML node.
/// \param[in]  image   : Image object to convert to repository snapshot.
///
/// \return True if operation succeed, false otherwise
///
static inline bool __save_snapshot(OmXmlNode& xml_pic, const OmImage& image)
{
  // Load image data to raw RGB
  if(image.valid()) {

    // Get RGBA thumbnail
    uint8_t* rgb_data = Om_imgMakeThumb(128, OM_SIZE_FILL, image.data(), image.width(), image.height());

    // Encode RGBA to JPEG
    size_t jpg_size;
    uint8_t* jpg_data = Om_imgEncodeJpg(&jpg_size, rgb_data, 128, 128, 4, 7);

    // format jpeg to base64 encoded data URI
    OmWString data_uri;
    Om_encodeDataUri(data_uri, L"image/jpeg", L"", jpg_data, jpg_size);

    // set node content to data URI string
    xml_pic.setContent(data_uri);

    // free allocated data
    Om_free(rgb_data);
    Om_free(jpg_data);

    return true;

  }

  return false;
}

/// \brief Save repository remote package description
///
/// Function to create and store (as Data URI) repository remote package description
/// from the given image object. The text is:
///  1 - encoded to UTF-8 text.
///  2 - compressed using Deflate algorithm.
///  3 - the result is encoded to Base64 then assembled in Data URI scheme.
///
/// \param[in]  xml_des : <description> child node of a <package> XML node.
/// \param[in]  text    : Description string to convert to repository description.
///
/// \return True if operation succeed, false otherwise
///
static inline bool __save_description(OmXmlNode& xml_des, const OmWString& text)
{
  // convert to UTF-8
  OmCString utf8 = Om_toUTF8(text);
  size_t txt_size = utf8.size() + 1; // we include the nullchar

  if(txt_size > 1) {

    // set original data size in xml
    xml_des.setAttr(L"bytes", static_cast<int>(txt_size));

    // compress data
    size_t zip_size;
    uint8_t* zip = Om_zDeflate(&zip_size, reinterpret_cast<const uint8_t*>(utf8.c_str()), txt_size, 9);

    // Encode to data URI Base64
    OmWString data_uri;
    Om_encodeDataUri(data_uri, L"application/octet-stream", L"", zip, zip_size);
    Om_free(zip);

    // set node content to data URI string
    xml_des.setContent(data_uri);

    return true;

  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolRep::OmUiToolRep(HINSTANCE hins) : OmDialog(hins),
  _xmlconf(),
  _rmtCur(),
  _unsaved(false),
  _addDir_hth(nullptr),
  _addDir_path()
{
  this->addChild(new OmUiProgress(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolRep::~OmUiToolRep()
{
  HBITMAP hBm = this->setStImage(IDC_SB_SNAP, nullptr);
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);

  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiToolRep::id() const
{
  return IDD_TOOL_REP;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repInit()
{
  // Initialize new Repository definition XML scheme
  this->_xmlconf.init(OM_XMAGIC_REP);

  // Create repository base scheme
  this->_xmlconf.addChild(L"uuid").setContent(Om_genUUID());
  this->_xmlconf.addChild(L"title").setContent(L"New Repository");
  this->_xmlconf.addChild(L"downpath").setContent(REPO_DEFAULT_DOWLOAD);
  this->_xmlconf.addChild(L"remotes").setAttr(L"count", 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_repOpen(const OmWString& path)
{
  // Initialize new Repository definition XML scheme
  if(!this->_xmlconf.load(path, OM_XMAGIC_REP)) {
    Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_ERR,
                 L"Repository definition parse error", L"The specified file is "
                 "not valid Repository definition:", path);
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmUiToolRep::_rmtGet(const OmWString& ident)
{
  OmXmlNode result;

  // Get the package list XML node
  OmXmlNode xml_rmts = this->_xmlconf.child(L"remotes");

  // Get all <remote> children
  OmXmlNodeArray xml_rmt_ls;
  xml_rmts.children(xml_rmt_ls, L"remote");

  // search <remote> with specified identity
  for(size_t i = 0; i < xml_rmt_ls.size(); ++i) {
    if(ident == xml_rmt_ls[i].attrAsString(L"ident")) {
      result = xml_rmt_ls[i]; break;
    }
  }

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_rmtAdd(const OmWString& path)
{
  // parse the package
  OmXmlNode xml_node, xml_rmt, xml_rmts;

  // try to parse package
  OmModPack ModPack;
  if(!ModPack.parseSource(path))
    return false;

  // Search for already existing <remote>
  xml_rmt = this->_rmtGet(ModPack.iden());

  // if <remote> with same identity already exist, user have to choose
  if(!xml_rmt.empty()) {

    if(!Om_dlgBox_yn(this->_hwnd, L"Repository Editor", IDI_QRY,
                L"Empty Package source folder", L"Package with identity \""
                +ModPack.iden()+L"\" already exists in current Repository, "
                "do you want to replace the existing one ?"))
      return true; // cancel operation

    // check whether the current duplicate <remote> is the current selected one
    if(!this->_rmtCur.empty()) {
      // unselect package in list, to prevent inconsistent control contents
      if(this->_rmtCur.attrAsString(L"ident") == ModPack.iden()) {
        this->_rmtSel(L"");
      }
    }

    // reset the current Repository <remote>
    xml_rmt.remChild(L"dependencies");
    xml_rmt.remChild(L"picture");
    xml_rmt.remChild(L"description");
    xml_rmt.setAttr(L"file", Om_getFilePart(ModPack.sourcePath()));
    xml_rmt.setAttr(L"bytes", Om_itemSize(path));
    xml_rmt.setAttr(L"xxhsum", Om_getXXHsum(path)); //< use XXHash3 by default
    xml_rmt.setAttr(L"category", ModPack.category());

  } else {

    // Get the remote package list XML node
    xml_rmts = this->_xmlconf.child(L"remotes");

    // create new <remote> in repository
    xml_rmt = xml_rmts.addChild(L"remote");
    xml_rmt.setAttr(L"ident", ModPack.iden());
    xml_rmt.setAttr(L"file", Om_getFilePart(ModPack.sourcePath()));
    xml_rmt.setAttr(L"bytes", Om_itemSize(path));
    xml_rmt.setAttr(L"xxhsum", Om_getXXHsum(path)); //< use XXHash3 by default
    xml_rmt.setAttr(L"category", ModPack.category());

    // Add package to ListBox
    this->msgItem(IDC_LB_PKG, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ModPack.iden().c_str()));

    // increment <remote> count
    int n = xml_rmts.attrAsInt(L"count");
    xml_rmts.setAttr(L"count", n + 1);
  }

  if(ModPack.dependCount()) {
    xml_node = xml_rmt.addChild(L"dependencies");
    for(size_t i = 0; i < ModPack.dependCount(); ++i) {
      xml_node.addChild(L"ident").setContent(ModPack.getDependIden(i));
    }
  }

  if(ModPack.thumbnail().valid()) {
    xml_node = xml_rmt.addChild(L"picture");
    __save_snapshot(xml_node, ModPack.thumbnail());
  }

  if(!ModPack.description().empty()) {
    xml_node = xml_rmt.addChild(L"description");
    __save_description(xml_node, ModPack.description());
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_rmtRem(const OmWString& ident)
{
  // Get the package list XML node
  OmXmlNode xml_rmts = this->_xmlconf.child(L"remotes");

  // Get all <remote> children
  OmXmlNodeArray xml_rmt_ls;
  xml_rmts.children(xml_rmt_ls, L"remote");

  // search <remote> with specified identity
  for(size_t i = 0; i < xml_rmt_ls.size(); ++i) {
    if(ident == xml_rmt_ls[i].attrAsString(L"ident")) {
      xml_rmts.remChild(xml_rmt_ls[i]);

      // decrement package count
      int n = xml_rmts.attrAsInt(L"count");
      xml_rmts.setAttr(L"count", n - 1);

      return true;
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_rmtSel(const OmWString& ident)
{
  // clear current selected <remote>
  this->_rmtCur.clear();

  // check for empty selection
  if(ident.empty()) {

    // unselect all in ListBox
    this->msgItem(IDC_LB_PKG, LB_SETCURSEL, -1);

    // no package selected, disable and reset all related controls
    this->setItemText(IDC_EC_READ1, L"");
    this->setItemText(IDC_EC_READ2, L"");
    this->setItemText(IDC_EC_READ4, L"");
    this->setItemText(IDC_EC_INP03, L"");
    this->enableItem(IDC_EC_INP03, false);
    this->enableItem(IDC_BC_SAV01, false);
    this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 0);
    this->setItemText(IDC_EC_READ3, L"");
    this->enableItem(IDC_BC_QRY, false);
    this->enableItem(IDC_BC_BRW08, false);
    this->enableItem(IDC_BC_DEL, false);
    HBITMAP hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(this->_hins, IDB_BLANK));
    if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);
    this->enableItem(IDC_BC_BRW09, false);
    this->enableItem(IDC_BC_SAV02, false);
    this->setItemText(IDC_EC_DESC, L"");
    this->enableItem(IDC_EC_DESC, false);

    return true;
  }

  // Get the remote packages list XML node
  OmXmlNode xml_rmts = this->_xmlconf.child(L"remotes");

  // Get all <remote> children
  OmXmlNodeArray xml_rmt_ls;
  xml_rmts.children(xml_rmt_ls, L"remote");

  // search <remote> with specified identity
  for(size_t i = 0; i < xml_rmt_ls.size(); ++i) {
    if(ident == xml_rmt_ls[i].attrAsString(L"ident")) {
      this->_rmtCur = xml_rmt_ls[i]; break;
    }
  }

  // verify we found something
  if(this->_rmtCur.empty())
    return false;

  OmXmlNode xml_node;
  OmWString tmp_str1, tmp_str2;
  HBITMAP hBm_new, hBm_old;

  this->setItemText(IDC_EC_READ1, this->_rmtCur.attrAsString(L"file"));

  if(this->_rmtCur.hasAttr(L"checksum")) { //< legacy, deprecated

    this->setItemText(IDC_EC_READ2, this->_rmtCur.attrAsString(L"checksum"));
    this->setItemText(IDC_SC_LBL10, L"Checksum (xxh) :");

  } else if(this->_rmtCur.hasAttr(L"xxhsum")) {

    this->setItemText(IDC_EC_READ2, this->_rmtCur.attrAsString(L"xxhsum"));
    this->setItemText(IDC_SC_LBL10, L"Checksum (xxh) :");

  } else if(this->_rmtCur.hasAttr(L"md5sum")) {

    this->setItemText(IDC_EC_READ2, this->_rmtCur.attrAsString(L"md5sum"));
    this->setItemText(IDC_SC_LBL10, L"Checksum (md5) :");
  }

  this->setItemText(IDC_EC_READ4, this->_rmtCur.attrAsString(L"category"));

  // allow custom URL CheckBox
  this->enableItem(IDC_BC_CKBX1, true);

  // Check whether this <remote> have a Custom URL
  if(this->_rmtCur.hasChild(L"url")) {

    this->enableItem(IDC_EC_INP03, true);
    this->setItemText(IDC_EC_INP03, this->_rmtCur.child(L"url").content());
    this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 1);

  } else {

    this->setItemText(IDC_EC_INP03, L"");
    this->enableItem(IDC_EC_INP03, false);
    this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 0);
  }

  // custom URL save button disabled by default
  this->enableItem(IDC_BC_SAV01, false);

  // check for <remote> dependencies
  if(this->_rmtCur.hasChild(L"dependencies")) {

    xml_node = this->_rmtCur.child(L"dependencies");
    OmXmlNodeArray xml_ls;
    xml_node.children(xml_ls, L"ident");

    OmWString dpn_str;

    for(unsigned i = 0; i < xml_ls.size(); ++i) {
      dpn_str += xml_ls[i].content();
      if(i < (xml_ls.size() - 1)) {
        dpn_str += L"; ";
      }
    }

    this->enableItem(IDC_BC_QRY, true);
    this->enableItem(IDC_EC_READ3, true);
    this->setItemText(IDC_EC_READ3, dpn_str);

  } else {

    this->enableItem(IDC_BC_QRY, false);
    this->enableItem(IDC_EC_READ3, false);
    this->setItemText(IDC_EC_READ3, L"");
  }

  // enable Snapshot "Select..." Button
  this->enableItem(IDC_BC_BRW08, true);

  // for default controls state
  bool has_snap = false;

  // check for <remote> snapshot
  if(this->_rmtCur.hasChild(L"picture")) {
    xml_node = this->_rmtCur.child(L"picture");

    // decode the DataURI
    size_t jpg_size;
    uint8_t* jpg = Om_decodeDataUri(&jpg_size, tmp_str1, tmp_str2, xml_node.content());

    // load Jpeg image
    if(jpg) {

      unsigned w, h;
      uint8_t* rgba = Om_imgLoadData(&w, &h, jpg, jpg_size);
      hBm_new = Om_imgEncodeHbmp(rgba, w, h, 4);
      Om_free(rgba);

        // set image to dialog
      hBm_old = this->setStImage(IDC_SB_SNAP, hBm_new);
      if(hBm_old && hBm_old != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm_old);

      this->enableItem(IDC_BC_DEL, true);
      this->setItemText(IDC_BC_OPEN1, L"Change...");

      has_snap = true;
    }

    if(!has_snap) {
      // TODO: do something in case snapshot exist but is corrupted or invalid
    }

  }

  // no snapshot, reset controls to default
  if(!has_snap) {
    hBm_old = this->setStImage(IDC_SB_SNAP, Om_getResImage(this->_hins, IDB_BLANK));
    if(hBm_old && hBm_old != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm_old);
    this->enableItem(IDC_BC_DEL, false);
  }

  // allow user to edit a description
  this->enableItem(IDC_BC_BRW09, true);
  this->enableItem(IDC_BC_SAV02, false);
  this->enableItem(IDC_EC_DESC, true);

  // for default controls state
  bool has_desc = false;

  // check for <remote> description
  if(this->_rmtCur.hasChild(L"description")) {
    xml_node = this->_rmtCur.child(L"description");

    // decode the DataURI
    size_t zip_size;
    uint8_t* zip = Om_decodeDataUri(&zip_size, tmp_str1, tmp_str2, xml_node.content());

    if(zip) {
      // get string original size
      size_t txt_size = xml_node.attrAsInt(L"bytes");

      // decompress data
      uint8_t* txt = Om_zInflate(zip, zip_size, txt_size);
      Om_free(zip);

      if(txt) {
        // set text to item
        this->setItemText(IDC_EC_DESC, Om_toUTF16(reinterpret_cast<char*>(txt)));
        Om_free(txt);
        has_desc = true;
      }
    }

    if(!has_desc) {
      // TODO: do something in case description exist but is corrupted or invalid
    }
  }

  if(!has_desc) {
    this->setItemText(IDC_EC_DESC, L"");
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmUiToolRep::_rmtGetDeps(OmWStringArray& miss_list, const OmWString& ident)
{
  // our missing result
  int miss = 0;

  // get <remote> XML node by ident
  OmXmlNode xml_rmt = this->_rmtGet(ident);

  // get list of dependencies
  if(xml_rmt.hasChild(L"dependencies")) {

    OmXmlNodeArray xml_dep_ls;
    xml_rmt.child(L"dependencies").children(xml_dep_ls, L"ident");

    bool unique;
    OmWString dep_idt;
    OmXmlNode xml_dep;

    for(unsigned i = 0; i < xml_dep_ls.size(); ++i) {

      // get dependency identity
      dep_idt = xml_dep_ls[i].content();

      // try to get <remote> with this identity
      xml_dep = this->_rmtGet(dep_idt);

      // if <remote> not found, dependency is missing
      if(xml_dep.empty()) {

        // we add only if unique
        unique = true;
        for(size_t k = 0; k < miss_list.size(); ++k) {
          if(miss_list[i] == dep_idt) {
            unique = false; break;
          }
        }
        if(unique) {
          miss_list.push_back(dep_idt);
          miss++;
        }
        // next item
        continue;
      }
      // recurse process with dependency node
      miss += this->_rmtGetDeps(miss_list, dep_idt);
    }
  }
  return miss;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_addDir_init(const OmWString& path)
{
  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));

  pUiProgress->open(true);
  pUiProgress->setCaption(L"Parse packages");
  pUiProgress->setScHeadText(L"Computing packages checksum");

  // keep path to folder to scan
  this->_addDir_path = path;

  // start package building thread
  DWORD dWid;
  this->_addDir_hth = CreateThread(nullptr, 0, this->_addDir_fth, this, 0, &dWid);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_addDir_stop()
{
  DWORD exitCode;

  if(this->_addDir_hth) {
    WaitForSingleObject(this->_addDir_hth, INFINITE);
    GetExitCodeThread(this->_addDir_hth, &exitCode);
    CloseHandle(this->_addDir_hth);
    this->_addDir_hth = nullptr;
  }

  this->_addDir_path.clear();

  // quit the progress dialog
  static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS))->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiToolRep::_addDir_fth(void* arg)
{
  OmUiToolRep* self = static_cast<OmUiToolRep*>(arg);

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(self->childById(IDD_PROGRESS));

  HWND hPb = pUiProgress->hPb();
  const bool* abort = pUiProgress->abortPtr();

  // get all file from given path
  OmWStringArray ls;
  Om_lsFile(&ls, self->_addDir_path, true);

  // initialize the progress bar
  if(hPb) {
    SendMessageW(hPb, PBM_SETRANGE, 0, MAKELPARAM(0, ls.size()));
    SendMessageW(hPb, PBM_SETSTEP, 1, 0);
    SendMessageW(hPb, PBM_SETPOS, 0, 0);
  }

  // try to add each file, silently fail
  for(size_t i = 0; i < ls.size(); ++i) {

    // set dialog progress detail text
    pUiProgress->setScItemText(Om_getFilePart(ls[i]).c_str());

    // proceed this package
    self->_rmtAdd(ls[i]);

    // step progress bar
    if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);

    #ifdef DEBUG
    Sleep(50); //< for debug
    #endif

    if(*abort) break;
  }


  // sends message to window to inform process ended
  PostMessage(self->_hwnd, UWM_ADDENTRIES_DONE, 0, 0);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcNew()
{
  // Check for unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgResetUnsaved(this->_hwnd, L"Mod Repo Editor")) {
      return; //< don't change anything
    }
  }

  // Unselect <remote>
  this->_rmtSel(L"");

  // Initialize a new Repository XML def
  this->_repInit();

  // reset ListBox content
  this->msgItem(IDC_LB_PKG, LB_RESETCONTENT, 0, 0);

  // Set default title and download path to controls
  this->setItemText(IDC_EC_INP01, this->_xmlconf.child(L"title").content());
  this->setItemText(IDC_EC_INP02, this->_xmlconf.child(L"downpath").content());

  // reset unsaved changes
  this->_unsaved = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcOpen()
{
  // Check for unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgResetUnsaved(this->_hwnd, L"Mod Repo Editor")) {
      return; //< don't change anything
    }
  }

  OmModHub* pModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  OmModChan* pModChan = pModHub ? pModHub->activeChannel() : nullptr;

  OmWString start, result;

  // select the initial location for browsing start
  if(pModChan) start = pModChan->libraryPath();

  // new dialog to open file
  if(!Om_dlgOpenFile(result, this->_hwnd, L"Open Repository definition", OM_XML_FILES_FILTER, start))
    return;

  if(!Om_isFile(result))
    return;

  // unselect all
  this->_rmtSel(L"");

  // reset ListBox content
  this->msgItem(IDC_LB_PKG, LB_RESETCONTENT, 0, 0);

  if(!this->_repOpen(result))
    return;

  // Get Repository Title
  this->setItemText(IDC_EC_INP01, this->_xmlconf.child(L"title").content());
  // Get Download path
  this->setItemText(IDC_EC_INP02, this->_xmlconf.child(L"downpath").content());

  OmXmlNode xml_rmts = this->_xmlconf.child(L"remotes");

  // get all <remote> nodes within <remotes>
  OmXmlNodeArray xml_rmt_ls;
  xml_rmts.children(xml_rmt_ls, L"remote");

  // Add each <remote> to ListBox
  for(size_t i = 0; i < xml_rmt_ls.size(); ++i) {
    this->msgItem(IDC_LB_PKG, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(xml_rmt_ls[i].attrAsString(L"ident")));
  }

  // reset unsaved changes
  this->_unsaved = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_onBcBrwPkg()
{
  OmModHub* pModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  OmModChan* pModChan = pModHub ? pModHub->activeChannel() : nullptr;

  OmWString start, result;

  // select the initial location for browsing start
  if(pModChan) start = pModChan->libraryPath();

  // open file dialog
  if(!Om_dlgOpenFile(result, this->_hwnd, L"Open Package file", OM_PKG_FILES_FILTER, start))
    return false;

  if(!Om_isFile(result))
    return false;

  // add package to repository
  if(!this->_rmtAdd(result)) {
    Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_ERR,
                 L"Package source parse error", L"Unable to parse "
                 "the specified file or folder as Package:", result);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_onBcBrwDir()
{
  OmModHub* pModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  OmModChan* pModChan = pModHub ? pModHub->activeChannel() : nullptr;

  OmWString start, result;

  // select the initial location for browsing start
  if(pModChan) start = pModChan->libraryPath();

  // open dialog to select folder
  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select a folder where to find packages to parse and add", start))
    return false;

  if(!Om_isDir(result))
    return false;

  // each add need to compute the file checksum, this operation can take
  // long time specially with huge files, to prevent unpleasant freeze of
  // dialog, we proceed through progress dialog within new thread.
  this->_addDir_init(result);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcRemPkg()
{
  // get current selected item string in ListBox
  int lb_sel = this->msgItem(IDC_LB_PKG, LB_GETCURSEL);

  if(lb_sel < 0)
    return;

  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  // get identity from ListBox string
  wchar_t iden_buf[OM_MAX_ITEM];
  this->msgItem(IDC_LB_PKG, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(iden_buf));

  // remove <remote> from XML definition
  this->_rmtRem(iden_buf);

  // Unselect remote
  this->_rmtSel(L"");

  // Remove remote from ListBox
  this->msgItem(IDC_LB_PKG, LB_DELETESTRING, lb_sel, 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onLbPkglsSel()
{
  // get ListBox current selection
  int lb_sel = this->msgItem(IDC_LB_PKG, LB_GETCURSEL);

  // enable or disable the Package "Remove" Button
  this->enableItem(IDC_BC_REM, (lb_sel >= 0));

  // if any selection, change current remote
  if(lb_sel >= 0) {

    // Get identity to select
    wchar_t iden_buf[OM_MAX_ITEM];
    this->msgItem(IDC_LB_PKG, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(iden_buf));

    // Select remote by identity
    this->_rmtSel(iden_buf);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcSavUrl()
{
  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  // Get URL from EditText
  OmWString url_str;
  this->getItemText(IDC_EC_INP03, url_str);

  // Check for empty string
  if(!url_str.empty()) {

    if(!Om_isValidUrlPath(url_str) && !Om_isValidUrl(url_str)) {

      Om_dlgBox_okl(this->_hwnd, L" Repository Editor", IDI_ERR,
                   L"Invalid Custom download path", L"The specified Custom "
                   "download path contains illegal character.", url_str);

      // return now
      return;
    }

    // Create node if needed
    if(!this->_rmtCur.hasChild(L"url"))
      this->_rmtCur.addChild(L"url");

    // Defile new URL string
    this->_rmtCur.child(L"url").setContent(url_str);

    // echo changes in EditText
    this->setItemText(IDC_EC_INP03, url_str);

  } else {

    // Remove <url> node
    if(this->_rmtCur.hasChild(L"url")) {
      this->_rmtCur.remChild(L"url");
    }
  }

  // disable Custom Url "Save" Button
  this->enableItem(IDC_BC_SAV01, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcChkDeps()
{
  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  // get identity for this remote
  OmWString ident = this->_rmtCur.attrAsString(L"ident");

  // Dependencies missing list
  OmWStringArray miss_ls;

  // go for recursive dependencies search
  int miss_cnt = this->_rmtGetDeps(miss_ls, ident);

  if(miss_cnt > 0) {

    // Compost list for warning message
    OmWString msg_ls;

    for(unsigned i = 0; i < miss_ls.size(); ++i) {
      msg_ls += miss_ls[i];
      if(i < (miss_ls.size() - 1)) msg_ls += L"\r\n";
    }

    Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_WRN,
                 L"Missing Package dependencies", L"The Package declare "
                 "dependencies which are not in the current Network "
                 "Repository definition.", msg_ls);
  } else {

    //Peaceful message
    Om_dlgBox_ok(this->_hwnd, L"Repository Editor", IDI_NFO,
                 L"Satisfied Package dependencies ", L"All the declared "
                 "dependencies by Package are currently in Network "
                 "Repository definition.");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_onBcBrwSnap()
{
  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return false;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  OmWString start, result;

  // select the initial location for browsing start
  if(ModChan)
    start = ModChan->libraryPath();

  // open file dialog
  if(!Om_dlgOpenFile(result, this->_hwnd, L"Open image file", OM_IMG_FILES_FILTER, start))
    return false;

  OmImage thumb;

  // try to load image file

  if(thumb.loadThumbnail(result, OM_MODPACK_THUMB_SIZE, OM_SIZE_FILL)) {

    OmXmlNode xml_pic;

    // retrieve or create <picture> child in <package>
    if(this->_rmtCur.hasChild(L"picture")) {
      xml_pic = this->_rmtCur.child(L"picture");
    } else {
      xml_pic = this->_rmtCur.addChild(L"picture");
    }

    // save snapshot data to <picture>
    if(__save_snapshot(xml_pic, thumb)) {

      HBITMAP hBm = this->setStImage(IDC_SB_SNAP, thumb.hbmp());
      if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);

      // enable Snapshot "Delete" Button
      this->enableItem(IDC_BC_DEL, true);

      // return now
      return true;
    }
  }

  // this what happen if something went wrong
  this->_onBcDelSnap(); //< delete Snapshot and reset controls

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcDelSnap()
{
  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  // retrieve or create <picture> child in <remote>
  if(this->_rmtCur.hasChild(L"picture")) {
    this->_rmtCur.remChild(L"picture");
  }

  HBITMAP hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(this->_hins, IDB_BLANK));
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);

  // disable Snapshot "Delete" Button
  this->enableItem(IDC_BC_DEL, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcBrwDesc()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  OmWString start, result;

  // select the initial location for browsing start
  if(ModChan) start = ModChan->libraryPath();

  // open file dialog
  if(!Om_dlgOpenFile(result, this->_hwnd, L"Open text file", OM_TXT_FILES_FILTER, start))
    return;

  if(!Om_isFile(result))
    return;

  // set loaded text as description
  OmCString text_str = Om_loadPlainText(result);
  SetDlgItemTextA(this->_hwnd, IDC_EC_DESC, text_str.c_str());

  // enable Description "Save" Button
  this->enableItem(IDC_BC_SAV02, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcSavDesc()
{
  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  // get description string
  OmWString desc_str;
  this->getItemText(IDC_EC_DESC, desc_str);

  if(!desc_str.empty()) {

    OmXmlNode xml_des;

    // retrieve or create <description> child in <remote>
    if(this->_rmtCur.hasChild(L"description")) {
      xml_des = this->_rmtCur.child(L"description");
    } else {
      xml_des = this->_rmtCur.addChild(L"description");
    }

    // save description string to <description>
    __save_description(xml_des, desc_str);

  } else {

    // retrieve or create <description> child in <remote>
    if(this->_rmtCur.hasChild(L"description")) {
      this->_rmtCur.remChild(L"description");
    }
  }

  // disable Description "Save" button
  this->enableItem(IDC_BC_SAV02, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcSave()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  // Before saving, update the download path and title
  OmWString item_str;
  this->getItemText(IDC_EC_INP01, item_str);

  // set title
  this->_xmlconf.child(L"title").setContent(item_str);

  this->getItemText(IDC_EC_INP02, item_str);

  // add a lasting / if needed
  if(item_str.back() != L'/')
    item_str.push_back(L'/');

  // check whether user entered an URL
  if(Om_isValidUrl(item_str)) {
    Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_ERR,
                 L"Invalid Default download path", L"The Default download path "
                 "cannot be an URL, please specify a valid URL path or let "
                 "empty.", item_str);
    return;
  }

  // check whether path is valid
  if(!Om_isValidUrlPath(item_str)) {
    Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_ERR,
                 L"Invalid Default download path", L"The specified Default "
                 "download path contains illegal character.", item_str);
    return;
  }

  // set download path
  this->_xmlconf.child(L"downpath").setContent(item_str);
  // echo changes in EditText
  this->setItemText(IDC_EC_INP02, item_str);

  OmWString start, result;

  // select the initial location for browsing start
  if(ModChan) start = ModChan->libraryPath();

  // send save dialog to user
  if(!Om_dlgSaveFile(result, this->_hwnd, L"Save Repository definition...", OM_XML_FILES_FILTER, start))
    return;

  // check for ".xml" extension, add it if needed
  if(!Om_extensionMatches(result, L"xml")) {
    result += L".xml";
  }

  if(Om_dlgValidPath(this->_hwnd, L"file path", result)) {
    if(!Om_dlgOverwriteFile(this->_hwnd, result))
      return;
  } else {
    return;
  }

  if(!this->_xmlconf.save(result)) {
    Om_dlgSaveError(this->_hwnd, L"Mod Repo Editor", L"Save Mod repo definition",
                    L"Mod repo definition", this->_xmlconf.lastErrorStr());
    return;
  }

  // Disable Save as... button
  this->enableItem(IDC_BC_SAVE, false);

  // reset unsaved changes
  this->_unsaved = false;

  // a reassuring message
  Om_dlgSaveSucces(this->_hwnd, L"Mod Repo Editor", L"Save Mod repo definition",
                   L"Mod repo definition");
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcClose()
{
  // check whether there is unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgCloseUnsaved(this->_hwnd, L"Mod Repo Editor")) {
      return; //< do NOT close
    }
  }

  // quit this dialog
  this->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins,IDI_APP,2),Om_getResIcon(this->_hins,IDI_APP,1));

  // dialog is modeless so we set dialog title with app name
  this->setCaption(L"Repository editor ");

  // define controls tool-tips
  this->_createTooltip(IDC_BC_BRW01,  L"Browse to select a Repository XML file");
  this->_createTooltip(IDC_BC_NEW,    L"Erase current and initialize a new definition from scratch");

  this->_createTooltip(IDC_EC_INP01,  L"Repository title, to name it indicatively");
  this->_createTooltip(IDC_EC_INP02,  L"Default path appended to repository URL to download files");

  this->_createTooltip(IDC_LB_PKG,    L"Repository remote package list");

  this->_createTooltip(IDC_BC_BRW02,  L"Browse to select a package to parse and add to Repository");
  this->_createTooltip(IDC_BC_BRW03,  L"Browse to select a folder where to find packages to parse and add");
  this->_createTooltip(IDC_BC_REM,    L"Remove the selected package from Repository");

  this->_createTooltip(IDC_BC_QRY,    L"Check for package dependencies availability within the Repository");

  this->_createTooltip(IDC_BC_BRW08,  L"Browse to select an image to set as Package snapshot");
  this->_createTooltip(IDC_BC_DEL,    L"Remove the current snapshot image");

  this->_createTooltip(IDC_BC_BRW09,  L"Browse to open text file and use its content as description");
  this->_createTooltip(IDC_BC_SAV02,  L"Save description changes to Repository");
  this->_createTooltip(IDC_EC_DESC,    L"Package description text");

  this->_createTooltip(IDC_BC_CKBX1,  L"Define a custom download path or URL to download file");
  this->_createTooltip(IDC_EC_INP03,  L"Custom download path, base or full URL to download file");
  this->_createTooltip(IDC_BC_SAV01,  L"Save custom download path setting to Repository");

  // Set font for description
  HFONT hFt = Om_createFont(14, 400, L"Consolas");
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  // Set default package picture
  this->setStImage(IDC_SB_SNAP, Om_getResImage(this->_hins, IDB_BLANK));
  // Set buttons icons
  this->setBmIcon(IDC_BC_NEW, Om_getResIcon(this->_hins, IDI_BT_NEW));
  this->setBmIcon(IDC_BC_BRW01, Om_getResIcon(this->_hins, IDI_BT_OPN));
  this->setBmIcon(IDC_BC_BRW02, Om_getResIcon(this->_hins, IDI_BT_FAD));
  this->setBmIcon(IDC_BC_BRW03, Om_getResIcon(this->_hins, IDI_BT_DAD));
  this->setBmIcon(IDC_BC_REM, Om_getResIcon(this->_hins, IDI_BT_FRM));

  // Set snapshot format advice
  this->setItemText(IDC_SC_NOTES, L"Optimal format:\nSquare image of 128 x 128 pixels");

  // Initialize new Repository definition XML scheme
  this->_repInit();

  // Set default title and download path to controls
  this->setItemText(IDC_EC_INP01, this->_xmlconf.child(L"title").content());
  this->setItemText(IDC_EC_INP02, this->_xmlconf.child(L"downpath").content());

  // reset unsaved changes
  this->_unsaved = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onResize()
{
  unsigned half_w = static_cast<float>(this->cliUnitX()) * 0.5f;

  // -- Left Frame --

  // New and Open.. Buttons
  this->_setItemPos(IDC_BC_NEW, 5, 5, 50, 14);
  this->_setItemPos(IDC_BC_BRW01, 60, 5, 50, 14);

  // [ - - -       Packages Title GroupBox        - - -
  this->_setItemPos(IDC_GB_GRP01, 5, 20, half_w-10, 45);
  // Title Label & EditText
  this->_setItemPos(IDC_SC_LBL01, 10, 30, 120, 9);
  this->_setItemPos(IDC_EC_INP01, 10, 45, half_w-20, 12);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Download path GroupBox        - - -
  this->_setItemPos(IDC_GB_GRP02, 5, 65, half_w-10, 45);
  // Download path Label & EditText
  this->_setItemPos(IDC_SC_LBL03, 10, 75, 120, 9);
  this->_setItemPos(IDC_EC_INP02, 10, 90, half_w-20, 12);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -       Packages List GroupBox        - - -
  this->_setItemPos(IDC_GB_GRP03, 5, 110, half_w-10, this->cliUnitY()-160);
  // Packages list Label
  this->_setItemPos(IDC_SC_LBL02, 10, 122, 120, 9);
  // Add folder.. , Add... & Remove Buttons
  this->_setItemPos(IDC_BC_BRW02, half_w-125, 120, 40, 14);
  this->_setItemPos(IDC_BC_BRW03, half_w-80, 120, 40, 14);
  this->_setItemPos(IDC_BC_REM, half_w-26, 120, 16, 14);
  // Packages list ListBox
  this->_setItemPos(IDC_LB_PKG, 10, 138, half_w-20, this->cliUnitY()-195);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]


  // Save As.. Button
  this->_setItemPos(IDC_BC_SAVE, 5, this->cliUnitY()-45, 55, 14);

  // -- Right Frame --

  // [ - - -     Package Name & Url GroupBox      - - -
  this->_setItemPos(IDC_GB_GRP06, half_w+5, 0, half_w-10, 55);
  // Filename Label & EditText
  this->_setItemPos(IDC_SC_LBL06, half_w+10, 10, 54, 9);
  this->_setItemPos(IDC_EC_READ1, half_w+65, 10, half_w-75, 11);
  // Checksum Label & EditText
  this->_setItemPos(IDC_SC_LBL10, half_w+10, 25, 54, 9);
  this->_setItemPos(IDC_EC_READ2, half_w+65, 25, half_w-75, 11);
  // Category Label & EditText
  this->_setItemPos(IDC_SC_LBL11, half_w+10, 40, 54, 9);
  this->_setItemPos(IDC_EC_READ4, half_w+65, 40, half_w-75, 11);

  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Dependencies GroupBox         - - -
  this->_setItemPos(IDC_GB_GRP07, half_w+5, 55, half_w-10, 50);
  // Dependencies Label
  this->_setItemPos(IDC_SC_LBL07, half_w+10, 65, 54, 9);
  // Dependencies EditText & Check Button
  this->_setItemPos(IDC_EC_READ3, half_w+65, 66, half_w-120, 30);
  this->_setItemPos(IDC_BC_QRY, this->cliUnitX()-50, 65, 40, 13);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -          Snapshot GroupBox           - - -
  this->_setItemPos(IDC_GB_GRP08, half_w+5, 105, half_w-10, 95);
  // Snapshot Label
  this->_setItemPos(IDC_SC_LBL08, half_w+10, 115, 54, 9);
  // Snapshot Static Bitmap
  this->_setItemPos(IDC_SB_SNAP, half_w+65, 115, 86, 79);
  // Change.. & Delete Buttons
  this->_setItemPos(IDC_BC_BRW08, this->cliUnitX()-50, 115, 40, 13);
  this->_setItemPos(IDC_BC_DEL, this->cliUnitX()-50, 130, 40, 13);
  // Snapshot helper Static text
  this->_setItemPos(IDC_SC_NOTES, half_w+165, 155, 60, 30);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Description GroupBox          - - -
  this->_setItemPos(IDC_GB_GRP09, half_w+5, 200, half_w-10, this->cliUnitY()-270);
  // Description Label
  this->_setItemPos(IDC_SC_LBL09, half_w+10, 211, 54, 9);
  // Load.. & Save Buttons
  this->_setItemPos(IDC_BC_BRW09, this->cliUnitX()-95, 210, 40, 13);
  this->_setItemPos(IDC_BC_SAV02, this->cliUnitX()-50, 210, 40, 13);
  // Description EditText
  this->_setItemPos(IDC_EC_DESC, half_w+10, 225, half_w-20, this->cliUnitY()-300);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // force controls to repaint
  InvalidateRect(this->getItem(IDC_GB_GRP10), nullptr, true);
  InvalidateRect(this->getItem(IDC_BC_CKBX1), nullptr, true);
  InvalidateRect(this->getItem(IDC_EC_INP03), nullptr, true);
  InvalidateRect(this->getItem(IDC_BC_SAV01), nullptr, true);

  // [ - - -        Description GroupBox          - - -
  this->_setItemPos(IDC_GB_GRP10, half_w+5, this->cliUnitY()-70, half_w-10, 40);
  // Custom Url CheckBox & EditText
  this->_setItemPos(IDC_BC_CKBX1, half_w+10, this->cliUnitY()-60, 120, 9);
  this->_setItemPos(IDC_EC_INP03, half_w+10, this->cliUnitY()-48, half_w-62, 12);
  // Custom url Save Button
  this->_setItemPos(IDC_BC_SAV01, this->cliUnitX()-50, this->cliUnitY()-48, 40, 13);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->cliUnitY()-25, this->cliUnitX()-10, 1);
  // Close button
  this->_setItemPos(IDC_BC_CLOSE, this->cliUnitX()-54, this->cliUnitY()-19, 50, 14);

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiToolRep::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool has_changed = false;

  // UWM_ADDENTRIES_DONE is a custom message sent from add entries thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_ADDENTRIES_DONE) {
    // end the Add Entries process
    this->_addDir_stop();
    has_changed = true;
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_NEW:
      this->_onBcNew();
      break;

    case IDC_BC_BRW01:
      this->_onBcOpen();
      break;

    case IDC_EC_INP01: //< Repository title EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
      break;

    case IDC_EC_INP02: //< Download path EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
      break;

    case IDC_BC_BRW02: //< Add Package Button
      has_changed = this->_onBcBrwPkg();
      break;

    case IDC_BC_BRW03: //< Add Package folder Button
      has_changed = this->_onBcBrwDir();
      break;

    case IDC_BC_REM:
      this->_onBcRemPkg();
      has_changed = true;
      break;

    case IDC_LB_PKG: //< Packages list ListBox
      // check for selection change
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_onLbPkglsSel();
      break;

    case IDC_BC_CKBX1: // Custom Url CheckBox
      if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
        this->enableItem(IDC_EC_INP03, true);
      } else {
        this->setItemText(IDC_EC_INP03, L"");
        this->_onBcSavUrl();
        this->enableItem(IDC_EC_INP03, false);
        has_changed = true;
      }
      break;

    case IDC_EC_INP03: //< Custon URL EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        this->enableItem(IDC_BC_SAV01, true);
      break;

    case IDC_BC_SAV01: //< Custom Url "Save" Button
      this->_onBcSavUrl();
      has_changed = true;
      break;

    case IDC_BC_QRY: //< Dependencies "Check" Button
      this->_onBcChkDeps();
      break;

    case IDC_BC_BRW08: //< Snapshot "Select..." Button
      has_changed = this->_onBcBrwSnap();
      break;

    case IDC_BC_DEL: //< Snapshot "Delete" Button
      this->_onBcDelSnap();
      has_changed = true;
      break;

    case IDC_BC_BRW09: //< Description "Load.." Button
      this->_onBcBrwDesc();
      break;

    case IDC_EC_DESC: //< Description EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        this->enableItem(IDC_BC_SAV02, true); //< enable Description "Save" Button
      break;

    case IDC_BC_SAV02: //< Description "Save" Button
      this->_onBcSavDesc();
      has_changed = true;
      break;

    case IDC_BC_SAVE: //< General "Save as.." Button
      this->_onBcSave();
      break;

    case IDC_BC_CLOSE: //< General "Close" Button
      this->_onBcClose();
      break;
    }
  }

  if(has_changed) {

    // we have unsaved changes
    this->_unsaved = true;

    // check whether ListBox have remote to be saved
    int lb_cnt = this->msgItem(IDC_LB_PKG, LB_GETCOUNT, 0, 0);
    this->enableItem(IDC_BC_SAVE, (lb_cnt > 0));
  }

  return false;
}

