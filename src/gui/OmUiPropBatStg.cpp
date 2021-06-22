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
#include "OmContext.h"
#include "OmBatch.h"
#include "OmDialogProp.h"
#include "gui/OmUiPropBat.h"
#include "gui/OmUiPropBatStg.h"



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropBatStg::OmUiPropBatStg(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i) {
    this->_chParam[i] = false;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropBatStg::~OmUiPropBatStg()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropBatStg::id() const
{
  return IDD_PROP_BAT_STG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::setChParam(unsigned i, bool en)
{
  this->_chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_buildLbs()
{
  // reset List-Box control
  this->msgItem(IDC_LB_EXC, LB_RESETCONTENT);

  OmBatch* pBat = static_cast<OmUiPropBat*>(this->_parent)->batCur();
  if(!pBat) return;
  OmContext* pCtx = pBat->ownerCtx();
  if(!pCtx) return;

  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get Location corresponding to current selection
  OmLocation* pLoc = pCtx->locGet(cb_sel);

  unsigned p;
  OmPackage* pPkg;
  wstring item_str;

  // fill the left List-Box
  for(size_t i = 0; i < this->_excluded[cb_sel].size(); i++) {

    p = this->_excluded[cb_sel][i];
    pPkg = pLoc->pkgGet(p);

    item_str = Om_getFilePart(pPkg->srcPath());
    this->msgItem(IDC_LB_EXC, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));
    this->msgItem(IDC_LB_EXC, LB_SETITEMDATA, i, p);
  }

  // reset List-Box control
  this->msgItem(IDC_LB_INC, LB_RESETCONTENT);

  // fill the right List-Box
  for(size_t i = 0; i < this->_included[cb_sel].size(); i++) {

    p = this->_included[cb_sel][i];
    pPkg = pLoc->pkgGet(p);

    item_str = Om_getFilePart(pPkg->srcPath());
    this->msgItem(IDC_LB_INC, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));
    this->msgItem(IDC_LB_INC, LB_SETITEMDATA, i, p);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_includePkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int sel_cnt = this->msgItem(IDC_LB_EXC, LB_GETSELCOUNT);
  if(sel_cnt <= 0) return;

  // get list of selected items (index)
  int* lb_sel = new int[sel_cnt];
  this->msgItem(IDC_LB_EXC, LB_GETSELITEMS, sel_cnt, reinterpret_cast<LPARAM>(lb_sel));

  int index, pos;
  wchar_t item_buf[OMM_ITM_BUFF];

  // copy selected items from one list to the other list
  for(int i = 0; i < sel_cnt; ++i) {

    // retrieve the package List-Box label
    this->msgItem(IDC_LB_EXC, LB_GETTEXT, lb_sel[i], reinterpret_cast<LPARAM>(item_buf));
    // retrieve the package reference index (in Location package list)
    index = this->msgItem(IDC_LB_EXC, LB_GETITEMDATA, lb_sel[i]);

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
    pos = this->msgItem(IDC_LB_INC, LB_GETCOUNT);
    // add item to the List-Box
    this->msgItem(IDC_LB_INC, LB_ADDSTRING, pos, reinterpret_cast<LPARAM>(item_buf));
    this->msgItem(IDC_LB_INC, LB_SETITEMDATA, pos, index);
  }

  // remove items from List-Box in reverse order to prevent indexing issues
  int i = sel_cnt;
  while(i--) {
    this->msgItem(IDC_LB_EXC, LB_DELETESTRING, lb_sel[i]);
  }

  // we do not need list-box selection anymore
  delete [] lb_sel;

  // disable button until new selection
  this->enableItem(IDC_BC_RIGH, false);

  // user modified parameter, notify it
  this->setChParam(BAT_PROP_STG_INSLS, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_excludePkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int sel_cnt = this->msgItem(IDC_LB_INC, LB_GETSELCOUNT);
  if(sel_cnt <= 0) return;

  // get list of selected items (index)
  int* lb_sel = new int[sel_cnt];
  this->msgItem(IDC_LB_INC, LB_GETSELITEMS, sel_cnt, reinterpret_cast<LPARAM>(lb_sel));

  int index, pos;
  wchar_t item_buf[OMM_ITM_BUFF];

  // copy selected items from one list to the other list
  for(int i = 0; i < sel_cnt; ++i) {
    // retrieve the package List-Box label
    this->msgItem(IDC_LB_INC, LB_GETTEXT, lb_sel[i], reinterpret_cast<LPARAM>(item_buf));
    // retrieve the package reference index (in Location package list)
    index = this->msgItem(IDC_LB_INC, LB_GETITEMDATA, lb_sel[i]);

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
    pos = this->msgItem(IDC_LB_EXC, LB_GETCOUNT);
    // add item to the List-Box
    this->msgItem(IDC_LB_EXC, LB_ADDSTRING, pos, reinterpret_cast<LPARAM>(item_buf));
    this->msgItem(IDC_LB_EXC, LB_SETITEMDATA, pos, index);
  }

  // remove items from List-Box in reverse order to prevent indexing issues
  int i = sel_cnt;
  while(i--) {
    this->msgItem(IDC_LB_INC, LB_DELETESTRING, lb_sel[i]);
  }

  // we do not need list-box selection anymore
  delete [] lb_sel;

  // disable button until new selection
  this->enableItem(IDC_BC_LEFT, false);
  this->enableItem(IDC_BC_UP, false);
  this->enableItem(IDC_BC_DN, false);

  // user modified parameter, notify it
  this->setChParam(BAT_PROP_STG_INSLS, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onLbExclsSel()
{
  int sel_cnt = this->msgItem(IDC_LB_EXC, LB_GETSELCOUNT);

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
void OmUiPropBatStg::_onLbInclsSel()
{
  int sel_cnt = this->msgItem(IDC_LB_INC, LB_GETSELCOUNT);

  if(sel_cnt > 0) {
    // unselect all from the other ListBox, this is less confusing
    this->msgItem(IDC_LB_EXC, LB_SETSEL, false, -1);
    this->enableItem(IDC_BC_RIGH, false);
    this->enableItem(IDC_BC_LEFT, true);
  }

  if(sel_cnt == 1) {

    int lb_sel;
    this->msgItem(IDC_LB_INC, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));
    int lb_max = this->msgItem(IDC_LB_INC, LB_GETCOUNT) - 1;

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
void OmUiPropBatStg::_onBcUpPkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int sel_cnt = this->msgItem(IDC_LB_INC, LB_GETSELCOUNT);
  if(sel_cnt != 1) return;

  // get list of selected item (index)
  int lb_sel;
  this->msgItem(IDC_LB_INC, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));

  // check whether we can move up
  if(lb_sel == 0) return;

  wchar_t item_buf[OMM_ITM_BUFF];

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_INC, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  int index = this->msgItem(IDC_LB_INC, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_INC, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_INC, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_INC, LB_SETITEMDATA, lb_sel, index);

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

  // user modified parameter, notify it
  this->setChParam(BAT_PROP_STG_INSLS, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onBcDnPkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cb_sel < 0) return;

  // get count of selected items
  int sel_cnt = this->msgItem(IDC_LB_INC, LB_GETSELCOUNT);
  if(sel_cnt != 1) return;

  // get count of item in List-Box as index to for insertion
  int lb_max = this->msgItem(IDC_LB_INC, LB_GETCOUNT) - 1;

  // get list of selected item (index)
  int lb_sel;
  this->msgItem(IDC_LB_INC, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));

  // check whether we can move down
  if(lb_sel == lb_max) return;

  wchar_t item_buf[OMM_ITM_BUFF];

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_INC, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  int index = this->msgItem(IDC_LB_INC, LB_GETITEMDATA, lb_sel);

  this->msgItem(IDC_LB_INC, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_INC, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_INC, LB_SETITEMDATA, lb_sel, index);
  this->msgItem(IDC_LB_INC, LB_SETSEL, true, lb_sel);

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

  // user modified parameter, notify it
  this->setChParam(BAT_PROP_STG_INSLS, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onInit()
{
  // Set icons for Up and Down buttons
  this->setBmImage(IDC_BC_UP, Om_getResImage(this->_hins, IDB_BTN_UP));
  this->setBmImage(IDC_BC_DN, Om_getResImage(this->_hins, IDB_BTN_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,L"Batch name");

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,L"Indicative name");

  this->_createTooltip(IDC_BC_CHK01,L"Create batch according current installed packages");
  this->_createTooltip(IDC_CB_LOC,  L"Active location");

  this->_createTooltip(IDC_BC_RIGH, L"Add to install list");
  this->_createTooltip(IDC_BC_LEFT, L"Remove from install list");

  this->_createTooltip(IDC_BC_UP,   L"Move up");
  this->_createTooltip(IDC_BC_DN,   L"Move down");

  // Set controls default states and parameters
  this->setItemText(IDC_EC_INP01, L"New Batch");


  OmBatch* pBat = static_cast<OmUiPropBat*>(this->_parent)->batCur();
  OmContext* pCtx = pBat->ownerCtx();

  if(pBat && pCtx) {
  // Automatic fix Batch / Context Location inconsistency
  for(size_t l = 0; l < pCtx->locCount(); ++l) //< Add missing Location
    if(!pBat->hasLoc(pCtx->locGet(l)->uuid()))
      pBat->locAdd(pCtx->locGet(l)->uuid());

  // Remove unavailable location
  vector<wstring> uuid_ls;
  for(size_t l = 0; l < pBat->locCount(); ++l)
    if(pCtx->locFind(pBat->locGetUuid(l)) < 0)
      uuid_ls.push_back(pBat->locGetUuid(l));

  for(size_t i = 0; i < uuid_ls.size(); ++i)
    pBat->locRem(uuid_ls[i]);
  }

  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onResize()
{
  int half_w = this->width() * 0.5f;
  int helf_h = this->height() * 0.5f;

  // Batch Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 10, 64, 9);
  this->_setItemPos(IDC_EC_INP01, 70, 10, this->width()-90, 13);

  // Location list ComboBox
  this->_setItemPos(IDC_CB_LOC, 5, 35, this->width()-15, 12);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_LOC), nullptr, true);
  // Not-Installed label
  this->_setItemPos(IDC_SC_LBL02, 10, 55, 150, 9);
  // Not-Installed ListBox
  this->_setItemPos(IDC_LB_EXC, 10, 65, half_w-35, this->height()-80);
  // Add and Rem buttons
  this->_setItemPos(IDC_BC_RIGH, half_w-20, helf_h+10, 16, 15);
  this->_setItemPos(IDC_BC_LEFT, half_w-20, helf_h+25, 16, 15);
  // Installed label
  this->_setItemPos(IDC_SC_LBL03, half_w, 55, 150, 9);
  // Installed ListBox
  this->_setItemPos(IDC_LB_INC, half_w, 65, half_w-35, this->height()-80);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->width()-30, helf_h+10, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->width()-30, helf_h+25, 16, 15);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onRefresh()
{
  OmBatch* pBat = static_cast<OmUiPropBat*>(this->_parent)->batCur();
  if(!pBat) return;

  OmContext* pCtx = pBat->ownerCtx();
  if(!pCtx) return;

  this->setItemText(IDC_EC_INP01, pBat->title());

  OmLocation* pLoc;

  // empty the ComboBox
  this->msgItem(IDC_CB_LOC, CB_RESETCONTENT);

  // check whether a package is unavailable
  bool unavail = false;

  wstring item_str;
  int l, p;

  this->_excluded.clear();
  this->_included.clear();

  // add Location(s) to Combo-Box
  for(unsigned i = 0; i < pCtx->locCount(); ++i) {

    pLoc = pCtx->locGet(i);

    item_str = pLoc->title();
    item_str += L" - ";
    item_str += pLoc->home();

    this->msgItem(IDC_CB_LOC, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));

    // initialize new lists for this Location
    this->_excluded.push_back(vector<int>());
    this->_included.push_back(vector<int>());

    // get Location index in Btach
    l = pBat->locGetIndex(pLoc->uuid());

    // Fill include and exclude list according batch install list
    if(l != -1) {
      // fill the include list ordered as in the batch
      for(size_t j = 0; j < pBat->insCount(l); ++j) {
        p = pLoc->pkgIndex(pBat->insGet(l, j));
        if(p >= 0) {
          this->_included.back().push_back(p);
        } else {
          unavail = true;
        }
      }
      // fill the exclude list
      for(size_t j = 0; j < pLoc->pkgCount(); ++j) {
        if(!pBat->hasIns(l, pLoc->pkgGet(j)->hash())) {
          this->_excluded.back().push_back(j);
        }
      }
    } else {
      // fill the exclude list
      for(size_t j = 0; j < pLoc->pkgCount(); ++j)
        this->_excluded.back().push_back(j);
    }
  }

  // Select first Location by default
  this->msgItem(IDC_CB_LOC, CB_SETCURSEL, 0);

  // Disable ComboBox and ListBoxes
  this->enableItem(IDC_CB_LOC, true);
  this->enableItem(IDC_LB_EXC, true);
  this->enableItem(IDC_LB_INC, true);

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;

  // fill up each ListBox
  this->_buildLbs();

  // check whether we got some unavailable package to be deleted
  if(unavail) {
    // force notify change of the install list
    this->setChParam(BAT_PROP_STG_INSLS, true);
    // notify user
    wstring wrn = L"The Batch have reference to Packages "
                  L"which are no longer available."
                  L"\n\nDo you want to clean invalid references ?";

    if(Om_dialogBoxQuerryWarn(this->_hwnd, L"Package reference error", wrn)) {
      // Force to apply changes
      static_cast<OmDialogProp*>(this->_parent)->applyChanges();
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropBatStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01: //< Title EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->setChParam(BAT_PROP_STG_TITLE, true);
      break;

    case IDC_CB_LOC:  //< Location ComboBox
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
