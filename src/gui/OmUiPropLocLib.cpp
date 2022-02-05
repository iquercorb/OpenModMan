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
#include "gui/OmUiPropLoc.h"
#include "gui/OmUiPropLocLib.h"
#include "OmDialogProp.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocLib::OmUiPropLocLib(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i)
    this->_chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocLib::~OmUiPropLocLib()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropLocLib::id() const
{
  return IDD_PROP_LOC_LIB;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocLib::setChParam(unsigned i, bool en)
{
  _chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocLib::_onCkBoxDev()
{
  // user modified parameter, notify it
  this->setChParam(LOC_PROP_LIB_DEVMODE, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocLib::_onCkBoxWrn()
{
  // user modified parameter, notify it
  this->setChParam(LOC_PROP_LIB_WARNINGS, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocLib::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CKBX1,  L"Parse library subfolders as packages source for mod development purpose or legacy support");

  this->_createTooltip(IDC_BC_CKBX2,  L"Warn when packages install will overlap any previously installed");
  this->_createTooltip(IDC_BC_CKBX3,  L"Warn when packages install require additional dependencies installation");
  this->_createTooltip(IDC_BC_CKBX4,  L"Warn when packages dependencies are missing");
  this->_createTooltip(IDC_BC_CKBX5,  L"Warn when packages uninstall require additional uninstallations");

  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->locCur();
  if(!pLoc) return;

  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, pLoc->libDevMode());
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, pLoc->warnOverlaps());
  this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, pLoc->warnExtraInst());
  this->msgItem(IDC_BC_CKBX4, BM_SETCHECK, pLoc->warnMissDeps());
  this->msgItem(IDC_BC_CKBX5, BM_SETCHECK, pLoc->warnExtraUnin());

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocLib::_onResize()
{
  // Library Dev Mode CheckBox
  this->_setItemPos(IDC_BC_CKBX1, 50, 20, 200, 9);

  // Enable warnings Label
  this->_setItemPos(IDC_SC_LBL01, 50, 45, 200, 9);

  // Enable warnings CheckBoxes
  this->_setItemPos(IDC_BC_CKBX2, 65, 55, 200, 9);
  this->_setItemPos(IDC_BC_CKBX3, 65, 65, 200, 9);
  this->_setItemPos(IDC_BC_CKBX4, 65, 75, 200, 9);
  this->_setItemPos(IDC_BC_CKBX5, 65, 85, 200, 9);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropLocLib::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_CKBX1: //< Check box for Package legacy (folders) support
      this->_onCkBoxDev();
      break;

    case IDC_BC_CKBX2: //< Check box for Warn at Installation overlaps
    case IDC_BC_CKBX3: //< Check box for Warn at Additional installation due to dependencies
    case IDC_BC_CKBX4: //< Check box for Warn at Installation dependencies missing
    case IDC_BC_CKBX5: //< Check box for Warn at Additional restoration due to overlaps
      this->_onCkBoxWrn();
      break;
    }
  }

  return false;
}
