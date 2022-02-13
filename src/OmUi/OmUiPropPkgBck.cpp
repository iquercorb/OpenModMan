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

#include "OmBaseUi.h"

#include "OmPackage.h"
#include "OmLocation.h"

#include "OmUiPropPkg.h"

#include "OmUtilFs.h"
#include "OmUtilHsh.h"
#include "OmUtilWin.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropPkgBck.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPkgBck::OmUiPropPkgBck(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPkgBck::~OmUiPropPkgBck()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  DeleteObject(hFt);
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
void OmUiPropPkgBck::_onInit()
{
  // defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(14, 400, L"Consolas");
  this->msgItem(IDC_EC_READ4, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  // define controls tool-tips
  this->_createTooltip(IDC_EC_READ1,  L"Package backup type, either zip archive or subfolder");
  this->_createTooltip(IDC_EC_READ2,  L"Package backup location path");
  this->_createTooltip(IDC_EC_READ3,  L"List of packages that the installation of this one overlaps");
  this->_createTooltip(IDC_EC_READ4,  L"If any, the total size of current backup data");
  this->_createTooltip(IDC_EC_READ5,  L"If any, list of currently installed files");

  OmPackage* pPkg = static_cast<OmUiPropPkg*>(this->_parent)->pkgCur();
  if(pPkg == nullptr) return;

  if(pPkg->hasBck()) {

    // Type
    if(Om_isDir(pPkg->bckPath())) {
      this->setItemText(IDC_EC_READ1, L"Subfolder");
    } else {
      this->setItemText(IDC_EC_READ1, L"Zip archive");
    }

    // Location
    this->enableItem(IDC_EC_READ2, true);
    this->setItemText(IDC_EC_READ2, pPkg->bckPath());

    // Overlap by
    this->enableItem(IDC_EC_READ3, true);
    if(pPkg->ovrCount()) {

      OmLocation* pLoc = pPkg->pLoc();

      wstring olap_str;
      uint64_t olap_hsh;
      OmPackage* olap_pkg;

      unsigned n = pPkg->ovrCount();
      for(unsigned i = 0; i < n; ++i) {

        olap_hsh = pPkg->ovrGet(i);
        olap_pkg = pLoc->pkgFind(olap_hsh);
        olap_str += olap_pkg != nullptr ? olap_pkg->ident() : Om_toHexString(olap_hsh);

        if(i < (n - 1)) {
          olap_str += L"\r\n";
        }
      }
      this->setItemText(IDC_EC_READ3, olap_str);
    } else {
      this->setItemText(IDC_EC_READ3, L"None");
    }

    // Total Size
    this->enableItem(IDC_EC_READ4, true);
    this->setItemText(IDC_EC_READ4, Om_formatSizeSysStr(Om_itemSize(pPkg->bckPath()), true));

    // Installed Files
    this->enableItem(IDC_EC_READ5, true);
    if(pPkg->bckItemCount()) {
      wstring inst_str;
      for(unsigned i = 0; i < pPkg->bckItemCount(); ++i) {
        if(pPkg->bckItemGet(i).dest == PKGITEM_DEST_DEL) {
          inst_str += L"+  ";
        } else {
          inst_str += L"≠  ";
        }
        inst_str += pPkg->bckItemGet(i).path;
        inst_str += L"\r\n";
      }
      this->setItemText(IDC_EC_READ5, inst_str);
    } else {
      this->setItemText(IDC_EC_READ5, L"None");
    }

  } else {
    // Type
    this->setItemText(IDC_EC_READ1, L"None");
    // Location
    this->setItemText(IDC_EC_READ2, L"N/A");
    this->enableItem(IDC_EC_READ2, false);
    // Overlap by
    this->setItemText(IDC_EC_READ3, L"N/A");
    this->enableItem(IDC_EC_READ3, false);
    // Total Size
    this->setItemText(IDC_EC_READ4, L"N/A");
    this->enableItem(IDC_EC_READ4, false);
    // Installed Files
    this->setItemText(IDC_EC_READ5, L"");
    this->enableItem(IDC_EC_READ5, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgBck::_onResize()
{
  // Package Backup Type Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 10, 64, 9);
  this->_setItemPos(IDC_EC_READ1, 70, 10, this->cliUnitX()-90, 13);
  // Package Backup Location Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 5, 26, 64, 9);
  this->_setItemPos(IDC_EC_READ2, 70, 26, this->cliUnitX()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP01, 5, 47, this->cliUnitX()-25, 1);

  // Package Backup Overlap Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 5, 58, 64, 9);
  this->_setItemPos(IDC_EC_READ3, 70, 58, this->cliUnitX()-90, 26);

  // separator
  this->_setItemPos(IDC_SC_SEP02, 5, 95, this->cliUnitX()-25, 1);

  // Package Backup Total size Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 5, 106, 64, 9);
  this->_setItemPos(IDC_EC_READ4, 70, 106, this->cliUnitX()-90, 13);
  // Package Backup Installed Files Label
  this->_setItemPos(IDC_SC_LBL05, 5, 122, 64, 9);
  // Package Backup Installed Files EditControl
  this->_setItemPos(IDC_EC_READ5, 70, 122, this->cliUnitX()-90, this->cliUnitY()-132);
}
