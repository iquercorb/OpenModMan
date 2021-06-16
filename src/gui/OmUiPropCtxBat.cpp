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
#include "gui/OmUiAddBat.h"
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
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onLbBatlsSel()
{
  int lb_sel = this->msgItem(IDC_LB_BATLS, LB_GETCURSEL);

  if(lb_sel >= 0) {

    this->enableItem(IDC_BC_DEL, true);
    this->enableItem(IDC_BC_EDI, true);

    this->enableItem(IDC_BC_UP, (lb_sel > 0));
    int lb_max = this->msgItem(IDC_LB_BATLS, LB_GETCOUNT) - 1;
    this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onBcUpBat()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_BATLS, LB_GETCURSEL);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_BATLS, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_BATLS, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_BATLS, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_BATLS, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_BATLS, LB_SETITEMDATA, lb_sel, idx);

  this->enableItem(IDC_BC_UP, (lb_sel > 1));
  this->enableItem(IDC_BC_DN, true);

  // user modified parameter, notify it
  this->setChParam(CTX_PROP_BAT_ORDER, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onBcDnBat()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_BATLS, LB_GETCURSEL);
  // get count of item in List-Box as index to for insertion
  int lb_max = this->msgItem(IDC_LB_BATLS, LB_GETCOUNT) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  this->msgItem(IDC_LB_BATLS, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_BATLS, LB_GETITEMDATA, lb_sel);
  this->msgItem(IDC_LB_BATLS, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_BATLS, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_BATLS, LB_SETITEMDATA, lb_sel, idx);
  this->msgItem(IDC_LB_BATLS, LB_SETCURSEL, true, lb_sel);

  this->enableItem(IDC_BC_UP, true);
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));

  // user modified parameter, notify it
  this->setChParam(CTX_PROP_BAT_ORDER, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onBcEdiBat()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->context();
  if(!pCtx) return;

  int lb_sel = this->msgItem(IDC_LB_BATLS, LB_GETCURSEL);

  if(lb_sel >= 0 && lb_sel < (int)pCtx->batchCount()) {

    // open the Batch Properties dialog
    int bat_id = this->msgItem(IDC_LB_BATLS, LB_GETITEMDATA, lb_sel, 0);
    OmUiPropBat* pUiPropBat = static_cast<OmUiPropBat*>(this->siblingById(IDD_PROP_BAT));
    pUiPropBat->setBatch(pCtx->batch(bat_id));
    pUiPropBat->open();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onBcDelBat()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->context();
  if(!pCtx) return;

  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_BATLS, LB_GETCURSEL);

  if(lb_sel >= 0) {

    int bat_id = this->msgItem(IDC_LB_BATLS, LB_GETITEMDATA, lb_sel, 0);

    // warns the user before committing the irreparable
    wstring qry = L"Delete the Batch \"";
    qry += pCtx->batch(bat_id)->title();
    qry += L"\" ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Batch", qry)) {
      return;
    }

    if(!pCtx->remBatch(bat_id)) {
      Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Batch failed", pCtx->lastError());
      return;
    }

    // refresh the main window dialog, this will also refresh this one
    this->root()->refresh();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onBcAddBat()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->context();
  if(!pCtx) return;

  OmUiAddBat* pUiNewBat = static_cast<OmUiAddBat*>(this->siblingById(IDD_ADD_BAT));
  pUiNewBat->setContext(pCtx);
  pUiNewBat->open(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onInit()
{
  this->setBmImage(IDC_BC_ADD, Om_getResImage(this->_hins, IDB_BTN_ADD));
  this->setBmImage(IDC_BC_DEL, Om_getResImage(this->_hins, IDB_BTN_REM));
  this->setBmImage(IDC_BC_EDI, Om_getResImage(this->_hins, IDB_BTN_MOD));
  this->setBmImage(IDC_BC_UP, Om_getResImage(this->_hins, IDB_BTN_UP));
  this->setBmImage(IDC_BC_DN, Om_getResImage(this->_hins, IDB_BTN_DN));

  // Define controls tool-tips
  this->_createTooltip(IDC_LB_BATLS,  L"Context's batches");

  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");

  this->_createTooltip(IDC_BC_DEL,    L"Delete batch");
  this->_createTooltip(IDC_BC_ADD,    L"Create new batch");
  this->_createTooltip(IDC_BC_EDI,   L"Batch properties");

  // Set controls default states and parameters
  this->enableItem(IDC_BC_DEL, false);
  this->enableItem(IDC_BC_EDI, false);

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
  this->_setItemPos(IDC_BC_EDI, 122, 110, 50, 14);
  // Add button
  this->_setItemPos(IDC_BC_ADD, this->width()-87, 110, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onRefresh()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->context();
  if(!pCtx) return;

  this->msgItem(IDC_LB_BATLS, LB_RESETCONTENT);

  for(unsigned i = 0; i < pCtx->batchCount(); ++i) {
    this->msgItem(IDC_LB_BATLS, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(pCtx->batch(i)->title().c_str()));
    this->msgItem(IDC_LB_BATLS, LB_SETITEMDATA, i, i); // for Location index reordering
  }

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
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

    switch(LOWORD(wParam))
    {
    case IDC_LB_BATLS: //< Location(s) list List-Box
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_onLbBatlsSel();
      break;

    case IDC_BC_UP: // Down Button
      this->_onBcUpBat();
      break;

    case IDC_BC_DN: // Up Button
      this->_onBcDnBat();
      break;

    case IDC_BC_EDI: // "Modify" Button
      this->_onBcEdiBat();
      break;

    case IDC_BC_DEL: //< "Remove" Button
      this->_onBcDelBat();
      break;

    case IDC_BC_ADD: //< "New" Button
      this->_onBcAddBat();
      break;
    }
  }

  return false;
}

