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
void OmUiPropPstLst::_buildLbs()
{
  // reset List-Box control
  this->msgItem(IDC_LB_EXC, LB_RESETCONTENT);

  OmModPset* ModPset = static_cast<OmUiPropPst*>(this->_parent)->ModPset();
  if(!ModPset)
    return;

  OmModHub* ModHub = ModPset->ModHub();
  if(!ModHub)
    return;

  // get current ComboBox selection first Mod Channel by default
  int cb_sel = this->msgItem(IDC_CB_CHN, CB_GETCURSEL);
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
void OmUiPropPstLst::_includePkg()
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
void OmUiPropPstLst::_excludePkg()
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
void OmUiPropPstLst::_onLbExclsSel()
{
  int32_t sel_cnt = this->msgItem(IDC_LB_EXC, LB_GETSELCOUNT);

  if(sel_cnt > 0) {
    // unselect all from the other ListBox, this is less confusing
    this->msgItem(IDC_LB_INC, LB_SETSEL, false, -1);

    this->enableItem(IDC_BC_RIGH, true);
    this->enableItem(IDC_BC_LEFT, false);
    this->enableItem(IDC_BC_UP, false);
    this->enableItem(IDC_BC_DN, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_onLbInclsSel()
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
void OmUiPropPstLst::_onBcUpPkg()
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
void OmUiPropPstLst::_onBcDnPkg()
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
void OmUiPropPstLst::_onTabInit()
{
  // Set icons for Up and Down buttons
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(this->_hins, IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(this->_hins, IDI_BT_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_CB_CHN,    L"Channel to configure");
  this->_createTooltip(IDC_LB_INC,    L"Packages the Script will install (or leave installed)");
  this->_createTooltip(IDC_LB_EXC,    L"Packages the Script will uninstall (or leave uninstalled)");
  this->_createTooltip(IDC_BC_RIGH,   L"Add to installed");
  this->_createTooltip(IDC_BC_LEFT,   L"Remove from installed");
  this->_createTooltip(IDC_BC_UP,     L"Move up in list");
  this->_createTooltip(IDC_BC_DN,     L"Move down in list");

  this->_onTabRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_onTabResize()
{
  int half_w = this->cliUnitX() * 0.5f;
  int half_h = this->cliUnitY() * 0.5f;

  // Batch Configuration Label
  this->_setItemPos(IDC_SC_LBL02, 50, 25, 240, 9);
  // Mod Channel list ComboBox
  this->_setItemPos(IDC_CB_CHN, 50, 35, this->cliUnitX()-100, 12);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_CHN), nullptr, true);

  // Not-Installed label
  this->_setItemPos(IDC_SC_LBL03, 50, 55, 150, 9);
  // Not-Installed ListBox
  this->_setItemPos(IDC_LB_EXC, 50, 65, half_w-70, this->cliUnitY()-95);
  // Add and Rem buttons
  this->_setItemPos(IDC_BC_RIGH, half_w-18, half_h, 16, 15);
  this->_setItemPos(IDC_BC_LEFT, half_w-18, half_h+15, 16, 15);

  // Installed label
  this->_setItemPos(IDC_SC_LBL04, half_w, 55, 150, 9);
  // Installed ListBox
  this->_setItemPos(IDC_LB_INC, half_w, 65, half_w-70, this->cliUnitY()-95);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliUnitX()-68, half_h, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->cliUnitX()-68, half_h+15, 16, 15);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPstLst::_onTabRefresh()
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
    cb_entry += L" [";
    cb_entry += ModChan->home();
    cb_entry += L"]";

    this->msgItem(IDC_CB_CHN, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(cb_entry.c_str()));

    // initialize new lists for this Mod Channel
    this->_excluded.push_back(std::vector<int>());
    this->_included.push_back(std::vector<int>());

    // get batch install list Packages
    OmPModPackArray mod_ls;
    ModPset->getSetupEntryList(ModChan, &mod_ls);

    if(mod_ls.size()) {

      // fill the include list ordered as in the batch
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

  // Disable ComboBox and ListBoxes
  this->enableItem(IDC_CB_CHN, true);
  this->enableItem(IDC_LB_EXC, true);
  this->enableItem(IDC_LB_INC, true);

  // fill up each ListBox
  this->_buildLbs();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropPstLst::_onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {

    case IDC_CB_CHN:  //< Mod Channel ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->_buildLbs();
      break;

    case IDC_LB_EXC: // Uninstall (Exclude) ListBox
      if(HIWORD(wParam) == LBN_SELCHANGE) this->_onLbExclsSel();
      if(HIWORD(wParam) == LBN_DBLCLK) this->_includePkg();
      break;

    case IDC_LB_INC: // Install (Include) ListBox
      if(HIWORD(wParam) == LBN_SELCHANGE) this->_onLbInclsSel();
      if(HIWORD(wParam) == LBN_DBLCLK) this->_excludePkg();
      break;

    case IDC_BC_RIGH: //< ">" Button
      this->_includePkg();
      break;

    case IDC_BC_LEFT: //< "<" Button
      this->_excludePkg();
      break;

    case IDC_BC_UP: //< Up Button
      this->_onBcUpPkg();
      break;

    case IDC_BC_DN: //< Down Button
      this->_onBcDnPkg();
      break;
    }
  }

  return false;
}
