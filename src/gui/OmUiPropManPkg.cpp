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
#include "gui/OmUiPropManPkg.h"
#include "OmDialogProp.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropManPkg::OmUiPropManPkg(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i)
    this->_chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropManPkg::~OmUiPropManPkg()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropManPkg::id() const
{
  return IDD_PROP_MAN_PKG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManPkg::setChParam(unsigned i, bool en)
{
  _chParam[i] = en;
  reinterpret_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManPkg::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CHK01,  L"Show and treat folders as packages");

  this->_createTooltip(IDC_BC_CHK02,  L"Warn when package installation may overwrite another");
  this->_createTooltip(IDC_BC_CHK03,  L"Warn when package installation require additional installation");
  this->_createTooltip(IDC_BC_CHK04,  L"Warn when package dependencies are missing");
  this->_createTooltip(IDC_BC_CHK05,  L"Warn when package uninstall require additional uninstall");

  this->_createTooltip(IDC_BC_CHK06,  L"Ignore installation warnings for batches");

  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  this->msgItem(IDC_BC_CHK01, BM_SETCHECK, manager->legacySupport());
  this->msgItem(IDC_BC_CHK02, BM_SETCHECK, manager->warnOverlaps());
  this->msgItem(IDC_BC_CHK03, BM_SETCHECK, manager->warnExtraInst());
  this->msgItem(IDC_BC_CHK04, BM_SETCHECK, manager->warnMissDpnd());
  this->msgItem(IDC_BC_CHK05, BM_SETCHECK, manager->warnExtraUnin());
  this->msgItem(IDC_BC_CHK06, BM_SETCHECK, manager->quietBatches());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManPkg::_onResize()
{
  // Legacy support CheckBox
  this->_setItemPos(IDC_BC_CHK01, 50, 20, 170, 9);

  // Enable warnings Label
  this->_setItemPos(IDC_SC_LBL01, 50, 40, 170, 9);
  // Enable warnings CheckBoxes
  this->_setItemPos(IDC_BC_CHK02, 65, 52, 170, 9);
  this->_setItemPos(IDC_BC_CHK03, 65, 62, 170, 9);
  this->_setItemPos(IDC_BC_CHK04, 65, 72, 170, 9);
  this->_setItemPos(IDC_BC_CHK05, 65, 82, 170, 9);

  // Ignore Batches warnings CheckBox
  this->_setItemPos(IDC_BC_CHK06, 50, 110, 170, 9);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropManPkg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_CHK01: //< Check box for Package legacy (folders) support
    case IDC_BC_CHK02: //< Check box for Warn at Installation overlaps
    case IDC_BC_CHK03: //< Check box for Warn at Additional installation due to dependencies
    case IDC_BC_CHK04: //< Check box for Warn at Installation dependencies missing
    case IDC_BC_CHK05: //< Check box for Warn at Additional restoration due to overlaps
    case IDC_BC_CHK06: //< Check box for Ignore warnings for batches
      // user modified parameter, notify it
      this->setChParam(MAN_PROP_PKG_PACKAGE_FLAGS, true);
      break;
    }
  }

  return false;
}
