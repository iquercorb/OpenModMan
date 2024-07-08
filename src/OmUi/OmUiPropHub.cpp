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

#include "OmUiMan.h"
#include "OmUiPropHubStg.h"
#include "OmUiPropHubChn.h"
#include "OmUiPropHubPst.h"

#include "OmUtilDlg.h"
#include "OmUtilStr.h"         //< Om_hasLegalPathChar
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropHub.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHub::OmUiPropHub(HINSTANCE hins) : OmDialogProp(hins),
  _ModHub(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"General Settings", new OmUiPropHubStg(hins));
  this->_addPage(L"Channels", new OmUiPropHubChn(hins));
  this->_addPage(L"Presets", new OmUiPropHubPst(hins));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHub::~OmUiPropHub()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropHub::id() const
{
  return IDD_PROP_HUB;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropHub::checkChanges()
{
  if(!this->_ModHub)
    return false;

  bool different, changed = false;

  // General Settings Tab
  OmUiPropHubStg* UiPropHubStg  = static_cast<OmUiPropHubStg*>(this->childById(IDD_PROP_HUB_STG));

  if(UiPropHubStg->paramChanged(HUB_PROP_STG_TITLE)) {  //< parameter for Mod Hub title

    OmWString ec_entry;

    UiPropHubStg->getItemText(IDC_EC_INP03, ec_entry);

    if(this->_ModHub->title() != ec_entry) {
      changed = true;
    } else {
      UiPropHubStg->paramReset(HUB_PROP_STG_TITLE);
    }
  }

  if(UiPropHubStg->paramChanged(HUB_PROP_STG_LAYOUT)) {  //< parameter for Layout options

    different = false;

    if(UiPropHubStg->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != this->_ModHub->layoutChannelsShow())
      different = true;

    if(UiPropHubStg->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != this->_ModHub->layoutPresetsShow())
      different = true;

    if(different) {
      changed = true;
    } else {
      UiPropHubStg->paramReset(HUB_PROP_STG_LAYOUT);
    }
  }

/*
  if(UiPropHubStg->paramChanged(HUB_PROP_STG_ICON)) { // parameter for Mod Hub icon

    OmWString ec_entry;

    UiPropHubStg->getItemText(IDC_EC_INP04, ec_entry);

    if(!Om_namesMatches(this->_ModHub->iconSource(), ec_entry)) {
      changed = true;
    } else {
      UiPropHubStg->paramReset(HUB_PROP_STG_ICON);
    }
  }
*/
  // Mod Channels Tab
  OmUiPropHubChn* UiPropHubChn  = static_cast<OmUiPropHubChn*>(this->childById(IDD_PROP_HUB_CHN));

  if(UiPropHubChn->paramChanged(HUB_PROP_CHN_ORDER)) { // parameter for Mod Channel index order

    different = false;

    int32_t n = UiPropHubChn->msgItem(IDC_LB_CHN, LB_GETCOUNT);
    for(int32_t i = 0; i < n; ++i) {
      if(UiPropHubChn->msgItem(IDC_LB_CHN, LB_GETITEMDATA, i) != i) {
        different = true; break;
      }
    }

    if(different) {
      changed = true;
    } else {
      UiPropHubChn->paramReset(HUB_PROP_CHN_ORDER);
    }
  }

  // Mod Presets Tab
  OmUiPropHubPst* UiPropHubPst  = static_cast<OmUiPropHubPst*>(this->childById(IDD_PROP_HUB_PST));

  if(UiPropHubPst->paramChanged(HUB_PROP_BAT_ORDER)) { // parameter for Batches index order

    different = false;

    int32_t n = UiPropHubPst->msgItem(IDC_LB_PST, LB_GETCOUNT);
    for(int32_t i = 0; i < n; ++i) {
      if(UiPropHubPst->msgItem(IDC_LB_PST, LB_GETITEMDATA, i) != i) {
        different = true; break;
      }
    }

    if(different) {
      changed = true;
    } else {
      UiPropHubPst->paramReset(HUB_PROP_BAT_ORDER);
    }
  }

  if(UiPropHubPst->paramChanged(HUB_PROP_BAT_QUIETMODE)) { // parameter for Batch Quiet Mode
    if(UiPropHubPst->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != this->_ModHub->presetQuietMode()) {
      changed = true;
    } else {
      UiPropHubPst->paramReset(HUB_PROP_BAT_QUIETMODE);
    }
  }

  // enable Apply button
  this->enableItem(IDC_BC_APPLY, changed);

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropHub::validChanges()
{
  if(!this->_ModHub)
    return true;

  // General Settings Tab
  OmUiPropHubStg* UiPropHubStg  = static_cast<OmUiPropHubStg*>(this->childById(IDD_PROP_HUB_STG));

  if(UiPropHubStg->paramChanged(HUB_PROP_STG_TITLE)) { //< parameter for Mod Hub title

    OmWString ec_entry;

    UiPropHubStg->getItemText(IDC_EC_INP03, ec_entry);

    if(!Om_dlgValidName(this->_hwnd, L"Mod Hub name", ec_entry))
      return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropHub::applyChanges()
{
  if(!this->_ModHub)
    return true;

  // Mod Settings Tab
  OmUiPropHubStg* UiPropHubStg  = static_cast<OmUiPropHubStg*>(this->childById(IDD_PROP_HUB_STG));

  if(UiPropHubStg->paramChanged(HUB_PROP_STG_TITLE)) { //< parameter for Mod Hub title

    OmWString ec_entry;

    UiPropHubStg->getItemText(IDC_EC_INP03, ec_entry);

    this->_ModHub->setTitle(ec_entry);
    // Reset parameter as unmodified
    UiPropHubStg->paramReset(HUB_PROP_STG_TITLE);
  }
/*
  if(UiPropHubStg->paramChanged(HUB_PROP_STG_ICON)) { // parameter for Mod Hub icon

    OmWString ec_entry;

    UiPropHubStg->getItemText(IDC_EC_INP04, ec_entry);

    if(Om_hasLegalPathChar(ec_entry)) {
      this->_ModHub->setIcon(ec_entry);
    } else {
      this->_ModHub->setIcon(L""); //< remove current icon
    }

    // Reset parameter as unmodified
    UiPropHubStg->paramReset(HUB_PROP_STG_ICON);
  }
*/
  if(UiPropHubStg->paramChanged(HUB_PROP_STG_LAYOUT)) {  //< parameter for Layout options

    this->_ModHub->setLayoutChannelsShow(UiPropHubStg->msgItem(IDC_BC_CKBX1, BM_GETCHECK));
    this->_ModHub->setLayoutPresetsShow(UiPropHubStg->msgItem(IDC_BC_CKBX2, BM_GETCHECK));

    // Reset parameter as unmodified
    UiPropHubStg->paramReset(HUB_PROP_STG_LAYOUT);
  }

  // Mod Channels Tab
  OmUiPropHubChn* UiPropHubChn  = static_cast<OmUiPropHubChn*>(this->childById(IDD_PROP_HUB_CHN));

  if(UiPropHubChn->paramChanged(HUB_PROP_CHN_ORDER)) { // parameter for Mod Channel index order

    // To prevent inconsistency we unselect location in the main dialog
    static_cast<OmUiMan*>(this->root())->enableLockMode(true);

    uint32_t n = UiPropHubChn->msgItem(IDC_LB_CHN, LB_GETCOUNT);
    for(uint32_t i = 0; i < n; ++i) {
      // set new index number of Mod Channel according current List-Box order
      this->_ModHub->getChannel(UiPropHubChn->msgItem(IDC_LB_CHN, LB_GETITEMDATA, i))->setIndex(i);
    }

    // unselect Mod Channel in context
    this->_ModHub->selectChannel(-1);
    // sort Mod Channel list
    this->_ModHub->sortChannels();
    // select the first location in list
    this->_ModHub->selectChannel(0);

    // restore main dialog to normal state
    static_cast<OmUiMan*>(this->root())->enableLockMode(false);

    // Reset parameter as unmodified
    UiPropHubChn->paramReset(HUB_PROP_CHN_ORDER);
  }

  // Mod Presets Tab
  OmUiPropHubPst* UiPropHubPst  = static_cast<OmUiPropHubPst*>(this->childById(IDD_PROP_HUB_PST));

  if(UiPropHubPst->paramChanged(HUB_PROP_BAT_ORDER)) { // parameter for Batches index order

    uint32_t n = UiPropHubPst->msgItem(IDC_LB_PST, LB_GETCOUNT);
    for(uint32_t i = 0; i < n; ++i) {
      // set new index number of Mod Channel according current List-Box order
      this->_ModHub->getPreset(UiPropHubPst->msgItem(IDC_LB_PST, LB_GETITEMDATA,i))->setIndex(i);
    }

    // sort Mod Channel list
    this->_ModHub->sortPresets();

    // Reset parameter as unmodified
    UiPropHubPst->paramReset(HUB_PROP_CHN_ORDER);
  }

  if(UiPropHubPst->paramChanged(HUB_PROP_BAT_QUIETMODE)) { // parameter for Batch Quiet Mode

    this->_ModHub->setPresetQuietMode(UiPropHubPst->msgItem(IDC_BC_CKBX1, BM_GETCHECK));

    // Reset parameter as unmodified
    UiPropHubPst->paramReset(HUB_PROP_BAT_QUIETMODE);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  return true;
}
