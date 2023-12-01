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

#include "OmBaseWin.h"
  #include <UxTheme.h>

#include "OmModMan.h"

#include "OmUiMan.h"
#include "OmUiAddChn.h"
#include "OmUiPropHub.h"
#include "OmUiPropChn.h"
#include "OmUiProgress.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropHubChn.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubChn::OmUiPropHubChn(HINSTANCE hins) : OmDialogPropTab(hins),
  _chn_delete_hth(nullptr),
  _chn_delete_hwo(nullptr),
  _chn_delete_chn(-1)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubChn::~OmUiPropHubChn()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropHubChn::id() const
{
  return IDD_PROP_HUB_CHN;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropHubChn::_chn_delete_run_fn(void* ptr)
{
  OmUiPropHubChn* self = static_cast<OmUiPropHubChn*>(ptr);

  OmModHub* ModHub = static_cast<OmUiPropHub*>(self->_parent)->modHub();
  if(!ModHub) return static_cast<DWORD>(OM_RESULT_ABORT);

  OmModChan* ModChan = ModHub->getChannel(self->_chn_delete_chn);
  if(!ModChan) return static_cast<DWORD>(OM_RESULT_ABORT);

  // set UI in safe mode
  static_cast<OmUiMgr*>(this->root())->enableSafeMode(false);

  // unselect Channel
  ModHub->selectChannel(-1);

  // Open progress dialog
  OmUiProgress* UiProgress = static_cast<OmUiProgress*>(self->siblingById(IDD_PROGRESS));
  UiProgress->open(true);
  UiProgress->setCaption(L"Delete Mod Channel");
  UiProgress->setScHeadText(L"Restoring backup data");

  // delete channel, will purge backup inf necessar
  OmResult result = ModHub->deleteChannel(self->_chn_delete_chn, OmUiPropHubChn::_chn_delete_progress_cb, self);

  return static_cast<DWORD>(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropHubChn::_chn_delete_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmUiPropHubChn* self = static_cast<OmUiPropHubChn*>(ptr);

  OmModHub* ModHub = static_cast<OmUiPropHub*>(self->_parent)->modHub();
  if(!ModHub) return false;

  OmModPack* ModPack = reinterpret_cast<OmModPack*>(param);

  OmUiProgress* UiProgress = static_cast<OmUiProgress*>(self->siblingById(IDD_PROGRESS));
  UiProgress->setScItemText(ModPack->iden());
  UiProgress->setPbRange(0, tot);
  UiProgress->setPbPos(cur);

  return !UiProgress->abortGet();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmUiPropHubChn::_chn_delete_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  OmUiPropHubChn* self = static_cast<OmUiPropHubChn*>(ptr);

  OmModHub* ModHub = static_cast<OmUiPropHub*>(self->_parent)->modHub();
  if(!ModHub) return;

  // quit the progress dialog
  static_cast<OmUiProgress*>(self->siblingById(IDD_PROGRESS))->quit();

  DWORD exit_code = Om_threadExitCode(self->_chn_delete_hth);
  Om_clearThread(self->_chn_delete_hth, self->_chn_delete_hwo);

  self->_chn_delete_hth = nullptr;
  self->_chn_delete_hwo = nullptr;


  OmResult result = static_cast<OmResult>(exit_code);

  if(result == OM_RESULT_ERROR) {
      // an error occurred during backup purge
      Om_dlgBox_okl(self->_hwnd, L"Hub properties", IDI_WRN, L"Delete channel error",
                    L"Channel deletion encountered error:", ModHub->lastError() );
  }

  // Select fist available Channel
  ModHub->selectChannel(0);

  // restore UI interactions
  static_cast<OmUiMgr*>(this->root())->enableSafeMode(false);

  // refresh all dialogs from root
  this->root()->refresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_lb_chn_on_selchg()
{
  OmModHub* pModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!pModHub) return;

  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  int chn_id = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);

  if(chn_id >= 0) {

    OmModChan* pModChan = pModHub->getChannel(chn_id);

    this->setItemText(IDC_EC_READ2, pModChan->targetPath());
    this->setItemText(IDC_EC_READ3, pModChan->libraryPath());
    this->setItemText(IDC_EC_READ4, pModChan->backupPath());

    this->enableItem(IDC_SC_LBL02, true);
    this->enableItem(IDC_EC_READ2, true);
    this->enableItem(IDC_SC_LBL03, true);
    this->enableItem(IDC_EC_READ3, true);
    this->enableItem(IDC_SC_LBL04, true);
    this->enableItem(IDC_EC_READ4, true);

    this->enableItem(IDC_BC_DEL, true);
    this->enableItem(IDC_BC_EDI, true);

    this->enableItem(IDC_BC_UP, (lb_sel > 0));
    int lb_max = this->msgItem(IDC_LB_CHN, LB_GETCOUNT) - 1;
    this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
  } else {
    this->setItemText(IDC_EC_READ2, L"<no selection>");
    this->setItemText(IDC_EC_READ3, L"<no selection>");
    this->setItemText(IDC_EC_READ4, L"<no selection>");

    this->enableItem(IDC_SC_LBL02, false);
    this->enableItem(IDC_EC_READ2, false);
    this->enableItem(IDC_SC_LBL03, false);
    this->enableItem(IDC_EC_READ3, false);
    this->enableItem(IDC_SC_LBL04, false);
    this->enableItem(IDC_EC_READ4, false);

    this->enableItem(IDC_BC_DEL, false);
    this->enableItem(IDC_BC_EDI, false);

    this->enableItem(IDC_BC_UP, false);
    this->enableItem(IDC_BC_DN, false);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_bc_up_pressed()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OM_MAX_ITEM];
  int idx;

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_CHN, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_CHN, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_CHN, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_CHN, LB_SETITEMDATA, lb_sel, idx);

  this->enableItem(IDC_BC_UP, (lb_sel > 1));
  this->enableItem(IDC_BC_DN, true);

  // user modified parameter, notify it
  this->paramCheck(HUB_PROP_CHN_ORDER);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_bc_dn_pressed()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  // get count of item in List-Box as index to for insertion
  int lb_max = this->msgItem(IDC_LB_CHN, LB_GETCOUNT) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OM_MAX_ITEM];
  int idx;

  this->msgItem(IDC_LB_CHN, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);
  this->msgItem(IDC_LB_CHN, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_CHN, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_CHN, LB_SETITEMDATA, lb_sel, idx);
  this->msgItem(IDC_LB_CHN, LB_SETCURSEL, lb_sel);

  this->enableItem(IDC_BC_UP, true);
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));

  // user modified parameter, notify it
  this->paramCheck(HUB_PROP_CHN_ORDER);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_bc_del_pressed()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!ModHub) return;

  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  int chn_id = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);

  if(chn_id < 0) return;

  OmModChan* ModChan = ModHub->getChannel(chn_id);

  // warns the user before committing the irreparable
  if(!Om_dlgBox_ca(this->_hwnd, L"Mod Hub properties", IDI_QRY, L"Delete Mod Channel",
                   L"The operation will permanently delete the Mod Channel \""+
                   ModChan->title()+L"\" and its associated data."))
    return;

  // disable all dialog controls
  this->enableItem(IDC_LB_CHN, true);

  this->enableItem(IDC_SC_LBL02, false);
  this->enableItem(IDC_EC_READ2, false);
  this->enableItem(IDC_SC_LBL03, false);
  this->enableItem(IDC_EC_READ3, false);
  this->enableItem(IDC_SC_LBL04, false);
  this->enableItem(IDC_EC_READ4, false);

  this->enableItem(IDC_BC_DEL,  false);
  this->enableItem(IDC_BC_EDI, false);

  // here we go for Mod Channel delete
  if(!this->_chn_delete_hth) {

    this->_chn_delete_chn = chn_id;

    // launch thread
    this->_chn_delete_hth = Om_createThread(OmUiPropHubChn::_chn_delete_run_fn, this);
    this->_chn_delete_hwo = Om_waitForThread(this->_chn_delete_hth, OmUiPropHubChn::_chn_delete_end_fn, this);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_bd_edi_pressed()
{
  OmModHub* pModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!pModHub) return;

  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  int chn_id = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);

  if(chn_id >= 0) {
    // open the Mod Channel Properties dialog
    OmUiPropChn* pUiPropLoc = static_cast<OmUiPropChn*>(this->siblingById(IDD_PROP_CHN));
    pUiPropLoc->setModChan(pModHub->getChannel(chn_id));
    pUiPropLoc->open();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_bc_add_pressed()
{
  OmModHub* pModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!pModHub) return;

  // open add Mod Channel dialog
  OmUiAddChn* pUiNewLoc = static_cast<OmUiAddChn*>(this->siblingById(IDD_ADD_CHN));
  pUiNewLoc->setModHub(pModHub);
  pUiNewLoc->open(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_onTabInit()
{
  // Set buttons inner icons
  this->setBmIcon(IDC_BC_ADD, Om_getResIcon(this->_hins, IDI_BT_ADD));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDI_BT_REM));
  this->setBmIcon(IDC_BC_EDI, Om_getResIcon(this->_hins, IDI_BT_EDI));
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(this->_hins, IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(this->_hins, IDI_BT_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_LB_CHN,  L"Mod Channels list");
  this->_createTooltip(IDC_BC_UP,   L"Move up in list");
  this->_createTooltip(IDC_BC_DN,   L"Move down in list");
  this->_createTooltip(IDC_BC_DEL,  L"Delete Mod Channel and its associated data");
  this->_createTooltip(IDC_BC_ADD,  L"Configure a new Mod Channel for this Mod Hub");
  this->_createTooltip(IDC_BC_EDI,  L"Modify Mod Channel properties");

  SetWindowTheme(this->getItem(IDC_LB_CHN),L"EXPLORER",nullptr);

  // Update values
  this->_onTabRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_onTabResize()
{
  // Mod Channel list Label & ListBox
  this->_setItemPos(IDC_SC_LBL01, 50, 15, 64, 9);
  this->_setItemPos(IDC_LB_CHN, 50, 25, this->cliUnitX()-107, 30);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliUnitX()-55, 25, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->cliUnitX()-55, 40, 16, 15);
  // Remove & Modify Buttons
  this->_setItemPos(IDC_BC_DEL, 50, 57, 50, 14);
  this->_setItemPos(IDC_BC_EDI, 105, 57, 50, 14);
  // Add button
  this->_setItemPos(IDC_BC_ADD, this->cliUnitX()-108, 57, 50, 14);
  // Target path Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 50, 75, 110, 9);
  this->_setItemPos(IDC_EC_READ2, 115, 75, this->cliUnitX()-155, 13);
  // Mods Library Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 50, 87, 110, 9);
  this->_setItemPos(IDC_EC_READ3, 115, 87, this->cliUnitX()-155, 13);
  // Mods Backup Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 50, 99, 110, 9);
  this->_setItemPos(IDC_EC_READ4, 115, 99, this->cliUnitX()-155, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_onTabRefresh()
{
  OmModHub* pModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!pModHub) return;

  this->enableItem(IDC_LB_CHN, true);

  this->msgItem(IDC_LB_CHN, LB_RESETCONTENT);

  for(unsigned i = 0; i < pModHub->channelCount(); ++i) {
    this->msgItem(IDC_LB_CHN, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(pModHub->getChannel(i)->title().c_str()));
    this->msgItem(IDC_LB_CHN, LB_SETITEMDATA, i, i); // for Mod Channel index reordering
  }

  // Set controls default states and parameters
  this->setItemText(IDC_EC_READ2, L"<no selection>");
  this->setItemText(IDC_EC_READ3, L"<no selection>");
  this->setItemText(IDC_EC_READ4, L"<no selection>");

  this->enableItem(IDC_SC_LBL02, false);
  this->enableItem(IDC_EC_READ2, false);
  this->enableItem(IDC_SC_LBL03, false);
  this->enableItem(IDC_EC_READ3, false);
  this->enableItem(IDC_SC_LBL04, false);
  this->enableItem(IDC_EC_READ4, false);

  this->enableItem(IDC_BC_DEL,  false);
  this->enableItem(IDC_BC_EDI, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropHubChn::_onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_CHN: //< Mod Channel(s) list List-Box
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_lb_chn_on_selchg();
      break;

    case IDC_BC_UP: //< Up Buttn
      this->_bc_up_pressed();
      break;

    case IDC_BC_DN: //< Down Buttn
      this->_bc_dn_pressed();
      break;

    case IDC_BC_DEL: //< "Remove" Button
      this->_bc_del_pressed();
      break;

    case IDC_BC_EDI: //< "Modify" Button
      this->_bd_edi_pressed();
      break;

    case IDC_BC_ADD: //< "New" Button
      this->_bc_add_pressed();
      break;
    }
  }

  return false;
}
