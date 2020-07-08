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
#include "gui/OmUiPropPkgBck.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPkgBck::OmUiPropPkgBck(HINSTANCE hins) : OmDialog(hins)
{
  //ctor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPkgBck::~OmUiPropPkgBck()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropPkgBck::id() const
{
  return IDD_PROP_PKG_BCK;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgBck::_onShow()
{
  OmPackage* package = reinterpret_cast<OmUiPropPkg*>(this->_parent)->package();

  if(package == nullptr)
    return;

  // defines fonts for package description, title, and log output
  HFONT hFont = CreateFont(14,0,0,0,400,false,false,false,1,0,0,5,0,"Consolas");
  SendMessage(GetDlgItem(_hwnd, IDC_EC_ENT05), WM_SETFONT, (WPARAM)hFont, 1);

  if(package->isType(PKG_TYPE_BCK)) {
    // Type
    if(Om_isDir(package->backupPath())) {
      SetDlgItemTextW(this->_hwnd, IDC_EC_ENT01, L"Sub-folder");
    } else {
      SetDlgItemTextW(this->_hwnd, IDC_EC_ENT01, L"Zip archive");
    }

    // Location
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT02, package->backupPath().c_str());
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT02), true);

    // Overlap by
    if(package->overlapCount()) {
      wstring olap_str;
      unsigned n = package->overlapCount();
      for(unsigned i = 0; i < n; ++i) {
        olap_str += Om_toHexString(package->overlap(i));
        if(i < (n - 1)) {
          olap_str += L"; ";
        }
      }
      SetDlgItemTextW(this->_hwnd, IDC_EC_ENT03, olap_str.c_str());
    } else {
      SetDlgItemTextW(this->_hwnd, IDC_EC_ENT03, L"None");
    }
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT03), true);

    // Total Size
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT04, Om_sizeString(Om_itemSize(package->backupPath())).c_str());
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT04), true);

    // Installed Files
    if(package->backupItemCount()) {
      wstring inst_str;
      for(unsigned i = 0; i < package->backupItemCount(); ++i) {
        if(package->backupItem(i).dest == PKGITEM_DEST_DEL) {
          inst_str += L"+  ";
        } else {
          inst_str += L"≠  ";
        }
        inst_str += package->backupItem(i).path;
        inst_str += L"\r\n";
      }
      SetDlgItemTextW(this->_hwnd, IDC_EC_ENT05, inst_str.c_str());
    } else {
      SetDlgItemTextW(this->_hwnd, IDC_EC_ENT05, L"None");
    }
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT05), true);

  } else {
    // Type
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT01, L"None");
    // Location
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT02, L"N/A");
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT02), false);
    // Overlap by
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT03, L"N/A");
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT03), false);
    // Total Size
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT04, L"N/A");
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT04), false);
    // Installed Files
    SetDlgItemTextW(this->_hwnd, IDC_EC_ENT05, L"");
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_ENT05), false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgBck::_onResize()
{
  // Package Backup Type Label & EditControl
  this->_setControlPos(IDC_SC_LBL01, 5, 10, 64, 9);
  this->_setControlPos(IDC_EC_ENT01, 70, 10, this->width()-90, 13);
  // Package Backup Location Label & EditControl
  this->_setControlPos(IDC_SC_LBL02, 5, 30, 64, 9);
  this->_setControlPos(IDC_EC_ENT02, 70, 30, this->width()-90, 13);

  // separator
  this->_setControlPos(IDC_SC_SEP01, 5, 55, this->width()-25, 1);

  // Package Backup Overlap Label & EditControl
  this->_setControlPos(IDC_SC_LBL03, 5, 70, 64, 9);
  this->_setControlPos(IDC_EC_ENT03, 70, 70, this->width()-90, 26);

  // separator
  this->_setControlPos(IDC_SC_SEP02, 5, 105, this->width()-25, 1);

  // Package Backup Total size Label & EditControl
  this->_setControlPos(IDC_SC_LBL04, 5, 120, 64, 9);
  this->_setControlPos(IDC_EC_ENT04, 70, 120, this->width()-90, 13);
  // Package Backup Installed Files Label
  this->_setControlPos(IDC_SC_LBL05, 5, 140, 64, 9);
  // Package Backup Installed Files EditControl
  this->_setControlPos(IDC_EC_ENT05, 70, 140, this->width()-90, this->height()-150);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgBck::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgBck::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropPkgBck::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return false;
}
