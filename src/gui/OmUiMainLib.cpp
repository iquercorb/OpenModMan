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
  _hBlankImg(nullptr),
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

  // load the package blank picture
  this->_hBlankImg = reinterpret_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_PKG_BLANK),IMAGE_BITMAP,0,0,0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainLib::~OmUiMainLib()
{
  // stop Library folder changes monitoring
  this->_monitor_stop();

  DeleteObject(this->_hBlankImg);
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
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  // stop Library folder monitoring
  this->_monitor_stop();

  // select the requested Location
  if(manager->curContext()) {

    manager->curContext()->selLocation(i);

    OmLocation* location = manager->curContext()->curLocation();

    if(location) {

      location->installAccess(this->_hwnd);

      location->backupAccess(this->_hwnd);

      if(location->libraryAccess(this->_hwnd)) {
        // start Library folder monitoring
        this->_monitor_init(location->libraryDir());
      }
    }
  }

  OmUiMain* uiMain = reinterpret_cast<OmUiMain*>(this->_parent);

  // disable "Edit > Package" in main menu
  uiMain->setMenuEdit(1, MF_BYPOSITION|MF_GRAYED);

  // disable the "Edit > Package > []" elements
  HMENU hMenu = uiMain->getMenuEdit(1);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_INST, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_UINS, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_OPEN, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_TRSH, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, MF_GRAYED);

  // refresh
  this->_reloadLibEc();
  this->_reloadLibLv();

  // forces control to select item
  HWND hCb = GetDlgItem(this->_hwnd, IDC_CB_LOCLS);

  if(i != SendMessageW(hCb, CB_GETCURSEL, 0, 0))
    SendMessageW(hCb, CB_SETCURSEL, i, 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::toggle()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmLocation* location = manager->curContext()->curLocation();

  HWND hLv = GetDlgItem(this->_hwnd, IDC_LV_PKGLS);

  DWORD dwid;
  unsigned n = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);
  for(unsigned i = 0; i < n; ++i) {
    if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
      // enable the On-Process state of parent window
      reinterpret_cast<OmUiMain*>(this->_parent)->setOnProcess(true);

      this->_abortPending = false;

      if(location->package(i)->hasBackup()) {
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
  reinterpret_cast<OmUiMain*>(this->_parent)->setOnProcess(true);

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
  reinterpret_cast<OmUiMain*>(this->_parent)->setOnProcess(true);

  this->_uninstall_hth = CreateThread(nullptr, 0, this->_uninstall_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::viewDetails()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmLocation* location = manager->curContext()->curLocation();
  OmPackage* package = nullptr;

  HWND hLv = GetDlgItem(this->_hwnd, IDC_LV_PKGLS);

  unsigned n = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);
  for(unsigned i = 0; i < n; ++i) {
    if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
      package = location->package(i);
      break;
    }
  }

  if(package) {
    OmUiPropPkg* uiPropPkg = static_cast<OmUiPropPkg*>(this->childById(IDD_PROP_PKG));
    uiPropPkg->setPackage(package);
    uiPropPkg->open(true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::moveTrash()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmLocation* location = manager->curContext()->curLocation();

  vector<OmPackage*> trash_list;

  HWND hLv = GetDlgItem(this->_hwnd, IDC_LV_PKGLS);

  this->setOnProcess(true);

  unsigned n = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);
  for(unsigned i = 0; i < n; ++i) {
    if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
      trash_list.push_back(location->package(i));
    }
  }

  if(trash_list.size()) {
    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete package(s)", L"Move the selected package(s) to trash ?"))
      return;

    for(size_t i = 0; i < trash_list.size(); ++i) {
      Om_moveToTrash(trash_list[i]->sourcePath());
      this->_reloadLibLv();
    }
  }

  // Unselect all items
  LVITEM lvI = {};
  lvI.mask = LVIF_STATE;
  lvI.stateMask = LVIS_SELECTED;
  SendMessage(hLv, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvI);

  this->setOnProcess(false);

  // update package selection
  this->_onSelectPkg();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::openExplore()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmLocation* location = manager->curContext()->curLocation();

  vector<OmPackage*> explo_list;

  HWND hLv = GetDlgItem(this->_hwnd, IDC_LV_PKGLS);

  unsigned n = SendMessageW(hLv, LVM_GETITEMCOUNT, 0, 0);
  for(unsigned i = 0; i < n; ++i) {
    if(SendMessageW(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
      explo_list.push_back(location->package(i));
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
  reinterpret_cast<OmUiMain*>(this->_parent)->setOnProcess(true);

  this->_batch_hth = CreateThread(nullptr, 0, this->_batch_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMainLib::deleteBatch()
{
  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_BATLS);

  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  if(lb_sel >= 0) {

    unsigned bat_id = SendMessageW(hLb, LB_GETITEMDATA, lb_sel, 0);

    OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
    OmContext* curCtx = manager->curContext();

    // warns the user before committing the irreparable
    wstring msg;
    msg = L"Are your sure you want to delete the Batch '";
    msg += curCtx->batch(bat_id)->title();
    msg += L"' ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Batch", msg)) {
      return false;
    }

    if(!curCtx->deleteBatch(bat_id)) {
      Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Batch failed", curCtx->lastError());
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
void OmUiMainLib::setOnProcess(bool enable)
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  // handle to "Edit > Package" sub-menu
  HMENU hMenu = reinterpret_cast<OmUiMain*>(this->_parent)->getMenuEdit(1);

  if(enable) {
    this->_onProcess = true;

    // disable Location combo-box
    EnableWindow(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), false);

    // disable install and uninstall button
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_INST), false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UNIN), false);
    // disable the package list
    EnableWindow(GetDlgItem(this->_hwnd, IDC_LV_PKGLS), false);

    // disable batches lisb-box and buttons
    EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_BATLS), false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_NEW), false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);

    EnableMenuItem(hMenu, IDM_EDIT_PKG_INST, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_UINS, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_OPEN, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_TRSH, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, MF_GRAYED);

    // enable abort button
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ABORT), true);

  } else {

    OmContext* context = manager->curContext();

    HWND hLv = GetDlgItem(this->_hwnd, IDC_LV_PKGLS);
    HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_BATLS);

    // enable Location combo-box
    EnableWindow(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), true);

    // enable install and uninstall button
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_INST), true);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UNIN), true);
    // disable abort button
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ABORT), false);
    // enable the package list
    EnableWindow(hLv, true);
    // disable batches lisb-box and buttons
    EnableWindow(hLb, true);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_NEW), (context != nullptr));

    bool lb_has_sel = (SendMessageW(hLb, LB_GETCURSEL, 0, 0) >= 0);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), lb_has_sel);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), lb_has_sel);

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
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmLocation* location = nullptr;

  if(manager->curContext())
    location = manager->curContext()->curLocation();

  if(location == nullptr)
    return;

  // keep handle to main dialog
  OmUiMain* uiMain = reinterpret_cast<OmUiMain*>(this->_parent);

  // disable "Edit > Package" in main menu
  uiMain->setMenuEdit(1, MF_BYPOSITION|MF_GRAYED);

  // handle to "Edit > Package" sub-menu
  HMENU hMenu = uiMain->getMenuEdit(1);

  HWND hLv = GetDlgItem(this->_hwnd, IDC_LV_PKGLS);
  HWND hSb = GetDlgItem(this->_hwnd, IDC_SB_PKIMG);

  // get count of selected item
  unsigned lv_nsl = SendMessage(hLv, LVM_GETSELECTEDCOUNT, 0, 0);

  if(lv_nsl > 0) {
    // at least one, we enable buttons
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_INST), true);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UNIN), true);

    // enable the "Edit > Package" sub-items
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INST, 0);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_UINS, 0);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_OPEN, 0);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_TRSH, 0);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, 0);

    // enable "Edit > Package" sub-menu
    uiMain->setMenuEdit(1, MF_BYPOSITION|MF_ENABLED);

    if(lv_nsl > 1) {

      // multiple selection, we cannot display readme and snapshot
      ShowWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), false);

      // set title default message
      ShowWindow(GetDlgItem(this->_hwnd, IDC_SC_TITLE), true);
      SetDlgItemTextW(this->_hwnd, IDC_SC_TITLE, L"<Multiple selection>");

      // set default blank picture
      ShowWindow(hSb, true);
      SendMessage(hSb, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)this->_hBlankImg);

      // disable the "view detail..." sub-menu
      EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, MF_GRAYED);

    } else {

      // get the select item id
      unsigned itm_count = SendMessage(hLv, LVM_GETITEMCOUNT, 0, 0);
      for(unsigned i = 0; i < itm_count; ++i) {

        if(SendMessage(hLv, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {

          SetDlgItemTextW(this->_hwnd, IDC_SC_TITLE, location->package(i)->name().c_str());

          if(location->package(i)->desc().size()) {
            SetDlgItemTextW(this->_hwnd, IDC_EC_PKTXT, location->package(i)->desc().c_str());
          } else {
            SetDlgItemTextW(this->_hwnd, IDC_EC_PKTXT, L"<no description available>");
          }

          if(location->package(i)->picture()) {
            HBITMAP hBmp = Om_getBitmapThumbnail(location->package(i)->picture(), OMM_PKG_THMB_SIZE, OMM_PKG_THMB_SIZE);
            SendMessage(hSb, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
            DeleteObject(hBmp);
          } else {
            SendMessage(hSb, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)this->_hBlankImg);
          }

          this->_setControlPos(IDC_SB_PKIMG, 5, this->height()-83, 85, 78);

          ShowWindow(GetDlgItem(this->_hwnd, IDC_SC_TITLE), true);
          ShowWindow(hSb, true);
          ShowWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), true);

        }
      }
    }
  } else {
    // nothing selected, we disable all
    ShowWindow(GetDlgItem(this->_hwnd, IDC_SC_TITLE), false);
    ShowWindow(hSb, false);
    ShowWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), false);

    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_INST), false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UNIN), false);

    // disable the "Edit > Package" sub-items
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INST, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_UINS, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_OPEN, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_TRSH, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, MF_GRAYED);

    // disable "Edit > Package" sub-menu
    uiMain->setMenuEdit(1, MF_BYPOSITION|MF_GRAYED);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onSelectBat()
{
  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_BATLS);

  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  if(lb_sel >= 0) {
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), true);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), true);
  } else {
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_reloadLibEc()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmLocation* location = nullptr;

  if(manager->curContext())
    location = manager->curContext()->curLocation();

  if(location) {

    // check for Library folder validity
    if(location->libraryAccess(this->_hwnd)) {
      // set the library path
      SetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, location->libraryDir().c_str());
    } else {
      SetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, L"<folder access error>");
    }

  } else {
    // empty library path
    SetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, L"<no Location selected>");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_reloadLibLv(bool clear)
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmLocation* location = nullptr;

  if(manager->curContext())
    location = manager->curContext()->curLocation();

  // get List view control
  HWND hLv = GetDlgItem(this->_hwnd, IDC_LV_PKGLS);

  if(location) {

    // if icon size changed, reload
    if(this->_lvIconsSize != manager->iconsSize()) {
      this->_reloadIcons();
    }

    // return now if library folder cannot be accessed
    if(!location->libraryAccess(this->_hwnd))
      return;

    // force Location library refresh
    if(clear) location->packageListClear(); //< clear to rebuild entirely
    location->packageListRefresh();

    // we enable the List-View
    EnableWindow(hLv, true);

    // empty list view
    SendMessageW(hLv, LVM_DELETEALLITEMS, 0, 0);

    // Save list-view scroll position to lvRect
    RECT lvRec;
    SendMessageW(hLv, LVM_GETVIEWRECT, 0, (LPARAM)&lvRec);

    // add item to list view
    OmPackage* package;
    LVITEMW lvItem;
    for(unsigned i = 0; i < location->packageCount(); ++i) {

      package = location->package(i);

      // the first colum, package status, here we INSERT the new item
      lvItem.iItem = i;
      lvItem.mask = LVIF_IMAGE;
      lvItem.iSubItem = 0;
      if(package->isType(PKG_TYPE_BCK)) {
        if(location->isBakcupOverlapped(package)) {
          lvItem.iImage = 6; // IDB_PKG_OWR
        } else {
          lvItem.iImage = 5; // IDB_PKG_BCK
        }
      } else {
        lvItem.iImage = -1; // none
      }
      lvItem.iItem = SendMessageW(hLv, LVM_INSERTITEMW, 0, (LPARAM)&lvItem);

      // Second column, the package name and type, here we set the subitem
      lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
      lvItem.iSubItem = 1;
      if(package->isType(PKG_TYPE_SRC)) {
        if(package->isType(PKG_TYPE_ZIP)) {
          if(package->dependCount()) {
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
      lvItem.pszText = (LPWSTR)location->package(i)->name().c_str();
      SendMessageW(hLv, LVM_SETITEMW, 0, (LPARAM)&lvItem);

      // Third column, the package version, we set the subitem
      lvItem.mask = LVIF_TEXT;
      lvItem.iSubItem = 2;
      lvItem.pszText = (LPWSTR)location->package(i)->version().asString().c_str();
      SendMessageW(hLv, LVM_SETITEMW, 0, (LPARAM)&lvItem);
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
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmContext* context = manager->curContext();

  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_BATLS);

  // empty List-Box
  SendMessage(hLb, LB_RESETCONTENT, 0, 0);

  if(context) {

    for(unsigned i = 0; i < context->batchCount(); ++i) {

      SendMessageW(hLb, LB_ADDSTRING, i, (LPARAM)context->batch(i)->title().c_str());
      SendMessageW(hLb, LB_SETITEMDATA, i, i); // for Location index reordering
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_reloadLocCb()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmContext* context = manager->curContext();

  HWND hCb = GetDlgItem(this->_hwnd, IDC_CB_LOCLS);

  if(context == nullptr) {
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
  if(context->locationCount()) {

    wstring label;

    EnableWindow(hCb, true);

    for(unsigned i = 0; i < context->locationCount(); ++i) {

      label = context->location(i)->title();
      label += L" - ";

      // checks whether installation destination path is valid
      if(context->location(i)->installAccess(this->_hwnd)) {
        label += context->location(i)->installDir();
      } else {
        label += L"<folder access error>";
      }

      SendMessageW(hCb, CB_ADDSTRING, i, (LPARAM)label.c_str());
    }

    // select the the previously selected Context
    if(cb_sel >= 0 && cb_sel < static_cast<int>(context->locationCount())) {
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
    wchar_t str[] = L"The current Context does not have any configured "
                    L"Location. A Context needs at least one Location.\n\n"
                    L"Do you want to configure a new Location now ?";

    if(Om_dialogBoxQuerry(this->_hwnd, L"No Location found", str)) {
      // TODO : adapter ceci
      this->childById(IDD_WIZ_LOC)->open(true);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_reloadIcons()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  // update size
  this->_lvIconsSize = manager->iconsSize();

   // hold the HWND of our list view control */
  HWND hLv = GetDlgItem(_hwnd, IDC_LV_PKGLS);

  // We add an image list to the list-view control, the image list will
  // contain all icons we needs.
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

  SendMessage(hLv, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)hImgList);
  SendMessage(hLv, LVM_SETIMAGELIST, LVSIL_NORMAL, (LPARAM)hImgList);

  DeleteObject(hImgList);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_showPkgPopup()
{
  // get handle to "Edit > Packages..." sub-menu
  HMENU hMenu = reinterpret_cast<OmUiMain*>(this->_parent)->getMenuEdit(1);

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

  OmManager* manager = static_cast<OmManager*>(self->_data);
  OmLocation* location = manager->curContext()->curLocation();

  HWND hPb = GetDlgItem(self->_hwnd, IDC_PB_PGRES);
  HWND hLv = GetDlgItem(self->_hwnd, IDC_LV_PKGLS);

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
  location->installSelection(selec_list, false, self->_hwnd, hLv, hPb, &self->_abortPending);

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

  OmManager* manager = static_cast<OmManager*>(self->_data);
  OmLocation* location = manager->curContext()->curLocation();

  HWND hPb = GetDlgItem(self->_hwnd, IDC_PB_PGRES);
  HWND hLv = GetDlgItem(self->_hwnd, IDC_LV_PKGLS);

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
  location->uninstSelection(selec_list, false, self->_hwnd, hLv, hPb, &self->_abortPending);

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

  OmManager* manager = static_cast<OmManager*>(self->_data);
  OmContext* context = manager->curContext();
  OmLocation* location;

  HWND hPb = GetDlgItem(self->_hwnd, IDC_PB_PGRES);
  HWND hLv = GetDlgItem(self->_hwnd, IDC_LV_PKGLS);
  HWND hLb = GetDlgItem(self->_hwnd, IDC_LB_BATLS);

  // get current selected location
  int cb_sel = SendMessageW(GetDlgItem(self->_hwnd, IDC_CB_LOCLS), CB_GETCURSEL, 0, 0);

  // get current select batch
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  if(lb_sel >= 0) {

    // hide package details
    ShowWindow(GetDlgItem(self->_hwnd, IDC_SB_PKIMG), false);
    ShowWindow(GetDlgItem(self->_hwnd, IDC_EC_PKTXT), false);
    ShowWindow(GetDlgItem(self->_hwnd, IDC_SC_TITLE), false);

    // retrieve the batch object from current selection
    OmBatch* batch = context->batch(SendMessageW(hLb,LB_GETITEMDATA,lb_sel,0));

    for(unsigned l = 0; l < batch->locationCount(); l++) {

      // Select the location
      self->selLocation(context->findLocation(batch->getLocationUuid(l)));
      location = context->curLocation();

      if(location == nullptr) {
        // warning here
        continue;
      }

      // enable on-process state
      self->setOnProcess(true);

      // create an install and an uninstall list
      vector<unsigned> inst_list, uins_list;

      // create the install list, to keep package order from batch we
      // fill the install list according the batch hash list
      unsigned n = batch->getInstallCount(l);
      int p;

      for(unsigned i = 0; i < n; ++i) {

        p = location->findPackageIndex(batch->getInstallHash(l, i));

        if(p >= 0) {
          if(!location->package(p)->hasBackup()) {
            inst_list.push_back(p);
          }
        }

      }

      // create the uninstall list, here we do not care order
      n = location->packageCount();
      for(unsigned i = 0; i < n; ++i) {

        if(!batch->hasInstallHash(l, location->package(i)->hash())) {
          if(location->package(i)->hasBackup()) {
            uins_list.push_back(i);
          }
        }
      }

      if(uins_list.size()) {
        // Launch uninstall process
        location->uninstSelection(uins_list, false, self->_hwnd, hLv, hPb, &self->_abortPending);
      }

      if(inst_list.size()) {

        // to ensure we respect batch install order, we process one by one
        vector<unsigned> inst;

        for(size_t i = 0; i < inst_list.size(); ++i) {

          // clear and replace package index in vector
          inst.clear(); inst.push_back(inst_list[i]);

          // Launch install process
          location->installSelection(inst, false, self->_hwnd, hLv, hPb, &self->_abortPending);
        }
      }

      // disable on-process state
      self->setOnProcess(false);
    }

    // restore package details
    ShowWindow(GetDlgItem(self->_hwnd, IDC_SB_PKIMG), true);
    ShowWindow(GetDlgItem(self->_hwnd, IDC_EC_PKTXT), true);
    ShowWindow(GetDlgItem(self->_hwnd, IDC_SC_TITLE), true);
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
  // create a new folder change notification event
  DWORD mask =  FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;
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
  OmManager* manager = static_cast<OmManager*>(self->_data);

  DWORD dwObj;

  while(true) {

    dwObj = WaitForMultipleObjects(2, self->_monitor_hev, false, INFINITE);

    if(dwObj == 0) //< custom "stop" event
      break;

    if(dwObj == 1) { //< folder content changed event

      if(manager->curContext()->curLocation()) {
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
void OmUiMainLib::_onShow()
{
  // define controls tool-tips
  this->_createTooltip(IDC_CB_LOCLS,  L"Select active location");
  this->_createTooltip(IDC_BC_INST,   L"Install selected package(s)");
  this->_createTooltip(IDC_BC_UNIN,   L"Uninstall selected package(s)");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort current process");

  HBITMAP hBm;

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_ADD), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_NEW), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_REM), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_DEL), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  // hold the HWND of our list view control
  HWND hLv = GetDlgItem(this->_hwnd, IDC_LV_PKGLS);

  // the _onShow() function is called each time the dialog receive the "Show"
  // message, which happen every the the corresponding Tab is selected. To prevent
  // the List-View to be rebuilt and adding columns each time, we must check the
  // List-View header content.
  HWND hLvHead = (HWND)SendMessage(hLv,LVM_GETHEADER,0,0);

  // create columns only if list-view header is empty
  if(SendMessage(hLvHead, HDM_GETITEMCOUNT, 0, 0L) <= 0) {

    DWORD dwExStyle = LVS_EX_FULLROWSELECT|
                      LVS_EX_SUBITEMIMAGES|
                      LVS_EX_DOUBLEBUFFER;

    SendMessage(hLv, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)dwExStyle);


    // we now add columns into our list-view control
    LVCOLUMNW lvCol;
    lvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;

    //  "The alignment of the leftmost column is always LVCFMT_LEFT; it
    // cannot be changed." says Mr Microsoft. Do not ask why, the Microsoft's
    // mysterious ways... So, don't try to fix this.

    lvCol.pszText = (LPWSTR)"Status";
    lvCol.fmt = LVCFMT_RIGHT;
    lvCol.cx = 43;
    lvCol.iSubItem = 0;
    SendMessage(hLv, LVM_INSERTCOLUMN, 0, (LPARAM)&lvCol);

    lvCol.pszText = (LPWSTR)"Name";
    lvCol.fmt = LVCFMT_LEFT;
    lvCol.cx = 550;
    lvCol.iSubItem = 1;
    SendMessage(hLv, LVM_INSERTCOLUMN, 1, (LPARAM)&lvCol);

    lvCol.pszText = (LPWSTR)"Version";
    lvCol.fmt = LVCFMT_LEFT;
    lvCol.cx = 80;
    lvCol.iSubItem = 2;
    SendMessage(hLv, LVM_INSERTCOLUMN, 2, (LPARAM)&lvCol);

    HFONT hFont;

    // defines fonts for package description, title, and log output
    hFont = CreateFont(14,0,0,0,700,false,false,false,1,0,0,5,0,"Consolas");
    SendMessage(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), WM_SETFONT, (WPARAM)hFont, 1);

    hFont = CreateFont(18,0,0,0,800,false,false,false,1,0,0,5,0,"Ms Shell Dlg");
    SendMessage(GetDlgItem(this->_hwnd, IDC_SC_TITLE), WM_SETFONT, (WPARAM)hFont, 1);
  }

  // force refresh
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onResize()
{
  // Locations Combo-Box
  this->_setControlPos(IDC_CB_LOCLS, 5, 5, this->width()-161, 12);
  // Library path EditControl
  this->_setControlPos(IDC_EC_INPT1, 5, 20, this->width()-161, 12);
  // Package List ListView
  this->_setControlPos(IDC_LV_PKGLS, 5, 35, this->width()-161, this->height()-151);
  // Resize the ListView column
  LONG size[4];
  HWND hLv = GetDlgItem(this->_hwnd, IDC_LV_PKGLS);
  GetClientRect(hLv, (LPRECT)&size);
  SendMessage(hLv, LVM_SETCOLUMNWIDTH, 1, size[2]-125);

  // Install and Uninstall buttons
  this->_setControlPos(IDC_BC_INST, 5, this->height()-114, 50, 14);
  this->_setControlPos(IDC_BC_UNIN, 55, this->height()-114, 50, 14);
  // Progress bar
  this->_setControlPos(IDC_PB_PGRES, 107, this->height()-113, this->width()-315, 12);
  // Abort button
  this->_setControlPos(IDC_BC_ABORT, this->width()-205, this->height()-114, 50, 14);
  // Package name/title
  this->_setControlPos(IDC_SC_TITLE, 5, this->height()-96, this->width()-161, 12);
  // Package snapshot
  this->_setControlPos(IDC_SB_PKIMG, 5, this->height()-83, 85, 78);
  // Package description
  this->_setControlPos(IDC_EC_PKTXT, 95, this->height()-83, this->width()-101, 78);

  // Vertical separator
  this->_setControlPos(IDC_SC_SEPAR, this->width()-150, 5, 1, this->height()-105);

  // Batches label
  this->_setControlPos(IDC_SC_LBL01, this->width()-143, 8, 136, 12);
  // Batches List-Box
  this->_setControlPos(IDC_LB_BATLS, this->width()-143, 20, 136, this->height()-137);
  // Batches Apply, New.. and Delete buttons
  this->_setControlPos(IDC_BC_APPLY, this->width()-143, this->height()-114, 45, 14);
  this->_setControlPos(IDC_BC_NEW, this->width()-97, this->height()-114, 45, 14);
  this->_setControlPos(IDC_BC_DEL, this->width()-51, this->height()-114, 45, 14);


  InvalidateRect(this->_hwnd, nullptr, true);

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainLib::_onRefresh()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  // disable all packages buttons
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ABORT), false);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_INST), false);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UNIN), false);

  // hide package details
  ShowWindow(GetDlgItem(this->_hwnd, IDC_SC_TITLE), false);
  ShowWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), false);
  ShowWindow(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), false);

  // disable the Progress-Bar
  EnableWindow(GetDlgItem(this->_hwnd, IDC_PB_PGRES), false);

  this->_reloadLocCb(); //< reload Location Combo-Box

  // disable all batches buttons
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), false);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_NEW), (manager->curContext() != nullptr));
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);

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
    reinterpret_cast<OmUiMain*>(this->_parent)->setOnProcess(false);
    // Refresh Package list
    this->_reloadLibLv();
  }

  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmContext* curCtx = manager->curContext();

  OmLocation* curLoc = (curCtx) ? curCtx->curLocation() : nullptr;

  if(curLoc == nullptr)
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
          curLoc->setPackageSorting(PKG_SORTING_STAT);
          break;
        case 2:
          curLoc->setPackageSorting(PKG_SORTING_VERS);
          break;
        default:
          curLoc->setPackageSorting(PKG_SORTING_NAME);
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
        this->selLocation(SendMessageW(GetDlgItem(this->_hwnd, IDC_CB_LOCLS), CB_GETCURSEL, 0, 0));
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
      EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ABORT), false);
      break;

    case IDC_BC_NEW:
      {
        OmUiNewBat* uiNewBat = reinterpret_cast<OmUiNewBat*>(this->siblingById(IDD_NEW_BAT));
        uiNewBat->setContext(curCtx);
        uiNewBat->open(true);
      }
      break;

    case IDC_BC_APPLY:
      this->batch();
      break;

    case IDC_BC_DEL:
      this->deleteBatch();
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
