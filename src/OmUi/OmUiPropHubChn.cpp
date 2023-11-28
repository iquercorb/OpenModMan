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
#include "OmUiPropHub.h"
#include "OmUiPropChn.h"
#include "OmUiProgress.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropHubChn.h"

/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the _delChn_fth
/// thread finished his job.
///
#define UWM_BACKPURGE_DONE     (WM_APP+1)


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubChn::OmUiPropHubChn(HINSTANCE hins) : OmDialogPropTab(hins),
  _delChn_hth(nullptr),
  _delChn_id(-1)
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
void OmUiPropHubChn::_delChn_init(int id)
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!ModHub)
    return;

  // store Mod Channel id
  this->_delChn_id = id;
  OmModChan* ModChan = ModHub->getChannel(id);

  // To prevent crash during operation we unselect location in the main dialog
  static_cast<OmUiMan*>(this->root())->safemode(true);

  // unselect location
  ModHub->selectChannel(-1);

  // if Mod Channel does not have backup data, we can bypass the purge and
  // do directly to the end
  if(ModChan->hasBackupData()) {

    OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS));

    pUiProgress->open(true);
    pUiProgress->setCaption(L"Delete Mod Channel");
    pUiProgress->setScHeadText(L"Restoring all backup data");

    DWORD dwId;
    this->_delChn_hth = CreateThread(nullptr, 0, this->_delChn_fth, this, 0, &dwId);

  } else {

    // directly delete the location
    this->_delChn_stop();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_delChn_stop()
{
  DWORD exitCode;

  if(this->_delChn_hth) {
    WaitForSingleObject(this->_delChn_hth, INFINITE);
    GetExitCodeThread(this->_delChn_hth, &exitCode);
    CloseHandle(this->_delChn_hth);
    this->_delChn_hth = nullptr;
  }

  // quit the progress dialog
  static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS))->quit();

  OmModHub* pModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!pModHub) return;

  OmWString msg;

  // check whether purge succeed
  if(exitCode == 0) {

    // backup data purged, now delete Mod Channel
    if(!pModHub->deleteChannel(this->_delChn_id)) {
      Om_dlgBox_ok(this->_hwnd, L"Hub properties", IDI_WRN,
                L"Channel remove error", L"Channel "
                "remove process encountered error(s), some file may "
                "had not properly deleted. Please read debug log for details.");
    }

  } else {
    // an error occurred during backup purge
    Om_dlgBox_ok(this->_hwnd, L"Hub properties", IDI_WRN,
                L"Channel backup purge error", L"Channel "
                "backup purge process encountered error(s), some backup data may "
                "had not properly restored. Please read debug log for details.");
  }

  // select the first location in list
  pModHub->selectChannel(0);

  // Back to main dialog window to normal state
  static_cast<OmUiMan*>(this->root())->safemode(false);

  // refresh all dialogs from root
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropHubChn::_delChn_fth(void* arg)
{
  OmUiPropHubChn* self = static_cast<OmUiPropHubChn*>(arg);

  OmModHub* pModHub = static_cast<OmUiPropHub*>(self->_parent)->modHub();
  if(!pModHub) return 1;
  OmModChan* pModChan = pModHub->getChannel(self->_delChn_id);
  if(!pModChan) return 1;

  DWORD exitCode = 0;

  if(!pModChan->purgeBackupData(&self->_delChn_progress_cb, self->siblingById(IDD_PROGRESS))) {
    exitCode = 1; //< report error
  }

  // sends message to window to inform process ended
  PostMessage(self->_hwnd, UWM_BACKPURGE_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropHubChn::_delChn_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t data)
{
  OmUiProgress* pUiProgress = reinterpret_cast<OmUiProgress*>(ptr);

  if(data) {
    pUiProgress->setScItemText(reinterpret_cast<wchar_t*>(data));
  }
  pUiProgress->setPbRange(0, tot);
  pUiProgress->setPbPos(cur);

  return !pUiProgress->abortGet();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_onLbLoclsSel()
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
void OmUiPropHubChn::_onBcUpModChan()
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
void OmUiPropHubChn::_onBcDnModChan()
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
void OmUiPropHubChn::_onBcDelModChan()
{
  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!ModHub)
    return;

  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  int chn_id = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);

  if(chn_id < 0) return;

  OmModChan* ModChan = ModHub->getChannel(chn_id);

  // warns the user before committing the irreparable
  if(!Om_dlgBox_ca(this->_hwnd, L"Mod Hub properties", IDI_QRY,
            L"Delete Mod Channel", L"The operation will permanently delete "
            "the Mod Channel \""+ModChan->title()+L"\" and its associated data."))
  {
    return;
  }

  if(ModChan->hasBackupData()) {

    if(!Om_dlgBox_ca(this->_hwnd, L"Mod Hub properties", IDI_QRY,
              L"Remaining backup data", L"The Mod Channel currently have "
              "installed packages, the deletion process will uninstall them "
              "and restore all backup data."))
    {
      return;
    }
  }

  // here we go for Mod Channel delete
  this->_delChn_init(chn_id);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubChn::_onBcEdiModChan()
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
void OmUiPropHubChn::_onBcAddModChan()
{
  OmModHub* pModHub = static_cast<OmUiPropHub*>(this->_parent)->modHub();
  if(!pModHub) return;

  // open add Mod Channel dialog
  OmUiAddChn* pUiNewLoc = static_cast<OmUiAddChn*>(this->siblingById(IDD_ADD_CHN));
  pUiNewLoc->ctxSet(pModHub);
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
  this->setBmIcon(IDC_BC_EDI, Om_getResIcon(this->_hins, IDI_BT_MOD));
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(this->_hins, IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(this->_hins, IDI_BT_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_LB_CHN,  L"Mod Channels list");
  this->_createTooltip(IDC_BC_UP,   L"Move up in list");
  this->_createTooltip(IDC_BC_DN,   L"Move down in list");
  this->_createTooltip(IDC_BC_DEL,  L"Delete Mod Channel and its associated data");
  this->_createTooltip(IDC_BC_ADD,  L"Configure a new Mod Channel for this Mod Hub");
  this->_createTooltip(IDC_BC_EDI,  L"Modify Mod Channel properties");

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
  // UWM_BACKPURGE_DONE is a custom message sent from Mod Channel backups purge thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_BACKPURGE_DONE) {
    // end the removing Mod Channel process
    this->_delChn_stop();
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_CHN: //< Mod Channel(s) list List-Box
      this->_onLbLoclsSel();
      break;

    case IDC_BC_UP: //< Up Buttn
      this->_onBcUpModChan();
      break;

    case IDC_BC_DN: //< Down Buttn
      this->_onBcDnModChan();
      break;

    case IDC_BC_DEL: //< "Remove" Button
      this->_onBcDelModChan();
      break;

    case IDC_BC_EDI: //< "Modify" Button
      this->_onBcEdiModChan();
      break;

    case IDC_BC_ADD: //< "New" Button
      this->_onBcAddModChan();
      break;
    }
  }

  return false;
}
