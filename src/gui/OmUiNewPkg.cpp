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
  _hPictureBlank(nullptr),
  _buildPkg_save(),
  _buildPkg_source(),
  _buildPkg_picture(nullptr),
  _buildPkg_depends(),
  _buildPkg_zipLvl(0),
  _buildPkg_desc(),
  _buildPkg_hth(nullptr)
{
  this->addChild(new OmUiProgress(hins)); //< for package creation process

  // load the package blank picture
  this->_hPictureBlank = reinterpret_cast<HBITMAP>(LoadImage(this->_hins,MAKEINTRESOURCE(IDB_PKG_BLANK),IMAGE_BITMAP,OMM_PKG_THMB_SIZE,OMM_PKG_THMB_SIZE,0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewPkg::~OmUiNewPkg()
{

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
void OmUiNewPkg::_onShow()
{
  // Enable Create From folder
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD01), BM_SETCHECK, 1, 0);

  // add items into Combo-Box
  HWND hCb = GetDlgItem(this->_hwnd, IDC_CB_LEVEL);

  unsigned cb_count = SendMessageW(hCb, CB_GETCOUNT, 0, 0);

  if(!cb_count) {
    SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)"None ( very fast )");
    SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)"Low ( fast )");
    SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)"Normal ( slow )");
    SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)"Best ( very slow )");
  }
  SendMessageW(hCb, CB_SETCURSEL, 2, 0);

  // set default snapshot
  SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)this->_hPictureBlank);

  // set font for readme
  HFONT hFont = CreateFont(14,0,0,0,400,false,false,false,1,0,0,5,0,"Consolas");
  SendMessage(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), WM_SETFONT, (WPARAM)hFont, 1);

  // Set Dependencies Add and Rem buttons "icon"
  SetWindowTextW(GetDlgItem(this->_hwnd, IDC_BC_ADD), L"\u2795"); // Heavy Plus Sign +
  //SetWindowTextW(GetDlgItem(this->_hwnd, IDC_BC_DEL), L"\u274C"); // Cross Mark
  SetWindowTextW(GetDlgItem(this->_hwnd, IDC_BC_DEL), L"\u2796"); // Heavy Minus Sign -
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_onResize()
{
  // From folder RadioButton
  this->_setControlPos(IDC_BC_RAD01, 10, 10, 150, 9);
  // From Folder EditControl & Brows Button
  this->_setControlPos(IDC_EC_INPT1, 10, 20, this->width()-38, 13);
  this->_setControlPos(IDC_BC_BROW1, this->width()-26, 20, 16, 13);
  // From existing Package RadioButton
  this->_setControlPos(IDC_BC_RAD02, 10, 40, 150, 9);
  // From existing Package EditControl & Brows Button
  this->_setControlPos(IDC_EC_INPT2, 10, 50, this->width()-38, 13);
  this->_setControlPos(IDC_BC_BROW2, this->width()-26, 50, 16, 13);

  // Dependencies CheckBox
  this->_setControlPos(IDC_BC_CHK03, 10, 75, 112, 9);
  // Dependencies EditControl a Add Button
  this->_setControlPos(IDC_EC_INPT3, 10, 90, this->width()-38, 13);
  this->_setControlPos(IDC_BC_ADD, this->width()-26, 90, 16, 13);
  // Dependencies ListBox & Del button
  this->_setControlPos(IDC_LB_DPNDS, 10, 105, this->width()-38, 24);
  this->_setControlPos(IDC_BC_DEL, this->width()-26, 105, 16, 13);

  // Picture CheckBox & Load button
  this->_setControlPos(IDC_BC_CHK04, 10, 140, 120, 9);
  this->_setControlPos(IDC_BC_BROW4, this->width()-60, 140, 50, 14);
  // Picture Bitmap & Label
  this->_setControlPos(IDC_SB_PKIMG, 10, 155, 85, 78);
  this->_setControlPos(IDC_SC_LBL04, 115, 185, 200, 9);

  // Description CheckBox & Load button
  this->_setControlPos(IDC_BC_CHK05, 10, 240, 120, 9);
  this->_setControlPos(IDC_BC_BROW5, this->width()-60, 240, 50, 14);
  // Description EditControl
  this->_setControlPos(IDC_EC_PKTXT, 10, 255, this->width()-20, this->height()-325);

  // Zip Level Label
  this->_setControlPos(IDC_SC_LBL06, 10, this->height()-60, 120, 9);
  // Zip Level ListBox
  this->_setControlPos(IDC_CB_LEVEL, 10, this->height()-50, this->width()-20, 14);

  // ----- Separator
  this->_setControlPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Save As... Button
  this->_setControlPos(IDC_BC_SAVE, this->width()-108, this->height()-19, 50, 14);
  // Close Button
  this->_setControlPos(IDC_BC_CLOSE, this->width()-54, this->height()-19, 50, 14);
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
    // refresh the main window dialog, this will also refresh this one
    this->root()->refresh();
  }

  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
    OmLocation* location = nullptr;

    if(manager->curContext())
      location = manager->curContext()->curLocation();

    int lb_sel;
    wchar_t dpn_buf[OMM_MAX_PATH];
    wchar_t sel_dir[OMM_MAX_PATH];
    wchar_t pkg_src[OMM_MAX_PATH];
    wchar_t img_src[OMM_MAX_PATH];
    wchar_t txt_src[OMM_MAX_PATH];

    switch(LOWORD(wParam))
    {
    case IDC_BC_RAD01:
    case IDC_BC_RAD02:
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD01), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT1), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW1), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT2), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW2), false);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT1), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW1), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT2), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW2), true);
      }
      break;

    case IDC_BC_BROW1:
      // select the initial location for browsing start
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, pkg_src, OMM_MAX_PATH);
      if(!wcslen(pkg_src)) {
        if(location) wcscpy(sel_dir, location->libraryDir().c_str());
      } else {
        wcscpy(sel_dir, Om_getDirPart(pkg_src).c_str());
      }

      if(Om_dialogBrowseDir(pkg_src, this->_hwnd, L"Select installation file(s) location", sel_dir)) {
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, pkg_src);
      }
      break;

    case IDC_BC_BROW2:
      // select the initial location for browsing start
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, pkg_src, OMM_MAX_PATH);
      if(!wcslen(pkg_src)) {
        if(location) wcscpy(sel_dir, location->libraryDir().c_str());
      } else {
        wcscpy(sel_dir, Om_getDirPart(pkg_src).c_str());
      }

      if(Om_dialogOpenFile(pkg_src, this->_hwnd, L"Select Package file", OMM_PKG_FILES_FILTER, sel_dir)) {
        OmPackage pkg;
        if(pkg.sourceParse(pkg_src)) {
          SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_RESETCONTENT, 0, 0);
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, pkg_src);
          if(pkg.dependCount()) {
            SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK03), BM_SETCHECK, 1, 0);
            EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), true);
            EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), true);
            for(unsigned i = 0; i < pkg.dependCount(); ++i) {
              SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_ADDSTRING, 0, (LPARAM)pkg.depend(i).c_str());
            }
          }
          if(pkg.picture()) {
            SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK04), BM_SETCHECK, 1, 0);
            EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), true);
            this->_buildPkg_picture = pkg.picture();
            HBITMAP hBmp = static_cast<HBITMAP>(Om_getBitmapThumbnail(this->_buildPkg_picture,OMM_PKG_THMB_SIZE,OMM_PKG_THMB_SIZE));
            SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
            DeleteObject(hBmp);
          }
          if(pkg.desc().size()) {
            SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK05), BM_SETCHECK, 1, 0);
            EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW5), true);
            EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), true);
            SetDlgItemTextA(this->_hwnd, IDC_EC_PKTXT, Om_toMbString(pkg.desc()).c_str());
          }
        } else {
          wstring err = L"The file \""; err += pkg_src;
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
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK03), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), true);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), false);
      }
    break;

    case IDC_EC_INPT3: // Dependencies EditControl
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, dpn_buf, OMM_MAX_PATH);
      if(wcslen(dpn_buf)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ADD), true);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ADD), false);
      }
      break;

    case IDC_BC_ADD: // Add Dependency Button
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, dpn_buf, OMM_MAX_PATH);
      if(wcslen(dpn_buf)) {
        SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_ADDSTRING, 0, (LPARAM)dpn_buf);
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, L"");
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ADD), false);
      }
      break;

    case IDC_LB_DPNDS: // Dependencies ListBox
      lb_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_GETCURSEL, 0, 0);
      if(lb_sel >= 0) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), true);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);
      }
      break;

    case IDC_BC_DEL: // Remove Dependency Button
      lb_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_GETCURSEL, 0, 0);
      if(lb_sel >= 0) {
        SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_DELETESTRING, lb_sel, 0);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);
      }
      break;

    case IDC_BC_CHK04:
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK04), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), true);
        if(wcslen(img_src)) {
          if(Om_isFile(img_src)) {
            this->_buildPkg_picture = static_cast<HBITMAP>(Om_loadBitmap(img_src));
            HBITMAP hBmp = static_cast<HBITMAP>(Om_getBitmapThumbnail(this->_buildPkg_picture,OMM_PKG_THMB_SIZE,OMM_PKG_THMB_SIZE));
            SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
            DeleteObject(hBmp);
          }
        }
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), false);
        this->_buildPkg_picture = nullptr;
        SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)this->_hPictureBlank);
      }
      break;

    case IDC_BC_BROW4:
      if(Om_dialogOpenFile(img_src, this->_hwnd, L"Open Image file", IMAGE_FILE_FILTER, img_src)) {
        if(wcslen(img_src)) {
          if(Om_isFile(img_src)) {
            this->_buildPkg_picture = static_cast<HBITMAP>(Om_loadBitmap(img_src));
            HBITMAP hBmp = static_cast<HBITMAP>(Om_getBitmapThumbnail(this->_buildPkg_picture,OMM_PKG_THMB_SIZE,OMM_PKG_THMB_SIZE));
            SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
            DeleteObject(hBmp);
          }
        }
      }
      break;

    case IDC_BC_CHK05:
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK05), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW5), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), true);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW5), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), false);
      }
    break;

    case IDC_BC_BROW5:
      if(Om_dialogOpenFile(txt_src, this->_hwnd, L"Open Text file", L"Text file (*.txt)\0*.TXT\0", txt_src)) {
        if(wcslen(txt_src)) {
          if(Om_isFile(txt_src)) {
            string ascii = Om_loadPlainText(txt_src);
            SetDlgItemTextA(this->_hwnd, IDC_EC_PKTXT, ascii.c_str());
          }
        }
      }
      break;

    case IDC_BC_SAVE:
      this->_apply();
      break;

    case IDC_BC_CLOSE:
      this->quit();
      break;
    }

    if(has_changed) {
      bool allow = true;
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD01), BM_GETCHECK, 0, 0)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, pkg_src, OMM_MAX_PATH);
      }
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD02), BM_GETCHECK, 0, 0)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, pkg_src, OMM_MAX_PATH);
      }
      if(!wcslen(pkg_src)) allow = false;

      EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_OK), allow);
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewPkg::_apply()
{
  wchar_t sel_dir[OMM_MAX_PATH];
  wchar_t pkg_src[OMM_MAX_PATH];
  wchar_t pkg_dst[OMM_MAX_PATH];

  bool rad01 = SendMessage(GetDlgItem(this->_hwnd,IDC_BC_RAD01),BM_GETCHECK,0,0);

  if(rad01) {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, pkg_src, OMM_MAX_PATH);
    if(!Om_isDir(pkg_src)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid source path",
                                    L"The specified source path is not a "
                                    L"valid folder.");
      return false;
    }
  } else {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, pkg_src, OMM_MAX_PATH);
    if(!Om_isFileZip(pkg_src)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid source file",
                                    L"The specified source Package is not a "
                                    L"valid zip file.");
      return false;
    }
  }

  // create the file initial name based on source folder name
  if(rad01) {
    swprintf(pkg_dst, OMM_MAX_PATH, L"%ls.zip", Om_getFilePart(pkg_src).c_str());
  } else {
    wcscpy(pkg_dst, Om_getFilePart(pkg_src).c_str());
  }

  // select the initial location for browsing start
  wcscpy(sel_dir, Om_getDirPart(pkg_src).c_str());
  if(Om_dialogSaveFile(pkg_dst, this->_hwnd, L"Save Packages as...", OMM_PKG_FILES_FILTER, sel_dir)) {
    if(Om_isValidName(pkg_dst)) {
      if(Om_isFile(pkg_dst)) {
        if(Om_dialogBoxQuerry(this->_hwnd, L"File already exists", L"Overwrite the existing file ?")) {
          this->_buildPkg_init(pkg_dst);
        }
      } else {
        this->_buildPkg_init(pkg_dst);
      }
    } else {
      Om_dialogBoxErr(this->_hwnd, L"Invalid file name", L"The specified file name is not valid.");
    }
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_buildPkg_init(const wstring& path)
{
  wchar_t wcbuf[OMM_MAX_PATH];

  this->_buildPkg_zipLvl = 2;
  this->_buildPkg_source.clear();
  this->_buildPkg_depends.clear();
  this->_buildPkg_desc.clear();

  if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD01), BM_GETCHECK, 0, 0)) {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
  } else {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
  }
  this->_buildPkg_source = wcbuf;

  // build the dependencies list
  if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01), BM_GETCHECK, 0, 0)) {
    wchar_t ident[OMM_MAX_PATH];
    unsigned lb_count = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_GETCOUNT, 0, 0);
    if(lb_count) {
      for(unsigned i = 0; i < lb_count; ++i) {
        SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_GETTEXT, i, (LPARAM)ident);
        this->_buildPkg_depends.push_back(ident);
      }
    }
  }

  this->_buildPkg_zipLvl = SendMessageW(GetDlgItem(this->_hwnd, IDC_CB_LEVEL), CB_GETCURSEL, 0, 0);

  // get description text
  if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK03), BM_GETCHECK, 0, 0)) {
    size_t s = SendMessageA(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), WM_GETTEXTLENGTH, 0, 0) + 1;
    char* buff = nullptr;
    try {
      buff = new char[s];
    } catch (std::bad_alloc& ba) {
      // TODO: Add some dialog error here... one day
      this->quit();
    }
    SendMessageA(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), WM_GETTEXT, s, (LPARAM)buff);
    this->_buildPkg_desc = buff;
    delete [] buff;
  }

  OmUiProgress* uiProgress = reinterpret_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));

  uiProgress->open(true);
  uiProgress->setCaption(L"Building Package");
  uiProgress->setTitle(L"Adding file(s) to Package...");

  this->_buildPkg_save = path;

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

  OmUiProgress* uiProgress = reinterpret_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));

  uiProgress->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiNewPkg::_buildPkg_fth(void* arg)
{
  OmUiNewPkg* self = reinterpret_cast<OmUiNewPkg*>(arg);

  OmUiProgress* uiProgress = reinterpret_cast<OmUiProgress*>(self->childById(IDD_PROGRESS));

  HWND hPb = (HWND)uiProgress->getProgressBar();
  HWND hSc = (HWND)uiProgress->getStaticComment();

  OmPackage package(nullptr);
  package.sourceParse(self->_buildPkg_source);

  if(self->_buildPkg_depends.size()) {
    for(unsigned i = 0; i < self->_buildPkg_depends.size(); ++i) {
      package.addDepend(self->_buildPkg_depends[i]);
    }
  }

  if(!self->_buildPkg_desc.empty()) {
    package.setDesc(Om_toWcString(self->_buildPkg_desc));
  }

  if(!self->_buildPkg_picture) {
    package.setPicture(self->_buildPkg_picture);
    DeleteObject(self->_buildPkg_picture);
  }

  if(!package.save(self->_buildPkg_save, self->_buildPkg_zipLvl, hPb, hSc, uiProgress->getAbortPtr())) {

    wstring str = L"An error occurred during Package creation:\n";
    str += package.lastError();

    Om_dialogBoxErr(uiProgress->hwnd(), L"Package creation error", str);
  }

  PostMessage(self->_hwnd, UWM_BUILDPKG_DONE, 0, 0);

  return 0;
}
