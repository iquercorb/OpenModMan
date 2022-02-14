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

#include "OmManager.h"

#include "OmUiMgr.h"
#include "OmUiMgrMain.h"
#include "OmUiMgrFoot.h"
#include "OmUiAddLoc.h"
#include "OmUiAddBat.h"
#include "OmUiPropPkg.h"
#include "OmUiPropBat.h"
#include "OmUiToolPkg.h"

#include "OmUtilFs.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiMgrMainLib.h"

/// \brief Custom "Package Install Done" Message
///
/// Custom "Package Install Done" window message to notify the dialog that the
/// running thread finished his job.
///
#define UWM_PKGINST_DONE      (WM_APP+1)

/// \brief Custom "Package Uninstall Done" Message
///
/// Custom "Package Uninstall Done" window message to notify the dialog that the
/// running thread finished his job.
///
#define UWM_PKGUNIN_DONE      (WM_APP+2)

/// \brief Custom "Package Clean Done" Message
///
/// Custom "Package Clean Done" window message to notify the dialog that the
/// running thread finished his job.
///
#define UWM_PKGCLNS_DONE     (WM_APP+3)

/// \brief Custom "Package Uninstall Done" Message
///
/// Custom "Package Uninstall Done" window message to notify the dialog that the
/// running thread finished his job.
///
#define UWM_BATEXE_DONE       (WM_APP+4)

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
OmUiMgrMainLib::OmUiMgrMainLib(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr),
  _dirMon_hth(nullptr),
  _dirMon_hev{0,0,0},
  _pkgInst_hth(nullptr),
  _pkgUnin_hth(nullptr),
  _pkgClns_hth(nullptr),
  _batExe_hth(nullptr),
  _thread_abort(false),
  _buildLvBat_icSize(0),
  _buildLvPkg_icSize(0),
  _buildLvPkg_legacy(true)
{
  // Package info sub-dialog
  this->addChild(new OmUiPropPkg(hins));

  // set the accelerator table for the dialog
  this->setAccel(IDR_ACCEL);

  // elements for real-time directory monitoring thread
  this->_dirMon_hev[0] = CreateEvent(nullptr, true, false, nullptr); //< custom event to notify thread must exit
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrMainLib::~OmUiMgrMainLib()
{
  // stop Library folder changes monitoring
  if(this->_dirMon_hth) this->_dirMon_stop();

  // Get the previous Image List to be destroyed (Small and Normal uses the same)
  HIMAGELIST hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_PKG, LVM_GETIMAGELIST, LVSIL_NORMAL));
  if(hImgLs) ImageList_Destroy(hImgLs);

  // Get the previous Image List to be destroyed (Small and Normal uses the same)
  hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_BAT, LVM_GETIMAGELIST, LVSIL_NORMAL));
  if(hImgLs) ImageList_Destroy(hImgLs);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMgrMainLib::id() const
{
  return IDD_MGR_MAIN_LIB;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // Location ComboBox
  this->enableItem(IDC_CB_LOC, !enable);

  // Packages ListView & buttons
  this->enableItem(IDC_LV_PKG, !enable);
  this->enableItem(IDC_BC_INST, !enable);
  this->enableItem(IDC_BC_UNIN, !enable);

  // Batches Label, ListBox & Buttons
  this->enableItem(IDC_SC_LBL01, !enable);
  this->enableItem(IDC_LB_BAT, !enable);
  this->enableItem(IDC_BC_NEW, !enable);
  // Batch Launch & Delete Buttons
  if(enable) {
    this->enableItem(IDC_BC_DEL, false);
    this->enableItem(IDC_BC_RUN, false);
  } else {
    int lb_sel = this->msgItem(IDC_LB_BAT, LB_GETCURSEL);
    this->enableItem(IDC_BC_DEL, (lb_sel >= 0));
    this->enableItem(IDC_BC_RUN, (lb_sel >= 0));
  }

  // Abort Button
  this->enableItem(IDC_BC_ABORT, enable);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
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
void OmUiMgrMainLib::locSel(int id)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::locSel " << id << "\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // stop Library folder monitoring
  if(this->_dirMon_hth) this->_dirMon_stop();

  // disable "Edit > Package []" in main menu
  this->_pUiMgr->setPopupItem(1, 5, MF_GRAYED);

  // select the requested Location
  if(pCtx) {

    pCtx->locSel(id);

    OmLocation* pLoc = pCtx->locCur();

    if(pLoc) {

      // Check Location Library folder access
      if(pLoc->libDirAccess(false)) { //< check only for reading
        // start Library folder monitoring
        this->_dirMon_init(pLoc->libDir());
      }

      // enable the "Edit > Location properties..." menu
      this->_pUiMgr->setPopupItem(1, 2, MF_ENABLED);

    } else {

      // disable the "Edit > Location properties..." menu
      this->_pUiMgr->setPopupItem(1, 2, MF_GRAYED);
    }
  }

  // refresh
  this->_buildEcLib();
  this->_buildLvPkg();

  // forces control to select item
  this->msgItem(IDC_CB_LOC, CB_SETCURSEL, id);

  if(!pCtx) return;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::pkgInst()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT))
    return;

  this->_pkgInst_init();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::pkgUnin()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT))
    return;

  this->_pkgUnin_init();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::pkgClns()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT))
    return;

  this->_pkgClns_init();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::pkgTogg()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx->locCur()) return;

  if(pCtx->locCur()->pkgGet(lv_sel)->hasBck()) {
    this->_pkgUnin_init();
  } else {
    this->_pkgInst_init();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::pkgProp()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx->locCur()) return;

  OmPackage* pPkg = nullptr;

  pPkg = pCtx->locCur()->pkgGet(lv_sel);

  if(pPkg) {
    OmUiPropPkg* pUiPropPkg = static_cast<OmUiPropPkg*>(this->childById(IDD_PROP_PKG));
    pUiPropPkg->pkgSet(pPkg);
    pUiPropPkg->open(true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::pkgTrsh()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT))
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx->locCur()) return;

  vector<OmPackage*> sel_ls;

  int lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    sel_ls.push_back(pCtx->locCur()->pkgGet(lv_sel));

    // next selected item
    lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  wstring msg;

  // Unselect all items
  LVITEM lvI = {};
  lvI.mask = LVIF_STATE;
  lvI.stateMask = LVIS_SELECTED;
  this->msgItem(IDC_LV_PKG, LVM_SETITEMSTATE, -1, reinterpret_cast<LPARAM>(&lvI));

  if(!Om_dlgBox_yn(this->_hwnd, L"Delete Packages", IDI_PKG_DEL,
               L"Delete selected Packages",
               L"Move the selected packages to recycle bin ?"))
  {
    return;
  }

  // freeze dialog so user cannot interact
  this->_pUiMgr->freeze(true);

  OmPackage* pPkg;

  for(size_t i = 0; i < sel_ls.size(); ++i) {

    pPkg = sel_ls[i];

    if(pPkg->hasSrc()) {
      Om_moveToTrash(pPkg->srcPath());
    } else {

      Om_dlgBox_ok(this->_hwnd, L"Delete Packages", IDI_PKG_WRN,
                    L"Package source does not exists",
                    L"Source file or folder of Package \""+pPkg->ident()+
                    L"\" does not exists in Library folder.");
    }
  }

  // unfreeze dialog to allow user to interact
  this->_pUiMgr->freeze(false);

  // update package selection
  this->_onLvPkgSel();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::pkgOpen()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT))
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx->locCur()) return;

  vector<OmPackage*> sel_ls;

  int lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    sel_ls.push_back(pCtx->locCur()->pkgGet(lv_sel));

    // next selected item
    lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  for(size_t i = 0; i < sel_ls.size(); ++i) {

    // the default behavior is to explore (open explorer with deployed folders)
    // however, it may happen that zip file are handled by an application
    // (typically, WinRar or 7zip) and the "explore" command may fail, in this
    // case, we call the "open" command.

    if(ShellExecuteW(this->_hwnd, L"explore", sel_ls[i]->srcPath().c_str(), nullptr, nullptr, SW_NORMAL ) <= (HINSTANCE)32) {
      ShellExecuteW(this->_hwnd, L"open", sel_ls[i]->srcPath().c_str(), nullptr, nullptr, SW_NORMAL );
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::pkgEdit()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx->locCur()) return;

  OmPackage* pPkg = nullptr;

  pPkg = pCtx->locCur()->pkgGet(lv_sel);

  if(pPkg) {
    OmUiToolPkg* pUiToolPkg = static_cast<OmUiToolPkg*>(this->_pUiMgr->childById(IDD_TOOL_PKG));
    if(pUiToolPkg->visible()) {
      pUiToolPkg->selSrc(pPkg->srcPath());
    } else {
      pUiToolPkg->setSrc(pPkg->srcPath());
      pUiToolPkg->modeless(true);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMgrMainLib::_pkgProgressCb(void* ptr, size_t tot, size_t cur, uint64_t data)
{
  OmUiMgrMainLib* self = reinterpret_cast<OmUiMgrMainLib*>(ptr);

  self->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, tot));
  self->msgItem(IDC_PB_PKG, PBM_SETPOS, cur);

  return !self->_thread_abort;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_pkgInstLs(const vector<OmPackage*>& pkg_ls, bool silent)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  wstring msg_lst;

  vector<OmPackage*> inst_ls; //< final install list
  vector<OmPackage*> over_ls; //< overlapping list
  vector<OmPackage*> dpcs_ls; //< extra dependencies install list
  vector<wstring> miss_ls;    //< missing dependencies lists

  // prepare package installation
  pLoc->pkgPrepareInst(inst_ls, over_ls, dpcs_ls, miss_ls, pkg_ls);

  // warn user for missing dependencies
  if(!silent && miss_ls.size() && pLoc->warnMissDeps()) {

    __msg_package_list(msg_lst, miss_ls);
    if(!Om_dlgBox_cal(this->_hwnd, L"Install Packages", IDI_PKG_WRN,
                  L"Missing Packages dependencies", L"One or more selected packages "
                  "have missing dependencies, the following packages are "
                  "required but not available:", msg_lst))
    {
      return;
    }
  }

  // warn for additional installation
  if(!silent && dpcs_ls.size() && pLoc->warnExtraInst()) {

    __msg_package_list(msg_lst, dpcs_ls);
    if(!Om_dlgBox_cal(this->_hwnd, L"Install Packages", IDI_PKG_ADD,
                  L"Packages dependencies", L"One or more selected packages "
                  "have dependencies, the following packages will also be "
                  "installed:", msg_lst))
    {
      return;
    }
  }

  // if there is overlapping, ask user if he really want to continue installation
  if(!silent && over_ls.size() && pLoc->warnOverlaps()) {

    __msg_package_list(msg_lst, over_ls);
    if(!Om_dlgBox_cal(this->_hwnd, L"Install Packages", IDI_PKG_OWR,
                  L"Packages overlaps", L"One or more selected packages "
                  "overlaps and will overwrites files previously installed "
                  "by the following package(s):", msg_lst))
    {
      return;
    }
  }

  // this is to update list view item's icon individually
  LVITEMW lvi;
  lvi.mask = LVIF_IMAGE;
  lvi.iSubItem = 0;

  OmPackage* pPkg;
  vector<OmPackage*> ovlp_ls; //< package overlapping list

  for(size_t i = 0; i < inst_ls.size(); ++i) {

    pPkg = inst_ls[i];

    // check whether abort is requested
    if(this->_thread_abort)
      break;

    // set WIP status image
    lvi.iItem = pLoc->pkgIndex(pPkg);
    lvi.iImage =  4; //< STS_WIP
    this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));

    if(!pPkg->hasSrc() || pPkg->hasBck())
      continue;

    // we check overlapping before installation, we must do it step by step
    // because overlapping are cumulative with previously installed packages
    ovlp_ls.clear();
    pLoc->pkgFindOverlaps(ovlp_ls, pPkg);

    // install package
    if(!pPkg->install(pLoc->bckZipLevel(), &this->_pkgProgressCb, this)) {
      Om_dlgBox_okl(this->_hwnd, L"Install Packages", IDI_PKG_ERR,
                    L"Package installation error", L"Installation of \""+
                    pPkg->name()+L"\" failed because of the following error:",
                    pPkg->lastError());
    }

    // update package icon in ListView
    if(pPkg->hasBck()) {
      lvi.iImage = 7; //< STS_BOK
      this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      // update icons for overlapped packages
      for(size_t j = 0; j < ovlp_ls.size(); ++j) {
        lvi.iItem = pLoc->pkgIndex(ovlp_ls[j]);
        lvi.iImage = 8; //< STS_OWR
        this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      }
    } else {
      lvi.iImage = -1; //< No Icon
      this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
    }

    // reset progress bar
    this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0, 0);

    #ifdef DEBUG
    Sleep(OMM_DEBUG_SLOW); //< for debug
    #endif
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_pkgUninLs(const vector<OmPackage*>& pkg_ls, bool silent)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  wstring msg_lst;

  vector<OmPackage*> over_ls; // extra overlapped uninstall list
  vector<OmPackage*> dpnd_ls; // extra dependents uninstall list
  vector<OmPackage*> unin_ls; // final uninstall list

  // prepare packages uninstall and backups restoration
  pLoc->bckPrepareUnin(unin_ls, over_ls, dpnd_ls, pkg_ls);

  // check and warn for extra uninstall due to overlaps
  if(!silent && over_ls.size() && pLoc->warnExtraUnin()) {

    __msg_package_list(msg_lst, over_ls);

    if(!Om_dlgBox_cal(this->_hwnd, L"Uninstall Packages", IDI_PKG_OWR,
                  L"Packages overlaps", L"One or more selected packages "
                  "are overlapped by others later installed, the "
                  "following packages must also be uninstalled:", msg_lst))
    {
      return;
    }

  }

  // check and warn for extra uninstall due to dependencies
  if(!silent && dpnd_ls.size() && pLoc->warnExtraUnin()) {

    __msg_package_list(msg_lst, dpnd_ls);

    if(!Om_dlgBox_cal(this->_hwnd, L"Uninstall Packages", IDI_PKG_WRN,
                  L"Packages dependencies", L"One or more selected packages "
                  "are required as dependency by others, the "
                  "following packages will also be uninstalled:", msg_lst))
    {
      return;
    }
  }

  // this is to update list view item's icon individually
  LVITEMW lvi;
  lvi.mask = LVIF_IMAGE;
  lvi.iSubItem = 0;

  OmPackage* pPkg;
  vector<OmPackage*> ovlp_ls; //< overlapped packages list

  for(size_t i = 0; i < unin_ls.size(); ++i) {

    pPkg = unin_ls[i];

    // check whether abort is requested
    if(this->_thread_abort)
      break;

    // set WIP status image
    lvi.iItem = pLoc->pkgIndex(pPkg);
    lvi.iImage = 4; //< STS_WIP
    this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));

    if(!pPkg->hasBck()) //< this should be always the case
      continue;

    // before uninstall, get list of overlapped packages (by this one)
    ovlp_ls.clear();
    for(size_t j = 0; j < pPkg->ovrCount(); ++j) {
      ovlp_ls.push_back(pLoc->pkgFind(pPkg->ovrGet(j)));
    }

    // uninstall package (restore backup)
    if(!pPkg->uninst(&this->_pkgProgressCb, this)) {
      Om_dlgBox_okl(this->_hwnd, L"Uninstall Packages", IDI_PKG_ERR,
                    L"Package uninstallation error", L"Backup restoration of \""+
                    pPkg->name()+L"\" failed or may be incomplete because of "
                    "the following error:", pPkg->lastError());
    }

    if(pPkg->hasBck()) { //< this mean something went wrong
      lvi.iImage = pLoc->bckOverlapped(pPkg) ? 8/*STS_OWR*/:7/*STS_BOK*/;
      this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
    } else {
      lvi.iImage = -1; //< No Icon
      this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      // update status icon for overlapped packages
      for(size_t j = 0; j < ovlp_ls.size(); ++j) {
        lvi.iItem = pLoc->pkgIndex(ovlp_ls[j]);
        lvi.iImage = pLoc->bckOverlapped(ovlp_ls[j]) ? 8/*STS_OWR*/:7/*STS_BOK*/;
        this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      }
    }

    // reset progress bar
    this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0, 0);

    #ifdef DEBUG
    Sleep(OMM_DEBUG_SLOW); //< for debug
    #endif
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_pkgClnsLs(const vector<OmPackage*>& pkg_ls, bool silent)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  wstring msg_lst;

  vector<OmPackage*> over_ls; // extra overlapped uninstall list
  vector<OmPackage*> dpnd_ls; // extra dependents uninstall list
  vector<OmPackage*> dpcs_ls; // extra dependencies uninstall list
  vector<OmPackage*> clns_ls; // final clean uninstall list

  // prepare packages uninstall and backups restoration
  pLoc->bckPrepareClns(clns_ls, over_ls, dpnd_ls, dpcs_ls, pkg_ls);

  // check and warn for extra uninstall due to dependencies
  if(!silent && dpcs_ls.size() && pLoc->warnExtraUnin()) {

    __msg_package_list(msg_lst, dpcs_ls);

    if(!Om_dlgBox_cal(this->_hwnd, L"Uninstall Tree", IDI_PKG_WRN,
                  L"Uninstall Package with dependencies", L"This will uninstall "
                  "selected packages including all dependencies, the "
                  "following packages will also be uninstalled:", msg_lst))
    {
      return;
    }
  }

  // check and warn for extra uninstall due to overlaps
  if(!silent && over_ls.size() && pLoc->warnExtraUnin()) {

    __msg_package_list(msg_lst, over_ls);

    if(!Om_dlgBox_cal(this->_hwnd, L"Uninstall Tree", IDI_PKG_OWR,
                  L"Packages overlaps", L"One or more selected packages "
                  "are overlapped by others later installed, the "
                  "following packages must also be uninstalled:", msg_lst))
    {
      return;
    }

  }

  // check and warn for extra uninstall due to dependencies
  if(!silent && dpnd_ls.size() && pLoc->warnExtraUnin()) {

    __msg_package_list(msg_lst, dpnd_ls);

    if(!Om_dlgBox_cal(this->_hwnd, L"Uninstall Tree", IDI_PKG_WRN,
                  L"Packages dependencies", L"One or more selected packages "
                  "are required as dependency by others, the "
                  "following packages will also be uninstalled:", msg_lst))
    {
      return;
    }
  }

  // this is to update list view item's icon individually
  LVITEMW lvi;
  lvi.mask = LVIF_IMAGE;
  lvi.iSubItem = 0;

  OmPackage* pPkg;
  vector<OmPackage*> ovlp_ls; //< overlapped packages list

  for(size_t i = 0; i < clns_ls.size(); ++i) {

    pPkg = clns_ls[i];

    // check whether abort is requested
    if(this->_thread_abort)
      break;

    // set WIP status image
    lvi.iItem = pLoc->pkgIndex(pPkg);
    lvi.iImage = 4; //< STS_WIP
    this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));

    if(!pPkg->hasBck()) //< this should be always the case
      continue;

    // before uninstall, get list of overlapped packages (by this one)
    ovlp_ls.clear();
    for(size_t j = 0; j < pPkg->ovrCount(); ++j) {
      ovlp_ls.push_back(pLoc->pkgFind(pPkg->ovrGet(j)));
    }

    // uninstall package (restore backup)
    if(!pPkg->uninst(&this->_pkgProgressCb, this)) {
      Om_dlgBox_okl(this->_hwnd, L"Uninstall Tree ", IDI_PKG_ERR,
                    L"Package uninstallation error", L"Backup restoration of \""+
                    pPkg->name()+L"\" failed or may be incomplete because of "
                    "the following error:", pPkg->lastError());
    }

    if(pPkg->hasBck()) { //< this mean something went wrong
      lvi.iImage = pLoc->bckOverlapped(pPkg) ? 8/*STS_OWR*/:7/*STS_BOK*/;
      this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
    } else {
      lvi.iImage = -1; //< No Icon
      this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      // update status icon for overlapped packages
      for(size_t j = 0; j < ovlp_ls.size(); ++j) {
        lvi.iItem = pLoc->pkgIndex(ovlp_ls[j]);
        lvi.iImage = pLoc->bckOverlapped(ovlp_ls[j]) ? 8/*STS_OWR*/:7/*STS_BOK*/;
        this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      }
    }

    // reset progress bar
    this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0, 0);

    #ifdef DEBUG
    Sleep(OMM_DEBUG_SLOW); //< for debug
    #endif
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_buildCbLoc()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_buildCbLoc\n";
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
    if(!Om_dlgBox_yn(this->_hwnd, L"Packages Library", IDI_QRY,
                  L"Empty Software Context", L"The selected Software Context is "
                  "empty and have no Target Location configured. Do you want "
                  "to add a Target Location now ?"))
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
void OmUiMgrMainLib::_buildEcLib()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_buildEcLib\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // get current context and location
  OmContext* pCtx = pMgr->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  wstring item_str;

  if(pLoc) {
    if(pLoc->libDirAccess(false)) { //< check only for reading
      item_str = pLoc->libDir();
    } else {
      item_str = L"<folder access error>";
    }
  }

  // set Library Path EditText control
  this->setItemText(IDC_EC_INP01, item_str);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_buildLvPkg()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_buildLvPkg\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // if icon size changed, create new ImageList
  if(this->_buildLvPkg_icSize != pMgr->iconsSize()) {

    HIMAGELIST hImgLs;

    // Get the previous Image List to be destroyed (Small and Normal uses the same)
    hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_PKG, LVM_GETIMAGELIST, LVSIL_NORMAL));
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
    hImgLs = ImageList_Create(pMgr->iconsSize(), pMgr->iconsSize(), ILC_COLOR32, 12, 0 );
    for(unsigned i = 0; i < 12; ++i)
      ImageList_Add(hImgLs, Om_getResImage(this->_hins, idb[i]), nullptr);

    // Set ImageList to ListView
    this->msgItem(IDC_LV_PKG, LVM_SETIMAGELIST, LVSIL_SMALL, reinterpret_cast<LPARAM>(hImgLs));
    this->msgItem(IDC_LV_PKG, LVM_SETIMAGELIST, LVSIL_NORMAL, reinterpret_cast<LPARAM>(hImgLs));

    // update size
    this->_buildLvPkg_icSize = pMgr->iconsSize();
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_PKG, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty list view
  this->msgItem(IDC_LV_PKG, LVM_DELETEALLITEMS);

  // get current context and location
  OmContext* pCtx = pMgr->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  if(!pLoc) {
    // disable ListView
    this->enableItem(IDC_LV_PKG, false);
    // update Package ListView selection
    this->_onLvPkgSel();
    // return now
    return;
  }

  // save current legacy support status
  this->_buildLvPkg_legacy = pLoc->libDevMode();

  // add item to list view
  OmPackage* pPkg;
  LVITEMW lvItem;
  for(unsigned i = 0; i < pLoc->pkgCount(); ++i) {

    pPkg = pLoc->pkgGet(i);

    // the first column, package status, here we INSERT the new item
    lvItem.iItem = i;
    lvItem.mask = LVIF_IMAGE;
    lvItem.iSubItem = 0;
    if(pPkg->isType(PKG_TYPE_BCK)) {
      lvItem.iImage = pLoc->bckOverlapped(pPkg) ? 8/*STS_OWR*/:7/*STS_BOK*/;
    } else {
      lvItem.iImage = -1; // No Icon
    }
    lvItem.iItem = this->msgItem(IDC_LV_PKG, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Second column, the package name and type, here we set the sub-item
    lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
    lvItem.iSubItem = 1;
    if(pPkg->isType(PKG_TYPE_SRC)) {
      lvItem.iImage = pPkg->isZip() ? (pPkg->depCount() ? 3/*PKG_DPN*/:2/*PKG_ZIP*/):1/*PKG_DIR*/;
    } else {
      lvItem.iImage = 0; // PKG_ERR
    }
    lvItem.pszText = const_cast<LPWSTR>(pPkg->name().c_str());
    this->msgItem(IDC_LV_PKG, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Third column, the package version, we set the sub-item
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 2;
    lvItem.pszText = const_cast<LPWSTR>(pPkg->version().asString().c_str());
    this->msgItem(IDC_LV_PKG, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Fourth column, the package category, we set the sub-item
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 3;
    lvItem.pszText = const_cast<LPWSTR>(pPkg->category().c_str());
    this->msgItem(IDC_LV_PKG, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
  }

  // we enable the ListView
  this->enableItem(IDC_LV_PKG, true);

  // restore ListView scroll position from lvRec
  this->msgItem(IDC_LV_PKG, LVM_SCROLL, 0, -lvRec.top );

  // adapt ListView column size to client area
  this->_rsizeLvPkg();

  // update Package ListView selection
  this->_onLvPkgSel();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_rsizeLvPkg()
{
  LONG size[4];

  // Resize the Packages ListView column
  GetClientRect(this->getItem(IDC_LV_PKG), reinterpret_cast<LPRECT>(&size));
  this->msgItem(IDC_LV_PKG, LVM_SETCOLUMNWIDTH, 1, size[2]-195);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_buildLvBat()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_buildLvBat\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  if(!pCtx) {
    // empty ListView
    this->msgItem(IDC_LV_BAT, LVM_DELETEALLITEMS);
    // disable ListView
    this->enableItem(IDC_LV_BAT, false);
    // update Batches ListView selection
    this->_onLvBatSel();
    // return now
    return;
  }

  // if icon size changed, create new ImageList
  if(this->_buildLvBat_icSize != pMgr->iconsSize()) {

    HIMAGELIST hImgLs;

    // Get the previous Image List to be destroyed (Small and Normal uses the same)
    hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_BAT, LVM_GETIMAGELIST, LVSIL_NORMAL));
    if(hImgLs) ImageList_Destroy(hImgLs);

    // Build list of images resource ID for the required size
    unsigned idb = IDB_BAT_16;

    switch(pMgr->iconsSize())
    {
    case 24:
      idb += 1; //< steps IDs to 24 pixels images
      break;
    case 32:
      idb += 2; //< steps IDs to 32 pixels images
      break;
    }

    // Create ImageList and fill it with bitmaps
    hImgLs = ImageList_Create(pMgr->iconsSize(), pMgr->iconsSize(), ILC_COLOR32, 1, 0 );
    ImageList_Add(hImgLs, Om_getResImage(this->_hins, idb), nullptr);

    // Set ImageList to ListView
    this->msgItem(IDC_LV_BAT, LVM_SETIMAGELIST, LVSIL_SMALL, reinterpret_cast<LPARAM>(hImgLs));
    this->msgItem(IDC_LV_BAT, LVM_SETIMAGELIST, LVSIL_NORMAL, reinterpret_cast<LPARAM>(hImgLs));

    // update size
    this->_buildLvBat_icSize = pMgr->iconsSize();
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_BAT, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty list view
  this->msgItem(IDC_LV_BAT, LVM_DELETEALLITEMS);

  OmBatch* pBat;
  LVITEMW lvItem;
  lvItem.mask = LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE; //< text and special data
  lvItem.iSubItem = 0;
  lvItem.iImage = 0;
  for(unsigned i = 0; i < pCtx->batCount(); ++i) {

    pBat = pCtx->batGet(i);
    lvItem.iItem = i; //< to order list according insertion order
    lvItem.pszText = const_cast<LPWSTR>(pBat->title().c_str());
    lvItem.lParam = static_cast<LPARAM>(i); // for Location index reordering

    this->msgItem(IDC_LV_BAT, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
  }

  // we enable the ListView
  this->enableItem(IDC_LV_BAT, true);

  // restore ListView scroll position from lvRec
  this->msgItem(IDC_LV_BAT, LVM_SCROLL, 0, -lvRec.top );

  // resize ListView columns to adapt client area
  this->_rsizeLvBat();

  // update Batches ListView selection
  this->_onLvBatSel();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_rsizeLvBat()
{
  LONG size[4];

  // Resize the Batches ListView columns
  GetClientRect(this->getItem(IDC_LV_BAT), reinterpret_cast<LPRECT>(&size));
  this->msgItem(IDC_LV_BAT, LVM_SETCOLUMNWIDTH, 0, size[2]);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_pkgInst_init()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx)return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc)return;

  // checks whether we have a valid Destination folder
  if(!pLoc->dstDirAccess(true)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, L"Install Packages", IDI_ERR,
                  L"Destination folder access error", L"The Destination folder "
                  "cannot be accessed because it do not exist or have read/write "
                  "access restrictions. Please check Target Location's settings "
                  "and folder permissions.", pLoc->dstDir());
    return;
  }
  // checks whether we have a valid Library folder
  if(!pLoc->libDirAccess(false)) { //< check only for read
    Om_dlgBox_okl(this->_hwnd, L"Install Packages", IDI_ERR,
                  L"Library folder access error", L"The Library folder "
                  "cannot be accessed because it do not exist or have read "
                  "access restrictions. Please check Target Location's settings "
                  "and folder permissions.", pLoc->libDir());
    return;
  }
  // checks whether we have a valid Backup folder
  if(!pLoc->bckDirAccess(true)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, L"Install Packages", IDI_ERR,
                  L"Backup folder access error", L"The Backup folder "
                  "cannot be accessed because it do not exist or have read/write "
                  "access restrictions. Please check Target Location's settings "
                  "and folder permissions.", pLoc->bckDir());
    return;
  }

  // Freezes the main dialog to prevent user to interact during process
  this->_pUiMgr->freeze(true);

  DWORD dwId;
  this->_pkgInst_hth = CreateThread(nullptr, 0, this->_pkgInst_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_pkgInst_stop()
{
  DWORD exitCode;

  // safely and cleanly close threads handles
  if(this->_pkgInst_hth) {
    WaitForSingleObject(this->_pkgInst_hth, INFINITE);
    GetExitCodeThread(this->_pkgInst_hth, &exitCode);
    CloseHandle(this->_pkgInst_hth);
    this->_pkgInst_hth = nullptr;
  }

  // Unfreezes dialog so user can interact again
  this->_pUiMgr->freeze(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMgrMainLib::_pkgInst_fth(void* arg)
{
  OmUiMgrMainLib* self = static_cast<OmUiMgrMainLib*>(arg);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx)return 1;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc)return 1;

  // string for dialog messages
  wstring msg;

  // get user selection
  vector<OmPackage*> user_ls;

  OmPackage* pPkg;

  int lv_sel = self->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    pPkg = pLoc->pkgGet(lv_sel);

    if(pPkg->hasSrc() && !pPkg->hasBck())
      user_ls.push_back(pPkg);

    // next selected item
    lv_sel = self->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  // reset abort status
  self->_thread_abort = false;

  // install packages
  self->_pkgInstLs(user_ls, false);

  // send message to notify process ended
  self->postMessage(UWM_PKGINST_DONE);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_pkgUnin_init()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx)return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc)return;

  // checks whether we have a valid Destination folder
  if(!pLoc->dstDirAccess(true)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, L"Uninstall Packages", IDI_ERR,
                  L"Destination folder access error", L"The Destination folder "
                  "cannot be accessed because it do not exist or have read/write "
                  "access restrictions. Please check Target Location's settings "
                  "and folder permissions.", pLoc->dstDir());
    return;
  }
  // checks whether we have a valid Backup folder
  if(!pLoc->bckDirAccess(true)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, L"Uninstall Packages", IDI_ERR,
                  L"Backup folder access error", L"The Backup folder "
                  "cannot be accessed because it do not exist or have read/write "
                  "access restrictions. Please check Target Location's settings "
                  "and folder permissions.", pLoc->bckDir());
    return;
  }

  // freeze dialog so user cannot interact
  this->_pUiMgr->freeze(true);

  DWORD dwId;
  this->_pkgUnin_hth = CreateThread(nullptr, 0, this->_pkgUnin_fth, this, 0, &dwId);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_pkgUnin_stop()
{
  DWORD exitCode;

  if(this->_pkgUnin_hth) {
    WaitForSingleObject(this->_pkgUnin_hth, INFINITE);
    GetExitCodeThread(this->_pkgUnin_hth, &exitCode);
    CloseHandle(this->_pkgUnin_hth);
    this->_pkgUnin_hth = nullptr;
  }

  // unfreeze dialog to allow user to interact again
  this->_pUiMgr->freeze(false);

  // Uninstall process may have leaved a ghost package (no source and
  // no backup), so we clean Library and rebuild ListView if needed
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  // clean Library list and rebuild ListView
  if(pLoc->libClean())
    this->_buildLvPkg();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMgrMainLib::_pkgUnin_fth(void* arg)
{
  OmUiMgrMainLib* self = static_cast<OmUiMgrMainLib*>(arg);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx)return 1;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc)return 1;

  // string for dialog messages
  wstring msg;

  // get user selection
  vector<OmPackage*> user_ls;

  OmPackage* pPkg;

  int lv_sel = self->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    pPkg = pLoc->pkgGet(lv_sel);

    if(pPkg->hasBck())
      user_ls.push_back(pPkg);

    // next selected item
    lv_sel = self->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  // reset abort status
  self->_thread_abort = false;

  // uninstall packages
  self->_pkgUninLs(user_ls, false);

  // send message to notify process ended
  self->postMessage(UWM_PKGUNIN_DONE);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_pkgClns_init()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx)return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc)return;

  // checks whether we have a valid Destination folder
  if(!pLoc->dstDirAccess(true)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, L"Uninstall Tree", IDI_ERR,
                  L"Destination folder access error", L"The Destination folder "
                  "cannot be accessed because it do not exist or have read/write "
                  "access restrictions. Please check Target Location's settings "
                  "and folder permissions.", pLoc->dstDir());
    return;
  }
  // checks whether we have a valid Backup folder
  if(!pLoc->bckDirAccess(true)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, L"Uninstall Tree", IDI_ERR,
                  L"Backup folder access error", L"The Backup folder "
                  "cannot be accessed because it do not exist or have read/write "
                  "access restrictions. Please check Target Location's settings "
                  "and folder permissions.", pLoc->bckDir());
    return;
  }

  // freeze dialog so user cannot interact
  this->_pUiMgr->freeze(true);

  DWORD dwId;
  this->_pkgClns_hth = CreateThread(nullptr, 0, this->_pkgClns_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_pkgClns_stop()
{
  DWORD exitCode;

  if(this->_pkgClns_hth) {
    WaitForSingleObject(this->_pkgClns_hth, INFINITE);
    GetExitCodeThread(this->_pkgClns_hth, &exitCode);
    CloseHandle(this->_pkgClns_hth);
    this->_pkgClns_hth = nullptr;
  }

  // unfreeze dialog to allow user to interact again
  this->_pUiMgr->freeze(false);

  // Uninstall process may have leaved a ghost package (no source and
  // no backup), so we clean Library and rebuild ListView if needed
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  // clean Library list and rebuild ListView
  if(pLoc->libClean())
    this->_buildLvPkg();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMgrMainLib::_pkgClns_fth(void* arg)
{
  OmUiMgrMainLib* self = static_cast<OmUiMgrMainLib*>(arg);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx)return 1;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc)return 1;

  // string for dialog messages
  wstring msg;

  // get user selection
  vector<OmPackage*> user_ls;

  OmPackage* pPkg;

  int lv_sel = self->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    pPkg = pLoc->pkgGet(lv_sel);

    if(pPkg->hasBck())
      user_ls.push_back(pPkg);

    // next selected item
    lv_sel = self->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  // reset abort status
  self->_thread_abort = false;

  // uninstall packages
  self->_pkgClnsLs(user_ls, false);

  // send message to notify process ended
  self->postMessage(UWM_PKGCLNS_DONE);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_batExe_init()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_BAT, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // freeze dialog so user cannot interact
  this->_pUiMgr->freeze(true);

  DWORD dwId;
  this->_batExe_hth = CreateThread(nullptr, 0, this->_batExe_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_batExe_stop()
{
  DWORD exitCode;

  if(this->_batExe_hth) {
    WaitForSingleObject(this->_batExe_hth, INFINITE);
    GetExitCodeThread(this->_batExe_hth, &exitCode);
    CloseHandle(this->_batExe_hth);
    this->_batExe_hth = nullptr;
  }

  // unfreeze dialog to allow user to interact again
  this->_pUiMgr->freeze(false);

  // Uninstall process may have leaved a ghost package (no source and
  // no backup), so we clean Library and rebuild ListView if needed
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  // clean Library list and rebuild ListView
  if(pLoc->libClean())
    this->_buildLvPkg();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMgrMainLib::_batExe_fth(void* arg)
{
  OmUiMgrMainLib* self = static_cast<OmUiMgrMainLib*>(arg);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // save current selected location
  int cb_sel = self->msgItem(IDC_CB_LOC, CB_GETCURSEL);

  // Get Batches ListView unique selection
  int lv_sel = self->msgItem(IDC_LV_BAT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  if(lv_sel >= 0) {

    // hide package details
    self->showItem(IDC_SB_SNAP, false);
    self->showItem(IDC_EC_DESC, false);
    self->showItem(IDC_SC_NAME, false);

    // structure for ListView update
    LVITEMW lvItem = {};
    lvItem.mask = LVIF_PARAM; //< we want item param
    lvItem.iItem = lv_sel;
    lvItem.lParam = -1;
    self->msgItem(IDC_LV_BAT, LVM_GETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // retrieve the batch object from current selection
    OmBatch* pBat = pCtx->batGet(lvItem.lParam);

    // Automatic fix Batch / Context Location inconsistency
    for(size_t l = 0; l < pCtx->locCount(); ++l) //< Add missing Location
      if(!pBat->hasLoc(pCtx->locGet(l)->uuid()))
        pBat->locAdd(pCtx->locGet(l)->uuid());

    // Remove unavailable location
    vector<wstring> uuid_ls;
    for(size_t l = 0; l < pBat->locCount(); ++l)
      if(pCtx->locFind(pBat->locGetUuid(l)) < 0)
        uuid_ls.push_back(pBat->locGetUuid(l));

    for(size_t i = 0; i < uuid_ls.size(); ++i)
      pBat->locRem(uuid_ls[i]);

    unsigned n;
    OmLocation* pLoc;

    // create an install and an uninstall list
    vector<OmPackage*> inst_ls, unin_ls;

    OmPackage* pPkg;

    for(unsigned l = 0; l < pBat->locCount(); l++) {

      // Select the Location found by UUID
      self->locSel(pCtx->locFind(pBat->locGetUuid(l)));

      pLoc = pCtx->locCur();

      if(!pLoc) {
        // TODO: warn here because Location no longer exists
        continue;
      }

      // create the install list, to keep package order from batch we
      // fill the install list according the batch hash list
      n = pBat->insCount(l);
      for(unsigned i = 0; i < n; ++i) {
        if((pPkg = pLoc->pkgFind(pBat->insGet(l, i))) != nullptr) {
          if(pPkg->hasSrc() && !pPkg->hasBck())
            inst_ls.push_back(pPkg);
        } else {
          // TODO: handle no longer available package
        }
      }

      // create the uninstall list, here we do not care order
      n = pLoc->pkgCount();
      for(unsigned i = 0; i < n; ++i) {
        pPkg = pLoc->pkgGet(i);
        if(!pBat->hasIns(l, pPkg->hash())) {
          if(pPkg->hasBck()) {
            unin_ls.push_back(pPkg);
          }
        }
      }

      // first, uninstall packages which must be uninstalled
      if(unin_ls.size()) {
        // uninstall packages
        self->_pkgUninLs(unin_ls, pCtx->batQuietMode());
      }

      // then, install packages which must be installed
      if(inst_ls.size()) {

        // batch execution require packages to be installed in the order
        // the user chosen, however, the package install process itself may
        // change this order due to dependencies.
        //
        // To ensure both exigences are respect, we launch one install
        // process per batch install package
        vector<OmPackage*> inst;
        for(size_t i = 0; i < inst_ls.size(); ++i) {
          // clear and replace package index in vector
          inst.clear(); inst.push_back(inst_ls[i]);
          // Launch install process
          self->_pkgInstLs(inst, pCtx->batQuietMode());
        }
      }
    }

    // restore package details
    self->showItem(IDC_SB_SNAP, true);
    self->showItem(IDC_EC_DESC, true);
    self->showItem(IDC_SC_NAME, true);
  }

  // Select previously selected location
  self->locSel(cb_sel);

  // send message to notify process ended
  self->postMessage(UWM_BATEXE_DONE);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_dirMon_init(const wstring& path)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_dirMon_init\n";
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
void OmUiMgrMainLib::_dirMon_stop()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_dirMon_stop\n";
  #endif

  // stops current directory monitoring thread
  if(this->_dirMon_hth) {

    // set custom event to request thread to quit, then wait for it
    SetEvent(this->_dirMon_hev[0]);
    WaitForSingleObject(this->_dirMon_hth, INFINITE);
    CloseHandle(this->_dirMon_hth);
    this->_dirMon_hth = nullptr;

    // reset the "stop" event for next usage
    ResetEvent(this->_dirMon_hev[0]);

    // close previous folder monitor
    FindCloseChangeNotification(this->_dirMon_hev[1]);
    this->_dirMon_hev[1] = nullptr;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMgrMainLib::_dirMon_fth(void* arg)
{
  OmUiMgrMainLib* self = static_cast<OmUiMgrMainLib*>(arg);

  DWORD dwObj;

  while(true) {

    dwObj = WaitForMultipleObjects(2, self->_dirMon_hev, false, INFINITE);

    if(dwObj == 0) //< custom "stop" event
      break;

    if(dwObj == 1) { //< folder content changed event

      #ifdef DEBUG
      std::cout << "DEBUG => OmUiMgrMainLib::_dirMon_fth (changes)\n";
      #endif

      OmManager* pMgr = static_cast<OmManager*>(self->_data);
      OmContext* pCtx = pMgr->ctxCur();

      if(pCtx) { //< this should be always the case
        if(pCtx->locCur()) { //< this should also be always the case
          // refresh Location Library
          if(pCtx->locCur()->libRefresh()) {
            // if list changed, rebuilt package ListView
            self->_buildLvPkg();
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
void OmUiMgrMainLib::_onCbLocSel()
{
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  this->locSel(cb_sel);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onLvPkgRclk()
{
  // get handle to "Edit > Packages..." sub-menu
  HMENU hMenu = this->_pUiMgr->getPopupItem(1, 5);

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  TrackPopupMenu(hMenu, TPM_TOPALIGN|TPM_LEFTALIGN, pt.x, pt.y, 0, this->_hwnd, nullptr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onLvPkgSel()
{
  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT);

  // check count of selected item
  if(!lv_nsl) {

    // hide all package bottom infos
    this->showItem(IDC_SB_SNAP, false);
    this->showItem(IDC_EC_DESC, false);
    this->showItem(IDC_SC_NAME, false);

    // disable install, uninstall abort and progress bar
    this->enableItem(IDC_BC_INST, false);
    this->enableItem(IDC_BC_UNIN, false);
    this->enableItem(IDC_PB_PKG, false);

    // disable "Edit > Package" in main menu
    this->_pUiMgr->setPopupItem(1, 5, MF_GRAYED);

    // show nothing in footer frame
    this->_pUiMgr->pUiMgrFoot()->clearItem();

    // return now
    return;
  }

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  // at least one, we enable buttons
  this->enableItem(IDC_BC_INST, true);
  this->enableItem(IDC_BC_UNIN, true);

  // enable "Edit > Package []" pop-up menu
  this->_pUiMgr->setPopupItem(1, 5, MF_ENABLED);
  HMENU hPopup = this->_pUiMgr->getPopupItem(1, 5);

  // Check whether we have multiple selection
  if(lv_nsl > 1) {

    // disable the "Edit > Package > View detail..." menu-item
    this->_pUiMgr->setPopupItem(hPopup, 6, MF_GRAYED); //< "View detail..." menu-item

    // on multiple selection, we hide package description
    this->showItem(IDC_EC_DESC, false);
    this->setItemText(IDC_SC_NAME, L"<Multiple selection>");
    this->showItem(IDC_SB_SNAP, false);

  } else {

    // show package title and thumbnail
    this->showItem(IDC_SC_NAME, true);
    this->showItem(IDC_SB_SNAP, true);

    // enable the "Edit > Package > .. " menu-item
    this->_pUiMgr->setPopupItem(hPopup, 6, MF_ENABLED); //< "View details" menu-item

    // show package description
    this->showItem(IDC_EC_DESC, true);

    OmPackage* pPkg;

    // get the selected item id (only one, no need to iterate)
    int lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
    if(lv_sel >= 0) {

      pPkg = pLoc->pkgGet(lv_sel);

      // show packages info in footer frame
      this->_pUiMgr->pUiMgrFoot()->selectItem(pPkg);

    } else {

      // reset footer frame
      this->_pUiMgr->pUiMgrFoot()->clearItem();
    }


  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onLvBatSel()
{
  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_BAT, LVM_GETSELECTEDCOUNT);

  this->enableItem(IDC_BC_RUN, (lv_nsl > 0));
  this->enableItem(IDC_BC_EDI, (lv_nsl > 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onBcRunBat()
{
  this->_batExe_init();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onBcNewBat()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  OmUiAddBat* pUiNewBat = static_cast<OmUiAddBat*>(this->_pUiMgr->childById(IDD_ADD_BAT));
  pUiNewBat->ctxSet(pCtx);
  pUiNewBat->open(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onBcEdiBat()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_BAT, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_BAT, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  // structure for ListView update
  LVITEMW lvItem = {};
  lvItem.mask = LVIF_PARAM; //< we want item param
  lvItem.iItem = lv_sel;
  lvItem.lParam = -1;
  this->msgItem(IDC_LV_BAT, LVM_GETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

  if(lvItem.lParam >= 0) {

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->ctxCur();
    if(!pCtx) return;

    OmUiPropBat* pUiPropBat = static_cast<OmUiPropBat*>(this->_pUiMgr->childById(IDD_PROP_BAT));
    pUiPropBat->batSet(pCtx->batGet(lvItem.lParam));
    pUiPropBat->open();
  }

  // reload the batch list-box
  this->_buildLvBat();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_onInit\n";
  #endif

  // retrieve main dialog
  this->_pUiMgr = static_cast<OmUiMgr*>(this->root());

  // Defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(21, 400, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_NAME, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(14, 700, L"Consolas");
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  // Set batches New and Delete buttons icons
  this->setBmIcon(IDC_BC_NEW, Om_getResIcon(this->_hins, IDI_BT_ADD));
  this->setBmIcon(IDC_BC_EDI, Om_getResIcon(this->_hins, IDI_BT_MOD));
  // set default package thumbnail
  this->setStImage(IDC_SB_SNAP, Om_getResImage(this->_hins, IDB_BLANK));

  // define controls tool-tips
  this->_createTooltip(IDC_CB_LOC,    L"Select active location");
  this->_createTooltip(IDC_BC_INST,   L"Install selected package(s)");
  this->_createTooltip(IDC_BC_UNIN,   L"Uninstall selected package(s)");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort current process");

  DWORD lvStyle;
  LVCOLUMNW lvCol;
  // Initialize Packages ListView control
  lvStyle = LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_DOUBLEBUFFER;
  this->msgItem(IDC_LV_PKG, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lvStyle);
  // set explorer theme
  SetWindowTheme(this->getItem(IDC_LV_PKG),L"Explorer",nullptr);

  // we now add columns into Packages list-view control
  lvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
  //  "The alignment of the leftmost column is always LVCFMT_LEFT; it
  // cannot be changed." says Mr Microsoft. Do not ask why, the Microsoft's
  // mysterious ways... So, don't try to fix this.
  lvCol.pszText = const_cast<LPWSTR>(L"Status");
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.cx = 43;
  lvCol.iSubItem = 0;
  this->msgItem(IDC_LV_PKG, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Name");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 440;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_PKG, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Version");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 70;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_PKG, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Category");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_PKG, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  // Initialize Batches ListView control
  this->msgItem(IDC_LV_BAT, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lvStyle);
  // set explorer theme
  SetWindowTheme(this->getItem(IDC_LV_BAT),L"Explorer",nullptr);

  // we now add columns into Batches list-view control
  lvCol.mask = LVCF_WIDTH;
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 440;
  lvCol.iSubItem = 0;
  this->msgItem(IDC_LV_BAT, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&lvCol));

  // hide package details
  this->showItem(IDC_SC_NAME, false);
  this->showItem(IDC_EC_DESC, false);
  this->showItem(IDC_SB_SNAP, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_onShow\n";
  #endif

  // refresh dialog
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onHide()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_onHide\n";
  #endif

  // disable "Edit > Package" in main menu
  this->_pUiMgr->setPopupItem(1, 5, MF_GRAYED);

  // stop folder monitoring
  if(this->_dirMon_hth) this->_dirMon_stop();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onResize()
{
  // Locations Combo-Box
  this->_setItemPos(IDC_CB_LOC, 2, 2, this->cliUnitX()-160, 12);
  // Library path EditControl
  this->_setItemPos(IDC_EC_INP01, 2, 18, this->cliUnitX()-160, 12);
  // Package List ListView
  this->_setItemPos(IDC_LV_PKG, 2, 34, this->cliUnitX()-160, this->cliUnitY()-52);
  this->_rsizeLvPkg(); //< Resize the Packages ListView column
  // Install and Uninstall buttons
  this->_setItemPos(IDC_BC_INST, 2, this->cliUnitY()-15, 52, 14);
  this->_setItemPos(IDC_BC_UNIN, 56, this->cliUnitY()-15, 52, 14);
  // Progress bar
  this->_setItemPos(IDC_PB_PKG, 110, this->cliUnitY()-14, this->cliUnitX()-322, 12);
  // Abort button
  this->_setItemPos(IDC_BC_ABORT, this->cliUnitX()-210, this->cliUnitY()-15, 52, 14);

  // Vertical separator
  this->_setItemPos(IDC_SC_SEPAR, this->cliUnitX()-154, 2, 1, this->cliUnitY()-3);

  // Batches label
  this->_setItemPos(IDC_SC_LBL01, this->cliUnitX()-150, 2, 140, 12);
  // Batches List-View
  this->_setItemPos(IDC_LV_BAT, this->cliUnitX()-150, 18, 148, this->cliUnitY()-36);
  // Batches Apply, New.. and Delete buttons
  this->_setItemPos(IDC_BC_RUN, this->cliUnitX()-150, this->cliUnitY()-15, 48, 14);
  this->_setItemPos(IDC_BC_NEW, this->cliUnitX()-100, this->cliUnitY()-15, 48, 14);
  this->_setItemPos(IDC_BC_EDI, this->cliUnitX()-50, this->cliUnitY()-15, 48, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_onRefresh\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // rebuild Batches ListBox
  this->_buildLvBat();

  // rebuild Location ComboBox
  this->_buildCbLoc();

  // disable abort button
  this->enableItem(IDC_BC_ABORT, false);

  // disable or enable elements depending context
  this->enableItem(IDC_SC_LBL01, (pCtx != nullptr));
  this->enableItem(IDC_LV_PKG, (pCtx != nullptr));
  this->enableItem(IDC_LB_BAT, (pCtx != nullptr));
  this->enableItem(IDC_BC_NEW, (pCtx != nullptr));

  // values for access errors
  bool dst_access = true;
  bool bck_access = true;
  bool lib_access = true;

  // We try to avoid unnecessary refresh of ListView by
  // select specific condition of refresh
  if(pCtx) {
    if(pCtx->locCur()) {

      dst_access = pCtx->locCur()->dstDirAccess(true);
      bck_access = pCtx->locCur()->bckDirAccess(true);

      // restart folder monitoring if required
      if(pCtx->locCur()->libDirAccess(false)) {
        this->_dirMon_init(pCtx->locCur()->libDir());
      } else {
        lib_access = false;
      }
    }
  }

  this->_buildEcLib();
  this->_buildLvPkg();

  if(!pCtx) return;

  // Display error dialog AFTER ListView refreshed its content
  if(pCtx->locCur()) {
    if(!dst_access) {
      Om_dlgBox_okl(this->_hwnd, L"Packages Library", IDI_WRN,
                    L"Destination folder access error", L"The Destination folder "
                    "cannot be accessed because it do not exist or have read/write "
                    "access restrictions. Please check Target Location's settings "
                    "and folder permissions.", pCtx->locCur()->dstDir());
    }

    if(!bck_access) {
      Om_dlgBox_okl(this->_hwnd, L"Packages Library", IDI_WRN,
                    L"Backup folder access error", L"The Backup folder "
                    "cannot be accessed because it do not exist or have read/write "
                    "access restrictions. Please check Target Location's settings "
                    "and folder permissions.", pCtx->locCur()->bckDir());
    }

    if(!lib_access) {
      Om_dlgBox_okl(this->_hwnd, L"Packages Library", IDI_WRN,
                    L"Library folder access error", L"The Library folder "
                    "cannot be accessed because it do not exist or have read "
                    "access restrictions. Please check Target Location's settings "
                    "and folder permissions.", pCtx->locCur()->libDir());
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainLib::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainLib::_onQuit\n";
  #endif

  // this should be done already...
  this->_thread_abort = true;

  // stop Library folder changes monitoring
  if(this->_dirMon_hth) this->_dirMon_stop();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMgrMainLib::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_PKGINST_DONE is a custom message sent from Package Install
  // thread function, to notify the thread ended is job.
  if(uMsg == UWM_PKGINST_DONE) {
    // properly stop the running thread and finish process
    this->_pkgInst_stop();
    return false;
  }

  // UWM_PKGINST_DONE is a custom message sent from Package Uninstall
  // thread function, to notify the thread ended is job.
  if(uMsg == UWM_PKGUNIN_DONE) {
    // properly stop the running thread and finish process
    this->_pkgUnin_stop();
    return false;
  }

  // UWM_PKGCLNS_DONE is a custom message sent from Package Clean
  // thread function, to notify the thread ended is job.
  if(uMsg == UWM_PKGCLNS_DONE) {
    // properly stop the running thread and finish process
    this->_pkgClns_stop();
    return false;
  }

  // UWM_BATEXE_DONE is a custom message sent from Batch Execution
  // thread function, to notify the thread ended is job.
  if(uMsg == UWM_BATEXE_DONE) {
    // properly stop the running thread and finish process
    this->_batExe_stop();
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
    this->enableItem(IDC_BC_ABORT, false);
    return false;
  }

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return false;

  if(uMsg == WM_NOTIFY) {

    OmLocation* pLoc = pCtx->locCur();
    if(!pLoc) return false;

    if(LOWORD(wParam) == IDC_LV_PKG) {

      // if thread is running we block all interaction
      if(this->_pkgInst_hth || this->_pkgUnin_hth)
        return false;

      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case NM_DBLCLK:
        this->pkgTogg();
        break;

      case NM_RCLICK:
        this->_onLvPkgRclk();
        break;

      case LVN_ITEMCHANGED:
        this->_onLvPkgSel();
        break;

      case LVN_COLUMNCLICK:
        switch(reinterpret_cast<NMLISTVIEW*>(lParam)->iSubItem)
        {
        case 0:
          pLoc->libSetSorting(LS_SORT_STAT);
          break;
        case 2:
          pLoc->libSetSorting(LS_SORT_VERS);
          break;
        case 3:
          pLoc->libSetSorting(LS_SORT_CATG);
          break;
        default:
          pLoc->libSetSorting(LS_SORT_NAME);
          break;
        }
        this->_buildLvPkg(); //< rebuild ListView
        break;
      }
    }

    if(LOWORD(wParam) == IDC_LV_BAT) {

      // if thread is running we block all interaction
      if(this->_pkgInst_hth || this->_pkgUnin_hth)
        return false;

      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case NM_DBLCLK:
        this->_onBcRunBat();
        break;

      case LVN_ITEMCHANGED:
        this->_onLvBatSel();
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
    std::cout << "DEBUG => OmUiMgrMainLib::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

    switch(LOWORD(wParam))
    {

    case IDC_CB_LOC:
      if(HIWORD(wParam) == CBN_SELCHANGE) this->_onCbLocSel();
      break;

    case IDC_BC_INST:
      this->pkgInst();
      break;

    case IDC_BC_UNIN:
      this->pkgUnin();
      break;

    case IDC_BC_ABORT:
      this->_thread_abort = true;
      this->enableItem(IDC_BC_ABORT, false);
      break;

    case IDC_BC_RUN:
      this->_onBcRunBat();
      break;

    case IDC_BC_NEW:
      this->_onBcNewBat();
      break;

    case IDC_BC_EDI:
      this->_onBcEdiBat();
      break;

    case IDM_EDIT_PKG_INST:
      this->pkgInst();
      break;

    case IDM_EDIT_PKG_UINS:
      this->pkgUnin();
      break;

    case IDM_EDIT_PKG_CLNS:
      this->pkgClns();
      break;

    case IDM_EDIT_PKG_TRSH:
      this->pkgTrsh();
      break;

    case IDM_EDIT_PKG_OPEN:
      this->pkgOpen();
      break;

    case IDM_EDIT_PKG_EDIT:
      this->pkgEdit();
      break;

    case IDM_EDIT_PKG_INFO:
      this->pkgProp();
      break;

    }
  }

  return false;
}
