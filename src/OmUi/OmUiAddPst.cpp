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
#include "OmModPack.h"
#include "OmModPset.h"

#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiAddPst.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiAddPst::OmUiAddPst(HINSTANCE hins) : OmDialog(hins), _ModHub(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiAddPst::~OmUiAddPst()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiAddPst::id() const
{
  return IDD_ADD_PST;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddPst::_buildLbs()
{
  if(!this->_ModHub)
    return;

  // get current Combo-Box selection first Mod Channel by default
  int cb_sel = this->msgItem(IDC_CB_CHN, CB_GETCURSEL);
  if(cb_sel < 0)
    return;

  // get Mod Channel corresponding to current selection
  OmModChan* ModChan = this->_ModHub->getChannel(cb_sel);

  // reset List-Box control
  this->msgItem(IDC_LB_EXC, LB_RESETCONTENT);

  OmWString lb_entry;

  // fill the left List-Box
  for(size_t i = 0; i < this->_excluded[cb_sel].size(); i++) {

    int32_t p = this->_excluded[cb_sel][i];
    OmModPack* ModPack = ModChan->getModpack(p);

    lb_entry = Om_getFilePart(ModPack->sourcePath());
    this->msgItem(IDC_LB_EXC, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(lb_entry.c_str()));
    this->msgItem(IDC_LB_EXC, LB_SETITEMDATA, i, p);
  }

  // reset List-Box control
  this->msgItem(IDC_LB_INC, LB_RESETCONTENT);

  // fill the left List-Box
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
void OmUiAddPst::_autoInclude()
{
  if(!this->_ModHub)
    return;

  // add Mod Channel(s) to Combo-Box
  for(size_t k = 0; k < this->_ModHub->channelCount(); ++k) {

    OmModChan* ModChan = this->_ModHub->getChannel(k);

    this->_excluded[k].clear();
    this->_included[k].clear();

    for(size_t i = 0; i < ModChan->modpackCount(); ++i) {

      OmModPack* ModPack = ModChan->getModpack(i);

      if(ModPack->hasSource()) {
        if(ModPack->hasBackup()) {
          this->_included[k].push_back(i);
        } else {
          this->_excluded[k].push_back(i);
        }
      }
    }
  }

  // refill the List-Box controls with new values
  this->_buildLbs();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddPst::_includePkg()
{
  // get current Combo-Box selection first Mod Channel by default
  int32_t cb_sel = this->msgItem(IDC_CB_CHN, CB_GETCURSEL);
  if(cb_sel < 0)
    return;

  // get count of selected items
  int32_t lb_sel_count = this->msgItem(IDC_LB_EXC, LB_GETSELCOUNT);
  if(lb_sel_count <= 0)
    return;

  // get list of selected items (index)
  int32_t* lb_sel = new(std::nothrow) int32_t[lb_sel_count];
  if(!lb_sel)
    return;

  this->msgItem(IDC_LB_EXC, LB_GETSELITEMS, lb_sel_count, reinterpret_cast<LPARAM>(lb_sel));

  OmWString lb_entry;

  // copy selected items from one list to the other list
  for(int32_t i = 0; i < lb_sel_count; ++i) {

    // retrieve the Mod List-Box label
    this->getLbText(IDC_LB_EXC, lb_sel[i], lb_entry);

    // retrieve the Mod reference index (in Mod Channel Mod list)
    int32_t index = this->msgItem(IDC_LB_EXC, LB_GETITEMDATA, lb_sel[i]);

    // remove Mod index from left mirror list
    for(size_t k = 0; k < this->_excluded[cb_sel].size(); ++k) {
      if(this->_excluded[cb_sel][k] == index) {
        this->_excluded[cb_sel].erase(this->_excluded[cb_sel].begin()+k);
        break;
      }
    }
    // add Mod index to right mirror list
    this->_included[cb_sel].push_back(index);

    // get count of item in List-Box as index to for insertion
    int32_t pos = this->msgItem(IDC_LB_INC, LB_GETCOUNT);
    // add item to the List-Box
    this->msgItem(IDC_LB_INC, LB_ADDSTRING, pos, reinterpret_cast<LPARAM>(lb_entry.c_str()));
    this->msgItem(IDC_LB_INC, LB_SETITEMDATA, pos, index);
  }

  // remove items from List-Box in reverse order to avoid indexing issues
  while(lb_sel_count--)
    this->msgItem(IDC_LB_EXC, LB_DELETESTRING, lb_sel[lb_sel_count]);

  // we do not need list-box selection anymore
  delete [] lb_sel;

  // disable button until new selection
  this->enableItem(IDC_BC_RIGH, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddPst::_excludePkg()
{
  // get current Combo-Box selection first Mod Channel by default
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

  OmWString lb_entry;

  // copy selected items from one list to the other list
  for(int32_t i = 0; i < lb_sel_count; ++i) {

    // retrieve the Mod List-Box label
    this->getLbText(IDC_LB_INC, lb_sel[i], lb_entry);

    // retrieve the Mod reference index (in Mod Channel Mod list)
    int32_t index = this->msgItem(IDC_LB_INC, LB_GETITEMDATA, lb_sel[i]);

    // remove Mod index from right mirror list
    for(size_t k = 0; k < this->_included[cb_sel].size(); ++k) {
      if(this->_included[cb_sel][k] == index) {
        this->_included[cb_sel].erase(this->_included[cb_sel].begin()+k);
        break;
      }
    }

    // add Mod index to left mirror list
    this->_excluded[cb_sel].push_back(index);

    // get count of item in List-Box as index to for insertion
    int32_t pos = this->msgItem(IDC_LB_EXC, LB_GETCOUNT);
    // add item to the List-Box
    this->msgItem(IDC_LB_EXC, LB_ADDSTRING, pos, reinterpret_cast<LPARAM>(lb_entry.c_str()));
    this->msgItem(IDC_LB_EXC, LB_SETITEMDATA, pos, index);
  }

  // remove items from List-Box in reverse order to prevent indexing issues
  while(lb_sel_count--) {
    this->msgItem(IDC_LB_INC, LB_DELETESTRING, lb_sel[lb_sel_count]);
  }

  // we do not need list-box selection anymore
  delete [] lb_sel;

  // disable button until new selection
  this->enableItem(IDC_BC_LEFT, false);
  this->enableItem(IDC_BC_UP, false);
  this->enableItem(IDC_BC_DN, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddPst::_onCkBoxAuto()
{
  int32_t bm_chk = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_CB_CHN, !bm_chk);
  this->enableItem(IDC_LB_EXC, !bm_chk);
  this->enableItem(IDC_LB_INC, !bm_chk);

  if(bm_chk)
    this->_autoInclude(); //< initialize with current state
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddPst::_onLbExclsSel()
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
void OmUiAddPst::_onLbInclsSel()
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
void OmUiAddPst::_onBcUpPkg()
{
  // get current Combo-Box selection first Mod Channel by default
  int32_t cb_sel = this->msgItem(IDC_CB_CHN, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int32_t sel_cnt = this->msgItem(IDC_LB_INC, LB_GETSELCOUNT);
  if(sel_cnt != 1) return;

  // get list of selected item (index)
  int32_t lb_sel;
  this->msgItem(IDC_LB_INC, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));
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
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddPst::_onBcDnPkg()
{
  // get current Combo-Box selection first Mod Channel by default
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
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiAddPst::_onBcOk()
{
  // retrieve Preset name
  OmWString input;
  this->getItemText(IDC_EC_INP01, input);

  // initialize a new Mod Preset
  OmModPset* ModPset = this->_ModHub->createPreset(input);

  // setup batch per Mod Channel install list
  for(size_t l = 0; l < this->_ModHub->channelCount(); ++l) {

    // ensure we got proper location
    OmModChan* ModChan = this->_ModHub->getChannel(l);

    for(size_t i = 0; i < this->_included[l].size(); ++i) {

      // retrieve package from stored index
      OmModPack* ModPack = ModChan->getModpack(this->_included[l][i]);

      // add package to install list
      ModPset->addSetupEntry(ModChan, ModPack);
    }
  }

  // set Install-Only option
  ModPset->setInstallOnly(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK));

  // save changes to XML definition
  ModPset->save();

  // refresh parent
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddPst::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins,IDI_APP,2),Om_getResIcon(this->_hins,IDI_APP,1));

  // Set icons for Up and Down buttons
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(this->_hins, IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(this->_hins, IDI_BT_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Script name, to identify it");
  this->_createTooltip(IDC_BC_CKBX2,  L"Script will installs selected packages without uninstalling others");
  this->_createTooltip(IDC_BC_CKBX1,  L"Create the Script according the current installed and not installed packages");
  this->_createTooltip(IDC_CB_CHN,    L"Channel to configure");
  this->_createTooltip(IDC_LB_INC,    L"Packages the Script will install (or leave installed)");
  this->_createTooltip(IDC_LB_EXC,    L"Packages the Script will uninstall (or leave uninstalled)");
  this->_createTooltip(IDC_BC_RIGH,   L"Add to installed");
  this->_createTooltip(IDC_BC_LEFT,   L"Remove from installed");
  this->_createTooltip(IDC_BC_UP,     L"Move up in list");
  this->_createTooltip(IDC_BC_DN,     L"Move down in list");

  // Set controls default states and parameters
  this->setItemText(IDC_EC_INP01, L"New Script");

  // Enable Quick create from current state
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 1);

  // Disable Install-Only
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 0);

  if(!this->_ModHub)
    return;

  OmWString cb_entry;

  // add Mod Channel(s) to Combo-Box
  for(unsigned i = 0; i < this->_ModHub->channelCount(); ++i) {

    cb_entry  = this->_ModHub->getChannel(i)->title();
    cb_entry += L" [";
    cb_entry += this->_ModHub->getChannel(i)->home();
    cb_entry += L"]";

    this->msgItem(IDC_CB_CHN, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(cb_entry.c_str()));

    // initialize a new install list per Mod Channel
    this->_excluded.push_back(std::vector<int>());
    this->_included.push_back(std::vector<int>());
  }

  // Select first Mod Channel by default
  this->msgItem(IDC_CB_CHN, CB_SETCURSEL, 0);

  // Disable ComboBox and ListBoxes
  this->enableItem(IDC_CB_CHN, false);
  this->enableItem(IDC_LB_EXC, false);
  this->enableItem(IDC_LB_INC, false);

  // initialize with current state
  this->_autoInclude();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddPst::_onResize()
{
  int half_w = this->cliUnitX() * 0.5f;
  int helf_h = this->cliUnitY() * 0.5f;

  // Title label
  this->_setItemPos(IDC_SC_LBL01, 10, 10, 180, 9);
  // Title entry
  this->_setItemPos(IDC_EC_INP01, 10, 20, this->cliUnitX()-20, 13);

  // Per Mod Channel Config label
  this->_setItemPos(IDC_SC_LBL02, 10, 45, 100, 9);
  // Crate from stat CheckBox
  this->_setItemPos(IDC_BC_CKBX1, 130, 45, 100, 9);
  // Mod Channel list ComboBox
  this->_setItemPos(IDC_CB_CHN, 10, 56, this->cliUnitX()-20, 12);

  // Not-Installed label
  this->_setItemPos(IDC_SC_LBL03, 10, 74, 150, 9);
  // Not-Installed ListBox
  this->_setItemPos(IDC_LB_EXC, 10, 85, half_w-30, this->cliUnitY()-140);
  // Add and Rem buttons
  this->_setItemPos(IDC_BC_RIGH, half_w-18, helf_h, 16, 15);
  this->_setItemPos(IDC_BC_LEFT, half_w-18, helf_h+15, 16, 15);

  // Installed label
  this->_setItemPos(IDC_SC_LBL04, half_w, 74, 150, 9);
  // Installed ListBox
  this->_setItemPos(IDC_LB_INC, half_w, 85, half_w-30, this->cliUnitY()-140);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliUnitX()-28, helf_h, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->cliUnitX()-28, helf_h+15, 16, 15);

  // Install-Only CheckBox
  this->_setItemPos(IDC_BC_CKBX2, 10, this->cliUnitY()-45, 180, 9);

  // ----- Separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->cliUnitY()-25, this->cliUnitX()-10, 1);
  // Save As... Button
  this->_setItemPos(IDC_BC_OK, this->cliUnitX()-108, this->cliUnitY()-19, 50, 14);
  // Close Button
  this->_setItemPos(IDC_BC_CANCEL, this->cliUnitX()-54, this->cliUnitY()-19, 50, 14);

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiAddPst::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01:
      if(HIWORD(wParam) == EN_CHANGE) {
        OmWString input;
        this->getItemText(IDC_EC_INP01, input);
        this->enableItem(IDC_BC_OK, !input.empty());
      }
      break;

    case IDC_BC_CKBX1:
      this->_onCkBoxAuto();
      break;

    case IDC_CB_CHN:
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->_buildLbs();
      break;

    case IDC_LB_EXC: //< Uninstall (exclude) ListBox
      if(HIWORD(wParam) == LBN_SELCHANGE) this->_onLbExclsSel();
      if(HIWORD(wParam) == LBN_DBLCLK)  this->_includePkg();
      break;

    case IDC_LB_INC: //< Install (include) ListBox
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

    case IDC_BC_OK: //< Main "OK" Button
      if(this->_onBcOk())
        this->quit();
      break;

    case IDC_BC_CANCEL: //< Main "Cancel" Button
      this->quit();
      break;
    }
  }

  return false;
}
