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
void OmUiWizHub::_onWizFinish()
{
  OmUiWizHubCfg* UiWizHubCfg = static_cast<OmUiWizHubCfg*>(this->childById(IDD_WIZ_HUB_CFG));

  OmWString hub_name, hub_path;

  // Retrieve Mod Hub parameters
  UiWizHubCfg->getItemText(IDC_EC_INP01, hub_name);
  UiWizHubCfg->getItemText(IDC_EC_INP02, hub_path);

  // quit dialog !! >>> NOW <<<  !!
  this->quit();

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  int32_t previous = ModMan->activeHubIndex();

  // Unselect any current Hub
  static_cast<OmUiMan*>(this->root())->selectHub(-1);

  // create the new Mod Hub, if an error occur, error message
  if(OM_RESULT_OK != ModMan->createHub(hub_path, hub_name, true)) {
    Om_dlgBox_okl(this->_hwnd, L"Mod Hub Wizard", IDI_DLG_ERR, L"Hub creation failed",
                  L"Unable to create the Mod Hub:", ModMan->lastError());

    // Select previous Hub
    if(ModMan->hubCount())
      static_cast<OmUiMan*>(this->root())->selectHub(previous);
  }

  // force parent dialog to refresh
  this->root()->refresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHub::_onWizInit()
{
  // set splash image
  this->setStImage(IDC_SB_IMAGE, Om_getResImage(IDB_SC_WIZ_HUB));
}
