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

#include "OmModMan.h"

#include "OmUiMan.h"
#include "OmUiAddPst.h"
#include "OmUiPropPst.h"
#include "OmUiPropHub.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropHubBat.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubBat::OmUiPropHubBat(HINSTANCE hins) : OmDialogPropTab(hins)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubBat::~OmUiPropHubBat()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropHubBat::id() const
{
  return IDD_PROP_HUB_BAT;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubBat::_onLbBatlsSel()
{
  int32_t lb_sel = this->msgItem(IDC_LB_PST, LB_GETCURSEL);

  if(lb_sel >= 0) {

    this->enableItem(IDC_BC_DEL, true);
    this->enableItem(IDC_BC_EDI, true);

    this->enableItem(IDC_BC_UP, (lb_sel > 0));
    int32_t lb_max = this->msgItem(IDC_LB_PST, LB_GETCOUNT) - 1;
    this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubBat::_onBcUpBat()
{
  // get selected item (index)
  int32_t lb_sel = this->msgItem(IDC_LB_PST, LB_GETCURSEL);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OM_MAX_ITEM];
  int32_t idx;

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_PST, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_PST, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_PST, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_PST, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_PST, LB_SETITEMDATA, lb_sel, idx);

  this->enableItem(IDC_BC_UP, (lb_sel > 1));
  this->enableItem(IDC_BC_DN, true);

  // user modified parameter, notify it
  this->paramCheck(HUB_PROP_BAT_ORDER);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubBat::_onBcDnBat()
{
  // get selected item (index)
  int32_t lb_sel = this->msgItem(IDC_LB_PST, LB_GETCURSEL);
  // get count of item in List-Box as index to for insertion
  int32_t lb_max = this->msgItem(IDC_LB_PST, LB_GETCOUNT) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OM_MAX_ITEM];
  int32_t idx;

  this->msgItem(IDC_LB_PST, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_PST, LB_GETITEMDATA, lb_sel);
  this->msgItem(IDC_LB_PST, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_PST, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_PST, LB_SETITEMDATA, lb_sel, idx);
  this->msgItem(IDC_LB_PST, LB_SETCURSEL, lb_sel);

  this->enableItem(IDC_BC_UP, true);
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));

  // user modified parameter, notify it
  this->paramCheck(HUB_PROP_BAT_ORDER);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubBat::_onBcEdiBat()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!ModHub)
    return;

  int32_t lb_sel = this->msgItem(IDC_LB_PST, LB_GETCURSEL);

  if(lb_sel >= 0 && lb_sel < static_cast<int32_t>(ModHub->presetCount())) {

    // open the Batch Properties dialog
    int32_t bat_id = this->msgItem(IDC_LB_PST, LB_GETITEMDATA, lb_sel, 0);

    OmUiPropPst* pUiPropBat = static_cast<OmUiPropPst*>(this->siblingById(IDD_PROP_PST));

    pUiPropBat->setModPset(ModHub->getPreset(bat_id));

    pUiPropBat->open();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubBat::_onBcDelBat()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!ModHub)
    return;

  // get selected item (index)
  int32_t lb_sel = this->msgItem(IDC_LB_PST, LB_GETCURSEL);

  if(lb_sel >= 0) {

    int32_t bat_id = this->msgItem(IDC_LB_PST, LB_GETITEMDATA, lb_sel, 0);

    // warns the user before committing the irreparable
    if(!Om_dlgBox_ynl(this->_hwnd, L"Mod Hub properties", IDI_QRY,
              L"Delete Script", L"Delete the Script ?",
              ModHub->getPreset(bat_id)->title()))
    {
      return;
    }

    if(!ModHub->deletePreset(bat_id)) {

      // warns the user error occurred
      Om_dlgBox_okl(this->_hwnd, L"Mod Hub properties", IDI_ERR,
                L"Script delete error", L"Script deletion "
                "process failed because of the following error:",
                ModHub->lastError());

      return;
    }

    // refresh the main window dialog, this will also refresh this one
    this->root()->refresh();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubBat::_onBcAddBat()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!ModHub)
    return;

  OmUiAddPst* pUiNewBat = static_cast<OmUiAddPst*>(this->siblingById(IDD_ADD_PST));
  pUiNewBat->setModHub(ModHub);
  pUiNewBat->open(true);
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubBat::_onCkBoxQuiet()
{
  // user modified parameter, notify it
  this->paramCheck(HUB_PROP_BAT_QUIETMODE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubBat::_onTabInit()
{
  this->setBmIcon(IDC_BC_ADD, Om_getResIcon(this->_hins, IDI_BT_ADD));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDI_BT_REM));
  this->setBmIcon(IDC_BC_EDI, Om_getResIcon(this->_hins, IDI_BT_MOD));
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(this->_hins, IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(this->_hins, IDI_BT_DN));

  // Define controls tool-tips
  this->_createTooltip(IDC_LB_PST,    L"Installation batches list");
  this->_createTooltip(IDC_BC_UP,     L"Move up in list");
  this->_createTooltip(IDC_BC_DN,     L"Move down in list");
  this->_createTooltip(IDC_BC_DEL,    L"Delete the selected Installation batch");
  this->_createTooltip(IDC_BC_ADD,    L"Create a new Installation batch for this Mod Hub");
  this->_createTooltip(IDC_BC_EDI,    L"Edit Installation batch properties");
  this->_createTooltip(IDC_BC_CKBX1,  L"Disable installation warning messages for batches execution.");

  // Set controls default states and parameters
  this->enableItem(IDC_BC_DEL, false);
  this->enableItem(IDC_BC_EDI, false);

  // Update values
  this->_onTabRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubBat::_onTabResize()
{
  // Mod Channel list Label & ListBox
  this->_setItemPos(IDC_SC_LBL01, 50, 15, 180, 9);
  this->_setItemPos(IDC_LB_PST, 50, 25, this->cliUnitX()-107, 58);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliUnitX()-55, 39, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->cliUnitX()-55, 55, 16, 15);

  // Remove & Modify Buttons
  this->_setItemPos(IDC_BC_DEL, 50, 85, 50, 14);
  this->_setItemPos(IDC_BC_EDI, 105, 85, 50, 14);
  // Add button
  this->_setItemPos(IDC_BC_ADD, this->cliUnitX()-108, 85, 50, 14);

  // Options label
  //this->_setItemPos(IDC_SC_LBL02, 50, 120, 180, 9);
  // Quiet Batches checkbox
  this->_setItemPos(IDC_BC_CKBX1, 50, 125, 240, 9);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubBat::_onTabRefresh()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!ModHub)
    return;

  this->msgItem(IDC_LB_PST, LB_RESETCONTENT);

  for(unsigned i = 0; i < ModHub->presetCount(); ++i) {
    this->msgItem(IDC_LB_PST, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(ModHub->getPreset(i)->title().c_str()));
    this->msgItem(IDC_LB_PST, LB_SETITEMDATA, i, i); // for Mod Channel index reordering
  }

  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, ModHub->presetQuietMode());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropHubBat::_onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_PST: //< Mod Channel(s) list List-Box
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_onLbBatlsSel();
      break;

    case IDC_BC_UP: // Down Button
      this->_onBcUpBat();
      break;

    case IDC_BC_DN: // Up Button
      this->_onBcDnBat();
      break;

    case IDC_BC_EDI: // "Modify" Button
      this->_onBcEdiBat();
      break;

    case IDC_BC_DEL: //< "Remove" Button
      this->_onBcDelBat();
      break;

    case IDC_BC_ADD: //< "New" Button
      this->_onBcAddBat();
      break;

    case IDC_BC_CKBX1:
      this->_onCkBoxQuiet();
      break;
    }
  }

  return false;
}

