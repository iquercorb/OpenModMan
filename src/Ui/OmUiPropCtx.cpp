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

#include "Ui/OmUiMgr.h"
#include "Ui/OmUiAddLoc.h"
#include "Ui/OmUiPropLoc.h"
#include "Ui/OmUiAddBat.h"
#include "Ui/OmUiPropBat.h"
#include "Ui/OmUiPropCtxStg.h"
#include "Ui/OmUiPropCtxLoc.h"
#include "Ui/OmUiPropCtxBat.h"
#include "Ui/OmUiProgress.h"

#include "Util/OmUtilDlg.h"
#include "Util/OmUtilStr.h"         //< Om_isValidPath
#include "Util/OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "Ui/OmUiPropCtx.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtx::OmUiPropCtx(HINSTANCE hins) : OmDialogProp(hins),
  _pCtx(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"Settings", new OmUiPropCtxStg(hins));
  this->_addPage(L"Target Locations", new OmUiPropCtxLoc(hins));
  this->_addPage(L"Installation Batches", new OmUiPropCtxBat(hins));

  this->addChild(new OmUiAddLoc(hins));     //< Dialog for Location creation
  this->addChild(new OmUiAddBat(hins));     //< Dialog for Batch creation
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
  if(!this->_pCtx)
    return false;

  OmUiPropCtxStg* pUiPropCtxStg  = static_cast<OmUiPropCtxStg*>(this->childById(IDD_PROP_CTX_STG));
  OmUiPropCtxLoc* pUiPropCtxLoc  = static_cast<OmUiPropCtxLoc*>(this->childById(IDD_PROP_CTX_LOC));
  OmUiPropCtxBat* pUiPropCtxBat  = static_cast<OmUiPropCtxBat*>(this->childById(IDD_PROP_CTX_BAT));

  bool changed = false;

  wstring item_str;

  if(pUiPropCtxStg->hasChParam(CTX_PROP_STG_TITLE)) {  //< parameter for Context title
    pUiPropCtxStg->getItemText(IDC_EC_INP03, item_str);
    if(this->_pCtx->title() != item_str) {
      changed = true;
    } else {
      pUiPropCtxStg->setChParam(CTX_PROP_STG_TITLE, false);
    }
  }

  if(pUiPropCtxStg->hasChParam(CTX_PROP_STG_ICON)) { // parameter for Context icon
    changed = true;
  }

  if(pUiPropCtxLoc->hasChParam(CTX_PROP_LOC_ORDER)) { // parameter for Location index order
    changed = true;
  }

  if(pUiPropCtxBat->hasChParam(CTX_PROP_BAT_ORDER)) { // parameter for Batches index order
    changed = true;
  }

  if(pUiPropCtxBat->hasChParam(CTX_PROP_BAT_QUIETMODE)) { // parameter for Batch Quiet Mode

    bool bm_chk = pUiPropCtxBat->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

    changed = bm_chk != this->_pCtx->batQuietMode();
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
  if(!this->_pCtx)
    return false;

  OmUiPropCtxStg* pUiPropCtxStg  = static_cast<OmUiPropCtxStg*>(this->childById(IDD_PROP_CTX_STG));
  OmUiPropCtxLoc* pUiPropCtxLoc  = static_cast<OmUiPropCtxLoc*>(this->childById(IDD_PROP_CTX_LOC));
  OmUiPropCtxBat* pUiPropCtxBat  = static_cast<OmUiPropCtxBat*>(this->childById(IDD_PROP_CTX_BAT));

  wstring ctx_name, ico_path;

  // Step 1, verify everything
  if(pUiPropCtxStg->hasChParam(CTX_PROP_STG_TITLE)) { //< parameter for Context title
    pUiPropCtxStg->getItemText(IDC_EC_INP03, ctx_name);
    if(!Om_dlgValidName(this->_hwnd, L"Software Context name", ctx_name))
      return false;
  }

  // Step 2, save changes
  if(pUiPropCtxStg->hasChParam(CTX_PROP_STG_TITLE)) { //< parameter for Context title
    this->_pCtx->setTitle(ctx_name);
    // Reset parameter as unmodified
    pUiPropCtxStg->setChParam(CTX_PROP_STG_TITLE, false);
  }

  if(pUiPropCtxStg->hasChParam(CTX_PROP_STG_ICON)) { // parameter for Context icon

    pUiPropCtxStg->getItemText(IDC_EC_INP04, ico_path);

    if(Om_isValidPath(ico_path)) {
      this->_pCtx->setIcon(ico_path);
    } else {
      this->_pCtx->setIcon(L""); //< remove current icon
    }
    // Reset parameter as unmodified
    pUiPropCtxStg->setChParam(CTX_PROP_STG_ICON, false);
  }

  if(pUiPropCtxLoc->hasChParam(CTX_PROP_LOC_ORDER)) { // parameter for Location index order

    // To prevent inconsistency we unselect location in the main dialog
    static_cast<OmUiMgr*>(this->root())->safemode(true);

    unsigned n = pUiPropCtxLoc->msgItem(IDC_LB_LOC, LB_GETCOUNT);
    for(unsigned i = 0; i < n; ++i) {
      // set new index number of Location according current List-Box order
      this->_pCtx->locGet(pUiPropCtxLoc->msgItem(IDC_LB_LOC, LB_GETITEMDATA, i))->setIndex(i);
    }

    // unselect Location in context
    this->_pCtx->locSel(-1);
    // sort Location list
    this->_pCtx->locSort();
    // select the first location in list
    this->_pCtx->locSel(0);

    // restore main dialog to normal state
    static_cast<OmUiMgr*>(this->root())->safemode(false);

    // Reset parameter as unmodified
    pUiPropCtxLoc->setChParam(CTX_PROP_LOC_ORDER, false);
  }

  if(pUiPropCtxBat->hasChParam(CTX_PROP_BAT_ORDER)) { // parameter for Batches index order

    unsigned n = pUiPropCtxBat->msgItem(IDC_LB_BAT, LB_GETCOUNT);
    for(unsigned i = 0; i < n; ++i) {
      // set new index number of Location according current List-Box order
      this->_pCtx->batGet(pUiPropCtxBat->msgItem(IDC_LB_BAT, LB_GETITEMDATA,i))->setIndex(i);
    }

    // sort Location list
    this->_pCtx->batSort();

    // Reset parameter as unmodified
    pUiPropCtxBat->setChParam(CTX_PROP_LOC_ORDER, false);
  }

  if(pUiPropCtxBat->hasChParam(CTX_PROP_BAT_QUIETMODE)) { // parameter for Batch Quiet Mode

    this->_pCtx->setBatQuietMode(pUiPropCtxBat->msgItem(IDC_BC_CKBX1, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropCtxBat->setChParam(CTX_PROP_BAT_QUIETMODE, false);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  // refresh all dialogs from root (Main dialog)
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtx::_onPropInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));
}
