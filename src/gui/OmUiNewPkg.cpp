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
#include "gui/OmUiNewPkg.h"
#include "OmPackage.h"
#include "gui/OmUiProgress.h"

/// \brief Compatible image formats filter
///
/// Open dialog file filter for compatibles images file formats.
///
#define IMAGE_FILE_FILTER     L"Image file (*.bmp,*.jpg,*.jpeg,*.tiff,*.tif,*.gif,*.png)\0*.BMP;*.JPG;*.JPEG;*.TIFF;*.GIF;*.PNG;\
                              \0BMP file (*.bmp)\0*.BMP;\0JPEG file (*.jpg,*.jpeg)\0*.JPG;*.JPEG\0PNG file (*.png)\0*.PNG;\0CompuServe GIF (*.gif)\0*.GIF;\
                              \0TIFF file (*.tiff,*.tif)\0*.TIFF;*.TIF;\0"

/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the remLocation_fth
/// thread finished his job.
///
#define UWM_BUILDPKG_DONE    (WM_APP+1)

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewPkg::OmUiNewPkg(HINSTANCE hins) : OmDialog(hins),
  _hBlankImg(nullptr),
  _hImgSource(nullptr),
  _buildPkg_hth(nullptr)
{
  this->addChild(new OmUiProgress(hins)); //< for package creation process

  // load the package blank picture
  this->_hBlankImg = reinterpret_cast<HBITMAP>(LoadImage(this->_hins,MAKEINTRESOURCE(IDB_PKG_BLANK),IMAGE_BITMAP,OMM_PKG_THMB_SIZE,OMM_PKG_THMB_SIZE,0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewPkg::~OmUiNewPkg()
{
  DeleteObject(this->_hBlankImg);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiNewPkg::id() const
{
  return IDD_NEW_PKG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewPkg::_parsePkg(const wstring& path)
{
  OmPackage package;

  if(package.sourceParse(path)) {

    this->msgItem(IDC_LB_DPNDS, LB_RESETCONTENT);

    if(package.dependCount()) {
      this->msgItem(IDC_BC_CHK03, BM_SETCHECK, 1);
      this->enableItem(IDC_LB_DPNDS, true);
      this->enableItem(IDC_EC_INPT3, true);
      for(unsigned i = 0; i < package.dependCount(); ++i) {
        this->msgItem(IDC_LB_DPNDS, LB_ADDSTRING, i, (LPARAM)package.depend(i).c_str());
      }
    }

    if(package.picture()) {
      this->msgItem(IDC_BC_CHK04, BM_SETCHECK, 1);
      this->enableItem(IDC_BC_BROW4, true);
      this->_hImgSource = static_cast<HBITMAP>(CopyImage(package.picture(),IMAGE_BITMAP,0,0,0));
      HBITMAP hBm = Om_getBitmapThumbnail(this->_hImgSource, OMM_PKG_THMB_SIZE, OMM_PKG_THMB_SIZE);
      this->msgItem(IDC_SB_PKIMG, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);
      DeleteObject(hBm);
    }

    if(package.desc().size()) {
      this->msgItem(IDC_BC_CHK05, BM_SETCHECK, 1);
      this->enableItem(IDC_BC_BROW5, true);
      this->enableItem(IDC_EC_PKTXT, true);
      this->setItemText(IDC_EC_PKTXT, package.desc());
    }

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_buildPkg_init()
{
  OmUiProgress* uiProgress = reinterpret_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));

  // open the progress dialog
  uiProgress->open(true);
  uiProgress->setCaption(L"Building Package");
  uiProgress->setTitle(L"Preparing data...");

  // start package building thread
  DWORD dWid;
  this->_buildPkg_hth = CreateThread(nullptr, 0, this->_buildPkg_fth, this, 0, &dWid);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_buildPkg_stop()
{
  if(this->_buildPkg_hth) {
    WaitForSingleObject(this->_buildPkg_hth, INFINITE);
    CloseHandle(this->_buildPkg_hth);
    this->_buildPkg_hth = nullptr;
  }

  // Close progress dialog
  reinterpret_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS))->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiNewPkg::_buildPkg_fth(void* arg)
{
  OmUiNewPkg* self = reinterpret_cast<OmUiNewPkg*>(arg);

  OmUiProgress* uiProgress = reinterpret_cast<OmUiProgress*>(self->childById(IDD_PROGRESS));

  wstring item_str;

  OmPackage package(nullptr);

  // get source (either a folder or another package) and parse it
  if(self->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    self->getItemText(IDC_EC_INPT1, item_str);
  } else {
    self->getItemText(IDC_EC_INPT2, item_str);
  }

  if(!package.sourceParse(item_str)) {
    // TODO: Add some dialog error here... one day
    return 1;
  }

  // get package dependencies list
  if(self->msgItem(IDC_BC_CHK03, BM_GETCHECK)) {
    int lb_cnt = self->msgItem(IDC_LB_DPNDS, LB_GETCOUNT);
    if(lb_cnt) {
      size_t n;
      for(int i = 0; i < lb_cnt; ++i) {
        item_str.resize(self->msgItem(IDC_LB_DPNDS, LB_GETTEXTLEN, i));
        n = self->msgItem(IDC_LB_DPNDS, LB_GETTEXT, i, (LPARAM)&item_str[0]);
        if(n < item_str.size()) item_str.resize(n);
        package.addDepend(item_str);
      }
    }
  }

  // get package picture data
  if(self->msgItem(IDC_BC_CHK04, BM_GETCHECK)) {
    package.setPicture(self->_hImgSource);
  }

  // get package description text
  if(self->msgItem(IDC_BC_CHK05, BM_GETCHECK)) {
    self->getItemText(IDC_EC_PKTXT, item_str);
    package.setDesc(item_str);
  }

  // get package compression level
  LRESULT zip_lvl = self->msgItem(IDC_CB_LEVEL, CB_GETCURSEL);

  // get destination filename
  self->getItemText(IDC_EC_INPT4, item_str);

  // hide the main dialog
  self->hide();

  uiProgress->setTitle(L"Adding files to package...");

  HWND hPb = (HWND)uiProgress->getProgressBar();
  HWND hSc = (HWND)uiProgress->getStaticComment();

  wstring msg;

  if(!package.save(item_str, zip_lvl, hPb, hSc, uiProgress->getAbortPtr())) {

    // show error dialog box
    msg = L"An error occurred during Package creation:\n" + package.lastError();
    Om_dialogBoxErr(uiProgress->hwnd(), L"Package creation error", msg);

    // close the progress dialog
    uiProgress->quit();

  } else {

    // close the progress dialog
    uiProgress->quit();

    // show success dialog box
    msg = L"The Package \"" + Om_getFilePart(item_str);
    msg += L"\" was successfully created.";
    Om_dialogBoxInfo(self->_hwnd, L"Package creation success", msg);
  }

  PostMessage(self->_hwnd, UWM_BUILDPKG_DONE, 0, 0);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewPkg::_apply()
{
  wstring item_str;

  // Step 1, verify everything
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INPT1, item_str);
    if(!Om_isDir(item_str)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid source path",
                                    L"The specified source path is "
                                    L"not a valid folder.");
      return false;
    }
  } else {
    this->getItemText(IDC_EC_INPT2, item_str);
    if(!Om_isFileZip(item_str)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid source file",
                                    L"The specified source file is "
                                    L"not a valid zip file.");
      return false;
    }
  }

  this->getItemText(IDC_EC_INPT4, item_str);
  if(Om_isValidName(item_str)) {
    if(Om_isFile(item_str)) {
      wstring msg = L"The file \"" + Om_getFilePart(item_str);
      msg += L"\" already exists, do you want to overwrite the existing file ?";
      if(!Om_dialogBoxQuerry(this->_hwnd, L"File already exists", msg)) {
        return false;
      }
    }
  } else {
    Om_dialogBoxErr(this->_hwnd,  L"Invalid file name",
                                  L"The specified destination file "
                                  L"contains illegal characters.");
    return false;
  }

  // disable the OK button
  this->enableItem(IDC_BC_OK, false);

  this->_buildPkg_init();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_onInit()
{
  // Enable Create From folder
  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, 1);

  // add items into Combo-Box
  HWND hCb = this->getItem(IDC_CB_LEVEL);

  unsigned cb_cnt = SendMessageW(hCb, CB_GETCOUNT, 0, 0);

  if(!cb_cnt) {
    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)L"None ( very fast )");
    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)L"Low ( fast )");
    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)L"Normal ( slow )");
    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)L"Best ( very slow )");
  }
  SendMessageW(hCb, CB_SETCURSEL, 2, 0);

  // set default snapshot
  this->msgItem(IDC_SB_PKIMG, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)this->_hBlankImg);

  // set font for readme
  HFONT hFont = CreateFont(14,0,0,0,400,false,false,false,1,0,0,5,0,"Consolas");
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, (WPARAM)hFont, true);

  // Set Dependencies Add and Rem buttons "icon"
  this->setItemText(IDC_BC_ADD, L"\u2795"); // Heavy Plus Sign +
  //this->setItemText(IDC_BC_DEL, L"\u274C"); // Cross Mark
  this->setItemText(IDC_BC_DEL, L"\u2796"); // Heavy Minus Sign -

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_onResize()
{
  // From folder RadioButton
  this->_setItemPos(IDC_BC_RAD01, 10, 10, 150, 9);
  // From Folder EditControl & Brows Button
  this->_setItemPos(IDC_EC_INPT1, 10, 20, this->width()-38, 13);
  this->_setItemPos(IDC_BC_BROW1, this->width()-26, 20, 16, 13);

  // From existing Package RadioButton
  this->_setItemPos(IDC_BC_RAD02, 10, 40, 150, 9);
  // From existing Package EditControl & Brows Button
  this->_setItemPos(IDC_EC_INPT2, 10, 50, this->width()-38, 13);
  this->_setItemPos(IDC_BC_BROW2, this->width()-26, 50, 16, 13);

  // Dependencies CheckBox
  this->_setItemPos(IDC_BC_CHK03, 10, 70, 112, 9);
  // Dependencies EditControl a Add Button
  this->_setItemPos(IDC_EC_INPT3, 10, 80, this->width()-38, 13);
  this->_setItemPos(IDC_BC_ADD, this->width()-26, 80, 16, 13);
  // Dependencies ListBox & Del button
  this->_setItemPos(IDC_LB_DPNDS, 10, 95, this->width()-38, 24);
  this->_setItemPos(IDC_BC_DEL, this->width()-26, 95, 16, 13);

  // Picture CheckBox & Load button
  this->_setItemPos(IDC_BC_CHK04, 10, 125, 120, 9);
  this->_setItemPos(IDC_BC_BROW4, this->width()-60, 125, 50, 14);
  // Picture Bitmap & Label
  this->_setItemPos(IDC_SB_PKIMG, 10, 136, 85, 78);
  this->_setItemPos(IDC_SC_LBL04, 115, 165, 200, 9);

  // Description CheckBox & Load button
  this->_setItemPos(IDC_BC_CHK05, 10, 220, 120, 9);
  this->_setItemPos(IDC_BC_BROW5, this->width()-60, 220, 50, 14);
  // Description EditControl
  this->_setItemPos(IDC_EC_PKTXT, 10, 235, this->width()-20, this->height()-360);

  // Destination label
  this->_setItemPos(IDC_SC_LBL06, 10, this->height()-115, 120, 9);
  // Destination file name
  this->_setItemPos(IDC_EC_INPT4, 10, this->height()-105, this->width()-72, 14);
  // Destination brows button
  this->_setItemPos(IDC_BC_SAVE, this->width()-60, this->height()-105, 50, 14);

  // Parsed name label & entry
  this->_setItemPos(IDC_SC_LBL07, 10, this->height()-84, 50, 9);
  this->_setItemPos(IDC_EC_INPT5, 60, this->height()-85, this->width()-175, 12);

  // Parsed version label & entry
  this->_setItemPos(IDC_SC_LBL08, this->width()-101, this->height()-84, 51, 9);
  this->_setItemPos(IDC_EC_INPT6, this->width()-45, this->height()-85, 35, 12);

  // Zip Level Label
  this->_setItemPos(IDC_SC_LBL09, 10, this->height()-60, 120, 9);
  // Zip Level ComboBox
  this->_setItemPos(IDC_CB_LEVEL, 10, this->height()-50, this->width()-20, 14);

  // ----- Separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Save As... Button
  this->_setItemPos(IDC_BC_OK, this->width()-108, this->height()-19, 50, 14);
  // Close Button
  this->_setItemPos(IDC_BC_CANCEL, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_onQuit()
{
  DeleteObject(this->_hImgSource);
  this->_hImgSource = nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewPkg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_BUILDPKG_DONE is a custom message sent from Location deletion thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_BUILDPKG_DONE) {
    // end the removing Location process
    this->_buildPkg_stop();
    // quit the dialog
    this->quit();
    // refresh the main window dialog, this will also refresh this one
    this->root()->refresh();
  }

  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
    OmContext* curCtx = manager->curContext();
    OmLocation* curLoc = curCtx ? curCtx->curLocation() : nullptr;

    bool bm_chk;

    int lb_sel;

    wstring item_str, brow_str;

    switch(LOWORD(wParam))
    {
    case IDC_BC_RAD01:
    case IDC_BC_RAD02:
      bm_chk = this->msgItem(IDC_BC_RAD01, BM_GETCHECK);
      this->enableItem(IDC_EC_INPT1, bm_chk);
      this->enableItem(IDC_BC_BROW1, bm_chk);
      this->enableItem(IDC_EC_INPT2, !bm_chk);
      this->enableItem(IDC_BC_BROW2, !bm_chk);
      has_changed = true;
      break;

    case IDC_BC_BROW1:
      // select the initial location for browsing start
      this->getItemText(IDC_EC_INPT1, item_str);
      if(item_str.empty()) {
        item_str = curLoc->libraryDir();
      } else {
        item_str = Om_getDirPart(item_str);
      }
      if(Om_dialogBrowseDir(brow_str, this->_hwnd, L"Select installation file(s) location", item_str)) {
        this->setItemText(IDC_EC_INPT1, brow_str);
      }
      break;

    case IDC_BC_BROW2:
      // select the initial location for browsing start
      this->getItemText(IDC_EC_INPT2, item_str);
      if(item_str.empty()) {
        item_str = curLoc->libraryDir();
      } else {
        item_str = Om_getDirPart(item_str);
      }

      if(Om_dialogOpenFile(brow_str, this->_hwnd, L"Select Package file", OMM_PKG_FILES_FILTER, item_str)) {
        if(this->_parsePkg(brow_str)) { //< try to parse package
          this->setItemText(IDC_EC_INPT2, brow_str);
        } else {
          wstring err = L"The file \""; err += brow_str;
          err += L"\" is not valid Open Mod Manager Package.";
          Om_dialogBoxErr(this->_hwnd, L"Error parsing Package", err);
        }
      }
      break;

    case IDC_EC_INPT1:
      has_changed = true;
      break;

    case IDC_EC_INPT2:
      has_changed = true;
      break;

    case IDC_BC_CHK03: // Add Dependencies CheckBox
      bm_chk = this->msgItem(IDC_BC_CHK03, BM_GETCHECK);
      this->enableItem(IDC_LB_DPNDS, bm_chk);
      this->enableItem(IDC_EC_INPT3, bm_chk);
    break;

    case IDC_EC_INPT3: // Dependencies EditControl
      this->getItemText(IDC_EC_INPT3, item_str);
      this->enableItem(IDC_BC_ADD, !item_str.empty());
      break;

    case IDC_LB_DPNDS: // Dependencies ListBox
      lb_sel = this->msgItem(IDC_LB_DPNDS, LB_GETCURSEL);
      this->enableItem(IDC_BC_DEL, (lb_sel >= 0));
      break;

    case IDC_BC_ADD: // Add Dependency Button
      this->getItemText(IDC_EC_INPT3, item_str);
      if(!item_str.empty()) {
        this->msgItem(IDC_LB_DPNDS, LB_ADDSTRING, 0, (LPARAM)item_str.c_str());
        this->setItemText(IDC_EC_INPT3, L"");
      }
      this->enableItem(IDC_BC_ADD, false);
      break;

    case IDC_BC_DEL: // Remove Dependency Button
      lb_sel = this->msgItem(IDC_LB_DPNDS, LB_GETCURSEL);
      if(lb_sel >= 0) {
        this->msgItem(IDC_LB_DPNDS, LB_DELETESTRING, lb_sel);
      }
      this->enableItem(IDC_BC_DEL, false);
      break;

    case IDC_BC_CHK04:
      if(this->msgItem(IDC_BC_CHK04, BM_GETCHECK)) {
        this->enableItem(IDC_BC_BROW4, true);
      } else {
        this->enableItem(IDC_BC_BROW4, false);
        this->_hImgSource = nullptr;
        this->msgItem(IDC_SB_PKIMG, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)this->_hBlankImg);
      }
      break;

    case IDC_BC_BROW4:
      // select the start directory from package source path
      if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INPT1, item_str);
      } else {
        this->getItemText(IDC_EC_INPT2, item_str);
      }
      item_str = Om_getDirPart(item_str);

      if(Om_dialogOpenFile(brow_str, this->_hwnd, L"Open Image file", IMAGE_FILE_FILTER, item_str)) {
        if(Om_isFile(brow_str)) {
          this->_hImgSource = Om_loadBitmap(brow_str);
          HBITMAP hBm = Om_getBitmapThumbnail(this->_hImgSource, OMM_PKG_THMB_SIZE, OMM_PKG_THMB_SIZE);
          this->msgItem(IDC_SB_PKIMG, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);
          DeleteObject(hBm);
        }
      }
      break;

    case IDC_BC_CHK05:
      bm_chk = this->msgItem(IDC_BC_CHK05, BM_GETCHECK);
      this->enableItem(IDC_BC_BROW5, bm_chk);
      this->enableItem(IDC_EC_PKTXT, bm_chk);
    break;

    case IDC_BC_BROW5:
      // select the start directory from package source path
      if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INPT1, item_str);
      } else {
        this->getItemText(IDC_EC_INPT2, item_str);
      }
      item_str = Om_getDirPart(item_str);

      if(Om_dialogOpenFile(brow_str, this->_hwnd, L"Open Text file", L"Text file (*.txt)\0*.TXT\0", item_str)) {
        if(Om_isFile(brow_str)) {
          string text_str = Om_loadPlainText(brow_str);
          SetDlgItemTextA(this->_hwnd, IDC_EC_PKTXT, text_str.c_str());
        }
      }
      break;

    case IDC_BC_SAVE:
      // create the file initial name based on source folder name
      if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INPT1, item_str);
        brow_str = Om_getFilePart(item_str) + L".zip";
      } else {
        this->getItemText(IDC_EC_INPT2, item_str);
        brow_str = Om_getFilePart(item_str);
      }

      // select the initial location for browsing start
      if(curLoc) {
        item_str = curLoc->libraryDir();
      } else {
        item_str = Om_getDirPart(item_str);
      }

      if(Om_dialogSaveFile(brow_str, this->_hwnd, L"Save Package as...", OMM_PKG_FILES_FILTER, item_str)) {
        if(Om_isValidName(brow_str)) {
          this->setItemText(IDC_EC_INPT4, brow_str);
        } else {
          Om_dialogBoxErr(this->_hwnd, L"Invalid file name", L"The specified file name is not valid.");
        }
      }
      break;

    case IDC_EC_INPT4:
      this->getItemText(IDC_EC_INPT4, item_str);
      if(!item_str.empty()) {
        wstring name, vers;
        Om_parsePkgIdent(name, vers, item_str, true, true);
        this->setItemText(IDC_EC_INPT5, name);
        this->setItemText(IDC_EC_INPT6, vers);
      }
      has_changed = true;
      break;

    case IDC_BC_OK:
      this->_apply();
      break;

    case IDC_BC_CANCEL:
      this->quit();
      break;
    }

    if(has_changed) {
      bool allow = true;

      if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INPT1, item_str);
      } else {
        this->getItemText(IDC_EC_INPT2, item_str);
      }
      if(item_str.empty()) allow = false;

      this->getItemText(IDC_EC_INPT4, item_str);
      if(item_str.empty()) allow = false;

      this->enableItem(IDC_BC_OK, allow);
    }
  }

  return false;
}
