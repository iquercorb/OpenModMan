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

#include "OmUiPropHub.h"
#include "OmUiAddPst.h"
#include "OmUiPropPst.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropHubPst.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubPst::OmUiPropHubPst(HINSTANCE hins) : OmDialogPropTab(hins)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubPst::~OmUiPropHubPst()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropHubPst::id() const
{
  return IDD_PROP_HUB_PST;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubPst::_lb_pst_on_selchg()
{
  int32_t lb_sel = this->msgItem(IDC_LB_PST, LB_GETCURSEL);

  bool has_select = (lb_sel >= 0);

  this->enableItem(IDC_BC_PSDEL, has_select);
  this->enableItem(IDC_BC_PSEDI, has_select);

  int32_t lb_max = this->msgItem(IDC_LB_PST, LB_GETCOUNT) - 1;
  this->enableItem(IDC_BC_UP, (lb_sel > 0));
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubPst::_preset_list_up()
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
void OmUiPropHubPst::_preset_list_dn()
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
void OmUiPropHubPst::_preset_dialog_add()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->ModHub();
  if(!ModHub) return;

  OmUiAddPst* UiAddPst = static_cast<OmUiAddPst*>(this->root()->childById(IDD_ADD_PST));

  UiAddPst->setModHub(ModHub);

  UiAddPst->open(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubPst::_preset_delete()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->ModHub();
  if(!ModHub) return;

  // get selected item (index)
  int32_t lb_sel = this->msgItem(IDC_LB_PST, LB_GETCURSEL);
  if(lb_sel < 0) return;

  // get real preset index in list from item data
  int32_t bat_id = this->msgItem(IDC_LB_PST, LB_GETITEMDATA, lb_sel, 0);

  OmModPset* ModPset = ModHub->getPreset(bat_id);
  if(!ModPset) return;

  // warns the user before committing the irreparable
  if(!Om_dlgBox_ynl(this->_hwnd, L"Hub properties", IDI_DLG_QRY, L"Delete Preset",
                    L"Delete the following Preset ?", ModPset->title()))
    return;

  if(!ModHub->deletePreset(bat_id)) {
    // warns the user error occurred
    Om_dlgBox_okl(this->_hwnd, L"Hub properties", IDI_DLG_ERR, L"Preset delete error",
                  L"Unable to delete Preset:",ModHub->lastError());
  }

  // refresh dialog tree
  this->root()->refresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubPst::_preset_dialog_prop()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->ModHub();
  if(!ModHub) return;

  int32_t lb_sel = this->msgItem(IDC_LB_PST, LB_GETCURSEL);
  if(lb_sel < 0) return;

  // get real preset index in list from item data
  int32_t bat_id = this->msgItem(IDC_LB_PST, LB_GETITEMDATA, lb_sel, 0);

  OmModPset* ModPset = ModHub->getPreset(bat_id);
  if(!ModPset) return;

  OmUiPropPst* UiPropPst = static_cast<OmUiPropPst*>(this->root()->childById(IDD_PROP_PST));

  UiPropPst->setModPset(ModPset);

  UiPropPst->open();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubPst::_onTbInit()
{
  this->setBmIcon(IDC_BC_PSNEW, Om_getResIcon(IDI_BT_ADD));
  this->setBmIcon(IDC_BC_PSDEL, Om_getResIcon(IDI_BT_REM));
  this->setBmIcon(IDC_BC_PSEDI, Om_getResIcon(IDI_BT_EDI));
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(IDI_BT_DN));

  // Define controls tool-tips
  this->_createTooltip(IDC_LB_PST,    L"Presets list");
  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");
  this->_createTooltip(IDC_BC_PSDEL,  L"Delete Preset");
  this->_createTooltip(IDC_BC_PSNEW,  L"Create new Preset");
  this->_createTooltip(IDC_BC_PSEDI,  L"Preset properties");
  this->_createTooltip(IDC_BC_CKBX1,  L"Disable all warning messages during Presets execution");

  // Set controls default states and parameters
  this->enableItem(IDC_BC_PSDEL, false);
  this->enableItem(IDC_BC_PSEDI, false);

  // Update values
  this->_onTbRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubPst::_onTbResize()
{
  int32_t y_base = 30;

  // Channel list Label
  this->_setItemPos(IDC_SC_LBL01, 50, y_base, 300, 16, true);

  // Channel Actions Buttons
  this->_setItemPos(IDC_BC_PSNEW, 50, y_base+20, 22, 22, true);
  this->_setItemPos(IDC_BC_PSDEL, 50, y_base+42, 22, 22, true);
  this->_setItemPos(IDC_BC_PSEDI, 50, y_base+64, 22, 22, true);

  // Channel list ListBox
  this->_setItemPos(IDC_LB_PST, 75, y_base+21, this->cliWidth()-150, 64, true);

  // Channel list Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliWidth()-73, y_base+20, 22, 22, true);
  this->_setItemPos(IDC_BC_DN, this->cliWidth()-73, y_base+64, 22, 22, true);

  // Options label
  this->_setItemPos(IDC_SC_LBL02, 50, y_base+120, 300, 16, true);
  // Execution in Quiet Mode CheckBox
  this->_setItemPos(IDC_BC_CKBX1, 75, y_base+140, 300, 16, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubPst::_onTbRefresh()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->ModHub();
  if(!ModHub) return;

  this->msgItem(IDC_LB_PST, LB_RESETCONTENT);

  for(unsigned i = 0; i < ModHub->presetCount(); ++i) {
    this->msgItem(IDC_LB_PST, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(ModHub->getPreset(i)->title().c_str()));
    this->msgItem(IDC_LB_PST, LB_SETITEMDATA, i, i); // for Mod Channel index reordering
  }

  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, ModHub->presetQuietMode());

  this->_lb_pst_on_selchg();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropHubPst::_onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_PST: //< Mod Channel(s) list List-Box
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_lb_pst_on_selchg();
      break;

    case IDC_BC_UP: // Down Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_preset_list_up();
      break;

    case IDC_BC_DN: // Up Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_preset_list_dn();
      break;

    case IDC_BC_PSEDI: // "Modify" Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_preset_dialog_prop();
      break;

    case IDC_BC_PSDEL: //< "Remove" Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_preset_delete();
      break;

    case IDC_BC_PSNEW: //< "New" Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_preset_dialog_add();
      break;

    case IDC_BC_CKBX1:
      if(HIWORD(wParam) == BN_CLICKED)
        // notify parameters changes
        this->paramCheck(HUB_PROP_BAT_QUIETMODE);
      break;
    }
  }

  return false;
}

