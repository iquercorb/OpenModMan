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
#include <algorithm>            //< std::replace

#include "OmBaseWin.h"
#include <ShlObj.h>

#include "OmBaseUi.h"

#include "OmBaseApp.h"

#include "OmArchive.h"

#include "OmModMan.h"
#include "OmModChan.h"
#include "OmImage.h"

#include "OmUtilFs.h"
#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiToolPkg.h"


/// \brief Custom "Package Save Done" Message
///
/// Custom "Package Save Done" window message to notify the dialog that the
/// running thread finished his job.
///
#define UWM_PKGSAVE_DONE    (WM_APP+1)

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolPkg::OmUiToolPkg(HINSTANCE hins) : OmDialog(hins),
  _unsaved(false),
  _save_hth(nullptr),
  _save_abort(false)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolPkg::~OmUiToolPkg()
{
  HBITMAP hBm = this->setStImage(IDC_SB_SNAP, nullptr);
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);

  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiToolPkg::id() const
{
  return IDD_TOOL_PKG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::selectSource(const OmWString& path)
{
  bool is_dir = Om_isDir(path);

  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, is_dir);
  this->msgItem(IDC_BC_RAD02, BM_SETCHECK, !is_dir);

  if(is_dir) {
    this->_onBcBrwDir(path.c_str());
  } else {
    this->_onBcBrwPkg(path.c_str());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_freeze(bool freeze)
{
  bool enable = !freeze;

  // disable all controls during creation
  this->enableItem(IDC_BC_RAD01, enable);
  this->enableItem(IDC_BC_RAD02, enable);
  this->enableItem(IDC_EC_INP03, enable);
  this->enableItem(IDC_EC_INP04, enable);
  this->enableItem(IDC_CB_EXT, enable);
  this->enableItem(IDC_CB_ZMD, enable);
  this->enableItem(IDC_CB_ZLV, enable);
  this->enableItem(IDC_EC_INP05, enable);
  this->enableItem(IDC_BC_BRW03, enable);
  this->enableItem(IDC_BC_CKBX1, enable);
  this->enableItem(IDC_BC_CKBX2, enable);
  this->enableItem(IDC_BC_CKBX3, enable);
  // disable according radios and check-boxes status
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->enableItem(IDC_EC_INP01, enable);
    this->enableItem(IDC_BC_BRW01, enable);
  } else {
    this->enableItem(IDC_EC_INP02, enable);
    this->enableItem(IDC_BC_BRW02, enable);
  }
  if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
    this->enableItem(IDC_EC_INP07, enable);
    if(enable) {
      OmWString ident;
      this->getItemText(IDC_EC_INP07, ident);
      this->enableItem(IDC_BC_ADD, !ident.empty());
    } else {
      this->enableItem(IDC_BC_ADD, false);
    }
    this->enableItem(IDC_BC_DEL, enable);
    this->enableItem(IDC_LB_DPN, enable);
  }
  if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
    this->enableItem(IDC_BC_BRW04, enable);
  }
  if(this->msgItem(IDC_BC_CKBX3, BM_GETCHECK)) {
    this->enableItem(IDC_BC_BRW05, enable);
    this->enableItem(IDC_EC_DESC, enable);
  }
  this->enableItem(IDC_BC_SAVE, enable);
  this->enableItem(IDC_BC_CLOSE, enable);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_parseSrc(const OmWString& path)
{
  HBITMAP hBm;

  // reset current package
  this->_ModPack.clearAll();

  // name and extension controls initial state
  this->setItemText(IDC_EC_INP03, L"");
  this->enableItem(IDC_EC_INP03, false);
  this->setItemText(IDC_EC_INP04, L"");
  this->enableItem(IDC_EC_INP04, false);
  this->enableItem(IDC_CB_EXT, false);
  this->enableItem(IDC_CB_ZMD, false);
  this->enableItem(IDC_CB_ZLV, false);
  this->setItemText(IDC_EC_READ1, L"");

  // destination folder disabled
  this->enableItem(IDC_EC_INP06, false);
  this->enableItem(IDC_BC_BRW03, false);

  // disable the save button
  this->enableItem(IDC_BC_SAVE, false);

  // category initial state
  this->msgItem(IDC_CB_CAT, CB_SETCURSEL, 0);
  this->enableItem(IDC_CB_CAT, false);
  this->setItemText(IDC_EC_INP09, L"");
  this->enableItem(IDC_EC_INP09, false);

  // Dependencies initial state
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CKBX1, false);
  this->enableItem(IDC_SC_LBL06, false); //< "Ident" label
  this->msgItem(IDC_LB_DPN, LB_RESETCONTENT);
  this->enableItem(IDC_LB_DPN, false);
  this->setItemText(IDC_EC_INP07, L"");
  this->enableItem(IDC_EC_INP07, false);
  this->enableItem(IDC_BC_ADD, false);
  this->enableItem(IDC_BC_DEL, false);

  // Snapshot initial states
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CKBX2, false);
  this->enableItem(IDC_BC_BRW04, false);
  hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(this->_hins, IDB_BLANK));
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);
  this->setItemText(IDC_EC_INP08, L"");

  // Description initial states
  this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CKBX3, false);
  this->enableItem(IDC_BC_BRW05, false);
  this->enableItem(IDC_EC_DESC, false);
  this->setItemText(IDC_EC_DESC, L"");

  // check whether source path is empty to
  // reset to initial state
  if(path.empty()) {
    this->_unsaved = false; //< reset unsaved changes
    return true;
  }

  // Try to parse the package
  if(!this->_ModPack.parseSource(path)) {
    return false;
  }

  // enable controls for package edition
  this->enableItem(IDC_EC_INP03, true);
  this->enableItem(IDC_EC_INP04, true);
  this->enableItem(IDC_CB_EXT, true);
  this->enableItem(IDC_CB_ZMD, true);
  this->enableItem(IDC_CB_ZLV, true);

  this->enableItem(IDC_EC_INP06, true);
  this->enableItem(IDC_BC_BRW03, true);

  this->enableItem(IDC_BC_CKBX1, true);
  this->enableItem(IDC_BC_CKBX2, true);
  this->enableItem(IDC_BC_CKBX3, true);

  this->enableItem(IDC_CB_CAT, true);

  // set source path to input EditText depending selected Radio
  this->enableItem(IDC_EC_INP01, true);
  this->enableItem(IDC_EC_INP02, true);
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->setItemText(IDC_EC_INP01, path);
    this->setItemText(IDC_EC_INP02, L"");
    this->enableItem(IDC_EC_INP02, false);
  } else {
    this->setItemText(IDC_EC_INP02, path);
    this->setItemText(IDC_EC_INP01, L"");
    this->enableItem(IDC_EC_INP01, false);
  }

  // check for package category
  if(!this->_ModPack.category().empty()) {
    int cb_idx = this->msgItem(IDC_CB_CAT, CB_FINDSTRING, -1, reinterpret_cast<LPARAM>(this->_ModPack.category().c_str()));
    if(cb_idx >= 0) {
      this->msgItem(IDC_CB_CAT, CB_SETCURSEL, cb_idx);
      this->setItemText(IDC_EC_INP09, L"");
      this->enableItem(IDC_EC_INP09, false);
    } else {
      this->msgItem(IDC_CB_CAT, CB_SETCURSEL, OmModCategoryCount);
      this->enableItem(IDC_EC_INP09, true);
      this->setItemText(IDC_EC_INP09, this->_ModPack.category());
    }
  } else {
    this->msgItem(IDC_CB_CAT, CB_SETCURSEL, 0);
    this->setItemText(IDC_EC_INP09, L"");
    this->enableItem(IDC_EC_INP09, false);
  }

  // check for package dependencies
  if(this->_ModPack.dependCount()) {
    this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 1);
    this->enableItem(IDC_SC_LBL06, true); //< "Ident" label
    this->enableItem(IDC_LB_DPN, true);
    this->enableItem(IDC_EC_INP07, true);
    for(unsigned i = 0; i < this->_ModPack.dependCount(); ++i) {
      this->msgItem(IDC_LB_DPN, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(this->_ModPack.getDependIden(i).c_str()));
    }
  }

  // check for package snapshot
  if(this->_ModPack.thumbnail().valid()) {
    this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 1);
    this->enableItem(IDC_BC_BRW04, true);
    hBm = this->setStImage(IDC_SB_SNAP, this->_ModPack.thumbnail().hbmp());
    if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);
  }

  // check for package description
  if(this->_ModPack.description().size()) {
    this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, 1);
    this->enableItem(IDC_BC_BRW05, true);
    this->enableItem(IDC_EC_DESC, true);
    this->setItemText(IDC_EC_DESC, this->_ModPack.description());
  }

  OmWString item_str, dst_path;

  // Add package content to output EditText
  unsigned n = this->_ModPack.sourceEntryCount();
  for(unsigned i = 0; i < this->_ModPack.sourceEntryCount(); ++i) {
    item_str.append(this->_ModPack.getSourceEntry(i).path);
    if(i < n - 1) item_str.append(L"\r\n");
  }
  this->setItemText(IDC_EC_READ1, item_str);

  // update name and version
  this->setItemText(IDC_EC_INP03, this->_ModPack.name());

  if(!this->_ModPack.version().isNull())
    this->setItemText(IDC_EC_INP04, this->_ModPack.version().asString());

  // get current source path
  item_str.clear();
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP01, item_str);
  } else {
    this->getItemText(IDC_EC_INP02, item_str);
  }

  if(!item_str.empty()) {

    // preselect file extention
    if(Om_extensionMatches(item_str, OM_PKG_FILE_EXT)) {
      this->msgItem(IDC_CB_EXT, CB_SETCURSEL, 1);
    } else {
      this->msgItem(IDC_CB_EXT, CB_SETCURSEL, 0);
    }

    // refresh final filename
    this->_onNameChange();

    // prefill destination path if empty
    if(this->getItemText(IDC_EC_INP06, dst_path) == 0) {

      dst_path = Om_getDirPart(item_str);

      if(Om_isDir(dst_path))
        this->setItemText(IDC_EC_INP06, dst_path);
    }
  }

  this->_unsaved = false; //< reset unsaved changes

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_save_init()
{
  // disable all dialog's controls
  this->_freeze(true);

  // enable progress bar and abort button
  this->enableItem(IDC_PB_PKG, true);
  this->enableItem(IDC_BC_ABORT, true);

  // start package building thread
  DWORD dWid;
  this->_save_hth = CreateThread(nullptr, 0, this->_save_fth, this, 0, &dWid);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_save_stop()
{
  DWORD exitCode;

  if(this->_save_hth) {
    WaitForSingleObject(this->_save_hth, INFINITE);
    GetExitCodeThread(this->_save_hth, &exitCode);
    CloseHandle(this->_save_hth);
    this->_save_hth = nullptr;
  }

  // enable all dialog's controls
  this->_freeze(false);

  // reset & disable progress bar & abort button
  this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0, 0);
  this->enableItem(IDC_PB_PKG, false);
  this->enableItem(IDC_BC_ABORT, false);

  // show a reassuring dialog message
  if(exitCode == 0) {

    // get destination filename
    OmWString item_str;
    this->getItemText(IDC_EC_RESUL, item_str);

    // a reassuring message
    Om_dlgSaveSucces(this->_hwnd, L"Mod Pack Editor", L"Save Mod pack", L"Mod pack");
  }

  // refresh the main window dialog, this will also refresh this one
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_save_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t data)
{
  OmUiToolPkg* self = reinterpret_cast<OmUiToolPkg*>(ptr);

  self->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, tot));
  self->msgItem(IDC_PB_PKG, PBM_SETPOS, cur);

  return !self->_save_abort;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiToolPkg::_save_fth(void* arg)
{
  OmUiToolPkg* self = static_cast<OmUiToolPkg*>(arg);

  OmWString item_str;

  // get package category
  int32_t cb_sel = self->msgItem(IDC_CB_CAT, CB_GETCURSEL);
  if(cb_sel == self->msgItem(IDC_CB_CAT, CB_GETCOUNT)-1) {
    // get category from text field
    self->getItemText(IDC_EC_INP09, item_str);
    Om_strToUpper(&item_str); //< convert to upper case
  } else {
    // Get from ComboBox
    wchar_t cate[OM_MAX_ITEM];
    self->msgItem(IDC_CB_CAT, CB_GETLBTEXT, cb_sel, reinterpret_cast<LPARAM>(cate));
    item_str = cate;
  }
  self->_ModPack.setCategory(item_str);


  // get/update package dependencies list
  self->_ModPack.clearDepend();
  if(self->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
    int32_t lb_cnt = self->msgItem(IDC_LB_DPN, LB_GETCOUNT);
    if(lb_cnt) {
      wchar_t iden[OM_MAX_ITEM];
      for(int32_t i = 0; i < lb_cnt; ++i) {
        self->msgItem(IDC_LB_DPN, LB_GETTEXT, i, reinterpret_cast<LPARAM>(iden));
        self->_ModPack.addDependIden(iden);
      }
    }
  }

  // get package image data from specified file if any
  if(self->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
    self->getItemText(IDC_EC_INP08, item_str);
    if(!item_str.empty()) {
      self->_ModPack.setThumbnail(item_str);
    }
  } else {
    self->_ModPack.clearThumbnail();
  }

  // get package description text
  if(self->msgItem(IDC_BC_CKBX3, BM_GETCHECK)) {
    self->getItemText(IDC_EC_DESC, item_str);
    self->_ModPack.setDescription(item_str);
  } else {
    self->_ModPack.setDescription(L"");
  }

  // get package compression level
  int32_t comp_md, comp_lv;

  switch(self->msgItem(IDC_CB_ZMD, CB_GETCURSEL)) {
  case 1:   comp_md = OM_METHOD_DEFLATE; break; //< MZ_COMPRESS_METHOD_DEFLATE
  case 2:   comp_md = OM_METHOD_LZMA; break;    //< MZ_COMPRESS_METHOD_LZMA
  case 3:   comp_md = OM_METHOD_LZMA2; break;   //< MZ_COMPRESS_METHOD_XZ
  case 4:   comp_md = OM_METHOD_ZSTD; break;    //< MZ_COMPRESS_METHOD_ZSTD
  default:  comp_md = OM_METHOD_STORE; break;   //< MZ_COMPRESS_METHOD_STORE
  }

  switch(self->msgItem(IDC_CB_ZLV, CB_GETCURSEL)) {
  case 1:   comp_lv = OM_LEVEL_FAST; break; //< MZ_COMPRESS_LEVEL_FAST
  case 2:   comp_lv = OM_LEVEL_SLOW; break; //< MZ_COMPRESS_LEVEL_NORMAL
  case 3:   comp_lv = OM_LEVEL_BEST; break; //< MZ_COMPRESS_LEVEL_BEST
  default:  comp_lv = OM_LEVEL_NONE; break;
  }

  // get destination path & filename
  OmWString out_path, out_dir, out_name;
  self->getItemText(IDC_EC_INP06, out_dir);
  self->getItemText(IDC_EC_RESUL, out_name);
  Om_concatPaths(out_path, out_dir, out_name);

  self->_save_abort = false;
  self->enableItem(IDC_BC_ABORT, true);

  DWORD exitCode = 0;

  if(!self->_ModPack.saveAs(out_path, comp_md, comp_lv, &self->_save_progress_cb, self)) {

    // show error dialog box
    Om_dlgSaveError(self->_hwnd, L"Mod Pack Editor", L"Save Mod pack",
                    L"Mod pack", self->_ModPack.lastError());

    exitCode = 1;
  }

  // if user aborted, the was package not successfully created
  if(self->_save_abort) {
    self->_unsaved = true; //< still have unsaved changes
    exitCode = 1;
  }

  // send message to stop thread
  self->postMessage(UWM_PKGSAVE_DONE);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcRadSrc()
{
  // get checked Radio
  bool bm_chk = this->msgItem(IDC_BC_RAD01, BM_GETCHECK);

  // Check for unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgResetUnsaved(this->_hwnd, L"Mod Pack Editor")) {
      // undo the radio button changes by user
      this->msgItem(IDC_BC_RAD01, BM_SETCHECK, !bm_chk);
      this->msgItem(IDC_BC_RAD02, BM_SETCHECK, bm_chk);
      return; //< return now, don't change anything
    }
  }

  // reset source input EditText
  this->enableItem(IDC_EC_INP01, true);
  this->setItemText(IDC_EC_INP01, L"");
  this->enableItem(IDC_EC_INP02, true);
  this->setItemText(IDC_EC_INP02, L"");

  // enable or disable EditText and Buttons according selection
  this->enableItem(IDC_EC_INP01, bm_chk);
  this->enableItem(IDC_BC_BRW01, bm_chk);
  this->enableItem(IDC_EC_INP02, !bm_chk);
  this->enableItem(IDC_BC_BRW02, !bm_chk);

  // reset package source
  this->_parseSrc(L"");

  // reset unsaved changes
  this->_unsaved = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_onBcBrwDir(const wchar_t* path)
{
  // Check for unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgResetUnsaved(this->_hwnd, L"Mod Pack Editor"))
      return false; //< return now, don't change anything
  }

  OmWString result, start;

  if(path != nullptr) {

    result = path;

  } else {

    // Select start directory, either previous one or current location library
    this->getItemText(IDC_EC_INP01, start);

    if(start.empty()) {

      // select current Mod Channel library as default
      OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
      if(ModHub) {
        OmModChan* ModChan = ModHub->activeChannel();
        if(ModChan) {
          start = ModChan->libraryPath();
        }
      }

    } else {
      start = Om_getDirPart(start);
    }

    if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Mod source directory", start)) {
      this->setItemText(IDC_EC_INP01, L"");
      this->_parseSrc(L"");
      return false;
    }

  }

  if(!Om_isDir(result)) {
    this->setItemText(IDC_EC_INP01, L"");
    this->_parseSrc(L"");
    return false;
  }

  // check whether selected folder is empty
  if(Om_isDirEmpty(result)) {

    // show warning dialog box
    if(!Om_dlgBox_yn(this->_hwnd, L"Mod Pack Editor", IDI_QRY,
                L"Empty source directory", L"The selected source directory is "
                "empty, the resulting package will have no content to install."
                "Do you want to continue anyway ?"))
    {
      this->setItemText(IDC_EC_INP01, L"");
      this->_parseSrc(L"");
      return false;
    }

  }



  // Try to parse the folder (as package source)
  if(!this->_parseSrc(result)) {
    // this cannot happen at this stage... but...
    Om_dlgBox_okl(this->_hwnd, L"Mod Pack Editor", IDI_ERR,
                 L"Mod source parse error", L"Unable to parse "
                 "the specified directory Mod source:", result);

    this->setItemText(IDC_EC_INP01, L"");
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_onBcBrwPkg(const wchar_t* path)
{
  // Check for unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgResetUnsaved(this->_hwnd, L"Mod Pack Editor"))
      return false; //< return now, don't change anything
  }

  // reset unsaved changes
  this->_unsaved = false;

  OmWString result, start;

  if(path != nullptr) {

    result = path;

  } else {

    // Select start directory, either previous one or current location library
    this->getItemText(IDC_EC_INP02, start);

    if(start.empty()) {

      // select current Mod Channel library as default
      OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
      if(ModHub) {
        OmModChan* ModChan = ModHub->activeChannel();
        if(ModChan) {
          start = ModChan->libraryPath();
        }
      }

    } else {
      start = Om_getDirPart(start);
    }

    // open select file dialog
    if(!Om_dlgOpenFile(result, this->_hwnd, L"Select Mod Pack file", OM_PKG_FILES_FILTER, start)) {
      this->setItemText(IDC_EC_INP02, L"");
      this->_parseSrc(L"");
      return false;
    }

  }

  if(!Om_isFile(result)) {
    this->setItemText(IDC_EC_INP02, L"");
    this->_parseSrc(L"");
    return false;
  }

  // parse this package source
  if(!this->_parseSrc(result)) {
    Om_dlgBox_okl(this->_hwnd, L"Mod Pack Editor", IDI_ERR,
                 L"Mod source parse error", L"Unable to parse "
                 "the specified file as Mod source:", result);

    this->setItemText(IDC_EC_INP02, L"");
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onNameChange()
{
  // get base name
  OmWString name_str;
  this->getItemText(IDC_EC_INP03, name_str);

  if(!name_str.empty()) {

    // Replace all spaces by underscores
    replace(name_str.begin(), name_str.end(), L' ', L'_');

    // get version
    OmWString vers_str;
    this->getItemText(IDC_EC_INP04, vers_str);

    // check if version string is valid
    OmVersion version(vers_str);
    if(!version.isNull()) {
      name_str += L"_v" + version.asString();
    }

    // get chosen file extension
    int cb_sel = this->msgItem(IDC_CB_EXT, CB_GETCURSEL, 0);
    wchar_t ext_str[6];
    this->msgItem(IDC_CB_EXT, CB_GETLBTEXT, cb_sel, reinterpret_cast<LPARAM>(ext_str));
    name_str += ext_str;

    // set final filename
    this->setItemText(IDC_EC_RESUL, name_str);

  } else {

    // reset output
    this->setItemText(IDC_EC_RESUL, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcBrwDest()
{
  OmWString result, start;

  // select start directory
  this->getItemText(IDC_EC_INP06, start);

  if(start.empty()) {
    if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
      this->getItemText(IDC_EC_INP01, start);
    } else {
      this->getItemText(IDC_EC_INP02, start);
    }
  }

  // open select folder dialog
  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select destination location", Om_getDirPart(start)))
    return;

  if(!Om_isDir(result))
    return;

  // Set path to input EditText
  this->setItemText(IDC_EC_INP06, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onLbDpnlsSel()
{
  // enable or disable Trash Button according selection
  int lb_sel = this->msgItem(IDC_LB_DPN, LB_GETCURSEL);

  this->enableItem(IDC_BC_DEL, (lb_sel >= 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onCbCatSel()
{
  int cb_sel = this->msgItem(IDC_CB_CAT, CB_GETCURSEL);

  // check whether user selected the last item (GENERIC)
  if(cb_sel == this->msgItem(IDC_CB_CAT, CB_GETCOUNT)-1) {
    // enable the text field
    this->enableItem(IDC_EC_INP09, true);
  } else {
    this->setItemText(IDC_EC_INP09, L"");
    this->enableItem(IDC_EC_INP09, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onCkBoxDep()
{
  bool bm_chk = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_SC_LBL06, bm_chk); //< "Ident" label
  this->enableItem(IDC_EC_INP07, bm_chk);
  this->enableItem(IDC_LB_DPN, bm_chk);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcAddDep()
{
  // Get identity string from EditText
  OmWString ident;
  this->getItemText(IDC_EC_INP07, ident);

  if(ident.empty())
    return;

  // Add string to dependencies
  this->msgItem(IDC_LB_DPN, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ident.c_str()));
  // Empty EditText
  this->setItemText(IDC_EC_INP07, L"");

  // disable Dependencies "+" Button
  this->enableItem(IDC_BC_ADD, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcDelDep()
{
  // get selected item index
  int lb_sel = this->msgItem(IDC_LB_DPN, LB_GETCURSEL);

  // remove from list
  if(lb_sel >= 0) {
    this->msgItem(IDC_LB_DPN, LB_DELETESTRING, lb_sel);
  }

  // disable Dependencies Trash Button
  this->enableItem(IDC_BC_DEL, false);
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onCkBoxSnap()
{
  if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {

    this->enableItem(IDC_BC_BRW04, true);

  } else {

    this->enableItem(IDC_BC_BRW04, false);

    HBITMAP hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(this->_hins, IDB_BLANK));
    if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);

    this->setItemText(IDC_EC_INP08, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_onBcBrwSnap()
{
  OmWString result, start;

  // select the start directory from package source path
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP01, start);
  } else {
    this->getItemText(IDC_EC_INP02, start);
  }

  // open file dialog
  if(!Om_dlgOpenFile(result, this->_hwnd, L"Open image file", OM_IMG_FILES_FILTER, Om_getDirPart(start)))
    return false;

  OmImage thumb;
  HBITMAP hBm;

  // Try to open image
  if(thumb.loadThumbnail(result, OM_MODPACK_THUMB_SIZE, OM_SIZE_FILL)) {

    // set thumbnail
    hBm = this->setStImage(IDC_SB_SNAP, thumb.hbmp());
    if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);

    // set EditText content to image path
    this->setItemText(IDC_EC_INP08, result);

  } else {

    // remove any thumbnail
    hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(this->_hins, IDB_BLANK));
    if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);

    // reset hidden EditText content
    this->setItemText(IDC_EC_INP08, L"");
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onCkBoxDesc()
{
  bool bm_chk = this->msgItem(IDC_BC_CKBX3, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW05, bm_chk);
  this->enableItem(IDC_EC_DESC, bm_chk);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_onBcBrwDesc()
{
  OmWString result, start;

  // select the start directory from package source path
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP01, start);
  } else {
    this->getItemText(IDC_EC_INP02, start);
  }

  // open file dialog
  if(!Om_dlgOpenFile(result, this->_hwnd, L"Open text file", OM_TXT_FILES_FILTER, Om_getDirPart(start)))
    return false;

  if(!Om_isFile(result))
    return false;

  OmWString text_wcs;
  Om_loadToUTF16(&text_wcs, result);

  SetDlgItemTextW(this->_hwnd, IDC_EC_DESC, text_wcs.c_str());

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcSave()
{
  OmWString item_str, msg;

  // verify package has parsed source
  if(!this->_ModPack.hasSource()) {

    // show error dialog box
    Om_dlgBox_ok(this->_hwnd, L"Mod Pack Editor", IDI_ERR,
                 L"Invalid Mod source", L"Invalid or empty Mod source. "
                 "Please select a file or directory as Mod source.");
    return;
  }

  // verify the destination path and name
  OmWString out_name, out_dir;

  this->getItemText(IDC_EC_INP06, out_dir);
  this->getItemText(IDC_EC_RESUL, out_name);

  if(!Om_dlgValidName(this->_hwnd, L"Mod Pack filename", out_name))
    return;

  if(Om_dlgValidPath(this->_hwnd, L"Save Destination", out_dir)) {
    if(!Om_dlgCreateFolder(this->_hwnd, L"Save Destination", out_dir))
      return;
  } else {
    return;
  }

  if(!Om_dlgOverwriteFile(this->_hwnd, out_dir + L"\\" + out_name))
    return;

  // changes has been saved
  this->_unsaved = false;

  // disable the Save button
  this->enableItem(IDC_BC_SAVE, false);

  // here we go
  this->_save_init();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins,IDI_APP,2),Om_getResIcon(this->_hins,IDI_APP,1));

  // dialog is modeless so we set dialog title with app name
  this->setCaption(L"Mod Pack editor ");

  // define controls tool-tips
  this->_createTooltip(IDC_BC_RAD01,  L"Use a directory to parse it as Mod source");
  this->_createTooltip(IDC_BC_BRW01,  L"Browse to select a folder to parse as Mod source");
  this->_createTooltip(IDC_EC_INP01,  L"Path to directory parsed as Mod source");
  this->_createTooltip(IDC_BC_RAD02,  L"Use an existing Mod Pack file to parse it as source");
  this->_createTooltip(IDC_BC_BRW02,  L"Browse to open a Mod Pack file to parse as source");
  this->_createTooltip(IDC_EC_INP02,  L"Path to Mod Pack file parsed as source");

  this->_createTooltip(IDC_EC_INP03,  L"Mod name");
  this->_createTooltip(IDC_EC_INP04,  L"Mod version string");
  this->_createTooltip(IDC_CB_EXT,    L"Mod Pack filename extension");

  this->_createTooltip(IDC_CB_ZMD,    L"Mod Pack compression method");
  this->_createTooltip(IDC_CB_ZLV,    L"Mod Pack compression level");

  this->_createTooltip(IDC_EC_INP06,  L"Save destination folder, where Mod Pack will be saved");
  this->_createTooltip(IDC_BC_BRW03,  L"Browse to select destination folder");

  this->_createTooltip(IDC_BC_SAVE,   L"Build and save Mod Pack");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort process");

  this->_createTooltip(IDC_BC_CKBX1,  L"Define dependencies for this Mod");
  this->_createTooltip(IDC_EC_INP07,  L"Dependency identity, the Mod identity to set as dependency");
  this->_createTooltip(IDC_BC_ADD,    L"Insert identity to dependency list");
  this->_createTooltip(IDC_BC_DEL,    L"Remove selected entry from dependency list");
  this->_createTooltip(IDC_LB_DPN,    L"List of Mod dependencies");

  this->_createTooltip(IDC_BC_CKBX2,  L"Define a snapshot for this Mod");
  this->_createTooltip(IDC_BC_BRW04,  L"Browse to select an image file to set as snapshot");

  this->_createTooltip(IDC_BC_CKBX3,  L"Define a description for this Mod");
  this->_createTooltip(IDC_BC_BRW05,  L"Browse to open text file and use its content as description");
  this->_createTooltip(IDC_EC_DESC,   L"Mod description text");

  // Set font for description
  HFONT hFt = Om_createFont(14, 400, L"Consolas");
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  // Set default package picture
  this->setStImage(IDC_SB_SNAP, Om_getResImage(this->_hins, IDB_BLANK));
  // Set buttons inner icons
  this->setBmIcon(IDC_BC_BRW01, Om_getResIcon(this->_hins, IDI_BT_NEW));
  this->setBmIcon(IDC_BC_BRW02, Om_getResIcon(this->_hins, IDI_BT_OPN));
  this->setBmIcon(IDC_BC_ADD, Om_getResIcon(this->_hins, IDI_BT_ENT));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDI_BT_REM));

  // Enable Create From folder
  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, 1);

  // add items to extension ComboBox
  this->msgItem(IDC_CB_EXT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L".zip"));
  this->msgItem(IDC_CB_EXT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"." OM_PKG_FILE_EXT));
  this->msgItem(IDC_CB_EXT, CB_SETCURSEL, 0, 0);

  // add items to Compression Method ComboBox
  this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None (Store only)"));
  this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Deflate (Legacy Zip)"));
  this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"LZMA"));
  this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"LZMA2"));
  this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Zstandard"));
  this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 4, 0);

  // add items into Compression Level ComboBox
  this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None (Store only)"));
  this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Fast"));
  this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal"));
  this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Best"));
  this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 2, 0);

  // add items into Category ComboBox
  for(size_t i = 0; i < OmModCategoryCount; ++i) {
    this->msgItem(IDC_CB_CAT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(OmModCategory[i]));
  }
  this->msgItem(IDC_CB_CAT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"- CUSTOM -"));
  this->msgItem(IDC_CB_CAT, CB_SETCURSEL, 0, 0);

  // Set snapshot format advice
  this->setItemText(IDC_SC_NOTES, L"Optimal format:\nSquare image of 128 x 128 pixels");

  // disable the Save & Abort button
  this->enableItem(IDC_BC_SAVE, false);
  this->enableItem(IDC_BC_ABORT, false);

  // Parse initial source if any
  if(!this->_initial_src.empty()) {

    // select initial source
    this->selectSource(this->_initial_src);

    // Reset initial source path
    this->_initial_src.clear();
  }

  // Nothing to save
  this->_unsaved = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onResize()
{
  unsigned half_w = static_cast<float>(this->cliUnitX()) * 0.5f;

  // -- Left Frame --

  // From folder RadioButton
  this->_setItemPos(IDC_BC_RAD01, 10, 10, 150, 9);
  // From Folder EditControl & Browse Button
  this->_setItemPos(IDC_EC_INP01, 10, 20, half_w-75, 13);
  this->_setItemPos(IDC_BC_BRW01, half_w-60, 19, 50, 14);

  // From existing Package RadioButton
  this->_setItemPos(IDC_BC_RAD02, 10, 40, 150, 9);
  // From existing Package EditControl & Browse Button
  this->_setItemPos(IDC_EC_INP02, 10, 50, half_w-75, 13);
  this->_setItemPos(IDC_BC_BRW02, half_w-60, 49, 50, 14);

  // [ - - - File name & Zip compression GroupBox - - -
  this->_setItemPos(IDC_GB_GRP01, 5, 65, half_w-10, this->cliUnitY()-147);
  // Package filename Label
  this->_setItemPos(IDC_SC_LBL01, 10, 75, 120, 9);
  // Name Label & EditText
  this->_setItemPos(IDC_SC_LBL02, 10, 90, 23, 9);
  this->_setItemPos(IDC_EC_INP03, 35, 88, half_w-145, 13);
  // Version label & EditText
  this->_setItemPos(IDC_SC_LBL03, half_w-105, 90, 27, 9);
  this->_setItemPos(IDC_EC_INP04, half_w-75, 88, 30, 13);
  // File extension ComboBox
  this->_setItemPos(IDC_CB_EXT, half_w-40, 88, 30, 13);
  // Output filname EditText
  this->_setItemPos(IDC_EC_RESUL, 10, 104, half_w-20, 13);
  // Compression Method Label & ComboBox
  this->_setItemPos(IDC_SC_LBL09, 10, 125, 90, 9);
  this->_setItemPos(IDC_CB_ZMD, 10, 137, half_w-140, 13);
  // Compression Level Label & ComboBox
  this->_setItemPos(IDC_SC_LBL04, 120, 125, 90, 9);
  this->_setItemPos(IDC_CB_ZLV, 120, 137, half_w-130, 13);
  // Package content Label & output EditText
  this->_setItemPos(IDC_SC_LBL07, 10, 158, 150, 13);
  this->_setItemPos(IDC_EC_READ1, 10, 170, half_w-20, this->cliUnitY()-258);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // Destination Label
  this->_setItemPos(IDC_SC_LBL05, 10, this->cliUnitY()-77, 120, 8);
  // Destination EditText & Browse Button
  this->_setItemPos(IDC_EC_INP06, 10, this->cliUnitY()-65, half_w-40, 13);
  this->_setItemPos(IDC_BC_BRW03, half_w-25, this->cliUnitY()-65, 15, 13);

  // Save Button
  this->_setItemPos(IDC_BC_SAVE, 10, this->cliUnitY()-45, 45, 14);
  // Progress Bar
  this->_setItemPos(IDC_PB_PKG, 57, this->cliUnitY()-44, half_w-114, 12);
  // Abort Button
  this->_setItemPos(IDC_BC_ABORT, half_w-55, this->cliUnitY()-45, 45, 14);

  // -- Right Frame --

  // [ - - -         Category GroupBox           - - -
  this->_setItemPos(IDC_GB_GRP02, half_w+5, 0, half_w-10, 60);
  // Category Label
  this->_setItemPos(IDC_SC_LBL08, half_w+10, 10, 120, 9);
  // Category ComboBox & EditText
  this->_setItemPos(IDC_CB_CAT, half_w+10, 25, half_w-20, 14);
  // Category ComboBox & EditText
  this->_setItemPos(IDC_EC_INP09, half_w+10, 40, half_w-20, 14);

  // [ - - -       Dependencies GroupBox          - - -
  this->_setItemPos(IDC_GB_GRP03, half_w+5, 60, half_w-10, 75);
  // Has Dependencies CheckBox
  this->_setItemPos(IDC_BC_CKBX1, half_w+10, 70, 120, 9);
  // Ident Label, EditText & + Button
  this->_setItemPos(IDC_SC_LBL06, half_w+10, 87, 35, 9);
  this->_setItemPos(IDC_EC_INP07, half_w+40, 85, half_w-70, 13);
  this->_setItemPos(IDC_BC_ADD, this->cliUnitX()-25, 85, 16, 13);
  // Depend ListBox & Trash Button
  this->_setItemPos(IDC_LB_DPN, half_w+10, 100, half_w-40, 30);
  this->_setItemPos(IDC_BC_DEL, this->cliUnitX()-25, 100, 16, 13);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Snapshot GroupBox             - - -
  this->_setItemPos(IDC_GB_GRP04, half_w+5, 135, half_w-10, 95);
  // Include snapshot CheckBox
  this->_setItemPos(IDC_BC_CKBX2, half_w+10, 145, 70, 9);
  // Snapshot Bitmap & Select... Button
  this->_setItemPos(IDC_SB_SNAP, this->cliUnitX()-160, 146, 86, 79);
  this->_setItemPos(IDC_BC_BRW04, this->cliUnitX()-50, 145, 40, 13);
  // Snapshot hidden EditText
  this->_setItemPos(IDC_EC_INP08, half_w+10, 160, 120, 13); // hidden
  // Snapshot helper Static text
  this->_setItemPos(IDC_SC_NOTES, this->cliUnitX()-230, 180, 60, 35);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Description GroupBox          - - -
  this->_setItemPos(IDC_GB_GRP05, half_w+5, 230, half_w-10, this->cliUnitY()-260);
  // Description CheckBox & Load.. Button
  this->_setItemPos(IDC_BC_CKBX3, half_w+10, 240, 100, 9);
  this->_setItemPos(IDC_BC_BRW05, this->cliUnitX()-50, 240, 40, 13);
  // Description EditText
  this->_setItemPos(IDC_EC_DESC, half_w+10, 255, half_w-20, this->cliUnitY()-290);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // ----- Separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->cliUnitY()-25, this->cliUnitX()-10, 1);
  // Close Button
  this->_setItemPos(IDC_BC_CLOSE, this->cliUnitX()-54, this->cliUnitY()-19, 50, 14);

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onClose()
{
  // Check for unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgCloseUnsaved(this->_hwnd, L"Mod Pack Editor")) {
      return; //< do NOT close
    }
  }

  // gracefully exit current thread before exist
  if(this->_save_hth != nullptr) {
    this->_save_abort = true;
    this->_save_stop();
  }

  this->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiToolPkg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_PKGSAVE_DONE is a custom message sent from Package Save
  // thread function, to notify the thread ended is job.
  if(uMsg == UWM_PKGSAVE_DONE) {
    // properly stop the running thread and finish process
    this->_save_stop();
    return false;
  }

  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    OmWString item_str;

    switch(LOWORD(wParam))
    {
    case IDC_BC_RAD01:
    case IDC_BC_RAD02:
      this->_onBcRadSrc();
      break;

    case IDC_BC_BRW01: //< Create from folder "Select..." Button
      has_changed = this->_onBcBrwDir();
      break;

    case IDC_EC_INP01:
      // check for content changes
      //if(HIWORD(wParam) == EN_CHANGE)
        //has_changed = true;
      break;

    case IDC_BC_BRW02: //< Edit from existing "Open..." Button
      has_changed = this->_onBcBrwPkg();
      break;

    case IDC_EC_INP02:
      // check for content changes
      //if(HIWORD(wParam) == EN_CHANGE)
        //has_changed = true;
      break;

    case IDC_EC_INP03: //< Package Name input EditText
    case IDC_EC_INP04: //< Package Version input EditText
      if(HIWORD(wParam) == EN_CHANGE) {
        this->_onNameChange();
        has_changed = true;
      }
      break;
    case IDC_CB_EXT: //< File Extension ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        this->_onNameChange();
        has_changed = true;
      }
      break;

    case IDC_BC_BRW03:  // Destination folder "..." Button
      this->_onBcBrwDest();
      break;

    case IDC_EC_INP06: //< Destination folder input EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
      break;

    case IDC_CB_CAT: //< Location ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE) this->_onCbCatSel();
      break;

    case IDC_BC_CKBX1: //< Has Dependencies CheckBox
      this->_onCkBoxDep();
    break;

    case IDC_EC_INP07: //< Dependencies input EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE) {
        this->getItemText(IDC_EC_INP07, item_str);
        this->enableItem(IDC_BC_ADD, !item_str.empty());
      }
      break;

    case IDC_LB_DPN: //< Dependencies ListBox
      // check for selection change
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_onLbDpnlsSel();
      break;

    case IDC_BC_ADD: //< Dependencies "+" Button
      this->_onBcAddDep();
      has_changed = true;
      break;

    case IDC_BC_DEL: //< Dependencies "-" Button
      this->_onBcDelDep();
      has_changed = true;
      break;

    case IDC_BC_CKBX2:  //< Include snapshot CheckBox
      this->_onCkBoxSnap();
      break;

    case IDC_BC_BRW04: //< Snapshot "Select..." Button
      has_changed = this->_onBcBrwSnap();
      break;

    case IDC_BC_CKBX3: //< Include Description CheckBox
      this->_onCkBoxDesc();
    break;

    case IDC_BC_BRW05: //< Description "Load..." Button
      has_changed = this->_onBcBrwDesc();
      break;

    case IDC_EC_DESC: //< Description EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE) {
        this->enableItem(IDC_BC_SAVE, true); //< enable "Save" Button
        has_changed = true;
      }
      break;

    case IDC_BC_SAVE: //< Main "Save" Button
      this->_onBcSave();
      break;

    case IDC_BC_ABORT: //< Main "Abort" Button
      this->_save_abort = true;
      break;

    case IDC_BC_CLOSE: //< Main "Close" Button
      this->_onClose();
      break;
    }

    if(has_changed) {
      bool allow = false;

      // we have unsaved changes
      this->_unsaved = true;

      if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INP01, item_str);
      } else {
        this->getItemText(IDC_EC_INP02, item_str);
      }

      if(!item_str.empty()) {
        this->getItemText(IDC_EC_INP06, item_str);
        if(!item_str.empty()) allow = true;
      }

      this->enableItem(IDC_BC_SAVE, allow);
    }
  }

  return false;
}
