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
OmUiPropCtxBat::OmUiPropCtxBat(HINSTANCE hins) : OmDialog(hins),
  _hBmBcNew(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_ADD), IMAGE_BITMAP, 0, 0, 0))),
  _hBmBcDel(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_REM), IMAGE_BITMAP, 0, 0, 0))),
  _hBmBcMod(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_MOD), IMAGE_BITMAP, 0, 0, 0))),
  _hBmBcUp(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_UP), IMAGE_BITMAP, 0, 0, 0))),
  _hBmBcDn(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_DN), IMAGE_BITMAP, 0, 0, 0)))
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
  DeleteObject(this->_hBmBcNew);
  DeleteObject(this->_hBmBcDel);
  DeleteObject(this->_hBmBcMod);
  DeleteObject(this->_hBmBcUp);
  DeleteObject(this->_hBmBcDn);
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
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_batchUp()
{
  HWND hLb = this->getItem(IDC_LB_BATLS);

  // get selected item (index)
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  // retrieve the package List-Box label
  SendMessageW(hLb, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  idx = SendMessageW(hLb, LB_GETITEMDATA, lb_sel - 1, 0);

  SendMessageW(hLb, LB_DELETESTRING, lb_sel - 1, 0);

  SendMessageW(hLb, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  SendMessageW(hLb, LB_SETITEMDATA, lb_sel, idx);

  this->enableItem(IDC_BC_UP, (lb_sel > 1));
  this->enableItem(IDC_BC_DN, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_batchDn()
{
  HWND hLb = this->getItem(IDC_LB_BATLS);

  // get selected item (index)
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);
  // get count of item in List-Box as index to for insertion
  int lb_max = SendMessageW(hLb, LB_GETCOUNT, 0, 0) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  SendMessageW(hLb, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  idx = SendMessageW(hLb, LB_GETITEMDATA, lb_sel, 0);
  SendMessageW(hLb, LB_DELETESTRING, lb_sel, 0);

  lb_sel++;

  SendMessageW(hLb, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  SendMessageW(hLb, LB_SETITEMDATA, lb_sel, idx);
  SendMessageW(hLb, LB_SETCURSEL, true, lb_sel);

  this->enableItem(IDC_BC_UP, true);
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtxBat::_batchDel()
{
  HWND hLb = this->getItem(IDC_LB_BATLS);

  // get selected item (index)
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  if(lb_sel >= 0) {

    unsigned bat_id = SendMessageW(hLb, LB_GETITEMDATA, lb_sel, 0);

    OmContext* ctx = static_cast<OmUiPropCtx*>(this->_parent)->context();

    // warns the user before committing the irreparable
    wstring qry = L"Are your sure you want to delete the Batch \"";
    qry += ctx->batch(bat_id)->title();
    qry += L"\" ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Batch", qry)) {
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
void OmUiPropCtxBat::_onInit()
{
  // Set buttons inner icons
  this->msgItem(IDC_BC_ADD,   BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcNew));
  this->msgItem(IDC_BC_DEL,   BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcDel));
  this->msgItem(IDC_BC_EDIT,  BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcMod));
  this->msgItem(IDC_BC_UP,    BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcUp));
  this->msgItem(IDC_BC_DN,    BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcDn));

  // Define controls tool-tips
  this->_createTooltip(IDC_LB_BATLS,  L"Context's batches");

  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");

  this->_createTooltip(IDC_BC_DEL,    L"Delete batch");
  this->_createTooltip(IDC_BC_ADD,    L"Create new batch");
  this->_createTooltip(IDC_BC_EDIT,   L"Batch properties");

  // Set controls default states and parameters
  this->enableItem(IDC_BC_DEL, false);
  this->enableItem(IDC_BC_EDIT, false);

  // Update values
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onResize()
{
  // Locations list Label & ListBox
  this->_setItemPos(IDC_SC_LBL01, 5, 20, 64, 9);
  this->_setItemPos(IDC_LB_BATLS, 70, 20, this->width()-107, 85);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->width()-35, 49, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->width()-35, 65, 16, 15);

  // Remove & Modify Buttons
  this->_setItemPos(IDC_BC_DEL, 70, 110, 50, 14);
  this->_setItemPos(IDC_BC_EDIT, 122, 110, 50, 14);
  // Add button
  this->_setItemPos(IDC_BC_ADD, this->width()-87, 110, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onRefresh()
{
  OmContext* context = static_cast<OmUiPropCtx*>(this->_parent)->context();

  if(context == nullptr)
    return;

  HWND hLb = this->getItem(IDC_LB_BATLS);

  SendMessageW(hLb, LB_RESETCONTENT, 0, 0);
  if(context) {
    for(unsigned i = 0; i < context->batchCount(); ++i) {
      SendMessageW(hLb, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(context->batch(i)->title().c_str()));
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

    OmContext* context = static_cast<OmUiPropCtx*>(this->_parent)->context();

    if(context == nullptr)
      return false;

    int lb_sel, lb_max;

    switch(LOWORD(wParam))
    {

    case IDC_LB_BATLS: //< Location(s) list List-Box
      lb_sel = this->msgItem(IDC_LB_BATLS, LB_GETCURSEL);
      if(lb_sel >= 0) {
        this->enableItem(IDC_BC_DEL, true);
        this->enableItem(IDC_BC_EDIT, true);
        this->enableItem(IDC_BC_UP, (lb_sel > 0));
        lb_max = this->msgItem(IDC_LB_BATLS, LB_GETCOUNT) - 1;
        this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
      }
      break;

    case IDC_BC_ADD: //< New button for Location(s) list
      {
        OmUiNewBat* uiNewBat = static_cast<OmUiNewBat*>(this->siblingById(IDD_NEW_BAT));
        uiNewBat->setContext(context);
        uiNewBat->open(true);
      }
      break;

    case IDC_BC_EDIT:
      lb_sel = this->msgItem(IDC_LB_BATLS, LB_GETCURSEL);
      if(lb_sel >= 0 && lb_sel < (int)context->batchCount()) {
        // open the Location Properties dialog
        int bat_id = this->msgItem(IDC_LB_BATLS, LB_GETITEMDATA, lb_sel, 0);
        OmUiPropBat* uiPropBat = static_cast<OmUiPropBat*>(this->siblingById(IDD_PROP_BAT));
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

