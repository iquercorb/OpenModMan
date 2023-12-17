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
#include "OmModHub.h"

#include "OmBaseUi.h"
#include "OmUtilWin.h"

#include "OmUiWizChnBeg.h"
#include "OmUiWizChnTgt.h"
#include "OmUiWizChnCus.h"

#include "OmUtilDlg.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizChn.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizChn::OmUiWizChn(HINSTANCE hins) : OmDialogWiz(hins),
  _ModHub(nullptr)
{
  // create wizard pages
  this->_addPage(new OmUiWizChnBeg(hins));
  this->_addPage(new OmUiWizChnTgt(hins));
  this->_addPage(new OmUiWizChnCus(hins));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizChn::~OmUiWizChn()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizChn::id() const
{
  return IDD_WIZ_CHN;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChn::_onWizFinish()
{
  if(!this->_ModHub)
    return;

  OmUiWizChnTgt* UiWizChnTgt = static_cast<OmUiWizChnTgt*>(this->childById(IDD_WIZ_CHN_TGT));
  OmUiWizChnCus* UiWizChnCus = static_cast<OmUiWizChnCus*>(this->childById(IDD_WIZ_CHN_CUS));

  OmWString title, target_dir;

  // Retrieve Mod Hub parameters
  UiWizChnTgt->getItemText(IDC_EC_INP01, title);
  UiWizChnTgt->getItemText(IDC_EC_INP02, target_dir);

  OmWString modlib_dir, backup_dir;

  // Retrieve Mod Channel parameter
  if(UiWizChnCus->msgItem(IDC_BC_CKBX1, BM_GETCHECK))
    UiWizChnCus->getItemText(IDC_EC_INP01, modlib_dir);

  if(UiWizChnCus->msgItem(IDC_BC_CKBX2, BM_GETCHECK))
    UiWizChnCus->getItemText(IDC_EC_INP02, backup_dir);

  // quit dialog !! >>> NOW <<<  !!
  this->quit();

  // get current selected Mod Hub (the just created one)
  OmModHub* ModHub = this->_ModHub;

  // create new Mod Channel in Mod Hub
  if(!ModHub->createChannel(title, target_dir, modlib_dir, backup_dir)) {
    Om_dlgBox_okl(this->_hwnd, L"Mod Channel Wizard", IDI_DLG_ERR, L"Mod Channel creation error",
                  L"The Channel creation failed:", ModHub->lastError());
  }

  // force parent dialog to refresh
  this->root()->refresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChn::_onWizInit()
{
  // set splash image
  this->setStImage(IDC_SB_IMAGE, Om_getResImage(IDB_SC_WIZ_CHN));
}
