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
void OmUiPropBatStg::_upPkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOCLS, CB_GETCURSEL);

  if(cb_sel < 0)
    return;

  HWND hLsr = this->getItem(IDC_LB_INCLS);

  // get count of selected items
  int sel_cnt = SendMessageW(hLsr, LB_GETSELCOUNT, 0, 0);

  if(sel_cnt == 1) {

    // get list of selected item (index)
    int lb_sel;
    SendMessageW(hLsr, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));

    // check whether we can move up
    if(lb_sel == 0)
      return;

    wchar_t item_buf[OMM_ITM_BUFF];

    // retrieve the package List-Box label
    SendMessageW(hLsr, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
    int index = SendMessageW(hLsr, LB_GETITEMDATA, lb_sel - 1, 0);

    SendMessageW(hLsr, LB_DELETESTRING, lb_sel - 1, 0);

    SendMessageW(hLsr, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
    SendMessageW(hLsr, LB_SETITEMDATA, lb_sel, index);

    // swap package index to move up
    for(size_t k = 0; k < this->_incLs[cb_sel].size(); ++k) {
      if(this->_incLs[cb_sel][k] == index) {
        int temp = this->_incLs[cb_sel][k];
        this->_incLs[cb_sel][k] = this->_incLs[cb_sel][k+1];
        this->_incLs[cb_sel][k+1] = temp;
        break;
      }
    }

    this->enableItem(IDC_BC_UP, (lb_sel > 1));
    this->enableItem(IDC_BC_DN, true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_dnPkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOCLS, CB_GETCURSEL);

  if(cb_sel < 0)
    return;

  HWND hLsr = this->getItem(IDC_LB_INCLS);

  // get count of selected items
  int sel_cnt = SendMessageW(hLsr, LB_GETSELCOUNT, 0, 0);

  // get count of item in List-Box as index to for insertion
  int lb_max = SendMessageW(hLsr, LB_GETCOUNT, 0, 0) - 1;

  if(sel_cnt == 1) {

    // get list of selected item (index)
    int lb_sel;
    SendMessageW(hLsr, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));

    // check whether we can move down
    if(lb_sel == lb_max)
      return;

    wchar_t item_buf[OMM_ITM_BUFF];

    // retrieve the package List-Box label
    SendMessageW(hLsr, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
    int index = SendMessageW(hLsr, LB_GETITEMDATA, lb_sel, 0);

    SendMessageW(hLsr, LB_DELETESTRING, lb_sel, 0);

    lb_sel++;

    SendMessageW(hLsr, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
    SendMessageW(hLsr, LB_SETITEMDATA, lb_sel, index);
    SendMessageW(hLsr, LB_SETSEL, true, lb_sel);

    // swap package index to move up
    for(size_t k = 0; k < this->_incLs[cb_sel].size(); ++k) {
      if(this->_incLs[cb_sel][k] == index) {
        int temp = this->_incLs[cb_sel][k];
        this->_incLs[cb_sel][k] = this->_incLs[cb_sel][k+1];
        this->_incLs[cb_sel][k+1] = temp;
        break;
      }
    }

    this->enableItem(IDC_BC_UP, true);
    this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_addPkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOCLS, CB_GETCURSEL);

  if(cb_sel < 0)
    return;

  HWND hLsl = this->getItem(IDC_LB_EXCLS);
  HWND hLsr = this->getItem(IDC_LB_INCLS);

  // get count of selected items
  int sel_cnt = SendMessageW(hLsl, LB_GETSELCOUNT, 0, 0);

  if(sel_cnt > 0) {

    // get list of selected items (index)
    int* lb_sel = new int[sel_cnt];
    SendMessageW(hLsl, LB_GETSELITEMS, sel_cnt, reinterpret_cast<LPARAM>(lb_sel));

    int index, pos;
    wchar_t item_buf[OMM_ITM_BUFF];

    // copy selected items from one list to the other list
    for(int i = 0; i < sel_cnt; ++i) {

      // retrieve the package List-Box label
      SendMessageW(hLsl, LB_GETTEXT, lb_sel[i], reinterpret_cast<LPARAM>(item_buf));
      // retrieve the package reference index (in Location package list)
      index = SendMessageW(hLsl, LB_GETITEMDATA, lb_sel[i], 0);

      // remove package index from left mirror list
      for(size_t k = 0; k < this->_excLs[cb_sel].size(); ++k) {
        if(this->_excLs[cb_sel][k] == index) {
          this->_excLs[cb_sel].erase(this->_excLs[cb_sel].begin()+k);
          break;
        }
      }
      // add package index to right mirror list
      this->_incLs[cb_sel].push_back(index);

      // get count of item in List-Box as index to for insertion
      pos = SendMessageW(hLsr, LB_GETCOUNT, 0, 0);
      // add item to the List-Box
      SendMessageW(hLsr, LB_ADDSTRING, pos, reinterpret_cast<LPARAM>(item_buf));
      SendMessageW(hLsr, LB_SETITEMDATA, pos, index);
    }

    // remove items from List-Box in reverse order to prevent indexing issues
    int i = sel_cnt;
    while(i--) {
      SendMessageW(hLsl, LB_DELETESTRING, lb_sel[i], 0);
    }

    // we do not need list-box selection anymore
    delete [] lb_sel;

    // disable button until new selection
    this->enableItem(IDC_BC_ADD, false);
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_remPkg()
{
  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOCLS, CB_GETCURSEL);

  if(cb_sel < 0)
    return;

  HWND hLsl = this->getItem(IDC_LB_EXCLS);
  HWND hLsr = this->getItem(IDC_LB_INCLS);

  // get count of selected items
  int sel_cnt = SendMessageW(hLsr, LB_GETSELCOUNT, 0, 0);

  if(sel_cnt > 0) {

    // get list of selected items (index)
    int* lb_sel = new int[sel_cnt];
    SendMessageW(hLsr, LB_GETSELITEMS, sel_cnt, reinterpret_cast<LPARAM>(lb_sel));

    int index, pos;
    wchar_t item_buf[OMM_ITM_BUFF];

    // copy selected items from one list to the other list
    for(int i = 0; i < sel_cnt; ++i) {
      // retrieve the package List-Box label
      SendMessageW(hLsr, LB_GETTEXT, lb_sel[i], reinterpret_cast<LPARAM>(item_buf));
      // retrieve the package reference index (in Location package list)
      index = SendMessageW(hLsr, LB_GETITEMDATA, lb_sel[i], 0);

      // remove package index from right mirror list
      for(size_t k = 0; k < this->_incLs[cb_sel].size(); ++k) {
        if(this->_incLs[cb_sel][k] == index) {
          this->_incLs[cb_sel].erase(this->_incLs[cb_sel].begin()+k);
          break;
        }
      }
      // add package index to left mirror list
      this->_excLs[cb_sel].push_back(index);

      // get count of item in List-Box as index to for insertion
      pos = SendMessageW(hLsl, LB_GETCOUNT, 0, 0);
      // add item to the List-Box
      SendMessageW(hLsl, LB_ADDSTRING, pos, reinterpret_cast<LPARAM>(item_buf));
      SendMessageW(hLsl, LB_SETITEMDATA, pos, index);
    }

    // remove items from List-Box in reverse order to prevent indexing issues
    int i = sel_cnt;
    while(i--) {
      SendMessageW(hLsr, LB_DELETESTRING, lb_sel[i], 0);
    }

    // we do not need list-box selection anymore
    delete [] lb_sel;

    // disable button until new selection
    this->enableItem(IDC_BC_DEL, false);
    this->enableItem(IDC_BC_UP, false);
    this->enableItem(IDC_BC_DN, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_rebuildPkgLb()
{
  OmBatch* pBat = static_cast<OmUiPropBat*>(this->_parent)->batch();

  if(pBat == nullptr)
    return;

  OmContext* pCtx = pBat->context();

  if(pCtx == nullptr)
    return;

  // get current Combo-Box selection first Location by default
  int cb_sel = this->msgItem(IDC_CB_LOCLS, CB_GETCURSEL);

  if(cb_sel < 0)
    return;

  // get Location corresponding to current selection
  OmLocation* pLoc = pCtx->location(cb_sel);

  // hold handle to List-Box controls
  HWND hLsl = this->getItem(IDC_LB_EXCLS);
  HWND hLsr = this->getItem(IDC_LB_INCLS);

  unsigned p;
  OmPackage* pPkg;
  wstring item_str;

  // reset List-Box control
  SendMessageW(hLsl, LB_RESETCONTENT, 0, 0);

  // fill the left List-Box
  for(size_t i = 0; i < this->_excLs[cb_sel].size(); i++) {

    p = this->_excLs[cb_sel][i];
    pPkg = pLoc->package(p);

    item_str = Om_getFilePart(pPkg->sourcePath());
    SendMessageW(hLsl, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));
    SendMessageW(hLsl, LB_SETITEMDATA, i, p);
  }

  // reset List-Box control
  SendMessageW(hLsr, LB_RESETCONTENT, 0, 0);

  // fill the right List-Box
  for(size_t i = 0; i < this->_incLs[cb_sel].size(); i++) {

    p = this->_incLs[cb_sel][i];
    pPkg = pLoc->package(p);

    item_str = Om_getFilePart(pPkg->sourcePath());
    SendMessageW(hLsr, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));
    SendMessageW(hLsr, LB_SETITEMDATA, i, p);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onInit()
{
  // Set icons for Up and Down buttons
  this->msgItem(IDC_BC_UP, BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcUp));
  this->msgItem(IDC_BC_DN, BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcDn));

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Batch name");

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Indicative name");

  this->_createTooltip(IDC_BC_CHK01,  L"Create batch according current installed packages");
  this->_createTooltip(IDC_CB_LOCLS,  L"Active location");

  this->_createTooltip(IDC_BC_ADD,    L"Add to install list");
  this->_createTooltip(IDC_BC_DEL,    L"Remove from install list");

  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");

  // Set controls default states and parameters
  this->setItemText(IDC_EC_INPT1, L"New Batch");

  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onResize()
{
  int half_width = this->width() * 0.5f;

  // Batch Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 10, 64, 9);
  this->_setItemPos(IDC_EC_INPT1, 70, 10, this->width()-90, 13);

  // Location list ComboBox
  this->_setItemPos(IDC_CB_LOCLS, 5, 35, this->width()-15, 12);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_LOCLS), nullptr, true);
  // Not-Installed label
  this->_setItemPos(IDC_SC_LBL02, 10, 55, 150, 9);
  // Not-Installed ListBox
  this->_setItemPos(IDC_LB_EXCLS, 10, 65, half_width-35, this->height()-80);
  // Add and Rem buttons
  this->_setItemPos(IDC_BC_ADD, half_width-20, 90, 16, 15);
  this->_setItemPos(IDC_BC_DEL, half_width-20, 105, 16, 15);
  // Installed label
  this->_setItemPos(IDC_SC_LBL03, half_width, 55, 150, 9);
  // Installed ListBox
  this->_setItemPos(IDC_LB_INCLS, half_width, 65, half_width-35, this->height()-80);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->width()-30, 90, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->width()-30, 105, 16, 15);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onRefresh()
{
  OmBatch* pBat = static_cast<OmUiPropBat*>(this->_parent)->batch();

  if(pBat == nullptr)
    return;

  OmContext* pCtx = pBat->context();

  if(pCtx == nullptr)
    return;

  this->setItemText(IDC_EC_INPT1, pBat->title());

  OmLocation* pLoc;

  // initialize Location ComboBox content
  HWND hCb = this->getItem(IDC_CB_LOCLS);

  // empty the ComboBox
  SendMessageW(hCb, CB_RESETCONTENT, 0, 0);

  wstring item_str;
  int l;

  this->_excLs.clear();
  this->_incLs.clear();

  // add Location(s) to Combo-Box
  for(unsigned i = 0; i < pCtx->locationCount(); ++i) {

    pLoc = pCtx->location(i);

    item_str = pLoc->title();
    item_str += L" - ";
    item_str += pLoc->home();

    SendMessageW(hCb, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));

    // initialize a new install list per Location
    this->_excLs.push_back(vector<int>());
    this->_incLs.push_back(vector<int>());

    // get Location index in Btach
    l = pBat->getLocationIndex(pLoc->uuid());

    // Fill include and exclude list according batch install list
    if(l != -1) {
      // fill the include list ordered as in the batch
      for(size_t j = 0; j < pBat->getInstallCount(l); ++j) {
        this->_incLs.back().push_back(pLoc->findPackageIndex(pBat->getInstallHash(l, j)));
      }
      // fill the exclude list
      for(size_t j = 0; j < pLoc->packageCount(); ++j) {
        if(!pBat->hasInstallHash(l, pLoc->package(j)->hash())) {
          this->_excLs.back().push_back(j);
        }
      }
    } else {
      // fill the exclude list
      for(size_t j = 0; j < pLoc->packageCount(); ++j)
        this->_excLs.back().push_back(j);
    }
  }

  // Select first Location by default
  SendMessageW(hCb, CB_SETCURSEL, 0, 0);

  // Disable ComboBox and ListBoxes
  this->enableItem(IDC_CB_LOCLS, true);
  this->enableItem(IDC_LB_EXCLS, true);
  this->enableItem(IDC_LB_INCLS, true);

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;

  this->_rebuildPkgLb();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropBatStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    OmBatch* pBat = static_cast<OmUiPropBat*>(this->_parent)->batch();

    if(pBat == nullptr)
      return false;

    int sel_cnt, lb_max, lb_sel;

    wstring item_str;

    switch(LOWORD(wParam))
    {

    case IDC_EC_INPT1: //< Entry for Title
      // user modified parameter, notify it
      this->setChParam(BAT_PROP_STG_TITLE, true);
      break;

    case IDC_CB_LOCLS:
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        this->_rebuildPkgLb();
      }
      break;

    case IDC_LB_EXCLS: // Dependencies ListBox
      if(HIWORD(wParam) == LBN_SELCHANGE) {

        sel_cnt = this->msgItem(IDC_LB_EXCLS, LB_GETSELCOUNT);

        if(sel_cnt > 0) {
          // unselect all from the other ListBox, this is less confusing
          this->msgItem(IDC_LB_INCLS, LB_SETSEL, false, -1);
          this->enableItem(IDC_BC_ADD, true);
          this->enableItem(IDC_BC_DEL, false);
          this->enableItem(IDC_BC_UP, false);
          this->enableItem(IDC_BC_DN, false);
        }

      }
      if(HIWORD(wParam) == LBN_DBLCLK) {
        this->_addPkg();
        // user modified parameter, notify it
        this->setChParam(BAT_PROP_STG_INSLS, true);
      }
      break;

    case IDC_LB_INCLS: // Dependencies ListBox
      if(HIWORD(wParam) == LBN_SELCHANGE) {

        sel_cnt = this->msgItem(IDC_LB_INCLS, LB_GETSELCOUNT);

        if(sel_cnt > 0) {
          // unselect all from the other ListBox, this is less confusing
          this->msgItem(IDC_LB_EXCLS, LB_SETSEL, false, -1);
          this->enableItem(IDC_BC_ADD, false);
          this->enableItem(IDC_BC_DEL, true);
        }

        if(sel_cnt == 1) {
          this->msgItem(IDC_LB_INCLS, LB_GETSELITEMS, 1, reinterpret_cast<LPARAM>(&lb_sel));
          lb_max = this->msgItem(IDC_LB_INCLS, LB_GETCOUNT) - 1;
          this->enableItem(IDC_BC_UP, (lb_sel > 0));
          this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
        } else {
          this->enableItem(IDC_BC_UP, false);
          this->enableItem(IDC_BC_DN, false);
        }

      }
      if(HIWORD(wParam) == LBN_DBLCLK) {
        this->_remPkg();
        // user modified parameter, notify it
        this->setChParam(BAT_PROP_STG_INSLS, true);
      }
      break;

    case IDC_BC_ADD:
      this->_addPkg();
      // user modified parameter, notify it
      this->setChParam(BAT_PROP_STG_INSLS, true);
      break;

    case IDC_BC_DEL:
      this->_remPkg();
      // user modified parameter, notify it
      this->setChParam(BAT_PROP_STG_INSLS, true);
      break;

    case IDC_BC_UP:
      this->_upPkg();
      // user modified parameter, notify it
      this->setChParam(BAT_PROP_STG_INSLS, true);
      break;

    case IDC_BC_DN:
      this->_dnPkg();
      // user modified parameter, notify it
      this->setChParam(BAT_PROP_STG_INSLS, true);
      break;
    }
  }

  return false;
}
