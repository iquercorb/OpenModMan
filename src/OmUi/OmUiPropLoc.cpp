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

#include "OmManager.h"

#include "OmUiMgr.h"
#include "OmUiAddRep.h"
#include "OmUiPropLocStg.h"
#include "OmUiPropLocLib.h"
#include "OmUiPropLocBck.h"
#include "OmUiPropLocNet.h"
#include "OmUiProgress.h"

#include "OmUtilFs.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropLoc.h"


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
  _pLoc(nullptr),
  _movBck_hth(nullptr),
  _movBck_dest()
{
  // create tab dialogs
  this->_addPage(L"Settings", new OmUiPropLocStg(hins));
  this->_addPage(L"Library", new OmUiPropLocLib(hins));
  this->_addPage(L"Backup", new OmUiPropLocBck(hins));
  this->_addPage(L"Repositories", new OmUiPropLocNet(hins));

  // creates child sub-dialogs
  this->addChild(new OmUiAddRep(hins));     //< Dialog for new Repository
  this->addChild(new OmUiProgress(hins));   //< for Location backup transfer or deletion
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
  OmUiPropLocStg* pUiPropLocStg  = static_cast<OmUiPropLocStg*>(this->childById(IDD_PROP_LOC_STG));
  OmUiPropLocLib* pUiPropLocLib  = static_cast<OmUiPropLocLib*>(this->childById(IDD_PROP_LOC_LIB));
  OmUiPropLocBck* pUiPropLocBck  = static_cast<OmUiPropLocBck*>(this->childById(IDD_PROP_LOC_BCK));
  OmUiPropLocNet* pUiPropLocNet  = static_cast<OmUiPropLocNet*>(this->childById(IDD_PROP_LOC_NET));

  bool changed = false;

  wstring item_str;

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title
    pUiPropLocStg->getItemText(IDC_EC_INP01, item_str);
    if(this->_pLoc->title() != item_str) changed = true;
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Target path
    pUiPropLocStg->getItemText(IDC_EC_INP02, item_str);
    if(this->_pLoc->dstDir() != item_str) changed = true;
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Library path
    if(pUiPropLocStg->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
      pUiPropLocStg->getItemText(IDC_EC_INP03, item_str);
      if(this->_pLoc->libDir() != item_str || !this->_pLoc->hasCustLibDir())
        changed = true;
    } else {
      if(this->_pLoc->hasCustLibDir()) changed = true;
    }
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Backup path
    if(pUiPropLocStg->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
      pUiPropLocStg->getItemText(IDC_EC_INP04, item_str);
      if(this->_pLoc->bckDir() != item_str || !this->_pLoc->hasCustBckDir())
        changed = true;
    } else {
      if(this->_pLoc->hasCustBckDir()) changed = true;
    }
  }

  if(pUiPropLocBck->hasChParam(LOC_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level
    if(pUiPropLocBck->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
      int cb_sel = pUiPropLocBck->msgItem(IDC_CB_LVL, CB_GETCURSEL);
      if(this->_pLoc->bckZipLevel() != cb_sel) changed = true;
    } else {
      if(this->_pLoc->bckZipLevel() != -1) changed = true;
    }
  }

  if(pUiPropLocLib->hasChParam(LOC_PROP_LIB_DEVMODE)) {
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != this->_pLoc->libDevMode())
      changed = true;
  }

  if(pUiPropLocLib->hasChParam(LOC_PROP_LIB_WARNINGS)) {
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != this->_pLoc->warnOverlaps())
      changed = true;
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX3, BM_GETCHECK) != this->_pLoc->warnExtraInst())
      changed = true;
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX4, BM_GETCHECK) != this->_pLoc->warnMissDeps())
      changed = true;
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX5, BM_GETCHECK) != this->_pLoc->warnExtraUnin())
      changed = true;
  }

  if(pUiPropLocLib->hasChParam(LOC_PROP_LIB_SHOWHIDDEN)) {
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX6, BM_GETCHECK) != this->_pLoc->libShowHidden())
      changed = true;
  }

  if(pUiPropLocNet->hasChParam(LOC_PROP_NET_WARNINGS)) {
    if(pUiPropLocNet->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != this->_pLoc->warnExtraDnld())
      changed = true;
    if(pUiPropLocNet->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != this->_pLoc->warnMissDnld())
      changed = true;
    if(pUiPropLocNet->msgItem(IDC_BC_CKBX3, BM_GETCHECK) != this->_pLoc->warnUpgdBrkDeps())
      changed = true;
  }

  if(pUiPropLocNet->hasChParam(LOC_PROP_NET_ONUPGRADE)) {
    if(pUiPropLocNet->msgItem(IDC_BC_RAD02, BM_GETCHECK) != this->_pLoc->upgdRename())
      changed = true;
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
  OmUiPropLocBck* pUiPropLocBck  = static_cast<OmUiPropLocBck*>(this->childById(IDD_PROP_LOC_BCK));
  OmUiPropLocLib* pUiPropLocLib  = static_cast<OmUiPropLocLib*>(this->childById(IDD_PROP_LOC_LIB));
  OmUiPropLocStg* pUiPropLocStg  = static_cast<OmUiPropLocStg*>(this->childById(IDD_PROP_LOC_STG));
  OmUiPropLocNet* pUiPropLocNet  = static_cast<OmUiPropLocNet*>(this->childById(IDD_PROP_LOC_NET));

  wstring loc_name, loc_dst, loc_lib, loc_bck;

  bool cust_lib = false;
  bool cust_bck = false;

  // Step 1, verify everything
  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Channel name
    pUiPropLocStg->getItemText(IDC_EC_INP01, loc_name);
    if(!Om_dlgValidName(this->_hwnd, L"Channel name", loc_name))
      return false;
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Target path
    pUiPropLocStg->getItemText(IDC_EC_INP02, loc_dst);
    if(!Om_dlgValidDir(this->_hwnd, L"Target path", loc_dst))
      return false;
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Library path
    cust_lib = pUiPropLocStg->msgItem(IDC_BC_CKBX1, BM_GETCHECK);
    if(cust_lib) { //< Custom Library folder Check-Box checked
      pUiPropLocStg->getItemText(IDC_EC_INP03, loc_lib);
      if(Om_dlgValidPath(this->_hwnd, L"Library folder path", loc_lib)) {
        if(!Om_dlgCreateFolder(this->_hwnd, L"Custom Library folder", loc_lib))
          return false;
      } else {
        return false;
      }
    }
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Backup path
    cust_bck = pUiPropLocStg->msgItem(IDC_BC_CKBX2, BM_GETCHECK);
    if(cust_bck) { //< Custom Backup folder Check-Box checked
      pUiPropLocStg->getItemText(IDC_EC_INP04, loc_bck);
      if(Om_dlgValidPath(this->_hwnd, L"Backup folder path", loc_lib)) {
        if(!Om_dlgCreateFolder(this->_hwnd, L"Custom Backup folder", loc_lib))
          return false;
      } else {
        return false;
      }
    }
  }

  // Step 2, save changes
  if(pUiPropLocLib->hasChParam(LOC_PROP_LIB_DEVMODE)) {

    this->_pLoc->setLibDevMode(pUiPropLocLib->msgItem(IDC_BC_CKBX1, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocLib->setChParam(LOC_PROP_LIB_DEVMODE, false);
  }

  if(pUiPropLocLib->hasChParam(LOC_PROP_LIB_WARNINGS)) {

    this->_pLoc->setWarnOverlaps(pUiPropLocLib->msgItem(IDC_BC_CKBX2, BM_GETCHECK));
    this->_pLoc->setWarnExtraInst(pUiPropLocLib->msgItem(IDC_BC_CKBX3, BM_GETCHECK));
    this->_pLoc->setWarnMissDeps(pUiPropLocLib->msgItem(IDC_BC_CKBX4, BM_GETCHECK));
    this->_pLoc->setWarnExtraUnin(pUiPropLocLib->msgItem(IDC_BC_CKBX5, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocLib->setChParam(LOC_PROP_LIB_WARNINGS, false);
  }

  if(pUiPropLocLib->hasChParam(LOC_PROP_LIB_SHOWHIDDEN)) {

    this->_pLoc->setLibShowHidden(pUiPropLocLib->msgItem(IDC_BC_CKBX6, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocLib->setChParam(LOC_PROP_LIB_SHOWHIDDEN, false);
  }

  if(pUiPropLocNet->hasChParam(LOC_PROP_NET_WARNINGS)) {

    this->_pLoc->setWarnExtraDnld(pUiPropLocNet->msgItem(IDC_BC_CKBX1, BM_GETCHECK));
    this->_pLoc->setWarnMissDnld(pUiPropLocNet->msgItem(IDC_BC_CKBX2, BM_GETCHECK));
    this->_pLoc->setWarnUpgdBrkDeps(pUiPropLocNet->msgItem(IDC_BC_CKBX3, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocNet->setChParam(LOC_PROP_NET_WARNINGS, false);
  }


  if(pUiPropLocNet->hasChParam(LOC_PROP_NET_ONUPGRADE)) {

    this->_pLoc->setUpgdRename(pUiPropLocNet->msgItem(IDC_BC_RAD02, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocNet->setChParam(LOC_PROP_NET_ONUPGRADE, false);
  }

  if(pUiPropLocBck->hasChParam(LOC_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level
    if(pUiPropLocBck->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
      int cb_sel = pUiPropLocBck->msgItem(IDC_CB_LVL, CB_GETCURSEL);
      if(cb_sel >= 0) this->_pLoc->setBckZipLevel(cb_sel);
    } else {
      // disable zipped backups
      this->_pLoc->setBckZipLevel(-1);
    }

    // Reset parameter as unmodified
    pUiPropLocBck->setChParam(LOC_PROP_BCK_COMP_LEVEL, false);
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_INSTALL)) { //< parameter for Location Install path
    this->_pLoc->setDstDir(loc_dst);

    // Reset parameter as unmodified
    pUiPropLocStg->setChParam(LOC_PROP_STG_INSTALL, false);
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_LIBRARY)) { //< parameter for Location Library path
    if(cust_lib) {
      this->_pLoc->setCustLibDir(loc_lib);
    } else {
      this->_pLoc->remCustLibDir();
    }
    // Reset parameter as unmodified
    pUiPropLocStg->setChParam(LOC_PROP_STG_LIBRARY, false);
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_BACKUP)) { //< parameter for Location Backup path

    // check whether we need to transfer backup data, if yes we
    // launch the process via a new thread with progress dialog window. The
    // Location backup setting will be properly modified withing the
    // dedicates thread
    if(this->_pLoc->bckDir() != loc_bck) {

      // start move backup thread
      this->_movBck_init(loc_bck);

      // if backup transfer thread is running, we do not quit since it will
      // end the process before it ends. We will wait for the UWM_MOVEBACKUP_DONE
      // message sent from the thread, then quit this dialog safely at this
      // moment
      return false;

    } else {
      // uncheck the unnecessary "custom" flag
      if(!cust_bck && this->_pLoc->hasCustBckDir())
        this->_pLoc->remCustBckDir();

      // Reset parameter as unmodified
      pUiPropLocStg->setChParam(LOC_PROP_STG_BACKUP, false);
    }
  }

  if(pUiPropLocStg->hasChParam(LOC_PROP_STG_TITLE)) { //< parameter for Location title

    this->_pLoc->setTitle(loc_name);

    // To prevent crash during operation we unselect location in the main dialog
    static_cast<OmUiMgr*>(this->root())->safemode(true);

    if(!this->_pLoc->renameHome(loc_name)) {
      Om_dlgBox_okl(this->_hwnd, L"Channel properties", IDI_WRN,
                   L"Channel files rename error", L"Channel "
                   "title changed but folder and definition file rename "
                   "failed because of the following error:", this->_pLoc->lastError());
    }

    // Back to main dialog window to normal state
    static_cast<OmUiMgr*>(this->root())->safemode(false);

    // Reset parameter as unmodified
    pUiPropLocStg->setChParam(LOC_PROP_STG_TITLE, false);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  // refresh all dialogs from root (Main dialog)
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_movBck_init(const wstring& dest)
{
  if(!this->_pLoc) return;

  // verify we have something to move
  if(!Om_isDirEmpty(this->_pLoc->bckDir())) {
    this->_movBck_stop();
  }

  // keep destination path
  this->_movBck_dest = dest;

  // To prevent crash during operation we unselect location in the main dialog
  static_cast<OmUiMgr*>(this->root())->safemode(true);

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));
  pUiProgress->open(true);
  pUiProgress->setCaption(L"Change backup folder");
  pUiProgress->setScHeadText(L"Transferring backup data");

  DWORD dwid;
  this->_movBck_hth = CreateThread(nullptr, 0, this->_movBck_fth, this, 0, &dwid);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLoc::_movBck_stop()
{
  DWORD exitCode;

  if(this->_movBck_hth) {
    WaitForSingleObject(this->_movBck_hth, INFINITE);
    GetExitCodeThread(this->_movBck_hth, &exitCode);
    CloseHandle(this->_movBck_hth);
    this->_movBck_hth = nullptr;
  }

  // Back to main dialog window to normal state
  static_cast<OmUiMgr*>(this->root())->safemode(false);

  // Close progress dialog
  static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS))->quit();

  if(exitCode == 1)  {
    // an error occurred during backup purge
    Om_dlgBox_ok(this->_hwnd, L"Channel properties", IDI_WRN,
                 L"Channel backup transfer error", L"Channel "
                 "backup data transfer encountered error(s), some backup data may "
                 "had not properly moved. Please read debug log for details.");
  }

  OmUiPropLocStg* pUiPropLocStg = static_cast<OmUiPropLocStg*>(this->childById(IDD_PROP_LOC_STG));

  // Reset parameter as unmodified
  pUiPropLocStg->setChParam(LOC_PROP_STG_BACKUP, false);

  // modify the backup path for the Location
  if(pUiPropLocStg->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) { // custom backup checked
    this->_pLoc->setCustBckDir(this->_movBck_dest);
  } else {
    this->_pLoc->remCustBckDir();
  }

  // Call apply again in case it still changes to be applied
  this->applyChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropLoc::_movBck_fth(void* arg)
{
  OmUiPropLoc* self = static_cast<OmUiPropLoc*>(arg);

  OmLocation* pLoc = self->_pLoc;
  if(!pLoc) return 1;

  DWORD exitCode = 0;

  if(!pLoc->bckMove(self->_movBck_dest, &self->_movBck_progress_cb, self->childById(IDD_PROGRESS))) {
    exitCode = 1;
  }

  // send message to window, to proper quit dialog and finish
  PostMessage(self->_hwnd, UWM_MOVEBACKUP_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLoc::_movBck_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t data)
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
void OmUiPropLoc::_onPropInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropLoc::_onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_MOVEBACKUP_DONE is a custom message sent from backup transfer thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_MOVEBACKUP_DONE) {
    // end the backup transfer process
    this->_movBck_stop();
  }

  return false;
}
