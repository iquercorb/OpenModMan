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
  #include <algorithm>    // std::find

#include "OmBaseUi.h"

#include "OmModHub.h"
#include "OmModPset.h"
#include "OmDialogProp.h"

#include "OmUiPropPst.h"

#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropPstStg.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPstStg::OmUiPropPstStg(HINSTANCE hins) : OmDialogPropTab(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPstStg::~OmUiPropPstStg()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropPstStg::id() const
{
  return IDD_PROP_PST_STG;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstStg::_onCkBoxIonly()
{
  this->paramCheck(PST_PROP_STG_IONLY);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstStg::_onTabInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Script name, to identify it");
  this->_createTooltip(IDC_BC_CKBX1,  L"Script will install selected packages without uninstalling others");

  this->_onTabRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstStg::_onTabResize()
{
  // Batch Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 50, 25, 240, 9);
  this->_setItemPos(IDC_EC_INP01, 50, 35, this->cliUnitX()-100, 13);

  // Install Only checkbox
  this->_setItemPos(IDC_BC_CKBX1, 50, 65, this->cliUnitX()-100, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstStg::_onTabRefresh()
{
  OmModPset* ModPset = static_cast<OmUiPropPst*>(this->_parent)->ModPset();
  if(!ModPset)
    return;

  OmModHub* ModHub = ModPset->ModHub();
  if(!ModHub)
    return;

  // batch title
  this->setItemText(IDC_EC_INP01, ModPset->title());

  // batch install-only option
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, ModPset->installOnly());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropPstStg::_onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01: //< Title EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->paramCheck(PST_PROP_STG_TITLE);
      break;

    case IDC_BC_CKBX1:
      this->_onCkBoxIonly();
      break;
    }
  }

  return false;
}
