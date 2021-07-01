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
#include "gui/OmUiMainNet.h"
#include "gui/OmUiPropRmt.h"
#include "gui/OmUiAddLoc.h"
#include "gui/OmUiAddRep.h"
#include "gui/OmUiMain.h"

/// \brief Custom "Repositories Query Done" Message
///
/// Custom "Repositories Query Done" window message to notify the dialog that the
/// running thread finished his job.
///
#define UWM_REPQUERY_DONE      (WM_APP+1)


/// \brief Custom "All Downloads Done" Message
///
/// Custom "All Downloads Done" window message to notify the dialog that all
/// running download thread finished their job.
///
#define UWM_DOWNLOADS_DONE    (WM_APP+2)

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainNet::OmUiMainNet(HINSTANCE hins) : OmDialog(hins),
  _dirMon_hth(nullptr),
  _dirMon_hev{0,0,0},
  _repQryt_hth(nullptr),
  _rmtDnl_count(0),
  _rmtDnl_abort(0),
  _thread_abort(false),
  _buildLvRep_icSize(0),
  _buildLvRmt_icSize(0)
{
  // Package info sub-dialog
  this->addChild(new OmUiPropRmt(hins));

  // set the accelerator table for the dialog
  this->setAccel(IDR_ACCEL);

  // elements for real-time directory monitoring thread
  this->_dirMon_hev[0] = CreateEvent(nullptr, true, false, nullptr); //< custom event to notify thread must exit
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainNet::~OmUiMainNet()
{
  // stop Library folder changes monitoring
  this->_dirMon_stop();

  HFONT hFt;
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_SC_TITLE, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_TXT, WM_GETFONT));
  if(hFt) DeleteObject(hFt);

  // Get the previous Image List to be destroyed (Small and Normal uses the same)
  HIMAGELIST hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_RMT, LVM_GETIMAGELIST, LVSIL_NORMAL));
  if(hImgLs) ImageList_Destroy(hImgLs);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMainNet::id() const
{
  return IDD_MAIN_NET;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // Location ComboBox
  this->enableItem(IDC_CB_LOC, !enable);

  // Repository Label ListBox & buttons
  this->enableItem(IDC_SC_LBL01, !enable);
  this->enableItem(IDC_LB_REP, !enable);

  // If enter freeze mode, unselect repository
  if(enable)
    this->msgItem(IDC_LB_REP, LB_SETCURSEL, -1);

  // Repository Buttons
  this->enableItem(IDC_BC_NEW, !enable);
  this->enableItem(IDC_BC_DEL, false);

  // then, user still can use Remote ListView
  // to watch, add or cancel downloads
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  if(enable) {
    // force to unselect current location
    this->locSel(-1);
  } else {
    // rebuild Location ComboBox, this
    // will also select the default Location
    this->_buildCbLoc();

    // rebuild Repository ListView
    this->_buildLvRep();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::locSel(int id)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::locSel " << id << "\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // main window dialog
  OmUiMain* pUiMain = static_cast<OmUiMain*>(this->_parent);

  // stop Library folder monitoring
  this->_dirMon_stop();

  // disable "Edit > Package []" in main menu
  pUiMain->setPopupItem(1, 5, MF_GRAYED);

  // select the requested Location
  if(pCtx) {

    pCtx->locSel(id);

    OmLocation* pLoc = pCtx->locCur();

    if(pLoc) {

      // Check Location Destination folder access
      if(!pLoc->checkAccessDst()) {

        wstring wrn = L"Configured Location's destination folder \"";
        wrn += pLoc->dstDir()+L"\""; wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";

        Om_dialogBoxWarn(this->_hwnd, L"Destination folder access error", wrn);
      }

      // Check Location Backup folder access
      if(!pLoc->checkAccessBck()) {

        wstring wrn = L"Configured Location's backup folder \"";
        wrn += pLoc->bckDir()+L"\""; wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";

        Om_dialogBoxWarn(this->_hwnd, L"Backup folder access error", wrn);
      }

      // Check Location Library folder access
      if(pLoc->checkAccessLib()) {

        // start Library folder monitoring
        this->_dirMon_init(pLoc->libDir());

      } else {
        wstring wrn = L"Configured Location's library folder \"";
        wrn += pLoc->bckDir()+L"\""; wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";

        Om_dialogBoxWarn(this->_hwnd, L"Library folder access error", wrn);
      }

      // enable the "Edit > Location properties..." menu
      pUiMain->setPopupItem(1, 2, MF_ENABLED);

    } else {

      // disable the "Edit > Location properties..." menu
      pUiMain->setPopupItem(1, 2, MF_GRAYED);
    }
  }

  // refresh
  this->_buildLvRep();
  this->_buildLvRmt();

  // forces control to select item
  this->msgItem(IDC_CB_LOC, CB_SETCURSEL, id);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::rmtDown(bool upgrade)
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT))
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  // string for dialog messages
  wstring msg;

  // checks whether we have a valid Library folder
  if(!pLoc->checkAccessLib()) {
    msg = L"Library folder \""+pLoc->libDir()+L"\"";
    msg += OMM_STR_ERR_DIRACCESS;
    Om_dialogBoxErr(this->_hwnd, L"Package(s) install aborted", msg);
    return;
  }

  // reset global abort status
  this->_thread_abort = false;

  // get user selection
  vector<OmRemote*> user_ls;

  OmRemote* pRmt;

  int lv_sel = this->msgItem(IDC_LV_RMT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    pRmt = pLoc->rmtGet(lv_sel);

    if(pRmt->isState(RMT_STATE_NEW))
      user_ls.push_back(pRmt);

    // next selected item
    lv_sel = this->msgItem(IDC_LV_RMT, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  vector<OmRemote*> dwnl_ls;  //< final download list
  vector<OmRemote*> deps_ls;  //< extra download list
  vector<OmPackage*> olds_ls; //< superseded required packages
  vector<wstring> miss_ls;    //< missing dependencies lists

  // prepare package download
  pLoc->rmtPrepareDown(dwnl_ls, deps_ls, miss_ls, olds_ls, user_ls);

  // warn user for missing dependencies
  if(miss_ls.size() && pLoc->warnMissDnld()) {
    msg = L"One or more selected packages have missing dependencies, "
          L"The following packages are required but not available:\n";
    for(size_t k = 0; k < miss_ls.size(); ++k) msg+=L"\n  "+miss_ls[k];
    msg +=  L"\n\nDo you want to proceed download anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Dependencies missing", msg))
      return;
  }

  // warn user for superseded packages required as dependency
  if(upgrade && olds_ls.size()) {
    msg = L"One or more selected packages will supersedes old versions "
          L"required as dependency by other, upgrading the following "
          L"packages will break some dependencies:\n";
    for(size_t k = 0; k < olds_ls.size(); ++k) msg+=L"\n  "+olds_ls[k]->ident();
    msg +=  L"\n\nDo you want to proceed upgrade anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Upgrade breaks dependencies", msg))
      return;
  }

  // warn for additional installation
  if(deps_ls.size() && pLoc->warnExtraDnld()) {
    msg = L"One or more selected packages have dependencies, "
          L"the following packages will also be downloaded:\n";
    for(size_t i = 0; i < deps_ls.size(); ++i) msg += L"\n "+deps_ls[i]->ident();
    msg +=  L"\n\nDo you want to continue ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Packages dependencies", msg))
      return;
  }

  // freeze dialog so user cannot interact
  static_cast<OmUiMain*>(this->root())->freeze(true);

  // necessary to update icon in ListView
  LVFINDINFOW lvFind = {};
  lvFind.flags = LVFI_PARAM;

  LVITEMW lvItem = {};
  lvItem.iSubItem = 0;
  lvItem.mask = LVIF_IMAGE;
  lvItem.iImage = 11; //< STS_DNL

  for(size_t i = 0; i < dwnl_ls.size(); ++i) {

    pRmt = dwnl_ls[i];

    if(pRmt->download(pLoc->libDir(), upgrade, &this->_rmtDnl_download_cb, this)) {

      // update state image
      lvFind.lParam = static_cast<LPARAM>(pRmt->hash()); //< Remote package hash
      lvItem.iItem = this->msgItem(IDC_LV_RMT, LVM_FINDITEMW, -1, reinterpret_cast<LPARAM>(&lvFind));
      this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

      // disable download and upgrade buttons
      this->enableItem(IDC_BC_LOAD, false);
      this->enableItem(IDC_BC_UPGD, false);

      // increment download count
      this->_rmtDnl_count++;

    } else {

      msg = L"The package \"" + pRmt->ident() + L"\" ";
      msg += L"cannot be downloaded:\n\n" + pRmt->lastError();
      Om_dialogBoxErr(this->_hwnd, L"Package download failed", msg);

    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::rmtFixd(bool upgrade)
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT) != 1)
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  // string for dialog messages
  wstring msg;

  // checks whether we have a valid Library folder
  if(!pLoc->checkAccessLib()) {
    msg = L"Library folder \""+pLoc->libDir()+L"\"";
    msg += OMM_STR_ERR_DIRACCESS;
    Om_dialogBoxErr(this->_hwnd, L"Package(s) install aborted", msg);
    return;
  }

  // reset global abort status
  this->_thread_abort = false;

  OmRemote* pRmt;

  // get selection, should be single, since UI does not allow this
  // process for multiple selection
  int lv_sel = this->msgItem(IDC_LV_RMT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  pRmt = pLoc->rmtGet(lv_sel);

  // this should never happen be we handle it
  if(pRmt->isState(RMT_STATE_NEW)) {
    return;
  }

  // the user single  selection
  vector<OmRemote*> user_ls;
  user_ls.push_back(pRmt);

  // here we go like a full download
  vector<OmRemote*> deps_ls;  //< dependency download list
  vector<wstring> miss_ls;    //< missing dependencies lists

  // Get remote package depdencies
  pLoc->rmtGetDepends(deps_ls, miss_ls, pRmt);

  // warn user for missing dependencies
  if(miss_ls.size() && pLoc->warnMissDnld()) {
    msg = L"The selected package have unavailable missing dependencies, "
          L"The following packages are required but not available:\n";
    for(size_t k = 0; k < miss_ls.size(); ++k) msg+=L"\n  "+miss_ls[k];
    msg +=  L"\n\nDo you want to proceed download anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Dependencies missing", msg))
      return;
  }

  // ask user for download X packages
  if(deps_ls.size() && pLoc->warnExtraDnld()) {
    msg =   L"The selected package have "+to_wstring(deps_ls.size())+L" missing ";
    msg +=  L"dependencies, the following packages will be downloaded:\n";
    for(size_t i = 0; i < deps_ls.size(); ++i) msg += L"\n "+deps_ls[i]->ident();
    msg +=  L"\n\nDownload missing dependencies ?";

    if(!Om_dialogBoxQuerry(this->_hwnd, L"Fix dependencies", msg))
      return;
  }

  // freeze dialog so user cannot interact
  static_cast<OmUiMain*>(this->root())->freeze(true);

  // necessary to update icon in ListView
  LVFINDINFOW lvFind = {};
  lvFind.flags = LVFI_PARAM;

  LVITEMW lvItem = {};
  lvItem.iSubItem = 0;
  lvItem.mask = LVIF_IMAGE;
  lvItem.iImage = 11; //< STS_DNL

  for(size_t i = 0; i < deps_ls.size(); ++i) {

    pRmt = deps_ls[i];

    if(pRmt->download(pLoc->libDir(), upgrade, &this->_rmtDnl_download_cb, this)) {

      // update state image
      lvFind.lParam = static_cast<LPARAM>(pRmt->hash()); //< Remote package hash
      lvItem.iItem = this->msgItem(IDC_LV_RMT, LVM_FINDITEMW, -1, reinterpret_cast<LPARAM>(&lvFind));
      this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

      // disable download and upgrade buttons
      this->enableItem(IDC_BC_LOAD, false);
      this->enableItem(IDC_BC_UPGD, false);

      // increment download count
      this->_rmtDnl_count++;

    } else {

      msg = L"The package \"" + pRmt->ident() + L"\" ";
      msg += L"cannot be downloaded:\n\n" + pRmt->lastError();
      Om_dialogBoxErr(this->_hwnd, L"Package download failed", msg);

    }
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::rmtProp()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_RMT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx->locCur()) return;

  OmRemote* pRmt = pCtx->locCur()->rmtGet(lv_sel);

  if(pRmt) {
    OmUiPropRmt* pUiPropRmt = static_cast<OmUiPropRmt*>(this->childById(IDD_PROP_RMT));
    pUiPropRmt->rmtSet(pRmt);
    pUiPropRmt->open(true);
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_dirMon_init(const wstring& path)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_dirMon_init\n";
  #endif

  // first stops any running monitor
  if(this->_dirMon_hth) {
    this->_dirMon_stop();
  }

  // create a new folder change notification event
  DWORD mask = FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME;
  mask |= FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE;

  this->_dirMon_hev[1] = FindFirstChangeNotificationW(path.c_str(), false, mask);

  // launch new thread to handle notifications
  DWORD dwId;
  this->_dirMon_hth = CreateThread(nullptr, 0, this->_dirMon_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_dirMon_stop()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_dirMon_stop\n";
  #endif

  // stops current directory monitoring thread
  if(this->_dirMon_hth) {

    // set custom event to request thread quit, then wait for it
    SetEvent(this->_dirMon_hev[0]);
    WaitForSingleObject(this->_dirMon_hth, INFINITE);
    CloseHandle(this->_dirMon_hth);

    // reset the "stop" event for further usage
    ResetEvent(this->_dirMon_hev[0]);

    // close previous folder monitor
    FindCloseChangeNotification(this->_dirMon_hev[1]);
    this->_dirMon_hev[1] = nullptr;
    this->_dirMon_hth = nullptr;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMainNet::_dirMon_fth(void* arg)
{
  OmUiMainNet* self = static_cast<OmUiMainNet*>(arg);

  DWORD dwObj;

  while(true) {

    dwObj = WaitForMultipleObjects(2, self->_dirMon_hev, false, INFINITE);

    if(dwObj == 0) //< custom "stop" event
      break;

    if(dwObj == 1) { //< folder content changed event

      OmManager* pMgr = static_cast<OmManager*>(self->_data);
      OmContext* pCtx = pMgr->ctxCur();

      if(pCtx) { //< this should be always the case
        if(pCtx->locCur()) { //< this should also be always the case
          // refresh Location Library
          if(pCtx->locCur()->rmtRefresh()) {
            // if list changed, rebuilt package ListView
            self->_buildLvRmt();
          }
        }
      }

      FindNextChangeNotification(self->_dirMon_hev[1]);
    }
  }
  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_repQry_init()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_repQry_init\n";
  #endif

  // Freezes the main dialog to prevent user to interact during process
  static_cast<OmUiMain*>(this->root())->freeze(true);


  DWORD dwId;
  this->_repQryt_hth = CreateThread(nullptr, 0, this->_repQry_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_repQry_stop()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_repQry_stop\n";
  #endif

  DWORD exitCode;

  // safely and cleanly close threads handles
  if(this->_repQryt_hth) {
    WaitForSingleObject(this->_repQryt_hth, INFINITE);
    GetExitCodeThread(this->_repQryt_hth, &exitCode);
    CloseHandle(this->_repQryt_hth);
    this->_repQryt_hth = nullptr;
  }

  // refresh remote package list
  this->_buildLvRmt();

  // unfreezes dialog so user can interact again
  static_cast<OmUiMain*>(this->root())->freeze(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMainNet::_repQry_fth(void* ptr)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_repQry_fth\n";
  #endif

  OmUiMainNet* self = reinterpret_cast<OmUiMainNet*>(ptr);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx)return 1;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc)return 1;

  DWORD exitCode = 0;

  // reset abort status
  self->_thread_abort = false;

  // change button image from refresh to stop
  self->setBmImage(IDC_BC_QRY, Om_getResImage(self->_hins, IDB_BTN_NOT));

  OmRepository* pRep;
  LVITEMW lvItem;
  for(size_t i = 0; i < pLoc->repCount(); ++i) {

    // the first column, repository status, here we INSERT the new item
    lvItem.iItem = i;
    lvItem.mask = LVIF_IMAGE;
    lvItem.iSubItem = 0;
    lvItem.iImage = 0; //< WIP
    self->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    if(!pLoc->repQuery(i)) {
      exitCode = 1;
      lvItem.iImage = 1; //< ERR
      self->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
    } else {
      lvItem.iImage = 3; //< BOK
      self->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

      pRep = pLoc->repGet(i);

      // Third column, the repository title
      lvItem.mask = LVIF_TEXT;
      lvItem.iSubItem = 2;
      lvItem.pszText = const_cast<LPWSTR>(pRep->title().c_str());
      self->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
    }
  }

  // change button image from stop to refresh
  self->setBmImage(IDC_BC_QRY, Om_getResImage(self->_hins, IDB_BTN_REF));

  // reset progress bar position
  self->msgItem(IDC_PB_REP, PBM_SETPOS, 0);

  // send message to notify process ended
  self->postMessage(UWM_REPQUERY_DONE);

  return exitCode;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_rmtDnl_stop()
{
  // all download finished, unfreezes dialog so user can interact again
  static_cast<OmUiMain*>(this->root())->freeze(false);

  // ensure controls are at right state
  this->enableItem(IDC_BC_ABORT, false);
  this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0);
  this->enableItem(IDC_PB_PKG, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMainNet::_rmtDnl_update(double tot, double cur, double rate, uint64_t hash)
{
  wchar_t item_str[OMM_ITM_BUFF];

  // compute download progression percent
  unsigned percent = (tot > 0.0) ? static_cast<int>((cur / tot) * 100.0) : 0;

  // retrieve ListView entry corresponding to current object
  LVFINDINFOW lvFind = {};
  lvFind.flags = LVFI_PARAM;
  lvFind.lParam = static_cast<LPARAM>(hash); //< Remote package hash

  int lv_id = this->msgItem(IDC_LV_RMT, LVM_FINDITEMW, -1, reinterpret_cast<LPARAM>(&lvFind));
  // wait and retry until we found the package in ListView
  while(lv_id == -1) {
    Sleep(20); //< 20 Ms
    lv_id = this->msgItem(IDC_LV_RMT, LVM_FINDITEMW, -1, reinterpret_cast<LPARAM>(&lvFind));
  }

  // structure for ListView update
  LVITEMW lvItem;
  lvItem.iItem = lv_id;

  // update download percent
  lvItem.mask = LVIF_TEXT;
  lvItem.iSubItem = 4; //< this is the right most column, "Download"
  // compute and format remaining time according download rate
  wchar_t time_str[64];
  StrFromTimeIntervalW(time_str, 64, static_cast<unsigned>(((tot-cur)/rate))*1000, 3);
  // create download progression string and assign to ListView item
  swprintf(item_str, OMM_ITM_BUFF, L"%i %% - %ls", percent, time_str);
  lvItem.pszText =  const_cast<LPWSTR>(item_str);

  // send to ListView
  this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

   // We update main dialog the progress bar according item selection
  if(this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT) == 1) {
    // We update main dialog the progress bar according item selection
    if(this->msgItem(IDC_LV_RMT, LVM_GETITEMSTATE, lv_id, LVIS_SELECTED)) {
      this->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
      // to avoid progress bar slow transition and force it to jump the
      // right position, we step it too far then backward to the right
      // position. This is the only known workaround...
      this->msgItem(IDC_PB_PKG, PBM_SETPOS, percent + 1);
      this->msgItem(IDC_PB_PKG, PBM_SETPOS, percent);
    }
  }

  // check for specific or general abort signal
  if(this->_rmtDnl_abort == hash || this->_thread_abort) {
    // reset abort to ensure this will not conflict with repo query thread
    this->_thread_abort = false;
    // we ensure abort is reset
    this->_rmtDnl_abort = 0;
    // we enable the button again
    this->enableItem(IDC_BC_ABORT, true);

    return false; //< abort now

  } else {

    return true; //< continue
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_rmtDnl_finish(uint64_t hash)
{
  // retrieve ListView entry corresponding to current object
  LVFINDINFOW lvFind = {};
  lvFind.flags = LVFI_PARAM;
  lvFind.lParam = static_cast<LPARAM>(hash); //< Remote package hash

  int lv_id = this->msgItem(IDC_LV_RMT, LVM_FINDITEMW, -1, reinterpret_cast<LPARAM>(&lvFind));
  // wait and retry until we found the package in ListView
  while(lv_id == -1) {
    Sleep(20); //< 20 Ms
    lv_id = this->msgItem(IDC_LV_RMT, LVM_FINDITEMW, -1, reinterpret_cast<LPARAM>(&lvFind));
  }

  // structure for ListView update
  LVITEMW lvItem;
  lvItem.iItem = lv_id;

  // update download percent
  lvItem.mask = LVIF_TEXT;
  lvItem.iSubItem = 4; //< this is the right most column, "Download"
  lvItem.pszText = 0; // download finished, we erase text

  // send to ListView
  this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

   // We update main dialog the progress bar according item selection
  if(this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT) == 1) {
    // We update main dialog the progress bar according item selection
    if(this->msgItem(IDC_LV_RMT, LVM_GETITEMSTATE, lv_id, LVIS_SELECTED)) {
      this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0);
      this->enableItem(IDC_PB_PKG, false);
    }
  }

  // retrieve Remote object
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx)return; //< Houston we have a problem
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc)return; //< Houston we have a problem

  // retrieve Remote object
  OmRemote* pRmt = pLoc->rmtFind(hash);
  if(!pRmt)return; //< Houston we have a problem

  // update status icon
  lvItem.mask = LVIF_IMAGE;
  lvItem.iSubItem = 0; //< this is the left most column, "Status"
  lvItem.iImage = -1; //< No Icon
  if(pRmt->isState(RMT_STATE_NEW)) {
    if(pRmt->isState(RMT_STATE_UPG)) lvItem.iImage = 10; //< STS_UPG
    if(pRmt->isState(RMT_STATE_OLD)) lvItem.iImage =  9; //< STS_OLD
  } else if(pRmt->isState(RMT_STATE_ERR)) {
    lvItem.iImage = 5; //< STS_ERR
  } else {
    lvItem.iImage = pRmt->isState(RMT_STATE_DEP) ? 6/*STS_WRN*/:7/*STS_BOK*/;
  }

  // send to ListView
  this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

  // decrement download count
  this->_rmtDnl_count--;

  if(this->_rmtDnl_count == 0) {
    // send message to notify all download finished
    this->postMessage(UWM_DOWNLOADS_DONE);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMainNet::_rmtDnl_download_cb(void* ptr, double tot, double cur, double rate, uint64_t data)
{
  OmUiMainNet* self = reinterpret_cast<OmUiMainNet*>(ptr);

  // special invalid rate value mean download finished
  if(rate < 0.0) {
    self->_rmtDnl_finish(data);
  } else {
    return self->_rmtDnl_update(tot, cur, rate, data);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_buildCbLoc()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_buildCbLoc\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // check whether any context is selected
  if(!pCtx) {
    // empty the Combo-Box
    this->msgItem(IDC_CB_LOC, CB_RESETCONTENT);
    // disable Location ComboBox
    this->enableItem(IDC_CB_LOC, false);
    // force to reset current selection
    this->locSel(-1);
    // return now
    return;
  }

  // save current selection
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);

  // empty the Combo-Box
  this->msgItem(IDC_CB_LOC, CB_RESETCONTENT);

  // add Context(s) to Combo-Box
  if(pCtx->locCount()) {

    wstring label;

    for(unsigned i = 0; i < pCtx->locCount(); ++i) {

      // compose Location label
      label = pCtx->locGet(i)->title() + L" - ";

      if(pCtx->locGet(i)->checkAccessDst()) {
        label += pCtx->locGet(i)->dstDir();
      } else {
        label += L"<folder access error>";
      }

      this->msgItem(IDC_CB_LOC, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(label.c_str()));
    }

    // select the the previously selected Context
    if(cb_sel < 0) {
      // select the first Location by default
      this->locSel(0);
    } else {
      this->msgItem(IDC_CB_LOC, CB_SETCURSEL, cb_sel);
    }

    // enable the ComboBox control
    this->enableItem(IDC_CB_LOC, true);

  } else {

    // disable Location ComboBox
    this->enableItem(IDC_CB_LOC, false);
    // force to reset current selection
    this->locSel(-1);

    // ask user to create at least one Location in the Context
    wstring qry = L"The Context have not any configured "
                  L"Location, this does not make much sense."
                  L"\n\nDo you want to add a Location now ?";

    if(Om_dialogBoxQuerry(this->_hwnd, L"Context empty", qry)) {
      OmUiAddLoc* pUiAddLoc = static_cast<OmUiAddLoc*>(this->siblingById(IDD_ADD_LOC));
      pUiAddLoc->ctxSet(pCtx);
      pUiAddLoc->open(true);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_buildLvRep()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_buildLvRep\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  if(!pLoc) {
    // empty ListView
    this->msgItem(IDC_LV_BAT, LVM_DELETEALLITEMS);
    // disable ListView
    this->enableItem(IDC_LV_BAT, false);
    // disable query button
    this->enableItem(IDC_BC_QRY, false);
    // return now
    return;
  }

  // if icon size changed, create new ImageList
  if(this->_buildLvRep_icSize != pMgr->iconsSize()) {

    HIMAGELIST hImgLs;

    // Get the previous Image List to be destroyed (Small and Normal uses the same)
    hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_REP, LVM_GETIMAGELIST, LVSIL_NORMAL));
    if(hImgLs) ImageList_Destroy(hImgLs);

    // - 0: STS_WIP - 1: STS_ERR -  2: STS_WRN -  3: STS_BOK

    // Build list of images resource ID for the required size
    unsigned idb[] = {IDB_STS_WIP_16, IDB_STS_ERR_16, IDB_STS_WRN_16, IDB_STS_BOK_16};

    switch(pMgr->iconsSize())
    {
    case 24:
      for(unsigned i = 0; i < 4; ++i)
        idb[i] += 1; //< steps IDs to 24 pixels images
      break;
    case 32:
      for(unsigned i = 0; i < 4; ++i)
        idb[i] += 2; //< steps IDs to 32 pixels images
      break;
    }

    // Create ImageList and fill it with bitmaps
    hImgLs = ImageList_Create(pMgr->iconsSize(), pMgr->iconsSize(), ILC_COLOR32, 4, 0 );
    for(unsigned i = 0; i < 4; ++i)
      ImageList_Add(hImgLs, Om_getResImage(this->_hins, idb[i]), nullptr);

    // Set ImageList to ListView
    this->msgItem(IDC_LV_REP, LVM_SETIMAGELIST, LVSIL_SMALL, reinterpret_cast<LPARAM>(hImgLs));
    this->msgItem(IDC_LV_REP, LVM_SETIMAGELIST, LVSIL_NORMAL, reinterpret_cast<LPARAM>(hImgLs));

    // update size
    this->_buildLvRep_icSize = pMgr->iconsSize();
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_REP, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty list view
  this->msgItem(IDC_LV_REP, LVM_DELETEALLITEMS);

  wstring item_str;
  OmRepository* pRep;
  LVITEMW lvItem;
  for(unsigned i = 0; i < pLoc->repCount(); ++i) {

    pRep = pLoc->repGet(i);

    // the first column, repository status, here we INSERT the new item
    lvItem.iItem = i;
    lvItem.mask = LVIF_IMAGE;
    lvItem.iSubItem = 0;
    lvItem.iImage = pRep->isValid() ? 3 /*BOK*/ : -1/*No image*/;
    lvItem.iItem = this->msgItem(IDC_LV_REP, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Second column, the repository address, now empty, here we set the sub-item
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 1;
    item_str = pRep->base() + L" - " + pRep->name();
    lvItem.pszText = const_cast<LPWSTR>(item_str.c_str());
    this->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Third column, the repository title
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 2;
    lvItem.pszText = const_cast<LPWSTR>(pRep->title().c_str());
    this->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
  }

  // we enable the List-View
  this->enableItem(IDC_LV_REP, true);

  // restore list-view scroll position from lvRec
  this->msgItem(IDC_LV_REP, LVM_SCROLL, 0, -lvRec.top );

  // enable or disable query button
  this->enableItem(IDC_BC_QRY, (pLoc->repCount() > 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_buildLvRmt()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_buildLvRmt\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  if(!pLoc) {
    // empty ListView
    this->msgItem(IDC_LV_RMT, LVM_DELETEALLITEMS);
    // disable ListView
    this->enableItem(IDC_LV_RMT, false);
    // return now
    return;
  }

  // if icon size changed, create new ImageList
  if(this->_buildLvRmt_icSize != pMgr->iconsSize()) {

    HIMAGELIST hImgLs;

    // Get the previous Image List to be destroyed (Small and Normal uses the same)
    hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_RMT, LVM_GETIMAGELIST, LVSIL_NORMAL));
    if(hImgLs) ImageList_Destroy(hImgLs);

    // - 0: PKG_ERR - 1: PKG_DIR -  2: PKG_ZIP -  3: PKG_DPN
    // - 4: STS_WIP - 5: STS_ERR -  6: STS_WRN -  7: STS_BOK
    // - 8: STS_OWR - 9: STS_OLD - 10: STS_UPG - 11: STS_DNL

    // Build list of images resource ID for the required size
    unsigned idb[] = {IDB_PKG_ERR_16, IDB_PKG_DIR_16, IDB_PKG_ZIP_16, IDB_PKG_DPN_16,
                      IDB_STS_WIP_16, IDB_STS_ERR_16, IDB_STS_WRN_16, IDB_STS_BOK_16,
                      IDB_STS_OWR_16, IDB_STS_OLD_16, IDB_STS_UPG_16, IDB_STS_DNL_16};

    switch(pMgr->iconsSize())
    {
    case 24:
      for(unsigned i = 0; i < 12; ++i)
        idb[i] += 1; //< steps IDs to 24 pixels images
      break;
    case 32:
      for(unsigned i = 0; i < 12; ++i)
        idb[i] += 2; //< steps IDs to 32 pixels images
      break;
    }

    // Create ImageList and fill it with bitmaps
    hImgLs = ImageList_Create(pMgr->iconsSize(), pMgr->iconsSize(), ILC_COLOR32, 12, 0);
    for(unsigned i = 0; i < 12; ++i)
      ImageList_Add(hImgLs, Om_getResImage(this->_hins, idb[i]), nullptr);

    // Set ImageList to ListView
    this->msgItem(IDC_LV_RMT, LVM_SETIMAGELIST, LVSIL_SMALL, reinterpret_cast<LPARAM>(hImgLs));
    this->msgItem(IDC_LV_RMT, LVM_SETIMAGELIST, LVSIL_NORMAL, reinterpret_cast<LPARAM>(hImgLs));

    // update size
    this->_buildLvRmt_icSize = pMgr->iconsSize();
  }

  // return now if library folder cannot be accessed
  if(!pLoc->checkAccessLib()) {
    return;
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_RMT, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty list view
  this->msgItem(IDC_LV_RMT, LVM_DELETEALLITEMS);

  // add item to list view
  OmRemote* pRmt;
  LVITEMW lvItem;
  for(unsigned i = 0; i < pLoc->rmtCount(); ++i) {

    pRmt = pLoc->rmtGet(i);

    // the first column, package status, here we INSERT the new item
    lvItem.iItem = i;
    lvItem.mask = LVIF_IMAGE|LVIF_PARAM; //< icon and special data
    lvItem.iSubItem = 0;
    lvItem.iImage = -1; //< No Icon
    if(pRmt->isState(RMT_STATE_NEW)) {
      if(pRmt->isState(RMT_STATE_DNL)) {
        lvItem.iImage = 11; //< STS_DNL
      } else {
        if(pRmt->isState(RMT_STATE_UPG)) lvItem.iImage = 10; //< STS_UPG
        if(pRmt->isState(RMT_STATE_OLD)) lvItem.iImage =  9; //< STS_OLD
      }
    } else if(pRmt->isState(RMT_STATE_ERR)) {
      lvItem.iImage = 5; //< STS_ERR
    } else {
      lvItem.iImage = pRmt->isState(RMT_STATE_DEP) ? 6/*STS_WRN*/:7/*STS_BOK*/;
    }

    // notice for later : to work properly the lParam must be
    // defined on the first SubItem (iSubItem = 0)
    lvItem.lParam = static_cast<LPARAM>(pRmt->hash());

    lvItem.iItem = this->msgItem(IDC_LV_RMT, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Second column, the package name and type, here we set the sub-item
    lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
    lvItem.iSubItem = 1;
    lvItem.iImage = pRmt->depCount() ? 3/*PKG_DPN*/ : 2/*PKG_ZIP*/;

    lvItem.pszText = const_cast<LPWSTR>(pRmt->name().c_str());
    this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Third column, the package version, we set the sub-item
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 2;
    lvItem.pszText = const_cast<LPWSTR>(pRmt->version().asString().c_str());
    this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Fourth column, the package size, we set the sub-item
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 3;
    lvItem.pszText = const_cast<LPWSTR>(Om_formatSizeSysStr(pRmt->bytes(), true).c_str());
    this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
  }

  // we enable the List-View
  this->enableItem(IDC_LV_RMT, true);

  // restore list-view scroll position from lvRec
  this->msgItem(IDC_LV_RMT, LVM_SCROLL, 0, -lvRec.top );

  // update Package ListView selection
  this->_onLvRmtSel();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onCbLocSel()
{
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  this->locSel(cb_sel);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onLvRepSel()
{
  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_BAT, LVM_GETSELECTEDCOUNT);

  this->enableItem(IDC_BC_DEL, (lv_nsl >= 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onLvRmtHit()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_RMT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  // get remote package object
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  OmRemote* pRmt = pLoc->rmtGet(lv_sel);

  // check whether remote package is an upgrade
  if(pRmt->isState(RMT_STATE_UPG)) {

    // ask user for upgrade
    wstring msg = L"The selected remote package supersedes one or more "
                  L"local packages.\n\n"
                  L"Do you want to keep older package versions ?";

    if(Om_dialogBoxQuerry(this->_hwnd, L"Package upgrade", msg)) {
      // simply download the package
      this->rmtDown(false);
    } else {
      // upgrade (delete superseded packages)
      this->rmtDown(true);
    }

  } else {
    // simply download the package
    this->rmtDown(false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onLvRmtRclk()
{
  // get handle to "Edit > Remote..." sub-menu
  HMENU hMenu = static_cast<OmUiMain*>(this->_parent)->getPopupItem(1, 6);

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  TrackPopupMenu(hMenu, TPM_TOPALIGN|TPM_LEFTALIGN, pt.x, pt.y, 0, this->_hwnd, nullptr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onLvRmtSel()
{
  // hide all package bottom infos
  this->showItem(IDC_SB_PKG, false);
  this->showItem(IDC_EC_TXT, false);
  this->showItem(IDC_SC_TITLE, false);

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  // keep handle to main dialog
  OmUiMain* pUiMain = static_cast<OmUiMain*>(this->_parent);

  // disable "Edit > Remote" in main menu
  pUiMain->setPopupItem(1, 6, MF_GRAYED);

  // Handle to bitmap for package picture
  HBITMAP hBm = Om_getResImage(this->_hins, IDB_PKG_THN);

  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT);

  if(lv_nsl > 0) {

    // enable "Edit > Remote []" pop-up menu
    pUiMain->setPopupItem(1, 6, MF_ENABLED);

    // show package title and thumbnail
    this->showItem(IDC_SC_TITLE, true);
    this->showItem(IDC_SB_PKG, true);

    if(lv_nsl > 1) {

      // disable the "Edit > Remote > View detail..." menu-item
      HMENU hPopup = pUiMain->getPopupItem(1, 6);
      pUiMain->setPopupItem(hPopup, 3, MF_GRAYED); //< "Fix dependencies" menu-item
      pUiMain->setPopupItem(hPopup, 5, MF_GRAYED); //< "View detail..." menu-item

      // on multiple selection, we hide package description
      this->showItem(IDC_EC_TXT, false);
      this->setItemText(IDC_SC_TITLE, L"<Multiple selection>");

    } else {

      // enable the "Edit > Remote > .. " menu-item
      HMENU hPopup = pUiMain->getPopupItem(1, 6);
      pUiMain->setPopupItem(hPopup, 5, MF_ENABLED); //< "View details" menu-item

      // show package description
      this->showItem(IDC_EC_TXT, true);

      OmRemote* pRmt;

      // get the selected item id (only one, no need to iterate)
      int lv_sel = this->msgItem(IDC_LV_RMT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
      if(lv_sel >= 0) {

        pRmt = pLoc->rmtGet(lv_sel);

        // get remote package states
        bool can_dnld = pRmt->isState(RMT_STATE_NEW) && !pRmt->isState(RMT_STATE_DNL);
        bool can_upgd = can_dnld && pRmt->isState(RMT_STATE_UPG);
        bool can_fixd = pRmt->isState(RMT_STATE_DEP);
        bool progress = pRmt->isState(RMT_STATE_DNL);

        pUiMain->setPopupItem(hPopup, 0, can_dnld ? MF_ENABLED : MF_GRAYED); //< "Dwonload" menu-item
        pUiMain->setPopupItem(hPopup, 1, can_upgd ? MF_ENABLED : MF_GRAYED); //< "Upgrade" menu-item
        pUiMain->setPopupItem(hPopup, 3, can_fixd ? MF_ENABLED : MF_GRAYED); //< "Fix dependencies" menu-item

        this->enableItem(IDC_BC_LOAD, can_dnld);
        this->enableItem(IDC_BC_UPGD, can_upgd);
        this->enableItem(IDC_BC_ABORT, progress);
        this->enableItem(IDC_PB_PKG, progress);

        if(progress) {
          this->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
          // we first go beyond value then backward to workaround the
          // unwanted transition
          this->msgItem(IDC_PB_PKG, PBM_SETPOS, pRmt->downPercent() + 1);
          this->msgItem(IDC_PB_PKG, PBM_SETPOS, pRmt->downPercent());
        } else {
          this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0);
        }

        this->setItemText(IDC_SC_TITLE, pRmt->name() + L" " + pRmt->version().asString());

        if(pRmt->desc().size()) {
          this->setItemText(IDC_EC_TXT, pRmt->desc());
        } else {
          this->setItemText(IDC_EC_TXT, L"<no description available>");
        }

        if(pRmt->image().thumbnail()) {
          hBm = pRmt->image().thumbnail();
        }

      }
    }
  } else {

    // disable "Edit > Remote []" pop-up menu
    pUiMain->setPopupItem(1, 6, MF_GRAYED);

    // disable all action buttons
    this->enableItem(IDC_BC_LOAD, false);
    this->enableItem(IDC_BC_UPGD, false);
    this->enableItem(IDC_BC_ABORT, false);
  }

  // Update the selected picture
  hBm = this->setStImage(IDC_SB_PKG, hBm);
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);

  // force thumbnail static control to update its position
  this->_setItemPos(IDC_SB_PKG, 5, this->height()-83, 86, 79);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onBcChkRep()
{
  // action depend on current thread state, if thread is
  // running, the button act as an abort button
  if(this->_repQryt_hth) {
    this->enableItem(IDC_BC_QRY, false);
    this->_thread_abort = true;
  } else {
    this->_repQry_init();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onBcStopRep()
{
  this->_thread_abort = true;
  this->enableItem(IDC_BC_STOP, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onBcNewRep()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  if(pCtx->locCur()) {
    OmUiAddRep* pUiNewRep = static_cast<OmUiAddRep*>(this->siblingById(IDD_ADD_REP));
    pUiNewRep->locSet(pCtx->locCur());
    pUiNewRep->open(true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onBcDelRep()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  int lb_sel = this->msgItem(IDC_LB_REP, LB_GETCURSEL);
  if(lb_sel < 0) return;

  OmRepository* pRep = pLoc->repGet(lb_sel);

  // warns the user before committing the irreparable
  wstring qry = L"Are your sure you want to delete the Repository \"";
  qry += pRep->base()+L" - "+pRep->name()+L"\" ?";

  if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Repository", qry))
    return;

  pLoc->repRem(lb_sel);

  // reload the repository ListBox
  this->_buildLvRep();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onBcAbort()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_RMT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  // disable abort the button
  this->enableItem(IDC_BC_ABORT, false);

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  // we set abort value to Remote object hash to identify it
  // within the download callback function
  this->_rmtDnl_abort = pLoc->rmtGet(lv_sel)->hash();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_onInit\n";
  #endif

  // Defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(21, 400, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(14, 700, L"Consolas");
  this->msgItem(IDC_EC_TXT, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  // Set batches New and Delete buttons icons
  this->setBmImage(IDC_BC_STOP, Om_getResImage(this->_hins, IDB_BTN_NOT));
  this->setBmImage(IDC_BC_NEW, Om_getResImage(this->_hins, IDB_BTN_ADD));
  this->setBmImage(IDC_BC_DEL, Om_getResImage(this->_hins, IDB_BTN_REM));
  this->setBmImage(IDC_BC_QRY, Om_getResImage(this->_hins, IDB_BTN_REF));

  // define controls tool-tips
  this->_createTooltip(IDC_CB_LOC,    L"Select active Location");
  this->_createTooltip(IDC_LB_REP,    L"Repositories list");
  this->_createTooltip(IDC_BC_QRY,    L"Start or stop repositories query");
  this->_createTooltip(IDC_BC_NEW,    L"Configure and add new repository");
  this->_createTooltip(IDC_BC_DEL,    L"Remove selected repository entry");
  this->_createTooltip(IDC_LV_RMT,    L"Remote packages list");
  this->_createTooltip(IDC_BC_LOAD,   L"Download selected packages");
  this->_createTooltip(IDC_BC_UPGD,   L"Download selected packages for upgrade");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort download");

  DWORD lvStyle = LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_DOUBLEBUFFER;
  LVCOLUMNW lvCol;

  // Initialize Repository ListView control
  this->msgItem(IDC_LV_REP, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lvStyle);
  // set explorer theme
  SetWindowTheme(this->getItem(IDC_LV_REP),L"Explorer",nullptr);

  // we now add columns into our list-view control
  lvCol.mask = LVCF_WIDTH|LVCF_FMT;
  //  "The alignment of the leftmost column is always LVCFMT_LEFT; it
  // cannot be changed." says Mr Microsoft. Do not ask why, the Microsoft's
  // mysterious ways... So, don't try to fix this.
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.cx = 43;
  lvCol.iSubItem = 0;
  this->msgItem(IDC_LV_REP, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 300;
  lvCol.iSubItem = 1;
  this->msgItem(IDC_LV_REP, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 300;
  lvCol.iSubItem = 2;
  this->msgItem(IDC_LV_REP, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&lvCol));

  // Initialize Remote Packages ListView control
  this->msgItem(IDC_LV_RMT, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lvStyle);
  // set explorer theme
  SetWindowTheme(this->getItem(IDC_LV_RMT),L"Explorer",nullptr);

  // we now add columns into our list-view control
  lvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
  lvCol.pszText = const_cast<LPWSTR>(L"Status");
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.cx = 43;
  lvCol.iSubItem = 0;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Name");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 300;
  lvCol.iSubItem = 1;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Version");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem = 2;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Size");
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.cx = 80;
  lvCol.iSubItem = 3;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, 3, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Download");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 120;
  lvCol.iSubItem = 4;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, 4, reinterpret_cast<LPARAM>(&lvCol));

  // hide package details
  this->showItem(IDC_SC_TITLE, false);
  this->showItem(IDC_EC_TXT, false);
  this->showItem(IDC_SB_PKG, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_onShow\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  // if current location selection mismatch with current
  // ComboBox selection we select the proper location in
  // ComboBox and update ListView
  int cur_id = pCtx->locCurId();
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  if(cur_id != cb_sel) {
    this->msgItem(IDC_CB_LOC, CB_SETCURSEL, cur_id);
  }

  if(pCtx->locCur()) {

    // rebuild elements
    this->_buildLvRep();

    // restart folder monitoring
    if(!this->_dirMon_hth) {
      this->_dirMon_init(pCtx->locCur()->libDir());
    }

    // refresh remote package ListView
    pCtx->locCur()->rmtRefresh(true);
    this->_buildLvRmt();
  }

  // disable "Edit > Package" in main menu
  static_cast<OmUiMain*>(this->root())->setPopupItem(1, 5, MF_GRAYED);

  // refresh dialog
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onHide()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_onHide\n";
  #endif

  // stop folder monitoring
  if(this->_dirMon_hth)
    this->_dirMon_stop();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onResize()
{
  LONG size[4], half_s;

  // Locations Combo-Box
  this->_setItemPos(IDC_CB_LOC, 5, 5, this->width()-10, 12);
  // Repositories label
  this->_setItemPos(IDC_SC_LBL01, 5, 24, 180, 12);
  // Repositories ProgressBar
  this->_setItemPos(IDC_BC_QRY, this->width()-20, 20, 16, 14);
  //this->_setItemPos(IDC_PB_REP, 89, 21, this->width()-129, 13);
  //this->_setItemPos(IDC_BC_STOP, this->width()-37, 20, 32, 14);
  // Repositories ListBox
  this->_setItemPos(IDC_LV_REP, 5, 37, this->width()-30, 29);
  GetClientRect(this->getItem(IDC_LV_REP), reinterpret_cast<LPRECT>(&size));
  half_s = static_cast<float>(size[2]) * 0.5f;
  this->msgItem(IDC_LV_REP, LVM_SETCOLUMNWIDTH, 1, half_s-40);
  this->msgItem(IDC_LV_REP, LVM_SETCOLUMNWIDTH, 2, half_s-40);
  // Repositories Apply, New.. and Delete buttons
  this->_setItemPos(IDC_BC_NEW, this->width()-20, 36, 16, 14);
  this->_setItemPos(IDC_BC_DEL, this->width()-20, 52, 16, 14);

  // Horizontal separator
  this->_setItemPos(IDC_SC_SEPAR, 5, 70, this->width()-10, 1);

  // Package List ListView
  this->_setItemPos(IDC_LV_RMT, 5, 75, this->width()-10, this->height()-191);
  // Resize the ListView column
  GetClientRect(this->getItem(IDC_LV_RMT), reinterpret_cast<LPRECT>(&size));
  this->msgItem(IDC_LV_RMT, LVM_SETCOLUMNWIDTH, 1, size[2]-345);

  // Upgrade and Sync buttons
  this->_setItemPos(IDC_BC_LOAD, 5, this->height()-114, 50, 14);
  this->_setItemPos(IDC_BC_UPGD, 56, this->height()-114, 50, 14);
  // Progress bar
  this->_setItemPos(IDC_PB_PKG, 108, this->height()-113, this->width()-166, 12);
  // Abort button
  this->_setItemPos(IDC_BC_ABORT, this->width()-55, this->height()-114, 50, 14);

  // Package name/title
  this->_setItemPos(IDC_SC_TITLE, 5, this->height()-97, this->width()-161, 12);
  // Package snapshot
  this->_setItemPos(IDC_SB_PKG, 5, this->height()-85, 86, 79);
  // Package description
  this->_setItemPos(IDC_EC_TXT, 95, this->height()-85, this->width()-101, 78);

  InvalidateRect(this->_hwnd, nullptr, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_onRefresh\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // disable all packages buttons
  this->enableItem(IDC_BC_LOAD, false);
  this->enableItem(IDC_BC_UPGD, false);
  this->enableItem(IDC_BC_ABORT, false);

  // hide package details
  this->showItem(IDC_SC_TITLE, false);
  this->showItem(IDC_EC_TXT, false);
  this->showItem(IDC_SB_PKG, false);

  // disable the Progress-Bar
  this->enableItem(IDC_PB_PKG, false);

  // reload Location ComboBox
  this->_buildCbLoc();

  // reload Repository ListBox
  this->_buildLvRep();

  // if icon size changed, rebuild Package ListView
  if(this->_buildLvRmt_icSize != pMgr->iconsSize()) {
    this->_buildLvRmt();
  }

  // disable or enable elements depending context
  this->enableItem(IDC_SC_LBL01, (pCtx != nullptr));
  this->enableItem(IDC_LV_RMT, (pCtx != nullptr));
  this->enableItem(IDC_LB_REP, (pCtx != nullptr));

  // disable all batches buttons
  this->enableItem(IDC_BC_NEW, (pCtx != nullptr));
  this->enableItem(IDC_BC_DEL, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainNet::_onQuit\n";
  #endif

  // stop Library folder changes monitoring
  this->_dirMon_stop();

  // this should be done already...
  this->_thread_abort = true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMainNet::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_REPQUERY_DONE is a custom message sent from Repositories query
  // thread function, to notify the thread ended is job.
  if(uMsg == UWM_REPQUERY_DONE) {
    // properly stop the running thread and finish process
    this->_repQry_stop();
    return false;
  }

  // UWM_DOWNLOADS_DONE is a custom message sent from any download callback
  // thread function, to notify all download thread ended their job.
  if(uMsg == UWM_DOWNLOADS_DONE) {
    // properly unfreeze the dialog and perform cleaning job
    this->_rmtDnl_stop();
    return false;
  }

  // UWM_MAIN_CTX_CHANGED is a custom message sent from Main (parent) Dialog
  // to notify its child tab dialogs the Context selection changed.
  if(uMsg == UWM_MAIN_CTX_CHANGED) {
    // invalidate Location selection
    this->msgItem(IDC_CB_LOC, CB_SETCURSEL, -1);
    // Refresh the dialog
    this->_onRefresh();
    return false;
  }

  // UWM_MAIN_ABORT_REQUEST is a custom message sent from Main (parent) Dialog
  // to notify its child tab dialogs they must abort all running threaded jobs
  if(uMsg == UWM_MAIN_ABORT_REQUEST) {
    this->_thread_abort = true;
    this->enableItem(IDC_BC_STOP, false);
    this->enableItem(IDC_BC_ABORT, false);
    return false;
  }

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return false;

  if(uMsg == WM_NOTIFY) {

    OmLocation* pLoc = pCtx->locCur();
    if(!pLoc) return false;

    // if repositories query is running we block all interaction
    if(this->_repQryt_hth)
      return false;

    if(LOWORD(wParam) == IDC_LV_RMT) {

      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case NM_DBLCLK:
        this->_onLvRmtHit();
        break;

      case NM_RCLICK:
        this->_onLvRmtRclk();
        break;

      case LVN_ITEMCHANGED:
        this->_onLvRmtSel();
        break;

      case LVN_COLUMNCLICK:
        switch(reinterpret_cast<NMLISTVIEW*>(lParam)->iSubItem)
        {
        case 0:
          pLoc->rmtSetSorting(LS_SORT_STAT);
          break;
        case 2:
          pLoc->rmtSetSorting(LS_SORT_VERS);
          break;
        case 3:
          pLoc->rmtSetSorting(LS_SORT_SIZE);
          break;
        default:
          pLoc->rmtSetSorting(LS_SORT_NAME);
          break;
        }
        this->_onLvRmtRclk();
        break;
      }
    }

    if(LOWORD(wParam) == IDC_LV_REP) {

      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case NM_DBLCLK:
        //this->_onLvRmtHit();
        break;

      case LVN_ITEMCHANGED:
        this->_onLvRepSel();
        break;
      }
    }

    return false;
  }

  if(uMsg == WM_COMMAND) {

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->ctxCur();

    if(!pCtx->locCur())
      return false;

    switch(LOWORD(wParam))
    {

    case IDC_CB_LOC: //< Location ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE) this->_onCbLocSel();
      break;

    case IDC_BC_QRY: //< Repository "Refresh" button
      this->_onBcChkRep();
      break;

    case IDC_BC_STOP:
      this->_onBcStopRep();
      break;

    case IDC_BC_NEW: //< Repository "Add" button
      this->_onBcNewRep();
      break;

    case IDC_BC_DEL: //< Repository "Delete" button
      this->_onBcDelRep();
      break;

    case IDC_BC_LOAD: //< Main "Download" button
      this->rmtDown(false);
      break;

    case IDC_BC_UPGD: //< Main "Upgrade" button
      this->rmtDown(true);
      break;

    case IDC_BC_ABORT: //< Main "Abort" button
      this->_onBcAbort();
      break;

    // Menu : Edit > Remote > []
    case IDM_EDIT_RMT_DOWN:
      this->rmtDown(false);
      break;

    case IDM_EDIT_RMT_UPGR:
      this->rmtDown(true);
      break;

    case IDM_EDIT_RMT_FIXD:
      this->rmtFixd(false);
      break;

    case IDM_EDIT_RMT_INFO:
      this->rmtProp();
      break;

    }
  }

  return false;
}
