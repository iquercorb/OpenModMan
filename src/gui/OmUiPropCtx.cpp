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
#include "gui/OmUiPropCtx.h"
#include "gui/OmUiPropCtxStg.h"
#include "gui/OmUiPropCtxLoc.h"
#include "gui/OmUiPropCtxBat.h"
#include "gui/OmUiNewLoc.h"
#include "gui/OmUiPropLoc.h"
#include "gui/OmUiNewBat.h"
#include "gui/OmUiPropBat.h"
#include "gui/OmUiProgress.h"



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtx::OmUiPropCtx(HINSTANCE hins) : OmDialogProp(hins),
  _context(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"Settings", new OmUiPropCtxStg(hins));
  this->_addPage(L"Locations", new OmUiPropCtxLoc(hins));
  this->_addPage(L"Batches", new OmUiPropCtxBat(hins));

  this->addChild(new OmUiNewLoc(hins));     //< Dialog for Location creation
  this->addChild(new OmUiNewBat(hins));     //< Dialog for Batch creation
  this->addChild(new OmUiPropLoc(hins));    //< Dialog for Location properties
  this->addChild(new OmUiPropBat(hins));    //< Dialog for Location properties
  this->addChild(new OmUiProgress(hins));   //< for Location backup cleaning
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtx::~OmUiPropCtx()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropCtx::id() const
{
  return IDD_PROP_CTX;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtx::checkChanges()
{
  OmContext* context = reinterpret_cast<OmContext*>(this->_context);
  OmUiPropCtxStg* uiPropCtxStg  = reinterpret_cast<OmUiPropCtxStg*>(this->childById(IDD_PROP_CTX_STG));
  OmUiPropCtxLoc* uiPropCtxLoc  = reinterpret_cast<OmUiPropCtxLoc*>(this->childById(IDD_PROP_CTX_LOC));
  OmUiPropCtxBat* uiPropCtxBat  = reinterpret_cast<OmUiPropCtxBat*>(this->childById(IDD_PROP_CTX_BAT));

  bool changed = false;

  wchar_t wcbuf[OMM_MAX_PATH];

  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_TITLE)) {  //< parameter for Context title
    GetDlgItemTextW(uiPropCtxStg->hwnd(), IDC_EC_INPT3, wcbuf, OMM_MAX_PATH);
    if(context->title() != wcbuf) {
      changed = true;
    } else {
      uiPropCtxStg->setChParam(CTX_PROP_STG_TITLE, false);
    }
  }

  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_ICON)) { // parameter for Context icon
    changed = true;
  }

  if(uiPropCtxLoc->hasChParam(CTX_PROP_LOC_ORDER)) { // parameter for Location index order
    changed = true;
  }

  if(uiPropCtxBat->hasChParam(CTX_PROP_BAT_ORDER)) { // parameter for Location index order
    changed = true;
  }

  // enable Apply button
  if(IsWindowEnabled(GetDlgItem(this->_hwnd, IDC_BC_APPLY)) != changed) {
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), changed);
  }

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtx::applyChanges()
{
  OmContext* context = reinterpret_cast<OmContext*>(this->_context);
  OmUiPropCtxStg* uiPropCtxStg  = reinterpret_cast<OmUiPropCtxStg*>(this->childById(IDD_PROP_CTX_STG));
  OmUiPropCtxLoc* uiPropCtxLoc  = reinterpret_cast<OmUiPropCtxLoc*>(this->childById(IDD_PROP_CTX_LOC));
  OmUiPropCtxBat* uiPropCtxBat  = reinterpret_cast<OmUiPropCtxBat*>(this->childById(IDD_PROP_CTX_BAT));

  wchar_t inpt3[OMM_MAX_PATH];
  wchar_t inpt4[OMM_MAX_PATH];

  // Step 1, verify everything
  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_TITLE)) { //< parameter for Context title
    GetDlgItemTextW(uiPropCtxStg->hwnd(), IDC_EC_INPT3, inpt3, OMM_MAX_PATH);
    if(!wcslen(inpt3)) {
      Om_dialogBoxErr(this->_hwnd, L"Invalid Context title",
                                   L"Please enter a title.");
      return false;
    }
  }

  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_ICON)) { // parameter for Context icon
    GetDlgItemTextW(uiPropCtxStg->hwnd(), IDC_EC_INPT4, inpt4, OMM_MAX_PATH);
  }

  // Step 2, save changes
  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_TITLE)) { //< parameter for Context title
    GetDlgItemTextW(uiPropCtxStg->hwnd(), IDC_EC_INPT3, inpt3, OMM_MAX_PATH);
    context->setTitle(inpt3);

    // Reset parameter as unmodified
    uiPropCtxStg->setChParam(CTX_PROP_STG_TITLE, false);
  }

  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_ICON)) { // parameter for Context icon
    if(wcslen(inpt4)) {
      if(Om_isFile(inpt4)) {
        context->setIcon(inpt4);
      } else {
        context->remIcon();
      }
    } else {
      context->remIcon();
    }

    // Reset parameter as unmodified
    uiPropCtxStg->setChParam(CTX_PROP_STG_ICON, false);
  }

  if(uiPropCtxLoc->hasChParam(CTX_PROP_LOC_ORDER)) { // parameter for Location index order

    // To prevent inconsistency we unselect location in the main dialog
    reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(true);

    HWND hLb = GetDlgItem(uiPropCtxLoc->hwnd(), IDC_LB_LOCLS);

    unsigned n = SendMessageW(hLb, LB_GETCOUNT, 0, 0);
    for(unsigned i = 0; i < n; ++i) {
      // set new index number of Location according current List-Box order
      context->location(SendMessageW(hLb,LB_GETITEMDATA,i,0))->setIndex(i);
    }

    // unselect Location in context
    context->selLocation(-1);
    // sort Location list
    context->sortLocations();

    // restore main dialog to normal state
    reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(false);

    // Reset parameter as unmodified
    uiPropCtxLoc->setChParam(CTX_PROP_LOC_ORDER, false);
  }

  if(uiPropCtxBat->hasChParam(CTX_PROP_BAT_ORDER)) { // parameter for Location index order

    HWND hLb = GetDlgItem(uiPropCtxBat->hwnd(), IDC_LB_BATLS);

    unsigned n = SendMessageW(hLb, LB_GETCOUNT, 0, 0);
    for(unsigned i = 0; i < n; ++i) {
      // set new index number of Location according current List-Box order
      context->batch(SendMessageW(hLb,LB_GETITEMDATA,i,0))->setIndex(i);
    }

    // sort Location list
    context->sortBatches();

    // Reset parameter as unmodified
    uiPropCtxBat->setChParam(CTX_PROP_LOC_ORDER, false);
  }

  // disable Apply button
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), false);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtx::_onShow()
{
  // Initialize TabControl with pages dialogs
  this->_pagesOnShow(IDC_TC_TABS1);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtx::_onResize()
{
  // TabControl
  this->_setControlPos(IDC_TC_TABS1, 4, 5, this->width()-8, this->height()-28);
  // Resize page dialogs according IDC_TC_TABS1
  this->_pagesOnResize(IDC_TC_TABS1);

  // OK Button
  this->_setControlPos(IDC_BC_OK, this->width()-161, this->height()-19, 50, 14);
  // Cancel Button
  this->_setControlPos(IDC_BC_CANCEL, this->width()-108, this->height()-19, 50, 14);
  // Apply Button
  this->_setControlPos(IDC_BC_APPLY, this->width()-54, this->height()-19, 50, 14);

  // force buttons to redraw to prevent artifacts
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_OK), nullptr, true);
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_CANCEL), nullptr, true);
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_APPLY), nullptr, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtx::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtx::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtx::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {
    // handle TabControl page selection change
    this->_pagesOnNotify(IDC_TC_TABS1, wParam, lParam);
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_APPLY:
      if(this->applyChanges()) {
        // refresh all tree from the main dialog
        this->root()->refresh();
      }
      break;

    case IDC_BC_OK:
      if(this->checkChanges()) {
        if(this->applyChanges()) {
          // quit the dialog
          this->quit();
          // refresh all tree from the main dialog
          this->root()->refresh();
        }
      } else {
        // quit the dialog
        this->quit();
      }
      break; // case BTN_OK:

    case IDC_BC_CANCEL:
      this->quit();
      break; // case BTN_CANCEL:
    }
  }

  return false;
}
