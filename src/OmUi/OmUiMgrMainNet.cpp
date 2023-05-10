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
  #include <ShlWApi.h>      //< StrFromTimeIntervalW

#include "OmManager.h"

#include "OmUiMgr.h"
#include "OmUiMgrMain.h"
#include "OmUiMgrFoot.h"
#include "OmUiAddLoc.h"
#include "OmUiAddRep.h"
#include "OmUiPropRmt.h"

#include "OmUtilFs.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiMgrMainNet.h"

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


/// \brief Create Packages name list string.
///
/// Create Package name list string with proper CRLF from
/// the given Package object array.
///
/// \param[out] lst_str   String to receive name list.
/// \param[in]  pkgs_ls   Package object list to create name list from.
///
inline static void __msg_package_list(wstring& lst_str, const vector<OmPackage*>& pkgs_ls)
{
  size_t size = pkgs_ls.size();
  size_t stop = pkgs_ls.size() - 1;

  lst_str.clear();

  for(size_t i = 0; i < size; ++i) {
    lst_str += pkgs_ls[i]->ident();
    if(i < stop) lst_str += L"\r\n";
  }
}

/// \brief Create Remotes name list string.
///
/// Create Remotes name list string with proper CRLF from
/// the given Remotes object array.
///
/// \param[out] lst_str   String to receive name list.
/// \param[in]  rmts_ls   Remotes object list to create name list from.
///
inline static void __msg_package_list(wstring& lst_str, const vector<OmRemote*>& rmts_ls)
{
  size_t size = rmts_ls.size();
  size_t stop = rmts_ls.size() - 1;

  lst_str.clear();

  for(size_t i = 0; i < size; ++i) {
    lst_str += rmts_ls[i]->ident();
    if(i < stop) lst_str += L"\r\n";
  }
}

