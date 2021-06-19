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
#include "gui/OmUiAddBat.h"
#include "OmManager.h"
#include "OmBatch.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiAddBat::OmUiAddBat(HINSTANCE hins) : OmDialog(hins),
  _pCtx(nullptr),
  _excluded(),
  _included()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiAddBat::~OmUiAddBat()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiAddBat::id() const
{
  return IDD_ADD_BAT;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddBat::_buildLbs()
{
  if(!this->_pCtx) return;

  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get Location corresponding to current selection
  OmLocation* pLoc = this->_pCtx->locGet(cb_sel);

  unsigned p;
  OmPackage* pPkg;
  wstring item_str;

  // reset List-Box control
  this->msgItem(IDC_LB_EXCLS, LB_RESETCONTENT);

  // fill the left List-Box
  for(size_t i = 0; i < this->_excluded[cb_sel].size(); i++) {

    p = this->_excluded[cb_sel][i];
    pPkg = pLoc->pkgGet(p);

    item_str = Om_getFilePart(pPkg->srcPath());
    this->msgItem(IDC_LB_EXCLS, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));
    this->msgItem(IDC_LB_EXCLS, LB_SETITEMDATA, i, p);
  }

  // reset List-Box control
  this->msgItem(IDC_LB_INCLS, LB_RESETCONTENT);

  // fill the left List-Box
  for(size_t i = 0; i < this->_included[cb_sel].size(); i++) {

    p = this->_included[cb_sel][i];
    pPkg = pLoc->pkgGet(p);

    item_str = Om_getFilePart(pPkg->srcPath());
    this->msgItem(IDC_LB_INCLS, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));
    this->msgItem(IDC_LB_INCLS, LB_SETITEMDATA, i, p);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddBat::_autoInclude()
{
  if(!this->_pCtx) return;

  OmLocation* pLoc;
  OmPackage* pPkg;

  // add Location(s) to Combo-Box
  for(size_t k = 0; k < this->_pCtx->locCount(); ++k) {

    pLoc = this->_pCtx->locGet(k);

    this->_excluded[k].clear();
    this->_included[k].clear();

    for(size_t i = 0; i < pLoc->pkgCount(); ++i) {

      pPkg = pLoc->pkgGet(i);

      if(pPkg->hasSrc()) {
        if(pPkg->hasBck()) {
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
void OmUiAddBat::_includePkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int sel_cnt = this->msgItem(IDC_LB_EXCLS, LB_GETSELCOUNT);
  if(sel_cnt <= 0) return;

  // get list of selected items (index)
  int* lb_sel = new int[sel_cnt];
  this->msgItem(IDC_LB_EXCLS, LB_GETSELITEMS, sel_cnt, reinterpret_cast<LPARAM>(lb_sel));

  int index, pos;
  wchar_t item_buf[OMM_ITM_BUFF];

  // copy selected items from one list to the other list
  for(int i = 0; i < sel_cnt; ++i) {

    // retrieve the package List-Box label
    this->msgItem(IDC_LB_EXCLS, LB_GETTEXT, lb_sel[i], reinterpret_cast<LPARAM>(item_buf));
    // retrieve the package reference index (in Location package list)
    index = this->msgItem(IDC_LB_EXCLS, LB_GETITEMDATA, lb_sel[i]);

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
    pos = this->msgItem(IDC_LB_INCLS, LB_GETCOUNT);
    // add item to the List-Box
    this->msgItem(IDC_LB_INCLS, LB_ADDSTRING, pos, reinterpret_cast<LPARAM>(item_buf));
    this->msgItem(IDC_LB_INCLS, LB_SETITEMDATA, pos, index);
  }

  // remove items from List-Box in reverse order to prevent indexing issues
  int i = sel_cnt;
  while(i--) {
    this->msgItem(IDC_LB_EXCLS, LB_DELETESTRING, lb_sel[i]);
  }

  // we do not need list-box selection anymore
  delete [] lb_sel;

  // disable button until new selection
  this->enableItem(IDC_BC_RIGH, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddBat::_excludePkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int sel_cnt = this->msgItem(IDC_LB_INCLS, LB_GETSELCOUNT);
  if(sel_cnt <= 0) return;

  // get list of selected items (index)
  int* lb_sel = new int[sel_cnt];
  this->msgItem(IDC_LB_INCLS, LB_GETSELITEMS, sel_cnt, reinterpret_cast<LPARAM>(lb_sel));

  int index, pos;
  wchar_t item_buf[OMM_ITM_BUFF];

  // copy selected items from one list to the other list
  for(int i = 0; i < sel_cnt; ++i) {
    // retrieve the package List-Box label
    this->msgItem(IDC_LB_INCLS, LB_GETTEXT, lb_sel[i], reinterpret_cast<LPARAM>(item_buf));
    // retrieve the package reference index (in Location package list)
    index = this->msgItem(IDC_LB_INCLS, LB_GETITEMDATA, lb_sel[i]);

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
    pos = this->msgItem(IDC_LB_EXCLS, LB_GETCOUNT);
    // add item to the List-Box
    this->msgItem(IDC_LB_EXCLS, LB_ADDSTRING, pos, reinterpret_cast<LPARAM>(item_buf));
    this->msgItem(IDC_LB_EXCLS, LB_SETITEMDATA, pos, index);
  }

  // remove items from List-Box in reverse order to prevent indexing issues
  int i = sel_cnt;
  while(i--) {
    this->msgItem(IDC_LB_INCLS, LB_DELETESTRING, lb_sel[i]);
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
void OmUiAddBat::_onCkBoxAuto()
{
  int bm_chk = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);

  this->enableItem(IDC_CB_LOC, !bm_chk);
  this->enableItem(IDC_LB_EXCLS, !bm_chk);
  this->enableItem(IDC_LB_INCLS, !bm_chk);

  if(bm_chk)
    // initialize with current state
    this->_autoInclude();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddBat::_onLbExclsSel()
{
  int sel_cnt = this->msgItem(IDC_LB_EXCLS, LB_GETSELCOUNT);

  if(sel_cnt > 0) {

    // unselect all from the other ListBox, this is less confusing
    this->msgItem(IDC_LB_INCLS, LB_SETSEL, false, -1);

    this->enableItem(IDC_BC_RIGH, true);
    this->enableItem(IDC_BC_LEFT, false);
    this->enableItem(IDC_BC_UP, false);
    this->enableItem(IDC_BC_DN, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddBat::_onLbInclsSel()
{
  int sel_cnt = this->msgItem(IDC_LB_INCLS, LB_GETSELCOUNT);

  if(sel_cnt > 0) {
    // unselect all from the other ListBox, this is less confusing
    this->msgItem(IDC_LB_EXCLS, LB_SETSEL, false, -1);
    this->enableItem(IDC_BC_RIGH, false);
    this->enableItem(IDC_BC_LEFT, true);
  }

  if(sel_cnt == 1) {
    int lb_sel;
    this->msgItem(IDC_LB_INCLS, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));
    int lb_max = this->msgItem(IDC_LB_INCLS, LB_GETCOUNT) - 1;

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
void OmUiAddBat::_onBcUpPkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int sel_cnt = this->msgItem(IDC_LB_INCLS, LB_GETSELCOUNT);
  if(sel_cnt != 1) return;

  // get list of selected item (index)
  int lb_sel;
  this->msgItem(IDC_LB_INCLS, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));
  if(lb_sel == 0) return;

  wchar_t item_buf[OMM_ITM_BUFF];

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_INCLS, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  int index = this->msgItem(IDC_LB_INCLS, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_INCLS, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_INCLS, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_INCLS, LB_SETITEMDATA, lb_sel, index);

  // swap package index to move up
  for(size_t k = 0; k < this->_included[cb_sel].size(); ++k) {
    if(this->_included[cb_sel][k] == index) {
      int temp = this->_included[cb_sel][k];
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
void OmUiAddBat::_onBcDnPkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int sel_cnt = this->msgItem(IDC_LB_INCLS, LB_GETSELCOUNT);
  if(sel_cnt != 1) return;

  // get count of item in List-Box as index to for insertion
  int lb_max = this->msgItem(IDC_LB_INCLS, LB_GETCOUNT) - 1;

  // get list of selected item (index)
  int lb_sel;
  this->msgItem(IDC_LB_INCLS, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));

  // check whether we can move down
  if(lb_sel == lb_max) return;

  wchar_t item_buf[OMM_ITM_BUFF];

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_INCLS, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  int index = this->msgItem(IDC_LB_INCLS, LB_GETITEMDATA, lb_sel);

  this->msgItem(IDC_LB_INCLS, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_INCLS, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_INCLS, LB_SETITEMDATA, lb_sel, index);
  this->msgItem(IDC_LB_INCLS, LB_SETSEL, true, lb_sel);

  // swap package index to move up
  for(size_t k = 0; k < this->_included[cb_sel].size(); ++k) {
    if(this->_included[cb_sel][k] == index) {
      int temp = this->_included[cb_sel][k];
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
bool OmUiAddBat::_onBcOk()
{
  // build the per-Location hash lists
  vector<wstring> loc_uuid;
  vector<vector<uint64_t>> loc_hash_list;

  OmLocation* pLoc;
  OmPackage* pPkg;

  for(size_t k = 0; k < this->_pCtx->locCount(); ++k) {

    pLoc = this->_pCtx->locGet(k);

    // append Location UUID
    loc_uuid.push_back(pLoc->uuid());

    // new hash list
    vector<uint64_t> hash_list;

    for(size_t i = 0; i < this->_included[k].size(); ++i) {

      // retrieve package from stored index
      pPkg = pLoc->pkgGet(this->_included[k][i]);

      // add <install> entry with package hash
      hash_list.push_back(pPkg->hash());
    }

    // add hash list
    loc_hash_list.push_back(hash_list);
  }

  // retrieve batch name
  wstring bat_name;
  this->getItemText(IDC_EC_INP01, bat_name);

  // try to create a new batch
  if(!this->_pCtx->batAdd(bat_name, loc_uuid, loc_hash_list)) {
    Om_dialogBoxErr(this->_hwnd,  L"Batch creation failed", this->_pCtx->lastError());
    return false;
  }

  // refresh parent
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddBat::_onInit()
{
  // Set icons for Up and Down buttons
  this->setBmImage(IDC_BC_UP, Om_getResImage(this->_hins, IDB_BTN_UP));
  this->setBmImage(IDC_BC_DN, Om_getResImage(this->_hins, IDB_BTN_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Indicative name");

  this->_createTooltip(IDC_BC_CHK01,  L"Create batch according current installed packages");
  this->_createTooltip(IDC_CB_LOC,  L"Active location");

  this->_createTooltip(IDC_BC_RIGH,    L"Add to install list");
  this->_createTooltip(IDC_BC_LEFT,    L"Remove from install list");

  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");

  // Set controls default states and parameters
  this->setItemText(IDC_EC_INP01, L"New Batch");

  // Enable Quick create from current state
  this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 1);

  if(!this->_pCtx) return;

  wstring item_str;

  // add Location(s) to Combo-Box
  for(unsigned i = 0; i < this->_pCtx->locCount(); ++i) {

    item_str = this->_pCtx->locGet(i)->title();
    item_str += L" - ";
    item_str += this->_pCtx->locGet(i)->home();

    this->msgItem(IDC_CB_LOC, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));

    // initialize a new install list per Location
    this->_excluded.push_back(vector<int>());
    this->_included.push_back(vector<int>());
  }

  // Select first Location by default
  this->msgItem(IDC_CB_LOC, CB_SETCURSEL, 0);

  // Disable ComboBox and ListBoxes
  this->enableItem(IDC_CB_LOC, false);
  this->enableItem(IDC_LB_EXCLS, false);
  this->enableItem(IDC_LB_INCLS, false);

  // initialize with current state
  this->_autoInclude();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddBat::_onResize()
{
  int half_w = this->width() * 0.5f;
  int helf_h = this->height() * 0.5f;

  // Title label
  this->_setItemPos(IDC_SC_LBL01, 10, 10, 150, 9);
  // Title entry
  this->_setItemPos(IDC_EC_INP01, 10, 20, this->width()-20, 13);

  // Crate from stat CheckBox
  this->_setItemPos(IDC_BC_CHK01, 10, 50, 150, 9);
  // Location list ComboBox
  this->_setItemPos(IDC_CB_LOC, 10, 65, this->width()-20, 12);
  // Not-Installed label
  this->_setItemPos(IDC_SC_LBL02, 10, 85, 150, 9);
  // Not-Installed ListBox
  this->_setItemPos(IDC_LB_EXCLS, 10, 95, half_w-35, this->height()-130);
  // Add and Rem buttons
  this->_setItemPos(IDC_BC_RIGH, half_w-20, helf_h+15, 16, 15);
  this->_setItemPos(IDC_BC_LEFT, half_w-20, helf_h+30, 16, 15);
  // Installed label
  this->_setItemPos(IDC_SC_LBL03, half_w, 85, 150, 9);
  // Installed ListBox
  this->_setItemPos(IDC_LB_INCLS, half_w, 95, half_w-35, this->height()-130);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->width()-30, helf_h+15, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->width()-30, helf_h+30, 16, 15);

  // ----- Separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Save As... Button
  this->_setItemPos(IDC_BC_OK, this->width()-108, this->height()-19, 50, 14);
  // Close Button
  this->_setItemPos(IDC_BC_CANCEL, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiAddBat::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    wstring item_str;

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01:
      if(HIWORD(wParam) == EN_CHANGE) {
        this->getItemText(IDC_EC_INP01, item_str);
        this->enableItem(IDC_BC_OK, !item_str.empty());
      }
      break;

    case IDC_BC_CHK01:
      this->_onCkBoxAuto();
      break;

    case IDC_CB_LOC:
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->_buildLbs();
      break;

    case IDC_LB_EXCLS: //< Uninstall (exclude) ListBox
      if(HIWORD(wParam) == LBN_SELCHANGE) this->_onLbExclsSel();
      if(HIWORD(wParam) == LBN_DBLCLK)  this->_includePkg();
      break;

    case IDC_LB_INCLS: //< Install (include) ListBox
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
