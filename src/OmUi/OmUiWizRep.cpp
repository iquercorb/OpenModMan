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
#include "OmModChan.h"

#include "OmBaseUi.h"

#include "OmUiWizRepBeg.h"
#include "OmUiWizRepCfg.h"
#include "OmUiWizRepQry.h"

#include "OmUtilDlg.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizRep.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizRep::OmUiWizRep(HINSTANCE hins) : OmDialogWiz(hins),
  _ModChan(nullptr)
{
  // create wizard pages
  this->_addPage(new OmUiWizRepBeg(hins));
  this->_addPage(new OmUiWizRepCfg(hins));
  this->_addPage(new OmUiWizRepQry(hins));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizRep::~OmUiWizRep()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizRep::id() const
{
  return IDD_WIZ_REP;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizRep::_onWizFinish()
{
  if(!this->_ModChan)
    return;

  OmUiWizRepCfg* UiWizRepCfg = static_cast<OmUiWizRepCfg*>(this->childById(IDD_WIZ_REP_CFG));

  OmWString base, name, full;

  if(UiWizRepCfg->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    UiWizRepCfg->getItemText(IDC_EC_INP01, base);
    UiWizRepCfg->getItemText(IDC_EC_INP02, name);
  } else {
    UiWizRepCfg->getItemText(IDC_EC_INP03, base);
  }

  // quit dialog !! >>> NOW <<<  !!
  this->quit();

  if(!this->_ModChan->addRepository(base, name)) {
    Om_dlgBox_okl(this->_hwnd, L"Repository Configuration Wizard", IDI_ERR, L"Repository config error",
                    L"Unable to add Repository to Channel:", this->_ModChan->lastError());
  }

  // force parent dialog to refresh
  this->root()->refresh();
}
