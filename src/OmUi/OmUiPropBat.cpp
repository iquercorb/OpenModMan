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

#include "OmUiPropBatStg.h"
#include "OmUiPropBatLst.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"   //< Om_getResIcon, etc.

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropBat.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropBat::OmUiPropBat(HINSTANCE hins) : OmDialogProp(hins),
  _pBat(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"Settings", new OmUiPropBatStg(hins));
  this->_addPage(L"Installation Lists", new OmUiPropBatLst(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropBat::~OmUiPropBat()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropBat::id() const
{
  return IDD_PROP_BAT;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropBat::checkChanges()
{
  if(!this->_pBat)
    return false;

  OmUiPropBatStg* pUiPropBatStg  = static_cast<OmUiPropBatStg*>(this->childById(IDD_PROP_BAT_STG));
  OmUiPropBatLst* pUiPropBatLst  = static_cast<OmUiPropBatLst*>(this->childById(IDD_PROP_BAT_LST));

  bool changed = false;

  wstring item_str;

  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) {  //< parameter for Batch title
    pUiPropBatStg->getItemText(IDC_EC_INP01, item_str);
    if(this->_pBat->title() != item_str) {
      changed = true;
    } else {
      pUiPropBatStg->setChParam(BAT_PROP_STG_TITLE, false);
    }
  }

  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_IONLY)) {  //< parameter for Batch title
    int bm_chk = pUiPropBatStg->msgItem(IDC_BC_CKBX1, BM_GETCHECK);
    if(bm_chk != this->_pBat->installOnly()) {
      changed = true;
    } else {
      pUiPropBatStg->setChParam(BAT_PROP_STG_IONLY, false);
    }
  }


  if(pUiPropBatLst->hasChParam(BAT_PROP_STG_INSLS)) {  //< parameter for Batch install list
    changed = true;
  }

  // enable Apply button
  this->enableItem(IDC_BC_APPLY, changed);

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropBat::applyChanges()
{
  if(!this->_pBat)
    return false;

  OmContext* pCtx = this->_pBat->pCtx();

  OmUiPropBatStg* pUiPropBatStg  = static_cast<OmUiPropBatStg*>(this->childById(IDD_PROP_BAT_STG));
  OmUiPropBatLst* pUiPropBatLst  = static_cast<OmUiPropBatLst*>(this->childById(IDD_PROP_BAT_LST));

  wstring bat_name;

  // Step 1, verify everything
  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) { //< parameter for Batch title
    pUiPropBatStg->getItemText(IDC_EC_INP01, bat_name);
    if(!Om_dlgValidName(this->_hwnd, L"Script name", bat_name))
    return false;

    // Check whether name already exists
    for(unsigned i = 0; i < pCtx->batCount(); ++i) {
      if(pCtx->batGet(i)->title() == bat_name) {
        Om_dlgBox_ok(this->_hwnd, L"Script Properties", IDI_ERR,
                     L"Script name exists", L"Script "
                     "with the same name already exists, please choose another one.");
        return false;
      }
    }
  }


  // Step 2, save changes
  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) { //< parameter for Context title
    if(!this->_pBat->rename(bat_name)) { //< rename Batch filename
      Om_dlgBox_okl(this->_hwnd, L"Script Properties", IDI_WRN,
                   L"Script rename error", L"Script "
                   "file rename failed because of the following error:",
                   this->_pBat->lastError());
    }
    this->_pBat->setTitle(bat_name); //< change Batch title
    // Reset parameter as unmodified
    pUiPropBatStg->setChParam(BAT_PROP_STG_TITLE, false);
  }

  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_IONLY)) {  //< parameter for Batch title
    int bm_chk = pUiPropBatStg->msgItem(IDC_BC_CKBX1, BM_GETCHECK);
    this->_pBat->setInstallOnly(bm_chk); //< change option value
    // Reset parameter as unmodified
    pUiPropBatStg->setChParam(BAT_PROP_STG_IONLY, false);
  }

  if(pUiPropBatLst->hasChParam(BAT_PROP_STG_INSLS)) {  //< parameter for Batch install list

    // build the per-Location hash lists
    OmLocation* pLoc;
    OmPackage* pPkg;

    for(size_t k = 0; k < pCtx->locCount(); ++k) {

      pLoc = pCtx->locGet(k);

      // clear previous install list
      this->_pBat->instClear(pLoc);

      for(size_t i = 0; i < pUiPropBatLst->incCount(k); ++i) {

        // get package from stored index
        pPkg = pLoc->pkgGet(pUiPropBatLst->incGet(k, i));

        // add Package to install list
        this->_pBat->instAdd(pLoc, pPkg);
      }
    }

    // Reset parameter as unmodified
    pUiPropBatLst->setChParam(BAT_PROP_STG_INSLS, false);
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
void OmUiPropBat::_onPropInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));
}
