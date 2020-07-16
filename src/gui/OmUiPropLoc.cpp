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
  OmUiPropLocStg* uiPropLocStg  = reinterpret_cast<OmUiPropLocStg*>(this->childById(IDD_PROP_LOC_STG));
  OmUiPropLocBck* uiPropLocBck  = reinterpret_cast<OmUiPropLocBck*>(this->childById(IDD_PROP_LOC_BCK));

  bool changed = false;

  wstring item_str;

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title
    uiPropLocStg->getItemText(IDC_EC_INPT1, item_str);
    if(location->title() != item_str) changed = true;
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Location Destination path
    uiPropLocStg->getItemText(IDC_EC_INPT2, item_str);
    if(location->installDir() != item_str) changed = true;
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Location Library path
    if(uiPropLocStg->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
      uiPropLocStg->getItemText(IDC_EC_INPT3, item_str);
      if(location->libraryDir() != item_str || !location->hasCustLibraryDir())
        changed = true;
    } else {
      if(location->hasCustLibraryDir()) changed = true;
    }
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Location Backup path
    if(uiPropLocStg->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
      uiPropLocStg->getItemText(IDC_EC_INPT4, item_str);
      if(location->backupDir() != item_str || !location->hasCustBackupDir()) changed = true;
    } else {
      if(location->hasCustBackupDir()) changed = true;
    }
  }

  if(uiPropLocBck->hasChParam(LOC_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level
    if(uiPropLocBck->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
      int cb_sel = uiPropLocBck->msgItem(IDC_CB_LEVEL, CB_GETCURSEL);
      if(location->backupZipLevel() != cb_sel) changed = true;
    } else {
      if(location->backupZipLevel() != -1) changed = true;
    }
  }

  // enable Apply button
  this->enableItem(IDC_BC_APPLY, changed);

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

  wstring loc_name, loc_dst, loc_lib, loc_bck;

  bool cust_lib, cust_bck;

  // Step 1, verify everything
  if(uiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title
    uiPropLocStg->getItemText(IDC_EC_INPT1, loc_name);
    if(loc_name.empty()) {
      Om_dialogBoxErr(this->_hwnd, L"Empty Location title", L"Please enter a title.");
      return false;
    }
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Location Destination path
    uiPropLocStg->getItemText(IDC_EC_INPT2, loc_dst);
    if(!loc_dst.empty()) {
      if(!Om_isDir(loc_dst)) {
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
    cust_lib = uiPropLocStg->msgItem(IDC_BC_CHK01, BM_GETCHECK);
    if(cust_lib) { //< Custom Library folder Check-Box checked
      uiPropLocStg->getItemText(IDC_EC_INPT3, loc_lib);
      if(!loc_lib.empty()) {
        if(!Om_isDir(loc_lib)) {
          Om_dialogBoxErr(this->_hwnd, L"Invalid Location Library folder",
                                        L"Please select an existing folder as "
                                        L"Location's packages Library.");
          return false;
        }
      } else {
        Om_dialogBoxErr(this->_hwnd, L"Invalid Location Library path",
                                      L"Please enter a valid path for "
                                      L"Location's packages Library folder.");
        return false;
      }
    }
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Location Backup path
    cust_bck = uiPropLocStg->msgItem(IDC_BC_CHK02, BM_GETCHECK);
    if(cust_bck) { //< Custom Backup folder Check-Box checked
      uiPropLocStg->getItemText(IDC_EC_INPT4, loc_bck);
      if(!loc_bck.empty()) {
        if(!Om_isDir(loc_bck)) {
          Om_dialogBoxErr(this->_hwnd, L"Invalid Location Backup folder",
                                        L"Please select an existing folder to "
                                        L"store Location's Backup data.");
          return false;
        }
      } else {
        Om_dialogBoxErr(this->_hwnd, L"Invalid Location Backup path",
                                      L"Please enter a valid path for "
                                      L"Location's Backup data folder.");
        return false;
      }
    }
  }

  // Step 2, save changes
  if(uiPropLocBck->hasChParam(LOC_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level
    if(uiPropLocBck->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
      int cb_sel = uiPropLocBck->msgItem(IDC_CB_LEVEL, CB_GETCURSEL);
      if(cb_sel >= 0) location->setBackupZipLevel(cb_sel);
    } else {
      // disable zipped backups
      location->setBackupZipLevel(-1);
    }

    // Reset parameter as unmodified
    uiPropLocStg->setChParam(LOC_PROP_BCK_COMP_LEVEL, false);
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Location Install path
    location->setInstallDir(loc_dst);

    // Reset parameter as unmodified
    uiPropLocStg->setChParam(LOC_PROP_STG_INSTALL, false);
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Location Library path
    if(cust_lib) {
      location->setCustLibraryDir(loc_lib);
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
    if(location->backupDir() != loc_bck) {

      this->_moveBackup_init();

      // if backup transfer thread is running, we do not quit since it will
      // end the process before it ends. We will wait for the UWM_TRANSFER_ENDED
      // message sent from the thread, then quit this dialog safely at this
      // moment
      return false;

    } else {
      // uncheck the unnecessary "custom" flag
      if(!cust_bck && location->hasCustBackupDir())
        location->remCustBackupDir();
    }

    // Reset parameter as unmodified
    uiPropLocStg->setChParam(LOC_PROP_STG_BACKUP, false);
  }

  if(uiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title

    location->setTitle(loc_name);

    wstring mesg =  L"Location title changed, do you also want to rename "
                    L"folder and definition file according the new title ?";

    if(Om_dialogBoxQuerry(this->_hwnd, L"Change Location title", mesg)) {

      // To prevent crash during operation we unselect location in the main dialog
      reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(true);

      location->rename(loc_name, this->_hwnd);

      // Back to main dialog window to normal state
      reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(false);
    }

    // Reset parameter as unmodified
    uiPropLocStg->setChParam(LOC_PROP_STG_TITLE, false);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  // refresh all tree from the main dialog
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLoc::_onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_MOVEBACKUP_DONE is a custom message sent from backup transfer thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_MOVEBACKUP_DONE) {
    // end the backup transfer process
    this->_moveBackup_stop();
    // refresh the main window dialog, this will also refresh this one
    this->root()->refresh();
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_moveBackup_init()
{
  // To prevent crash during operation we unselect location in the main dialog
  reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(true);

  OmUiProgress* uiProgress = reinterpret_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));
  uiProgress->open(true);
  uiProgress->setCaption(L"Location Backup transfer");
  uiProgress->setTitle(L"Transfer backup data...");

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
  OmUiPropLocStg* uiPropLocStg  = reinterpret_cast<OmUiPropLocStg*>(self->childById(IDD_PROP_LOC_STG));

  // retrieve new backup data folder
  wstring bck_dest;
  uiPropLocStg->getItemText(IDC_EC_INPT4, bck_dest);

  // launch move process only if directory not empty
  if(!Om_isDirEmpty(location->backupDir())) {

    OmUiProgress* uiProgress = static_cast<OmUiProgress*>(self->childById(IDD_PROGRESS));

    HWND hPb = (HWND)uiProgress->getProgressBar();
    HWND hSc = (HWND)uiProgress->getStaticComment();

    location->moveBackups(bck_dest, uiProgress->hwnd(), hPb, hSc, uiProgress->getAbortPtr());
  }

  // modify the backup path for the Location
  if(uiPropLocStg->msgItem(IDC_BC_CHK02, BM_GETCHECK)) { // custom backup checked
    location->setCustBackupDir(bck_dest);
  } else {
    location->remCustBackupDir();
  }

  // send message to window, to proper quit dialog and finish
  PostMessage(self->_hwnd, UWM_MOVEBACKUP_DONE, 0, 0);

  return 0;
}
