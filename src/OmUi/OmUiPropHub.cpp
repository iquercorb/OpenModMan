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
#include "OmUiAddChn.h"
#include "OmUiPropChn.h"
#include "OmUiAddPst.h"
#include "OmUiPropPst.h"
#include "OmUiPropHubStg.h"
#include "OmUiPropHubChn.h"
#include "OmUiPropHubBat.h"
#include "OmUiProgress.h"

#include "OmUtilDlg.h"
#include "OmUtilStr.h"         //< Om_isValidPath
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropHub.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHub::OmUiPropHub(HINSTANCE hins) : OmDialogProp(hins),
  _modHub(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"General Settings", new OmUiPropHubStg(hins));
  this->_addPage(L"Mod Channels", new OmUiPropHubChn(hins));
  this->_addPage(L"Mod Presets", new OmUiPropHubBat(hins));

  this->addChild(new OmUiAddChn(hins));     //< Dialog for Mod Channel creation
  this->addChild(new OmUiAddPst(hins));     //< Dialog for Batch creation
  this->addChild(new OmUiPropChn(hins));    //< Dialog for Mod Channel properties
  this->addChild(new OmUiPropPst(hins));    //< Dialog for Mod Channel properties
  this->addChild(new OmUiProgress(hins));   //< for Mod Channel backup cleaning
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
  if(!this->modHub())
    return false;

  bool different, changed = false;

  OmWString inp_str;

  // General Settings Tab
  OmUiPropHubStg* pUiPropHubStg  = static_cast<OmUiPropHubStg*>(this->childById(IDD_PROP_HUB_STG));

  if(pUiPropHubStg->paramChanged(HUB_PROP_STG_TITLE)) {  //< parameter for Mod Hub title

    pUiPropHubStg->getItemText(IDC_EC_INP03, inp_str);

    if(this->modHub()->title() != inp_str) {
      changed = true;
    } else {
      pUiPropHubStg->paramReset(HUB_PROP_STG_TITLE);
    }
  }

  if(pUiPropHubStg->paramChanged(HUB_PROP_STG_ICON)) { // parameter for Mod Hub icon

    pUiPropHubStg->getItemText(IDC_EC_INP04, inp_str);

    if(!Om_namesMatches(this->modHub()->iconSource(), inp_str)) {
      changed = true;
    } else {
      pUiPropHubStg->paramReset(HUB_PROP_STG_ICON);
    }
  }

  // Mod Channels Tab
  OmUiPropHubChn* pUiPropHubChn  = static_cast<OmUiPropHubChn*>(this->childById(IDD_PROP_HUB_CHN));

  if(pUiPropHubChn->paramChanged(HUB_PROP_CHN_ORDER)) { // parameter for Mod Channel index order

    different = false;

    unsigned n = pUiPropHubChn->msgItem(IDC_LB_CHN, LB_GETCOUNT);
    for(unsigned i = 0; i < n; ++i) {
      if(pUiPropHubChn->msgItem(IDC_LB_CHN, LB_GETITEMDATA, i) != i) {
        different = true; break;
      }
    }

    if(different) {
      changed = true;
    } else {
      pUiPropHubChn->paramReset(HUB_PROP_CHN_ORDER);
    }
  }

  // Mod Presets Tab
  OmUiPropHubBat* pUiPropHubBat  = static_cast<OmUiPropHubBat*>(this->childById(IDD_PROP_HUB_BAT));

  if(pUiPropHubBat->paramChanged(HUB_PROP_BAT_ORDER)) { // parameter for Batches index order

    different = false;

    unsigned n = pUiPropHubBat->msgItem(IDC_LB_PST, LB_GETCOUNT);
    for(unsigned i = 0; i < n; ++i) {
      if(pUiPropHubBat->msgItem(IDC_LB_PST, LB_GETITEMDATA, i) != i) {
        different = true; break;
      }
    }

    if(different) {
      changed = true;
    } else {
      pUiPropHubBat->paramReset(HUB_PROP_BAT_ORDER);
    }
  }

  if(pUiPropHubBat->paramChanged(HUB_PROP_BAT_QUIETMODE)) { // parameter for Batch Quiet Mode
    if(pUiPropHubBat->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != this->modHub()->presetQuietMode()) {
      changed = true;
    } else {
      pUiPropHubBat->paramReset(HUB_PROP_BAT_QUIETMODE);
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
  if(!this->modHub())
    return true;

  OmWString inp_str;

  // General Settings Tab
  OmUiPropHubStg* pUiPropHubStg  = static_cast<OmUiPropHubStg*>(this->childById(IDD_PROP_HUB_STG));

  if(pUiPropHubStg->paramChanged(HUB_PROP_STG_TITLE)) { //< parameter for Mod Hub title
    pUiPropHubStg->getItemText(IDC_EC_INP03, inp_str);
    if(!Om_dlgValidName(this->_hwnd, L"Mod Hub name", inp_str))
      return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropHub::applyChanges()
{
  if(!this->modHub())
    return true;

  OmWString inp_str;

  // Mod Settings Tab
  OmUiPropHubStg* pUiPropHubStg  = static_cast<OmUiPropHubStg*>(this->childById(IDD_PROP_HUB_STG));

  if(pUiPropHubStg->paramChanged(HUB_PROP_STG_TITLE)) { //< parameter for Mod Hub title

    pUiPropHubStg->getItemText(IDC_EC_INP03, inp_str);

    this->modHub()->setTitle(inp_str);
    // Reset parameter as unmodified
    pUiPropHubStg->paramReset(HUB_PROP_STG_TITLE);
  }

  if(pUiPropHubStg->paramChanged(HUB_PROP_STG_ICON)) { // parameter for Mod Hub icon

    pUiPropHubStg->getItemText(IDC_EC_INP04, inp_str);

    if(Om_isValidPath(inp_str)) {
      this->modHub()->setIcon(inp_str);
    } else {
      this->modHub()->setIcon(L""); //< remove current icon
    }

    // Reset parameter as unmodified
    pUiPropHubStg->paramReset(HUB_PROP_STG_ICON);
  }

  // Mod Channels Tab
  OmUiPropHubChn* pUiPropHubChn  = static_cast<OmUiPropHubChn*>(this->childById(IDD_PROP_HUB_CHN));

  if(pUiPropHubChn->paramChanged(HUB_PROP_CHN_ORDER)) { // parameter for Mod Channel index order

    // To prevent inconsistency we unselect location in the main dialog
    static_cast<OmUiMan*>(this->root())->safemode(true);

    unsigned n = pUiPropHubChn->msgItem(IDC_LB_CHN, LB_GETCOUNT);
    for(unsigned i = 0; i < n; ++i) {
      // set new index number of Mod Channel according current List-Box order
      this->modHub()->getChannel(pUiPropHubChn->msgItem(IDC_LB_CHN, LB_GETITEMDATA, i))->setIndex(i);
    }

    // unselect Mod Channel in context
    this->modHub()->selectChannel(-1);
    // sort Mod Channel list
    this->modHub()->sortChannels();
    // select the first location in list
    this->modHub()->selectChannel(0);

    // restore main dialog to normal state
    static_cast<OmUiMan*>(this->root())->safemode(false);

    // Reset parameter as unmodified
    pUiPropHubChn->paramReset(HUB_PROP_CHN_ORDER);
  }

  // Mod Presets Tab
  OmUiPropHubBat* pUiPropHubBat  = static_cast<OmUiPropHubBat*>(this->childById(IDD_PROP_HUB_BAT));

  if(pUiPropHubBat->paramChanged(HUB_PROP_BAT_ORDER)) { // parameter for Batches index order

    unsigned n = pUiPropHubBat->msgItem(IDC_LB_PST, LB_GETCOUNT);
    for(unsigned i = 0; i < n; ++i) {
      // set new index number of Mod Channel according current List-Box order
      this->modHub()->getPreset(pUiPropHubBat->msgItem(IDC_LB_PST, LB_GETITEMDATA,i))->setIndex(i);
    }

    // sort Mod Channel list
    this->modHub()->sortPresets();

    // Reset parameter as unmodified
    pUiPropHubBat->paramReset(HUB_PROP_CHN_ORDER);
  }

  if(pUiPropHubBat->paramChanged(HUB_PROP_BAT_QUIETMODE)) { // parameter for Batch Quiet Mode

    this->modHub()->setPresetQuietMode(pUiPropHubBat->msgItem(IDC_BC_CKBX1, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropHubBat->paramReset(HUB_PROP_BAT_QUIETMODE);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  return true;
}
