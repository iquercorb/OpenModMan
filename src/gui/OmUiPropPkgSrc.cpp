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
OmUiPropPkgSrc::OmUiPropPkgSrc(HINSTANCE hins) : OmDialog(hins),
  _hFtMonos(Om_createFont(14, 400, L"Consolas")),
  _hBmThumb(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPkgSrc::~OmUiPropPkgSrc()
{
  DeleteObject(this->_hFtMonos);
  if(this->_hBmThumb) DeleteObject(this->_hBmThumb);
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
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtMonos), 1);

  OmPackage* pPkg = static_cast<OmUiPropPkg*>(this->_parent)->package();
  if(pPkg == nullptr) return;

  // Ident
  this->setItemText(IDC_EC_OUT01, pPkg->ident());
  // Name
  this->setItemText(IDC_EC_OUT02, pPkg->name());
  // Parsed Version
  this->setItemText(IDC_EC_OUT03, pPkg->version().asString());
  // Computed Hash
  this->setItemText(IDC_EC_OUT04, Om_toHexString(pPkg->hash()));

  if(pPkg->isType(PKG_TYPE_SRC)) {

    // Type
    if(pPkg->isType(PKG_TYPE_ZIP)) {
      this->setItemText(IDC_EC_OUT06, L"Zip archive");
    } else {
      this->setItemText(IDC_EC_OUT06, L"Sub-folder");
    }

    // Location
    this->enableItem(IDC_EC_OUT07, true);
    this->setItemText(IDC_EC_OUT07, pPkg->sourcePath());

    // Dependencies
    this->enableItem(IDC_EC_OUT08, true);
    if(pPkg->dependCount()) {
      wstring dpn_str;
      for(unsigned i = 0; i < pPkg->dependCount(); ++i) {
        dpn_str += pPkg->depend(i);
        if(i < (pPkg->dependCount() - 1)) {
          dpn_str += L"; ";
        }
      }
      this->setItemText(IDC_EC_OUT08, dpn_str);
    } else {
      this->setItemText(IDC_EC_OUT08, L"None");
    }

    // Snapshot image
    if(pPkg->image().thumbnail()) {
      this->_hBmThumb = pPkg->image().thumbnail();
    } else {
      this->_hBmThumb = static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_BLANK),IMAGE_BITMAP,0,0,0));
    }
    this->setStImage(IDC_SB_PKIMG, this->_hBmThumb);


    // Package description
    this->enableItem(IDC_EC_PKTXT, true);
    if(pPkg->desc().size()) {
      this->setItemText(IDC_EC_PKTXT, pPkg->desc());
    } else {
      this->setItemText(IDC_EC_PKTXT, L"");
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
    this->_hBmThumb = static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_BLANK),IMAGE_BITMAP,0,0,0));
    this->setStImage(IDC_SB_PKIMG, this->_hBmThumb);
    // Package description
    this->setItemText(IDC_EC_PKTXT, L"N/A");
    this->enableItem(IDC_EC_PKTXT, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkgSrc::_onResize()
{
  // Package Identity Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 10, 64, 9);
  this->_setItemPos(IDC_EC_OUT01, 70, 10, this->width()-90, 13);
  // Package Display Name Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 5, 26, 64, 9);
  this->_setItemPos(IDC_EC_OUT02, 70, 26, this->width()-90, 13);
  // Package Parsed Version Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 5, 42, 64, 9);
  this->_setItemPos(IDC_EC_OUT03, 70, 42, this->width()-90, 13);
  // Package Computed UID Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 5, 58, 64, 9);
  this->_setItemPos(IDC_EC_OUT04, 70, 58, this->width()-90, 13);

    // Package Computed CRC Label & EditControl
  //this->_setItemPos(IDC_SC_LBL05, 5, 90, 64, 9);
  //this->_setItemPos(IDC_EC_OUT05, 70, 90, this->width()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP01, 5, 79, this->width()-25, 1);

  // Package Source Type Label & EditControl
  this->_setItemPos(IDC_SC_LBL06, 5, 92, 64, 9);
  this->_setItemPos(IDC_EC_OUT06, 70, 92, this->width()-90, 13);
  // Package Source Location Label & EditControl
  this->_setItemPos(IDC_SC_LBL07, 5, 108, 64, 9);
  this->_setItemPos(IDC_EC_OUT07, 70, 108, this->width()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP02, 5, 129, this->width()-25, 1);

  // Package Source Dependencies Label & EditControl
  this->_setItemPos(IDC_SC_LBL08, 5, 140, 64, 9);
  this->_setItemPos(IDC_EC_OUT08, 70, 140, this->width()-90, 26);

  // separator
  this->_setItemPos(IDC_SC_SEP03, 5, 177, this->width()-25, 1);

  // Package Source Snapshot Label
  this->_setItemPos(IDC_SC_LBL09, 5, 188, 64, 9);
  // Package Source Snapshot Image
  this->_setItemPos(IDC_SB_PKIMG, 70, 188, 85, 78);

  // Package Source Description Label
  this->_setItemPos(IDC_SC_LBL10, 5, 275, 64, 9);
  // Package Source Description EditControl
  this->_setItemPos(IDC_EC_PKTXT, 70, 275, this->width()-90, this->height()-285);
}
