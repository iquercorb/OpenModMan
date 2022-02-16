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

#include "OmRemote.h"
#include "OmPackage.h"

#include "OmUiPropRmt.h"

#include "OmUtilHsh.h"
#include "OmUtilWin.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropRmtDet.h"


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
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
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
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), 1);

  // define controls tool-tips
  this->_createTooltip(IDC_EC_READ1,  L"Package identity, the raw filename without extension");
  this->_createTooltip(IDC_EC_READ2,  L"Package core (or master) name, raw filename without version");
  this->_createTooltip(IDC_EC_READ3,  L"Package version number");
  this->_createTooltip(IDC_EC_READ4,  L"Package filename hash value, an unique file/folder identifer");
  this->_createTooltip(IDC_EC_READ5,  L"Package download URLs");
  this->_createTooltip(IDC_EC_READ6,  L"Package file size");
  this->_createTooltip(IDC_EC_READ7,  L"Package file MD5 checksum");
  this->_createTooltip(IDC_EC_READ8,  L"List of package that this one supersedes as newer version");
  this->_createTooltip(IDC_EC_READ9,  L"List of package dependencies");
  this->_createTooltip(IDC_SB_SNAP,    L"Package snapshot image");
  this->_createTooltip(IDC_EC_DESC,    L"Package description text");

  OmRemote* pRmt = static_cast<OmUiPropRmt*>(this->_parent)->rmtCur();
  if(pRmt == nullptr) return;

  wstring item_str;

  // Ident
  this->setItemText(IDC_EC_READ1, pRmt->ident());
  // Core
  this->setItemText(IDC_EC_READ2, pRmt->core());
  // Parsed Version
  this->setItemText(IDC_EC_READ3, pRmt->version().asString());
  // Computed Hash
  this->setItemText(IDC_EC_READ4, Om_uint64ToStr(pRmt->hash()));

  // Urls
  this->enableItem(IDC_EC_READ5, true);
  item_str.clear();
  for(unsigned i = 0; i < pRmt->urlCount(); ++i) {
    item_str += pRmt->urlGet(i);
    if(i < (pRmt->urlCount() - 1)) item_str += L"\r\n";
  }
  this->setItemText(IDC_EC_READ5, item_str);

  // Size
  item_str = Om_formatSizeSysStr(pRmt->bytes(), true);
  this->setItemText(IDC_EC_READ6, item_str);

  // Checksum
  switch(pRmt->checksumType())
  {
  case RMT_CHECKSUM_XXH:
    this->setItemText(IDC_SC_LBL07, L"Checksum (xxh) :");
    break;
  case RMT_CHECKSUM_MD5:
    this->setItemText(IDC_SC_LBL07, L"Checksum (md5) :");
    break;
  default:
    this->setItemText(IDC_SC_LBL07, L"Checksum :");
    break;
  }

  this->setItemText(IDC_EC_READ7, pRmt->checksum());

  // Supersedes
  this->enableItem(IDC_EC_READ8, true);
  if(pRmt->supCount()) {
    item_str.clear();
    for(unsigned i = 0; i < pRmt->supCount(); ++i) {
      item_str += pRmt->supGet(i)->ident();
      if(i < (pRmt->supCount() - 1)) item_str += L"\r\n";
    }
    this->setItemText(IDC_EC_READ8, item_str);
  } else {
    this->setItemText(IDC_EC_READ8, L"None");
  }

  // Dependencies
  this->enableItem(IDC_EC_READ9, true);
  if(pRmt->depCount()) {
    item_str.clear();
    for(unsigned i = 0; i < pRmt->depCount(); ++i) {
      item_str += pRmt->depGet(i);
      if(i < (pRmt->depCount() - 1)) item_str += L"\r\n";
    }
    this->setItemText(IDC_EC_READ9, item_str);
  } else {
    this->setItemText(IDC_EC_READ9, L"None");
  }

  // Snapshot image
  if(pRmt->image().thumbnail()) {
    this->setStImage(IDC_SB_SNAP, pRmt->image().thumbnail());
  }

  // Package description
  this->enableItem(IDC_EC_DESC, true);
  if(pRmt->desc().size()) {
    this->setItemText(IDC_EC_DESC, pRmt->desc());
  } else {
    this->setItemText(IDC_EC_DESC, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropRmtDet::_onResize()
{
  // Identity Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 5, 64, 9);
  this->_setItemPos(IDC_EC_READ1, 70, 5, this->cliUnitX()-90, 13);
  // Core Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 5, 20, 64, 9);
  this->_setItemPos(IDC_EC_READ2, 70, 20, this->cliUnitX()-90, 13);
  // Version Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 5, 35, 64, 9);
  this->_setItemPos(IDC_EC_READ3, 70, 35, this->cliUnitX()-90, 13);
  // Hash Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 5, 50, 64, 9);
  this->_setItemPos(IDC_EC_READ4, 70, 50, this->cliUnitX()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP01, 5, 66, this->cliUnitX()-25, 1);

  // Dowload URLs Label & EditControl
  this->_setItemPos(IDC_SC_LBL05, 5, 75, 64, 9);
  this->_setItemPos(IDC_EC_READ5, 70, 75, this->cliUnitX()-90, 13);
  // Size Label & EditControl
  this->_setItemPos(IDC_SC_LBL06, 5, 90, 64, 9);
  this->_setItemPos(IDC_EC_READ6, 70, 90, this->cliUnitX()-90, 13);
  // Md5sum Label & EditControl
  this->_setItemPos(IDC_SC_LBL07, 5, 105, 64, 9);
  this->_setItemPos(IDC_EC_READ7, 70, 105, this->cliUnitX()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP02, 5, 120, this->cliUnitX()-25, 1);

  // Supersedes Label & EditControl
  this->_setItemPos(IDC_SC_LBL08, 5, 125, 64, 9);
  this->_setItemPos(IDC_EC_READ8, 70, 125, this->cliUnitX()-90, 26);

  // separator
  this->_setItemPos(IDC_SC_SEP03, 5, 155, this->cliUnitX()-25, 1);

  // Dependencies Label & EditControl
  this->_setItemPos(IDC_SC_LBL09, 5, 160, 64, 9);
  this->_setItemPos(IDC_EC_READ9, 70, 160, this->cliUnitX()-90, 26);

  // separator
  this->_setItemPos(IDC_SC_SEP04, 5, 190, this->cliUnitX()-25, 1);

  // Package Source Snapshot Label
  this->_setItemPos(IDC_SC_LBL10, 5, 195, 64, 9);
  // Package Source Snapshot Image
  this->_setItemPos(IDC_SB_SNAP, 70, 195, 85, 78);

  // Package Source Description Label
  this->_setItemPos(IDC_SC_LBL11, 5, 278, 64, 9);
  // Package Source Description EditControl
  this->_setItemPos(IDC_EC_DESC, 70, 278, this->cliUnitX()-90, this->cliUnitY()-285);
}
