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
#include "gui/OmUiPropBat.h"
#include "gui/OmUiPropBatStg.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropBat::OmUiPropBat(HINSTANCE hins) : OmDialogProp(hins),
  _pBat(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"Settings", new OmUiPropBatStg(hins));
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

  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_INSLS)) {  //< parameter for Batch install list
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

  OmContext* pCtx = this->_pBat->ownerCtx();

  OmUiPropBatStg* pUiPropBatStg  = static_cast<OmUiPropBatStg*>(this->childById(IDD_PROP_BAT_STG));

  wstring bat_name;

  // Step 1, verify everything
  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) { //< parameter for Batch title
    pUiPropBatStg->getItemText(IDC_EC_INP01, bat_name);
    if(!Om_isValidName(bat_name)) {
      wstring wrn = L"The title"; wrn += OMM_STR_ERR_VALIDNAME;
      Om_dialogBoxWarn(this->_hwnd, L"Invalid Batch title", wrn);
      return false;
    }
    // Check whether name already exists
    for(unsigned i = 0; i < pCtx->batCount(); ++i) {
      if(pCtx->batGet(i)->title() == bat_name) {

        wstring err = L"A Batch with the same title already "
                      L"exists. Please choose another title.";

        Om_dialogBoxErr(this->_hwnd, L"Batch title already exists",err);

        return false;
      }
    }
  }


  // Step 2, save changes
  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) { //< parameter for Context title
    if(!this->_pBat->rename(bat_name)) { //< rename Batch filename
      Om_dialogBoxErr(this->_hwnd, L"Batch rename failed", this->_pBat->lastError());
    }
    this->_pBat->setTitle(bat_name); //< change Batch title
    // Reset parameter as unmodified
    pUiPropBatStg->setChParam(BAT_PROP_STG_TITLE, false);
  }

  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_INSLS)) {  //< parameter for Batch install list

    // build the per-Location hash lists
    vector<uint64_t> hash_ls;
    OmLocation* pLoc;
    OmPackage* pPkg;

    for(size_t k = 0; k < pCtx->locCount(); ++k) {

      pLoc = pCtx->locGet(k);

      // reset list
      hash_ls.clear();

      for(size_t i = 0; i < pUiPropBatStg->incCount(k); ++i) {

        // retrieve package from stored index
        pPkg = pLoc->pkgGet(pUiPropBatStg->incGet(k, i));

        // add <install> entry with package hash
        hash_ls.push_back(pPkg->hash());
      }

      // add new location in batch if required
      if(!this->_pBat->hasLoc(pLoc->uuid()))
         this->_pBat->locAdd(pLoc->uuid());

      // set new Install list
      this->_pBat->insSetList(pLoc->uuid(), hash_ls);
    }

    // Reset parameter as unmodified
    pUiPropBatStg->setChParam(BAT_PROP_STG_INSLS, false);
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
  this->setIcon(Om_getResIcon(this->_hins, IDB_APP_ICON, 2), Om_getResIcon(this->_hins, IDB_APP_ICON, 1));
}
