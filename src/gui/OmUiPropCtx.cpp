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
  OmContext* context = static_cast<OmContext*>(this->_context);
  OmUiPropCtxStg* uiPropCtxStg  = static_cast<OmUiPropCtxStg*>(this->childById(IDD_PROP_CTX_STG));
  OmUiPropCtxLoc* uiPropCtxLoc  = static_cast<OmUiPropCtxLoc*>(this->childById(IDD_PROP_CTX_LOC));
  OmUiPropCtxBat* uiPropCtxBat  = static_cast<OmUiPropCtxBat*>(this->childById(IDD_PROP_CTX_BAT));

  bool changed = false;

  wstring item_str;

  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_TITLE)) {  //< parameter for Context title
    uiPropCtxStg->getItemText(IDC_EC_INPT3, item_str);
    if(context->title() != item_str) {
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
  this->enableItem(IDC_BC_APPLY, changed);

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtx::applyChanges()
{
  OmContext* context = static_cast<OmContext*>(this->_context);
  OmUiPropCtxStg* uiPropCtxStg  = static_cast<OmUiPropCtxStg*>(this->childById(IDD_PROP_CTX_STG));
  OmUiPropCtxLoc* uiPropCtxLoc  = static_cast<OmUiPropCtxLoc*>(this->childById(IDD_PROP_CTX_LOC));
  OmUiPropCtxBat* uiPropCtxBat  = static_cast<OmUiPropCtxBat*>(this->childById(IDD_PROP_CTX_BAT));

  wstring ctx_name, ctx_icon;

  // Step 1, verify everything
  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_TITLE)) { //< parameter for Context title
    uiPropCtxStg->getItemText(IDC_EC_INPT3, ctx_name);
    if(Om_isValidName(ctx_name)) {
      Om_dialogBoxErr(this->_hwnd, L"Invalid Context title", OMM_STR_ERR_VALIDNAME);
      return false;
    }
  }

  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_ICON)) { // parameter for Context icon
    uiPropCtxStg->getItemText(IDC_EC_INPT4, ctx_icon);
  }

  // Step 2, save changes
  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_TITLE)) { //< parameter for Context title
    context->setTitle(ctx_name);
    // Reset parameter as unmodified
    uiPropCtxStg->setChParam(CTX_PROP_STG_TITLE, false);
  }

  if(uiPropCtxStg->hasChParam(CTX_PROP_STG_ICON)) { // parameter for Context icon
    if(Om_isValidPath(ctx_icon)) {
      context->setIcon(ctx_icon);
    } else {
      context->setIcon(L""); //< remove current icon
    }
    // Reset parameter as unmodified
    uiPropCtxStg->setChParam(CTX_PROP_STG_ICON, false);
  }

  if(uiPropCtxLoc->hasChParam(CTX_PROP_LOC_ORDER)) { // parameter for Location index order

    // To prevent inconsistency we unselect location in the main dialog
    static_cast<OmUiMain*>(this->root())->setSafeEdit(true);

    HWND hLb = uiPropCtxLoc->getItem(IDC_LB_LOCLS);

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
    static_cast<OmUiMain*>(this->root())->setSafeEdit(false);

    // Reset parameter as unmodified
    uiPropCtxLoc->setChParam(CTX_PROP_LOC_ORDER, false);
  }

  if(uiPropCtxBat->hasChParam(CTX_PROP_BAT_ORDER)) { // parameter for Location index order

    HWND hLb = uiPropCtxBat->getItem(IDC_LB_BATLS);

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
  this->enableItem(IDC_BC_APPLY, false);

  // refresh all tree from the main dialog
  this->root()->refresh();

  return true;
}
