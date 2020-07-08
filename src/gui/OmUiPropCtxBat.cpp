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
#include "OmManager.h"
#include "gui/OmUiMain.h"
#include "gui/OmUiNewBat.h"
#include "gui/OmUiPropBat.h"
#include "gui/OmUiPropCtx.h"
#include "gui/OmUiPropCtxBat.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtxBat::OmUiPropCtxBat(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i) {
    this->_chParam[i] = false;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtxBat::~OmUiPropCtxBat()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropCtxBat::id() const
{
  return IDD_PROP_CTX_BAT;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::setChParam(unsigned i, bool en)
{
  this->_chParam[i] = en;
  reinterpret_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_batchUp()
{
  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_BATLS);

  // get selected item (index)
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t buf[512];
  int idx;

  // retrieve the package List-Box label
  SendMessageW(hLb, LB_GETTEXT, lb_sel - 1, (LPARAM)buf);
  idx = SendMessageW(hLb, LB_GETITEMDATA, lb_sel - 1, 0);

  SendMessageW(hLb, LB_DELETESTRING, lb_sel - 1, 0);

  SendMessageW(hLb, LB_INSERTSTRING, lb_sel, (LPARAM)buf);
  SendMessageW(hLb, LB_SETITEMDATA, lb_sel, (LPARAM)idx);

  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), (lb_sel > 1));
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_batchDn()
{
  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_BATLS);

  // get selected item (index)
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);
  // get count of item in List-Box as index to for insertion
  int lb_max = SendMessageW(hLb, LB_GETCOUNT, 0, 0) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t buf[512];
  int idx;

  SendMessageW(hLb, LB_GETTEXT, lb_sel, (LPARAM)buf);
  idx = SendMessageW(hLb, LB_GETITEMDATA, lb_sel, 0);
  SendMessageW(hLb, LB_DELETESTRING, lb_sel, 0);

  lb_sel++;

  SendMessageW(hLb, LB_INSERTSTRING, lb_sel, (LPARAM)buf);
  SendMessageW(hLb, LB_SETITEMDATA, lb_sel, (LPARAM)idx);
  SendMessageW(hLb, LB_SETCURSEL , true, (LPARAM)(lb_sel));

  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), true);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), (lb_sel < lb_max));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtxBat::_batchDel()
{
  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_BATLS);

  // get selected item (index)
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  if(lb_sel >= 0) {

    unsigned bat_id = SendMessageW(hLb, LB_GETITEMDATA, lb_sel, 0);

    OmContext* ctx = reinterpret_cast<OmUiPropCtx*>(this->_parent)->context();

    // warns the user before committing the irreparable
    wstring msg;
    msg = L"Are your sure you want to delete the Batch '";
    msg += ctx->batch(bat_id)->title();
    msg += L"' ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Batch", msg)) {
      return false;
    }

    if(!ctx->deleteBatch(bat_id)) {
      Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Batch failed", ctx->lastError());
      return false;
    }

    // refresh the main window dialog, this will also refresh this one
    this->root()->refresh();

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onShow()
{
  // define controls tool-tips
  this->_createTooltip(IDC_LB_BATLS,  L"Context's batches");

  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");

  this->_createTooltip(IDC_BC_DEL,    L"Delete batch");
  this->_createTooltip(IDC_BC_ADD,    L"Create new batch");
  this->_createTooltip(IDC_BC_EDIT,   L"Batch properties");


  HBITMAP hBm;

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_ADD), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_ADD), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_REM), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_DEL), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_MOD), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_EDIT), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_UP), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_UP), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_DN), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_DN), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onResize()
{
  // Locations list Label & ListBox
  this->_setControlPos(IDC_SC_LBL01, 5, 20, 64, 9);
  this->_setControlPos(IDC_LB_BATLS, 70, 20, this->width()-107, 85);
  // Up and Down buttons
  this->_setControlPos(IDC_BC_UP, this->width()-35, 49, 16, 15);
  this->_setControlPos(IDC_BC_DN, this->width()-35, 65, 16, 15);


  // Remove & Modify Buttons
  this->_setControlPos(IDC_BC_DEL, 70, 110, 50, 14);
  this->_setControlPos(IDC_BC_EDIT, 122, 110, 50, 14);
  // Add button
  this->_setControlPos(IDC_BC_ADD, this->width()-87, 110, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onRefresh()
{
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_EDIT), false);

  OmContext* context = reinterpret_cast<OmUiPropCtx*>(this->_parent)->context();

  if(context == nullptr)
    return;

  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_BATLS);

  SendMessage(hLb, LB_RESETCONTENT, 0, 0);
  if(context) {
    for(unsigned i = 0; i < context->batchCount(); ++i) {
      SendMessageW(hLb, LB_ADDSTRING, i, (LPARAM)context->batch(i)->title().c_str());
      SendMessageW(hLb, LB_SETITEMDATA, i, i); // for Location index reordering
    }
  }
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtxBat::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    OmContext* context = reinterpret_cast<OmUiPropCtx*>(this->_parent)->context();

    if(context == nullptr)
      return false;

    int lb_sel, lb_max;

    switch(LOWORD(wParam))
    {

    case IDC_LB_BATLS: //< Location(s) list List-Box
      lb_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_BATLS), LB_GETCURSEL, 0, 0);
      if(lb_sel >= 0) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_EDIT), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), (lb_sel > 0));
        lb_max = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_BATLS), LB_GETCOUNT, 0, 0) - 1;
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), (lb_sel < lb_max));
      }
      break;

    case IDC_BC_ADD: //< New button for Location(s) list
      {
        OmUiNewBat* uiNewBat = reinterpret_cast<OmUiNewBat*>(this->siblingById(IDD_NEW_BAT));
        uiNewBat->setContext(context);
        uiNewBat->open(true);
      }
      break;

    case IDC_BC_EDIT:
      lb_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_BATLS), LB_GETCURSEL, 0, 0);
      if(lb_sel >= 0 && lb_sel < (int)context->batchCount()) {
        // open the Location Properties dialog
        int bat_id = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_BATLS), LB_GETITEMDATA, lb_sel, 0);
        OmUiPropBat* uiPropBat = reinterpret_cast<OmUiPropBat*>(this->siblingById(IDD_PROP_BAT));
        uiPropBat->setBatch(context->batch(bat_id));
        uiPropBat->open();
      }
      break;

    case IDC_BC_DEL: //< Remove button for Batches list
      this->_batchDel();
      break;


    case IDC_BC_UP:
      this->_batchUp();
      // user modified parameter, notify it
      this->setChParam(CTX_PROP_BAT_ORDER, true);
      break;

    case IDC_BC_DN:
      this->_batchDn();
      // user modified parameter, notify it
      this->setChParam(CTX_PROP_BAT_ORDER, true);
      break;
    }
  }

  return false;
}

