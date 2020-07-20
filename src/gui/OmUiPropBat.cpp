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
#include "OmUiPropBat.h"
#include "OmUiPropBatStg.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropBat::OmUiPropBat(HINSTANCE hins) : OmDialogProp(hins),
  _batch(nullptr)
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
  OmBatch* batch = static_cast<OmBatch*>(this->_batch);
  OmUiPropBatStg* pUiPropBatStg  = static_cast<OmUiPropBatStg*>(this->childById(IDD_PROP_BAT_STG));

  bool changed = false;

  wstring item_str;

  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) {  //< parameter for Context title
    pUiPropBatStg->getItemText(IDC_EC_INPT1, item_str);
    if(batch->title() != item_str) {
      changed = true;
    } else {
      pUiPropBatStg->setChParam(BAT_PROP_STG_TITLE, false);
    }
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
  OmBatch* batch = static_cast<OmBatch*>(this->_batch);
  OmUiPropBatStg* pUiPropBatStg  = static_cast<OmUiPropBatStg*>(this->childById(IDD_PROP_BAT_STG));

  wstring bat_name;

  // Step 1, verify everything
  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) { //< parameter for Context title
    pUiPropBatStg->getItemText(IDC_EC_INPT1, bat_name);
    if(Om_isValidName(bat_name)) {
      Om_dialogBoxErr(this->_hwnd, L"Invalid Batch title", OMM_STR_ERR_VALIDNAME);
      return false;
    }
    // Check whether name already exists
    OmContext* pCtx = batch->context();
    for(unsigned i = 0; i < pCtx->batchCount(); ++i) {
      if(pCtx->batch(i)->title() == bat_name) {
        Om_dialogBoxErr(this->_hwnd, L"Not unique Batch title",
                                     L"A Batch with the same title already "
                                     L"exists. Please choose another title.");
        return false;
      }
    }
  }

  // Step 2, save changes
  if(pUiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) { //< parameter for Context title
    if(!batch->renameHome(bat_name)) { //< rename Batch filename
      Om_dialogBoxErr(this->_hwnd,  L"Batch rename failed",
                                    batch->lastError());
    }
    batch->setTitle(bat_name); //< change Batch title
    // Reset parameter as unmodified
    pUiPropBatStg->setChParam(BAT_PROP_STG_TITLE, false);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  // refresh all tree from the main dialog
  this->root()->refresh();

  return true;
}
