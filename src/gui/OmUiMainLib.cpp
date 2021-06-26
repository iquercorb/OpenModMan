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
#include "gui/OmUiMainLib.h"
#include "gui/OmUiPropPkg.h"
#include "gui/OmUiPropBat.h"
#include "gui/OmUiAddLoc.h"
#include "gui/OmUiAddBat.h"
#include "gui/OmUiMain.h"


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

/// \brief Custom "Package Uninstall Done" Message
///
/// Custom "Package Uninstall Done" window message to notify the dialog that the
/// running thread finished his job.
///
#define UWM_BATEXE_DONE       (WM_APP+3)

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainLib::OmUiMainLib(HINSTANCE hins) : OmDialog(hins),
  _dirMon_hth(nullptr),
  _dirMon_hev{0,0,0},
  _pkgInst_hth(nullptr),
  _pkgUnin_hth(nullptr),
  _batExe_hth(nullptr),
  _thread_abort(false),
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
OmUiMainLib::~OmUiMainLib()
{
  // stop Library folder changes monitoring
  this->_dirMon_stop();

  HFONT hFt;
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_SC_TITLE, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_PKTXT, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMainLib::id() const
{
  return IDD_MAIN_LIB;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // Location ComboBox
  this->enableItem(IDC_CB_LOC, !enable);
  // Packages ListView
  this->enableItem(IDC_LV_PKG, !enable);
  // Batches ListBox
  this->enableItem(IDC_LB_BAT, !enable);

  // Package Install and Uninstall buttons
  this->enableItem(IDC_BC_INST, !enable);
  this->enableItem(IDC_BC_UNIN, !enable);

  // Batch Buttons
  if(enable) {
    this->enableItem(IDC_BC_NEW, false);
    this->enableItem(IDC_BC_DEL, false);
    this->enableItem(IDC_BC_RUN, false);
  } else {
    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    this->enableItem(IDC_BC_NEW, (pMgr->ctxCur() != nullptr));
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
void OmUiMainLib::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  if(enable) {
    // force to unselect current location
    this->locSel(-1);
  } else {
    // rebuild Location ComboBox, this
    // will also select the default Location
    this->_buildCbLoc();

    // rebuild Batches ListBox
    this->_buildLbBat();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::locSel(int id)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::locSel " << id << "\n";
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

        // set Library Path EditText control
        this->setItemText(IDC_EC_INP01, pLoc->libDir());

      } else {

        wstring wrn = L"Configured Location's library folder \"";
        wrn += pLoc->bckDir()+L"\""; wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";

        Om_dialogBoxWarn(this->_hwnd, L"Library folder access error", wrn);

        // set Library Path EditText control
        this->setItemText(IDC_EC_INP01, L"<folder access error>");
      }

      // enable the "Edit > Location properties..." menu
      pUiMain->setPopupItem(1, 2, MF_ENABLED);

    } else {

      // set Library Path EditText control
      this->setItemText(IDC_EC_INP01, L"<no Location selected>");

      // disable the "Edit > Location properties..." menu
      pUiMain->setPopupItem(1, 2, MF_GRAYED);
    }
  }

  // refresh
  this->_buildLvPkg();

  // forces control to select item
  this->msgItem(IDC_CB_LOC, CB_SETCURSEL, id);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::pkgInst()
{
  this->_pkgInst_init();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::pkgUnin()
{
  this->_pkgUnin_init();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::pkgTogg()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx->locCur()) return;

  // Get ListView unique selection
  int lv_sel = -1;
  if(this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT) == 1)
    lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  if(lv_sel < 0)
    return;

  if(pCtx->locCur()->pkgGet(lv_sel)->hasBck()) {
    this->_pkgUnin_init();
  } else {
    this->_pkgInst_init();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::pkgProp()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx->locCur()) return;

  OmPackage* pPkg = nullptr;

  // Get ListView unique selection
  int lv_sel = -1;
  if(this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT) == 1)
    lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  if(lv_sel < 0)
    return;

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
void OmUiMainLib::pkgTrsh()
{
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

  if(sel_ls.size()) {

    msg = L"Move the selected packages to recycle bin ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete packages", msg))
      return;

    // freeze dialog so user cannot interact
    static_cast<OmUiMain*>(this->root())->freeze(true);

    OmPackage* pPkg;

    for(size_t i = 0; i < sel_ls.size(); ++i) {

      pPkg = sel_ls[i];

      if(pPkg->hasSrc()) {
        Om_moveToTrash(pPkg->srcPath());
      } else {
        msg =   L"The package \""+pPkg->ident()+L"\" ";
        msg +=  L"does not have source data. To remove it from list "
                L"restores its backup by uninstalling it.";
        Om_dialogBoxWarn(this->_hwnd, L"No package source", msg);
      }
    }

    // unfreeze dialog to allow user to interact
    static_cast<OmUiMain*>(this->root())->freeze(false);
  }

  // update package selection
  this->_onLvPkgSel();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::pkgOpen()
{
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
bool OmUiMainLib::_pkgProgressCb(void* ptr, size_t tot, size_t cur, const wchar_t* str)
{
  OmUiMainLib* self = reinterpret_cast<OmUiMainLib*>(ptr);

  self->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, tot));
  self->msgItem(IDC_PB_PKG, PBM_SETPOS, cur);

  return !self->_thread_abort;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_pkgInstLs(const vector<OmPackage*>& pkg_ls, bool silent)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  wstring msg;

  vector<OmPackage*> inst_ls; //< final install list
  vector<OmPackage*> over_ls; //< overlapping list
  vector<OmPackage*> deps_ls; //< extra install list
  vector<wstring> miss_ls;    //< missing dependencies lists

  // prepare package installation
  pLoc->pkgPrepareInst(inst_ls, over_ls, deps_ls, miss_ls, pkg_ls);

  // warn user for missing dependencies
  if(!silent && miss_ls.size() && pMgr->warnMissDeps()) {
    msg = L"One or more selected packages have missing dependencies, "
          L"The following packages are required but not available:\n";
    for(size_t k = 0; k < miss_ls.size(); ++k) msg+=L"\n  "+miss_ls[k];
    msg +=  L"\n\nDo you want to proceed installation anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Dependencies missing", msg))
      return;
  }

  // warn for additional installation
  if(!silent && deps_ls.size() && pMgr->warnExtraInst()) {
    msg = L"One or more selected packages have dependencies, "
          L"the following packages will also be installed:\n";
    for(size_t i = 0; i < deps_ls.size(); ++i) msg += L"\n "+deps_ls[i]->ident();
    msg +=  L"\n\nContinue installation ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Packages dependencies", msg))
      return;
  }

  // if there is overlapping, ask user if he really want to continue installation
  if(!silent && over_ls.size() && pMgr->warnOverlaps()) {
    msg = L"One or more selected packages overlaps and will overwrites "
          L"files previously installed by the following package(s):\n";
    for(size_t j = 0; j < over_ls.size(); ++j) msg += L"\n "+over_ls[j]->ident();
    msg +=  L"\n\nDo you want to continue installation anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Packages overlaps", msg))
      return;
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
    lvi.iImage =  4; //< WIP
    this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));

    if(!pPkg->hasSrc() || pPkg->hasBck())
      continue;

    // we check overlapping before installation, we must do it step by step
    // because overlapping are cumulative with previously installed packages
    ovlp_ls.clear();
    pLoc->pkgFindOverlaps(ovlp_ls, pPkg);

    // install package
    //if(!pPkg->install(pLoc->bckZipLevel(), this->getItem(IDC_PB_PKG), &this->_thread_abort)) {
    if(!pPkg->install(pLoc->bckZipLevel(), &this->_pkgProgressCb, this)) {
      msg = L"The package \"" + pPkg->name() + L"\" ";
      msg += L"has not been installed because the following error occurred:\n\n";
      msg += pPkg->lastError();
      Om_dialogBoxErr(this->_hwnd, L"Package install failed", msg);
    }

    // update package icon in ListView
    if(pPkg->hasBck()) {
      lvi.iImage = 5; //< BCK
      this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      // update icons for overlapped packages
      for(size_t j = 0; j < ovlp_ls.size(); ++j) {
        lvi.iItem = pLoc->pkgIndex(ovlp_ls[j]);
        lvi.iImage = 6; //< OWR
        this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      }
    } else {
      lvi.iImage = -1; //< not installed
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
void OmUiMainLib::_pkgUninLs(const vector<OmPackage*>& pkg_ls, bool silent)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  wstring msg;

  vector<OmPackage*> over_ls;
  vector<OmPackage*> deps_ls;
  vector<OmPackage*> unin_ls;

  // prepare packages uninstall and backups restoration
  pLoc->bckPrepareUnin(unin_ls, over_ls, deps_ls, pkg_ls);

  // check and warn for extra uninstall due to overlaps
  if(!silent && over_ls.size() && pMgr->warnExtraUnin()) {
    msg = L"One or more selected packages are overlapped by others later "
          L"installed, the following packages must also be uninstalled:\n";
    for(size_t i = 0; i < over_ls.size(); ++i) msg += L"\n "+over_ls[i]->ident();
    msg += L"\n\nDo you want to continue anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Packages overlaps", msg))
      return;
  }

  // check and warn for extra uninstall due to dependencies
  if(!silent && deps_ls.size() && pMgr->warnExtraUnin()) {
    msg = L"One or more selected packages are required as dependency "
          L"by others, the following packages will also be uninstalled:\n";
    for(size_t i = 0; i < deps_ls.size(); ++i) msg += L"\n "+deps_ls[i]->ident();
    msg += L"\n\nDo you want to continue anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Packages dependencies", msg))
      return;
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
    lvi.iImage = 4; //< WIP
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
      msg =  L"The backup of \"" + pPkg->name() + L"\" ";
      msg += L"has not been properly restored because the following error occurred:\n\n";
      msg += pPkg->lastError();
      Om_dialogBoxErr(this->_hwnd, L"Package uninstall failed", msg);
    }

    if(pPkg->hasBck()) { //< this mean something went wrong
      lvi.iImage = pLoc->bckOverlapped(pPkg) ? 6 /*OWR*/ : 5 /*BCK*/;
      this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
    } else {
      lvi.iImage = -1; //< not installed
      this->msgItem(IDC_LV_PKG, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
      // update status icon for overlapped packages
      for(size_t j = 0; j < ovlp_ls.size(); ++j) {
        lvi.iItem = pLoc->pkgIndex(ovlp_ls[j]);
        lvi.iImage = pLoc->bckOverlapped(ovlp_ls[j]) ? 6 /*OWR*/ : 5 /*BCK*/;
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
void OmUiMainLib::_buildCbLoc()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::_buildCbLoc\n";
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
void OmUiMainLib::_buildLvPkg()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::_buildLvPkg\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  if(!pLoc) {
    // empty ListView
    this->msgItem(IDC_LV_PKG, LVM_DELETEALLITEMS);
    // disable ListView
    this->enableItem(IDC_LV_PKG, false);
    // return now
    return;
  }

  // if icon size changed, create new ImageList
  if(this->_buildLvPkg_icSize != pMgr->iconsSize()) {

    // Build list of images resource ID for the required size
    unsigned idb[7];
    switch(pMgr->iconsSize())
    {
    case 16:
      idb[0] = IDB_PKG_ERR_16; idb[1] = IDB_PKG_DIR_16; idb[2] = IDB_PKG_ZIP_16;
      idb[3] = IDB_PKG_DPN_16; idb[4] = IDB_PKG_WIP_16; idb[5] = IDB_PKG_BCK_16;
      idb[6] = IDB_PKG_OWR_16;
      break;
    case 32:
      idb[0] = IDB_PKG_ERR_32; idb[1] = IDB_PKG_DIR_32; idb[2] = IDB_PKG_ZIP_32;
      idb[3] = IDB_PKG_DPN_32; idb[4] = IDB_PKG_WIP_32; idb[5] = IDB_PKG_BCK_32;
      idb[6] = IDB_PKG_OWR_32;
      break;
    default:
      idb[0] = IDB_PKG_ERR_24; idb[1] = IDB_PKG_DIR_24; idb[2] = IDB_PKG_ZIP_24;
      idb[3] = IDB_PKG_DPN_24; idb[4] = IDB_PKG_WIP_24; idb[5] = IDB_PKG_BCK_24;
      idb[6] = IDB_PKG_OWR_24;
      break;
    }

    // Create ImageList and fill it with bitmaps
    HIMAGELIST hImgList = ImageList_Create(pMgr->iconsSize(), pMgr->iconsSize(), ILC_COLOR32, 7, 0 );
    for(unsigned i = 0; i < 7; ++i)
      ImageList_Add(hImgList, Om_getResImage(this->_hins, idb[i]), nullptr);

    // Set ImageList to ListView
    this->msgItem(IDC_LV_PKG, LVM_SETIMAGELIST, LVSIL_SMALL, reinterpret_cast<LPARAM>(hImgList));
    this->msgItem(IDC_LV_PKG, LVM_SETIMAGELIST, LVSIL_NORMAL, reinterpret_cast<LPARAM>(hImgList));
    DeleteObject(hImgList);

    // update size
    this->_buildLvPkg_icSize = pMgr->iconsSize();
  }

  // save current legacy support status
  this->_buildLvPkg_legacy = pMgr->legacySupport();

  // return now if library folder cannot be accessed
  if(!pLoc->checkAccessLib()) {
    return;
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_PKG, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty list view
  this->msgItem(IDC_LV_PKG, LVM_DELETEALLITEMS);

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
      lvItem.iImage = pLoc->bckOverlapped(pPkg) ? 6/*OWR*/ : 5/*BCK*/;
    } else {
      lvItem.iImage = -1; // none
    }
    lvItem.iItem = this->msgItem(IDC_LV_PKG, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Second column, the package name and type, here we set the sub-item
    lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
    lvItem.iSubItem = 1;
    if(pPkg->isType(PKG_TYPE_SRC)) {
      lvItem.iImage = pPkg->isZip() ? (pPkg->depCount() ? 3/*DPN*/ : 2/*ZIP*/) : 1/*DIR*/;
    } else {
      lvItem.iImage = 0; // IDB_PKG_ERR
    }

    lvItem.pszText = const_cast<LPWSTR>(pPkg->name().c_str());
    this->msgItem(IDC_LV_PKG, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

    // Third column, the package version, we set the sub-item
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 2;
    lvItem.pszText = const_cast<LPWSTR>(pPkg->version().asString().c_str());
    this->msgItem(IDC_LV_PKG, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
  }

  // we enable the List-View
  this->enableItem(IDC_LV_PKG, true);

  // restore list-view scroll position from lvRec
  this->msgItem(IDC_LV_PKG, LVM_SCROLL, 0, -lvRec.top );

  // update Package ListView selection
  this->_onLvPkgSel();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_buildLbBat()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // empty List-Box
  this->msgItem(IDC_LB_BAT, LB_RESETCONTENT);

  if(pCtx) {

    OmBatch* pBat;

    for(unsigned i = 0; i < pCtx->batCount(); ++i) {

      pBat = pCtx->batGet(i);

      this->msgItem(IDC_LB_BAT, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(pBat->title().c_str()));
      this->msgItem(IDC_LB_BAT, LB_SETITEMDATA, i, i); // for Location index reordering
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_pkgInst_init()
{
  // Freezes the main dialog to prevent user to interact during process
  static_cast<OmUiMain*>(this->root())->freeze(true);

  DWORD dwId;
  this->_pkgInst_hth = CreateThread(nullptr, 0, this->_pkgInst_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_pkgInst_stop()
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
  static_cast<OmUiMain*>(this->root())->freeze(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMainLib::_pkgInst_fth(void* arg)
{
  OmUiMainLib* self = static_cast<OmUiMainLib*>(arg);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx)return 1;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc)return 1;

  // string for dialog messages
  wstring msg;

  // checks whether we have a valid Destination folder
  if(!pLoc->checkAccessDst()) {
    msg = L"Destination folder \""+pLoc->dstDir()+L"\""; msg += OMM_STR_ERR_DIRACCESS;
    Om_dialogBoxErr(self->_hwnd, L"Package(s) install aborted", msg);
    return 1;
  }
  // checks whether we have a valid Library folder
  if(!pLoc->checkAccessLib()) {
    msg = L"Library folder \""+pLoc->libDir()+L"\""; msg += OMM_STR_ERR_DIRACCESS;
    Om_dialogBoxErr(self->_hwnd, L"Package(s) install aborted", msg);
    return 1;
  }
  // checks whether we have a valid Backup folder
  if(!pLoc->checkAccessBck()) {
    msg = L"Backup folder \""+pLoc->bckDir()+L"\""; msg += OMM_STR_ERR_DIRACCESS;
    Om_dialogBoxErr(self->_hwnd, L"Package(s) install aborted", msg);
    return 1;
  }

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
void OmUiMainLib::_pkgUnin_init()
{
  // freeze dialog so user cannot interact
  static_cast<OmUiMain*>(this->root())->freeze(true);

  DWORD dwId;
  this->_pkgUnin_hth = CreateThread(nullptr, 0, this->_pkgUnin_fth, this, 0, &dwId);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_pkgUnin_stop()
{
  DWORD exitCode;

  if(this->_pkgUnin_hth) {
    WaitForSingleObject(this->_pkgUnin_hth, INFINITE);
    GetExitCodeThread(this->_pkgUnin_hth, &exitCode);
    CloseHandle(this->_pkgUnin_hth);
    this->_pkgUnin_hth = nullptr;
  }

  // unfreeze dialog to allow user to interact again
  static_cast<OmUiMain*>(this->root())->freeze(false);

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
DWORD WINAPI OmUiMainLib::_pkgUnin_fth(void* arg)
{
  OmUiMainLib* self = static_cast<OmUiMainLib*>(arg);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx)return 1;
  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc)return 1;

  // string for dialog messages
  wstring msg;

  // checks whether we have a valid Destination folder
  if(!pLoc->checkAccessDst()) {
    msg = L"Destination folder \""+pLoc->dstDir()+L"\""; msg += OMM_STR_ERR_DIRACCESS;
    Om_dialogBoxErr(self->_hwnd, L"Package(s) uninstall aborted", msg);
    return 1;
  }
  // checks whether we have a valid Backup folder
  if(!pLoc->checkAccessBck()) {
    msg = L"Backup folder \""+pLoc->bckDir()+L"\""; msg += OMM_STR_ERR_DIRACCESS;
    Om_dialogBoxErr(self->_hwnd, L"Package(s) uninstall aborted", msg);
    return 1;
  }

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
void OmUiMainLib::_batExe_init()
{
  // freeze dialog so user cannot interact
  static_cast<OmUiMain*>(this->root())->freeze(true);

  DWORD dwId;
  this->_batExe_hth = CreateThread(nullptr, 0, this->_batExe_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_batExe_stop()
{
  DWORD exitCode;

  if(this->_batExe_hth) {
    WaitForSingleObject(this->_batExe_hth, INFINITE);
    GetExitCodeThread(this->_batExe_hth, &exitCode);
    CloseHandle(this->_batExe_hth);
    this->_batExe_hth = nullptr;
  }

  // unfreeze dialog to allow user to interact again
  static_cast<OmUiMain*>(this->root())->freeze(false);

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
DWORD WINAPI OmUiMainLib::_batExe_fth(void* arg)
{
  OmUiMainLib* self = static_cast<OmUiMainLib*>(arg);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // save current selected location
  int cb_sel = self->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  // save current select batch
  int lb_sel = self->msgItem(IDC_LB_BAT, LB_GETCURSEL);

  if(lb_sel >= 0) {

    // hide package details
    ShowWindow(self->getItem(IDC_SB_PKG), false);
    ShowWindow(self->getItem(IDC_EC_PKTXT), false);
    ShowWindow(self->getItem(IDC_SC_TITLE), false);

    // retrieve the batch object from current selection
    OmBatch* pBat = pCtx->batGet(self->msgItem(IDC_LB_BAT, LB_GETITEMDATA, lb_sel));

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
        self->_pkgUninLs(unin_ls, pMgr->quietBatches());
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
          self->_pkgInstLs(inst, pMgr->quietBatches());
        }
      }
    }

    // restore package details
    ShowWindow(self->getItem(IDC_SB_PKG), true);
    ShowWindow(self->getItem(IDC_EC_PKTXT), true);
    ShowWindow(self->getItem(IDC_SC_TITLE), true);
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
void OmUiMainLib::_dirMon_init(const wstring& path)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::_dirMon_init\n";
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
void OmUiMainLib::_dirMon_stop()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::_dirMon_stop\n";
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
DWORD WINAPI OmUiMainLib::_dirMon_fth(void* arg)
{
  OmUiMainLib* self = static_cast<OmUiMainLib*>(arg);

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
void OmUiMainLib::_onCbLocSel()
{
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  this->locSel(cb_sel);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onLvPkgRclk()
{
  // get handle to "Edit > Packages..." sub-menu
  HMENU hMenu = static_cast<OmUiMain*>(this->_parent)->getPopupItem(1, 5);

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  TrackPopupMenu(hMenu, TPM_TOPALIGN|TPM_LEFTALIGN, pt.x, pt.y, 0, this->_hwnd, nullptr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onLvPkgSel()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  // keep handle to main dialog
  OmUiMain* pUiMain = static_cast<OmUiMain*>(this->_parent);

  // disable "Edit > Package" in main menu
  pUiMain->setPopupItem(1, 5, MF_GRAYED);

  // Handle to bitmap for package picture
  HBITMAP hBm = Om_getResImage(this->_hins, IDB_PKG_THN);

  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_PKG, LVM_GETSELECTEDCOUNT);

  if(lv_nsl > 0) {

    // at least one, we enable buttons
    this->enableItem(IDC_BC_INST, true);
    this->enableItem(IDC_BC_UNIN, true);

    // enable "Edit > Package []" pop-up menu
    pUiMain->setPopupItem(1, 5, MF_ENABLED);

    // show package title and thumbnail
    ShowWindow(this->getItem(IDC_SC_TITLE), true);
    ShowWindow(this->getItem(IDC_SB_PKG), true);

    if(lv_nsl > 1) {

      // disable the "Edit > Package > View detail..." menu-item
      HMENU hPopup = pUiMain->getPopupItem(1, 5);
      pUiMain->setPopupItem(hPopup, 6, MF_GRAYED); //< "View detail..." menu-item

      // on multiple selection, we hide package description
      ShowWindow(this->getItem(IDC_EC_PKTXT), false);
      this->setItemText(IDC_SC_TITLE, L"<Multiple selection>");

    } else {

      // enable the "Edit > Package > .. " menu-item
      HMENU hPopup = pUiMain->getPopupItem(1, 5);
      pUiMain->setPopupItem(hPopup, 6, MF_ENABLED); //< "View details" menu-item

      // show package description
      ShowWindow(this->getItem(IDC_EC_PKTXT), true);

      OmPackage* pPkg;

      // get the selected item id (only one, no need to iterate)
      int lv_sel = this->msgItem(IDC_LV_PKG, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
      if(lv_sel >= 0) {

        pPkg = pLoc->pkgGet(lv_sel);

        this->setItemText(IDC_SC_TITLE, pPkg->name());

        if(pPkg->desc().size()) {
          this->setItemText(IDC_EC_PKTXT, pPkg->desc());
        } else {
          this->setItemText(IDC_EC_PKTXT, L"<no description available>");
        }

        if(pPkg->image().thumbnail()) {
          hBm = pPkg->image().thumbnail();
        }
      }
    }

  } else {

    // disable "Edit > Package []" pop-up menu
    pUiMain->setPopupItem(1, 5, MF_GRAYED);

    // hide all package bottom infos
    ShowWindow(this->getItem(IDC_SC_TITLE), false);
    ShowWindow(this->getItem(IDC_SB_PKG), false);
    ShowWindow(this->getItem(IDC_EC_PKTXT), false);

    this->enableItem(IDC_BC_INST, false);
    this->enableItem(IDC_BC_UNIN, false);
  }

  // Update the selected picture
  hBm = this->setStImage(IDC_SB_PKG, hBm);
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);

  // force thumbnail static control to update its position
  this->_setItemPos(IDC_SB_PKG, 5, this->height()-83, 85, 78);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onLbBatSel()
{
  int lb_sel = this->msgItem(IDC_LB_BAT, LB_GETCURSEL);

  this->enableItem(IDC_BC_RUN, (lb_sel >= 0));
  this->enableItem(IDC_BC_EDI, (lb_sel >= 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onBcRunBat()
{
  this->_batExe_init();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onBcNewBat()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();


  OmUiAddBat* pUiNewBat = static_cast<OmUiAddBat*>(this->siblingById(IDD_ADD_BAT));
  pUiNewBat->ctxSet(pCtx);
  pUiNewBat->open(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onBcEdiBat()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  int lb_sel = this->msgItem(IDC_LB_BAT, LB_GETCURSEL);

  if(lb_sel >= 0) {

    int bat_id = this->msgItem(IDC_LB_BAT, LB_GETITEMDATA, lb_sel);

    OmUiPropBat* pUiPropBat = static_cast<OmUiPropBat*>(this->siblingById(IDD_PROP_BAT));
    pUiPropBat->batSet(pCtx->batGet(bat_id));
    pUiPropBat->open();
  }

  // reload the batch list-box
  this->_buildLbBat();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::_onInit\n";
  #endif

  // Defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(18, 800, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(14, 700, L"Consolas");
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  // Set batches New and Delete buttons icons
  this->setBmImage(IDC_BC_NEW, Om_getResImage(this->_hins, IDB_BTN_ADD));
  this->setBmImage(IDC_BC_EDI, Om_getResImage(this->_hins, IDB_BTN_MOD));
  this->setStImage(IDC_SB_PKG, Om_getResImage(this->_hins, IDB_PKG_THN));

  // define controls tool-tips
  this->_createTooltip(IDC_CB_LOC,  L"Select active location");
  this->_createTooltip(IDC_BC_INST,   L"Install selected package(s)");
  this->_createTooltip(IDC_BC_UNIN,   L"Uninstall selected package(s)");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort current process");

  // Initialize the ListView control
  DWORD dwExStyle = LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_DOUBLEBUFFER;

  this->msgItem(IDC_LV_PKG, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwExStyle);

  // we now add columns into our list-view control
  LVCOLUMNW lvCol;
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
  lvCol.cx = 550;
  lvCol.iSubItem = 1;
  this->msgItem(IDC_LV_PKG, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Version");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem = 2;
  this->msgItem(IDC_LV_PKG, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&lvCol));

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // keep curent manager legacy option
  this->_buildLvPkg_legacy = pMgr->legacySupport();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::_onShow\n";
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

    // restart folder monitoring
    if(!this->_dirMon_hth) {
      this->_dirMon_init(pCtx->locCur()->libDir());
    }

    // refresh package ListView
    this->_buildLvPkg();
  }

  // disable "Edit > Remote" in main menu
  static_cast<OmUiMain*>(this->root())->setPopupItem(1, 6, MF_GRAYED);

  // refresh dialog
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onHide()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::_onHide\n";
  #endif

  // stop folder monitoring
  if(this->_dirMon_hth)
    this->_dirMon_stop();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onResize()
{
  // Locations Combo-Box
  this->_setItemPos(IDC_CB_LOC, 5, 5, this->width()-161, 12);
  // Library path EditControl
  this->_setItemPos(IDC_EC_INP01, 5, 20, this->width()-161, 12);
  // Package List ListView
  this->_setItemPos(IDC_LV_PKG, 5, 35, this->width()-161, this->height()-151);
  // Resize the ListView column
  LONG size[4];
  GetClientRect(this->getItem(IDC_LV_PKG), reinterpret_cast<LPRECT>(&size));
  this->msgItem(IDC_LV_PKG, LVM_SETCOLUMNWIDTH, 1, size[2]-145);

  // Install and Uninstall buttons
  this->_setItemPos(IDC_BC_INST, 5, this->height()-114, 50, 14);
  this->_setItemPos(IDC_BC_UNIN, 55, this->height()-114, 50, 14);
  // Progress bar
  this->_setItemPos(IDC_PB_PKG, 107, this->height()-113, this->width()-315, 12);
  // Abort button
  this->_setItemPos(IDC_BC_ABORT, this->width()-205, this->height()-114, 50, 14);
  // Package name/title
  this->_setItemPos(IDC_SC_TITLE, 5, this->height()-96, this->width()-161, 12);
  // Package snapshot
  this->_setItemPos(IDC_SB_PKG, 5, this->height()-83, 85, 78);
  // Package description
  this->_setItemPos(IDC_EC_PKTXT, 95, this->height()-83, this->width()-101, 78);

  // Vertical separator
  this->_setItemPos(IDC_SC_SEPAR, this->width()-150, 5, 1, this->height()-105);

  // Batches label
  this->_setItemPos(IDC_SC_LBL01, this->width()-143, 8, 136, 12);
  // Batches List-Box
  this->_setItemPos(IDC_LB_BAT, this->width()-143, 20, 136, this->height()-137);
  // Batches Apply, New.. and Delete buttons
  this->_setItemPos(IDC_BC_RUN, this->width()-143, this->height()-114, 45, 14);
  this->_setItemPos(IDC_BC_NEW, this->width()-97, this->height()-114, 45, 14);
  this->_setItemPos(IDC_BC_EDI, this->width()-51, this->height()-114, 45, 14);


  InvalidateRect(this->_hwnd, nullptr, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::_onRefresh\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  //OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  // disable all packages buttons
  this->enableItem(IDC_BC_ABORT, false);
  this->enableItem(IDC_BC_INST, false);
  this->enableItem(IDC_BC_UNIN, false);

  // hide package details
  ShowWindow(this->getItem(IDC_SC_TITLE), false);
  ShowWindow(this->getItem(IDC_EC_PKTXT), false);
  ShowWindow(this->getItem(IDC_SB_PKG), false);

  // disable the Progress-Bar
  this->enableItem(IDC_PB_PKG, false);

  // rebuild Location ComboBox
  this->_buildCbLoc();

  // if icon size changed, rebuild Package ListView
  if(this->_buildLvPkg_icSize != pMgr->iconsSize()) {
    this->_buildLvPkg();
  }

  // if legacy support changed, rebuild Package ListView
  if(this->_buildLvPkg_legacy != pMgr->legacySupport()) {
    this->_buildLvPkg();
  }

  // disable all batches buttons
  this->enableItem(IDC_BC_RUN, false);
  this->enableItem(IDC_BC_NEW, (pCtx != nullptr));
  this->enableItem(IDC_BC_EDI, false);

  // rebuild Batches ListBox
  this->_buildLbBat();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMainLib::_onQuit\n";
  #endif

  // stop Library folder changes monitoring
  this->_dirMon_stop();

  // safely and cleanly close threads handles
  if(this->_pkgInst_hth) {
    WaitForSingleObject(this->_pkgInst_hth, INFINITE);
    CloseHandle(this->_pkgInst_hth);
    this->_pkgInst_hth = nullptr;
  }
  if(this->_pkgUnin_hth) {
    WaitForSingleObject(this->_pkgUnin_hth, INFINITE);
    CloseHandle(this->_pkgUnin_hth);
    this->_pkgUnin_hth = nullptr;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMainLib::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

  if(uMsg == WM_NOTIFY) {

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->ctxCur();

    if(!pCtx->locCur())
      return false;

    OmLocation* pLoc = pCtx->locCur();

    NMHDR* pNmhdr = reinterpret_cast<NMHDR*>(lParam);

    if(LOWORD(wParam) == IDC_LV_PKG) {

      // if thread is running we block all interaction
      if(this->_pkgInst_hth || this->_pkgUnin_hth)
        return false;

      if(pNmhdr->code == NM_DBLCLK) {
        this->pkgTogg();
        return false;
      }

      if(pNmhdr->code == LVN_ITEMCHANGED) {
        // update package(s) selection
        this->_onLvPkgSel();
        return false;
      }

      if(pNmhdr->code == NM_RCLICK) {
        // Open the popup menu
        this->_onLvPkgRclk();
        return false;
      }

      if(pNmhdr->code == LVN_COLUMNCLICK) {

        NMLISTVIEW* pNmlv = reinterpret_cast<NMLISTVIEW*>(lParam);

        switch(pNmlv->iSubItem)
        {
        case 0:
          pLoc->libSetSorting(LS_SORT_STAT);
          break;
        case 2:
          pLoc->libSetSorting(LS_SORT_VERS);
          break;
        default:
          pLoc->libSetSorting(LS_SORT_NAME);
          break;
        }

        // rebuild package ListView
        this->_buildLvPkg();

        return false;
      }
    }
  }

  if(uMsg == WM_COMMAND) {

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->ctxCur();

    if(!pCtx) return false;

    switch(LOWORD(wParam))
    {

    case IDC_CB_LOC:
      if(HIWORD(wParam) == CBN_SELCHANGE) this->_onCbLocSel();
      break;

    case IDC_LB_BAT: //< Location(s) list List-Box
      if(HIWORD(wParam) == LBN_SELCHANGE) this->_onLbBatSel();
      if(HIWORD(wParam) == LBN_DBLCLK) this->_onBcRunBat();
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

    case IDM_EDIT_PKG_TRSH:
      this->pkgTrsh();
      break;

    case IDM_EDIT_PKG_OPEN:
      this->pkgOpen();
      break;

    case IDM_EDIT_PKG_INFO:
      this->pkgProp();
      break;

    }
  }

  return false;
}
