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

#include "gui/res/resource.h"
#include "OmManager.h"
#include "gui/OmUiToolPkg.h"
#include "OmPackage.h"
#include "OmImage.h"

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
  _save_hth(nullptr),
  _save_abort(false)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolPkg::~OmUiToolPkg()
{
  HBITMAP hBm = this->setStImage(IDC_SB_PKG, nullptr);
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);

  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_PKTXT, WM_GETFONT));
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
void OmUiToolPkg::_freeze(bool freeze)
{
  bool enable = !freeze;

  // disable all controls during creation
  this->enableItem(IDC_BC_RAD01, enable);
  this->enableItem(IDC_BC_RAD02, enable);
  this->enableItem(IDC_EC_INP03, enable);
  this->enableItem(IDC_EC_INP04, enable);
  this->enableItem(IDC_CB_EXT, enable);
  this->enableItem(IDC_CB_LVL, enable);
  this->enableItem(IDC_EC_INP05, enable);
  this->enableItem(IDC_BC_BRW03, enable);
  this->enableItem(IDC_BC_CHK01, enable);
  this->enableItem(IDC_BC_CHK02, enable);
  this->enableItem(IDC_BC_CHK03, enable);
  // disable according radios and check-boxes status
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->enableItem(IDC_EC_INP01, enable);
    this->enableItem(IDC_BC_BRW01, enable);
  } else {
    this->enableItem(IDC_EC_INP02, enable);
    this->enableItem(IDC_BC_BRW02, enable);
  }
  if(this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
    this->enableItem(IDC_EC_INP07, enable);
    if(enable) {
      wstring ident;
      this->getItemText(IDC_EC_INP07, ident);
      this->enableItem(IDC_BC_ADD, !ident.empty());
    } else {
      this->enableItem(IDC_BC_ADD, false);
    }
    this->enableItem(IDC_BC_DEL, enable);
    this->enableItem(IDC_LB_DPN, enable);
  }
  if(this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
    this->enableItem(IDC_BC_BRW04, enable);
  }
  if(this->msgItem(IDC_BC_CHK03, BM_GETCHECK)) {
    this->enableItem(IDC_BC_BRW05, enable);
    this->enableItem(IDC_EC_PKTXT, enable);
  }
  this->enableItem(IDC_BC_SAVE, enable);
  this->enableItem(IDC_BC_CLOSE, enable);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_parseSrc(const wstring& path)
{
  HBITMAP hBm;

  // reset current package
  this->_package.clear();

  // name and extension controls initial state
  this->setItemText(IDC_EC_INP03, L"");
  this->enableItem(IDC_EC_INP03, false);
  this->setItemText(IDC_EC_INP04, L"");
  this->enableItem(IDC_EC_INP04, false);
  this->enableItem(IDC_CB_EXT, false);
  this->enableItem(IDC_CB_LVL, false);
  this->setItemText(IDC_EC_OUT02, L"");

  // destination folder disabled
  this->enableItem(IDC_EC_INP06, false);
  this->enableItem(IDC_BC_BRW03, false);

  // disable the save button
  this->enableItem(IDC_BC_SAVE, false);

  // Dependencies initial state
  this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CHK01, false);
  this->msgItem(IDC_LB_DPN, LB_RESETCONTENT);
  this->enableItem(IDC_LB_DPN, false);
  this->setItemText(IDC_EC_INP07, L"");
  this->enableItem(IDC_EC_INP07, false);
  this->enableItem(IDC_BC_ADD, false);
  this->enableItem(IDC_BC_DEL, false);

  // Snapshot initial states
  this->msgItem(IDC_BC_CHK02, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CHK02, false);
  this->enableItem(IDC_BC_BRW04, false);
  hBm = this->setStImage(IDC_SB_PKG, Om_getResImage(this->_hins, IDB_PKG_THN));
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);
  this->setItemText(IDC_EC_INP08, L"");

  // Description initial states
  this->msgItem(IDC_BC_CHK03, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CHK03, false);
  this->enableItem(IDC_BC_BRW05, false);
  this->enableItem(IDC_EC_PKTXT, false);
  this->setItemText(IDC_EC_PKTXT, L"");

  // check whether source path is empty to
  // reset to initial state
  if(path.empty()) {
    return true;
  }

  // Try to parse the package
  if(!this->_package.srcParse(path)) {
    return false;
  }

  // enable controls for package edition
  this->enableItem(IDC_EC_INP03, true);
  this->enableItem(IDC_EC_INP04, true);
  this->enableItem(IDC_CB_EXT, true);
  this->enableItem(IDC_CB_LVL, true);

  this->enableItem(IDC_EC_INP06, true);
  this->enableItem(IDC_BC_BRW03, true);

  this->enableItem(IDC_BC_CHK01, true);
  this->enableItem(IDC_BC_CHK02, true);
  this->enableItem(IDC_BC_CHK03, true);

  // set source path to input EditText depending selected Radio
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->setItemText(IDC_EC_INP01, path);
  } else {
    this->setItemText(IDC_EC_INP02, path);
  }

  // check for package dependencies
  if(this->_package.depCount()) {
    this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 1);
    this->enableItem(IDC_LB_DPN, true);
    this->enableItem(IDC_EC_INP07, true);
    for(unsigned i = 0; i < this->_package.depCount(); ++i) {
      this->msgItem(IDC_LB_DPN, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(this->_package.depGet(i).c_str()));
    }
  }

  // check for package snapshot
  if(this->_package.image().thumbnail()) {
    this->msgItem(IDC_BC_CHK02, BM_SETCHECK, 1);
    this->enableItem(IDC_BC_BRW04, true);
    hBm = this->setStImage(IDC_SB_PKG, this->_package.image().thumbnail());
    if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);
  }

  // check for package description
  if(this->_package.desc().size()) {
    this->msgItem(IDC_BC_CHK03, BM_SETCHECK, 1);
    this->enableItem(IDC_BC_BRW05, true);
    this->enableItem(IDC_EC_PKTXT, true);
    this->setItemText(IDC_EC_PKTXT, this->_package.desc());
  }

  // Add package content to output EditText
  wstring content;
  unsigned n = this->_package.srcItemCount();
  for(unsigned i = 0; i < this->_package.srcItemCount(); ++i) {
    content.append(this->_package.srcItemGet(i).path);
    if(i < n - 1) content.append(L"\r\n");
  }
  this->setItemText(IDC_EC_OUT02, content);

  // update name and version
  this->setItemText(IDC_EC_INP03, this->_package.name());

  if(!this->_package.version().isNull())
    this->setItemText(IDC_EC_INP04, this->_package.version().asString());

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
    wstring item_str;
    this->getItemText(IDC_EC_OUT01, item_str);

    wstring info = L"The Package \""+Om_getFilePart(item_str);
    info += L"\" was successfully created.";

    Om_dialogBoxInfo(this->_hwnd, L"Package created", info);
  }

  // refresh the main window dialog, this will also refresh this one
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_save_progress_cb(void* ptr, size_t tot, size_t cur, const wchar_t* str)
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

  wstring item_str;

  // get package dependencies list
  if(self->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
    int lb_cnt = self->msgItem(IDC_LB_DPN, LB_GETCOUNT);
    if(lb_cnt) {
      wchar_t ident[OMM_ITM_BUFF];
      for(int i = 0; i < lb_cnt; ++i) {
        self->msgItem(IDC_LB_DPN, LB_GETTEXT, i, reinterpret_cast<LPARAM>(ident));
        self->_package.depAdd(ident);
      }
    }
  }

  // get package image data from specified file if any
  if(self->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
    self->getItemText(IDC_EC_INP08, item_str);
    if(!item_str.empty()) {
      self->_package.loadImage(item_str, OMM_PKG_THMB_SIZE);
    }
  }

  // get package description text
  if(self->msgItem(IDC_BC_CHK03, BM_GETCHECK)) {
    self->getItemText(IDC_EC_PKTXT, item_str);
    self->_package.setDesc(item_str);
  }

  // get package compression level
  int zip_lvl = self->msgItem(IDC_CB_LVL, CB_GETCURSEL);

  // get destination path & filename
  wstring out_path, out_file;
  self->getItemText(IDC_EC_INP06, out_path);
  self->getItemText(IDC_EC_OUT01, out_file);

  self->_save_abort = false;
  self->enableItem(IDC_BC_ABORT, true);

  DWORD exitCode = 0;

  if(!self->_package.save(out_path + L"\\" + out_file, zip_lvl, &self->_save_progress_cb, self)) {
    // show error dialog box
    wstring err = L"An error occurred during Package creation:\n";
    err += self->_package.lastError();
    Om_dialogBoxErr(self->_hwnd, L"Package creation error", err);

    exitCode = 1;
  }

  // if user aborted, the was package not successfully created
  if(self->_save_abort) {
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

  // enable or disable EditText and Buttons according selection
  this->enableItem(IDC_EC_INP01, bm_chk);
  this->enableItem(IDC_BC_BRW01, bm_chk);
  this->enableItem(IDC_EC_INP02, !bm_chk);
  this->enableItem(IDC_BC_BRW02, !bm_chk);

  // reset source input EditText
  this->setItemText(IDC_EC_INP01, L"");
  this->setItemText(IDC_EC_INP02, L"");

  // reset package source
  this->_parseSrc(L"");
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcBrwDir()
{
  OmContext* pCtx = static_cast<OmManager*>(this->_data)->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  wstring result, start;

  // Select start directory, either previous one or current location library
  this->getItemText(IDC_EC_INP01, start);

  if(start.empty()) {
    if(pLoc) start = pLoc->libDir();
  } else {
    start = Om_getDirPart(start);
  }

  if(!Om_dialogBrowseDir(result, this->_hwnd, L"Select installation file(s) location", start)) {
    this->setItemText(IDC_EC_INP01, L"");
    return;
  }

  if(!Om_isDir(result)) {
    this->setItemText(IDC_EC_INP01, L"");
    return;
  }

  // check whether selected folder is empty
  if(Om_isDirEmpty(result)) {

    // show warning dialog box
    wstring wrn = L"The selected source folder is empty, the resulting "
                  L"package will have no content to install."
                  L"\n\nDo you want to continue anyway ?";

    if(!Om_dialogBoxQuerry(this->_hwnd, L"Empty source folder", wrn)) {
      this->setItemText(IDC_EC_INP01, L"");
      return;
    }
  }

  // Try to parse the folder (as package source)
  if(!this->_parseSrc(result)) {
    // this cannot happen at this stage... but...
    wstring err = L"The folder \""+result+L"\" is not suitable for Package source.";
    Om_dialogBoxErr(this->_hwnd, L"Error parsing Package source folder", err);
    this->setItemText(IDC_EC_INP01, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcBrwPkg()
{
  OmContext* pCtx = static_cast<OmManager*>(this->_data)->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  wstring result, start;

  // Select start directory, either previous one or current location library
  this->getItemText(IDC_EC_INP02, start);

  if(start.empty()) {
    if(pLoc) start = pLoc->libDir();
  } else {
    start = Om_getDirPart(start);
  }

  // open select file dialog
  if(!Om_dialogOpenFile(result, this->_hwnd, L"Select Package file", OMM_PKG_FILES_FILTER, start)) {
    this->setItemText(IDC_EC_INP02, L"");
    return;
  }

  if(!Om_isFile(result)) {
    this->setItemText(IDC_EC_INP02, L"");
    return;
  }

  // parse this package source
  if(!this->_parseSrc(result)) {
    wstring err = L"The file \""+result+L"\" is not valid Package file.";
    Om_dialogBoxErr(this->_hwnd, L"Error parsing Package source file", err);
    this->setItemText(IDC_EC_INP02, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onNameChange()
{
  // get base name
  wstring name_str;
  this->getItemText(IDC_EC_INP03, name_str);

  if(!name_str.empty()) {

    // Replace all spaces by underscores
    std::replace(name_str.begin(), name_str.end(), L' ', L'_');

    // get version
    wstring vers_str;
    this->getItemText(IDC_EC_INP04, vers_str);

    // check if version string is valid
    OmVersion version(vers_str);
    if(!version.isNull()) {
      name_str += L"v" + version.asString();
    }

    // get chosen file extension
    int cb_sel = this->msgItem(IDC_CB_EXT, CB_GETCURSEL, 0, 0);
    wchar_t ext_str[6];
    this->msgItem(IDC_CB_EXT, CB_GETLBTEXT, cb_sel, reinterpret_cast<LPARAM>(ext_str));
    name_str += ext_str;

    // set final filename
    this->setItemText(IDC_EC_OUT01, name_str);

  } else {

    // reset output
    this->setItemText(IDC_EC_OUT01, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcBrwDest()
{
  wstring result, start;

  // select start directory
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP01, start);
  } else {
    this->getItemText(IDC_EC_INP02, start);
  }

  // open select folder dialog
  if(!Om_dialogBrowseDir(result, this->_hwnd, L"Select destination location", Om_getDirPart(start)))
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
void OmUiToolPkg::_onCkBoxDep()
{
  bool bm_chk = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);

  this->enableItem(IDC_EC_INP07, bm_chk);
  this->enableItem(IDC_LB_DPN, bm_chk);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcAddDep()
{
  // Get identity string from EditText
  wstring ident;
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
  if(this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {

    this->enableItem(IDC_BC_BRW04, true);

  } else {

    this->enableItem(IDC_BC_BRW04, false);

    HBITMAP hBm = this->setStImage(IDC_SB_PKG, Om_getResImage(this->_hins, IDB_PKG_THN));
    if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);

    this->setItemText(IDC_EC_INP08, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcBrwSnap()
{
  wstring result, start;

  // select the start directory from package source path
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP01, start);
  } else {
    this->getItemText(IDC_EC_INP02, start);
  }

  // open file dialog
  if(!Om_dialogOpenFile(result, this->_hwnd, L"Select image file", OMM_IMG_FILES_FILTER, Om_getDirPart(start)))
    return;

  OmImage image;
  HBITMAP hBm;

  // Try to open image
  if(image.open(result, OMM_PKG_THMB_SIZE)) {

    // set thumbnail
    hBm = this->setStImage(IDC_SB_PKG, image.thumbnail());
    if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);

    // set EditText content to image path
    this->setItemText(IDC_EC_INP08, result);

  } else {

    // remove any thumbnail
    hBm = this->setStImage(IDC_SB_PKG, Om_getResImage(this->_hins, IDB_PKG_THN));
    if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);

    // reset hidden EditText content
    this->setItemText(IDC_EC_INP08, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onCkBoxDesc()
{
  bool bm_chk = this->msgItem(IDC_BC_CHK03, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW05, bm_chk);
  this->enableItem(IDC_EC_PKTXT, bm_chk);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcBrwDesc()
{
  wstring result, start;

  // select the start directory from package source path
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP01, start);
  } else {
    this->getItemText(IDC_EC_INP02, start);
  }

  // open file dialog
  if(!Om_dialogOpenFile(result, this->_hwnd, L"Select text file", OMM_TXT_FILES_FILTER, Om_getDirPart(start)))
    return;

  if(!Om_isFile(result))
    return;

  // load as plain text and send to contro
  string text_str = Om_loadPlainText(result);
  SetDlgItemTextA(this->_hwnd, IDC_EC_PKTXT, text_str.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcSave()
{
  wstring item_str;

  // verify package has parsed source
  if(!this->_package.srcValid()) {
    // show error dialog box
    wstring err = L"Current package source is empty or invalid, "
                  L"please select a valid source.";
    Om_dialogBoxErr(this->_hwnd, L"Invalid package source", err);
    return;
  }

  // verify the destination path and name
  wstring out_name, out_dir;

  this->getItemText(IDC_EC_INP06, out_dir);
  this->getItemText(IDC_EC_OUT01, out_name);

  if(Om_isDir(out_dir)) {
    if(Om_isValidName(out_name)) {
      wstring out_path = out_dir + L"\\" + out_name;
      if(Om_isFile(out_path)) {
        wstring qry = L"The file \""+out_name+L"\"";
        qry += OMM_STR_QRY_OVERWRITE;
        if(!Om_dialogBoxQuerry(this->_hwnd, L"File already exists", qry)) {
          return;
        }
      }
    } else {
      wstring err = L"File name ";
      err += OMM_STR_ERR_VALIDNAME;
      Om_dialogBoxErr(this->_hwnd, L"Invalid file name", err);
      return;
    }
  } else {
    wstring err = L"The destination folder \""+out_dir+L"\"";
    err += OMM_STR_ERR_ISDIR;
    Om_dialogBoxErr(this->_hwnd, L"Invalid destination", err);
    return;
  }

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
  // define controls tool-tips
  this->_createTooltip(IDC_BC_BRW01, L"Select folder");
  this->_createTooltip(IDC_BC_BRW02, L"Select Package file");

  this->_createTooltip(IDC_EC_INP03, L"Package name");
  this->_createTooltip(IDC_EC_INP04, L"Package version");
  this->_createTooltip(IDC_CB_EXT, L"Filename extension");

  this->_createTooltip(IDC_CB_LVL, L"Package ZIP compression level");

  this->_createTooltip(IDC_EC_INP06, L"Save destination path");
  this->_createTooltip(IDC_BC_BRW03, L"Select destination folder");

  this->_createTooltip(IDC_BC_SAVE, L"Save Package");
  this->_createTooltip(IDC_BC_ABORT, L"Abort process");

  this->_createTooltip(IDC_BC_CHK01, L"Defines dependencies for this Package");
  this->_createTooltip(IDC_EC_INP07, L"Dependency package identity");
  this->_createTooltip(IDC_LB_DPN, L"Dependencies list");

  this->_createTooltip(IDC_BC_CHK02, L"Defines a snapshot for this Package");
  this->_createTooltip(IDC_BC_BRW04, L"Select image file");

  this->_createTooltip(IDC_BC_CHK03, L"Defines a description for this Package");
  this->_createTooltip(IDC_BC_BRW05, L"Select text file");
  this->_createTooltip(IDC_EC_PKTXT, L"Package description text");

  // Set font for description
  HFONT hFt = Om_createFont(14, 400, L"Consolas");
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  // Set default package picture
  this->setStImage(IDC_SB_PKG, Om_getResImage(this->_hins, IDB_PKG_THN));
  // Set buttons inner icons
  this->setBmImage(IDC_BC_ADD, Om_getResImage(this->_hins, IDB_BTN_ENT));
  this->setBmImage(IDC_BC_DEL, Om_getResImage(this->_hins, IDB_BTN_REM));

  // Enable Create From folder
  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, 1);

  // add items to extension ComboBox
  wstring ext = L"."; ext += OMM_PKG_FILE_EXT;
  this->msgItem(IDC_CB_EXT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L".zip"));
  this->msgItem(IDC_CB_EXT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ext.c_str()));
  this->msgItem(IDC_CB_EXT, CB_SETCURSEL, 0, 0);

  // add items into Zip Level ComboBox
  this->msgItem(IDC_CB_LVL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None ( very fast )"));
  this->msgItem(IDC_CB_LVL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Low ( fast )"));
  this->msgItem(IDC_CB_LVL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal ( slow )"));
  this->msgItem(IDC_CB_LVL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Best ( very slow )"));
  this->msgItem(IDC_CB_LVL, CB_SETCURSEL, 2, 0);

  // Set snapshot format advice
  this->setItemText(IDC_SC_NOTES, L"Optimal format:\nSquare image of 128 x 128 pixels");

  // disable the Save & Abort button
  this->enableItem(IDC_BC_SAVE, false);
  this->enableItem(IDC_BC_ABORT, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onResize()
{
  unsigned half_w = static_cast<float>(this->width()) * 0.5f;

  // -- Left Frame --

  // From folder RadioButton
  this->_setItemPos(IDC_BC_RAD01, 10, 10, 150, 9);
  // From Folder EditControl & Browse Button
  this->_setItemPos(IDC_EC_INP01, 10, 20, half_w-65, 13);
  this->_setItemPos(IDC_BC_BRW01, half_w-50, 20, 40, 13);

  // From existing Package RadioButton
  this->_setItemPos(IDC_BC_RAD02, 10, 40, 150, 9);
  // From existing Package EditControl & Browse Button
  this->_setItemPos(IDC_EC_INP02, 10, 50, half_w-65, 13);
  this->_setItemPos(IDC_BC_BRW02, half_w-50, 50, 40, 13);

  // [ - - - File name & Zip compression GroupBox - - -
  this->_setItemPos(IDC_GB_GRP01, 5, 65, half_w-10, this->height()-147);
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
  this->_setItemPos(IDC_EC_OUT01, 10, 104, half_w-20, 13);
  // Zip compression Label & ComboBox
  this->_setItemPos(IDC_SC_LBL04, 10, 125, 120, 9);
  this->_setItemPos(IDC_CB_LVL, 10, 137, half_w-20, 13);
  // Package content Label & output EditText
  this->_setItemPos(IDC_SC_LBL07, 10, 158, 150, 13);
  this->_setItemPos(IDC_EC_OUT02, 10, 170, half_w-20, this->height()-258);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // Destination Label
  this->_setItemPos(IDC_SC_LBL05, 10, this->height()-77, 120, 8);
  // Destination EditText & Browse Button
  this->_setItemPos(IDC_EC_INP06, 10, this->height()-65, half_w-40, 13);
  this->_setItemPos(IDC_BC_BRW03, half_w-25, this->height()-65, 15, 13);

  // Save Button
  this->_setItemPos(IDC_BC_SAVE, 10, this->height()-45, 45, 14);
  // Progress Bar
  this->_setItemPos(IDC_PB_PKG, 57, this->height()-44, half_w-114, 12);
  // Abort Button
  this->_setItemPos(IDC_BC_ABORT, half_w-55, this->height()-45, 45, 14);

  // -- Right Frame --

  // [ - - -       Dependencies GroupBox          - - -
  this->_setItemPos(IDC_GB_GRP02, half_w+5, 0, half_w-10, 80);
  // Has Dependencies CheckBox
  this->_setItemPos(IDC_BC_CHK01, half_w+10, 10, 120, 9);
  // Ident Label, EditText & + Button
  this->_setItemPos(IDC_SC_LBL06, half_w+10, 27, 35, 9);
  this->_setItemPos(IDC_EC_INP07, half_w+40, 25, half_w-70, 13);
  this->_setItemPos(IDC_BC_ADD, this->width()-25, 25, 16, 13);
  // Depend ListBox & Trash Button
  this->_setItemPos(IDC_LB_DPN, half_w+10, 45, half_w-40, 30);
  this->_setItemPos(IDC_BC_DEL, this->width()-25, 45, 16, 13);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Snapshot GroupBox             - - -
  this->_setItemPos(IDC_GB_GRP03, half_w+5, 85, half_w-10, 95);
  // Include snapshot CheckBox
  this->_setItemPos(IDC_BC_CHK02, half_w+10, 95, 65, 9);
  // Snapshot Bitmap & Select... Button
  this->_setItemPos(IDC_SB_PKG, this->width()-160, 96, 85, 78);
  this->_setItemPos(IDC_BC_BRW04, this->width()-50, 95, 40, 13);
  // Snapshot hidden EditText
  this->_setItemPos(IDC_EC_INP08, half_w+10, 110, 120, 13); // hidden
  // Snapshot helper Static text
  this->_setItemPos(IDC_SC_NOTES, this->width()-230, 125, 60, 35);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Description GroupBox          - - -
  this->_setItemPos(IDC_GB_GRP04, half_w+5, 185, half_w-10, this->height()-215);
  // Description CheckBox & Load.. Button
  this->_setItemPos(IDC_BC_CHK03, half_w+10, 195, 100, 9);
  this->_setItemPos(IDC_BC_BRW05, this->width()-50, 195, 40, 13);
  // Description EditText
  this->_setItemPos(IDC_EC_PKTXT, half_w+10, 210, half_w-20, this->height()-245);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // ----- Separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Close Button
  this->_setItemPos(IDC_BC_CLOSE, this->width()-54, this->height()-19, 50, 14);
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
  // prevent to close dialog while in process
  if(this->_save_hth == nullptr) {
    this->quit();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    wstring item_str;

    switch(LOWORD(wParam))
    {
    case IDC_BC_RAD01:
    case IDC_BC_RAD02:
      this->_onBcRadSrc();
      has_changed = true;
      break;

    case IDC_BC_BRW01: //< Create from folder "Select..." Button
      this->_onBcBrwDir();
      break;

    case IDC_EC_INP01:
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
      break;

    case IDC_BC_BRW02: //< Edit from existing "Open..." Button
      this->_onBcBrwPkg();
      break;

    case IDC_EC_INP02:
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
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

    case IDC_BC_CHK01: //< Has Dependencies CheckBox
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
      break;

    case IDC_BC_DEL: //< Dependencies "-" Button
      this->_onBcDelDep();
      break;

    case IDC_BC_CHK02:  //< Include snapshot CheckBox
      this->_onCkBoxSnap();
      break;

    case IDC_BC_BRW04: //< Snapshot "Select..." Button
      this->_onBcBrwSnap();
      break;

    case IDC_BC_CHK03: //< Include Description CheckBox
      this->_onCkBoxDesc();
    break;

    case IDC_BC_BRW05: //< Description "Load..." Button
      this->_onBcBrwDesc();
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
      bool allow = true;

      if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INP01, item_str);
      } else {
        this->getItemText(IDC_EC_INP02, item_str);
      }
      if(!item_str.empty()) {

        this->getItemText(IDC_EC_INP06, item_str);
        if(item_str.empty()) allow = false;

      } else {
        allow = false;
      }

      this->enableItem(IDC_BC_SAVE, allow);
    }
  }

  return false;
}