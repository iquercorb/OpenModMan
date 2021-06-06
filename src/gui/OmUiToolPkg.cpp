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
#include "gui/OmUiProgress.h"
#include "OmImage.h"

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
OmUiToolPkg::OmUiToolPkg(HINSTANCE hins) : OmDialog(hins),
  _hFtMonos(Om_createFont(14, 400, L"Consolas")),
  _hBmBlank(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_PKG_BLANK),IMAGE_BITMAP,0,0,0))),
  _hBmBcNew(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_ADD), IMAGE_BITMAP, 0, 0, 0))),
  _hBmBcDel(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_REM), IMAGE_BITMAP, 0, 0, 0))),
  _image(),
  _buildPkg_hth(nullptr)
{
  this->addChild(new OmUiProgress(hins)); //< for package creation process
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolPkg::~OmUiToolPkg()
{
  DeleteObject(this->_hFtMonos);
  DeleteObject(this->_hBmBlank);
  DeleteObject(this->_hBmBcNew);
  DeleteObject(this->_hBmBcDel);
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
bool OmUiToolPkg::_parsePkg(const wstring& path)
{
  OmPackage package;

  if(package.sourceParse(path)) {

    this->msgItem(IDC_LB_DPNDS, LB_RESETCONTENT);

    if(package.dependCount()) {
      this->msgItem(IDC_BC_CHK03, BM_SETCHECK, 1);
      this->enableItem(IDC_LB_DPNDS, true);
      this->enableItem(IDC_EC_INPT3, true);
      for(unsigned i = 0; i < package.dependCount(); ++i) {
        this->msgItem(IDC_LB_DPNDS, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(package.depend(i).c_str()));
      }
    }

    if(package.image().thumbnail()) {
      this->msgItem(IDC_BC_CHK04, BM_SETCHECK, 1);
      this->enableItem(IDC_BC_BROW4, true);
      this->msgItem(IDC_SB_PKIMG, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(package.image().thumbnail()));
      this->setItemText(IDC_EC_INPT4, L"");
    }

    if(package.desc().size()) {
      this->msgItem(IDC_BC_CHK05, BM_SETCHECK, 1);
      this->enableItem(IDC_BC_BROW5, true);
      this->enableItem(IDC_EC_PKTXT, true);
      this->setItemText(IDC_EC_PKTXT, package.desc());
    }

    return true;

  } else {

    wstring err = L"The file \"";
    err += path;
    err += L"\" is not valid Package file.";
    Om_dialogBoxErr(this->_hwnd, L"Error parsing Package file", err);

    return false;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_buildPkg_init()
{
  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));

  // open the progress dialog
  pUiProgress->open(true);
  pUiProgress->setTitle(L"Package creation");
  pUiProgress->setDesc(L"Preparing data");

  // start package building thread
  DWORD dWid;
  this->_buildPkg_hth = CreateThread(nullptr, 0, this->_buildPkg_fth, this, 0, &dWid);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_buildPkg_stop()
{
  DWORD exitCode;

  if(this->_buildPkg_hth) {
    WaitForSingleObject(this->_buildPkg_hth, INFINITE);
    GetExitCodeThread(this->_buildPkg_hth, &exitCode);
    CloseHandle(this->_buildPkg_hth);
    this->_buildPkg_hth = nullptr;
  }

  // quit the progress dialog
  static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS))->quit();

  // show a reassuring dialog message
  if(exitCode == 0) {

    wstring item_str;

    // get destination filename
    this->getItemText(IDC_EC_INPT6, item_str);

    wstring info = L"The Package \"" + Om_getFilePart(item_str);
    info += L"\" was successfully created.";

    Om_dialogBoxInfo(this->_hwnd, L"Package creation success", info);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiToolPkg::_buildPkg_fth(void* arg)
{
  OmUiToolPkg* self = static_cast<OmUiToolPkg*>(arg);

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(self->childById(IDD_PROGRESS));

  wstring item_str, out_path;

  OmPackage package(nullptr);

  // get source (either a folder or another package) and parse it
  if(self->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    self->getItemText(IDC_EC_INPT1, item_str);
  } else {
    self->getItemText(IDC_EC_INPT2, item_str);
  }

  if(!package.sourceParse(item_str)) {
    // show error dialog box
    wstring err = L"Source data parsing failed.\n\n";
    err += package.lastError();
    Om_dialogBoxErr(pUiProgress->hwnd(), L"Package creation error", err);
    return 1;
  }

  // get package dependencies list
  if(self->msgItem(IDC_BC_CHK03, BM_GETCHECK)) {
    int lb_cnt = self->msgItem(IDC_LB_DPNDS, LB_GETCOUNT);
    if(lb_cnt) {
      size_t n;
      for(int i = 0; i < lb_cnt; ++i) {
        item_str.resize(self->msgItem(IDC_LB_DPNDS, LB_GETTEXTLEN, i));
        n = self->msgItem(IDC_LB_DPNDS, LB_GETTEXT, i, reinterpret_cast<LPARAM>(&item_str[0]));
        if(n < item_str.size()) item_str.resize(n);
        package.addDepend(item_str);
      }
    }
  }

  // get package image data from specified file if any
  if(self->msgItem(IDC_BC_CHK04, BM_GETCHECK)) {
    self->getItemText(IDC_EC_INPT4, item_str);
    if(!item_str.empty()) {
      package.loadImage(item_str, OMM_PKG_THMB_SIZE);
    }
  }

  // get package description text
  if(self->msgItem(IDC_BC_CHK05, BM_GETCHECK)) {
    self->getItemText(IDC_EC_PKTXT, item_str);
    package.setDesc(item_str);
  }

  // get package compression level
  LRESULT zip_lvl = self->msgItem(IDC_CB_LEVEL, CB_GETCURSEL);

  // get destination filename
  self->getItemText(IDC_EC_INPT6, out_path);

  // hide the main dialog
  self->hide();

  pUiProgress->setDesc(L"Adding files to package");

  HWND hPb = pUiProgress->getPbHandle();
  HWND hSc = pUiProgress->getDetailScHandle();

  DWORD exitCode = 0;

  if(!package.save(out_path, zip_lvl, hPb, hSc, pUiProgress->getAbortPtr())) {
    // show error dialog box
    wstring err = L"An error occurred during Package creation:\n";
    err += package.lastError();
    Om_dialogBoxErr(pUiProgress->hwnd(), L"Package creation error", err);

    exitCode = 1;
  }

  PostMessage(self->_hwnd, UWM_BUILDPKG_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_apply()
{
  wstring item_str;

  // Step 1, verify everything
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INPT1, item_str);
    if(!Om_isDir(item_str)) {
      wstring wrn = L"The folder \""+item_str+L"\"";
      wrn += OMM_STR_ERR_ISDIR;
      Om_dialogBoxWarn(this->_hwnd, L"Invalid source path", wrn);
      return false;
    }
  } else {
    this->getItemText(IDC_EC_INPT2, item_str);
    if(!Om_isFileZip(item_str)) {
      wstring wrn = L"The file \""+item_str+L"\"";
      wrn += OMM_STR_ERR_ISFILEZIP;
      Om_dialogBoxWarn(this->_hwnd, L"Invalid source file", wrn);
      return false;
    }
  }

  this->getItemText(IDC_EC_INPT6, item_str);
  if(Om_isDir(Om_getDirPart(item_str))) {
    if(Om_isValidName(Om_getFilePart(item_str))) {
      if(Om_isFile(item_str)) {
        wstring qry = L"The file \""+Om_getFilePart(item_str)+L"\"";
        qry += OMM_STR_QRY_OVERWRITE;
        if(!Om_dialogBoxQuerry(this->_hwnd, L"File already exists", qry)) {
          return false;
        }
      }
    } else {
      wstring err = L"File name ";
      err += OMM_STR_ERR_VALIDNAME;
      Om_dialogBoxErr(this->_hwnd, L"Invalid file name", err);
      return false;
    }
  } else {
    wstring err = L"The destination folder \""+Om_getDirPart(item_str)+L"\"";
    err += OMM_STR_ERR_ISDIR;
    Om_dialogBoxErr(this->_hwnd, L"Invalid destination", err);
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
void OmUiToolPkg::_onInit()
{
  // Set font for description
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtMonos), true);
  // Set default package picture
  this->msgItem(IDC_SB_PKIMG, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBlank));
  // Set buttons inner icons
  this->msgItem(IDC_BC_ADD,   BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcNew));
  this->msgItem(IDC_BC_DEL,   BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcDel));

  // Enable Create From folder
  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, 1);

  // add items into Combo-Box
  HWND hCb = this->getItem(IDC_CB_LEVEL);

  unsigned cb_cnt = SendMessageW(hCb, CB_GETCOUNT, 0, 0);

  if(!cb_cnt) {
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None ( very fast )"));
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Low ( fast )"));
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal ( slow )"));
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Best ( very slow )"));
  }
  SendMessageW(hCb, CB_SETCURSEL, 2, 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onResize()
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
  this->_setItemPos(IDC_EC_INPT4, 110, 125, this->width()-172, 14); // hidden
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
  this->_setItemPos(IDC_EC_INPT6, 10, this->height()-105, this->width()-72, 14);
  // Destination brows button
  this->_setItemPos(IDC_BC_SAVE, this->width()-60, this->height()-105, 50, 14);

  // Parsed name label & entry
  this->_setItemPos(IDC_SC_LBL07, 10, this->height()-84, 50, 9);
  this->_setItemPos(IDC_EC_INPT7, 60, this->height()-85, this->width()-175, 12);

  // Parsed version label & entry
  this->_setItemPos(IDC_SC_LBL08, this->width()-101, this->height()-84, 51, 9);
  this->_setItemPos(IDC_EC_INPT8, this->width()-45, this->height()-85, 35, 12);

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
void OmUiToolPkg::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->curContext();
    OmLocation* pLoc = pCtx ? pCtx->curLocation() : nullptr;

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
        item_str = pLoc ? pLoc->libraryDir() : L"";
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
        item_str = pLoc ? pLoc->libraryDir() : L"";
      } else {
        item_str = Om_getDirPart(item_str);
      }

      if(Om_dialogOpenFile(brow_str, this->_hwnd, L"Select Package file", OMM_PKG_FILES_FILTER, item_str)) {
        if(this->_parsePkg(brow_str)) { //< try to parse package
          this->setItemText(IDC_EC_INPT2, brow_str);
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
        this->msgItem(IDC_LB_DPNDS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item_str.c_str()));
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
        this->msgItem(IDC_SB_PKIMG, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBlank));
        this->setItemText(IDC_EC_INPT4, L"");
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
        if(this->_image.open(brow_str, OMM_PKG_THMB_SIZE)) {
          this->msgItem(IDC_SB_PKIMG, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_image.thumbnail()));
          this->setItemText(IDC_EC_INPT4, brow_str);
        } else {
          std::wcout << L"Image error : " << this->_image.lastErrorStr() << L"\n";
          this->msgItem(IDC_SB_PKIMG, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBlank));
          this->setItemText(IDC_EC_INPT4, L"");
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
      if(pLoc) {
        item_str = pLoc ? pLoc->libraryDir() : L"";
      } else {
        item_str = Om_getDirPart(item_str);
      }

      if(Om_dialogSaveFile(brow_str, this->_hwnd, L"Save Package as...", OMM_PKG_FILES_FILTER, item_str)) {
        this->setItemText(IDC_EC_INPT6, brow_str);
      }
      break;

    case IDC_EC_INPT6:
      this->getItemText(IDC_EC_INPT6, item_str);
      if(!item_str.empty()) {
        wstring name, vers;
        Om_parsePkgIdent(name, vers, item_str, true, true);
        this->setItemText(IDC_EC_INPT7, name);
        this->setItemText(IDC_EC_INPT8, vers);
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
      if(!item_str.empty()) {

        this->getItemText(IDC_EC_INPT6, item_str);
        if(item_str.empty()) allow = false;

      } else {
        allow = false;
      }

      this->enableItem(IDC_BC_OK, allow);
    }
  }

  return false;
}
