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
#include "OmPackage.h"
#include "gui/OmUiPropPkg.h"
#include "gui/OmUiPropPkgSrc.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPkgSrc::OmUiPropPkgSrc(HINSTANCE hins) : OmDialog(hins)
{
  //ctor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPkgSrc::~OmUiPropPkgSrc()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropPkgSrc::id() const
{
  return IDD_PROP_PKG_SRC;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgSrc::_onShow()
{
  OmPackage* package = reinterpret_cast<OmUiPropPkg*>(this->_parent)->package();

  if(package == nullptr)
    return;

  // defines fonts for package description, title, and log output
  HFONT hFont = CreateFont(14,0,0,0,400,false,false,false,1,0,0,5,0,"Consolas");
  SendMessage(GetDlgItem(_hwnd, IDC_EC_PKTXT), WM_SETFONT, (WPARAM)hFont, 1);

  wstring hash_str;

  // Ident
  SetDlgItemTextW(this->_hwnd, IDC_EC_ENT01, package->ident().c_str());
  // Name
  SetDlgItemTextW(this->_hwnd, IDC_EC_ENT02, package->name().c_str());
  // Parsed Version
  SetDlgItemTextW(this->_hwnd, IDC_EC_ENT03, package->version().asString().c_str());
  // Computed Hash
  hash_str = Om_toHexString(package->hash());
  SetDlgItemTextW(this->_hwnd, IDC_EC_ENT04, hash_str.c_str());

  if(package->isType(PKG_TYPE_SRC)) {
    // Type
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT06, package->isType(PKG_TYPE_ZIP)?L"Zip archive":L"Sub-folder");
    // Location
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT07, package->sourcePath().c_str());
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT04), true);

    // Dependencies
    if(package->dependCount()) {
      wstring dpn_str;
      for(unsigned i = 0; i < package->dependCount(); ++i) {
        dpn_str += package->depend(i);
        if(i < (package->dependCount() - 1)) {
          dpn_str += L"; ";
        }
      }
      SetDlgItemTextW(this->_hwnd, IDC_EC_ENT08, dpn_str.c_str());
    } else {
      SetDlgItemTextW(this->_hwnd, IDC_EC_ENT08, L"None");
    }
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT08), true);

    // Snapshot image
    if(package->picture()) {
      SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)package->picture());
    } else {
      HBITMAP hBmp = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_BLANK), IMAGE_BITMAP, 0, 0, 0);
      SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
      DeleteObject(hBmp);
    }

    // Package description
    if(package->desc().size()) {
      SetDlgItemTextW(this->_hwnd, IDC_EC_PKTXT, package->desc().c_str());
    } else {
      SetDlgItemTextW(this->_hwnd, IDC_EC_PKTXT, L"");
    }
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), true);

  } else {
    // Type
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT06, L"None");
    // Location
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT07, L"N/A");
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT04), false);
    // Dependencies
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT08, L"N/A");
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT08), false);
    // Snapshot image
    SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)nullptr);
    // Package description
    SetDlgItemTextW(this->_hwnd, IDC_EC_PKTXT, L"N/A");
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgSrc::_onResize()
{
  // Package Identity Label & EditControl
  this->_setControlPos(IDC_SC_LBL01, 5, 10, 64, 9);
  this->_setControlPos(IDC_EC_ENT01, 70, 10, this->width()-90, 13);
  // Package Display Name Label & EditControl
  this->_setControlPos(IDC_SC_LBL02, 5, 30, 64, 9);
  this->_setControlPos(IDC_EC_ENT02, 70, 30, this->width()-90, 13);
  // Package Parsed Version Label & EditControl
  this->_setControlPos(IDC_SC_LBL03, 5, 50, 64, 9);
  this->_setControlPos(IDC_EC_ENT03, 70, 50, this->width()-90, 13);
  // Package Computed UID Label & EditControl
  this->_setControlPos(IDC_SC_LBL04, 5, 70, 64, 9);
  this->_setControlPos(IDC_EC_ENT04, 70, 70, this->width()-90, 13);

    // Package Computed CRC Label & EditControl
  //this->_setControlPos(IDC_SC_LBL05, 5, 90, 64, 9);
  //this->_setControlPos(IDC_EC_ENT05, 70, 90, this->width()-90, 13);

  // separator
  this->_setControlPos(IDC_SC_SEP01, 5, 95, this->width()-25, 1);

  // Package Source Type Label & EditControl
  this->_setControlPos(IDC_SC_LBL06, 5, 110, 64, 9);
  this->_setControlPos(IDC_EC_ENT06, 70, 110, this->width()-90, 13);
  // Package Source Location Label & EditControl
  this->_setControlPos(IDC_SC_LBL07, 5, 130, 64, 9);
  this->_setControlPos(IDC_EC_ENT07, 70, 130, this->width()-90, 13);

  // separator
  this->_setControlPos(IDC_SC_SEP02, 5, 155, this->width()-25, 1);

  // Package Source Dependencies Label & EditControl
  this->_setControlPos(IDC_SC_LBL08, 5, 170, 64, 9);
  this->_setControlPos(IDC_EC_ENT08, 70, 170, this->width()-90, 26);

  // separator
  this->_setControlPos(IDC_SC_SEP03, 5, 205, this->width()-25, 1);

  // Package Source Snapshot Label
  this->_setControlPos(IDC_SC_LBL09, 5, 220, 64, 9);
  // Package Source Snapshot Image
  this->_setControlPos(IDC_SB_PKIMG, 70, 220, 42, 38);
  // Package Source Description Label
  this->_setControlPos(IDC_SC_LBL10, 5, 265, 64, 9);
  // Package Source Description EditControl
  this->_setControlPos(IDC_EC_PKTXT, 70, 265, this->width()-90, this->height()-275);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgSrc::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgSrc::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropPkgSrc::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return false;
}
