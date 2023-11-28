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

#include "OmModPack.h"

#include "OmUiPropMod.h"

#include "OmUtilHsh.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropModCmn.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropModCmn::OmUiPropModCmn(HINSTANCE hins) : OmDialogPropTab(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropModCmn::~OmUiPropModCmn()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropModCmn::id() const
{
  return IDD_PROP_PKG_CMN;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropModCmn::_onTabInit()
{
  // defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(14,400,L"Consolas");
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), 1);

  // define controls tool-tips
  this->_createTooltip(IDC_EC_READ1,  L"Mod identity, the raw filename without extension");
  this->_createTooltip(IDC_EC_READ2,  L"Mod core (or master) name, raw filename without version");
  this->_createTooltip(IDC_EC_READ3,  L"Mod version number");
  this->_createTooltip(IDC_EC_READ4,  L"Mod modification category as defined by creator");

  OmModPack* ModPack = static_cast<OmUiPropMod*>(this->_parent)->getModPack();
  if(!ModPack)
    return;

  // name
  this->setItemText(IDC_EC_READ1, ModPack->name());

  // Parsed Version
  if(ModPack->version().valid()) {
    this->enableItem(IDC_EC_READ2, true);
    this->setItemText(IDC_EC_READ2, ModPack->version().asString());
  } else {
    this->enableItem(IDC_EC_READ2, false);
    this->setItemText(IDC_EC_READ2, L"N/A");
  }

  // Mod Category
  if(ModPack->category().size()) {
    this->enableItem(IDC_EC_READ3, true);
    this->setItemText(IDC_EC_READ3, ModPack->category());
  } else {
    this->enableItem(IDC_EC_READ3, false);
    this->setItemText(IDC_EC_READ3, L"<not defined>");
  }

  // hash
  wchar_t hash_str[16];
  swprintf(hash_str, L"0x%x\n", ModPack->hash());
  this->setItemText(IDC_EC_READ4, hash_str);

  // Identity
  this->setItemText(IDC_EC_READ5, ModPack->iden());

  // Core name
  this->setItemText(IDC_EC_READ6, ModPack->core());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropModCmn::_onTabResize()
{
  // Name Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 10, 64, 9);
  this->_setItemPos(IDC_EC_READ1, 70, 10, this->cliUnitX()-90, 13);
  // Version Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 5, 26, 64, 9);
  this->_setItemPos(IDC_EC_READ2, 70, 26, this->cliUnitX()-90, 13);
  // Category Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 5, 42, 64, 9);
  this->_setItemPos(IDC_EC_READ3, 70, 42, this->cliUnitX()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP01, 5, 62, this->cliUnitX()-25, 1);

  // hash Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 5, 74, 64, 9);
  this->_setItemPos(IDC_EC_READ4, 70, 74, this->cliUnitX()-90, 13);
  // Identity Label & EditControl
  this->_setItemPos(IDC_SC_LBL05, 5, 90, 64, 9);
  this->_setItemPos(IDC_EC_READ5, 70, 90, this->cliUnitX()-90, 13);
  // Core name Label & EditControl
  this->_setItemPos(IDC_SC_LBL06, 5, 106, 64, 9);
  this->_setItemPos(IDC_EC_READ6, 70, 106, this->cliUnitX()-90, 13);

}
