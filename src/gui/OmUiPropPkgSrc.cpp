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

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPkgSrc::~OmUiPropPkgSrc()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_TXT, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
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
void OmUiPropPkgSrc::_onInit()
{
  // defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(14,400,L"Consolas");
  this->msgItem(IDC_EC_TXT, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), 1);

  // define controls tool-tips
  this->_createTooltip(IDC_EC_OUT01,  L"Package identity, the raw filename without extension");
  this->_createTooltip(IDC_EC_OUT02,  L"Package core (or master) name, raw filename without version");
  this->_createTooltip(IDC_EC_OUT03,  L"Package version number");
  this->_createTooltip(IDC_EC_OUT04,  L"Package filename hash value, an unique file/folder identifer");
  this->_createTooltip(IDC_EC_OUT06,  L"Package source type, either zip archive or subfolder (developer mode)");
  this->_createTooltip(IDC_EC_OUT07,  L"Package source location path");
  this->_createTooltip(IDC_EC_OUT08,  L"List of package dependencies");
  this->_createTooltip(IDC_SB_PKG,    L"Package snapshot image");
  this->_createTooltip(IDC_EC_TXT,    L"Package description text");

  OmPackage* pPkg = static_cast<OmUiPropPkg*>(this->_parent)->pkgCur();
  if(pPkg == nullptr) return;

  // Ident
  this->setItemText(IDC_EC_OUT01, pPkg->ident());
  // Core name
  this->setItemText(IDC_EC_OUT02, pPkg->core());
  // Parsed Version
  this->setItemText(IDC_EC_OUT03, pPkg->version().asString());
  // Filename hash
  this->setItemText(IDC_EC_OUT04, Om_toHexString(pPkg->hash()));

  if(pPkg->hasSrc()) {

    // Type
    if(pPkg->isZip()) {
      this->setItemText(IDC_EC_OUT06, L"Zip archive");
    } else {
      this->setItemText(IDC_EC_OUT06, L"Subfolder");
    }

    // Location
    this->enableItem(IDC_EC_OUT07, true);
    this->setItemText(IDC_EC_OUT07, pPkg->srcPath());

    // Dependencies
    this->enableItem(IDC_EC_OUT08, true);
    if(pPkg->depCount()) {
      wstring dpn_str;
      for(unsigned i = 0; i < pPkg->depCount(); ++i) {
        dpn_str += pPkg->depGet(i);
        if(i < (pPkg->depCount() - 1)) {
          dpn_str += L"\r\n";
        }
      }
      this->setItemText(IDC_EC_OUT08, dpn_str);
    } else {
      this->setItemText(IDC_EC_OUT08, L"None");
    }

    // Snapshot image
    if(pPkg->image().thumbnail()) {
      this->setStImage(IDC_SB_PKG, pPkg->image().thumbnail());
    }

    // Package description
    this->enableItem(IDC_EC_TXT, true);
    if(pPkg->desc().size()) {
      this->setItemText(IDC_EC_TXT, pPkg->desc());
    } else {
      this->setItemText(IDC_EC_TXT, L"");
    }

  } else {

    // Type
    this->setItemText(IDC_EC_OUT06, L"None");
    // Location
    this->setItemText(IDC_EC_OUT07, L"N/A");
    this->enableItem(IDC_EC_OUT07, false);
    // Dependencies
    this->setItemText(IDC_EC_OUT08, L"N/A");
    this->enableItem(IDC_EC_OUT08, false);
    // Snapshot image
    this->setStImage(IDC_SB_PKG, Om_getResImage(this->_hins, IDB_PKG_THN));
    // Package description
    this->setItemText(IDC_EC_TXT, L"N/A");
    this->enableItem(IDC_EC_TXT, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgSrc::_onResize()
{
  // Identity Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 10, 64, 9);
  this->_setItemPos(IDC_EC_OUT01, 70, 10, this->width()-90, 13);
  // Core name Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 5, 26, 64, 9);
  this->_setItemPos(IDC_EC_OUT02, 70, 26, this->width()-90, 13);
  // Version Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 5, 42, 64, 9);
  this->_setItemPos(IDC_EC_OUT03, 70, 42, this->width()-90, 13);
  // Filename hash Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 5, 58, 64, 9);
  this->_setItemPos(IDC_EC_OUT04, 70, 58, this->width()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP01, 5, 79, this->width()-25, 1);

  // Source Type Label & EditControl
  this->_setItemPos(IDC_SC_LBL06, 5, 92, 64, 9);
  this->_setItemPos(IDC_EC_OUT06, 70, 92, this->width()-90, 13);
  // Source Location Label & EditControl
  this->_setItemPos(IDC_SC_LBL07, 5, 108, 64, 9);
  this->_setItemPos(IDC_EC_OUT07, 70, 108, this->width()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP02, 5, 129, this->width()-25, 1);

  // Dependencies Label & EditControl
  this->_setItemPos(IDC_SC_LBL08, 5, 140, 64, 9);
  this->_setItemPos(IDC_EC_OUT08, 70, 140, this->width()-90, 26);

  // separator
  this->_setItemPos(IDC_SC_SEP03, 5, 177, this->width()-25, 1);

  // PSnapshot Label
  this->_setItemPos(IDC_SC_LBL09, 5, 188, 64, 9);
  // Snapshot Image
  this->_setItemPos(IDC_SB_PKG, 70, 188, 85, 78);

  // Description Label
  this->_setItemPos(IDC_SC_LBL10, 5, 275, 64, 9);
  // Description EditControl
  this->_setItemPos(IDC_EC_TXT, 70, 275, this->width()-90, this->height()-285);
}
