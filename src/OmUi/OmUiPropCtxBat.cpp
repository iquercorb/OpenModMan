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

#include "OmManager.h"

#include "OmUiMgr.h"
#include "OmUiAddBat.h"
#include "OmUiPropBat.h"
#include "OmUiPropCtx.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropCtxBat.h"

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
  int lb_sel = this->msgItem(IDC_LB_BAT, LB_GETCURSEL);

  if(lb_sel >= 0) {

    this->enableItem(IDC_BC_DEL, true);
    this->enableItem(IDC_BC_EDI, true);

    this->enableItem(IDC_BC_UP, (lb_sel > 0));
    int lb_max = this->msgItem(IDC_LB_BAT, LB_GETCOUNT) - 1;
    this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onBcUpBat()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_BAT, LB_GETCURSEL);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_BAT, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_BAT, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_BAT, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_BAT, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_BAT, LB_SETITEMDATA, lb_sel, idx);

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
  int lb_sel = this->msgItem(IDC_LB_BAT, LB_GETCURSEL);
  // get count of item in List-Box as index to for insertion
  int lb_max = this->msgItem(IDC_LB_BAT, LB_GETCOUNT) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  this->msgItem(IDC_LB_BAT, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_BAT, LB_GETITEMDATA, lb_sel);
  this->msgItem(IDC_LB_BAT, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_BAT, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_BAT, LB_SETITEMDATA, lb_sel, idx);
  this->msgItem(IDC_LB_BAT, LB_SETCURSEL, lb_sel);

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
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  int lb_sel = this->msgItem(IDC_LB_BAT, LB_GETCURSEL);

  if(lb_sel >= 0 && lb_sel < (int)pCtx->batCount()) {

    // open the Batch Properties dialog
    int bat_id = this->msgItem(IDC_LB_BAT, LB_GETITEMDATA, lb_sel, 0);
    OmUiPropBat* pUiPropBat = static_cast<OmUiPropBat*>(this->siblingById(IDD_PROP_BAT));
    pUiPropBat->batSet(pCtx->batGet(bat_id));
    pUiPropBat->open();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onBcDelBat()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_BAT, LB_GETCURSEL);

  if(lb_sel >= 0) {

    int bat_id = this->msgItem(IDC_LB_BAT, LB_GETITEMDATA, lb_sel, 0);

    // warns the user before committing the irreparable
    if(!Om_dlgBox_ynl(this->_hwnd, L"Software Context properties", IDI_QRY,
              L"Delete Installation Batch", L"Delete the Installation Batch ?",
              pCtx->batGet(bat_id)->title()))
    {
      return;
    }

    if(!pCtx->batRem(bat_id)) {

      // warns the user error occurred
      Om_dlgBox_okl(this->_hwnd, L"Software Context properties", IDI_ERR,
                L"Installation Batch delete error", L"Installation Batch deletion "
                "process failed because of the following error:",
                pCtx->lastError());

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
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  OmUiAddBat* pUiNewBat = static_cast<OmUiAddBat*>(this->siblingById(IDD_ADD_BAT));
  pUiNewBat->ctxSet(pCtx);
  pUiNewBat->open(true);
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onCkBoxQuiet()
{
  // user modified parameter, notify it
  this->setChParam(CTX_PROP_BAT_QUIETMODE, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onInit()
{
  this->setBmIcon(IDC_BC_ADD, Om_getResIcon(this->_hins, IDI_BT_ADD));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDI_BT_REM));
  this->setBmIcon(IDC_BC_EDI, Om_getResIcon(this->_hins, IDI_BT_MOD));
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(this->_hins, IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(this->_hins, IDI_BT_DN));

  // Define controls tool-tips
  this->_createTooltip(IDC_LB_BAT,    L"Installation batches list");
  this->_createTooltip(IDC_BC_UP,     L"Move up in list");
  this->_createTooltip(IDC_BC_DN,     L"Move down in list");
  this->_createTooltip(IDC_BC_DEL,    L"Delete the selected Installation batch");
  this->_createTooltip(IDC_BC_ADD,    L"Create a new Installation batch for this Context");
  this->_createTooltip(IDC_BC_EDI,    L"Edit Installation batch properties");
  this->_createTooltip(IDC_BC_CKBX1,  L"Disable installation warning messages for batches execution.");

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
  this->_setItemPos(IDC_SC_LBL01, 50, 15, 180, 9);
  this->_setItemPos(IDC_LB_BAT, 50, 25, this->cliUnitX()-107, 58);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliUnitX()-55, 39, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->cliUnitX()-55, 55, 16, 15);

  // Remove & Modify Buttons
  this->_setItemPos(IDC_BC_DEL, 50, 85, 50, 14);
  this->_setItemPos(IDC_BC_EDI, 105, 85, 50, 14);
  // Add button
  this->_setItemPos(IDC_BC_ADD, this->cliUnitX()-108, 85, 50, 14);

  // Options label
  //this->_setItemPos(IDC_SC_LBL02, 50, 120, 180, 9);
  // Quiet Batches checkbox
  this->_setItemPos(IDC_BC_CKBX1, 50, 125, 240, 9);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxBat::_onRefresh()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  this->msgItem(IDC_LB_BAT, LB_RESETCONTENT);

  for(unsigned i = 0; i < pCtx->batCount(); ++i) {
    this->msgItem(IDC_LB_BAT, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(pCtx->batGet(i)->title().c_str()));
    this->msgItem(IDC_LB_BAT, LB_SETITEMDATA, i, i); // for Location index reordering
  }

  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, pCtx->batQuietMode());

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
INT_PTR OmUiPropCtxBat::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_BAT: //< Location(s) list List-Box
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

    case IDC_BC_CKBX1:
      this->_onCkBoxQuiet();
      break;
    }
  }

  return false;
}

