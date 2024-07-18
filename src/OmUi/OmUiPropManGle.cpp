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
#include "OmDialogProp.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropManGle.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropManGle::OmUiPropManGle(HINSTANCE hins) : OmDialogPropTab(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropManGle::~OmUiPropManGle()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropManGle::id() const
{
  return IDD_PROP_MGR_GLE;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_starthub_toggle()
{
  int32_t bm_chk = this->msgItem(IDC_BC_CKBX5, BM_GETCHECK);

  this->enableItem(IDC_LB_PATH, bm_chk);
  this->enableItem(IDC_BC_BRW01, bm_chk);

  // user modified parameter, notify it
  this->paramCheck(MAN_PROP_GLE_START_LIST);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_lb_path_on_selchg()
{
  int32_t lb_sel = this->msgItem(IDC_LB_PATH, LB_GETCURSEL);

  this->enableItem(IDC_BC_DEL, (lb_sel >= 0));

  int32_t lb_max = this->msgItem(IDC_LB_PATH, LB_GETCOUNT) - 1;
  this->enableItem(IDC_BC_UP, (lb_sel > 0));
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_starthub_list_up()
{
  // get selected item (index)
  int32_t lb_sel = this->msgItem(IDC_LB_PATH, LB_GETCURSEL);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OM_MAX_ITEM];
  int32_t idx;

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_PATH, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_PATH, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_PATH, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_PATH, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_PATH, LB_SETITEMDATA, lb_sel, idx);

  this->enableItem(IDC_BC_UP, (lb_sel > 1));
  this->enableItem(IDC_BC_DN, true);

  // user modified parameter, notify it
  this->paramCheck(MAN_PROP_GLE_START_LIST);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_starthub_list_dn()
{
  // get selected item (index)
  int32_t lb_sel = this->msgItem(IDC_LB_PATH, LB_GETCURSEL);
  // get count of item in List-Box as index to for insertion
  int32_t lb_max = this->msgItem(IDC_LB_PATH, LB_GETCOUNT) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OM_MAX_ITEM];
  int32_t idx;

  this->msgItem(IDC_LB_PATH, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_PATH, LB_GETITEMDATA, lb_sel);
  this->msgItem(IDC_LB_PATH, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_PATH, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_PATH, LB_SETITEMDATA, lb_sel, idx);
  this->msgItem(IDC_LB_PATH, LB_SETCURSEL, lb_sel);

  this->enableItem(IDC_BC_UP, true);
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));

  // user modified parameter, notify it
  this->paramCheck(MAN_PROP_GLE_START_LIST);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_browse_hub_file()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP01, start);

  if(!Om_dlgOpenFile(result, this->_hwnd, L"Select Mod Hub file", OM_HUB_FILES_FILTER, start))
    return;

  // add file path to startup context list
  this->msgItem(IDC_LB_PATH, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(result.c_str()));

  // user modified parameter, notify it
  this->paramCheck(MAN_PROP_GLE_START_LIST);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_starthub_delete()
{
  int32_t lb_sel = this->msgItem(IDC_LB_PATH, LB_GETCURSEL);
  if(lb_sel < 0) return;

  this->msgItem(IDC_LB_PATH, LB_DELETESTRING, lb_sel);

  // user modified parameter, notify it
  this->paramCheck(MAN_PROP_GLE_START_LIST);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onTbInit()
{
  // Set buttons inner icons
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(IDI_BT_DN));

  this->setBmIcon(IDC_BC_BRW01, Om_getResIcon(IDI_BT_OPN));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(IDI_BT_REM));

  // define controls tool-tips
  this->_createTooltip(IDC_CB_ICS,    L"Size of icons in interface List Views");

  this->_createTooltip(IDC_BC_CKBX1,  L"Disables Markdown parsing and display Mod overview as raw text");
  this->_createTooltip(IDC_BC_CKBX2,  L"Show confirmation dialog before opening external links from Mod overview");

  this->_createTooltip(IDC_BC_CKBX5,  L"Automatically opens Mod Hub files at application startup");
  this->_createTooltip(IDC_LB_PATH,   L"Path to Mod Hub files");
  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");
  this->_createTooltip(IDC_BC_BRW01,  L"Select a Mod Hub file");
  this->_createTooltip(IDC_BC_DEL,    L"Delete entry");

  // add items to Icon Size ComboBox
  this->msgItem(IDC_CB_ICS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal"));
  this->msgItem(IDC_CB_ICS, CB_ADDSTRING, 1, reinterpret_cast<LPARAM>(L"Large"));

  // Update values
  this->_onTbRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onTbResize()
{
  int32_t y_base = 30;

  // Icon size Label & ComboBox
  this->_setItemPos(IDC_SC_LBL01, 50, y_base, 300, 16, true);
  this->_setItemPos(IDC_CB_ICS, 50, y_base+20, this->cliWidth()-100, 21, true);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_ICS), nullptr, true);

  // Overview options label
  this->_setItemPos(IDC_SC_LBL02, 50, y_base+70, 300, 16, true);
  // No Markdown checkbox
  this->_setItemPos(IDC_BC_CKBX1, 75, y_base+90, 300, 16, true);
  // Open Link warning check box
  this->_setItemPos(IDC_BC_CKBX2, 75, y_base+110, 300, 16, true);

  // Startup Mod Hub list Actions buttons
  this->_setItemPos(IDC_BC_BRW01, 50, y_base+170, 22, 22, true);
  this->_setItemPos(IDC_BC_DEL, 50, y_base+193, 22, 22, true);

  // Startup Mod Hub list CheckBox & ListBox
  this->_setItemPos(IDC_BC_CKBX5, 50, y_base+150, 300, 16, true);
  this->_setItemPos(IDC_LB_PATH, 75, y_base+171, this->cliWidth()-150, 64, true);

  // Startup Mod Hub list Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliWidth()-73, y_base+170, 22, 22, true);
  this->_setItemPos(IDC_BC_DN, this->cliWidth()-73, y_base+214, 22, 22, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onTbRefresh()
{
  OmModMan* pMgr = static_cast<OmModMan*>(this->_data);

  switch(pMgr->iconsSize()) {
  case 16:
    this->msgItem(IDC_CB_ICS, CB_SETCURSEL, 0, 0);
    break;
  case 32:
    this->msgItem(IDC_CB_ICS, CB_SETCURSEL, 2, 0);
    break;
  default:
    this->msgItem(IDC_CB_ICS, CB_SETCURSEL, 1, 0);
    break;
  }

  // set No Markdown CheckBox
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, pMgr->noMarkdown());

  // set Confirm link open
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, pMgr->linkConfirm());

  bool auto_open;
  OmWStringArray path_ls;

  // set Load at Startup CheckBox
  pMgr->getStartHubs(&auto_open, path_ls);
  this->msgItem(IDC_BC_CKBX5, BM_SETCHECK, auto_open);

  // Enable or disable Browse button and ListBox
  this->enableItem(IDC_BC_BRW01, auto_open);
  this->enableItem(IDC_LB_PATH, auto_open);

  // Add paths to ListBox
  this->msgItem(IDC_LB_PATH, LB_RESETCONTENT);


  for(size_t i = 0; i < path_ls.size(); ++i) {
    this->msgItem(IDC_LB_PATH, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(path_ls[i].c_str()));
  }

  // Disable the Remove button
  this->enableItem(IDC_BC_DEL, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropManGle::_onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_CB_ICS: //< Combo-Box for icons size
      if(HIWORD(wParam) == CBN_SELCHANGE)
        // parameter modified, must be saved we parent dialog valid changes
        this->paramCheck(MAN_PROP_GLE_ICON_SIZE);
      break;

    case IDC_BC_CKBX1: //< CheckBox: Disable Markdown parsing
      if(HIWORD(wParam) == BN_CLICKED)
        // notify parameter changes
        this->paramCheck(MAN_PROP_GLE_NO_MDPARSE);
      break;

    case IDC_BC_CKBX2: //< CheckBox: Ask Confirmation to open links
      if(HIWORD(wParam) == BN_CLICKED)
        // notify parameter changes
        this->paramCheck(MAN_PROP_GLE_LINK_CONFIRM);
      break;

    case IDC_BC_CKBX5: //< CheckBox for Open Mod Hub(s) at startup
      if(HIWORD(wParam) == BN_CLICKED)
        this->_starthub_toggle();
      break;

    case IDC_LB_PATH: //< ListBox for startup Mod Hub(s) list
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_lb_path_on_selchg();
      break;

    case IDC_BC_UP: //< Up Buttn
      if(HIWORD(wParam) == BN_CLICKED)
        this->_starthub_list_up();
      break;

    case IDC_BC_DN: //< Down Buttn
      if(HIWORD(wParam) == BN_CLICKED)
        this->_starthub_list_dn();
      break;

    case IDC_BC_BRW01: //< Button: Browse to select Mod Hub
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_hub_file();
      break;

    case IDC_BC_DEL:  //< Startup Mod Hub list "Remove" Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_starthub_delete();
      break;
    }
  }

  return false;
}
