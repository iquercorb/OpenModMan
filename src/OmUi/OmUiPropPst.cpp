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

#include "OmModMan.h"

#include "OmUiPropPstStg.h"
#include "OmUiPropPstLst.h"

#include "OmUtilDlg.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropPst.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPst::OmUiPropPst(HINSTANCE hins) : OmDialogProp(hins), _ModPset(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"General Settings", new OmUiPropPstStg(hins));
  this->_addPage(L"Installation Setups", new OmUiPropPstLst(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPst::~OmUiPropPst()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropPst::id() const
{
  return IDD_PROP_PST;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropPst::checkChanges()
{
  if(!this->_ModPset)
    return false;

  bool different, changed = false;

  OmWString item_str;

  // General Settings Tab
  OmUiPropPstStg* pUiPropBatStg  = static_cast<OmUiPropPstStg*>(this->childById(IDD_PROP_PST_STG));

  if(pUiPropBatStg->paramChanged(PST_PROP_STG_TITLE)) {

    pUiPropBatStg->getItemText(IDC_EC_INP01, item_str);

    if(this->_ModPset->title() != item_str) {
      changed = true;
    } else {
      pUiPropBatStg->paramReset(PST_PROP_STG_TITLE);
    }
  }

  if(pUiPropBatStg->paramChanged(PST_PROP_STG_IONLY)) {

    if(pUiPropBatStg->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != this->_ModPset->installOnly()) {
      changed = true;
    } else {
      pUiPropBatStg->paramReset(PST_PROP_STG_IONLY);
    }
  }

  // Mods Install List Tab
  OmUiPropPstLst* pUiPropBatLst  = static_cast<OmUiPropPstLst*>(this->childById(IDD_PROP_PST_LST));

  if(pUiPropBatLst->paramChanged(PST_PROP_STG_INSLS)) {

    different = false;

    OmModHub* ModHub = this->_ModPset->ModHub();

    for(size_t k = 0; k < ModHub->channelCount(); ++k) {

      OmModChan* ModChan = ModHub->getChannel(k);

      if(pUiPropBatLst->instCount(k) != this->_ModPset->setupEntryCount(ModChan)) {
        different = true;
        break;
      }

      for(size_t i = 0; i < pUiPropBatLst->instCount(k); ++i) {

        OmModPack* ModPack = ModChan->getModpack(pUiPropBatLst->getSetupEntry(k, i));

        if(!this->_ModPset->setupHasEntry(ModChan, ModPack)) {
          different = true;
          break;
        }
      }
    }

    if(different) {
      changed = true;
    } else {
      pUiPropBatLst->paramReset(PST_PROP_STG_INSLS);
    }
  }

  // enable Apply button
  this->enableItem(IDC_BC_APPLY, changed);

  return changed;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropPst::validChanges()
{
  if(!this->_ModPset)
    return true;

  // General Settings Tab
  OmUiPropPstStg* pUiPropBatStg  = static_cast<OmUiPropPstStg*>(this->childById(IDD_PROP_PST_STG));

  if(pUiPropBatStg->paramChanged(PST_PROP_STG_TITLE)) {

    OmWString input;
    pUiPropBatStg->getItemText(IDC_EC_INP01, input);

    if(!Om_dlgValidName(this->_hwnd, L"Mod Preset name", input))
      return false;

    OmModHub* ModHub = this->_ModPset->ModHub();

    // Check whether name already exists
    for(unsigned i = 0; i < ModHub->presetCount(); ++i) {

      if(ModHub->getPreset(i)->title() == input) {

        Om_dlgBox_ok(this->_hwnd, L"Preset Properties", IDI_DLG_ERR,
                     L"Mod Preset name exists", L"Mod Preset "
                     "with the same name already exists.");

        return false;
      }
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropPst::applyChanges()
{
  if(!this->_ModPset)
    return true;

  // General Settings Tab
  OmUiPropPstStg* pUiPropBatStg  = static_cast<OmUiPropPstStg*>(this->childById(IDD_PROP_PST_STG));

  if(pUiPropBatStg->paramChanged(PST_PROP_STG_TITLE)) {

    OmWString input;
    pUiPropBatStg->getItemText(IDC_EC_INP01, input);

    OmModHub* ModHub = this->_ModPset->ModHub();

    // get Mod Preset index in Mod Hub
    int32_t p = ModHub->indexOfPreset(this->_ModPset);

    OmResult result = ModHub->renamePreset(p, input);

    if(result != OM_RESULT_OK) { //< rename Preset filename

      if(result == OM_RESULT_ERROR) {
        Om_dlgBox_okl(this->_hwnd, L"Mod Preset Properties", IDI_DLG_WRN, L"Preset rename error",
                      L"Preset file rename failed:", ModHub->lastError());
      } else {
        Om_dlgBox_okl(this->_hwnd, L"Mod Preset Properties", IDI_DLG_WRN, L"Preset rename error",
                      L"Unable to rename Preset:", ModHub->lastError());
      }

      return false;
    }

    pUiPropBatStg->paramReset(PST_PROP_STG_TITLE);
  }

  if(pUiPropBatStg->paramChanged(PST_PROP_STG_IONLY)) {

    this->_ModPset->setInstallOnly(pUiPropBatStg->msgItem(IDC_BC_CKBX1, BM_GETCHECK));

    pUiPropBatStg->paramReset(PST_PROP_STG_IONLY);
  }

  // Mods Install List Tab
  OmUiPropPstLst* pUiPropBatLst  = static_cast<OmUiPropPstLst*>(this->childById(IDD_PROP_PST_LST));

  if(pUiPropBatLst->paramChanged(PST_PROP_STG_INSLS)) {

    OmModHub* ModHub = this->_ModPset->ModHub();

    // build the per-Mod Channel hash lists
    for(size_t k = 0; k < ModHub->channelCount(); ++k) {

      OmModChan* ModChan = ModHub->getChannel(k);

      // clear previous install list
      this->_ModPset->discardSetup(ModChan);

      for(size_t i = 0; i < pUiPropBatLst->instCount(k); ++i) {

        // get package from stored index
        OmModPack* ModPack = ModChan->getModpack(pUiPropBatLst->getSetupEntry(k, i));

        // add Package to install list
        this->_ModPset->addSetupEntry(ModChan, ModPack);
      }
    }

    pUiPropBatLst->paramReset(PST_PROP_STG_INSLS);
  }

  // save changes to XML definition
  this->_ModPset->save();

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  return true;
}
