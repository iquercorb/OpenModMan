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
#include "gui/OmUiNewBat.h"
#include "OmManager.h"
#include "OmBatch.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewBat::OmUiNewBat(HINSTANCE hins) : OmDialog(hins),
  _context(nullptr),
  _excLs(),
  _incLs()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewBat::~OmUiNewBat()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiNewBat::id() const
{
  return IDD_NEW_BAT;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewBat::_onShow()
{
    // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Indicative name");
  this->_createTooltip(IDC_BC_CHK01,  L"Create batch according current installed packages");
  this->_createTooltip(IDC_CB_LOCLS,  L"Active location");
  this->_createTooltip(IDC_BC_ADD,    L"Add to install list");
  this->_createTooltip(IDC_BC_DEL,    L"Remove from install list");
  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");

  // Set icons for Up and Down buttons
  HBITMAP hbm;
  hbm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_UP), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_UP), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbm);

  hbm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_DN), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_DN), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbm);

  SetWindowTextW(GetDlgItem(this->_hwnd, IDC_EC_INPT1), L"Batch Install");

  //SetWindowTextW(GetDlgItem(this->_hwnd, IDC_BC_UP), L"\u25b2");
  //SetWindowTextW(GetDlgItem(this->_hwnd, IDC_BC_DN), L"\u25bc");

  // Enable Quick create from current state
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01), BM_SETCHECK, 1, 0);

  if(this->_context == nullptr)
    return;

  // initialize Location ComboBox content
  HWND hcb = GetDlgItem(this->_hwnd, IDC_CB_LOCLS);

  wstring name;

  // add Location(s) to Combo-Box
  for(unsigned i = 0; i < this->_context->locationCount(); ++i) {

    name = this->_context->location(i)->title();
    name += L" - ";
    name += this->_context->location(i)->home();

    SendMessageW(hcb, CB_ADDSTRING, i, (LPARAM)name.c_str());

    // initialize a new install list per Location
    this->_excLs.push_back(vector<int>());
    this->_incLs.push_back(vector<int>());
  }

  // Select first Location by default
  SendMessageW(hcb, CB_SETCURSEL, 0, 0);

  // Disable ComboBox and ListBoxes
  EnableWindow(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), false);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_EXC), false);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_INC), false);

  // initialize with current state
  this->_qucikFromCur();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewBat::_onResize()
{
  int half_width = this->width() * 0.5f;

  // Title label
  this->_setItemPos(IDC_SC_LBL01, 10, 10, 150, 9);
  // Title entry
  this->_setItemPos(IDC_EC_INPT1, 10, 20, this->width()-20, 13);

  // Crate from stat CheckBox
  this->_setItemPos(IDC_BC_CHK01, 10, 50, 150, 9);
  // Location list ComboBox
  this->_setItemPos(IDC_CB_LOCLS, 10, 65, this->width()-20, 12);
  // Not-Installed label
  this->_setItemPos(IDC_SC_LBL02, 10, 85, 150, 9);
  // Not-Installed ListBox
  this->_setItemPos(IDC_LB_EXC, 10, 95, half_width-35, this->height()-130);
  // Add and Rem buttons
  this->_setItemPos(IDC_BC_ADD, half_width-20, 180, 16, 15);
  this->_setItemPos(IDC_BC_DEL, half_width-20, 195, 16, 15);
  // Installed label
  this->_setItemPos(IDC_SC_LBL03, half_width, 85, 150, 9);
  // Installed ListBox
  this->_setItemPos(IDC_LB_INC, half_width, 95, half_width-35, this->height()-130);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->width()-30, 180, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->width()-30, 195, 16, 15);

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
void OmUiNewBat::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewBat::_onQuit()
{

}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewBat::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_EXC:
      if(((LPNMHDR)lParam)->code == LBN_DBLCLK) {
        this->_addPkg();
      }
      break;

    case IDC_LB_INC:
      if(((LPNMHDR)lParam)->code == LBN_DBLCLK) {
        this->_remPkg();
      }
      break;
    }
  }

  if(uMsg == WM_COMMAND) {

    wchar_t wcbuf[OMM_MAX_PATH];
    int n_sel, lb_max, lb_sel;
    bool chk01;

    switch(LOWORD(wParam))
    {
    case IDC_EC_INPT1:
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
      EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_OK), wcslen(wcbuf));
      break;

    case IDC_BC_CHK01:
      chk01 = SendMessage(GetDlgItem(this->_hwnd,IDC_BC_CHK01),BM_GETCHECK,0,0);
      if(chk01) {
        // disable ComboBox and ListBoxes
        EnableWindow(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_EXC), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_INC), false);
        // initialize with current state
        this->_qucikFromCur();
      } else {
        // enable ComboBox and ListBoxes
        EnableWindow(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_EXC), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_INC), true);
      }
      break;

    case IDC_CB_LOCLS:
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        this->_rebuildPkgLb();
      }
      break;

    case IDC_LB_EXC: // Dependencies ListBox
      if(HIWORD(wParam) == LBN_SELCHANGE) {

        n_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_EXC), LB_GETSELCOUNT, 0, 0);

        if(n_sel > 0) {
          // unselect all from the other ListBox, this is less confusing
          SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_INC), LB_SETSEL, false, -1);
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ADD), true);
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), false);
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), false);
        }

      }
      if(HIWORD(wParam) == LBN_DBLCLK) {
        this->_addPkg();
      }
      break;

    case IDC_LB_INC: // Dependencies ListBox
      if(HIWORD(wParam) == LBN_SELCHANGE) {

        n_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_INC), LB_GETSELCOUNT, 0, 0);

        if(n_sel > 0) {
          // unselect all from the other ListBox, this is less confusing
          SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_EXC), LB_SETSEL, false, -1);
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ADD), false);
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), true);
        }

        if(n_sel == 1) {
          SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_INC), LB_GETSELITEMS, 1, (LPARAM)&lb_sel);
          lb_max = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_INC), LB_GETCOUNT, 0, 0) - 1;
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), (lb_sel > 0));
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), (lb_sel < lb_max));
        } else {
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), false);
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), false);
        }

      }
      if(HIWORD(wParam) == LBN_DBLCLK) {
        this->_remPkg();
      }
      break;

    case IDC_BC_ADD:
      this->_addPkg();
      break;

    case IDC_BC_DEL:
      this->_remPkg();
      break;

    case IDC_BC_UP:
      this->_upPkg();
      break;

    case IDC_BC_DN:
      this->_dnPkg();
      break;

    case IDC_BC_OK:
      if(this->_apply()) {
        this->quit();
      }
      break;

    case IDC_BC_CANCEL:
      this->quit();
      break;
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewBat::_apply()
{
  // build the per-Location hash lists
  vector<vector<uint64_t>> loc_hash_lsts;

  OmPackage* package;

  for(unsigned l = 0; l < this->_context->locationCount(); ++l) {

    // new hash list
    vector<uint64_t> hash_list;

    for(size_t i = 0; i < this->_incLs[l].size(); ++i) {

      // retrieve package from stored index
      package = this->_context->location(l)->package(this->_incLs[l][i]);

      // add <install> entry with package hash
      hash_list.push_back(package->hash());
    }

    // add hash list
    loc_hash_lsts.push_back(hash_list);
  }

  wchar_t wcbuf[OMM_MAX_PATH];
  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);

  // try to create a new batch
  if(!this->_context->makeBatch(wcbuf, loc_hash_lsts)) {
    Om_dialogBoxErr(this->_hwnd, L"Batch creation failed", this->_context->lastError());
    return false;
  }

  // refresh parent
  this->_parent->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewBat::_upPkg()
{
  // get current Combo-Box selection first Location by default
  int l = SendMessageW(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), CB_GETCURSEL, 0, 0);

  if(l < 0)
    return;

  HWND hlsr = GetDlgItem(this->_hwnd, IDC_LB_INC);

  // get count of selected items
  int n = SendMessageW(hlsr, LB_GETSELCOUNT, 0, 0);

  if(n == 1) {

    // get list of selected item (index)
    int lb_sel;
    SendMessageW(hlsr, LB_GETSELITEMS, 1, (LPARAM)&lb_sel);

    // check whether we can move up
    if(lb_sel == 0)
      return;

    wchar_t wcbuf[OMM_MAX_PATH];

    // retrieve the package List-Box label
    SendMessageW(hlsr, LB_GETTEXT, lb_sel - 1, (LPARAM)wcbuf);
    int index = SendMessageW(hlsr, LB_GETITEMDATA, lb_sel - 1, 0);

    SendMessageW(hlsr, LB_DELETESTRING, lb_sel - 1, 0);

    SendMessageW(hlsr, LB_INSERTSTRING, lb_sel, (LPARAM)wcbuf);
    SendMessageW(hlsr, LB_SETITEMDATA, lb_sel, (LPARAM)index);

    // swap package index to move up
    for(size_t k = 0; k < this->_incLs[l].size(); ++k) {
      if(this->_incLs[l][k] == index) {
        int temp = this->_incLs[l][k];
        this->_incLs[l][k] = this->_incLs[l][k+1];
        this->_incLs[l][k+1] = temp;
        break;
      }
    }

    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), (lb_sel > 1));
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), true);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewBat::_dnPkg()
{
  // get current Combo-Box selection first Location by default
  int l = SendMessageW(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), CB_GETCURSEL, 0, 0);

  if(l < 0)
    return;

  HWND hlsr = GetDlgItem(this->_hwnd, IDC_LB_INC);

  // get count of selected items
  int n = SendMessageW(hlsr, LB_GETSELCOUNT, 0, 0);

  // get count of item in List-Box as index to for insertion
  int lb_max = SendMessageW(hlsr, LB_GETCOUNT, 0, 0) - 1;

  if(n == 1) {

    // get list of selected item (index)
    int lb_sel;
    SendMessageW(hlsr, LB_GETSELITEMS, 1, (LPARAM)&lb_sel);

    // check whether we can move down
    if(lb_sel == lb_max)
      return;

    wchar_t wcbuf[OMM_MAX_PATH];

    // retrieve the package List-Box label
    SendMessageW(hlsr, LB_GETTEXT, lb_sel, (LPARAM)wcbuf);
    int index = SendMessageW(hlsr, LB_GETITEMDATA, lb_sel, 0);

    SendMessageW(hlsr, LB_DELETESTRING, lb_sel, 0);

    lb_sel++;

    SendMessageW(hlsr, LB_INSERTSTRING, lb_sel, (LPARAM)wcbuf);
    SendMessageW(hlsr, LB_SETITEMDATA, lb_sel, (LPARAM)index);
    SendMessageW(hlsr, LB_SETSEL , true, (LPARAM)(lb_sel));

    // swap package index to move up
    for(size_t k = 0; k < this->_incLs[l].size(); ++k) {
      if(this->_incLs[l][k] == index) {
        int temp = this->_incLs[l][k];
        this->_incLs[l][k] = this->_incLs[l][k+1];
        this->_incLs[l][k+1] = temp;
        break;
      }
    }

    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), true);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), (lb_sel < lb_max));
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewBat::_addPkg()
{
  // get current Combo-Box selection first Location by default
  int l = SendMessageW(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), CB_GETCURSEL, 0, 0);

  if(l < 0)
    return;

  HWND hlsl = GetDlgItem(this->_hwnd, IDC_LB_EXC);
  HWND hlsr = GetDlgItem(this->_hwnd, IDC_LB_INC);

  // get count of selected items
  int n = SendMessageW(hlsl, LB_GETSELCOUNT, 0, 0);

  if(n > 0) {

    // get list of selected items (index)
    int* lb_sel = new int[n];
    SendMessageW(hlsl, LB_GETSELITEMS, n, (LPARAM)lb_sel);

    int index, pos;
    wchar_t wcbuf[OMM_MAX_PATH];

    // copy selected items from one list to the other list
    for(int i = 0; i < n; ++i) {

      // retrieve the package List-Box label
      SendMessageW(hlsl, LB_GETTEXT, lb_sel[i], (LPARAM)wcbuf);
      // retrieve the package reference index (in Location package list)
      index = SendMessageW(hlsl, LB_GETITEMDATA, lb_sel[i], 0);

      // remove package index from left mirror list
      for(size_t k = 0; k < this->_excLs[l].size(); ++k) {
        if(this->_excLs[l][k] == index) {
          this->_excLs[l].erase(this->_excLs[l].begin()+k);
          break;
        }
      }
      // add package index to right mirror list
      this->_incLs[l].push_back(index);

      // get count of item in List-Box as index to for insertion
      pos = SendMessageW(hlsr, LB_GETCOUNT, 0, 0);
      // add item to the List-Box
      SendMessageW(hlsr, LB_ADDSTRING, pos, (LPARAM)wcbuf);
      SendMessageW(hlsr, LB_SETITEMDATA, pos, (LPARAM)index);
    }

    // remove items from List-Box in reverse order to prevent indexing issues
    int i = n;
    while(i--) {
      SendMessageW(hlsl, LB_DELETESTRING, lb_sel[i], 0);
    }

    // we do not need list-box selection anymore
    delete [] lb_sel;

    // disable button until new selection
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ADD), false);
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewBat::_remPkg()
{
  // get current Combo-Box selection first Location by default
  int l = SendMessageW(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), CB_GETCURSEL, 0, 0);

  if(l < 0)
    return;

  HWND hlsl = GetDlgItem(this->_hwnd, IDC_LB_EXC);
  HWND hlsr = GetDlgItem(this->_hwnd, IDC_LB_INC);

  // get count of selected items
  int n = SendMessageW(hlsr, LB_GETSELCOUNT, 0, 0);

  if(n > 0) {

    // get list of selected items (index)
    int* lb_sel = new int[n];
    SendMessageW(hlsr, LB_GETSELITEMS, n, (LPARAM)lb_sel);

    int index, pos;
    wchar_t wcbuf[OMM_MAX_PATH];

    // copy selected items from one list to the other list
    for(int i = 0; i < n; ++i) {
      // retrieve the package List-Box label
      SendMessageW(hlsr, LB_GETTEXT, lb_sel[i], (LPARAM)wcbuf);
      // retrieve the package reference index (in Location package list)
      index = SendMessageW(hlsr, LB_GETITEMDATA, lb_sel[i], 0);

      // remove package index from right mirror list
      for(size_t k = 0; k < this->_incLs[l].size(); ++k) {
        if(this->_incLs[l][k] == index) {
          this->_incLs[l].erase(this->_incLs[l].begin()+k);
          break;
        }
      }
      // add package index to left mirror list
      this->_excLs[l].push_back(index);

      // get count of item in List-Box as index to for insertion
      pos = SendMessageW(hlsl, LB_GETCOUNT, 0, 0);
      // add item to the List-Box
      SendMessageW(hlsl, LB_ADDSTRING, pos, (LPARAM)wcbuf);
      SendMessageW(hlsl, LB_SETITEMDATA, pos, (LPARAM)index);
    }

    // remove items from List-Box in reverse order to prevent indexing issues
    int i = n;
    while(i--) {
      SendMessageW(hlsr, LB_DELETESTRING, lb_sel[i], 0);
    }

    // we do not need list-box selection anymore
    delete [] lb_sel;

    // disable button until new selection
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), false);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewBat::_rebuildPkgLb()
{
  if(this->_context == nullptr)
    return;

  // get current Combo-Box selection first Location by default
  int l = SendMessageW(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), CB_GETCURSEL, 0, 0);

  if(l < 0)
    return;

  // get Location corresponding to current selection
  OmLocation* location = this->_context->location(l);

  // hold handle to List-Box controls
  HWND hlsl = GetDlgItem(this->_hwnd, IDC_LB_EXC);
  HWND hlsr = GetDlgItem(this->_hwnd, IDC_LB_INC);

  unsigned p;
  OmPackage* package;
  wstring label;

  // reset List-Box control
  SendMessageW(hlsl, LB_RESETCONTENT, 0, 0);

  // fill the left List-Box
  for(size_t i = 0; i < this->_excLs[l].size(); i++) {

    p = this->_excLs[l][i];
    package = location->package(p);

    label = Om_getFilePart(package->sourcePath());
    SendMessageW(hlsl, LB_ADDSTRING, i, (LPARAM)label.c_str());
    SendMessageW(hlsl, LB_SETITEMDATA, i, (LPARAM)p);
  }

  // reset List-Box control
  SendMessageW(hlsr, LB_RESETCONTENT, 0, 0);

  // fill the left List-Box
  for(size_t i = 0; i < this->_incLs[l].size(); i++) {

    p = this->_incLs[l][i];
    package = location->package(p);

    label = Om_getFilePart(package->sourcePath());
    SendMessageW(hlsr, LB_ADDSTRING, i, (LPARAM)label.c_str());
    SendMessageW(hlsr, LB_SETITEMDATA, i, (LPARAM)p);
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewBat::_qucikFromCur()
{
  if(this->_context == nullptr)
    return;

  OmLocation* location;
  OmPackage* package;

  // add Location(s) to Combo-Box
  for(unsigned l = 0; l < this->_context->locationCount(); ++l) {

    location = this->_context->location(l);

    this->_excLs[l].clear();
    this->_incLs[l].clear();

    for(unsigned p = 0; p < location->packageCount(); ++p) {

      package = location->package(p);

      if(package->hasSource()) {
        if(package->hasBackup()) {
          this->_incLs[l].push_back(p);
        } else {
          this->_excLs[l].push_back(p);
        }
      }
    }
  }

  // refill the List-Box controls with new values
  this->_rebuildPkgLb();
}

