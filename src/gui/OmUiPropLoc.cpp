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
  OmLocation* pLoc = static_cast<OmLocation*>(this->_location);
  OmUiPropLocStg* pUiPropLocStg  = static_cast<OmUiPropLocStg*>(this->childById(IDD_PROP_LOC_STG));
  OmUiPropLocBck* pUiPropLocBck  = static_cast<OmUiPropLocBck*>(this->childById(IDD_PROP_LOC_BCK));

  bool changed = false;

  wstring item_str;

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title
    pUiPropLocStg->getItemText(IDC_EC_INPT1, item_str);
    if(pLoc->title() != item_str) changed = true;
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Location Destination path
    pUiPropLocStg->getItemText(IDC_EC_INPT2, item_str);
    if(pLoc->installDir() != item_str) changed = true;
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Location Library path
    if(pUiPropLocStg->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
      pUiPropLocStg->getItemText(IDC_EC_INPT3, item_str);
      if(pLoc->libraryDir() != item_str || !pLoc->hasCustLibraryDir())
        changed = true;
    } else {
      if(pLoc->hasCustLibraryDir()) changed = true;
    }
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Location Backup path
    if(pUiPropLocStg->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
      pUiPropLocStg->getItemText(IDC_EC_INPT4, item_str);
      if(pLoc->backupDir() != item_str || !pLoc->hasCustBackupDir())
        changed = true;
    } else {
      if(pLoc->hasCustBackupDir()) changed = true;
    }
  }

  if(pUiPropLocBck->hasChParam(LOC_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level
    if(pUiPropLocBck->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
      int cb_sel = pUiPropLocBck->msgItem(IDC_CB_LEVEL, CB_GETCURSEL);
      if(pLoc->backupZipLevel() != cb_sel) changed = true;
    } else {
      if(pLoc->backupZipLevel() != -1) changed = true;
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
  OmLocation* pLoc = static_cast<OmLocation*>(this->_location);
  OmUiPropLocBck* pUiPropLocBck  = static_cast<OmUiPropLocBck*>(this->childById(IDD_PROP_LOC_BCK));
  OmUiPropLocStg* pUiPropLocStg  = static_cast<OmUiPropLocStg*>(this->childById(IDD_PROP_LOC_STG));

  wstring loc_name, loc_dst, loc_lib, loc_bck;

  bool cust_lib, cust_bck;

  // Step 1, verify everything
  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title
    pUiPropLocStg->getItemText(IDC_EC_INPT1, loc_name);
    if(!Om_isValidName(loc_name)) {
      wstring wrn = L"Title";
      wrn += OMM_STR_ERR_VALIDNAME;
      Om_dialogBoxWarn(this->_hwnd, L"Invalid Location title", OMM_STR_ERR_VALIDNAME);
      return false;
    }
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Location Destination path
    pUiPropLocStg->getItemText(IDC_EC_INPT2, loc_dst);
    if(!Om_isDir(loc_dst)) {
      wstring wrn = L"The folder \""+loc_dst+L"\"";
      wrn += OMM_STR_ERR_ISDIR;
      Om_dialogBoxWarn(this->_hwnd, L"Invalid install destination folder", wrn);
      return false;
    }
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Location Library path
    cust_lib = pUiPropLocStg->msgItem(IDC_BC_CHK01, BM_GETCHECK);
    if(cust_lib) { //< Custom Library folder Check-Box checked
      pUiPropLocStg->getItemText(IDC_EC_INPT3, loc_lib);
      if(!Om_isDir(loc_lib)) {
        wstring wrn = L"The folder \""+loc_lib+L"\"";
        wrn += OMM_STR_ERR_ISDIR;
        Om_dialogBoxWarn(this->_hwnd, L"Invalid custom library folder", wrn);
        return false;
      }
    }
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Location Backup path
    cust_bck = pUiPropLocStg->msgItem(IDC_BC_CHK02, BM_GETCHECK);
    if(cust_bck) { //< Custom Backup folder Check-Box checked
      pUiPropLocStg->getItemText(IDC_EC_INPT4, loc_bck);
      if(!Om_isDir(loc_bck)) {
        wstring wrn = L"The folder \""+loc_bck+L"\"";
        wrn += OMM_STR_ERR_ISDIR;
        Om_dialogBoxWarn(this->_hwnd, L"Invalid custom backup folder", wrn);
        return false;
      }
    }
  }

  // Step 2, save changes
  if(pUiPropLocBck->hasChParam(LOC_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level
    if(pUiPropLocBck->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
      int cb_sel = pUiPropLocBck->msgItem(IDC_CB_LEVEL, CB_GETCURSEL);
      if(cb_sel >= 0) pLoc->setBackupZipLevel(cb_sel);
    } else {
      // disable zipped backups
      pLoc->setBackupZipLevel(-1);
    }

    // Reset parameter as unmodified
    pUiPropLocBck->setChParam(LOC_PROP_BCK_COMP_LEVEL, false);
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Location Install path
    pLoc->setInstallDir(loc_dst);

    // Reset parameter as unmodified
    pUiPropLocStg->setChParam(LOC_PROP_STG_INSTALL, false);
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Location Library path
    if(cust_lib) {
      pLoc->setCustLibraryDir(loc_lib);
    } else {
      pLoc->remCustLibraryDir();
    }
    // Reset parameter as unmodified
    pUiPropLocStg->setChParam(LOC_PROP_STG_LIBRARY, false);
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Location Backup path

    // check whether we need to transfer backup data, if yes we
    // launch the process via a new thread with progress dialog window. The
    // Location backup setting will be properly modified withing the
    // dedicates thread
    if(pLoc->backupDir() != loc_bck) {

      std::wcout << pLoc->backupDir() << L" != " << loc_bck << L"\n";
      this->_moveBackup_init();

      // if backup transfer thread is running, we do not quit since it will
      // end the process before it ends. We will wait for the UWM_TRANSFER_ENDED
      // message sent from the thread, then quit this dialog safely at this
      // moment
      return false;

    } else {
      // uncheck the unnecessary "custom" flag
      if(!cust_bck && pLoc->hasCustBackupDir())
        pLoc->remCustBackupDir();
    }

    // Reset parameter as unmodified
    pUiPropLocStg->setChParam(LOC_PROP_STG_BACKUP, false);
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title

    pLoc->setTitle(loc_name);

    wstring qry = L"Location title changed, do you also want to rename "
                  L"folder and definition file according the new title ?";

    if(Om_dialogBoxQuerry(this->_hwnd, L"Change Location title", qry)) {

      // To prevent crash during operation we unselect location in the main dialog
      static_cast<OmUiMain*>(this->root())->setSafeEdit(true);

      if(!pLoc->renameHome(loc_name)) {
        Om_dialogBoxErr(this->_hwnd, L"Location rename failed", pLoc->lastError());
      }

      // Back to main dialog window to normal state
      static_cast<OmUiMain*>(this->root())->setSafeEdit(false);
    }

    // Reset parameter as unmodified
    pUiPropLocStg->setChParam(LOC_PROP_STG_TITLE, false);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  // refresh all tree from the main dialog
  this->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_moveBackup_init()
{
  // To prevent crash during operation we unselect location in the main dialog
  static_cast<OmUiMain*>(this->root())->setSafeEdit(true);

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));
  pUiProgress->open(true);
  pUiProgress->setTitle(L"Change backup folder");
  pUiProgress->setDesc(L"Transferring backup data");

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
  static_cast<OmUiMain*>(this->root())->setSafeEdit(false);

  // Close progress dialog
  static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS))->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropLoc::_moveBackup_fth(void* arg)
{
  OmUiPropLoc* self = static_cast<OmUiPropLoc*>(arg);
  OmLocation* pLoc = static_cast<OmLocation*>(self->_location);
  OmUiPropLocStg* pUiPropLocStg  = static_cast<OmUiPropLocStg*>(self->childById(IDD_PROP_LOC_STG));

  // retrieve new backup data folder
  wstring bck_dest;
  pUiPropLocStg->getItemText(IDC_EC_INPT4, bck_dest);

  // launch move process only if directory not empty
  if(!Om_isDirEmpty(pLoc->backupDir())) {

    OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(self->childById(IDD_PROGRESS));

    HWND hPb = pUiProgress->getPbHandle();
    HWND hSc = pUiProgress->getDetailScHandle();

    if(!pLoc->backupsMove(bck_dest, hPb, hSc, pUiProgress->getAbortPtr())) {
      Om_dialogBoxWarn(pUiProgress->hwnd(), L"Backup data transfer error", pLoc->lastError());
    }
  }

  // modify the backup path for the Location
  if(pUiPropLocStg->msgItem(IDC_BC_CHK02, BM_GETCHECK)) { // custom backup checked
    pLoc->setCustBackupDir(bck_dest);
  } else {
    pLoc->remCustBackupDir();
  }

  // send message to window, to proper quit dialog and finish
  PostMessage(self->_hwnd, UWM_MOVEBACKUP_DONE, 0, 0);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_onPropQuit()
{
  // refresh all tree from the main dialog
  this->_parent->refresh();
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
    // refresh dialog
    this->refresh();
  }

  return false;
}
