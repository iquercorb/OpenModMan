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
#include "OmModPack.h"
#include "OmModPset.h"
#include "OmDialogProp.h"

#include "OmUiPropPst.h"

#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropPstLst.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPstLst::OmUiPropPstLst(HINSTANCE hins) : OmDialogPropTab(hins)
{

}

OmUiPropPstLst::~OmUiPropPstLst()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropPstLst::id() const
{
  return IDD_PROP_PST_LST;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_lb_populate()
{
  // reset List-Box control
  this->msgItem(IDC_LB_EXC, LB_RESETCONTENT);

  OmModPset* ModPset = static_cast<OmUiPropPst*>(this->_parent)->ModPset();
  if(!ModPset) return;

  OmModHub* ModHub = ModPset->ModHub();
  if(!ModHub) return;

  // get current ComboBox selection first Mod Channel by default
  int32_t cb_sel = this->msgItem(IDC_CB_CHN, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get Mod Channel corresponding to current selection
  OmModChan* ModChan = ModHub->getChannel(cb_sel);

  // fill the left List-Box
  OmWString lb_entry;

  for(size_t i = 0; i < this->_excluded[cb_sel].size(); i++) {

    int32_t p = this->_excluded[cb_sel][i];
    OmModPack* ModPack = ModChan->getModpack(p);

    lb_entry = Om_getFilePart(ModPack->sourcePath());
    this->msgItem(IDC_LB_EXC, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(lb_entry.c_str()));
    this->msgItem(IDC_LB_EXC, LB_SETITEMDATA, i, p);
  }

  // reset List-Box control
  this->msgItem(IDC_LB_INC, LB_RESETCONTENT);

  // fill the right List-Box
  for(size_t i = 0; i < this->_included[cb_sel].size(); i++) {

    int32_t p = this->_included[cb_sel][i];
    OmModPack* ModPack = ModChan->getModpack(p);

    lb_entry = Om_getFilePart(ModPack->sourcePath());
    this->msgItem(IDC_LB_INC, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(lb_entry.c_str()));
    this->msgItem(IDC_LB_INC, LB_SETITEMDATA, i, p);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_install_include()
{
  // get current ComboBox selection first Mod Channel by default
  int32_t cb_sel = this->msgItem(IDC_CB_CHN, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int32_t lb_sel_count = this->msgItem(IDC_LB_EXC, LB_GETSELCOUNT);
  if(lb_sel_count <= 0) return;

  // get list of selected items (index)
  int32_t* lb_sel = new(std::nothrow) int32_t[lb_sel_count];
  if(!lb_sel)
    return;

  this->msgItem(IDC_LB_EXC, LB_GETSELITEMS, lb_sel_count, reinterpret_cast<LPARAM>(lb_sel));

  // copy selected items from one list to the other list
  OmWString lb_entry;

  for(int32_t i = 0; i < lb_sel_count; ++i) {

    // retrieve the package ListBox label
    this->getLbText(IDC_LB_EXC, lb_sel[i], lb_entry);

    // retrieve the package reference index (in Mod Channel package list)
    int32_t index = this->msgItem(IDC_LB_EXC, LB_GETITEMDATA, lb_sel[i]);

    // remove package index from left mirror list
    for(size_t k = 0; k < this->_excluded[cb_sel].size(); ++k) {
      if(this->_excluded[cb_sel][k] == index) {
        this->_excluded[cb_sel].erase(this->_excluded[cb_sel].begin()+k);
        break;
      }
    }
    // add package index to right mirror list
    this->_included[cb_sel].push_back(index);

    // get count of item in List-Box as index to for insertion
    int32_t pos = this->msgItem(IDC_LB_INC, LB_GETCOUNT);
    // add item to the List-Box
    this->msgItem(IDC_LB_INC, LB_ADDSTRING, pos, reinterpret_cast<LPARAM>(lb_entry.c_str()));
    this->msgItem(IDC_LB_INC, LB_SETITEMDATA, pos, index);
  }

  // remove items from List-Box in reverse order to prevent indexing issues
  while(lb_sel_count--)
    this->msgItem(IDC_LB_EXC, LB_DELETESTRING, lb_sel[lb_sel_count]);

  // we do not need list-box selection anymore
  delete [] lb_sel;

  // disable button until new selection
  this->enableItem(IDC_BC_RIGH, false);

  // user modified parameter, notify it
  this->paramCheck(PST_PROP_STG_INSLS);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_install_exclude()
{
  // get current ComboBox selection first Mod Channel by default
  int32_t cb_sel = this->msgItem(IDC_CB_CHN, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int32_t lb_sel_count = this->msgItem(IDC_LB_INC, LB_GETSELCOUNT);
  if(lb_sel_count <= 0) return;

  // get list of selected items (index)
  int32_t* lb_sel = new(std::nothrow) int32_t[lb_sel_count];
  if(!lb_sel)
    return;

  this->msgItem(IDC_LB_INC, LB_GETSELITEMS, lb_sel_count, reinterpret_cast<LPARAM>(lb_sel));


  // copy selected items from one list to the other list
  OmWString lb_entry;

  for(int32_t i = 0; i < lb_sel_count; ++i) {

    // retrieve the package List-Box label
    this->getLbText(IDC_LB_INC, lb_sel[i], lb_entry);

    // retrieve the package reference index (in Mod Channel package list)
    int32_t index = this->msgItem(IDC_LB_INC, LB_GETITEMDATA, lb_sel[i]);

    // remove package index from right mirror list
    for(size_t k = 0; k < this->_included[cb_sel].size(); ++k) {
      if(this->_included[cb_sel][k] == index) {
        this->_included[cb_sel].erase(this->_included[cb_sel].begin()+k);
        break;
      }
    }
    // add package index to left mirror list
    this->_excluded[cb_sel].push_back(index);

    // get count of item in List-Box as index to for insertion
    int32_t pos = this->msgItem(IDC_LB_EXC, LB_GETCOUNT);
    // add item to the List-Box
    this->msgItem(IDC_LB_EXC, LB_ADDSTRING, pos, reinterpret_cast<LPARAM>(lb_entry.c_str()));
    this->msgItem(IDC_LB_EXC, LB_SETITEMDATA, pos, index);
  }

  // remove items from List-Box in reverse order to prevent indexing issues
  while(lb_sel_count--)
    this->msgItem(IDC_LB_INC, LB_DELETESTRING, lb_sel[lb_sel_count]);

  // we do not need list-box selection anymore
  delete [] lb_sel;

  // disable button until new selection
  this->enableItem(IDC_BC_LEFT, false);
  this->enableItem(IDC_BC_UP, false);
  this->enableItem(IDC_BC_DN, false);

  // user modified parameter, notify it
  this->paramCheck(PST_PROP_STG_INSLS);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_lb_exc_on_selchg()
{
  int32_t sel_cnt = this->msgItem(IDC_LB_EXC, LB_GETSELCOUNT);
  bool has_select = (sel_cnt > 0);

  // unselect all from the other ListBox, this is less confusing
  this->msgItem(IDC_LB_INC, LB_SETSEL, !has_select, -1);

  this->enableItem(IDC_BC_RIGH, has_select);
  this->enableItem(IDC_BC_LEFT, !has_select);
  this->enableItem(IDC_BC_UP, !has_select);
  this->enableItem(IDC_BC_DN, !has_select);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_lb_inc_on_selchg()
{
  int32_t sel_cnt = this->msgItem(IDC_LB_INC, LB_GETSELCOUNT);

  if(sel_cnt > 0) {
    // unselect all from the other ListBox, this is less confusing
    this->msgItem(IDC_LB_EXC, LB_SETSEL, false, -1);
    this->enableItem(IDC_BC_RIGH, false);
    this->enableItem(IDC_BC_LEFT, true);
  }

  if(sel_cnt == 1) {

    int32_t lb_sel;
    this->msgItem(IDC_LB_INC, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));
    int32_t lb_max = this->msgItem(IDC_LB_INC, LB_GETCOUNT) - 1;

    this->enableItem(IDC_BC_UP, (lb_sel > 0));
    this->enableItem(IDC_BC_DN, (lb_sel < lb_max));

  } else {

    this->enableItem(IDC_BC_UP, false);
    this->enableItem(IDC_BC_DN, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_install_list_up()
{
  // get current ComboBox selection first Mod Channel by default
  int32_t cb_sel = this->msgItem(IDC_CB_CHN, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int32_t sel_cnt = this->msgItem(IDC_LB_INC, LB_GETSELCOUNT);
  if(sel_cnt != 1) return;

  // get list of selected item (index)
  int32_t lb_sel;
  this->msgItem(IDC_LB_INC, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));

  // check whether we can move up
  if(lb_sel == 0) return;

  wchar_t item_buf[OM_MAX_ITEM];

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_INC, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  int32_t index = this->msgItem(IDC_LB_INC, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_INC, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_INC, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_INC, LB_SETITEMDATA, lb_sel, index);

  // swap package index to move up
  for(size_t k = 0; k < this->_included[cb_sel].size(); ++k) {
    if(this->_included[cb_sel][k] == index) {
      int32_t temp = this->_included[cb_sel][k];
      this->_included[cb_sel][k] = this->_included[cb_sel][k+1];
      this->_included[cb_sel][k+1] = temp;
      break;
    }
  }

  this->enableItem(IDC_BC_UP, (lb_sel > 1));
  this->enableItem(IDC_BC_DN, true);

  // user modified parameter, notify it
  this->paramCheck(PST_PROP_STG_INSLS);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_install_list_dn()
{
  // get current ComboBox selection first Mod Channel by default
  int32_t cb_sel = this->msgItem(IDC_CB_CHN, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int32_t sel_cnt = this->msgItem(IDC_LB_INC, LB_GETSELCOUNT);
  if(sel_cnt != 1) return;

  // get count of item in List-Box as index to for insertion
  int32_t lb_max = this->msgItem(IDC_LB_INC, LB_GETCOUNT) - 1;

  // get list of selected item (index)
  int32_t lb_sel;
  this->msgItem(IDC_LB_INC, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));

  // check whether we can move down
  if(lb_sel == lb_max) return;

  wchar_t item_buf[OM_MAX_ITEM];

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_INC, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  int32_t index = this->msgItem(IDC_LB_INC, LB_GETITEMDATA, lb_sel);

  this->msgItem(IDC_LB_INC, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_INC, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_INC, LB_SETITEMDATA, lb_sel, index);
  this->msgItem(IDC_LB_INC, LB_SETSEL, true, lb_sel);

  // swap package index to move up
  for(size_t k = 0; k < this->_included[cb_sel].size(); ++k) {
    if(this->_included[cb_sel][k] == index) {
      int32_t temp = this->_included[cb_sel][k];
      this->_included[cb_sel][k] = this->_included[cb_sel][k+1];
      this->_included[cb_sel][k+1] = temp;
      break;
    }
  }

  this->enableItem(IDC_BC_UP, true);
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));

  // user modified parameter, notify it
  this->paramCheck(PST_PROP_STG_INSLS);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_onTbInit()
{
  // Set icons for Up and Down buttons
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(IDI_BT_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_CB_CHN,    L"Channel to configure");
  this->_createTooltip(IDC_LB_INC,    L"Mods the Preset will install (or leave installed)");
  this->_createTooltip(IDC_LB_EXC,    L"Mods the Preset will uninstall (or leave uninstalled)");
  this->_createTooltip(IDC_BC_RIGH,   L"Include to be installed");
  this->_createTooltip(IDC_BC_LEFT,   L"Set to be uninstalled (or ignored)");
  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");

  this->_onTbRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_onTbResize()
{
  int32_t y_base = 30;

  int32_t half_w = this->cliWidth() * 0.5f;
  int32_t half_h = this->cliHeight() * 0.5f;

  // Preset Configuration Label
  this->_setItemPos(IDC_SC_LBL02, 50, y_base, 240, 16, true);
  // Mod Channel list ComboBox
  this->_setItemPos(IDC_CB_CHN, 50, y_base+20, this->cliWidth()-100, 21, true);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_CHN), nullptr, true);

  // Not-Installed label
  this->_setItemPos(IDC_SC_LBL03, 50, y_base+50, 150, 16, true);
  // Not-Installed ListBox
  this->_setItemPos(IDC_LB_EXC, 50, y_base+70, half_w-75, this->cliHeight()-140, true);
  // Add and Rem buttons
  this->_setItemPos(IDC_BC_RIGH, half_w-23, half_h, 22, 22, true);
  this->_setItemPos(IDC_BC_LEFT, half_w-23, half_h+23, 22, 22, true);

  // Installed label
  this->_setItemPos(IDC_SC_LBL04, half_w+1, y_base+50, 150, 16, true);
  // Installed ListBox
  this->_setItemPos(IDC_LB_INC, half_w+1, y_base+70, half_w-75, this->cliHeight()-140, true);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliWidth()-73, half_h, 22, 22, true);
  this->_setItemPos(IDC_BC_DN, this->cliWidth()-73, half_h+23, 22, 22, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_onTbRefresh()
{
  OmModPset* ModPset = static_cast<OmUiPropPst*>(this->_parent)->ModPset();
  if(!ModPset)
    return;

  OmModHub* ModHub = ModPset->ModHub();
  if(!ModHub)
    return;

  // empty the ComboBox
  this->msgItem(IDC_CB_CHN, CB_RESETCONTENT);

  this->_excluded.clear();
  this->_included.clear();

  // add Mod Channel(s) to Combo-Box
  OmWString cb_entry;

  for(unsigned i = 0; i < ModHub->channelCount(); ++i) {

    OmModChan* ModChan = ModHub->getChannel(i);

    cb_entry = ModChan->title();
    /*
    cb_entry += L" [";
    cb_entry += ModChan->home();
    cb_entry += L"]";
    */
    this->msgItem(IDC_CB_CHN, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(cb_entry.c_str()));

    // initialize new lists for this Mod Channel
    this->_excluded.push_back(std::vector<int>());
    this->_included.push_back(std::vector<int>());

    // get preset install list of Mods
    OmPModPackArray mod_ls;
    ModPset->getSetupEntryList(ModChan, &mod_ls);

    if(mod_ls.size()) {

      // fill the include list ordered as in the preset
      for(size_t j = 0; j < mod_ls.size(); ++j) {
        int32_t p = ModChan->indexOfModpack(mod_ls[j]);
        if(p >= 0) this->_included.back().push_back(p);
      }

      // fill the exclude list
      for(size_t j = 0; j < ModChan->modpackCount(); ++j) {
        if(std::find(mod_ls.begin(), mod_ls.end(), ModChan->getModpack(j)) == mod_ls.end())
          this->_excluded.back().push_back(j);
      }

    } else {

      // fill the exclude list
      for(size_t j = 0; j < ModChan->modpackCount(); ++j)
        this->_excluded.back().push_back(j);
    }
  }

  // Select first Mod Channel by default
  this->msgItem(IDC_CB_CHN, CB_SETCURSEL, 0);

  // enable or disable ComboBox and ListBoxes
  this->enableItem(IDC_CB_CHN, !ModPset->locked());
  this->enableItem(IDC_LB_EXC, !ModPset->locked());
  this->enableItem(IDC_LB_INC, !ModPset->locked());

  // fill up each ListBox
  this->_lb_populate();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropPstLst::_onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {

    case IDC_CB_CHN:  //< Mod Channel ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->_lb_populate();
      break;

    case IDC_LB_EXC: // Uninstall (Exclude) ListBox

      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_lb_exc_on_selchg();

      if(HIWORD(wParam) == LBN_DBLCLK)
        this->_install_include();

      break;

    case IDC_LB_INC: // Install (Include) ListBox

      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_lb_inc_on_selchg();

      if(HIWORD(wParam) == LBN_DBLCLK)
        this->_install_exclude();

      break;

    case IDC_BC_RIGH: //< ">" Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_install_include();
      break;

    case IDC_BC_LEFT: //< "<" Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_install_exclude();
      break;

    case IDC_BC_UP: //< Up Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_install_list_up();
      break;

    case IDC_BC_DN: //< Down Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_install_list_dn();
      break;
    }
  }

  return false;
}
