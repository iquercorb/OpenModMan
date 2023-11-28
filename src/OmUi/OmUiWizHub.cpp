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
#include "OmModHub.h"

#include "OmUiWizHubBeg.h"
#include "OmUiWizHubCfg.h"
#include "OmUiWizHubChn.h"
#include "OmUiMan.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizHub.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizHub::OmUiWizHub(HINSTANCE hins) : OmDialogWiz(hins)
{
  // create wizard pages
  this->_addPage(new OmUiWizHubBeg(hins));
  this->_addPage(new OmUiWizHubCfg(hins));
  this->_addPage(new OmUiWizHubChn(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizHub::~OmUiWizHub()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizHub::id() const
{
  return IDD_WIZ_HUB;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizHub::_onWizNext()
{
  switch(this->_currPage)
  {
  case 1: // Mod Hub parameters Wizard page
    return static_cast<OmUiWizHubCfg*>(this->childById(IDD_WIZ_HUB_CFG))->hasValidParams();
    break;
  case 2: // Mod Channel parameters Wizard page
    return static_cast<OmUiWizHubChn*>(this->childById(IDD_WIZ_HUB_CHN))->hasValidParams();
    break;
  default:
    return true;
    break;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHub::_onWizFinish()
{
  OmUiWizHubCfg* pUiWizCtxCfg = static_cast<OmUiWizHubCfg*>(this->childById(IDD_WIZ_HUB_CFG));
  OmUiWizHubChn* pUiWizLocCfg = static_cast<OmUiWizHubChn*>(this->childById(IDD_WIZ_HUB_CHN));

  // Retrieve Mod Hub parameters
  OmWString hub_name, hub_path;
  pUiWizCtxCfg->getItemText(IDC_EC_INP01, hub_name);
  pUiWizCtxCfg->getItemText(IDC_EC_INP02, hub_path);

  // Retrieve Mod Channel parameters
  OmWString chn_name, chn_dst, chn_lib, chn_bck;
  pUiWizLocCfg->getItemText(IDC_EC_INP01, chn_name);
  pUiWizLocCfg->getItemText(IDC_EC_INP02, chn_dst);

  if(pUiWizLocCfg->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
    pUiWizLocCfg->getItemText(IDC_EC_INP03, chn_lib);
  }

  if(pUiWizLocCfg->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
    pUiWizLocCfg->getItemText(IDC_EC_INP04, chn_bck);
  }

  this->quit();

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // create the new Mod Hub, if an error occur, error message
  if(ModMan->createHub(hub_path, hub_name, true)) {

    // get current selected Mod Hub (the just created one)
    OmModHub* ModHub = ModMan->activeHub();

    // create new Mod Channel in Mod Hub
    if(!ModHub->createChannel(chn_name, chn_dst, chn_lib, chn_bck)) {

      Om_dlgBox_okl(this->_hwnd, L"Mod Hub Wizard", IDI_ERR,
                    L"Mod Channel creation error", L"Mod Channel "
                    "creation failed because of the following error:",
                    ModHub->lastError());
    }

  } else {

    Om_dlgBox_okl(this->_hwnd, L"Mod Hub Wizard", IDI_ERR,
                  L"Mod Hub creation error", L"Mod Hub "
                  "creation failed because of the following error:",
                  ModMan->lastError());

  }

  // force parent dialog to refresh
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHub::_onWizInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));
}
