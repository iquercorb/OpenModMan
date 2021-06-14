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
  _buildPkg_hth(nullptr),
  _abort(false)
{

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
void OmUiToolPkg::_nameParse()
{
  wchar_t ext_str[16];
  wstring name_str, vers_str;
  OmVersion vers;

  // get name
  this->getItemText(IDC_EC_INP03, name_str);

  if(name_str.size()) {

    // Replace all spaces by underscores
    std::replace(name_str.begin(), name_str.end(), L' ', L'_');

    // get version
    this->getItemText(IDC_EC_INP04, vers_str);

    // check if version string is valid
    if(vers.parse(vers_str))
      name_str += L"v" + vers.asString();

    // get chosen file extension
    int cb_sel = this->msgItem(IDC_CB_EXTEN, CB_GETCURSEL, 0, 0);
    this->msgItem(IDC_CB_EXTEN, CB_GETLBTEXT, cb_sel, reinterpret_cast<LPARAM>(ext_str));
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
bool OmUiToolPkg::_snapLoad(const wstring& path)
{
  OmImage image;
  HBITMAP hBm;

  // Open image
  if(image.open(path, OMM_PKG_THMB_SIZE)) {

    // set thumbnail
    hBm = this->setStImage(IDC_SB_PKIMG, image.thumbnail());
    if(hBm && hBm != this->_hBmBlank) DeleteObject(hBm);

    return true;

  } else {

    // remove any thumbnail
    hBm = this->setStImage(IDC_SB_PKIMG, this->_hBmBlank);
    if(hBm && hBm != this->_hBmBlank) DeleteObject(hBm);
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_parsePkg(const wstring& path)
{
  OmPackage pkg;

  HBITMAP hBm;

  if(pkg.sourceParse(path)) {

    this->msgItem(IDC_LB_DPNLS, LB_RESETCONTENT);

    if(pkg.dependCount()) {
      this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 1);
      this->enableItem(IDC_LB_DPNLS, true);
      this->enableItem(IDC_EC_INP07, true);
      for(unsigned i = 0; i < pkg.dependCount(); ++i) {
        this->msgItem(IDC_LB_DPNLS, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(pkg.depend(i).c_str()));
      }
    } else {
      this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 0);
      this->enableItem(IDC_LB_DPNLS, false);
      this->setItemText(IDC_EC_INP07, L"");
      this->enableItem(IDC_EC_INP07, false);
      this->enableItem(IDC_BC_ADD, false);
      this->enableItem(IDC_BC_DEL, false);
    }

    if(pkg.image().thumbnail()) {
      this->msgItem(IDC_BC_CHK02, BM_SETCHECK, 1);
      this->enableItem(IDC_BC_BRW04, true);
      hBm = this->setStImage(IDC_SB_PKIMG, pkg.image().thumbnail());
      if(hBm && hBm != this->_hBmBlank) DeleteObject(hBm);
      this->setItemText(IDC_EC_INP08, L"");
    } else {
      this->msgItem(IDC_BC_CHK02, BM_SETCHECK, 0);
      this->enableItem(IDC_BC_BRW04, false);
      hBm = this->setStImage(IDC_SB_PKIMG, this->_hBmBlank);
      if(hBm && hBm != this->_hBmBlank) DeleteObject(hBm);
    }

    if(pkg.desc().size()) {
      this->msgItem(IDC_BC_CHK03, BM_SETCHECK, 1);
      this->enableItem(IDC_BC_BRW05, true);
      this->enableItem(IDC_EC_PKTXT, true);
      this->setItemText(IDC_EC_PKTXT, pkg.desc());
    } else {
      this->msgItem(IDC_BC_CHK03, BM_SETCHECK, 0);
      this->enableItem(IDC_BC_BRW05, false);
      this->enableItem(IDC_EC_PKTXT, false);
      this->setItemText(IDC_EC_PKTXT, L"");
    }

    // update name and version
    this->setItemText(IDC_EC_INP03, pkg.name());
    if(!pkg.version().isNull())
      this->setItemText(IDC_EC_INP04, pkg.version().asString());

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
void OmUiToolPkg::_resetPkg()
{
  this->setItemText(IDC_EC_INP04, L"");
  this->setItemText(IDC_EC_INP03, L"");

  this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 0);
  this->msgItem(IDC_LB_DPNLS, LB_RESETCONTENT);
  this->enableItem(IDC_LB_DPNLS, false);
  this->enableItem(IDC_EC_INP07, false);
  this->setItemText(IDC_EC_INP07, L"");
  this->enableItem(IDC_BC_ADD, false);
  this->enableItem(IDC_BC_DEL, false);

  this->msgItem(IDC_BC_CHK02, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_BRW04, false);
  HBITMAP hBm = this->setStImage(IDC_SB_PKIMG, this->_hBmBlank);
  if(hBm && hBm != this->_hBmBlank) DeleteObject(hBm);

  this->msgItem(IDC_BC_CHK03, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_BRW05, false);
  this->enableItem(IDC_EC_PKTXT, false);
  this->setItemText(IDC_EC_PKTXT, L"");
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_buildPkg_init()
{
  // disable all controls during creation
  this->enableItem(IDC_BC_RAD01, false);
  this->enableItem(IDC_BC_RAD02, false);
  this->enableItem(IDC_EC_INP03, false);
  this->enableItem(IDC_EC_INP04, false);
  this->enableItem(IDC_CB_EXTEN, false);
  this->enableItem(IDC_CB_LEVEL, false);
  this->enableItem(IDC_EC_INP05, false);
  this->enableItem(IDC_BC_BRW03, false);
  this->enableItem(IDC_BC_CHK01, false);
  this->enableItem(IDC_BC_CHK02, false);
  this->enableItem(IDC_BC_CHK03, false);
  // disable according radios and check-boxes status
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->enableItem(IDC_EC_INP01, false);
    this->enableItem(IDC_BC_BRW01, false);
  } else {
    this->enableItem(IDC_EC_INP02, false);
    this->enableItem(IDC_BC_BRW02, false);
  }
  if(this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
    this->enableItem(IDC_EC_INP07, false);
    this->enableItem(IDC_BC_ADD, false);
    this->enableItem(IDC_BC_DEL, false);
    this->enableItem(IDC_LB_DPNLS, false);
  }
  if(this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
    this->enableItem(IDC_BC_BRW04, false);
  }
  if(this->msgItem(IDC_BC_CHK03, BM_GETCHECK)) {
    this->enableItem(IDC_BC_BRW05, false);
    this->enableItem(IDC_EC_PKTXT, false);
  }
  this->enableItem(IDC_BC_SAVE, false);
  this->enableItem(IDC_BC_CLOSE, false);

  // enable the progress bar
  this->enableItem(IDC_PB_PGBAR, true);

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

  // enable all controls again
  this->enableItem(IDC_BC_RAD01, true);
  this->enableItem(IDC_BC_RAD02, true);
  this->enableItem(IDC_EC_INP03, true);
  this->enableItem(IDC_EC_INP04, true);
  this->enableItem(IDC_CB_EXTEN, true);
  this->enableItem(IDC_CB_LEVEL, true);
  this->enableItem(IDC_EC_INP05, true);
  this->enableItem(IDC_BC_BRW03, true);
  this->enableItem(IDC_BC_CHK01, true);
  this->enableItem(IDC_BC_CHK02, true);
  this->enableItem(IDC_BC_CHK03, true);
  // enable according radios and check-boxes status
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->enableItem(IDC_EC_INP01, true);
    this->enableItem(IDC_BC_BRW01, true);
  } else {
    this->enableItem(IDC_EC_INP02, true);
    this->enableItem(IDC_BC_BRW02, true);
  }
  if(this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
    this->enableItem(IDC_EC_INP07, true);
    this->enableItem(IDC_BC_ADD, true);
    this->enableItem(IDC_BC_DEL, true);
    this->enableItem(IDC_LB_DPNLS, true);
  }
  if(this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
    this->enableItem(IDC_BC_BRW04, true);
  }
  if(this->msgItem(IDC_BC_CHK03, BM_GETCHECK)) {
    this->enableItem(IDC_BC_BRW05, true);
    this->enableItem(IDC_EC_PKTXT, true);
  }
  this->enableItem(IDC_BC_SAVE, true);
  this->enableItem(IDC_BC_CLOSE, true);

  // reset & disable the progress bar
  this->msgItem(IDC_PB_PGBAR, PBM_SETPOS, 0, 0);
  this->enableItem(IDC_PB_PGBAR, false);

  // show a reassuring dialog message
  if(exitCode == 0) {

    wstring item_str;

    // get destination filename
    this->getItemText(IDC_EC_INP06, item_str);

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

  wstring item_str, out_path, out_file;

  OmPackage package(nullptr);

  // get source (either a folder or another package) and parse it
  if(self->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    self->getItemText(IDC_EC_INP01, item_str);
  } else {
    self->getItemText(IDC_EC_INP02, item_str);
  }

  if(!package.sourceParse(item_str)) {
    // show error dialog box
    wstring err = L"Source data parsing failed.\n\n";
    err += package.lastError();
    Om_dialogBoxErr(self->_hwnd, L"Package creation error", err);
    return 1;
  }

  // get package dependencies list
  if(self->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
    int lb_cnt = self->msgItem(IDC_LB_DPNLS, LB_GETCOUNT);
    if(lb_cnt) {
      size_t n;
      for(int i = 0; i < lb_cnt; ++i) {
        item_str.resize(self->msgItem(IDC_LB_DPNLS, LB_GETTEXTLEN, i));
        n = self->msgItem(IDC_LB_DPNLS, LB_GETTEXT, i, reinterpret_cast<LPARAM>(&item_str[0]));
        if(n < item_str.size()) item_str.resize(n);
        package.addDepend(item_str);
      }
    }
  }

  // get package image data from specified file if any
  if(self->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
    self->getItemText(IDC_EC_INP08, item_str);
    if(!item_str.empty()) {
      package.loadImage(item_str, OMM_PKG_THMB_SIZE);
    }
  }

  // get package description text
  if(self->msgItem(IDC_BC_CHK03, BM_GETCHECK)) {
    self->getItemText(IDC_EC_PKTXT, item_str);
    package.setDesc(item_str);
  }

  // get package compression level
  LRESULT zip_lvl = self->msgItem(IDC_CB_LEVEL, CB_GETCURSEL);

  // get destination path & filename
  self->getItemText(IDC_EC_INP06, out_path);
  self->getItemText(IDC_EC_OUT01, out_file);

  self->_abort = false;
  self->enableItem(IDC_BC_ABORT, true);

  HWND hPb = self->getItem(IDC_PB_PGBAR);

  DWORD exitCode = 0;

  if(!package.save(out_path + L"\\" + out_file, zip_lvl, hPb, nullptr, &self->_abort)) {
    // show error dialog box
    wstring err = L"An error occurred during Package creation:\n";
    err += package.lastError();
    Om_dialogBoxErr(self->_hwnd, L"Package creation error", err);

    exitCode = 1;
  }

  self->enableItem(IDC_BC_ABORT, false);

  PostMessage(self->_hwnd, UWM_BUILDPKG_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_save()
{
  wstring item_str;

  // Step 1, verify everything
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP01, item_str);
    if(!Om_isDir(item_str)) {
      wstring wrn = L"The folder \""+item_str+L"\"";
      wrn += OMM_STR_ERR_ISDIR;
      Om_dialogBoxWarn(this->_hwnd, L"Invalid source path", wrn);
      return false;
    }
  } else {
    this->getItemText(IDC_EC_INP02, item_str);
    if(!Om_isFileZip(item_str)) {
      wstring wrn = L"The file \""+item_str+L"\"";
      wrn += OMM_STR_ERR_ISFILEZIP;
      Om_dialogBoxWarn(this->_hwnd, L"Invalid source file", wrn);
      return false;
    }
  }

  wstring name_str, path_str;

  this->getItemText(IDC_EC_INP06, path_str);
  this->getItemText(IDC_EC_OUT01, name_str);

  if(Om_isDir(path_str)) {
    if(Om_isValidName(name_str)) {
      item_str = path_str + L"\\" + name_str;
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
    wstring err = L"The destination folder \""+path_str+L"\"";
    err += OMM_STR_ERR_ISDIR;
    Om_dialogBoxErr(this->_hwnd, L"Invalid destination", err);
    return false;
  }

  // disable the Save button
  this->enableItem(IDC_BC_SAVE, false);

  this->_buildPkg_init();

  return true;
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
  this->_createTooltip(IDC_CB_EXTEN, L"Filename extension");

  this->_createTooltip(IDC_CB_LEVEL, L"Package ZIP compression level");

  this->_createTooltip(IDC_EC_INP06, L"Save destination path");
  this->_createTooltip(IDC_BC_BRW03, L"Select destination folder");

  this->_createTooltip(IDC_BC_SAVE, L"Save Package");
  this->_createTooltip(IDC_BC_ABORT, L"Abort process");

  this->_createTooltip(IDC_BC_CHK01, L"Defines dependencies for this Package");
  this->_createTooltip(IDC_EC_INP07, L"Dependency package identity");
  this->_createTooltip(IDC_LB_DPNLS, L"Dependencies list");

  this->_createTooltip(IDC_BC_CHK02, L"Defines a snapshot for this Package");
  this->_createTooltip(IDC_BC_BRW04, L"Select image file");

  this->_createTooltip(IDC_BC_CHK03, L"Defines a description for this Package");
  this->_createTooltip(IDC_BC_BRW05, L"Select text file");
  this->_createTooltip(IDC_EC_PKTXT, L"Package description text");

  // Set font for description
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtMonos), true);
  // Set default package picture
  this->setStImage(IDC_SB_PKIMG, this->_hBmBlank);
  // Set buttons inner icons
  this->setBmImage(IDC_BC_ADD, this->_hBmBcNew);
  this->setBmImage(IDC_BC_DEL, this->_hBmBcDel);

  // Enable Create From folder
  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, 1);

  // add items into File Extension Combo-Box
  HWND hCb = this->getItem(IDC_CB_EXTEN);
  unsigned cb_cnt = SendMessageW(hCb, CB_GETCOUNT, 0, 0);
  if(!cb_cnt) {
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L".zip"));
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L".ozp"));
  }
  SendMessageW(hCb, CB_SETCURSEL, 0, 0);

  // add items into Zip Level Combo-Box
  hCb = this->getItem(IDC_CB_LEVEL);
  cb_cnt = SendMessageW(hCb, CB_GETCOUNT, 0, 0);
  if(!cb_cnt) {
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None ( very fast )"));
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Low ( fast )"));
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal ( slow )"));
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Best ( very slow )"));
  }
  SendMessageW(hCb, CB_SETCURSEL, 2, 0);

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
  this->_setItemPos(IDC_EC_INP01, 10, 25, half_w-40, 13);
  this->_setItemPos(IDC_BC_BRW01, half_w-25, 25, 15, 13);

  // From existing Package RadioButton
  this->_setItemPos(IDC_BC_RAD02, 10, 50, 150, 9);
  // From existing Package EditControl & Browse Button
  this->_setItemPos(IDC_EC_INP02, 10, 65, half_w-40, 13);
  this->_setItemPos(IDC_BC_BRW02, half_w-25, 65, 15, 13);

  // [ - - - File name & Zip compression GroupBox - - -
  this->_setItemPos(IDC_GB_GRP01, 5, 100, half_w-10, 110);
  // Package filename Label
  this->_setItemPos(IDC_SC_LBL01, 10, 110, 120, 9);
  // Name Label & EditText
  this->_setItemPos(IDC_SC_LBL02, 10, 127, 23, 9);
  this->_setItemPos(IDC_EC_INP03, 35, 125, half_w-145, 13);
  // Version label & EditText
  this->_setItemPos(IDC_SC_LBL03, half_w-105, 127, 27, 9);
  this->_setItemPos(IDC_EC_INP04, half_w-75, 125, 30, 13);
  // File extension ComboBox
  this->_setItemPos(IDC_CB_EXTEN, half_w-40, 125, 30, 13);
  // Output filname EditText
  this->_setItemPos(IDC_EC_OUT01, 10, 145, half_w-20, 13);
  // Zip compression Label & ComboBox
  this->_setItemPos(IDC_SC_LBL04, 10, 175, 120, 9);
  this->_setItemPos(IDC_CB_LEVEL, 10, 190, half_w-20, 13);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // Destination Label
  this->_setItemPos(IDC_SC_LBL05, 10, this->height()-85, 120, 9);
  // Destination EditText & Browse Button
  this->_setItemPos(IDC_EC_INP06, 10, this->height()-70, half_w-40, 13);
  this->_setItemPos(IDC_BC_BRW03, half_w-25, this->height()-70, 15, 13);

  // Save Button
  this->_setItemPos(IDC_BC_SAVE, 10, this->height()-45, 45, 14);
  // Progress Bar
  this->_setItemPos(IDC_PB_PGBAR, 57, this->height()-44, half_w-114, 12);
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
  this->_setItemPos(IDC_LB_DPNLS, half_w+10, 45, half_w-40, 30);
  this->_setItemPos(IDC_BC_DEL, this->width()-25, 45, 16, 13);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Snapshot GroupBox             - - -
  this->_setItemPos(IDC_GB_GRP03, half_w+5, 85, half_w-10, 95);
  // Include snapshot CheckBox
  this->_setItemPos(IDC_BC_CHK02, half_w+10, 95, 65, 9);
  // Snapshot Bitmap & Select... Button
  this->_setItemPos(IDC_SB_PKIMG, this->width()-160, 96, 85, 78);
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

    HBITMAP hBm;

    switch(LOWORD(wParam))
    {
    case IDC_BC_RAD01:
    case IDC_BC_RAD02:
      bm_chk = this->msgItem(IDC_BC_RAD01, BM_GETCHECK);
      this->enableItem(IDC_EC_INP01, bm_chk);
      this->enableItem(IDC_BC_BRW01, bm_chk);
      this->enableItem(IDC_EC_INP02, !bm_chk);
      this->enableItem(IDC_BC_BRW02, !bm_chk);
      if(bm_chk) {
        this->getItemText(IDC_EC_INP01, item_str);
      } else {
        this->getItemText(IDC_EC_INP02, item_str);
      }
      if(!item_str.empty()) {
        this->_parsePkg(item_str);
      } else {
        this->_resetPkg();
      }
      has_changed = true;
      break;

    case IDC_BC_BRW01:
      // select the initial location for browsing start
      this->getItemText(IDC_EC_INP01, item_str);
      if(item_str.empty()) {
        item_str = pLoc ? pLoc->libraryDir() : L"";
      } else {
        item_str = Om_getDirPart(item_str);
      }
      if(Om_dialogBrowseDir(brow_str, this->_hwnd, L"Select installation file(s) location", item_str)) {
        if(this->_parsePkg(brow_str)) { //< try to parse folder as package
          this->setItemText(IDC_EC_INP01, brow_str);
        }
      }
      break;

    case IDC_EC_INP01:
      has_changed = true;
      break;

    case IDC_BC_BRW02:
      // select the initial location for browsing start
      this->getItemText(IDC_EC_INP02, item_str);
      if(item_str.empty()) {
        item_str = pLoc ? pLoc->libraryDir() : L"";
      } else {
        item_str = Om_getDirPart(item_str);
      }

      if(Om_dialogOpenFile(brow_str, this->_hwnd, L"Select Package file", OMM_PKG_FILES_FILTER, item_str)) {
        if(this->_parsePkg(brow_str)) { //< try to parse package
          this->setItemText(IDC_EC_INP02, brow_str);
        }
      }
      break;

    case IDC_EC_INP02:
      has_changed = true;
      break;

    case IDC_CB_EXTEN:
    case IDC_EC_INP03:
    case IDC_EC_INP04:
      this->_nameParse();
      has_changed = true;
      break;

    case IDC_BC_BRW03:  // Destination folder Browse Button
      // create the file initial name based on source folder name
      if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INP01, item_str);
      } else {
        this->getItemText(IDC_EC_INP02, item_str);
      }

      item_str = Om_getDirPart(item_str);

      if(Om_dialogBrowseDir(brow_str, this->_hwnd, L"Select destination location", item_str)) {
        this->setItemText(IDC_EC_INP06, brow_str);
      }
      break;

    case IDC_EC_INP06: // Destination folder EditText entry
      this->getItemText(IDC_EC_INP06, item_str);
      has_changed = true;
      break;

    case IDC_BC_CHK01: // Has Dependencies CheckBox
      bm_chk = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);
      this->enableItem(IDC_EC_INP07, bm_chk);
      this->enableItem(IDC_LB_DPNLS, bm_chk);
    break;

    case IDC_EC_INP07: // Dependencies EditControl
      this->getItemText(IDC_EC_INP07, item_str);
      this->enableItem(IDC_BC_ADD, !item_str.empty());
      break;

    case IDC_LB_DPNLS: // Dependencies ListBox
      lb_sel = this->msgItem(IDC_LB_DPNLS, LB_GETCURSEL);
      this->enableItem(IDC_BC_DEL, (lb_sel >= 0));
      break;

    case IDC_BC_ADD: // Add Dependency Button
      this->getItemText(IDC_EC_INP07, item_str);
      if(!item_str.empty()) {
        this->msgItem(IDC_LB_DPNLS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item_str.c_str()));
        this->setItemText(IDC_EC_INP07, L"");
      }
      this->enableItem(IDC_BC_ADD, false);
      break;

    case IDC_BC_DEL: // Remove Dependency Button
      lb_sel = this->msgItem(IDC_LB_DPNLS, LB_GETCURSEL);
      if(lb_sel >= 0) {
        this->msgItem(IDC_LB_DPNLS, LB_DELETESTRING, lb_sel);
      }
      this->enableItem(IDC_BC_DEL, false);
      break;

    case IDC_BC_CHK02:  // Include snapshot CheckBox
      if(this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
        this->enableItem(IDC_BC_BRW04, true);
      } else {
        this->enableItem(IDC_BC_BRW04, false);
        hBm = this->setStImage(IDC_SB_PKIMG, this->_hBmBlank);
        if(hBm && hBm != this->_hBmBlank) DeleteObject(hBm);
        this->setItemText(IDC_EC_INP08, L"");
      }
      break;

    case IDC_BC_BRW04:
      // select the start directory from package source path
      if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INP01, item_str);
      } else {
        this->getItemText(IDC_EC_INP02, item_str);
      }
      item_str = Om_getDirPart(item_str);

      if(Om_dialogOpenFile(brow_str, this->_hwnd, L"Select image file", OMM_IMG_FILES_FILTER, item_str)) {
        if(this->_snapLoad(brow_str)) {
          this->setItemText(IDC_EC_INP08, brow_str);
        } else {
          this->setItemText(IDC_EC_INP08, L"");
        }
      }
      break;

    case IDC_BC_CHK03:
      bm_chk = this->msgItem(IDC_BC_CHK03, BM_GETCHECK);
      this->enableItem(IDC_BC_BRW05, bm_chk);
      this->enableItem(IDC_EC_PKTXT, bm_chk);
    break;

    case IDC_BC_BRW05:
      // select the start directory from package source path
      if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INP01, item_str);
      } else {
        this->getItemText(IDC_EC_INP02, item_str);
      }
      item_str = Om_getDirPart(item_str);

      if(Om_dialogOpenFile(brow_str, this->_hwnd, L"Select text file", OMM_TXT_FILES_FILTER, item_str)) {
        if(Om_isFile(brow_str)) {
          string text_str = Om_loadPlainText(brow_str);
          SetDlgItemTextA(this->_hwnd, IDC_EC_PKTXT, text_str.c_str());
        }
      }
      break;

    case IDC_BC_SAVE:
      this->_save();
      break;

    case IDC_BC_ABORT:
      this->_abort = true;
      break;

    case IDC_BC_CLOSE:
      this->quit();
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
