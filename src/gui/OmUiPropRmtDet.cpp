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
#include "OmRemote.h"
#include "OmPackage.h"
#include "gui/OmUiPropRmt.h"
#include "gui/OmUiPropRmtDet.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropRmtDet::OmUiPropRmtDet(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropRmtDet::~OmUiPropRmtDet()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_PKTXT, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropRmtDet::id() const
{
  return IDD_PROP_RMT_DET;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropRmtDet::_onInit()
{
  // defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(14,400,L"Consolas");
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), 1);

  OmRemote* pRmt = static_cast<OmUiPropRmt*>(this->_parent)->rmtCur();
  if(pRmt == nullptr) return;

  wstring item_str;

  // Ident
  this->setItemText(IDC_EC_OUT01, pRmt->ident());
  // Core
  this->setItemText(IDC_EC_OUT02, pRmt->core());
  // Parsed Version
  this->setItemText(IDC_EC_OUT03, pRmt->version().asString());
  // Computed Hash
  this->setItemText(IDC_EC_OUT04, Om_toHexString(pRmt->hash()));

  // Urls
  this->enableItem(IDC_EC_OUT05, true);
  item_str.clear();
  for(size_t i = 0; i < pRmt->urlCount(); ++i)
    item_str += pRmt->urlGet(i) + L"\n";
  this->setItemText(IDC_EC_OUT05, item_str);

  // Size
  item_str = Om_formatSizeSysStr(pRmt->bytes(), true);
  this->setItemText(IDC_EC_OUT06, item_str);

  // Checksum
  this->setItemText(IDC_EC_OUT07, pRmt->checksum());

  // Supersedes
  this->enableItem(IDC_EC_OUT08, true);
  if(pRmt->supCount()) {
    item_str.clear();
    for(size_t i = 0; i < pRmt->supCount(); ++i)
      item_str += pRmt->supGet(i)->ident() + L"\n";
    this->setItemText(IDC_EC_OUT08, item_str);
  } else {
    this->setItemText(IDC_EC_OUT08, L"None");
  }

  // Dependencies
  this->enableItem(IDC_EC_OUT09, true);
  if(pRmt->depCount()) {
    item_str.clear();
    for(unsigned i = 0; i < pRmt->depCount(); ++i) {
      item_str += pRmt->depGet(i);
      if(i < (pRmt->depCount() - 1)) {
        item_str += L"; ";
      }
    }
    this->setItemText(IDC_EC_OUT09, item_str);
  } else {
    this->setItemText(IDC_EC_OUT09, L"None");
  }

  // Snapshot image
  if(pRmt->image().thumbnail()) {
    this->setStImage(IDC_SB_PKG, pRmt->image().thumbnail());
  }

  // Package description
  this->enableItem(IDC_EC_PKTXT, true);
  if(pRmt->desc().size()) {
    this->setItemText(IDC_EC_PKTXT, pRmt->desc());
  } else {
    this->setItemText(IDC_EC_PKTXT, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropRmtDet::_onResize()
{
  // Identity Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 5, 64, 9);
  this->_setItemPos(IDC_EC_OUT01, 70, 5, this->width()-90, 13);
  // Core Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 5, 20, 64, 9);
  this->_setItemPos(IDC_EC_OUT02, 70, 20, this->width()-90, 13);
  // Version Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 5, 35, 64, 9);
  this->_setItemPos(IDC_EC_OUT03, 70, 35, this->width()-90, 13);
  // Hash Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 5, 50, 64, 9);
  this->_setItemPos(IDC_EC_OUT04, 70, 50, this->width()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP01, 5, 66, this->width()-25, 1);

  // Dowload URLs Label & EditControl
  this->_setItemPos(IDC_SC_LBL05, 5, 75, 64, 9);
  this->_setItemPos(IDC_EC_OUT05, 70, 75, this->width()-90, 26);
  // Size Label & EditControl
  this->_setItemPos(IDC_SC_LBL06, 5, 100, 64, 9);
  this->_setItemPos(IDC_EC_OUT06, 70, 100, this->width()-90, 13);
  // Checksum Label & EditControl
  //this->_setItemPos(IDC_SC_LBL07, 5, 125, 64, 9);
  //this->_setItemPos(IDC_EC_OUT07, 70, 125, this->width()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP02, 5, 116, this->width()-25, 1);

  // Supersedes Label & EditControl
  this->_setItemPos(IDC_SC_LBL08, 5, 125, 64, 9);
  this->_setItemPos(IDC_EC_OUT08, 70, 125, this->width()-90, 26);

  // separator
  this->_setItemPos(IDC_SC_SEP03, 5, 151, this->width()-25, 1);

  // Dependencies Label & EditControl
  this->_setItemPos(IDC_SC_LBL09, 5, 160, 64, 9);
  this->_setItemPos(IDC_EC_OUT09, 70, 160, this->width()-90, 26);

  // separator
  this->_setItemPos(IDC_SC_SEP04, 5, 185, this->width()-25, 1);

  // Package Source Snapshot Label
  this->_setItemPos(IDC_SC_LBL10, 5, 195, 64, 9);
  // Package Source Snapshot Image
  this->_setItemPos(IDC_SB_PKG, 70, 195, 85, 78);

  // Package Source Description Label
  this->_setItemPos(IDC_SC_LBL11, 5, 280, 64, 9);
  // Package Source Description EditControl
  this->_setItemPos(IDC_EC_PKTXT, 70, 280, this->width()-90, this->height()-285);
}
