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
#include <algorithm>            //< std::find

#include "OmBase.h"

#include "OmBaseUi.h"

#include "OmBaseWin.h"
  #include <UxTheme.h>

#include "OmBaseApp.h"

#include "OmModMan.h"

#include "OmUiManMain.h"
#include "OmUiManFoot.h"
#include "OmUiManMainLib.h"
#include "OmUiManMainNet.h"
#include "OmUiAddPst.h"
#include "OmUiAddRep.h"
#include "OmUiAddChn.h"
#include "OmUiPropHub.h"
#include "OmUiPropChn.h"
#include "OmUiPropMan.h"
#include "OmUiPropPst.h"
#include "OmUiPropMod.h"
#include "OmUiPropNet.h"
#include "OmUiHelpLog.h"
#include "OmUiHelpAbt.h"
#include "OmUiWizHub.h"
#include "OmUiToolPkg.h"
#include "OmUiToolRep.h"
#include "OmUiPictView.h"

#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiMan.h"

#define FOOT_MIN_HEIGHT 170
#define MAIN_MIN_HEIGHT 200

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMan::OmUiMan(HINSTANCE hins) : OmDialog(hins),
  /*_pageName(), _pageDial(), */
  _UiManMain(nullptr),
  _UiManFoot(nullptr),
  _UiManMainLib(nullptr),
  _freeze_mode(false),
  _freeze_quit(false),
  _split_curs_hover(false),
  _split_curs_dragg(false),
  _split_move_param{},
  _lib_monitor_hth(nullptr),
  _listview_himl(nullptr),
  _listview_himl_size(0),
  _setups_count(0),
  _setups_chan_sel(0),
  _lv_chn_icons_size(0),
  _lv_pst_icons_size(0)
{
  // add main frames
  this->_UiManMain = new OmUiManMain(hins);
  this->addChild(this->_UiManMain);
  this->_UiManFoot = new OmUiManFoot(hins);
  this->addChild(this->_UiManFoot);

  // add children dialogs
  this->addChild(new OmUiPropMan(hins));    //< Dialog for Manager Options
  this->addChild(new OmUiPropHub(hins));    //< Dialog for Mod Hub Properties
  this->addChild(new OmUiPropChn(hins));    //< Dialog for Mod Channel Properties
  this->addChild(new OmUiPropPst(hins));    //< Dialog for Mod Preset Properties
  this->addChild(new OmUiPropMod(hins));    //< Dialog for Mod Pack properties
  this->addChild(new OmUiPropNet(hins));    //< Dialog for Remote Mod Pack properties
  this->addChild(new OmUiHelpLog(hins));    //< Dialog for Help Debug log
  this->addChild(new OmUiHelpAbt(hins));    //< Dialog for Help About
  this->addChild(new OmUiWizHub(hins));     //< Dialog for New Mod Hub Wizard
  this->addChild(new OmUiToolPkg(hins));    //< Dialog for New Package
  this->addChild(new OmUiAddPst(hins));     //< Dialog for New Preset
  this->addChild(new OmUiAddChn(hins));     //< Dialog for Adding Mod Channel
  this->addChild(new OmUiAddRep(hins));     //< Dialog for Add Repository
  this->addChild(new OmUiToolRep(hins));    //< Dialog for Repository Editor
  this->addChild(new OmUiPictView(hins));   //< Dialog for Picture Viewer

  // set the accelerator table for the dialog
  this->setAccel(IDR_ACCEL);

  // elements for real-time directory monitoring thread
  this->_lib_monitor_hev[0] = CreateEvent(nullptr, true, false, nullptr); //< custom event to notify thread must exit
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMan::~OmUiMan()
{
  // stop Library folder changes monitoring
  this->monitorLibrary(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMan::id() const
{
  return IDD_MGR;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  this->_freeze_mode = enable;

  // disable Mod Hub ComboBox
  this->enableItem(IDC_CB_HUB, !enable);

  // disable menus
  int state = enable ? MF_GRAYED : MF_ENABLED;
  this->setPopupItem(this->_menu, MNU_FILE, state); //< File menu
  this->setPopupItem(this->_menu, MNU_EDIT, state); //< Edit menu
  this->setPopupItem(this->_menu, MNU_TOOL, state); //< Tools menu

  // force menu bar to redraw so enabled/grayed state
  // is properly visually updated
  DrawMenuBar(this->_hwnd);

  // passes the message to child tab dialog
  this->_UiManMain->freeze(enable);
  this->_UiManFoot->freeze(enable);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
/*
  if(enable)
    this->monitorLibrary(false);

  this->_UiManMain->safemode(enable);
  this->_UiManFoot->safemode(enable);
*/
  this->setPopupItem(MNU_EDIT, MNU_EDIT_HUB, enable?MF_GRAYED:MF_ENABLED);
  this->setPopupItem(MNU_EDIT, MNU_EDIT_CHN, enable?MF_GRAYED:MF_ENABLED);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::monitorLibrary(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::monitorLibrary (" << enable << ")\n";
  #endif

  // stops current directory monitoring thread
  if(this->_lib_monitor_hth) {

    // set custom event to request thread quit, then wait for it
    SetEvent(this->_lib_monitor_hev[0]);
    WaitForSingleObject(this->_lib_monitor_hth, INFINITE);
    CloseHandle(this->_lib_monitor_hth);
    this->_lib_monitor_hth = nullptr;

    // reset the "stop" event for further usage
    ResetEvent(this->_lib_monitor_hev[0]);

    // close previous folder monitor
    FindCloseChangeNotification(this->_lib_monitor_hev[1]);
    this->_lib_monitor_hev[1] = nullptr;
  }

  if(enable) {

    OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

    OmModChan* ModChan = ModMan->activeChannel();
    if(!ModChan)
      return;

    // create a new folder change notification event
    DWORD mask =  FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|
                  FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE;

    this->_lib_monitor_hev[1] = FindFirstChangeNotificationW(ModChan->libraryPath().c_str(), false, mask);

    // launch new thread to handle notifications
    DWORD dwId;
    this->_lib_monitor_hth = CreateThread(nullptr, 0, OmUiMan::_lib_monitor_fth, this, 0, &dwId);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMan::_lib_monitor_fth(void* arg)
{
  OmUiMan* self = static_cast<OmUiMan*>(arg);

  DWORD dwObj;

  while(true) {

    dwObj = WaitForMultipleObjects(2, self->_lib_monitor_hev, false, INFINITE);

    if(dwObj == 0) //< custom "stop" event
      break;

    if(dwObj == 1) { //< folder content changed event

      #ifdef DEBUG
      std::cout << "DEBUG => OmUiMan::_lib_monitor_fth (changes)\n";
      #endif

      OmModMan* ModMan = static_cast<OmModMan*>(self->_data);

      if(ModMan) {
        if(ModMan->activeChannel()) {//< this should be always the case
          if(ModMan->activeChannel()->refreshLibraries()) {
            self->_UiManMain->refreshLibrary();
          }
        }
      }

      FindNextChangeNotification(self->_lib_monitor_hev[1]);
    }
  }
  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::openHub(const OmWString& path)
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // Try to open Mod Hub
  if(ModMan->openHub(path)) {

    // enable library monitor
    this->monitorLibrary(true);

    // refresh
    this->refresh();

  } else {

    Om_dlgBox_okl(this->_hwnd, L"Open Mod Hub", IDI_ERR,
                 L"Mod Hub open error", L"Mod Hub "
                 "loading failed because of the following error:",
                 ModMan->lastError());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::closeHub()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::closeHub\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // close the current context
  ModMan->closeHub();

  // enable library monitor
  this->monitorLibrary(true);

  // refresh
  this->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::selectHub(int32_t id)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::selectHub " << id << "\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // select the requested Mod Hub
  ModMan->selectHub(id);

  // enable library monitor
  this->monitorLibrary(true);

  // refresh all
  this->refresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::selectChannel(int32_t id)
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  if(ModHub->activeChannelIndex() == id)
    return;

  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::selectChannel " << id << "\n";
  #endif

  // stop Library folder monitoring
  this->monitorLibrary(false);

  // disable "Edit > Mod Pack []" in main menu
  this->setPopupItem(MNU_EDIT, MNU_EDIT_MOD, MF_GRAYED);

  // select the requested Mod Channel
  if(ModHub) {

    ModHub->selectChannel(id);

    OmModChan* ModChan = ModHub->activeChannel();

    if(ModChan) {

      // Check Mod Channel Library folder access
      if(ModChan->libraryDirAccess(false)) { //< check only for reading
        // force refresh library
        ModChan->refreshModLibrary();
        // start Library folder monitoring
        this->monitorLibrary(true);
      }

      // enable the "Edit > Mod Channel properties..." menu
      this->setPopupItem(MNU_EDIT, MNU_EDIT_CHN, MF_ENABLED);

    } else {

      // disable the "Edit > Mod Channel properties..." menu
      this->setPopupItem(MNU_EDIT, MNU_EDIT_CHN, MF_GRAYED);
    }
  }

  // update status icons
  LVITEMW lvI = {LVIF_IMAGE, 0, 0};

  for(size_t i = 0; i < ModHub->channelCount(); ++i) {

    lvI.iItem = static_cast<int>(i);
    lvI.iImage = (lvI.iItem == ModHub->activeChannelIndex()) ? ICON_STS_RB1 : ICON_STS_RB0;
    this->msgItem(IDC_LV_CHN, LVM_SETITEM, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  this->_UiManMain->refresh();
  this->_UiManFoot->refresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::createPreset()
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  OmUiAddPst* UiNewBat = static_cast<OmUiAddPst*>(this->childById(IDD_ADD_PST));

  UiNewBat->setModHub(ModHub);

  UiNewBat->open(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::deletePreset()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_PST, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_PST, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  // search for Preset internal Index stored in ListView item lParam
  LVITEMW lvI = {LVIF_PARAM, lv_sel, 0}; lvI.lParam = -1;
  this->msgItem(IDC_LV_PST, LVM_GETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  if(lvI.lParam < 0) return;

  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  OmModPset* ModPset = ModHub->getPreset(lvI.lParam);

  // warns the user before committing the irreparable
  if(!Om_dlgBox_ynl(this->_hwnd, L"Mod Hub properties", IDI_QRY, L"Delete Preset",
                    L"Delete the following Preset ?", ModPset->title()))
    return;

  if(!ModHub->deletePreset(lvI.lParam)) {
    // warns the user error occurred
    Om_dlgBox_okl(this->_hwnd, L"Mod Hub properties", IDI_ERR, L"Preset delete error",
                  L"Unable to delete Preset:",ModHub->lastError());

    return;
  }

  // reload the Presets ListView
  this->_lv_pst_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::runPreset()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_PST, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_PST, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  // search for Preset internal Index stored in ListView item lParam
  LVITEMW lvI = {LVIF_PARAM, lv_sel, 0}; lvI.lParam = -1;
  this->msgItem(IDC_LV_PST, LVM_GETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  if(lvI.lParam < 0) return;

  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  this->_setups_add(ModHub->getPreset(lvI.lParam));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::presetProperties()
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_PST, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_PST, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  // search for Preset internal Index stored in ListView item lParam
  LVITEMW lvI = {LVIF_PARAM, lv_sel, 0}; lvI.lParam = -1;
  this->msgItem(IDC_LV_PST, LVM_GETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  if(lvI.lParam < 0) return;

  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  OmUiPropPst* UiPropPst = static_cast<OmUiPropPst*>(this->childById(IDD_PROP_PST));

  UiPropPst->setModPset(ModHub->getPreset(lvI.lParam));

  UiPropPst->open();

  // reload the Presets ListView
  this->_lv_pst_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::checkTargetWrite(const OmWString& operation)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return false;

  // checks whether we have a valid Target directory
  if(!ModChan->targetDirAccess(true)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, operation, IDI_ERR, L"Target directory access error",
                  L"The Target directory cannot be accessed because it do not exist or have read or write "
                  "access restrictions. Please check Mod Channel settings and directory permissions.",
                  ModChan->targetPath());
    return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::checkLibraryRead(const OmWString& operation)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return false;

  // checks whether we have a valid Library directory
  if(!ModChan->libraryDirAccess(false)) { //< check only for read
    Om_dlgBox_okl(this->_hwnd, operation, IDI_ERR, L"Library directory access error",
                  L"The Library directory cannot be accessed because it do not exist or have read "
                  "access restrictions. Please check Mod Channel settings and directory permissions.",
                  ModChan->libraryPath());
    return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::checkLibraryWrite(const OmWString& operation)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return false;

  // checks whether we have a valid Library directory
  if(!ModChan->libraryDirAccess(true)) { //< check only for read
    Om_dlgBox_okl(this->_hwnd, operation, IDI_ERR, L"Library directory access error",
                  L"The Library directory cannot be accessed because it do not exist or have read or write "
                  "access restrictions. Please check Mod Channel settings and directory permissions.",
                  ModChan->libraryPath());
    return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::checkBackupWrite(const OmWString& operation)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return false;

  // checks whether we have a valid Backup folder
  if(!ModChan->backupDirAccess(true)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, operation, IDI_ERR, L"Backup directory access error",
                  L"The Backup directory cannot be accessed because it do not exist or have read or write "
                  "access restrictions. Please check Mod Channel's settings and directory permissions.",
                  ModChan->backupPath());
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::warnMissings(bool enabled, const OmWString& operation, const OmWStringArray& missings)
{
  if(missings.size() && enabled) {
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_PKG_WRN, L"Missing Mods dependencies",
                      L"Selected Mods have missing dependencies, the following Mods are not available:",
                      Om_concatStrings(missings, L"\r\n")))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::warnOverlaps(bool enabled, const OmWString& operation, const OmWStringArray& overlaps)
{
  if(overlaps.size() && enabled) {
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_PKG_OWR, L"Mods are overlapping",
                      L"Selected Mods are overlapping others, the following Mods will be overwritten:",
                      Om_concatStrings(overlaps, L"\r\n")))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::warnExtraInstalls(bool enabled, const OmWString& operation, const OmWStringArray& depends)
{
  if(depends.size() && enabled) {
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_PKG_ADD, L"Mods with dependencies",
                      L"Selected Mods have dependencies, the following Mods will also be installed:",
                      Om_concatStrings(depends, L"\r\n")))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::warnExtraDownloads(bool enabled, const OmWString& operation, const OmWStringArray& depends)
{
  if(depends.size() && enabled) {
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_PKG_ADD, L"Mods with dependencies",
                      L"Selected Mods have dependencies, the following Mods will also be downloaded:",
                      Om_concatStrings(depends, L"\r\n")))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::warnExtraRestores(bool enabled, const OmWString& operation, const OmWStringArray& overlappers, const OmWStringArray& dependents)
{
  if(overlappers.size() && enabled) {
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_PKG_OWR, L"Overlapped Mods",
                      L"Selected Mods are overlapped by others, the following Mods must also be uninstalled:",
                      Om_concatStrings(overlappers, L"\r\n")))
      return false;
  }

  if(dependents.size() && enabled) {
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_PKG_WRN, L"Dependency Mods",
                      L"Selected Mods are required as dependency, the following Mods will also be uninstalled:",
                      Om_concatStrings(dependents, L"\r\n")))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::warnBreakings(bool enabled, const OmWString& operation, const OmWStringArray& breakings)
{
  if(breakings.size() && enabled) {
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_PKG_WRN, L"Upgrade breaks dependencies",
                      L"Selected Mods replaces whom others depends, upgrading following Mods may break dependencies:",
                      Om_concatStrings(breakings,L"\r\n")))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_setups_abort()
{
  if(this->_setups_count) {

    OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
    if(!ModHub) return;

    ModHub->abortPresets();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_setups_add(OmModPset* ModPset)
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  // if no queued operation, this is global start
  if(!this->_setups_count)
    //this->_update_safemode_status();

  // increase count of queued mod operations
  this->_setups_count++;

  ModHub->queuePresets(ModPset, OmUiMan::_setups_begin_fn, OmUiMan::_setups_progress_fn, OmUiMan::_setups_result_fn, this);

  // Enable 'Abort' and disable 'Download'
  /*
  if(this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT)) {
    this->enableItem(IDC_BC_INST, false);
    this->enableItem(IDC_BC_UNIN, false);
  }
  */
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_setups_begin_fn(void* ptr, uint64_t param)
{
  OmUiMan* self = reinterpret_cast<OmUiMan*>(ptr);

  OmModHub* ModHub = static_cast<OmModMan*>(self->_data)->activeHub();
  if(!ModHub) return;

  // store the currently selected channel to restore it after
  self->_setups_chan_sel = ModHub->activeChannelIndex();

  OmModPset* ModPset = reinterpret_cast<OmModPset*>(param);

  // get index of Preset in Hub, therefore, in ListView
  int32_t lv_index = ModHub->indexOfPreset(ModPset);

  // change status icon
  LVITEMW lvI = {LVIF_IMAGE, lv_index, 0}; lvI.iImage = ICON_STS_WIP;
  self->msgItem(IDC_LV_PST, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::_setups_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmUiMan* self = reinterpret_cast<OmUiMan*>(ptr);

  OmModHub* ModHub = static_cast<OmModMan*>(self->_data)->activeHub();
  if(!ModHub) return false;

  OmModPack* ModPack = reinterpret_cast<OmModPack*>(param);

  int32_t chn_index = ModHub->indexOfChannel(ModPack->ModChan());

  // FIXME: le changement de selection n'est pas visible lors du processus
  self->selectChannel(chn_index);

  #ifdef DEBUG
  std::wcout << L"DEBUG => OmUiMan::_setups_progress_fn : chn_index=" << chn_index << L"\n";
  #endif // DEBUG

  if(self->_UiManMainLib) {
    if(tot > 0) {
      self->_UiManMainLib->execInstalls(OmPModPackArray(1, ModPack), ModHub->presetQuietMode());
    } else {
      self->_UiManMainLib->execRestores(OmPModPackArray(1, ModPack), ModHub->presetQuietMode());
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_setups_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OmUiMan* self = reinterpret_cast<OmUiMan*>(ptr);

  OmModHub* ModHub = static_cast<OmModMan*>(self->_data)->activeHub();
  if(!ModHub) return;

  OmModPset* ModPset = reinterpret_cast<OmModPset*>(param);

  // get index of Preset in Hub, therefore, in ListView
  int32_t lv_index = ModHub->indexOfPreset(ModPset);

  // reset status icon
  LVITEMW lvI = {LVIF_IMAGE, lv_index, 0}; lvI.iImage = ICON_NONE;
  self->msgItem(IDC_LV_PST, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

  // select the initially selected channel
  self->selectChannel(self->_setups_chan_sel);

  #ifdef DEBUG
  std::wcout << L"DEBUG => OmUiMan::_setups_result_fn : _setups_chan_sel=" << self->_setups_chan_sel << L"\n";
  #endif // DEBUG
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_caption_populate()
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);
  OmModHub* ModHub = ModMan->activeHub();

  // update dialog window title
  OmWString caption;
  if(ModHub) caption = ModHub->title() + L" - ";

  this->setCaption(caption + OM_APP_NAME);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_sb_hub_populate()
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);
  OmModHub* ModHub = ModMan->activeHub();

  // update the Mod Hub icon
  HICON hIc = nullptr;

  // get context icon
  if(ModHub)
    if(ModHub->icon())
      hIc = ModHub->icon();

  // Get default icon
  if(!hIc)
    hIc = Om_getShellIcon(SIID_APPLICATION, true);

  // Update icon static control
  hIc = this->setStIcon(IDC_SB_ICON, hIc);

  // Properly delete unused icon
  if(hIc) {
    if(hIc != Om_getShellIcon(SIID_APPLICATION, true)) DeleteObject(hIc);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_menu_recent_populate()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_menu_recent_populate\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // handle to "File > Recent files" pop-up
  HMENU hMenu = this->getPopupItem(MNU_FILE, MNU_FILE_RECENT); //< "File > Recent files" pop-up

  // remove all entry from "File > Recent files >" pop-up except the two last
  // ones which are the separator and and the "Clear history" menu-item
  unsigned n = GetMenuItemCount(hMenu) - 2;
  for(unsigned i = 0; i < n; ++i)
    RemoveMenu(hMenu, 0, MF_BYPOSITION);

  // get recent files path list from manager
  OmWStringArray path;
  ModMan->loadRecentFiles(path);

  // add the recent file path or disable popup
  if(path.size()) {

    OmWString mn_entry;

    for(size_t i = 0; i < path.size(); ++i) {

      mn_entry = std::to_wstring(path.size()-i);
      mn_entry += L" ";
      mn_entry += path[i];

      InsertMenuW(hMenu, 0, MF_BYPOSITION|MF_STRING, IDM_FILE_RECENT_PATH + i, mn_entry.c_str());
    }
    // enable the "File > Recent Files" popup
    this->setPopupItem(MNU_FILE, MNU_FILE_RECENT, MF_ENABLED);
  } else {
    // disable the "File > Recent Files" popup
    this->setPopupItem(MNU_FILE, MNU_FILE_RECENT, MF_GRAYED);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_cb_hub_populate()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_cb_hub_populate\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // empty the ComboBox
  this->msgItem(IDC_CB_HUB, CB_RESETCONTENT);

  // add Mod Hub(s) to ComboBox
  if(ModMan->hubCount()) {

    OmWString cb_entry;

    for(unsigned i = 0; i < ModMan->hubCount(); ++i) {

      cb_entry = ModMan->getHub(i)->title();
      cb_entry += L" - ";
      cb_entry += ModMan->getHub(i)->home();

      this->msgItem(IDC_CB_HUB, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(cb_entry.c_str()));
    }

    // If no context selected, force the selection of the first context in list
    if(!ModMan->activeHub())
      ModMan->selectHub(0);

    // select context according current active one
    this->msgItem(IDC_CB_HUB, CB_SETCURSEL, ModMan->activeHubIndex());

    // enable the ComboBox control
    this->enableItem(IDC_CB_HUB, true);

  } else {

    // no selection
    this->msgItem(IDC_CB_HUB, CB_SETCURSEL, -1);

    // disable the ComboBox control
    this->enableItem(IDC_CB_HUB, false);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_cb_hub_on_selchange()
{
  int cb_sel = this->msgItem(IDC_CB_HUB, CB_GETCURSEL);

  if(cb_sel >= 0) this->selectHub(cb_sel);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_lv_chn_populate()
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // if icon size changed we must 'reload' ImageList
  if(this->_lv_chn_icons_size != ModMan->iconsSize()) {
    LPARAM lparam = reinterpret_cast<LPARAM>(this->_listview_himl);
    this->msgItem(IDC_LV_CHN, LVM_SETIMAGELIST, LVSIL_SMALL, lparam);
    this->msgItem(IDC_LV_CHN, LVM_SETIMAGELIST, LVSIL_NORMAL, lparam);
    this->_lv_chn_icons_size = ModMan->iconsSize(); //< update size
  }

  OmModHub* ModHub = ModMan->activeHub();

  if(!ModHub) {
    // empty ListView
    this->msgItem(IDC_LV_CHN, LVM_DELETEALLITEMS);
    // disable ListView
    this->enableItem(IDC_LV_CHN, false);
    // force to reset current selection
    this->selectChannel(-1);
    // return now
    return;
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_CHN, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty ListView
  this->msgItem(IDC_LV_CHN, LVM_DELETEALLITEMS);

  // Check for presence of Mod Channel
  if(ModHub->channelCount()) {

    // add Mod Chan(s) to ListView
    for(size_t i = 0; i < ModHub->channelCount(); ++i) {

      OmModChan* ModChan = ModHub->getChannel(i);

      LVITEMW lvI = {LVIF_IMAGE, static_cast<int>(i), 0};
      lvI.iImage = (ModHub->activeChannelIndex() == static_cast<int>(i)) ? ICON_STS_RB1 : ICON_STS_RB0;
      this->msgItem(IDC_LV_CHN, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

      lvI.iSubItem = 1; lvI.mask = LVIF_TEXT|LVIF_IMAGE; lvI.iImage = ICON_CHN;
      lvI.pszText = const_cast<LPWSTR>(ModChan->title().c_str());
      this->msgItem(IDC_LV_CHN, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

      lvI.iSubItem = 2; lvI.mask = LVIF_TEXT;
      lvI.pszText = const_cast<LPWSTR>(ModChan->targetPath().c_str());
      this->msgItem(IDC_LV_CHN, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
    }

    // we enable the ListView
    this->enableItem(IDC_LV_CHN, true);

    // resize ListView columns adapted to client area
    this->_lv_chn_on_resize();

    // update ListView selection
    this->_lv_chn_on_selchg();

  } else {

    // disable Mod Channel ComboBox
    this->enableItem(IDC_LV_CHN, false);

    // ask user to create at least one Mod Channel in the Mod Hub
    if(Om_dlgBox_yn(this->_hwnd, L"Mod Hub", IDI_QRY,
                  L"Mod Hub is empty", L"The selected Mod Hub is "
                  "empty and have no Mod Channel configured. Do you want "
                  "to add a Mod Channel now ?"))
    {
      OmUiAddChn* UiAddLoc = static_cast<OmUiAddChn*>(this->childById(IDD_ADD_CHN));

      UiAddLoc->ctxSet(ModHub);

      UiAddLoc->open(true);
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_lv_chn_on_resize()
{
  LONG size[4], half_s;

  // Resize the Mods ListView column
  GetClientRect(this->getItem(IDC_LV_CHN), reinterpret_cast<LPRECT>(&size));
  half_s = static_cast<float>(size[2]) * 0.5f;
  this->msgItem(IDC_LV_CHN, LVM_SETCOLUMNWIDTH, 1, (half_s - 140) - 34);
  this->msgItem(IDC_LV_CHN, LVM_SETCOLUMNWIDTH, 2, (half_s + 140) - 34);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_lv_chn_on_selchg()
{
  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_CHN, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  this->selectChannel(lv_sel);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_lv_pst_populate()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_lv_pst_populate\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);
  OmModHub* ModHub = ModMan->activeHub();

  if(!ModHub) {
    // empty ListView
    this->msgItem(IDC_LV_PST, LVM_DELETEALLITEMS);
    // disable ListView
    this->enableItem(IDC_LV_PST, false);
    // update ListView selection
    this->_lv_pst_on_selchg();
    // return now
    return;
  }

  // if icon size changed we must 'reload' ImageList
  if(this->_lv_pst_icons_size != ModMan->iconsSize()) {
    LPARAM lparam = reinterpret_cast<LPARAM>(this->_listview_himl);
    this->msgItem(IDC_LV_PST, LVM_SETIMAGELIST, LVSIL_SMALL, lparam);
    this->msgItem(IDC_LV_PST, LVM_SETIMAGELIST, LVSIL_NORMAL, lparam);
    this->_lv_pst_icons_size = ModMan->iconsSize(); //< update size
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_PST, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty list view
  this->msgItem(IDC_LV_PST, LVM_DELETEALLITEMS);

  for(size_t i = 0; i < ModHub->presetCount(); ++i) {

    OmModPset* ModPset = ModHub->getPreset(i);

    LVITEMW lvI = {LVIF_IMAGE|LVIF_PARAM, static_cast<int>(i), 0};
    lvI.lParam = static_cast<LPARAM>(i); // for Preset index sorting and reordering
    lvI.iImage = ICON_NONE;
    this->msgItem(IDC_LV_PST, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    lvI.iSubItem = 1; lvI.mask = LVIF_TEXT|LVIF_IMAGE; lvI.iImage = ICON_PST;
    lvI.pszText = const_cast<LPWSTR>(ModPset->title().c_str());
    this->msgItem(IDC_LV_PST, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  // we enable the ListView
  this->enableItem(IDC_LV_PST, true);

  // restore ListView scroll position from lvRec
  this->msgItem(IDC_LV_PST, LVM_SCROLL, 0, -lvRec.top );

  // resize ListView columns to adapt client area
  this->_lv_pst_on_resize();

  // update Presets ListView selection
  this->_lv_pst_on_selchg();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_lv_pst_on_resize()
{
  LONG size[4];

  // Resize the Mods ListView column
  GetClientRect(this->getItem(IDC_LV_PST), reinterpret_cast<LPRECT>(&size));
  this->msgItem(IDC_LV_PST, LVM_SETCOLUMNWIDTH, 1, size[2]-55);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_lv_pst_on_selchg()
{
  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_PST, LVM_GETSELECTEDCOUNT);

  this->enableItem(IDC_BC_PSRUN, (lv_nsl > 0));
  this->enableItem(IDC_BC_PSDEL, (lv_nsl > 0));
  this->enableItem(IDC_BC_PSEDI, (lv_nsl > 0));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_lv_pst_on_rclick()
{
  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_PST, LVM_GETSELECTEDCOUNT);

  HMENU hMenu;
  HMENU hPopup;

  // Load the menu resource.
  if((hMenu = LoadMenu(this->hins(), MAKEINTRESOURCE(IDR_MENU_PST))) == nullptr)
      return;

  // TrackPopupMenu cannot display the menu bar so get
  // a handle to the first shortcut menu.
  hPopup = GetSubMenu(hMenu, 0);

  // Disable all menu-item if no item is selected
  if(!lv_nsl) {
    for(unsigned i = 0; i < 6; ++i)
      this->setPopupItem(hPopup, i, MF_GRAYED);
  }

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  // Display the shortcut menu. Track the right mouse
  // button.
  TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,  pt.x, pt.y, 0, this->_hwnd, nullptr);

  // Destroy the menu.
  DestroyMenu(hMenu);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_onShow\n";
  #endif

  this->_UiManMain->show();
  this->_UiManFoot->show();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_onInit\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // Set Presets buttons icons
  this->setBmIcon(IDC_BC_PSRUN, Om_getResIcon(this->_hins, IDI_BT_RUN));
  this->setBmIcon(IDC_BC_PSNEW, Om_getResIcon(this->_hins, IDI_BT_ADD));
  this->setBmIcon(IDC_BC_PSEDI, Om_getResIcon(this->_hins, IDI_BT_MOD));
  this->setBmIcon(IDC_BC_PSDEL, Om_getResIcon(this->_hins, IDI_BT_REM));

  // new array for the icons res ID
  uint32_t idb[OM_LISTVIEW_ICON_COUNT];

  // initialize array with the 16px icons res ID
  Om_setupLvIconsDb(idb);

  uint32_t shift = 0;
  switch(ModMan->iconsSize())
  {
  case 24: shift = 1; break; //< shift to 24px icons res ID
  case 32: shift = 2; break; //< shift to 32px icons res ID
  }

  // Create ImageList and fill it with bitmaps
  this->_listview_himl = ImageList_Create(ModMan->iconsSize(), ModMan->iconsSize(), ILC_COLOR32, 0, OM_LISTVIEW_ICON_COUNT);

  for(uint32_t i = 0; i < OM_LISTVIEW_ICON_COUNT; ++i)
    ImageList_Add((HIMAGELIST)this->_listview_himl, Om_getResImage(this->_hins, idb[i]+shift), nullptr);

  this->_listview_himl_size =  ModMan->iconsSize();

  // load startup Mod Hub files if any
  bool autoload;
  OmWStringArray path_ls;

  ModMan->loadStartHubs(&autoload, path_ls);

  if(autoload) {

    OmWStringArray remv_ls; //< in case we must remove entries

    for(size_t i = 0; i < path_ls.size(); ++i) {

      if(!ModMan->openHub(path_ls[i], false)) {

        Om_dlgBox_okl(this->_hwnd, L"Mod Hub startup autoload", IDI_ERR,
               L"Startup Mod Hub open error", path_ls[i], ModMan->lastError());

        if(Om_dlgBox_ynl(this->_hwnd, L"Mod Hub startup autoload", IDI_WRN,
                       L"Remove startup Mod Hub", L"The following Mod Hub "
                       "cannot be loaded, do you want to remove it from "
                       "startup load list ?", path_ls[i])) {

          remv_ls.push_back(path_ls[i]);
        }
      }
    }

    // Remove invalid startup Mod Hub
    if(remv_ls.size()) {

      for(size_t i = 0; i < remv_ls.size(); ++i)
        path_ls.erase(find(path_ls.begin(), path_ls.end(), remv_ls[i]));

      ModMan->saveStartHubs(autoload, path_ls);
    }

    // we select the first Mod Hub in list
    ModMan->selectHub(0);
  }

  // set window icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));

  // Defines fonts for Mod Hub ComboBox
  HFONT hFt = Om_createFont(18, 200, L"Ms Shell Dlg");
  this->msgItem(IDC_CB_HUB, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  this->_createTooltip(IDC_CB_HUB,    L"Select active Mod Hub");
  this->_createTooltip(IDC_CB_CHN,    L"Select active Mod Channel");

  this->_createTooltip(IDC_BC_PSRUN,    L"Run Preset");
  this->_createTooltip(IDC_BC_PSNEW,    L"New Preset");
  this->_createTooltip(IDC_BC_PSEDI,    L"Edit Preset");
  this->_createTooltip(IDC_BC_PSDEL,    L"Delete Preset");

  // retrieve saved window rect values
  RECT rec = {0,0,0,0};
  ModMan->loadWindowRect(rec);

  // window size
  int w = rec.right - rec.left;
  int h = rec.bottom - rec.top;
  if(w <= 0) w = 505; //< default width
  if(h <= 0) h = 340; //< default height

  // window placement flags
  unsigned uFlags = SWP_NOZORDER;

  // check whether any top-left or top-right window corner is outside visible desktop
  if(!MonitorFromPoint({rec.top, rec.left}, MONITOR_DEFAULTTONULL) ||
     !MonitorFromPoint({rec.top, rec.right}, MONITOR_DEFAULTTONULL) ) {
    uFlags |= SWP_NOMOVE; //< keep window default placement by the system
  }

  // move window rect to the last saved
  SetWindowPos(this->_hwnd, nullptr, rec.left, rec.top, w, h, uFlags);

  // create frames dialogs
  this->_UiManMain->modeless(true);
  this->_UiManFoot->modeless(true);

  // initialize frames to the proper size and position
  h = -1;
  ModMan->loadWindowFoot(&h);
  if(h < FOOT_MIN_HEIGHT) h = FOOT_MIN_HEIGHT;
  // we emulate frame resize by user
  this->_split_curs_dragg = true;
  this->_split_move_param[2] = h;
  this->_onResize();
  this->_split_curs_dragg = false;

  // get Library dialog, required for Presets
  this->_UiManMainLib = static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB));

  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_onInit : _UiManMainLib = " << (uint64_t)this->_UiManMainLib << "\n";
  #endif

  // Set default context icon
  HICON hIc = Om_getShellIcon(SIID_APPLICATION, true);
  this->msgItem(IDC_SB_ICON, STM_SETICON, reinterpret_cast<WPARAM>(hIc));

  // Shared Image list for ListView controls
  LPARAM himl = reinterpret_cast<LPARAM>(this->_listview_himl);

  // ListView collumn struct
  LVCOLUMNW lvC = {};

  // Initialize Mod Channels ListView control with explorer theme
  this->msgItem(IDC_LV_CHN, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_DOUBLEBUFFER);
  SetWindowTheme(this->getItem(IDC_LV_CHN),L"EXPLORER",nullptr);
  // we now add columns into our ListView control
  lvC.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
  // status icon
  lvC.fmt = LVCFMT_RIGHT;
  lvC.iSubItem = 0; lvC.cx = 30; lvC.pszText = const_cast<LPWSTR>(L"");
  this->msgItem(IDC_LV_CHN, LVM_INSERTCOLUMNW, lvC.iSubItem, reinterpret_cast<LPARAM>(&lvC));
  // channel name
  lvC.fmt = LVCFMT_LEFT;
  lvC.iSubItem++; lvC.cx = 300; lvC.pszText = const_cast<LPWSTR>(L"Channel");
  this->msgItem(IDC_LV_CHN, LVM_INSERTCOLUMNW, lvC.iSubItem, reinterpret_cast<LPARAM>(&lvC));
  // target path
  lvC.fmt = LVCFMT_LEFT;
  lvC.iSubItem++; lvC.cx = 300; lvC.pszText = const_cast<LPWSTR>(L"Target");
  this->msgItem(IDC_LV_CHN, LVM_INSERTCOLUMNW, lvC.iSubItem, reinterpret_cast<LPARAM>(&lvC));
  // set shared ImageList
  this->msgItem(IDC_LV_CHN, LVM_SETIMAGELIST, LVSIL_SMALL, himl);
  this->msgItem(IDC_LV_CHN, LVM_SETIMAGELIST, LVSIL_NORMAL, himl);

  // Initialize Presets ListView control with explorer theme
  this->msgItem(IDC_LV_PST, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_DOUBLEBUFFER);
  SetWindowTheme(this->getItem(IDC_LV_PST),L"EXPLORER",nullptr);
  // we now add columns into Presets ListView control
  lvC.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
  // status icon
  lvC.fmt = LVCFMT_RIGHT;
  lvC.iSubItem = 0; lvC.cx = 30; lvC.pszText = const_cast<LPWSTR>(L"");
  this->msgItem(IDC_LV_PST, LVM_INSERTCOLUMNW, lvC.iSubItem, reinterpret_cast<LPARAM>(&lvC));
  // presets name
  lvC.fmt = LVCFMT_LEFT;
  lvC.iSubItem++; lvC.cx = 144; lvC.pszText = const_cast<LPWSTR>(L"Presets");
  this->msgItem(IDC_LV_PST, LVM_INSERTCOLUMNW, lvC.iSubItem, reinterpret_cast<LPARAM>(&lvC));
  // set shared ImageList
  this->msgItem(IDC_LV_PST, LVM_SETIMAGELIST, LVSIL_SMALL, himl);
  this->msgItem(IDC_LV_PST, LVM_SETIMAGELIST, LVSIL_NORMAL, himl);

  // refresh all elements
  this->_onRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_onResize()
{
  // ignore resizing when in minimized state
  if(this->minimized())
    return;

  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_onResize\n";
  #endif

  long y, h, rc[4];

  if(!this->_split_curs_dragg) {

    // Mod Hubs ComboBox
    this->_setItemPos(IDC_CB_HUB, 4, 5, this->cliWidth()-46 , 28, true);
    // Mod Hub Icon
    this->_setItemPos(IDC_SB_ICON, this->cliWidth()-37, 1, 32, 32, true);
    // Mod Channel ListView
    this->_setItemPos(IDC_LV_CHN, 4, 35, this->cliWidth()-8, 78, true);
    this->_lv_chn_on_resize(); //< Resize the Mod Channel ListView column

    // set Presets buttons Position
    this->_setItemPos(IDC_BC_PSRUN, this->cliWidth() - 204, 116, 22, 22, true);
    this->_setItemPos(IDC_BC_PSNEW, this->cliWidth() - 72, 116, 22, 22, true);
    this->_setItemPos(IDC_BC_PSDEL, this->cliWidth() - 49, 116, 22, 22, true);
    this->_setItemPos(IDC_BC_PSEDI, this->cliWidth() - 26, 116, 22, 22, true);

  } else {

    // store old foot frame rect for future redraw
    GetWindowRect(this->_UiManFoot->hwnd(), reinterpret_cast<LPRECT>(&rc));
    MapWindowPoints(HWND_DESKTOP, this->_hwnd, reinterpret_cast<LPPOINT>(&rc), 2);
  }

  // get foot frame height, if we are in frame resize process we get it
  // from the temporary value stored after WM_MOUSEMOVE message
  h = (this->_split_curs_dragg) ? this->_split_move_param[2] : this->_UiManFoot->height();

  // Make sure frame height got correct value
  if(h < FOOT_MIN_HEIGHT) h = FOOT_MIN_HEIGHT;
  if(h > this->cliHeight() - MAIN_MIN_HEIGHT) h = this->cliHeight() - MAIN_MIN_HEIGHT;

  // foot frame top position, relative to client
  y = this->cliHeight() - h;

  // resize and move frames
  this->_setChildPos(this->_UiManFoot->hwnd(), 4, y - 4 , this->cliWidth() - 8, h, true);
  this->_setChildPos(this->_UiManMain->hwnd(), 4, 117, this->cliWidth() - 212, y - 125, true);

  // resize Presets ListView
  this->_setItemPos(IDC_LV_PST, this->cliWidth() - 204, 141, 200, y - 150, true);
  this->_lv_pst_on_resize(); //< Resize the Mod Channel ListView column

  if(!this->_split_curs_dragg) {

    // simple resize, we can redraw entire window
    RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW);

  } else {

    // Calling SetWindowPos outside a resize modal loop (standard resize by user),
    // causes insane amount of flickering, probably due to suboptimal erase and
    // redraw sequences from child to parent.
    //
    // I tested almost every approach, handling WM_NCCALCSIZE and WM_ERASEBKGND
    // window messages, implementing a hierachical DeferWindowPos mechanism,
    // NOTHING WORKED ! The resize modal loop seem to use some internal bypass to
    // gracefully redraw the whole window at once without flickering. Thing that I
    // cannot reproduce (but THIS is what should be done), since this is
    // documented nowhere.
    //
    // The only thing that reduce significantly the flickering where to prevent
    // ALL call to SetWindowPos to redraw by including the SWP_NOREDRAW flag, then
    // handling redraw 'manually' from the parent window, using only RedrawWindow
    // with the proper flags.

    // update the footer frame and area around the splitter, without
    // erasing window background to reduce flickering.
    rc[1] -= 56; //< encompasses the bottom row of buttons and h-scroll of the main frame
    RedrawWindow(this->_hwnd, reinterpret_cast<RECT*>(&rc), nullptr, RDW_INVALIDATE|RDW_UPDATENOW);

    // force redraw only the area between the two frames, now we erase
    // the window background
    rc[1] = y - 8; rc[3] = y - 4;
    RedrawWindow(this->_hwnd, reinterpret_cast<RECT*>(&rc), nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_onRefresh\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);
  OmModHub* ModHub = ModMan->activeHub();

  // update menus
  int state = ModHub ? MF_ENABLED : MF_GRAYED;
  this->setPopupItem(MNU_FILE, MNU_FILE_CLOSE, state); // File > Close
  this->setPopupItem(MNU_EDIT, MNU_EDIT_HUB, state); // Edit > Mod Hub...
  if(ModHub) {
    // Edit > Mod Channel properties...
    this->setPopupItem(MNU_EDIT, MNU_EDIT_CHN, ModHub->activeChannel() ? MF_ENABLED : MF_GRAYED);
  } else {
    this->setPopupItem(MNU_EDIT, MNU_EDIT_CHN, MF_GRAYED); // Edit > Mod Channel
    this->setPopupItem(MNU_EDIT, MNU_EDIT_MOD, MF_GRAYED); // Edit > Package []
    this->setPopupItem(MNU_EDIT, MNU_EDIT_NET, MF_GRAYED); // Edit > Remote []
  }

  if(this->_listview_himl_size != ModMan->iconsSize()) {

    HIMAGELIST himl = reinterpret_cast<HIMAGELIST>(this->_listview_himl);

    // new array for the icons res ID
    uint32_t idb[OM_LISTVIEW_ICON_COUNT];

    // initialize array with the 16px icons res ID
    Om_setupLvIconsDb(idb);

    uint32_t shift = 0;
    switch(ModMan->iconsSize())
    {
    case 24: shift = 1; break; //< shift to 24px icons res ID
    case 32: shift = 2; break; //< shift to 32px icons res ID
    }

    // set new icon size
    ImageList_SetIconSize(himl, ModMan->iconsSize(), ModMan->iconsSize());

    // reload icon with new size
    for(uint32_t i = 0; i < OM_LISTVIEW_ICON_COUNT; ++i)
      ImageList_Add(himl, Om_getResImage(this->_hins, idb[i]+shift), nullptr);

    this->_listview_himl_size = ModMan->iconsSize();
  }

  this->enableItem(IDC_LV_CHN, (ModHub != nullptr));
  this->enableItem(IDC_LV_PST, (ModHub != nullptr));
  this->enableItem(IDC_BC_PSNEW, (ModHub != nullptr));

  // rebuild the Recent Mod Hub menu
  this->_menu_recent_populate();

  // rebuild the Mod Hub list ComboBox
  this->_cb_hub_populate();

  // update window caption
  this->_caption_populate();

  // update Mod Hub Icon
  this->_sb_hub_populate();

  // update Mod Channel ListView
  this->_lv_chn_populate();

  // update Mod Presets ListView
  this->_lv_pst_populate();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_onClose()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_onClose\n";
  #endif

  // disable the window to notify close/quit request is acknowledged
  EnableWindow(this->_hwnd, false);

  this->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_onQuit\n";
  #endif

  // stop Library folder changes monitoring
  this->monitorLibrary(false);

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  RECT rec;
  GetWindowRect(this->_hwnd, &rec);

  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::_onQuit : Window Rect { left=" << rec.left
                                                    << ", top=" << rec.top
                                                    << ", bottom=" << rec.bottom
                                                    << ", right=" << rec.right << "}\n";
  #endif

  ModMan->saveWindowRect(rec);
  ModMan->saveWindowFoot(this->_UiManFoot->height());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMan::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // release the previously captured mouse for frames move and resize process
  if(uMsg == WM_LBUTTONUP) {
    if(this->_split_curs_dragg) {
      // release captured mouse
      ReleaseCapture();
      // quit the frame move and resize process
      this->_split_curs_dragg = false;
    }
    return 0;
  }

  // if mouse cursor is hovering between frames, checks for left button click
  // by user to capture mouse and entering the frames move and resize process
  if(uMsg == WM_LBUTTONDOWN || (uMsg == WM_PARENTNOTIFY && wParam == WM_LBUTTONDOWN)) {
    if(this->_split_curs_hover) {
      // keeps mouse pointer position and foot frame height at
      // capture to later calculate relative moves and size changes
      this->_split_move_param[0] = HIWORD(lParam);
      this->_split_move_param[1] = this->_UiManFoot->height();
      // capture the mouse
      SetCapture(this->_hwnd);
      // we now are in frame move and resize process
      this->_split_curs_dragg = true;
    }
    return 0;
  }

  // changes the default cursor arrow to north-south resize arrows according
  // cursor hovering between the frames.
  if(uMsg == WM_SETCURSOR) {
    // checks whether cursor is hovering between frames
    if(this->_split_curs_hover) {
      SetCursor(LoadCursor(0,IDC_SIZENS));
      return 1; //< bypass default process
    }
  }

  // track mouse cursor position either to detect whether cursor hover
  // between the frames (to change cursor) or, if we captured cursor, to
  // process the move and resize of the frames
  if(uMsg == WM_MOUSEMOVE) {
    long p = HIWORD(lParam);
    if(GetCapture() == this->_hwnd) {
      // calculate new foot height according new cursor position
      long h = this->_split_move_param[1] + this->_split_move_param[0] - p;
      // clamp to reasonable values
      if(h < FOOT_MIN_HEIGHT) h = FOOT_MIN_HEIGHT;
      if(h > this->cliHeight() - MAIN_MIN_HEIGHT) h = this->cliHeight() - MAIN_MIN_HEIGHT;
      // move the splitter / resize frames
      if(h != this->_UiManFoot->height()) {
        this->_split_move_param[2] = h;
        this->_onResize();
      }
    } else {
      // checks whether mouse cursor is hovering between frames, we take a
      // good margin around the gap to make it easier to catch.
      long y = this->cliHeight() - this->_UiManFoot->height();
      this->_split_curs_hover = (p > (y - 10) && p < (y - 1));
    }
    return 0;
  }

  // The WM_COPYDATA is received if another instance was run
  // with arguments, in this case it pass the argument string
  // to initial instance using WM_COPYDATA
  if(uMsg == WM_COPYDATA) {

    COPYDATASTRUCT* pCd = reinterpret_cast<COPYDATASTRUCT*>(lParam);

    if(pCd->dwData == 42) { //< this mean this is argument line from another instance

      if(pCd->cbData > 3) { //< at least more than a nullchar and two quotes

        // convert ANSI string to proper wide string
        OmWString path;
        Om_fromAnsiCp(&path, reinterpret_cast<char*>(pCd->lpData));

        // check for quotes and removes them
        if(path.back() == L'"' && path.front() == L'"') {
          path.erase(0, 1);  path.pop_back();
        }

        // try to open
        this->openHub(path);
      }
    }

    return 0;
  }

  if(uMsg == WM_NOTIFY) {

    if(LOWORD(wParam) == IDC_LV_CHN) {
      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case NM_DBLCLK:
        this->_lv_chn_on_selchg();
        break;

      case NM_RCLICK:
        //this->_lv_mod_on_rclick();
        break;

      case LVN_ITEMCHANGED:
        this->_lv_chn_on_selchg();
        break;
      }
    }

    if(LOWORD(wParam) == IDC_LV_PST) {
      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case NM_DBLCLK:
        this->runPreset();
        break;

      case NM_RCLICK:
        this->_lv_pst_on_rclick();
        break;

      case LVN_ITEMCHANGED: {
          NMLISTVIEW* nmLv = reinterpret_cast<NMLISTVIEW*>(lParam);
          // detect only selection changes
          if((nmLv->uNewState ^ nmLv->uOldState) & LVIS_SELECTED)
            this->_lv_pst_on_selchg();
          break;
        }
      }
    }

    return false;
  }

  if(uMsg == WM_COMMAND) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMan::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

    // Prevent command/shorcut execution when dialog is not active
    if(!this->active())
      return false;

    OmModMan* ModMan = static_cast<OmModMan*>(this->_data);
    OmModHub* ModHub = ModMan->activeHub();

    OmWString item_str;

    // handle "File > Recent Files" path click
    if(LOWORD(wParam) >= IDM_FILE_RECENT_PATH) { // recent

      OmWStringArray paths;
      ModMan->loadRecentFiles(paths);

      // subtract Command ID by the base resource ID to get real index
      this->openHub(paths[LOWORD(wParam) - IDM_FILE_RECENT_PATH]);
    }

    // Menus and Shortcuts Messages
    switch(LOWORD(wParam))
    {

    case IDC_CB_HUB: //< Mod Hub ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->_cb_hub_on_selchange();
      break;

    case IDC_BC_PSRUN: //< Preset Run Button
      this->runPreset();
      break;

    case IDC_BC_PSNEW: //< Preset New Button
      this->deletePreset();
      break;

    case IDC_BC_PSDEL: //< Preset Delete Button
      this->deletePreset();
      break;

    case IDC_BC_PSEDI: //< Preset Edit Button
      this->presetProperties();
      break;

    // Menu : File []
    case IDM_FILE_NEW_CTX:
      this->childById(IDD_WIZ_HUB)->open(); // New Mod Hub Wizard
      break;

    case IDM_FILE_OPEN:
      if(Om_dlgBrowseDir(item_str, this->_hwnd, L"Select Mod Hub home directory to load it", item_str, false, true)) {
        this->openHub(item_str);
      }
      break;

    case IDM_FILE_CLOSE:
      this->closeHub();
      break;

    case IDM_FILE_CLEAR_HIST:
      ModMan->clearRecentFiles();
      this->_menu_recent_populate();
      break;

    case IDM_FILE_QUIT:
      this->quit();
      break;

    // Menu : Edit []
    case IDM_EDIT_CTX_PROP: {
      OmUiPropHub* pUiPropHub = static_cast<OmUiPropHub*>(this->childById(IDD_PROP_HUB));
      pUiPropHub->setModHub(ModHub);
      pUiPropHub->open();
      break;
    }

    case IDM_EDIT_CHN_PROP: {
      OmUiPropChn* pUiPropLoc = static_cast<OmUiPropChn*>(this->childById(IDD_PROP_CHN));
      pUiPropLoc->setModChan(ModHub->activeChannel());
      pUiPropLoc->open();
      break;
    }

    case IDM_EDIT_ADD_CHN: {
      OmUiAddChn* pUiAddLoc = static_cast<OmUiAddChn*>(this->childById(IDD_ADD_CHN));
      pUiAddLoc->ctxSet(ModHub);
      pUiAddLoc->open();
      break;
    }

    // Menu : Edit > Package > []
    case IDM_EDIT_MOD_INST:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->queueInstalls();
      break;

    case IDM_EDIT_MOD_UINS:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->queueRestores();
      break;

    case IDM_EDIT_MOD_CLNS:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->queueCleaning();
      break;

    case IDM_EDIT_MOD_TRSH:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->deleteSources();
      break;

    case IDM_EDIT_MOD_DISC:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->discardBackups();
      break;

    case IDM_EDIT_MOD_OPEN:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->exploreSources();
      break;

    case IDM_EDIT_MOD_EDIT:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->editSource();
      break;

    case IDM_EDIT_MOD_INFO:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->showProperties();
      break;


    // Menu : Edit > Remote > []
    case IDM_EDIT_NET_DNWS:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->queueDownloads(false);
      break;

    case IDM_EDIT_NET_DNLD:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->queueDownloads(true);
      break;

    case IDM_EDIT_NET_STOP:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->stopDownloads();
      break;

    case IDM_EDIT_NET_RVOK:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->revokeDownloads();
      break;

    case IDM_EDIT_NET_FIXD:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->downloadDepends(false);
      break;

    case IDM_EDIT_NET_INFO:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->showProperties();
      break;

    case IDM_EDIT_OPTIONS:
      this->childById(IDD_PROP_MGR)->open();
      break;

    // Presets Pop-Up menu
    case IDM_PRESET_NEW:
      this->createPreset();
      break;

    case IDM_PRESET_DEL:
      this->deletePreset();
      break;

    case IDM_PRESET_EXE:
      this->runPreset();
      break;

    case IDM_PRESET_EDI:
      this->presetProperties();
      break;

    // Menu : Tools > []
    case IDM_TOOLS_EDI_REP:
      this->childById(IDD_TOOL_REP)->modeless();
      break;

    case IDM_TOOLS_EDI_PKG:
      this->childById(IDD_TOOL_PKG)->modeless();
      break;


    // Menu : Help > []
    case IDM_HELP_LOG:
      this->childById(IDD_HELP_LOG)->modeless();
      break;

    case IDM_HELP_ABOUT:
      this->childById(IDD_HELP_ABT)->open();
      break;
    }
  }

  if(uMsg == WM_EXITSIZEMOVE) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMan::_onMsg : WM_EXITSIZEMOVE\n";
    #endif

  }

  return false;
}

