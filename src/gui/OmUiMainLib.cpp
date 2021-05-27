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
#include "gui/OmUiNewLoc.h"
#include "gui/OmUiNewBat.h"
#include "gui/OmUiMain.h"


/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the previously
/// started thread ended his job
///
#define UWM_PACKAGES_DONE     (WM_APP+1)


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainLib::OmUiMainLib(HINSTANCE hins) : OmDialog(hins),
  _onProcess(false),
  _lvIconsSize(0),
  _hFtTitle(Om_createFont(18, 800, L"Ms Shell Dlg")),
  _hFtMonos(Om_createFont(14, 700, L"Consolas")),
  _hBmBlank(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_PKG_BLANK),0,0,0,0))),
  _hBmBcNew(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_BTN_ADD),0,0,0,0))),
  _hBmBcDel(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_BTN_REM),0,0,0,0))),
  _hBmBcMod(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_BTN_MOD),0,0,0,0))),
  _abortPending(false),
  _install_hth(nullptr),
  _uninstall_hth(nullptr),
  _batch_hth(nullptr),
  _monitor_hth(nullptr)
{
  // Package info sub-dialog
  this->addChild(new OmUiPropPkg(hins));

  // set the accelerator table for the dialog
  this->setAccelerator(IDR_ACCEL);

  // elements for real-time directory monitoring thread
  this->_monitor_hev[0] = CreateEvent(nullptr, true, false, nullptr); //< custom event to notify thread must exit
  this->_monitor_hev[1] = nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainLib::~OmUiMainLib()
{
  // stop Library folder changes monitoring
  this->_monitor_stop();

  DeleteObject(this->_hBmBlank);
  DeleteObject(this->_hBmBcNew);
  DeleteObject(this->_hBmBcDel);
  DeleteObject(this->_hBmBcMod);
  DeleteObject(this->_hFtTitle);
  DeleteObject(this->_hFtMonos);
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
void OmUiMainLib::selLocation(int i)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // stop Library folder monitoring
  this->_monitor_stop();

  // main window dialog
  OmUiMain* pUiMain = static_cast<OmUiMain*>(this->_parent);

  // select the requested Location
  if(pMgr->curContext()) {

    OmContext* pCtx = pMgr->curContext();

    pCtx->selLocation(i);

    if(pCtx->curLocation()) {

      OmLocation* pLoc = pCtx->curLocation();

      if(!pLoc->checkAccessDst()) {
        wstring wrn = L"Configured Location's destination folder \"";
        wrn += pLoc->installDir()+L"\""; wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";
        Om_dialogBoxWarn(this->_hwnd, L"Destination folder access error", wrn);
      }

      if(!pLoc->checkAccessBck()) {
        wstring wrn = L"Configured Location's backup folder \"";
        wrn += pLoc->backupDir()+L"\""; wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";
        Om_dialogBoxWarn(this->_hwnd, L"Backup folder access error", wrn);
      }

      if(pLoc->checkAccessLib()) {
        // start Library folder monitoring
        this->_monitor_init(pLoc->libraryDir());
      } else {
        wstring wrn = L"Configured Location's library folder \"";
        wrn += pLoc->backupDir()+L"\""; wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";
        Om_dialogBoxWarn(this->_hwnd, L"Library folder access error", wrn);
      }

      // enable the "Edit > Location properties..." menu
      pUiMain->setMenuEdit(2, MF_BYPOSITION|MF_ENABLED);

    } else {
      // disable the "Edit > Location properties..." menu
      pUiMain->setMenuEdit(2, MF_BYPOSITION|MF_GRAYED);
    }
  }

  // disable "Edit > Package" in main menu
  pUiMain->setMenuEdit(5, MF_BYPOSITION|MF_GRAYED);
  // disable the "Edit > Package > []" elements
  HMENU hMenu = pUiMain->getMenuEdit(5);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_INST, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_UINS, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_OPEN, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_TRSH, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, MF_GRAYED);

  // refresh
  this->_reloadLibEc();
  this->_reloadLibLv();

  // forces control to select item
  HWND hCb = this->getItem(IDC_CB_LOCLS);

  if(i != SendMessageW(hCb, CB_GETCURSEL, 0, 0))
    SendMessageW(hCb, CB_SETCURSEL, i, 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::toggle()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->curContext()->curLocation();

  HWND hLv = this->getItem(IDC_LV_PKGLS);

  DWORD dwid;
  unsigned n = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);
  for(unsigned i = 0; i < n; ++i) {
    if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
      // enable the On-Process state of parent window
      static_cast<OmUiMain*>(this->_parent)->setOnProcess(true);

      this->_abortPending = false;

      if(pLoc->package(i)->hasBackup()) {
        this->_uninstall_hth = CreateThread(nullptr, 0, this->_uninstall_fth, this, 0, &dwid);
      } else {
        this->_install_hth = CreateThread(nullptr, 0, this->_install_fth, this, 0, &dwid);
      }
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::install()
{
  DWORD dwId;
  this->_abortPending = false;

  // enable the On-Process state of parent window
  static_cast<OmUiMain*>(this->_parent)->setOnProcess(true);

  this->_install_hth = CreateThread(nullptr, 0, this->_install_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::uninstall()
{
  DWORD dwId;
  this->_abortPending = false;

  // enable the On-Process state of parent window
  static_cast<OmUiMain*>(this->_parent)->setOnProcess(true);

  this->_uninstall_hth = CreateThread(nullptr, 0, this->_uninstall_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::viewDetails()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->curContext()->curLocation();
  OmPackage* pPkg = nullptr;

  HWND hLv = this->getItem(IDC_LV_PKGLS);

  unsigned lv_cnt = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);
  for(unsigned i = 0; i < lv_cnt; ++i) {
    if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
      pPkg = pLoc->package(i);
      break;
    }
  }

  if(pPkg) {
    OmUiPropPkg* pUiPropPkg = static_cast<OmUiPropPkg*>(this->childById(IDD_PROP_PKG));
    pUiPropPkg->setPackage(pPkg);
    pUiPropPkg->open(true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::moveTrash()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->curContext()->curLocation();

  vector<OmPackage*> trash_list;

  HWND hLv = this->getItem(IDC_LV_PKGLS);

  this->setOnProcess(true);

  unsigned n = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);
  for(unsigned i = 0; i < n; ++i) {
    if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
      trash_list.push_back(pLoc->package(i));
    }
  }

  // Unselect all items
  LVITEM lvI = {};
  lvI.mask = LVIF_STATE;
  lvI.stateMask = LVIS_SELECTED;
  SendMessageW(hLv, LVM_SETITEMSTATE, -1, reinterpret_cast<LPARAM>(&lvI));

  if(trash_list.size()) {
    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete package(s)",
                                            L"Move the selected package(s) to trash ?"))
      return;

    for(size_t i = 0; i < trash_list.size(); ++i) {
      Om_moveToTrash(trash_list[i]->sourcePath());
    }
  }

  this->setOnProcess(false);

  // update package selection
  this->_onSelectPkg();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::openExplore()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = pMgr->curContext()->curLocation();

  vector<OmPackage*> explo_list;

  HWND hLv = this->getItem(IDC_LV_PKGLS);

  unsigned n = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);
  for(unsigned i = 0; i < n; ++i) {
    if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
      explo_list.push_back(pLoc->package(i));
    }
  }

  for(size_t i = 0; i < explo_list.size(); ++i) {

    // the default behavior is to explore (open explorer with deployed folders)
    // however, it may happen that zip file are handled by an application
    // (typically, WinRar or 7zip) and the "explore" command may fail, in this
    // case, we call the "open" command.

    if(ShellExecuteW(this->_hwnd, L"explore", explo_list[i]->sourcePath().c_str(), nullptr, nullptr, SW_NORMAL ) <= (HINSTANCE)32) {
      ShellExecuteW(this->_hwnd, L"open", explo_list[i]->sourcePath().c_str(), nullptr, nullptr, SW_NORMAL );
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::batch()
{
  DWORD dwId;
  this->_abortPending = false;

  // enable the On-Process state of parent window
  static_cast<OmUiMain*>(this->_parent)->setOnProcess(true);

  this->_batch_hth = CreateThread(nullptr, 0, this->_batch_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMainLib::remBatch()
{
  HWND hLb = this->getItem(IDC_LB_BATLS);

  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  if(lb_sel >= 0) {

    unsigned bat_id = SendMessageW(hLb, LB_GETITEMDATA, lb_sel, 0);

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->curContext();

    // warns the user before committing the irreparable
    wstring qry = L"Are your sure you want to delete the Batch \"";
    qry += pCtx->batch(bat_id)->title();
    qry += L"\" ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Batch", qry)) {
      return false;
    }

    if(!pCtx->remBatch(bat_id)) {
      Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Batch failed", pCtx->lastError());
      return false;
    }
  }

  // reload the batch list-box
  this->_reloadBatLb();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMainLib::ediBatch()
{
  HWND hLb = this->getItem(IDC_LB_BATLS);

  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  if(lb_sel >= 0) {

    int bat_id = SendMessageW(hLb, LB_GETITEMDATA, lb_sel, 0);

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->curContext();

    OmUiPropBat* pUiPropBat = static_cast<OmUiPropBat*>(this->siblingById(IDD_PROP_BAT));
    pUiPropBat->setBatch(pCtx->batch(bat_id));
    pUiPropBat->open();
  }

  // reload the batch list-box
  this->_reloadBatLb();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::setOnProcess(bool enable)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // handle to "Edit > Package" sub-menu
  HMENU hMenu = static_cast<OmUiMain*>(this->_parent)->getMenuEdit(5);

  // enable/disable Location combo-box
  this->enableItem(IDC_CB_LOCLS, !enable);

  // enable/disable install and uninstall button
  this->enableItem(IDC_BC_INST, !enable);
  this->enableItem(IDC_BC_UNIN, !enable);
  // enable/disable the package list
  this->enableItem(IDC_LV_PKGLS, !enable);

  // disable/enable abort button
  this->enableItem(IDC_BC_ABORT, enable);

  // enable/disable the batches list
  this->enableItem(IDC_LB_BATLS, !enable);

  if(enable) {

    this->_onProcess = true;

    // disable batches buttons
    this->enableItem(IDC_BC_APPLY, false);
    this->enableItem(IDC_BC_NEW, false);
    this->enableItem(IDC_BC_DEL, false);

    EnableMenuItem(hMenu, IDM_EDIT_PKG_INST, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_UINS, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_OPEN, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_TRSH, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, MF_GRAYED);

  } else {

    OmContext* pCtx = pMgr->curContext();

    HWND hLv = this->getItem(IDC_LV_PKGLS);
    HWND hLb = this->getItem(IDC_LB_BATLS);

    this->enableItem(IDC_BC_NEW, (pCtx != nullptr));

    bool lb_has_sel = (SendMessageW(hLb, LB_GETCURSEL, 0, 0) >= 0);
    this->enableItem(IDC_BC_DEL, lb_has_sel);
    this->enableItem(IDC_BC_APPLY, lb_has_sel);

    // enable the "Edit > Package" sub-items
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INST, MF_ENABLED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_UINS, MF_ENABLED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_OPEN, MF_ENABLED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_TRSH, MF_ENABLED);

    bool lv_sing_sel = (SendMessageW(hLv, LVM_GETSELECTEDCOUNT, 0, 0) == 1);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, (lv_sing_sel) ? MF_ENABLED : MF_GRAYED);

    this->_onProcess = false;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onSelectPkg()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = (pMgr->curContext())?pMgr->curContext()->curLocation():nullptr;

  if(pLoc == nullptr)
    return;

  // keep handle to main dialog
  OmUiMain* pUiMain = static_cast<OmUiMain*>(this->_parent);

  // disable "Edit > Package" in main menu
  pUiMain->setMenuEdit(5, MF_BYPOSITION|MF_GRAYED);

  // handle to "Edit > Package" sub-menu
  HMENU hMenu = pUiMain->getMenuEdit(5);

  HWND hLv = this->getItem(IDC_LV_PKGLS);
  HWND hSb = this->getItem(IDC_SB_PKIMG);

  // Handle to bitmap for package picture
  HBITMAP hBm  = this->_hBmBlank;

  // get count of selected item
  unsigned lv_nsl = SendMessageW(hLv, LVM_GETSELECTEDCOUNT, 0, 0);

  if(lv_nsl > 0) {
    // at least one, we enable buttons
    this->enableItem(IDC_BC_INST, true);
    this->enableItem(IDC_BC_UNIN, true);

    // enable the "Edit > Package" sub-items
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INST, 0);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_UINS, 0);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_OPEN, 0);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_TRSH, 0);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, 0);

    // enable "Edit > Package" sub-menu
    pUiMain->setMenuEdit(5, MF_BYPOSITION|MF_ENABLED);

    if(lv_nsl > 1) {

      // multiple selection, we cannot display readme and snapshot
      ShowWindow(this->getItem(IDC_EC_PKTXT), false);

      // set title default message
      ShowWindow(this->getItem(IDC_SC_TITLE), true);
      this->setItemText(IDC_SC_TITLE, L"<Multiple selection>");

      // set default blank picture
      ShowWindow(hSb, true);

      // disable the "view detail..." sub-menu
      EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, MF_GRAYED);

    } else {

      // get the select item id
      unsigned itm_count = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);
      for(unsigned i = 0; i < itm_count; ++i) {

        if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {

          this->setItemText(IDC_SC_TITLE, pLoc->package(i)->name());

          if(pLoc->package(i)->desc().size()) {
            this->setItemText(IDC_EC_PKTXT, pLoc->package(i)->desc());
          } else {
            this->setItemText(IDC_EC_PKTXT, L"<no description available>");
          }

          if(pLoc->package(i)->picture()) {
            hBm = Om_getBitmapThumbnail(pLoc->package(i)->picture(), OMM_PKG_THMB_SIZE, OMM_PKG_THMB_SIZE);
          }

          ShowWindow(this->getItem(IDC_SC_TITLE), true);
          ShowWindow(hSb, true);
          ShowWindow(this->getItem(IDC_EC_PKTXT), true);
        }
      }
    }
  } else {
    // nothing selected, we disable all
    ShowWindow(this->getItem(IDC_SC_TITLE), false);
    ShowWindow(hSb, false);
    ShowWindow(this->getItem(IDC_EC_PKTXT), false);

    this->enableItem(IDC_BC_INST, false);
    this->enableItem(IDC_BC_UNIN, false);

    // disable the "Edit > Package" sub-items
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INST, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_UINS, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_OPEN, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_TRSH, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, MF_GRAYED);

    // disable "Edit > Package" sub-menu
    pUiMain->setMenuEdit(5, MF_BYPOSITION|MF_GRAYED);
  }

  // Update the selected picture
  hBm = reinterpret_cast<HBITMAP>(SendMessageW(hSb, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBm)));
  if(hBm != this->_hBmBlank) DeleteObject(hBm);

  this->_setItemPos(IDC_SB_PKIMG, 5, this->height()-83, 85, 78);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onSelectBat()
{
  int lb_sel = this->msgItem(IDC_LB_BATLS, LB_GETCURSEL);

  this->enableItem(IDC_BC_APPLY, (lb_sel >= 0));
  this->enableItem(IDC_BC_EDI, (lb_sel >= 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_reloadLibEc()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = (pMgr->curContext())?pMgr->curContext()->curLocation():nullptr;;

  if(pLoc != nullptr) {

    // check for Library folder validity
    if(pLoc->checkAccessLib()) {
      // set the library path
      this->setItemText(IDC_EC_INPT1, pLoc->libraryDir());
    } else {
      this->setItemText(IDC_EC_INPT1, L"<folder access error>");
    }

  } else {
    // empty library path
    this->setItemText(IDC_EC_INPT1, L"<no Location selected>");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_reloadLibLv(bool clear)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmLocation* pLoc = (pMgr->curContext())?pMgr->curContext()->curLocation():nullptr;

  // get List view control
  HWND hLv = this->getItem(IDC_LV_PKGLS);

  if(pLoc != nullptr) {

    // if icon size changed, reload
    if(this->_lvIconsSize != pMgr->iconsSize()) {
      this->_reloadIcons();
    }

    // return now if library folder cannot be accessed
    if(!pLoc->checkAccessLib()) {
      return;
    }

    // force Location library refresh
    if(clear) {
      pLoc->packageListClear(); //< clear to rebuild entirely
    }
    pLoc->packageListRefresh();

    // we enable the List-View
    EnableWindow(hLv, true);

    // Save list-view scroll position to lvRect
    RECT lvRec;
    SendMessageW(hLv, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

    // empty list view
    SendMessageW(hLv, LVM_DELETEALLITEMS, 0, 0);

    // add item to list view
    OmPackage* pPkg;
    LVITEMW lvItem;
    for(unsigned i = 0; i < pLoc->packageCount(); ++i) {

      pPkg = pLoc->package(i);

      // the first colum, package status, here we INSERT the new item
      lvItem.iItem = i;
      lvItem.mask = LVIF_IMAGE;
      lvItem.iSubItem = 0;
      if(pPkg->isType(PKG_TYPE_BCK)) {
        if(pLoc->isBakcupOverlapped(pPkg)) {
          lvItem.iImage = 6; // IDB_PKG_OWR
        } else {
          lvItem.iImage = 5; // IDB_PKG_BCK
        }
      } else {
        lvItem.iImage = -1; // none
      }
      lvItem.iItem = SendMessageW(hLv, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

      // Second column, the package name and type, here we set the subitem
      lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
      lvItem.iSubItem = 1;
      if(pPkg->isType(PKG_TYPE_SRC)) {
        if(pPkg->isType(PKG_TYPE_ZIP)) {
          if(pPkg->dependCount()) {
            lvItem.iImage = 3; // IDB_PKG_DPN
          } else {
            lvItem.iImage = 2; // IDB_PKG_ZIP
          }
        } else {
          lvItem.iImage = 1; // IDB_PKG_DIR
        }
      } else {
        lvItem.iImage = 0; // IDB_PKG_ERR
      }
      lvItem.pszText = const_cast<LPWSTR>(pLoc->package(i)->name().c_str());
      SendMessageW(hLv, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));

      // Third column, the package version, we set the subitem
      lvItem.mask = LVIF_TEXT;
      lvItem.iSubItem = 2;
      lvItem.pszText = const_cast<LPWSTR>(pLoc->package(i)->version().asString().c_str());
      SendMessageW(hLv, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
    }

    // restore list-view scroll position from lvmRect
    SendMessageW(hLv, LVM_SCROLL, 0, -lvRec.top );

  } else {
    // empty list view
    SendMessageW(hLv, LVM_DELETEALLITEMS, 0, 0);
    // disable the List-View
    EnableWindow(hLv, true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_reloadBatLb()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->curContext();

  HWND hLb = this->getItem(IDC_LB_BATLS);

  // empty List-Box
  SendMessageW(hLb, LB_RESETCONTENT, 0, 0);

  if(pCtx) {
    for(unsigned i = 0; i < pCtx->batchCount(); ++i) {
      SendMessageW(hLb, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(pCtx->batch(i)->title().c_str()));
      SendMessageW(hLb, LB_SETITEMDATA, i, i); // for Location index reordering
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_reloadLocCb()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->curContext();

  HWND hCb = this->getItem(IDC_CB_LOCLS);

  if(pCtx == nullptr) {
    // no Location, disable the List-Box
    EnableWindow(hCb, false);
    // unselect Location
    this->selLocation(-1);

    return;
  }

  // save current selection
  int cb_sel = SendMessageW(hCb, CB_GETCURSEL, 0, 0);

  // empty the Combo-Box
  SendMessageW(hCb, CB_RESETCONTENT, 0, 0);

  // add Context(s) to Combo-Box
  if(pCtx->locationCount()) {

    wstring label;

    EnableWindow(hCb, true);

    for(unsigned i = 0; i < pCtx->locationCount(); ++i) {

      label = pCtx->location(i)->title();
      label += L" - ";

      // checks whether installation destination path is valid
      if(pCtx->location(i)->checkAccessDst()) {
        label += pCtx->location(i)->installDir();
      } else {
        label += L"<folder access error>";

        wstring wrn = L"Configured Location's Destination folder \"";
        wrn += pCtx->location(i)->installDir()+L"\"";
        wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";
        Om_dialogBoxWarn(this->_hwnd, L"Destination folder access error", wrn);
      }

      SendMessageW(hCb, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(label.c_str()));
    }

    // select the the previously selected Context
    if(cb_sel >= 0) {
      SendMessageW(hCb, CB_SETCURSEL, cb_sel, 0);
      this->_reloadLibEc(); //< reload displayed library path
      this->_reloadLibLv(true); //< reload + reparse packages list
    } else {
      SendMessageW(hCb, CB_SETCURSEL, 0, 0);
      // select the first Location by default
      this->selLocation(0);
    }
  } else {
    // no Location, disable the List-Box
    EnableWindow(hCb, false);
    // unselect Location
    this->selLocation(-1);

    // if Context have no Location, we ask user to create at least one
    wstring qry = L"The current Context does not have any configured "
                  L"Location. A Context needs at least one Location.\n\n"
                  L"Do you want to configure a new Location now ?";

    if(Om_dialogBoxQuerry(this->_hwnd, L"No Location found", qry)) {
      OmUiNewLoc* pUiNewLoc = static_cast<OmUiNewLoc*>(this->siblingById(IDD_NEW_LOC));
      pUiNewLoc->setContext(pCtx);
      pUiNewLoc->open(true);
    }

  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_reloadIcons()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // update size
  this->_lvIconsSize = pMgr->iconsSize();

   // hold the HWND of our list view control */
  HWND hLv = this->getItem(IDC_LV_PKGLS);

  // We add an image list to the list-view control, the image list will
  // contain all icons we need.
  HBITMAP hBm[7];

  switch(this->_lvIconsSize)
  {
  case 16:
    hBm[0] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_ERR_16), IMAGE_BITMAP, 0, 0, 0);
    hBm[1] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_DIR_16), IMAGE_BITMAP, 0, 0, 0);
    hBm[2] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_ZIP_16), IMAGE_BITMAP, 0, 0, 0);
    hBm[3] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_DPN_16), IMAGE_BITMAP, 0, 0, 0);
    hBm[4] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_WIP_16), IMAGE_BITMAP, 0, 0, 0);
    hBm[5] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_BCK_16), IMAGE_BITMAP, 0, 0, 0);
    hBm[6] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_OWR_16), IMAGE_BITMAP, 0, 0, 0);
    break;
  case 32:
    hBm[0] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_ERR_32), IMAGE_BITMAP, 0, 0, 0);
    hBm[1] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_DIR_32), IMAGE_BITMAP, 0, 0, 0);
    hBm[2] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_ZIP_32), IMAGE_BITMAP, 0, 0, 0);
    hBm[3] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_DPN_32), IMAGE_BITMAP, 0, 0, 0);
    hBm[4] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_WIP_32), IMAGE_BITMAP, 0, 0, 0);
    hBm[5] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_BCK_32), IMAGE_BITMAP, 0, 0, 0);
    hBm[6] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_OWR_32), IMAGE_BITMAP, 0, 0, 0);
    break;
  default:
    hBm[0] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_ERR_24), IMAGE_BITMAP, 0, 0, 0);
    hBm[1] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_DIR_24), IMAGE_BITMAP, 0, 0, 0);
    hBm[2] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_ZIP_24), IMAGE_BITMAP, 0, 0, 0);
    hBm[3] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_DPN_24), IMAGE_BITMAP, 0, 0, 0);
    hBm[4] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_WIP_24), IMAGE_BITMAP, 0, 0, 0);
    hBm[5] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_BCK_24), IMAGE_BITMAP, 0, 0, 0);
    hBm[6] = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_OWR_24), IMAGE_BITMAP, 0, 0, 0);
    break;
  }

  HIMAGELIST hImgList = ImageList_Create(this->_lvIconsSize, this->_lvIconsSize, ILC_COLOR32, 7, 0 );

  for(unsigned i = 0; i < 7; ++i) {
    ImageList_Add(hImgList, hBm[i], nullptr);
    DeleteObject(hBm[i]);
  }

  SendMessageW(hLv, LVM_SETIMAGELIST, LVSIL_SMALL, reinterpret_cast<LPARAM>(hImgList));
  SendMessageW(hLv, LVM_SETIMAGELIST, LVSIL_NORMAL, reinterpret_cast<LPARAM>(hImgList));

  DeleteObject(hImgList);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_showPkgPopup()
{
  // get handle to "Edit > Packages..." sub-menu
  HMENU hMenu = static_cast<OmUiMain*>(this->_parent)->getMenuEdit(5);

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  TrackPopupMenu(hMenu, TPM_TOPALIGN|TPM_LEFTALIGN, pt.x, pt.y, 0, this->_hwnd, nullptr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMainLib::_install_fth(void* arg)
{
  OmUiMainLib* self = static_cast<OmUiMainLib*>(arg);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmLocation* pLoc = (pMgr->curContext())?pMgr->curContext()->curLocation():nullptr;

  if(pLoc == nullptr)
    return 0;

  HWND hPb = self->getItem(IDC_PB_PGRES);
  HWND hLv = self->getItem(IDC_LV_PKGLS);

  // enable on-process state
  self->setOnProcess(true);

  // get user selection
  vector<unsigned> selec_list;

  unsigned lv_cnt = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);

  for(unsigned i = 0; i < lv_cnt; ++i) {

    if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
      selec_list.push_back(i);
    }
  }

  // Launch install process
  pLoc->packagesInst(selec_list, false, self->_hwnd, hLv, hPb, &self->_abortPending);

  // disable on-process state
  self->setOnProcess(false);

  // send message to notify thread ended
  PostMessage(self->_hwnd, UWM_PACKAGES_DONE, 0, 0);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMainLib::_uninstall_fth(void* arg)
{
  OmUiMainLib* self = static_cast<OmUiMainLib*>(arg);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmLocation* pLoc = (pMgr->curContext())?pMgr->curContext()->curLocation():nullptr;

  if(pLoc == nullptr)
    return 0;

  HWND hPb = self->getItem(IDC_PB_PGRES);
  HWND hLv = self->getItem(IDC_LV_PKGLS);

  // enable on-process state
  self->setOnProcess(true);

  // get user selection
  vector<unsigned> selec_list;

  unsigned lv_cnt = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);

  for(unsigned i = 0; i < lv_cnt; ++i) {

    if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
      selec_list.push_back(i);
    }
  }

  // Launch uninstall process
  pLoc->packagesUnin(selec_list, false, self->_hwnd, hLv, hPb, &self->_abortPending);

  // disable on-process state
  self->setOnProcess(false);

  // send message to notify thread ended
  PostMessage(self->_hwnd, UWM_PACKAGES_DONE, 0, 0);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMainLib::_batch_fth(void* arg)
{
  OmUiMainLib* self = static_cast<OmUiMainLib*>(arg);

  OmManager* pMgr = static_cast<OmManager*>(self->_data);
  OmContext* pCtx = pMgr->curContext();

  HWND hPb = self->getItem(IDC_PB_PGRES);
  HWND hLv = self->getItem(IDC_LV_PKGLS);
  HWND hLb = self->getItem(IDC_LB_BATLS);

  // get current selected location
  int cb_sel = self->msgItem(IDC_CB_LOCLS, CB_GETCURSEL);

  // get current select batch
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  if(lb_sel >= 0) {

    // hide package details
    ShowWindow(self->getItem(IDC_SB_PKIMG), false);
    ShowWindow(self->getItem(IDC_EC_PKTXT), false);
    ShowWindow(self->getItem(IDC_SC_TITLE), false);

    // retrieve the batch object from current selection
    OmBatch* pBat = pCtx->batch(SendMessageW(hLb,LB_GETITEMDATA,lb_sel,0));

    OmLocation* pLoc;

    for(unsigned l = 0; l < pBat->locationCount(); l++) {

      // Select the location
      self->selLocation(pCtx->findLocationIndex(pBat->getLocationUuid(l)));
      pLoc = pCtx->curLocation();

      if(pLoc == nullptr) {
        // warning here
        continue;
      }

      // enable on-process state
      self->setOnProcess(true);

      // create an install and an uninstall list
      vector<unsigned> inst_list, uins_list;

      // create the install list, to keep package order from batch we
      // fill the install list according the batch hash list
      unsigned n = pBat->getInstallCount(l);
      int p;

      for(unsigned i = 0; i < n; ++i) {

        p = pLoc->findPackageIndex(pBat->getInstallHash(l, i));

        if(p >= 0) {
          if(!pLoc->package(p)->hasBackup()) {
            inst_list.push_back(p);
          }
        }

      }

      // create the uninstall list, here we do not care order
      n = pLoc->packageCount();
      for(unsigned i = 0; i < n; ++i) {

        if(!pBat->hasInstallHash(l, pLoc->package(i)->hash())) {
          if(pLoc->package(i)->hasBackup()) {
            uins_list.push_back(i);
          }
        }
      }

      if(uins_list.size()) {
        // Launch uninstall process
        pLoc->packagesUnin(uins_list, false, self->_hwnd, hLv, hPb, &self->_abortPending);
      }

      if(inst_list.size()) {

        // to ensure we respect batch install order, we process one by one
        vector<unsigned> inst;

        for(size_t i = 0; i < inst_list.size(); ++i) {

          // clear and replace package index in vector
          inst.clear(); inst.push_back(inst_list[i]);

          // Launch install process
          pLoc->packagesInst(inst, false, self->_hwnd, hLv, hPb, &self->_abortPending);
        }
      }

      // disable on-process state
      self->setOnProcess(false);
    }

    // restore package details
    ShowWindow(self->getItem(IDC_SB_PKIMG), true);
    ShowWindow(self->getItem(IDC_EC_PKTXT), true);
    ShowWindow(self->getItem(IDC_SC_TITLE), true);
  }

  // Select previously selected location
  self->selLocation(cb_sel);

  // send message to notify thread ended
  PostMessage(self->_hwnd, UWM_PACKAGES_DONE, 0, 0);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_monitor_init(const wstring& path)
{
  // first stops any running monitor
  if(this->_monitor_hth) {
    this->_monitor_stop();
  }

  // create a new folder change notification event
  DWORD mask = FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME;
  mask |= FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE;

  this->_monitor_hev[1] = FindFirstChangeNotificationW(path.c_str(), false, mask);

  // launch new thread to handle notifications
  DWORD dwId;
  this->_monitor_hth = CreateThread(nullptr, 0, this->_monitor_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_monitor_stop()
{
  // stops current directory monitoring thread
  if(this->_monitor_hth) {

    // set custom event to request thread quit, then wait for it
    SetEvent(this->_monitor_hev[0]);
    WaitForSingleObject(this->_monitor_hth, INFINITE);
    CloseHandle(this->_monitor_hth);

    // reset the "stop" event for further usage
    ResetEvent(this->_monitor_hev[0]);

    // close previous folder monitor
    FindCloseChangeNotification(this->_monitor_hev[1]);
    this->_monitor_hev[1] = nullptr;
    this->_monitor_hth = nullptr;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMainLib::_monitor_fth(void* arg)
{
  OmUiMainLib* self = static_cast<OmUiMainLib*>(arg);
  OmManager* pMgr = static_cast<OmManager*>(self->_data);

  DWORD dwObj;

  while(true) {

    dwObj = WaitForMultipleObjects(2, self->_monitor_hev, false, INFINITE);

    if(dwObj == 0) //< custom "stop" event
      break;

    if(dwObj == 1) { //< folder content changed event

      if(pMgr->curContext()->curLocation()) {
        // reload the package list
        self->_reloadLibLv();
      }

      FindNextChangeNotification(self->_monitor_hev[1]);
    }
  }

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onInit()
{
  // Defines fonts for package description, title, and log output
  this->msgItem(IDC_SC_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtTitle), true);
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtMonos), true);
  // Set batches New and Delete buttons icons
  this->msgItem(IDC_BC_NEW, BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcNew));
  this->msgItem(IDC_BC_EDI, BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcMod));

  // define controls tool-tips
  this->_createTooltip(IDC_CB_LOCLS,  L"Select active location");
  this->_createTooltip(IDC_BC_INST,   L"Install selected package(s)");
  this->_createTooltip(IDC_BC_UNIN,   L"Uninstall selected package(s)");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort current process");

  // Initialize the ListView control
  HWND hLv = this->getItem(IDC_LV_PKGLS);

  DWORD dwExStyle = LVS_EX_FULLROWSELECT|
                    LVS_EX_SUBITEMIMAGES|
                    LVS_EX_DOUBLEBUFFER;

  SendMessageW(hLv, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwExStyle);

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
  SendMessageW(hLv, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Name");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 550;
  lvCol.iSubItem = 1;
  SendMessageW(hLv, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Version");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem = 2;
  SendMessageW(hLv, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&lvCol));

  // force refresh
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onShow()
{
  // select location according current ComboBox selection
  this->selLocation(this->msgItem(IDC_CB_LOCLS, CB_GETCURSEL));

  // refresh dialog
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onResize()
{
  // Locations Combo-Box
  this->_setItemPos(IDC_CB_LOCLS, 5, 5, this->width()-161, 12);
  // Library path EditControl
  this->_setItemPos(IDC_EC_INPT1, 5, 20, this->width()-161, 12);
  // Package List ListView
  this->_setItemPos(IDC_LV_PKGLS, 5, 35, this->width()-161, this->height()-151);
  // Resize the ListView column
  LONG size[4];
  HWND hLv = this->getItem(IDC_LV_PKGLS);
  GetClientRect(hLv, reinterpret_cast<LPRECT>(&size));
  SendMessageW(hLv, LVM_SETCOLUMNWIDTH, 1, size[2]-125);

  // Install and Uninstall buttons
  this->_setItemPos(IDC_BC_INST, 5, this->height()-114, 50, 14);
  this->_setItemPos(IDC_BC_UNIN, 55, this->height()-114, 50, 14);
  // Progress bar
  this->_setItemPos(IDC_PB_PGRES, 107, this->height()-113, this->width()-315, 12);
  // Abort button
  this->_setItemPos(IDC_BC_ABORT, this->width()-205, this->height()-114, 50, 14);
  // Package name/title
  this->_setItemPos(IDC_SC_TITLE, 5, this->height()-96, this->width()-161, 12);
  // Package snapshot
  this->_setItemPos(IDC_SB_PKIMG, 5, this->height()-83, 85, 78);
  // Package description
  this->_setItemPos(IDC_EC_PKTXT, 95, this->height()-83, this->width()-101, 78);

  // Vertical separator
  this->_setItemPos(IDC_SC_SEPAR, this->width()-150, 5, 1, this->height()-105);

  // Batches label
  this->_setItemPos(IDC_SC_LBL01, this->width()-143, 8, 136, 12);
  // Batches List-Box
  this->_setItemPos(IDC_LB_BATLS, this->width()-143, 20, 136, this->height()-137);
  // Batches Apply, New.. and Delete buttons
  this->_setItemPos(IDC_BC_APPLY, this->width()-143, this->height()-114, 45, 14);
  this->_setItemPos(IDC_BC_NEW, this->width()-97, this->height()-114, 45, 14);
  this->_setItemPos(IDC_BC_EDI, this->width()-51, this->height()-114, 45, 14);


  InvalidateRect(this->_hwnd, nullptr, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onRefresh()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // check whether a context is selected
  if(!pMgr->curContext()) {
    // unselect location
    this->selLocation(-1);
  }

  // disable all packages buttons
  this->enableItem(IDC_BC_ABORT, false);
  this->enableItem(IDC_BC_INST, false);
  this->enableItem(IDC_BC_UNIN, false);

  // hide package details
  ShowWindow(this->getItem(IDC_SC_TITLE), false);
  ShowWindow(this->getItem(IDC_EC_PKTXT), false);
  ShowWindow(this->getItem(IDC_SB_PKIMG), false);

  // disable the Progress-Bar
  this->enableItem(IDC_PB_PGRES, false);

  this->_reloadLocCb(); //< reload Location Combo-Box

  // disable all batches buttons
  this->enableItem(IDC_BC_APPLY, false);
  this->enableItem(IDC_BC_NEW, (pMgr->curContext() != nullptr));
  this->enableItem(IDC_BC_EDI, false);

  this->_reloadBatLb(); //< reload Batches list
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onQuit()
{
  // stop Library folder changes monitoring
  this->_monitor_stop();

  // safely and cleanly close threads handles
  if(this->_install_hth) {
    WaitForSingleObject(this->_install_hth, INFINITE);
    CloseHandle(this->_install_hth);
    this->_install_hth = nullptr;
  }
  if(this->_uninstall_hth) {
    WaitForSingleObject(this->_uninstall_hth, INFINITE);
    CloseHandle(this->_uninstall_hth);
    this->_uninstall_hth = nullptr;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMainLib::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // packages processing thread ended
  if(uMsg == UWM_PACKAGES_DONE) {
    // safely and cleanly close threads handles
    if(this->_install_hth) {
      WaitForSingleObject(this->_install_hth, INFINITE);
      CloseHandle(this->_install_hth);
      this->_install_hth = nullptr;
    }
    if(this->_uninstall_hth) {
      WaitForSingleObject(this->_uninstall_hth, INFINITE);
      CloseHandle(this->_uninstall_hth);
      this->_uninstall_hth = nullptr;
    }
    // disable the On-Process state of parent window
    static_cast<OmUiMain*>(this->_parent)->setOnProcess(false);
    // Refresh Package list
    this->_reloadLibLv();
  }

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->curContext();
  OmLocation* pLoc = (pCtx) ? pCtx->curLocation() : nullptr;

  if(pLoc == nullptr)
    return false;

  if(uMsg == WM_NOTIFY) {

    NMHDR* pNmhdr = reinterpret_cast<NMHDR*>(lParam);

    if(LOWORD(wParam) == IDC_LV_PKGLS) {

      // if thread is running we block all interaction
      if(this->_install_hth || this->_uninstall_hth)
        return false;

      if(pNmhdr->code == NM_DBLCLK) {
        this->toggle();
        return false;
      }

      if(pNmhdr->code == LVN_ITEMCHANGED) {
        // update package(s) selection
        this->_onSelectPkg();
        return false;
      }

      if(pNmhdr->code == NM_RCLICK) {
        // Open the popup menu
        this->_showPkgPopup();
        return false;
      }

      if(pNmhdr->code == LVN_COLUMNCLICK) {

        NMLISTVIEW* pNmlv = reinterpret_cast<NMLISTVIEW*>(lParam);

        switch(pNmlv->iSubItem)
        {
        case 0:
          pLoc->setPackageSorting(PKG_SORTING_STAT);
          break;
        case 2:
          pLoc->setPackageSorting(PKG_SORTING_VERS);
          break;
        default:
          pLoc->setPackageSorting(PKG_SORTING_NAME);
          break;
        }
        this->refresh();
        return false;
      }
    }
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {

    case IDC_CB_LOCLS:
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        this->selLocation(this->msgItem(IDC_CB_LOCLS, CB_GETCURSEL));
      }
      break;

    case IDC_LB_BATLS: //< Location(s) list List-Box
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        this->_onSelectBat();
      }
      if(HIWORD(wParam) == LBN_DBLCLK) {
        this->batch();
      }
      break;

    case IDC_BC_INST:
      this->install();
      break;

    case IDC_BC_UNIN:
      this->uninstall();
      break;

    case IDC_BC_ABORT:
      this->_abortPending = true;
      this->enableItem(IDC_BC_ABORT, false);
      break;

    case IDC_BC_NEW:
      {
        OmUiNewBat* pUiNewBat = static_cast<OmUiNewBat*>(this->siblingById(IDD_NEW_BAT));
        pUiNewBat->setContext(pCtx);
        pUiNewBat->open(true);
      }
      break;

    case IDC_BC_APPLY:
      this->batch();
      break;

    case IDC_BC_EDI:
      this->ediBatch();
      break;

    case IDM_EDIT_PKG_INST:
      this->install();
      break;

    case IDM_EDIT_PKG_UINS:
      this->uninstall();
      break;

    case IDM_EDIT_PKG_TRSH:
      this->moveTrash();
      break;

    case IDM_EDIT_PKG_OPEN:
      this->openExplore();
      break;

    case IDM_EDIT_PKG_INFO:
      this->viewDetails();
      break;

    }
  }

  return false;
}
