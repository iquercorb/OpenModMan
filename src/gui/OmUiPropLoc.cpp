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
#include "gui/OmUiPropLoc.h"
#include "gui/OmUiPropLocStg.h"
#include "gui/OmUiPropLocBck.h"
#include "gui/OmUiProgress.h"
#include "gui/OmUiMain.h"


/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the moveBackup_fth
/// thread finished his job.
///
#define UWM_MOVEBACKUP_DONE     (WM_APP+4)


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLoc::OmUiPropLoc(HINSTANCE hins) : OmDialogProp(hins),
  _location(nullptr),
  _moveBackup_dest(),
  _moveBackup_cust(false),
  _moveBackup_hth(nullptr)
{
  // create tab dialogs
  this->_addPage(L"Settings", new OmUiPropLocStg(hins));
  this->_addPage(L"Backups", new OmUiPropLocBck(hins));

  // creates child sub-dialogs
  this->addChild(new OmUiProgress(hins)); //< for Location backup transfer
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLoc::~OmUiPropLoc()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropLoc::id()  const
{
  return IDD_PROP_LOC;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLoc::checkChanges()
{
  OmLocation* location = reinterpret_cast<OmLocation*>(this->_location);
  OmUiPropLocBck* uiPropLocBck  = reinterpret_cast<OmUiPropLocBck*>(this->childById(IDD_PROP_LOC_BCK));
  OmUiPropLocStg* uiPropLocStg  = reinterpret_cast<OmUiPropLocStg*>(this->childById(IDD_PROP_LOC_STG));

  bool changed = false;

  wchar_t wcbuf[OMM_MAX_PATH];

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title
    GetDlgItemTextW(uiPropLocStg->hwnd(), IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
    if(location->title() != wcbuf) changed = true;
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Location Destination path
    GetDlgItemTextW(uiPropLocStg->hwnd(), IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
    if(location->installDir() != wcbuf) changed = true;
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Location Library path
    bool chk01 = SendMessage(GetDlgItem(uiPropLocStg->hwnd(), IDC_BC_CHK01), BM_GETCHECK, 0, 0);
    GetDlgItemTextW(uiPropLocStg->hwnd(), IDC_EC_INPT3, wcbuf, OMM_MAX_PATH);
    if(chk01) {
      if(location->libraryDir() != wcbuf || !location->hasCustLibraryDir()) changed = true;
    } else {
      if(location->hasCustLibraryDir()) changed = true;
    }
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Location Backup path
    bool chk02 = SendMessage(GetDlgItem(uiPropLocStg->hwnd(), IDC_BC_CHK02), BM_GETCHECK, 0, 0);
    GetDlgItemTextW(uiPropLocStg->hwnd(), IDC_EC_INPT4, wcbuf, OMM_MAX_PATH);
    if(chk02) {
      if(location->backupDir() != wcbuf || !location->hasCustBackupDir()) changed = true;
    } else {
      if(location->hasCustBackupDir()) changed = true;
    }
  }

  if(uiPropLocBck->hasChParam(LOC_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level
    if(SendMessage(GetDlgItem(uiPropLocBck->hwnd(),IDC_BC_CHK01), BM_GETCHECK, 0, 0)) {
      int cb_sel = SendMessageW(GetDlgItem(uiPropLocBck->hwnd(), IDC_CB_LEVEL), CB_GETCURSEL, 0, 0);
      if(location->backupZipLevel() != cb_sel) changed = true;
    } else {
      if(location->backupZipLevel() != -1) changed = true;
    }
  }

  // enable Apply button
  if(IsWindowEnabled(GetDlgItem(this->_hwnd, IDC_BC_APPLY)) != changed) {
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), changed);
    //if(changed) SetFocus(GetDlgItem(this->_hwnd, IDC_BC_APPLY));
  }

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLoc::applyChanges()
{
  OmLocation* location = reinterpret_cast<OmLocation*>(this->_location);
  OmUiPropLocBck* uiPropLocBck  = reinterpret_cast<OmUiPropLocBck*>(this->childById(IDD_PROP_LOC_BCK));
  OmUiPropLocStg* uiPropLocStg  = reinterpret_cast<OmUiPropLocStg*>(this->childById(IDD_PROP_LOC_STG));

  wchar_t inpt1[OMM_MAX_PATH];
  wchar_t inpt2[OMM_MAX_PATH];
  wchar_t inpt3[OMM_MAX_PATH];
  wchar_t inpt4[OMM_MAX_PATH];

  bool chk01, chk02;

  // Step 1, verify everything
  if(uiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title
    GetDlgItemTextW(uiPropLocStg->hwnd(), IDC_EC_INPT1, inpt1, OMM_MAX_PATH);
    if(!wcslen(inpt1)) {
      Om_dialogBoxErr(this->_hwnd, L"Invalid Location title",
                                L"Please enter a title.");
      return false;
    }
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Location Destination path
    GetDlgItemTextW(uiPropLocStg->hwnd(), IDC_EC_INPT2, inpt2, OMM_MAX_PATH);
    if(wcslen(inpt2)) {
      if(!Om_isDir(inpt2)) {
        Om_dialogBoxErr(this->_hwnd, L"Invalid Location Destination folder",
                                  L"Please select an existing folder for "
                                  L"packages installation destination.");
        return false;
      }
    } else {
      Om_dialogBoxErr(this->_hwnd, L"Invalid Location Destination path",
                                L"Please enter a valid path for Location "
                                L"packages installation destination folder.");
      return false;
    }
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Location Library path
    GetDlgItemTextW(uiPropLocStg->hwnd(), IDC_EC_INPT3, inpt3, OMM_MAX_PATH);
    chk01 = SendMessage(GetDlgItem(uiPropLocStg->hwnd(), IDC_BC_CHK01), BM_GETCHECK, 0, 0);
    if(chk01) { //< Custom Library folder Check-Box checked
      if(wcslen(inpt3)) {
        if(!Om_isDir(inpt3)) {
          Om_dialogBoxErr(this->_hwnd, L"Invalid Location Library folder",
                                    L"Please select an existing folder for "
                                    L"Location packages Library.");
          return false;
        }
      } else {
        Om_dialogBoxErr(this->_hwnd, L"Invalid Location Library path",
                                  L"Please enter a valid path for Location "
                                  L"packages Library folder.");
        return false;
      }
    }
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Location Backup path
    GetDlgItemTextW(uiPropLocStg->hwnd(), IDC_EC_INPT4, inpt4, OMM_MAX_PATH);
    chk02 = SendMessage(GetDlgItem(uiPropLocStg->hwnd(), IDC_BC_CHK02), BM_GETCHECK, 0, 0);
    if(chk02) { //< Custom Backup folder Check-Box checked
      if(wcslen(inpt4)) {
        if(!Om_isDir(inpt4)) {
          Om_dialogBoxErr(this->_hwnd, L"Invalid Location Backup folder",
                                    L"Please select an existing folder for "
                                    L"Location Backup data location.");
          return false;
        }
      } else {
        Om_dialogBoxErr(this->_hwnd, L"Invalid Location Backup path",
                                  L"Please enter a valid path for "
                                  L"Location Backup data location.");
        return false;
      }
    }
  }

  // Step 2, save changes

  if(uiPropLocBck->hasChParam(LOC_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level
    if(SendMessage(GetDlgItem(uiPropLocBck->hwnd(),IDC_BC_CHK01), BM_GETCHECK, 0, 0)) {
      int cb_sel = SendMessageW(GetDlgItem(uiPropLocBck->hwnd(), IDC_CB_LEVEL), CB_GETCURSEL, 0, 0);
      if(cb_sel >= 0)
        location->setBackupZipLevel(cb_sel);
    } else {
      // disable zipped backups
      location->setBackupZipLevel(-1);
    }

    // Reset parameter as unmodified
    uiPropLocStg->setChParam(LOC_PROP_BCK_COMP_LEVEL, false);
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Location Install path
    location->setInstallDir(inpt2);

    // Reset parameter as unmodified
    uiPropLocStg->setChParam(LOC_PROP_STG_INSTALL, false);
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Location Library path
    if(chk01) {
      location->setCustLibraryDir(inpt3);
    } else {
      location->remCustLibraryDir();
    }
    // Reset parameter as unmodified
    uiPropLocStg->setChParam(LOC_PROP_STG_LIBRARY, false);
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Location Backup path

    // check whether we need to transfer backup data, if yes we
    // launch the process via a new thread with progress dialog window. The
    // Location backup setting will be properly modified withing the
    // dedicates thread
    if(location->backupDir() != inpt4) {

      this->_moveBackup_init(inpt4, chk02);

      // if backup transfer thread is running, we do not quit since it will
      // end the process before it ends. We will wait for the UWM_TRANSFER_ENDED
      // message sent from the thread, then quit this dialog safely at this
      // moment
      return false;

    } else {
      // uncheck the unnecessary "custom" flag
      if(!chk02 && location->hasCustBackupDir())
        location->remCustBackupDir();
    }

    // Reset parameter as unmodified
    uiPropLocStg->setChParam(LOC_PROP_STG_BACKUP, false);
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title

    location->setTitle(inpt1);

    wstring mesg =  L"Location title changed, do you also want to rename "
                    L"folder and definition file according the new title ?";

    if(Om_dialogBoxQuerry(this->_hwnd, L"Change Location title", mesg)) {

      // To prevent crash during operation we unselect location in the main dialog
      reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(true);

      location->rename(inpt1, this->_hwnd);

      // Back to main dialog window to normal state
      reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(false);
    }

    // Reset parameter as unmodified
    uiPropLocStg->setChParam(LOC_PROP_STG_TITLE, false);
  }

  // disable Apply button
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), false);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_onShow()
{
  // Initialize TabControl with pages dialogs
  this->_pagesOnShow(IDC_TC_TABS1);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_onResize()
{
  // TabControl
  this->_setControlPos(IDC_TC_TABS1, 4, 5, this->width()-8, this->height()-28);
  // Resize page dialogs according IDC_TC_TABS1
  this->_pagesOnResize(IDC_TC_TABS1);
  // OK Button
  this->_setControlPos(IDC_BC_OK, this->width()-161, this->height()-19, 50, 14);
  // Cancel Button
  this->_setControlPos(IDC_BC_CANCEL, this->width()-108, this->height()-19, 50, 14);
  // Apply Button
  this->_setControlPos(IDC_BC_APPLY, this->width()-54, this->height()-19, 50, 14);

  // force buttons to redraw to prevent artifacts
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_OK), nullptr, true);
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_CANCEL), nullptr, true);
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_APPLY), nullptr, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLoc::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_MOVEBACKUP_DONE is a custom message sent from backup transfer thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_MOVEBACKUP_DONE) {
    // end the backup transfer process
    this->_moveBackup_stop();
    // quit this dialog (we come from a click on IDC_BC_OK)
    this->quit();
    // refresh the main window dialog, this will also refresh this one
    this->root()->refresh();
  }

  if(uMsg == WM_NOTIFY) {
    // handle TabControl page selection change
    this->_pagesOnNotify(IDC_TC_TABS1, wParam, lParam);
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_APPLY:
      if(this->applyChanges()) {
        // refresh all tree from the main dialog
        this->root()->refresh();
      }
      break;

    case IDC_BC_OK:
      if(this->checkChanges()) {
        if(this->applyChanges()) {
          // quit the dialog
          this->quit();
          // refresh all tree from the main dialog
          this->root()->refresh();
        }
      } else {
        // quit the dialog
        this->quit();
      }
      break; // case IDC_BC_OK:

    case IDC_BC_CANCEL:
      this->quit();
      break; // case IDC_BC_CANCEL:
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_moveBackup_init(const wstring& dest, bool custom)
{
  // To prevent crash during operation we unselect location in the main dialog
  reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(true);

  OmUiProgress* uiProgress = reinterpret_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));

  uiProgress->open(true);
  uiProgress->setCaption(L"Location Backup transfer");
  uiProgress->setTitle(L"Transferring backups data...");

  this->_moveBackup_dest = dest;
  this->_moveBackup_cust = custom;

  DWORD dwid;
  this->_moveBackup_hth = CreateThread(nullptr, 0, this->_moveBackup_fth, this, 0, &dwid);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_moveBackup_stop()
{
  if(this->_moveBackup_hth) {
    WaitForSingleObject(this->_moveBackup_hth, INFINITE);
    CloseHandle(this->_moveBackup_hth);
    this->_moveBackup_hth = nullptr;
  }

  // Back to main dialog window to normal state
  reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(false);

  // Close progress dialog
  reinterpret_cast<OmUiProgress*>(this->childById(IDD_PROGRESS))->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropLoc::_moveBackup_fth(void* arg)
{
  OmUiPropLoc* self = reinterpret_cast<OmUiPropLoc*>(arg);
  OmLocation* location = reinterpret_cast<OmLocation*>(self->_location);

  // launch move process only if directory not empty
  if(!Om_isDirEmpty(location->backupDir())) {

    OmUiProgress* uiProgress = static_cast<OmUiProgress*>(self->childById(IDD_PROGRESS));

    HWND hPb = (HWND)uiProgress->getProgressBar();
    HWND hSc = (HWND)uiProgress->getStaticComment();

    location->moveBackups(self->_moveBackup_dest, uiProgress->hwnd(), hPb, hSc, uiProgress->getAbortPtr());
  }

  // modify the backup path for the Location
  if(self->_moveBackup_cust) {
    location->setCustBackupDir(self->_moveBackup_dest);
  } else {
    location->remCustBackupDir();
  }

  // send message to window, to proper quit dialog and finish
  PostMessage(self->_hwnd, UWM_MOVEBACKUP_DONE, 0, 0);

  return 0;
}
