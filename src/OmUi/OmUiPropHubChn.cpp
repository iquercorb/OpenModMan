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

#include "OmBaseWin.h"
  #include <UxTheme.h>

#include "OmModMan.h"

#include "OmUiPropHub.h"
#include "OmUiMan.h"
#include "OmUiWizChn.h"
#include "OmUiPropChn.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropHubChn.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubChn::OmUiPropHubChn(HINSTANCE hins) : OmDialogPropTab(hins)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubChn::~OmUiPropHubChn()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropHubChn::id() const
{
  return IDD_PROP_HUB_CHN;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_lb_chn_on_selchg()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->ModHub();
  if(!ModHub) return;

  int32_t lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);

  bool has_select = (lb_sel >= 0);

  this->enableItem(IDC_SC_LBL02, has_select);
  this->enableItem(IDC_SC_LBL03, has_select);
  this->enableItem(IDC_SC_LBL04, has_select);

  this->enableItem(IDC_BC_CHDEL, has_select);
  this->enableItem(IDC_BC_CHEDI, has_select);

  int lb_max = this->msgItem(IDC_LB_CHN, LB_GETCOUNT) - 1;
  this->enableItem(IDC_BC_UP, (lb_sel > 0));
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));

  if(!has_select) {
    this->setItemText(IDC_EC_READ2, L"no selection");
    this->setItemText(IDC_EC_READ3, L"no selection");
    this->setItemText(IDC_EC_READ4, L"no selection");
    return;
  }

  // get real channel index from item data
  int32_t chn_id = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);

  OmModChan* ModChan = ModHub->getChannel(chn_id);

  if(ModChan) {
    this->setItemText(IDC_EC_READ2, ModChan->targetPath());
    this->setItemText(IDC_EC_READ3, ModChan->libraryPath());
    this->setItemText(IDC_EC_READ4, ModChan->backupPath());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_channel_list_up()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OM_MAX_ITEM];
  int idx;

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_CHN, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_CHN, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_CHN, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_CHN, LB_SETITEMDATA, lb_sel, idx);

  this->enableItem(IDC_BC_UP, (lb_sel > 1));
  this->enableItem(IDC_BC_DN, true);

  // user modified parameter, notify it
  this->paramCheck(HUB_PROP_CHN_ORDER);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_channel_list_dn()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  // get count of item in List-Box as index to for insertion
  int lb_max = this->msgItem(IDC_LB_CHN, LB_GETCOUNT) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OM_MAX_ITEM];
  int idx;

  this->msgItem(IDC_LB_CHN, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);
  this->msgItem(IDC_LB_CHN, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_CHN, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_CHN, LB_SETITEMDATA, lb_sel, idx);
  this->msgItem(IDC_LB_CHN, LB_SETCURSEL, lb_sel);

  this->enableItem(IDC_BC_UP, true);
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));

  // user modified parameter, notify it
  this->paramCheck(HUB_PROP_CHN_ORDER);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_channel_delete()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->ModHub();
  if(!ModHub) return;

  int32_t lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  if(lb_sel < 0) return;

  int32_t chn_id = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);
  if(chn_id < 0) return;

  // disable all dialog controls
  this->enableItem(IDC_LB_CHN, true);

  this->enableItem(IDC_SC_LBL02, false);
  this->enableItem(IDC_SC_LBL03, false);
  this->enableItem(IDC_SC_LBL04, false);

  this->enableItem(IDC_BC_CHDEL, false);
  this->enableItem(IDC_BC_CHEDI, false);

  static_cast<OmUiMan*>(this->root())->deleteChannel(chn_id);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_channel_dialog_prop()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->ModHub();
  if(!ModHub) return;

  int32_t lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  if(lb_sel < 0) return;

  // get real channel index from item data
  int32_t chn_id = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);

  OmModChan* ModChan = ModHub->getChannel(chn_id);
  if(!ModChan) return;

  // open the Mod Channel Properties dialog
  OmUiPropChn* UiPropChn = static_cast<OmUiPropChn*>(this->root()->childById(IDD_PROP_CHN));

  UiPropChn->setModChan(ModChan);

  UiPropChn->open(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_channel_dialog_add()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->ModHub();
  if(!ModHub) return;

  // open add Mod Channel dialog
  OmUiWizChn* UiWizChn = static_cast<OmUiWizChn*>(this->root()->childById(IDD_WIZ_CHN));

  UiWizChn->setModHub(ModHub);

  UiWizChn->open(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_onTbInit()
{
  // Set buttons inner icons
  this->setBmIcon(IDC_BC_CHADD, Om_getResIcon(IDI_BT_ADD));
  this->setBmIcon(IDC_BC_CHDEL, Om_getResIcon(IDI_BT_REM));
  this->setBmIcon(IDC_BC_CHEDI, Om_getResIcon(IDI_BT_EDI));
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(IDI_BT_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_LB_CHN,    L"Channels list");
  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");
  this->_createTooltip(IDC_BC_CHDEL,  L"Delete Channel");
  this->_createTooltip(IDC_BC_CHADD,  L"Create Channel");
  this->_createTooltip(IDC_BC_CHEDI,  L"Channel properties");

  SetWindowTheme(this->getItem(IDC_LB_CHN),L"EXPLORER",nullptr);

  // Update values
  this->_onTbRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_onTbResize()
{
  int32_t y_base = 30;

  // Channel list Label
  this->_setItemPos(IDC_SC_LBL01, 50, y_base, 300, 16, true);

  // Channel actions Buttons
  this->_setItemPos(IDC_BC_CHADD, 50, y_base+20, 22, 22, true);
  this->_setItemPos(IDC_BC_CHDEL, 50, y_base+42, 22, 22, true);
  this->_setItemPos(IDC_BC_CHEDI, 50, y_base+64, 22, 22, true);

  // Channel ListBox
  this->_setItemPos(IDC_LB_CHN, 75, y_base+21, this->cliWidth()-150, 64, true);

  // Channel Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliWidth()-73, y_base+20, 22, 22, true);
  this->_setItemPos(IDC_BC_DN, this->cliWidth()-73, y_base+64, 22, 22, true);

  // Target path Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 75, y_base+100, 200, 16, true);
  this->_setItemPos(IDC_EC_READ2, 80, y_base+120, this->cliWidth()-80, 21, true);

  // Mods Library Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 75, y_base+150, 200, 16, true);
  this->_setItemPos(IDC_EC_READ3, 80, y_base+170, this->cliWidth()-80, 21, true);

  // Mods Backup Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 75, y_base+200, 200, 16, true);
  this->_setItemPos(IDC_EC_READ4, 80, y_base+220, this->cliWidth()-80, 21, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_onTbRefresh()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->ModHub();
  if(!ModHub) return;

  this->enableItem(IDC_LB_CHN, true);

  this->msgItem(IDC_LB_CHN, LB_RESETCONTENT);

  for(unsigned i = 0; i < ModHub->channelCount(); ++i) {
    this->msgItem(IDC_LB_CHN, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(ModHub->getChannel(i)->title().c_str()));
    this->msgItem(IDC_LB_CHN, LB_SETITEMDATA, i, i); // for Mod Channel index reordering
  }

  this->_lb_chn_on_selchg();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropHubChn::_onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_CHN: //< Mod Channel(s) list List-Box
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_lb_chn_on_selchg();
      break;

    case IDC_BC_UP: //< Up Buttn
      if(HIWORD(wParam) == BN_CLICKED)
        this->_channel_list_up();
      break;

    case IDC_BC_DN: //< Down Buttn
      if(HIWORD(wParam) == BN_CLICKED)
        this->_channel_list_dn();
      break;

    case IDC_BC_CHADD: //< Button : Add
      if(HIWORD(wParam) == BN_CLICKED)
        this->_channel_dialog_add();
      break;

    case IDC_BC_CHDEL: //< Button : Delete
      if(HIWORD(wParam) == BN_CLICKED)
        this->_channel_delete();
      break;

    case IDC_BC_CHEDI: //< Button : Properties
      if(HIWORD(wParam) == BN_CLICKED)
        this->_channel_dialog_prop();
      break;
    }
  }

  return false;
}
