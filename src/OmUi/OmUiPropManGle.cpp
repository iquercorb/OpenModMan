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
void OmUiPropManGle::_onCkBoxRaw()
{
  // user modified parameter, notify it
  this->paramCheck(MAN_PROP_GLE_NO_MDPARSE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onCkBoxStr()
{
  int bm_chk = this->msgItem(IDC_BC_CKBX2, BM_GETCHECK);

  this->enableItem(IDC_LB_PATH, bm_chk);
  this->enableItem(IDC_BC_BRW01, bm_chk);

  // user modified parameter, notify it
  this->paramCheck(MAN_PROP_GLE_START_LIST);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onLbStrlsSel()
{
  int lb_sel = this->msgItem(IDC_LB_PATH, LB_GETCURSEL);

  if(lb_sel >= 0) {
    this->enableItem(IDC_BC_REM, true);
    this->enableItem(IDC_BC_UP, (lb_sel > 0));
    int lb_max = this->msgItem(IDC_LB_PATH, LB_GETCOUNT) - 1;
    this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
  } else {
    this->enableItem(IDC_BC_REM, false);
    this->enableItem(IDC_BC_UP, false);
    this->enableItem(IDC_BC_DN, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onBcUpStr()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_PATH, LB_GETCURSEL);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OM_MAX_ITEM];
  int idx;

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
void OmUiPropManGle::_onBcDnStr()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_PATH, LB_GETCURSEL);
  // get count of item in List-Box as index to for insertion
  int lb_max = this->msgItem(IDC_LB_PATH, LB_GETCOUNT) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OM_MAX_ITEM];
  int idx;

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
void OmUiPropManGle::_onBcBrwStr()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP01, start);

  if(!Om_dlgOpenFile(result, this->_hwnd, L"Select Mod Hub file", OM_CTX_DEF_FILE_FILER, start))
    return;

  // add file path to startup context list
  this->msgItem(IDC_LB_PATH, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(result.c_str()));

  // user modified parameter, notify it
  this->paramCheck(MAN_PROP_GLE_START_LIST);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onBcRemStr()
{
  int lb_sel = this->msgItem(IDC_LB_PATH, LB_GETCURSEL);

  if(lb_sel >= 0) {
    this->msgItem(IDC_LB_PATH, LB_DELETESTRING, lb_sel);
    // user modified parameter, notify it
    this->paramCheck(MAN_PROP_GLE_START_LIST);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onTabInit()
{
  // Set buttons inner icons
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(this->_hins, IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(this->_hins, IDI_BT_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_CB_ICS,    L"Size of icons in packages lists");

  this->_createTooltip(IDC_BC_CKBX2,  L"Disables Markdown parsing and display descriptions as raw text");
  this->_createTooltip(IDC_BC_CKBX2,  L"Automatically opens Mod Hub files at application startup");
  this->_createTooltip(IDC_LB_PATH,   L"Mod Hub files to be opened at application startup");
  this->_createTooltip(IDC_BC_UP,     L"Move up in list");
  this->_createTooltip(IDC_BC_DN,     L"Move down in list");
  this->_createTooltip(IDC_BC_BRW01,  L"Browse to select a Mod Hub file to add");
  this->_createTooltip(IDC_BC_REM,    L"Remove the selected entry");

  // add items to Icon Size ComboBox
  this->msgItem(IDC_CB_ICS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Small"));
  this->msgItem(IDC_CB_ICS, CB_ADDSTRING, 1, reinterpret_cast<LPARAM>(L"Large"));

  // Update values
  this->_onTabRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onTabResize()
{
  // Icon size Label & ComboBox
  this->_setItemPos(IDC_SC_LBL01, 50, 20, 100, 9);
  this->_setItemPos(IDC_CB_ICS, 50, 30, this->cliUnitX()-100, 14);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_ICS), nullptr, true);

  // No Markdown checkbox
  this->_setItemPos(IDC_BC_CKBX1, 50, 55, 200, 9);

  // Startup Mod Hub list CheckBox & ListBox
  this->_setItemPos(IDC_BC_CKBX2, 50, 79, 100, 9);
  this->_setItemPos(IDC_LB_PATH, 50, 90, this->cliUnitX()-100, this->cliUnitY()-130);

  // Startup Mod Hub list Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliUnitX()-48, 105, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->cliUnitX()-48, 120, 16, 15);

  // Startup Mod Hub list Add and Remove... buttons
  this->_setItemPos(IDC_BC_BRW01, 50, this->cliUnitY()-38, 50, 14);
  this->_setItemPos(IDC_BC_REM, 102, this->cliUnitY()-38, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onTabRefresh()
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

  bool auto_open;
  OmWStringArray path_ls;

  // set Load at Startup CheckBox
  pMgr->loadStartHubs(&auto_open, path_ls);
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, auto_open);

  // Enable or disable Browse button and ListBox
  this->enableItem(IDC_BC_BRW01, auto_open);
  this->enableItem(IDC_LB_PATH, auto_open);

  // Add paths to ListBox
  this->msgItem(IDC_LB_PATH, LB_RESETCONTENT);


  for(size_t i = 0; i < path_ls.size(); ++i) {
    this->msgItem(IDC_LB_PATH, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(path_ls[i].c_str()));
  }

  // Disable the Remove button
  this->enableItem(IDC_BC_REM, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropManGle::_onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_CB_ICS: //< Combo-Box for icons size
      if(HIWORD(wParam) == CBN_SELCHANGE)
        // parameter modified, must be saved we parent dialog valid changes
        this->paramCheck(MAN_PROP_GLE_ICON_SIZE);
      break;

    case IDC_BC_CKBX1: //< CheckBox for Display as raw text
      this->_onCkBoxRaw();
      break;

    case IDC_BC_CKBX2: //< CheckBox for Open Mod Hub(s) at startup
      this->_onCkBoxStr();
      break;

    case IDC_LB_PATH: //< ListBox for startup Mod Hub(s) list
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_onLbStrlsSel();
      break;

    case IDC_BC_UP: //< Up Buttn
      this->_onBcUpStr();
      break;

    case IDC_BC_DN: //< Down Buttn
      this->_onBcDnStr();
      break;

    case IDC_BC_BRW01: //< Startup Mod Hub list "Add.." Button
      this->_onBcBrwStr();
      break;

    case IDC_BC_REM: //< Startup Mod Hub list "Remove" Button
      this->_onBcRemStr();
      break;
    }
  }

  return false;
}