/// \brief Create Packages name list string.
///
/// Create Package name list string with proper CRLF from
/// the given string array.
///
/// \param[out] lst_str   String to receive name list.
/// \param[in]  ident_ls  Package object list to create name list from.
///
inline static void __msg_package_list(wstring& lst_str, const vector<wstring>& ident_ls)
{
  size_t size = ident_ls.size();
  size_t stop = ident_ls.size() - 1;

  lst_str.clear();

  for(size_t i = 0; i < size; ++i) {
    lst_str += ident_ls[i];
    if(i < stop) lst_str += L"\r\n";
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrMainNet::OmUiMgrMainNet(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr),
  _dirMon_hth(nullptr),
  _dirMon_hev{0,0,0},
  _repQry_sel(-1),
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
OmUiMgrMainNet::~OmUiMgrMainNet()
{
  // stop Library folder changes monitoring
  if(this->_dirMon_hth) this->_dirMon_stop();

  // Get the previous Image List to be destroyed (Small and Normal uses the same)
  HIMAGELIST hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_RMT, LVM_GETIMAGELIST, LVSIL_NORMAL));
  if(hImgLs) ImageList_Destroy(hImgLs);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMgrMainNet::id() const
{
  return IDD_MGR_MAIN_NET;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // Location ComboBox
  this->enableItem(IDC_CB_LOC, !enable);

  // Repository Label ListBox & buttons
  this->enableItem(IDC_SC_LBL01, !enable);
  this->enableItem(IDC_LV_REP, !enable);

  // Repository Buttons
  this->enableItem(IDC_BC_NEW, !enable);
  this->enableItem(IDC_BC_DEL, false);

  // then, user still can use Remote ListView
  // to watch, add or cancel downloads
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  if(enable) {
    this->_dirMon_stop();
  } else {
    if(this->visible())
      this->_onRefresh();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::locSel(int id)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::locSel " << id << "\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // stop Library folder monitoring
  if(this->_dirMon_hth) this->_dirMon_stop();

  // disable "Edit > Remote []" in main menu
  this->_pUiMgr->setPopupItem(MNU_EDIT, MNU_EDIT_RMT, MF_GRAYED);

  // select the requested Location
  if(pCtx) {

    pCtx->locSel(id);

    OmLocation* pLoc = pCtx->locCur();

    if(pLoc) {

      // Check Location Library folder access
      if(pLoc->libDirAccess(false)) { //< check only for reading
        // start Library folder monitoring
        this->_dirMon_init(pLoc->libDir());
      } else {
        // warning message will be already thrown by Library tab
      }

      // enable the "Edit > Location properties..." menu
      this->_pUiMgr->setPopupItem(MNU_EDIT, MNU_EDIT_LOCPROP, MF_ENABLED);

    } else {

      // disable the "Edit > Location properties..." menu
      this->_pUiMgr->setPopupItem(MNU_EDIT, MNU_EDIT_LOCPROP, MF_GRAYED);
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
void OmUiMgrMainNet::rmtDown(bool upgrade)
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT))
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->locCur();
  if(!pLoc) return;

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

  if(user_ls.empty())
    return;

  // string for dialog messages
  wstring msg_lst;

  // checks whether we have a valid Library folder
  if(!pLoc->libDirAccess(true)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, L"Download Packages", IDI_ERR,
                  L"Library folder access error", L"The Library folder "
                  "cannot be accessed because it do not exist or have read/write "
                  "access restrictions. Please check Channel's settings "
                  "and folder permissions", pLoc->libDir());
    return;
  }

  // reset global abort status
  this->_thread_abort = false;

  vector<OmRemote*> dwnl_ls;  //< final download list
  vector<OmRemote*> deps_ls;  //< extra download list
  vector<OmPackage*> olds_ls; //< superseded required packages
  vector<wstring> miss_ls;    //< missing dependencies lists

  // prepare package download
  pLoc->rmtPrepareDown(dwnl_ls, deps_ls, miss_ls, olds_ls, user_ls);

  // warn user for missing dependencies
  if(miss_ls.size() && pLoc->warnMissDnld()) {

    __msg_package_list(msg_lst, miss_ls);
    if(!Om_dlgBox_cal(this->_hwnd, L"Download Packages", IDI_PKG_WRN,
                  L"Missing Packages dependencies", L"One or more selected packages "
                  "have missing dependencies, the following packages are "
                  "required but not available:", msg_lst))
    {
      return;
    }

  }

  // warn user for superseded packages required as dependency
  if(upgrade && olds_ls.size()) {

    __msg_package_list(msg_lst, olds_ls);
    if(!Om_dlgBox_cal(this->_hwnd, L"Download Packages", IDI_PKG_WRN,
                  L"Upgrade breaks dependencies", L"One or more selected packages "
                  "will supersedes old versions required as dependency by other, "
                  "upgrading the following packages will break some dependencies:", msg_lst))
    {
      return;
    }

  }

  // warn for additional installation
  if(deps_ls.size() && pLoc->warnExtraDnld()) {

    __msg_package_list(msg_lst, deps_ls);
    if(!Om_dlgBox_cal(this->_hwnd, L"Download Packages", IDI_PKG_ADD,
                  L"Packages dependencies", L"One or more selected packages "
                  "have dependencies, the following packages will also "
                  "be downloaded:", msg_lst))
    {
      return;
    }
  }

  // freeze dialog so user cannot interact
  this->_pUiMgr->freeze(true);

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

      // disable download buttons
      this->enableItem(IDC_BC_DNLD, false);
      // enable abort button
      this->enableItem(IDC_BC_ABORT, true);

      // increment download count
      this->_rmtDnl_count++;

    } else {

      Om_dlgBox_okl(this->_hwnd, L"Download Packages", IDI_PKG_ERR,
                  L"Package download error", L"The download of Package \""
                  +pRmt->ident()+L"\" failed because of the following error:",
                  pRmt->lastError());
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::rmtFixd(bool upgrade)
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT) != 1)
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->locCur();
  if(!pLoc) return;

  // string for dialog messages
  wstring msg_lst;

  // checks whether we have a valid Library folder
  if(!pLoc->libDirAccess(true)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, L"Fix dependencies", IDI_ERR,
                  L"Library folder access error", L"The Library folder "
                  "cannot be accessed because it do not exist or have read/write "
                  "access restrictions. Please check Channel's settings "
                  "and folder permissions", pLoc->libDir());
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

    __msg_package_list(msg_lst, miss_ls);
    if(!Om_dlgBox_cal(this->_hwnd, L"Fix dependencies", IDI_PKG_WRN,
                  L"Missing Package dependencies", L"One or more selected packages "
                  "have missing dependencies, the following packages are "
                  "required but not available:", msg_lst))
    {
      return;
    }

  }

  // ask user for download X packages
  if(deps_ls.size() && pLoc->warnExtraDnld()) {

    __msg_package_list(msg_lst, deps_ls);
    if(!Om_dlgBox_cal(this->_hwnd, L"Fix dependencies", IDI_PKG_WRN,
                  L"Fix Package dependencies", L"The selected package have "
                  +to_wstring(deps_ls.size())+L" missing dependencies, "
                  "the following packages will be downloaded:", msg_lst))
    {
      return;
    }
  }

  // freeze dialog so user cannot interact
  this->_pUiMgr->freeze(true);

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

      // disable download button
      this->enableItem(IDC_BC_DNLD, false);

      // increment download count
      this->_rmtDnl_count++;

    } else {

      Om_dlgBox_okl(this->_hwnd, L"Fix dependencies", IDI_PKG_ERR,
                  L"Package download error", L"The download of Package \""
                  +pRmt->ident()+L"\" failed because of the following error:",
                  pRmt->lastError());
    }
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::rmtProp()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_RMT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->locCur();
  if(!pLoc) return;

  OmRemote* pRmt = pLoc->rmtGet(lv_sel);

  if(pRmt) {
    OmUiPropRmt* pUiPropRmt = static_cast<OmUiPropRmt*>(this->childById(IDD_PROP_RMT));
    pUiPropRmt->rmtSet(pRmt);
    pUiPropRmt->open(true);
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_dirMon_init(const wstring& path)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_dirMon_init\n";
  #endif

  // first stops any running monitor
  if(this->_dirMon_hth) this->_dirMon_stop();

  // create a new folder change notification event
  DWORD mask =  FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|
                FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE;

  this->_dirMon_hev[1] = FindFirstChangeNotificationW(path.c_str(), false, mask);

  // launch new thread to handle notifications
  DWORD dwId;
  this->_dirMon_hth = CreateThread(nullptr, 0, this->_dirMon_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_dirMon_stop()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_dirMon_stop\n";
  #endif

  // stops current directory monitoring thread
  if(this->_dirMon_hth) {

    // set custom event to request thread quit, then wait for it
    SetEvent(this->_dirMon_hev[0]);
    WaitForSingleObject(this->_dirMon_hth, INFINITE);
    CloseHandle(this->_dirMon_hth);
    this->_dirMon_hth = nullptr;

    // reset the "stop" event for further usage
    ResetEvent(this->_dirMon_hev[0]);

    // close previous folder monitor
    FindCloseChangeNotification(this->_dirMon_hev[1]);
    this->_dirMon_hev[1] = nullptr;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMgrMainNet::_dirMon_fth(void* arg)
{
  OmUiMgrMainNet* self = static_cast<OmUiMgrMainNet*>(arg);

  DWORD dwObj;

  while(true) {

    dwObj = WaitForMultipleObjects(2, self->_dirMon_hev, false, INFINITE);

    if(dwObj == 0) //< custom "stop" event
      break;

    if(dwObj == 1) { //< folder content changed event

      #ifdef DEBUG
      std::cout << "DEBUG => OmUiMgrMainNet::_dirMon_fth (changes)\n";
      #endif

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
void OmUiMgrMainNet::_repQry_init(int sel)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_repQry_init\n";
  #endif

  // Freezes the main dialog to prevent user to interact during process
  this->_pUiMgr->freeze(true);

  this->_repQry_sel = sel;

  DWORD dwId;
  this->_repQryt_hth = CreateThread(nullptr, 0, this->_repQry_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_repQry_stop()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_repQry_stop\n";
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
  this->_pUiMgr->freeze(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMgrMainNet::_repQry_fth(void* ptr)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_repQry_fth\n";
  #endif

  OmUiMgrMainNet* self = reinterpret_cast<OmUiMgrMainNet*>(ptr);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmLocation* pLoc = pMgr->locCur();
  if(!pLoc) return 1;

  DWORD exitCode = 0;

  // reset abort status
  self->_thread_abort = false;

  // change button image from refresh to stop
  self->setBmIcon(IDC_BC_QRY, Om_getResIcon(self->_hins, IDI_BT_NOT));

  OmRepository* pRep;
  LVITEMW lvItem;

  size_t i, n;

  if(self->_repQry_sel >= 0) {
    i = self->_repQry_sel;
    n = i + 1;
  } else {
    i = 0;
    n = pLoc->repCount();
  }

  for( ; i < n; ++i) {
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
  self->setBmIcon(IDC_BC_QRY, Om_getResIcon(self->_hins, IDI_BT_REF));

  // send message to notify process ended
  self->postMessage(UWM_REPQUERY_DONE);

  return exitCode;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_rmtDnl_stop()
{
  // all download finished, unfreezes dialog so user can interact again
  this->_pUiMgr->freeze(false);

  // ensure controls are at right state
  this->enableItem(IDC_BC_ABORT, false);
  this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0);
  this->enableItem(IDC_PB_PKG, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMgrMainNet::_rmtDnl_update(double tot, double cur, double rate, uint64_t hash)
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
  lvItem.iSubItem = 5; //< this is the right most column, "Download"
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
void OmUiMgrMainNet::_rmtDnl_finish(uint64_t hash)
{
  // retrieve Remote object
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->locCur();
  if(!pLoc) return;

  // retrieve Remote object
  OmRemote* pRmt = pLoc->rmtFind(hash);
  if(!pRmt)return; //< Houston we have a problem

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

  // We update main dialog progress bar according item selection
  if(this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT) == 1) {
    // We update main dialog progress bar according item selection
    if(this->msgItem(IDC_LV_RMT, LVM_GETITEMSTATE, lv_id, LVIS_SELECTED)) {
      if(pRmt->isState(RMT_STATE_WIP)) {
        this->msgItem(IDC_PB_PKG, PBM_SETPOS, 100);
      } else {
        this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0);
        this->enableItem(IDC_PB_PKG, false);
      }
    }
  }

  // update status text
  lvItem.mask = LVIF_TEXT;
  lvItem.iSubItem = 5; //< this is the right most column, "Download"

  // check whether Remote package is in WIP status
  if(pRmt->isState(RMT_STATE_WIP)) {
    lvItem.pszText = const_cast<LPWSTR>(L"Processing...");
    this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // update status icon
    lvItem.mask = LVIF_IMAGE;
    lvItem.iSubItem = 0; //< this is the left most column, "Status"
    lvItem.iImage = 4; //< STS_WIP
    this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    return; //< return now, will finish next time

  } else {
    // update status text
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 5; //< this is the right most column, "Download"
    lvItem.pszText = const_cast<LPWSTR>(L""); // download finished, we erase text
    this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
  }

  // update status icon
  lvItem.mask = LVIF_IMAGE;
  lvItem.iSubItem = 0; //< this is the left most column, "Status"
  lvItem.iImage = -1; //< No Icon

  if(pRmt->isState(RMT_STATE_ERR)) {
    lvItem.iImage = 5; //< STS_ERR
  } else if(pRmt->isState(RMT_STATE_NEW)) {
    if(pRmt->isState(RMT_STATE_UPG)) {
      lvItem.iImage = 10; //< STS_UPG
    } else if(pRmt->isState(RMT_STATE_OLD)) {
      lvItem.iImage =  9; //< STS_OLD
    } else {
      lvItem.iImage = 12; //< STS_NEW
    }
  } else {
    lvItem.iImage = pRmt->isState(RMT_STATE_DEP) ? 6/*STS_WRN*/:7/*STS_BOK*/;
  }

  // send to ListView
  this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

  // if an error occurred, display error dialog
  if(pRmt->isState(RMT_STATE_ERR)) {
      Om_dlgBox_okl(this->_hwnd, L"Download Packages", IDI_PKG_ERR,
                  L"Package download error", L"The download of Package \""
                  +pRmt->ident()+L"\" failed because of the following error:",
                  pRmt->lastError());
  }


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
bool OmUiMgrMainNet::_rmtDnl_download_cb(void* ptr, double tot, double cur, double rate, uint64_t data)
{
  OmUiMgrMainNet* self = reinterpret_cast<OmUiMgrMainNet*>(ptr);

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
void OmUiMgrMainNet::_buildCbLoc()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_buildCbLoc\n";
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

  // empty the Combo-Box
  this->msgItem(IDC_CB_LOC, CB_RESETCONTENT);

  // add Context(s) to Combo-Box
  if(pCtx->locCount()) {

    wstring label;

    for(unsigned i = 0; i < pCtx->locCount(); ++i) {

      // compose Location label
      label = pCtx->locGet(i)->title() + L" - ";

      if(pCtx->locGet(i)->dstDirAccess(true)) { //< check for read and write
        label += pCtx->locGet(i)->dstDir();
      } else {
        label += L"<folder access error>";
      }

      this->msgItem(IDC_CB_LOC, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(label.c_str()));
    }

    // set selection to current active location
    this->msgItem(IDC_CB_LOC, CB_SETCURSEL, pCtx->locCurId());

    // enable the ComboBox control
    this->enableItem(IDC_CB_LOC, true);

  } else {

    // disable Location ComboBox
    this->enableItem(IDC_CB_LOC, false);
    // no selection
    this->msgItem(IDC_CB_LOC, CB_SETCURSEL, -1);

    // ask user to create at least one Target Location in the Software Context
    if(!Om_dlgBox_yn(this->_hwnd, L"Repositories", IDI_QRY,
                  L"Empty Hub", L"The Hub is empty and have no Channel configured. "
                  "Do you want to add a Channel now ?"))
    {
      OmUiAddLoc* pUiAddLoc = static_cast<OmUiAddLoc*>(this->_pUiMgr->childById(IDD_ADD_LOC));
      pUiAddLoc->ctxSet(pCtx);
      pUiAddLoc->open(true);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_buildLvRep()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_buildLvRep\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

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

  // get current context and location
  OmLocation* pLoc = pMgr->locCur();

  if(!pLoc) {
    // disable ListView
    this->enableItem(IDC_LV_REP, false);
    // disable query button
    this->enableItem(IDC_BC_QRY, false);
    // update Repositories ListView selection
    this->_onLvRepSel();
    // return now
    return;
  }

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

  // resize ListView columns adapted to client area
  this->_rsizeLvRep();

  // update Repositories ListView selection
  this->_onLvRepSel();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_rsizeLvRep()
{
  LONG size[4], half_s;

  GetClientRect(this->getItem(IDC_LV_REP), reinterpret_cast<LPRECT>(&size));
  half_s = static_cast<float>(size[2]) * 0.5f;
  this->msgItem(IDC_LV_REP, LVM_SETCOLUMNWIDTH, 1, (half_s - 100) - 25);
  this->msgItem(IDC_LV_REP, LVM_SETCOLUMNWIDTH, 2, (half_s + 100) - 25);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_buildLvRmt()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_buildLvRmt\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // if icon size changed, create new ImageList
  if(this->_buildLvRmt_icSize != pMgr->iconsSize()) {

    HIMAGELIST hImgLs;

    // Get the previous Image List to be destroyed (Small and Normal uses the same)
    hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_RMT, LVM_GETIMAGELIST, LVSIL_NORMAL));
    if(hImgLs) ImageList_Destroy(hImgLs);

    // -  0: PKG_ERR -  1: PKG_DIR -  2: PKG_ZIP -  3: PKG_DPN
    // -  4: STS_WIP -  5: STS_ERR -  6: STS_WRN -  7: STS_BOK
    // -  8: STS_OWR -  9: STS_OLD - 10: STS_UPG - 11: STS_DNL
    // - 12: STS_NEW

    // Build list of images resource ID for the required size
    unsigned idb[] = {IDB_PKG_ERR_16, IDB_PKG_DIR_16, IDB_PKG_ZIP_16, IDB_PKG_DPN_16,
                      IDB_STS_WIP_16, IDB_STS_ERR_16, IDB_STS_WRN_16, IDB_STS_BOK_16,
                      IDB_STS_OWR_16, IDB_STS_OLD_16, IDB_STS_UPG_16, IDB_STS_DNL_16,
                      IDB_STS_NEW_16};

    unsigned idb_size = sizeof(idb) / 4;

    switch(pMgr->iconsSize())
    {
    case 24:
      for(unsigned i = 0; i < idb_size; ++i)
        idb[i] += 1; //< steps IDs to 24 pixels images
      break;
    case 32:
      for(unsigned i = 0; i < idb_size; ++i)
        idb[i] += 2; //< steps IDs to 32 pixels images
      break;
    }

    // Create ImageList and fill it with bitmaps
    hImgLs = ImageList_Create(pMgr->iconsSize(), pMgr->iconsSize(), ILC_COLOR32, idb_size, 0);
    for(unsigned i = 0; i < idb_size; ++i)
      ImageList_Add(hImgLs, Om_getResImage(this->_hins, idb[i]), nullptr);

    // Set ImageList to ListView
    this->msgItem(IDC_LV_RMT, LVM_SETIMAGELIST, LVSIL_SMALL, reinterpret_cast<LPARAM>(hImgLs));
    this->msgItem(IDC_LV_RMT, LVM_SETIMAGELIST, LVSIL_NORMAL, reinterpret_cast<LPARAM>(hImgLs));

    // update size
    this->_buildLvRmt_icSize = pMgr->iconsSize();
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_RMT, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty list view
  this->msgItem(IDC_LV_RMT, LVM_DELETEALLITEMS);

  // get current context and location
  OmLocation* pLoc = pMgr->locCur();

  if(!pLoc) {
    // disable ListView
    this->enableItem(IDC_LV_RMT, false);
    // update Package ListView selection
    this->_onLvRmtSel();
    // return now
    return;
  }

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
    if(pRmt->isState(RMT_STATE_ERR)) {
      lvItem.iImage = 5; //< STS_ERR
    } else if(pRmt->isState(RMT_STATE_DNL)) {
      lvItem.iImage = 11; //< STS_DNL
    } else if(pRmt->isState(RMT_STATE_WIP)) {
      lvItem.iImage = 4; //< STS_WIP
    } else if(pRmt->isState(RMT_STATE_NEW)) {
      if(pRmt->isState(RMT_STATE_UPG)) {
        lvItem.iImage = 10; //< STS_UPG
      } else if(pRmt->isState(RMT_STATE_OLD)) {
        lvItem.iImage =  9; //< STS_OLD
      } else {
        lvItem.iImage = 12; //< STS_NEW
      }
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

    // Fourth column, the package category, we set the sub-item
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 3;
    lvItem.pszText = const_cast<LPWSTR>(pRmt->category().c_str());
    this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Fifth column, the package size, we set the sub-item
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 4;
    lvItem.pszText = const_cast<LPWSTR>(Om_formatSizeSysStr(pRmt->bytes(), true).c_str());
    this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Sixth column, the package download progress, we set to empty
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 5;
    if(pRmt->isState(RMT_STATE_WIP)) {
      lvItem.pszText = const_cast<LPWSTR>(L"Processing...");
    } else {
      lvItem.pszText = const_cast<LPWSTR>(L""); //< download string will be updated if currently running
    }
    this->msgItem(IDC_LV_RMT, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
  }

  // we enable the ListView
  this->enableItem(IDC_LV_RMT, true);

  // restore ListView scroll position from lvRec
  this->msgItem(IDC_LV_RMT, LVM_SCROLL, 0, -lvRec.top );

  // resize ListView columns adapted to client area
  this->_rsizeLvRmt();

  // update Package ListView selection
  this->_onLvRmtSel();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_rsizeLvRmt()
{
  LONG size[4];
  // Resize the ListView column
  GetClientRect(this->getItem(IDC_LV_RMT), reinterpret_cast<LPRECT>(&size));
  this->msgItem(IDC_LV_RMT, LVM_SETCOLUMNWIDTH, 1, size[2]-405);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onCbLocSel()
{
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  this->locSel(cb_sel);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onLvRepSel()
{
  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_REP, LVM_GETSELECTEDCOUNT);

  this->enableItem(IDC_BC_DEL, (lv_nsl > 0));
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onLvRepHit()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_REP, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_REP, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  this->_repQry_init(lv_sel);
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onLvRmtHit()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_RMT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  // replacing the previous package is the new default behavior
  this->rmtDown(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onLvRmtRclk()
{
  // get handle to "Edit > Remote..." sub-menu
  HMENU hMenu = this->_pUiMgr->getPopupItem(MNU_EDIT, MNU_EDIT_RMT);

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  TrackPopupMenu(hMenu, TPM_TOPALIGN|TPM_LEFTALIGN, pt.x, pt.y, 0, this->_hwnd, nullptr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onLvRmtSel()
{
  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_RMT, LVM_GETSELECTEDCOUNT);

  // handle to "Edit > Remote >" sub-menu
  HMENU hPopup = this->_pUiMgr->getPopupItem(MNU_EDIT, MNU_EDIT_RMT);

  if(!lv_nsl) {

    // disable all action buttons
    this->enableItem(IDC_BC_DNLD, false);
    this->enableItem(IDC_BC_ABORT, false);

    // disable "Edit > Remote []" pop-up menu
    this->_pUiMgr->setPopupItem(MNU_EDIT, MNU_EDIT_RMT, MF_GRAYED);

    // disable all menu-item (for right click menu)
    for(unsigned i = 0; i < 6; ++i)
      this->_pUiMgr->setPopupItem(hPopup, i, MF_GRAYED);

    // show nothing in footer frame
    this->_pUiMgr->pUiMgrFoot()->clearItem();

    // return now
    return;
  }

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->locCur();
  if(!pLoc) return;

  // at least one selected, enable "Edit > Remote []" pop-up menu
  this->_pUiMgr->setPopupItem(MNU_EDIT, MNU_EDIT_RMT, MF_ENABLED);

  if(lv_nsl > 1) {

    // multiple selection, we allow more than one download at a time
    this->enableItem(IDC_BC_DNLD, true); //< enable anyway when multiple selection

    // enable and disable "Edit > Remote" menu-items
    this->_pUiMgr->setPopupItem(hPopup, MNU_EDIT_RMT_DNLD, MF_ENABLED); //< "Download" menu-item
    this->_pUiMgr->setPopupItem(hPopup, MNU_EDIT_RMT_DNWS, MF_ENABLED); //< "Download without supersede" menu-item
    this->_pUiMgr->setPopupItem(hPopup, MNU_EDIT_RMT_FIXD, MF_GRAYED); //< "Fix dependencies" menu-item
    this->_pUiMgr->setPopupItem(hPopup, MNU_EDIT_RMT_INFO, MF_GRAYED); //< "View detail..." menu-item

    // on multiple selection, we hide package description
    this->_pUiMgr->pUiMgrFoot()->clearItem();

  } else {

    // enable the "Edit > Remote > .. " menu-item
    this->_pUiMgr->setPopupItem(hPopup, 5, MF_ENABLED); //< "View details" menu-item

    OmRemote* pRmt = nullptr;

    // get the selected item id (only one, no need to iterate)
    int lv_sel = this->msgItem(IDC_LV_RMT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
    if(lv_sel >= 0) {

      pRmt = pLoc->rmtGet(lv_sel);

      // show packages info in footer frame
      this->_pUiMgr->pUiMgrFoot()->selectItem(pRmt);

      // get remote package states
      bool can_dnld = pRmt->isState(RMT_STATE_NEW) && !pRmt->isState(RMT_STATE_DNL);
      bool can_fixd = pRmt->isState(RMT_STATE_DEP);
      bool progress = pRmt->isState(RMT_STATE_DNL) || pRmt->isState(RMT_STATE_WIP);

      // enable and disable "Edit > Remote" menu-items
      this->_pUiMgr->setPopupItem(hPopup, MNU_EDIT_RMT_DNLD, can_dnld ? MF_ENABLED : MF_GRAYED); //< "Download" menu-item
      this->_pUiMgr->setPopupItem(hPopup, MNU_EDIT_RMT_DNWS, can_dnld ? MF_ENABLED : MF_GRAYED); //< "Download without supersede" menu-item
      this->_pUiMgr->setPopupItem(hPopup, MNU_EDIT_RMT_FIXD, can_fixd ? MF_ENABLED : MF_GRAYED); //< "Fix dependencies" menu-item
      this->_pUiMgr->setPopupItem(hPopup, MNU_EDIT_RMT_INFO, MF_ENABLED); //< "View detail..." menu-item

      this->enableItem(IDC_BC_DNLD, can_dnld);
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

    } else {

      // reset footer frame
      this->_pUiMgr->pUiMgrFoot()->clearItem();
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onBcChkRep()
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
void OmUiMgrMainNet::_onBcStopRep()
{
  this->_thread_abort = true;
  this->enableItem(IDC_BC_STOP, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onBcNewRep()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->locCur();

  if(pLoc) {
    OmUiAddRep* pUiNewRep = static_cast<OmUiAddRep*>(this->_pUiMgr->childById(IDD_ADD_REP));
    pUiNewRep->locSet(pLoc);
    pUiNewRep->open(true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onBcDelRep()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->locCur();
  if(!pLoc) return;

  int lb_sel = this->msgItem(IDC_LB_REP, LB_GETCURSEL);
  if(lb_sel < 0) return;

  OmRepository* pRep = pLoc->repGet(lb_sel);

  // warns the user before committing the irreparable
  if(!Om_dlgBox_ynl(this->_hwnd, L"Remove Repository", IDI_QRY,
                L"Remove Repository", L"Remove Repository from list ?",
                pRep->base()+L" - "+pRep->name()))
    return;

  pLoc->repRem(lb_sel);

  // reload the repository ListBox
  this->_buildLvRep();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onBcAbort()
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
  OmLocation* pLoc = pMgr->locCur();
  if(!pLoc) return;

  // we set abort value to Remote object hash to identify it
  // within the download callback function
  this->_rmtDnl_abort = pLoc->rmtGet(lv_sel)->hash();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_onInit\n";
  #endif

  // retrieve main dialog
  this->_pUiMgr = static_cast<OmUiMgr*>(this->root());

  // Set batches New and Delete buttons icons
  this->setBmIcon(IDC_BC_STOP, Om_getResIcon(this->_hins, IDI_BT_NOT));
  this->setBmIcon(IDC_BC_NEW, Om_getResIcon(this->_hins, IDI_BT_ADD));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDI_BT_REM));
  this->setBmIcon(IDC_BC_QRY, Om_getResIcon(this->_hins, IDI_BT_REF));

  // define controls tool-tips
  this->_createTooltip(IDC_CB_LOC,    L"Select active Location");
  this->_createTooltip(IDC_LB_REP,    L"Repositories list");
  this->_createTooltip(IDC_BC_QRY,    L"Start or stop repositories query");
  this->_createTooltip(IDC_BC_NEW,    L"Configure and add new repository");
  this->_createTooltip(IDC_BC_DEL,    L"Remove selected repository entry");
  this->_createTooltip(IDC_LV_RMT,    L"Remote packages list");
  this->_createTooltip(IDC_BC_DNLD,   L"Download selected packages");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort download");

  DWORD lvStyle = LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_DOUBLEBUFFER;
  LVCOLUMNW lvCol;

  // Initialize Repository ListView control
  this->msgItem(IDC_LV_REP, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lvStyle);
  // set explorer theme
  SetWindowTheme(this->getItem(IDC_LV_REP),L"Explorer",nullptr);

  // we now add columns into our list-view control
  lvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
  //  "The alignment of the leftmost column is always LVCFMT_LEFT; it
  // cannot be changed." says Mr Microsoft. Do not ask why, the Microsoft's
  // mysterious ways... So, don't try to fix this.
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.pszText = const_cast<LPWSTR>(L"Status");
  lvCol.cx = 43;
  lvCol.iSubItem = 0;
  this->msgItem(IDC_LV_REP, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.fmt = LVCFMT_LEFT;
  lvCol.pszText = const_cast<LPWSTR>(L"Repository");
  lvCol.cx = 200;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_REP, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.fmt = LVCFMT_LEFT;
  lvCol.pszText = const_cast<LPWSTR>(L"Description");
  lvCol.cx = 400;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_REP, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  // Initialize Remote Packages ListView control
  this->msgItem(IDC_LV_RMT, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lvStyle);
  // set explorer theme
  SetWindowTheme(this->getItem(IDC_LV_RMT),L"Explorer",nullptr);

  // we now add columns into our list-view control
  lvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
  //  "The alignment of the leftmost column is always LVCFMT_LEFT; it
  // cannot be changed." says Mr Microsoft. Do not ask why, the Microsoft's
  // mysterious ways... So, don't try to fix this.
  lvCol.pszText = const_cast<LPWSTR>(L"Status");
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.cx = 43;
  lvCol.iSubItem = 0;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Name");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 300;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Version");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Category");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Size");
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.cx = 80;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Download");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 120;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_RMT, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_onShow\n";
  #endif

  // refresh dialog
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onHide()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_onHide\n";
  #endif

  // disable "Edit > Remote" in main menu
  this->_pUiMgr->setPopupItem(MNU_EDIT, MNU_EDIT_RMT, MF_GRAYED);

  // stop folder monitoring
  if(this->_dirMon_hth) this->_dirMon_stop();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onResize()
{
  // Locations Combo-Box
  this->_setItemPos(IDC_CB_LOC, 2, 2, this->cliUnitX()-4, 12);
  // Repositories label
  //this->_setItemPos(IDC_SC_LBL01, 2, 20, 180, 12);
  // Repositories ListBox
  this->_setItemPos(IDC_LV_REP, 2, 18, this->cliUnitX()-24, 46);
  this->_rsizeLvRep(); //< resize ListView columns adapted to client area

  // Repositories Apply, New.. and Delete buttons
  this->_setItemPos(IDC_BC_QRY, this->cliUnitX()-18, 18, 16, 14);
  this->_setItemPos(IDC_BC_NEW, this->cliUnitX()-18, 34, 16, 14);
  this->_setItemPos(IDC_BC_DEL, this->cliUnitX()-18, 50, 16, 14);

  // Horizontal separator
  this->_setItemPos(IDC_SC_SEPAR, 2, 68, this->cliUnitX()-4, 1);

  // Package List ListView
  this->_setItemPos(IDC_LV_RMT, 2, 74, this->cliUnitX()-4, this->cliUnitY()-92);
  this->_rsizeLvRmt(); //< resize ListView columns adapted to client area

  this->_setItemPos(IDC_BC_DNLD, 2, this->cliUnitY()-15, 52, 14);
  // Progress bar
  this->_setItemPos(IDC_PB_PKG, 56, this->cliUnitY()-14, this->cliUnitX()-113, 12);
  // Abort button
  this->_setItemPos(IDC_BC_ABORT, this->cliUnitX()-54, this->cliUnitY()-15, 52, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_onRefresh\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  OmContext* pCtx = pMgr->ctxCur();

  // disable the Progress-Bar
  this->enableItem(IDC_PB_PKG, false);

  // reload Location ComboBox
  this->_buildCbLoc();

  // reload Repository ListBox
  this->_buildLvRep();

  // disable or enable elements depending context
  this->enableItem(IDC_SC_LBL01, (pCtx != nullptr));
  this->enableItem(IDC_LV_RMT, (pCtx != nullptr));
  this->enableItem(IDC_LB_REP, (pCtx != nullptr));
  this->enableItem(IDC_BC_NEW, (pCtx != nullptr));

  // values for access errors
  bool lib_access = true;

  OmLocation* pLoc = pMgr->locCur();

  // We try to avoid unnecessary refresh of ListView by
  // select specific condition of refresh
  if(pLoc) {

    // restart folder monitoring if required
    if(pLoc->libDirAccess(true)) {
      this->_dirMon_init(pLoc->libDir());
    } else {
      lib_access = false;
    }

    pLoc->rmtRefresh(true);
  }

  this->_buildLvRmt();

  if(!pCtx) return;

  // Display error dialog AFTER ListView refreshed its content
  if(pLoc) {
    if(!lib_access) {
      Om_dlgBox_okl(this->_hwnd, L"Repositories", IDI_WRN,
                    L"Library folder access error", L"The Library folder "
                    "cannot be accessed because it do not exist or have read "
                    "access restrictions. Please check Channel's settings "
                    "and folder permissions.", pLoc->libDir());
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainNet::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainNet::_onQuit\n";
  #endif

  // stop Library folder changes monitoring
  if(this->_dirMon_hth) this->_dirMon_stop();

  // this should be done already...
  this->_thread_abort = true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMgrMainNet::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    if(LOWORD(wParam) == IDC_LV_REP) {

      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case NM_DBLCLK:
        this->_onLvRepHit();
        break;

      case LVN_ITEMCHANGED:
        this->_onLvRepSel();
        break;
      }
    }

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
          pLoc->rmtSetSorting(LS_SORT_CATG);
          break;
        case 4:
          pLoc->rmtSetSorting(LS_SORT_SIZE);
          break;
        case 5:
          return false; // ignore action
        default:
          pLoc->rmtSetSorting(LS_SORT_NAME);
          break;
        }
        this->_buildLvRmt(); //< rebuild ListView
        break;
      }
    }
    return false;
  }

  if(uMsg == WM_COMMAND) {

    // Prevent command/shorcut execution when main dialog is not active
    if(!this->_pUiMgr->active())
      return false;

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMgrMainNet::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

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

    case IDC_BC_DNLD: //< Main "Upgrade" button
      this->rmtDown(true);
      break;

    case IDC_BC_ABORT: //< Main "Abort" button
      this->_onBcAbort();
      break;

    // Menu : Edit > Remote > []
    case IDM_EDIT_RMT_DNWS:
      this->rmtDown(false);
      break;

    case IDM_EDIT_RMT_DNLD:
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
