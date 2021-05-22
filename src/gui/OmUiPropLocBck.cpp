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
#include "gui/OmUiMain.h"
#include "gui/OmUiProgress.h"
#include "gui/OmUiPropLocBck.h"
#include "gui/OmUiPropLoc.h"

/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the backupDcard_fth
/// thread finished his job.
///
#define UWM_BACKDISCARD_DONE     (WM_APP+1)

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocBck::OmUiPropLocBck(HINSTANCE hins) : OmDialog(hins),
  _backupDcard_hth(nullptr)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i)
    this->_chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocBck::~OmUiPropLocBck()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropLocBck::id() const
{
  return IDD_PROP_LOC_BCK;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::setChParam(unsigned i, bool en)
{
  _chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_backupDcard_init()
{
  // To prevent crash during operation we unselect location in the main dialog
  static_cast<OmUiMain*>(this->root())->setSafeEdit(true);

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS));

  pUiProgress->open(true);
  pUiProgress->setTitle(L"Discard Location backups data");
  pUiProgress->setDesc(L"Backups data deletion");

  DWORD dwId;
  this->_backupDcard_hth = CreateThread(nullptr, 0, this->_backupDcard_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_backupDcard_stop()
{
  DWORD exitCode;

  if(this->_backupDcard_hth) {
    WaitForSingleObject(this->_backupDcard_hth, INFINITE);
    GetExitCodeThread(this->_backupDcard_hth, &exitCode);
    CloseHandle(this->_backupDcard_hth);
    this->_backupDcard_hth = nullptr;
  }

  // quit the progress dialog
  static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS))->quit();

  // Back to main dialog window to normal state
  static_cast<OmUiMain*>(this->root())->setSafeEdit(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropLocBck::_backupDcard_fth(void* arg)
{
  OmUiPropLocBck* self = static_cast<OmUiPropLocBck*>(arg);

  OmLocation* pLoc = static_cast<OmUiPropLoc*>(self->_parent)->location();

  if(pLoc == nullptr)
    return 1;

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(self->siblingById(IDD_PROGRESS));

  HWND hPb = pUiProgress->getPbHandle();
  HWND hSc = pUiProgress->getDetailScHandle();

  DWORD exitCode = 0;

  // launch backups data deletion process
  if(!pLoc->backupsDiscard(hPb, hSc, pUiProgress->getAbortPtr())) {
    // we encounter error during backup data purge
    Om_dialogBoxErr(pUiProgress->hwnd(), L"Backups data deletion error", pLoc->lastError());
    exitCode = 1;
  }

  // sends message to window to inform process ended
  PostMessage(self->_hwnd, UWM_BACKDISCARD_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CHK01,  L"Store backup data as zip archives");
  this->_createTooltip(IDC_CB_LEVEL,  L"Compression level for backup zip files");

  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->location();

  // add items in combo box
  HWND hCb = this->getItem(IDC_CB_LEVEL);

  unsigned cb_cnt = SendMessageW(hCb, CB_GETCOUNT, 0, 0);
  if(!cb_cnt) {
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None ( very fast )"));
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Low ( fast )"));
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal ( slow )"));
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Best ( very slow )"));
  }

  if(pLoc == nullptr)
    return;

  int comp_levl = pLoc->backupZipLevel();

  if(comp_levl >= 0) {

    this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 1);

    EnableWindow(hCb, true);

    switch(comp_levl)
    {
    case 1:
      SendMessageW(hCb, CB_SETCURSEL, 1, 0);
      break;
    case 2:
      SendMessageW(hCb, CB_SETCURSEL, 2, 0);
      break;
    case 3:
      SendMessageW(hCb, CB_SETCURSEL, 3, 0);
      break;
    default:
      SendMessageW(hCb, CB_SETCURSEL, 0, 0);
      break;
    }

  } else {

    this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 0);

    EnableWindow(hCb, false);

    SendMessageW(hCb, CB_SETCURSEL, 0, 0);
  }

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_onResize()
{
  // Compressed Backup CheckBox
  this->_setItemPos(IDC_BC_CHK01, 50, 20, 120, 9);
  // Compression level Label & ComboBox
  this->_setItemPos(IDC_SC_LBL01, 50, 40, 120, 9);
  this->_setItemPos(IDC_CB_LEVEL, 50, 50, this->width()-100, 14);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_LEVEL), nullptr, true);
  // Maintenance operations Label
  this->_setItemPos(IDC_SC_LBL02, 50, 80, 120, 9);
  // Discard backups Button
  this->_setItemPos(IDC_BC_DEL, 50, 90, 80, 15);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLocBck::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_BACKDISCARD_DONE is a custom message sent from Location backups discard thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_BACKDISCARD_DONE) {
    // end the Location backups deletion process
    this->_backupDcard_stop();
  }

  if(uMsg == WM_COMMAND) {

    bool bm_chk;

    switch(LOWORD(wParam))
    {
    case IDC_BC_CHK01:
      bm_chk = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);
      this->enableItem(IDC_CB_LEVEL, bm_chk);
      this->setChParam(LOC_PROP_BCK_COMP_LEVEL, true);
      break;

    case IDC_CB_LEVEL:
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        // user modified parameter, notify it
        this->setChParam(LOC_PROP_BCK_COMP_LEVEL, true);
      }
      break;

    case IDC_BC_DEL: {
        // warns the user before committing the irreparable
        wstring wrn = L"This will permanently delete all existing "
                      L"backups data without restoring them (which should "
                      "never be done except in emergency situation)."

                      L"\n\nDiscard all backups data for this Location ?";

        if(Om_dialogBoxQuerryWarn(this->_hwnd, L"Discard backups data", wrn)) {
          // delete the Location
          this->_backupDcard_init();
        }
      }
      break;
    }
  }

  return false;
}
