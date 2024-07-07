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
//#include "OmUiAddRep.h"
//#include "OmUiAddChn.h"
#include "OmUiPropHub.h"
#include "OmUiPropChn.h"
#include "OmUiPropMan.h"
#include "OmUiPropPst.h"
#include "OmUiPropMod.h"
#include "OmUiPropNet.h"
#include "OmUiHelpLog.h"
#include "OmUiHelpAbt.h"
#include "OmUiWizHub.h"
#include "OmUiWizChn.h"
#include "OmUiWizRep.h"
#include "OmUiToolPkg.h"
#include "OmUiToolRep.h"
#include "OmUiPictView.h"

#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiMan.h"

// fixed values for resize and split calculations
#define RSIZE_SEP_H         4
#define RSIZE_SEP_W         4
#define RSIZE_TOP_H         30
#define RSIZE_BOT_H         28
#define RSIZE_SIDE_MIN_W    140
#define RSIZE_HEAD_MIN_H    70
#define RSIZE_MAIN_MIN_H    200
#define RSIZE_MAIN_MIN_W    200
#define RSIZE_FOOT_MIN_H    170

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMan::OmUiMan(HINSTANCE hins) : OmDialog(hins),
  _UiManMain(nullptr),
  _UiManFoot(nullptr),
  _UiManMainLib(nullptr),
  _safe_mode(false),
  _lock_mode(false),
  _lock_idch(-1),
  _split_hover_head(false),
  _split_hover_foot(false),
  _split_captured(false),
  _split_params{},
  _listview_himl(nullptr),
  _listview_himl_size(0),
  _psetup_count(0),
  _psetup_idch(-1),
  _psetup_abort(false),
  _delchan_hth(nullptr),
  _delchan_hwo(nullptr),
  _delchan_idch(-1),
  _delchan_hdlg(nullptr),
  _delchan_abort(0),
  _lv_chn_icons_size(0),
  _lv_chn_show(true),
  _lv_chn_span(RSIZE_HEAD_MIN_H),
  _lv_pst_icons_size(0),
  _lv_pst_show(true),
  _lv_pst_span(RSIZE_SIDE_MIN_W),
  _ui_ovw_show(true),
  _ui_ovw_span(RSIZE_FOOT_MIN_H)
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
  this->addChild(new OmUiWizChn(hins));     //< Dialog for New Mod Channel Wizard
  this->addChild(new OmUiWizRep(hins));     //< Dialog for Repository Config Wizard
  this->addChild(new OmUiAddPst(hins));     //< Dialog for New Preset
  //this->addChild(new OmUiAddChn(hins));     //< Dialog for Adding Mod Channel
  //this->addChild(new OmUiAddRep(hins));     //< Dialog for Add Repository
  this->addChild(new OmUiToolPkg(hins));    //< Dialog for New Package
  this->addChild(new OmUiToolRep(hins));    //< Dialog for Repository Editor
  this->addChild(new OmUiPictView(hins));   //< Dialog for Picture Viewer

  // set the accelerator table for the dialog
  this->setAccel(IDR_ACCEL);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMan::~OmUiMan()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_CB_HUB, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
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
void OmUiMan::enableLockMode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::enableLockMode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // disable/enable Mod Hub ComboBox
  this->enableItem(IDC_CB_HUB, !enable);

  // disable/enable Presets buttons
  this->enableItem(IDC_BC_PSNEW, !enable);
  this->enableItem(IDC_BC_PSDEL, !enable);
  this->enableItem(IDC_BC_PSEDI, !enable);

  // disable/enable Mod Channel ListView
  this->enableItem(IDC_LV_CHN, !enable);

  // disable/enable Repository ListView & buttons
  this->_UiManMainNet->enableItem(IDC_LV_REP, !enable);
  this->_UiManMainNet->enableItem(IDC_BC_RPQRY, !enable);
  this->_UiManMainNet->enableItem(IDC_BC_RPADD, !enable);
  this->_UiManMainNet->enableItem(IDC_BC_RPDEL, !enable);

  if(enable) {
    this->setPopupItem(MNU_FILE, MNU_FILE_NEW, MF_GRAYED);
    this->setPopupItem(MNU_FILE, MNU_FILE_OPEN, MF_GRAYED);
    this->setPopupItem(MNU_FILE, MNU_FILE_RECENT, MF_GRAYED);
    this->setPopupItem(MNU_FILE, MNU_FILE_CLOSE, MF_GRAYED);

    this->setPopupItem(MNU_EDIT, MNU_EDIT_MANPROP, MF_GRAYED);

    for(uint32_t i = 0; i < 5; ++i) {
      this->setPopupItem(MNU_HUB, i, MF_GRAYED);
      this->setPopupItem(MNU_CHN, i, MF_GRAYED);
    }
  } else {
    // disable/enable menu items
    this->_menu_enable();
  }

  this->_lock_mode = enable;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::enableSafeMode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::enableSafeMode (" << enable << ")\n";
  #endif

  if(enable) {
    this->selectChannel(-1);
  }

  //this->monitorLibrary(!enable);

  this->enableLockMode(enable);

  this->_safe_mode = enable;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::openFile()
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmWString result, start;

  ModMan->loadDefaultLocation(start);

  if(Om_dlgOpenFile(result, this->_hwnd, L"Select Hub definition file", OM_HUB_FILES_FILTER, start)) {

    this->openHub(result);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::openRecent(int32_t index)
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmWStringArray paths;
  ModMan->getRecentFileList(paths);

  if((index >= 0) && (static_cast<uint32_t>(index) < paths.size()))
    this->openHub(paths[index]);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::clearRecents()
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  ModMan->clearRecentFileList();

  this->_menu_recent_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::openHub(const OmWString& path)
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  int32_t previous = ModMan->activeHubIndex();

  // unselect Hub
  ModMan->selectHub(-1);

  // refresh
  this->refresh();

  // Try to open Mod Hub
  if(OM_RESULT_OK != ModMan->openHub(path)) {

    Om_dlgBox_okl(this->_hwnd, L"Open Mod Hub", IDI_DLG_ERR, L"Mod Hub open error",
                  L"Unable to open Mod Hub:", ModMan->lastError());

    if(ModMan->hubCount())
      ModMan->selectHub(previous);
  }

  // refresh
  this->refresh();
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

  // refresh
  this->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::selectHub(int32_t index)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::selectHub " << index << "\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // select the requested Mod Hub
  ModMan->selectHub(index);

  // refresh all
  this->refresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::hubProperties()
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  OmUiPropHub* UiPropHub = static_cast<OmUiPropHub*>(this->childById(IDD_PROP_HUB));

  UiPropHub->setModHub(ModHub);

  UiPropHub->open(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::selectChannel(int32_t index)
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  if(ModHub->activeChannelIndex() == index)
    return;

  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMan::selectChannel " << index << "\n";
  #endif

  // disable menus
  this->setPopupItem(MNU_EDIT, MNU_CHN, MF_GRAYED);

  // select the requested Mod Channel
  if(ModHub)
    ModHub->selectChannel(index);

  // update status icons
  LVITEMW lvI = {};
  lvI.mask = LVIF_IMAGE; lvI.iSubItem = 0;

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
void OmUiMan::createChannel()
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  OmUiWizChn* UiWizChn = static_cast<OmUiWizChn*>(this->childById(IDD_WIZ_CHN));

  UiWizChn->setModHub(ModHub);

  UiWizChn->open(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::deleteChannel(int32_t index)
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  // we rely on current active channel since channel may be active but not actually
  // selected in the ListView control

  OmModChan* ModChan;

  if(index  < 0) {
    ModChan = ModHub->activeChannel();
  } else {
    ModChan = ModHub->getChannel(index);
  }

  if(!ModChan) return;

  // warns the user before committing the irreparable
  if(!Om_dlgBox_cal(this->_hwnd, L"Hub properties", IDI_DLG_QRY, L"Delete Channel",
                   L"This operation will permanently delete the following Channel "
                   L"and associated data:", ModChan->title()))
    return;

  // here we go for Mod Channel delete
  if(!this->_delchan_hth) {

    this->_delchan_idch = ModHub->activeChannelIndex();

    // launch thread
    this->_delchan_hth = Om_threadCreate(OmUiMan::_delchan_run_fn, this);
    this->_delchan_hwo = Om_threadWaitEnd(this->_delchan_hth, OmUiMan::_delchan_end_fn, this);
  }

  // reload the Channels ListView
  this->_lv_chn_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::channelProperties()
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  // we rely on current active channel since channel may be active but not actually
  // selected in the ListView control

  OmModChan* ModChan = ModHub->activeChannel();
  if(!ModChan) return;

  OmUiPropChn* UiPropChn = static_cast<OmUiPropChn*>(this->childById(IDD_PROP_CHN));

  UiPropChn->setModChan(ModChan);

  UiPropChn->open(true);
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
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  // we ignore multiple selection
  if(this->msgItem(IDC_LV_PST, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int32_t lv_sel = this->msgItem(IDC_LV_PST, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  OmModPset* ModPset = ModHub->getPreset(lv_sel);

  // warns the user before committing the irreparable
  if(!Om_dlgBox_ynl(this->_hwnd, L"Mod Hub properties", IDI_DLG_QRY, L"Delete Preset",
                    L"Delete the following Preset ?", ModPset->title()))
    return;

  if(!ModHub->deletePreset(lv_sel)) {
    // warns the user error occurred
    Om_dlgBox_okl(this->_hwnd, L"Mod Hub properties", IDI_DLG_ERR, L"Preset delete error",
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
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  // we ignore multiple selection
  if(this->msgItem(IDC_LV_PST, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int32_t lv_sel = this->msgItem(IDC_LV_PST, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  this->_psetup_add(ModHub->getPreset(lv_sel));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::presetProperties()
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  // we ignore multiple selection
  if(this->msgItem(IDC_LV_PST, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView single selection
  int lv_sel = this->msgItem(IDC_LV_PST, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  OmUiPropPst* UiPropPst = static_cast<OmUiPropPst*>(this->childById(IDD_PROP_PST));

  UiPropPst->setModPset(ModHub->getPreset(lv_sel));

  UiPropPst->open();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::openArg(const OmWString& path)
{
  if(Om_extensionMatches(path, OM_PKG_FILE_EXT)) {

    this->openPkgEditor(path);

    return true;
  }

  if(Om_extensionMatches(path, OM_XML_DEF_EXT)) {

    OmXmlConf unknown_cfg;

    if(unknown_cfg.load(path, OM_XMAGIC_CHN)) {

      OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
      if(ModHub) {
        // select channel, which should be in the current active hub
        OmWString uuid = unknown_cfg.child(L"uuid").content();
        int32_t chan_id = ModHub->indexOfChannel(uuid);
        if(chan_id >= 0) this->selectChannel(chan_id);
      }

      unknown_cfg.clear();

      return true;
    }

    if(unknown_cfg.load(path, OM_XMAGIC_REP)) {

      unknown_cfg.clear();

      this->openRepEditor(path);

      return true;
    }

    unknown_cfg.clear();
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::openPkgEditor(const OmWString& path)
{
  OmUiToolPkg* UiToolPkg = static_cast<OmUiToolPkg*>(this->childById(IDD_TOOL_PKG));

  if(UiToolPkg->visible()) {

    UiToolPkg->parseSource(path);

  } else {

    UiToolPkg->setInitParse(path);

    UiToolPkg->modeless(true);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::openRepEditor(const OmWString& path)
{
  OmUiToolRep* UiToolRep = static_cast<OmUiToolRep*>(this->childById(IDD_TOOL_REP));

  if(UiToolRep->visible()) {

    UiToolRep->loadRepository(path);

  } else {

    UiToolRep->setInitLoad(path);

    UiToolRep->modeless(true);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::checkTargetWrite(const OmWString& operation)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return false;

  // checks whether we have a valid Target directory
  if(!ModChan->accessesTarget(OM_ACCESS_DIR_READ|OM_ACCESS_DIR_WRITE)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, operation, IDI_DLG_ERR, L"Target directory access error",
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
  if(!ModChan->accessesLibrary(OM_ACCESS_DIR_READ)) { //< check only for read
    Om_dlgBox_okl(this->_hwnd, operation, IDI_DLG_ERR, L"Library directory access error",
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
  if(!ModChan->accessesLibrary(OM_ACCESS_DIR_READ|OM_ACCESS_DIR_WRITE)) { //< check only for read
    Om_dlgBox_okl(this->_hwnd, operation, IDI_DLG_ERR, L"Library directory access error",
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
  if(!ModChan->accessesBackup(OM_ACCESS_DIR_READ|OM_ACCESS_DIR_WRITE)) { //< check for read and write
    Om_dlgBox_okl(this->_hwnd, operation, IDI_DLG_ERR, L"Backup directory access error",
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
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_DLG_PKG_WRN, L"Missing Mods dependencies",
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
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_DLG_PKG_OWR, L"Mods are overlapping",
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
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_DLG_PKG_ADD, L"Mods with dependencies",
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
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_DLG_PKG_ADD, L"Mods with dependencies",
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
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_DLG_PKG_OWR, L"Overlapped Mods",
                      L"Selected Mods are overlapped by others, the following Mods must also be uninstalled:",
                      Om_concatStrings(overlappers, L"\r\n")))
      return false;
  }

  if(dependents.size() && enabled) {
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_DLG_PKG_WRN, L"Dependency Mods",
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
    if(!Om_dlgBox_cal(this->_hwnd, operation, IDI_DLG_PKG_WRN, L"Upgrade breaks dependencies",
                      L"Selected Mods replaces whom others depends, upgrading following Mods may break dependencies:",
                      Om_concatStrings(breakings,L"\r\n")))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_psetup_add(OmModPset* ModPset)
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  // if no queued operation, this is global start
  if(!this->_psetup_count)
    //this->_refresh_processing();

  // increase count of queued mod operations
  this->_psetup_count++;

  // reset abort status
  this->_psetup_abort = false;

  ModHub->queuePresets(ModPset, OmUiMan::_psetup_begin_fn, OmUiMan::_psetup_progress_fn, OmUiMan::_psetup_result_fn, this);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_psetup_begin_fn(void* ptr, uint64_t param)
{
  OmUiMan* self = reinterpret_cast<OmUiMan*>(ptr);

  OmModHub* ModHub = static_cast<OmModMan*>(self->_data)->activeHub();
  if(!ModHub) return;

  // store the currently selected channel to restore it after
  self->_psetup_idch = ModHub->activeChannelIndex();

  OmModPset* ModPset = reinterpret_cast<OmModPset*>(param);

  // get index of Preset in Hub, therefore, in ListView
  int32_t lv_index = ModHub->indexOfPreset(ModPset);

  // change status icon
  LVITEMW lvI = {};
  lvI.mask = LVIF_IMAGE; lvI.iItem = lv_index; lvI.iSubItem = 0; lvI.iImage = ICON_STS_WIP;
  self->msgItem(IDC_LV_PST, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::_psetup_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OM_UNUSED(cur);

  OmUiMan* self = reinterpret_cast<OmUiMan*>(ptr);

  OmModHub* ModHub = static_cast<OmModMan*>(self->_data)->activeHub();
  if(!ModHub) return false;

  OmModPack* ModPack = reinterpret_cast<OmModPack*>(param);

  int32_t chn_index = ModHub->indexOfChannel(ModPack->ModChan());

  // FIXME: le changement de selection de channel n'est pas visible lors de l'installation du Preset
  self->selectChannel(chn_index);

  #ifdef DEBUG
  std::wcout << L"DEBUG => OmUiMan::_psetup_progress_fn : chn_index=" << chn_index << L"\n";
  #endif // DEBUG

  OmResult result;

  if(self->_UiManMainLib) {
    if(tot > 0) {
      result = self->_UiManMainLib->execInstalls(OmPModPackArray(1, ModPack), ModHub->presetQuietMode());
    } else {
      result = self->_UiManMainLib->execRestores(OmPModPackArray(1, ModPack), ModHub->presetQuietMode());
    }
  } else {
    result = OM_RESULT_ABORT;
  }

  if(result == OM_RESULT_ABORT)
    self->_psetup_abort = true;

  return !self->_psetup_abort;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_psetup_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OM_UNUSED(result);

  OmUiMan* self = reinterpret_cast<OmUiMan*>(ptr);

  OmModHub* ModHub = static_cast<OmModMan*>(self->_data)->activeHub();
  if(!ModHub) return;

  OmModPset* ModPset = reinterpret_cast<OmModPset*>(param);

  // get index of Preset in Hub, therefore, in ListView
  int32_t lv_index = ModHub->indexOfPreset(ModPset);

  // reset status icon
  LVITEMW lvI = {};
  lvI.mask = LVIF_IMAGE; lvI.iItem = lv_index; lvI.iSubItem = 0; lvI.iImage = ICON_NONE;
  self->msgItem(IDC_LV_PST, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

  // select the initially selected channel
  self->selectChannel(self->_psetup_idch);

  #ifdef DEBUG
  std::wcout << L"DEBUG => OmUiMan::_psetup_result_fn : _psetup_chan_sel=" << self->_psetup_idch << L"\n";
  #endif // DEBUG
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiMan::_delchan_run_fn(void* ptr)
{
  OmUiMan* self = static_cast<OmUiMan*>(ptr);

  OmModHub* ModHub = static_cast<OmModMan*>(self->_data)->activeHub();
  if(!ModHub) return static_cast<DWORD>(OM_RESULT_ABORT);

  OmModChan* ModChan = ModHub->getChannel(self->_delchan_idch);
  if(!ModChan) return static_cast<DWORD>(OM_RESULT_ABORT);

  // set UI in safe mode
  static_cast<OmUiMan*>(self->root())->enableSafeMode(true);

  // unselect Channel
  ModHub->selectChannel(-1);

  // Open progress dialog
  self->_delchan_abort = 0;
  self->_delchan_hdlg = Om_dlgProgress(self->_hwnd, L"delete Channel", IDI_DLG_PKG_DEL, L"Restoring backup data", &self->_delchan_abort);

  // delete channel, will purge backup if required
  OmResult result = ModHub->deleteChannel(self->_delchan_idch, OmUiMan::_delchan_progress_fn, self);

  // quit the progress dialog (dialogs must be opened and closed within the same thread)
  Om_dlgProgressClose(static_cast<HWND>(self->_delchan_hdlg));
  self->_delchan_hdlg = nullptr;

  return static_cast<DWORD>(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMan::_delchan_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmUiMan* self = static_cast<OmUiMan*>(ptr);

  OmModHub* ModHub = static_cast<OmModMan*>(self->_data)->activeHub();
  if(!ModHub) return false;

  OmModPack* ModPack = reinterpret_cast<OmModPack*>(param);

  OmWString progress_text = L"Restores backup data: ";
  progress_text += ModPack->iden();

  Om_dlgProgressUpdate(static_cast<HWND>(self->_delchan_hdlg), tot, cur, progress_text.c_str());

  return (self->_delchan_abort == 0);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmUiMan::_delchan_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  OmUiMan* self = static_cast<OmUiMan*>(ptr);

  OmModHub* ModHub = static_cast<OmModMan*>(self->_data)->activeHub();
  if(!ModHub) return;

  DWORD exit_code = Om_threadExitCode(self->_delchan_hth);
  Om_threadClear(self->_delchan_hth, self->_delchan_hwo);

  self->_delchan_hth = nullptr;
  self->_delchan_hwo = nullptr;

  OmResult result = static_cast<OmResult>(exit_code);

  if(result == OM_RESULT_ERROR) {
      // an error occurred during backup purge
      Om_dlgBox_okl(self->_hwnd, L"Delete Channel", IDI_DLG_WRN, L"Delete channel error",
                    L"Deletion process encountered error(s):", ModHub->lastError() );
  }

  // restore UI interactions
  static_cast<OmUiMan*>(self->root())->enableSafeMode(false);

  // Select fist available Channel
  ModHub->selectChannel(0);

  // refresh all dialogs from root
  self->refresh();
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
void OmUiMan::_status_populate()
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);
  OmModHub* ModHub = ModMan->activeHub();

  OmWString path;

  if(ModHub) {
    path = ModHub->home();
    if(path.back() != L'\\') path.push_back(L'\\');
  }

  this->setItemText(IDC_SC_FILE, path);
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
  ModMan->getRecentFileList(path);

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
void OmUiMan::_menu_enable()
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  this->setPopupItem(MNU_FILE, MNU_FILE_NEW, MF_ENABLED);
  this->setPopupItem(MNU_FILE, MNU_FILE_OPEN, MF_ENABLED);
  this->setPopupItem(MNU_FILE, MNU_FILE_RECENT, MF_ENABLED);

  this->setPopupItem(MNU_EDIT, MNU_EDIT_MANPROP, MF_ENABLED);

  uint32_t has_hub_stat = ModMan->activeHub() ? MF_ENABLED : MF_GRAYED;
  uint32_t has_chn_stat = ModMan->activeChannel() ? MF_ENABLED : MF_GRAYED;

  this->setPopupItem(MNU_FILE, MNU_FILE_CLOSE, has_hub_stat);

  for(uint32_t i = 0; i < 8; ++i) {
    this->setPopupItem(MNU_HUB, i, has_hub_stat);
    this->setPopupItem(MNU_CHN, i, has_chn_stat);
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
      //cb_entry += L" - ";
      //cb_entry += ModMan->getHub(i)->home();

      this->msgItem(IDC_CB_HUB, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(cb_entry.c_str()));
    }

    // If no context selected, force the selection of the first context in list
    /*
    if(!ModMan->activeHub())
      ModMan->selectHub(0);
    */

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

    LVITEMW lvI = {};

    // add Mod Chan(s) to ListView
    for(size_t i = 0; i < ModHub->channelCount(); ++i) {

      OmModChan* ModChan = ModHub->getChannel(i);

      lvI.iItem = static_cast<int>(i);

      lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE;
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
    if(Om_dlgBox_yn(this->_hwnd, L"Mod Hub", IDI_DLG_QRY, L"Adding Channel",
                    L"This Hub is empty and have no Channel configured. Do you want to add a modding Channel now ?"))
    {
      this->createChannel();
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
  // Get ListView single selection
  int32_t lv_sel = this->msgItem(IDC_LV_CHN, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0) return;

  this->selectChannel(lv_sel);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_lv_chn_on_rclick()
{
  // get Popup submenu from hidden context menu
  HMENU hPopup = this->getContextPopup(POP_CHN);
  if(!hPopup) return;

  // enable or disable menu-items according selection
  uint32_t item_stat = static_cast<OmModMan*>(this->_data)->activeChannel() ? MF_ENABLED : MF_GRAYED;
  this->setPopupItem(hPopup, POP_CHN_DEL,  item_stat);
  this->setPopupItem(hPopup, POP_CHN_PROP, item_stat);

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  // display popup menu along mouse cursor
  TrackPopupMenu(hPopup, TPM_LEFTALIGN|TPM_RIGHTBUTTON,  pt.x, pt.y, 0, this->_hwnd, nullptr);
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

  LVITEMW lvI = {};

  for(size_t i = 0; i < ModHub->presetCount(); ++i) {

    OmModPset* ModPset = ModHub->getPreset(i);

    lvI.iItem = static_cast<int>(i);

    // status icon collumn
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE|LVIF_PARAM;
    lvI.iImage = ICON_NONE; lvI.lParam = static_cast<LPARAM>(i); //< for Preset index sorting and reordering
    this->msgItem(IDC_LV_PST, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
    // preset name collumn
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
  // check for selection
  bool is_selected = (this->msgItem(IDC_LV_PST, LVM_GETSELECTEDCOUNT) > 0);
  this->enableItem(IDC_BC_PSRUN, is_selected);
  this->enableItem(IDC_BC_PSDEL, (this->_lock_mode) ? false : is_selected);
  this->enableItem(IDC_BC_PSEDI, (this->_lock_mode) ? false : is_selected);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_lv_pst_on_rclick()
{
  // get Popup submenu from hidden context menu
  HMENU hPopup = this->getContextPopup(POP_PST);
  if(!hPopup) return;

  // enable or disable menu-items according selection
  uint32_t item_stat = (this->msgItem(IDC_LV_PST, LVM_GETSELECTEDCOUNT) > 0) ? MF_ENABLED : MF_GRAYED;
  this->setPopupItem(hPopup, POP_PST_RUN,  item_stat);
  this->setPopupItem(hPopup, POP_PST_DEL,  (this->_lock_mode) ? MF_GRAYED : item_stat);
  this->setPopupItem(hPopup, POP_PST_PROP, (this->_lock_mode) ? MF_GRAYED : item_stat);

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  // display popup menu along mouse cursor
  TrackPopupMenu(hPopup, TPM_LEFTALIGN|TPM_RIGHTBUTTON,  pt.x, pt.y, 0, this->_hwnd, nullptr);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_layout_save()
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  ModHub->setLayoutChannelsSpan(this->_lv_chn_span);
  ModHub->setLayoutPresetsSpan(this->_lv_pst_span);
  ModHub->setLayoutOverviewSpan(this->_ui_ovw_span);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMan::_layout_load()
{
  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  this->_lv_chn_show = ModHub->layoutChannelsShow();
  this->_lv_chn_span = ModHub->layoutChannelsSpan();

  // show or hide Channels list
  this->showItem(IDC_LV_CHN, this->_lv_chn_show);
  this->showItem(IDC_BC_CHEDI, this->_lv_chn_show);
  this->showItem(IDC_BC_CHADD, this->_lv_chn_show);
  this->showItem(IDC_BC_CHDEL, this->_lv_chn_show);

  this->_lv_pst_show = ModHub->layoutPresetsShow();
  this->_lv_pst_span = ModHub->layoutPresetsSpan();

  // show or hide Presets list
  this->showItem(IDC_LV_PST, this->_lv_pst_show);
  this->showItem(IDC_BC_PSRUN, this->_lv_pst_show);
  this->showItem(IDC_BC_PSNEW, this->_lv_pst_show);
  this->showItem(IDC_BC_PSDEL, this->_lv_pst_show);
  this->showItem(IDC_BC_PSEDI, this->_lv_pst_show);

  this->_ui_ovw_show = true/*ModHub->layoutOverviewShow()*/;
  this->_ui_ovw_span = ModHub->layoutOverviewSpan();

  this->_onResize();

  // redraw entire client
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
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

  // set menu icons
  HMENU hMnuFile = this->getPopup(MNU_FILE);

  //this->setPopupItemIcon(hMnuFile, MNU_FILE_NEW, Om_getResIconPremult(IDI_BT_NEW));
  this->setPopupItemIcon(hMnuFile, MNU_FILE_NEW, Om_getResIconPremult(IDI_BT_NEW));
  this->setPopupItemIcon(hMnuFile, MNU_FILE_OPEN, Om_getResIconPremult(IDI_BT_OPN));
  this->setPopupItemIcon(hMnuFile, MNU_FILE_CLOSE, Om_getResIconPremult(IDI_BT_CLO));
  this->setPopupItemIcon(hMnuFile, MNU_FILE_QUIT, Om_getResIconPremult(IDI_BT_EXI));

  HMENU hMnuEdit = this->getPopup(MNU_EDIT);
  this->setPopupItemIcon(hMnuEdit, MNU_EDIT_MANPROP, Om_getResIconPremult(IDI_BT_EDI));

  HMENU hMnuHub = this->getPopup(MNU_HUB);
  this->setPopupItemIcon(hMnuHub, MNU_HUB_ADDCHN, Om_getResIconPremult(IDI_BT_ADD_CHN));
  this->setPopupItemIcon(hMnuHub, MNU_HUB_ADDPST, Om_getResIconPremult(IDI_BT_ADD_PST));
  this->setPopupItemIcon(hMnuHub, MNU_HUB_PROP, Om_getResIconPremult(IDI_BT_EDI));

  HMENU hMnuChn = this->getPopup(MNU_CHN);
  this->setPopupItemIcon(hMnuChn, MNU_CHN_ADDREP, Om_getResIconPremult(IDI_BT_ADD_REP));
  this->setPopupItemIcon(hMnuChn, MNU_CHN_ADDMOD, Om_getResIconPremult(IDI_BT_ADD_PKG));
  this->setPopupItemIcon(hMnuChn, MNU_CHN_IMPMOD, Om_getResIconPremult(IDI_BT_IMP));
  this->setPopupItemIcon(hMnuChn, MNU_CHN_QRYREP, Om_getResIconPremult(IDI_BT_REF));
  this->setPopupItemIcon(hMnuChn, MNU_CHN_PROP, Om_getResIconPremult(IDI_BT_EDI));

  HMENU hMnuTls = this->getPopup(MNU_TOOL);
  this->setPopupItemIcon(hMnuTls, MNU_TOOL_EDITREP, Om_getResIconPremult(IDI_BT_TOOLREP));
  this->setPopupItemIcon(hMnuTls, MNU_TOOL_EDITPKG, Om_getResIconPremult(IDI_BT_TOOLPKG));

  HMENU hMnuHlp = this->getPopup(MNU_HELP);
  this->setPopupItemIcon(hMnuHlp, MNU_HELP_ABOUT, Om_getResIconPremult(IDI_BT_NFO));
  this->setPopupItemIcon(hMnuHlp, MNU_HELP_DEBUG, Om_getResIconPremult(IDI_BT_LOG));


  // Set Presets buttons icons
  this->setBmIcon(IDC_BC_PSRUN, Om_getResIcon(IDI_BT_RUN));
  this->setBmIcon(IDC_BC_PSNEW, Om_getResIcon(IDI_BT_ADD));
  this->setBmIcon(IDC_BC_PSEDI, Om_getResIcon(IDI_BT_EDI));
  this->setBmIcon(IDC_BC_PSDEL, Om_getResIcon(IDI_BT_REM));

  // Set Channels buttons icons
  this->setBmIcon(IDC_BC_CHADD, Om_getResIcon(IDI_BT_ADD));
  this->setBmIcon(IDC_BC_CHDEL, Om_getResIcon(IDI_BT_REM));
  this->setBmIcon(IDC_BC_CHEDI, Om_getResIcon(IDI_BT_EDI));

  // - - - - - - - - - - -

  // new array for the icons res ID
  uint32_t lvIdb[OM_LISTVIEW_ICON_COUNT];

  // initialize array with the 16px icons res ID
  Om_setupLvIconsDb(lvIdb);

  uint32_t shift = 0;
  switch(ModMan->iconsSize())
  {
  case 24: shift = 1; break; //< shift to 24px icons res ID
  case 32: shift = 2; break; //< shift to 32px icons res ID
  }

  // Create ImageList and fill it with bitmaps
  this->_listview_himl = ImageList_Create(ModMan->iconsSize(), ModMan->iconsSize(), ILC_COLOR32, 0, OM_LISTVIEW_ICON_COUNT);

  for(uint32_t i = 0; i < OM_LISTVIEW_ICON_COUNT; ++i)
    ImageList_Add(this->_listview_himl, Om_getResImage(lvIdb[i]+shift), nullptr);

  this->_listview_himl_size =  ModMan->iconsSize();

  // - - - - - - - - - - -

  // new array for the icons res ID
  uint32_t TbIdb[OM_TOOLBARS_ICON_COUNT];

  // initialize array with the 16px icons res ID
  Om_setupTbIconsDb(TbIdb);

  // Create ImageList and fill it with bitmaps
  this->_toolbars_himl = ImageList_Create(16, 16, ILC_COLOR32, 0, OM_TOOLBARS_ICON_COUNT);

  for(uint32_t i = 0; i < OM_TOOLBARS_ICON_COUNT; ++i)
    ImageList_Add(this->_toolbars_himl, Om_getResIconPremult(TbIdb[i]), nullptr);

  // - - - - - - - - - - -

  // load the context menu ressource
  this->_context_menu = LoadMenu(this->hins(), MAKEINTRESOURCE(IDR_CONTEXT_MENU));

  // set window icon
  this->setIcon(Om_getResIcon(IDI_APP, 2), Om_getResIcon(IDI_APP, 1));

  // Defines fonts for Mod Hub ComboBox
  HFONT hFt = Om_createFont(19, 200, L"Ms Shell Dlg");
  this->msgItem(IDC_CB_HUB, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  this->_createTooltip(IDC_CB_HUB,    L"Select active Mod Hub");
  this->_createTooltip(IDC_CB_CHN,    L"Select active Mod Channel");

  this->_createTooltip(IDC_BC_CHADD,  L"Add Channel");
  this->_createTooltip(IDC_BC_CHDEL,  L"Delete Channel");
  this->_createTooltip(IDC_BC_CHEDI,  L"Channel properties");

  this->_createTooltip(IDC_BC_PSRUN,  L"Run Preset");
  this->_createTooltip(IDC_BC_PSNEW,  L"New Preset");
  this->_createTooltip(IDC_BC_PSEDI,  L"Edit Preset");
  this->_createTooltip(IDC_BC_PSDEL,  L"Delete Preset");


  // retrieve saved window rect values
  RECT rec = {0,0,0,0};
  ModMan->loadWindowRect(rec);

  // window size
  int w = rec.right - rec.left;
  int h = rec.bottom - rec.top;
  if(w <= 0) w = 505; //< default width
  if(h <= 0) h = 340; //< default height

  // window placement flags
  uint32_t uFlags = SWP_NOZORDER;

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

  // get Library dialog, required for Presets
  this->_UiManMainLib = static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB));
  this->_UiManMainNet = static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET));

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


  // check for pending command line arguments
  if(!ModMan->pendingArg().empty()) {
    if(this->openArg(ModMan->pendingArg())) {
      ModMan->clearPendingArg();
    }
  }

  // Subclass Channels and Presets ListView to properly forward WM_MOUSEMOVE message
  SetWindowSubclass(this->getItem(IDC_LV_CHN), OmUiMan::_subMsg, 0, reinterpret_cast<DWORD_PTR>(this));
  SetWindowSubclass(this->getItem(IDC_LV_PST), OmUiMan::_subMsg, 0, reinterpret_cast<DWORD_PTR>(this));

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

  // Setup limits values
  long head_min = RSIZE_TOP_H + RSIZE_HEAD_MIN_H;
  long head_max = this->cliHeight() - (this->_ui_ovw_span + RSIZE_MAIN_MIN_H + (RSIZE_SEP_H * 2));
  long foot_min = RSIZE_BOT_H + RSIZE_FOOT_MIN_H;
  long foot_max = this->cliHeight() - (this->_lv_chn_span + RSIZE_MAIN_MIN_H + (RSIZE_SEP_H * 2));
  long side_min = RSIZE_SEP_W + RSIZE_SIDE_MIN_W;
  long side_max = this->cliWidth() - (this->_lv_pst_span + RSIZE_MAIN_MIN_W + (RSIZE_SEP_W * 2));

  long rc[4];

  if(this->_split_captured) {

    // Save the area to be redrawn after resize and update line position from split move param
    if(this->_split_hover_head) {
      rc[0] = 2; rc[2] = this->cliWidth() - 4;
      rc[1] = RSIZE_TOP_H;
      rc[3] = this->cliHeight() - this->_ui_ovw_span;

      // update height
      this->_lv_chn_span = this->_split_params[2];
    }

    if(this->_split_hover_side) {
      rc[1] = this->_lv_chn_show ? this->_lv_chn_span : RSIZE_TOP_H;
      rc[3] = this->_ui_ovw_show ? this->cliHeight() - this->_ui_ovw_span : this->cliHeight() - RSIZE_BOT_H;
      rc[0] = RSIZE_SEP_W;
      rc[2] = this->cliWidth() - RSIZE_SEP_W;

      // update height
      this->_lv_pst_span = this->_split_params[2];
    }

    if(this->_split_hover_foot) {
      rc[0] = 2; rc[2] = this->cliWidth() - 4;
      long max_h = this->_ui_ovw_span > this->_split_params[2] ? this->_ui_ovw_span : this->_split_params[2];
      rc[1] = this->cliHeight() - (max_h + 45);
      rc[3] = this->cliHeight() - RSIZE_BOT_H;

      // update height
      this->_ui_ovw_span = this->_split_params[2];
    }
  }

  // Clamp to limits
  if(this->_lv_chn_span < head_min) this->_lv_chn_span = head_min;
  if(this->_lv_chn_span > head_max) this->_lv_chn_span = head_max;
  if(this->_lv_pst_span < side_min) this->_lv_pst_span = side_min;
  if(this->_lv_pst_span > side_max) this->_lv_pst_span = side_max;
  if(this->_ui_ovw_span < foot_min) this->_ui_ovw_span = foot_min;
  if(this->_ui_ovw_span > foot_max) this->_ui_ovw_span = foot_max;

  // ---

  // Mod Hubs ComboBox
  this->_setItemPos(IDC_CB_HUB, 5, 2, this->cliWidth()-10 , 26, true);

  int32_t main_y = this->_lv_chn_show ? this->_lv_chn_span + RSIZE_SEP_H : RSIZE_TOP_H;
  int32_t foot_y = this->_ui_ovw_show ? this->cliHeight() - this->_ui_ovw_span : this->cliHeight() - RSIZE_BOT_H;
  int32_t pset_x = this->_lv_pst_show ? this->cliWidth() - this->_lv_pst_span : this->cliWidth();

  if(this->_lv_chn_show) {

    // Mod Channel ListView
    this->_setItemPos(IDC_LV_CHN, 30, RSIZE_TOP_H, this->cliWidth()-35, this->_lv_chn_span - RSIZE_TOP_H, true);
    this->_lv_chn_on_resize(); //< Resize the Mod Channel ListView column

    // Channel buttons
    this->_setItemPos(IDC_BC_CHEDI, RSIZE_SEP_W, RSIZE_TOP_H, 22, 22, true);
    this->_setItemPos(IDC_BC_CHADD, RSIZE_SEP_W, this->_lv_chn_span - 46, 22, 22, true);
    this->_setItemPos(IDC_BC_CHDEL, RSIZE_SEP_W, this->_lv_chn_span - 22, 22, 22, true);
  }

  if(this->_lv_pst_show) {

    // Presets buttons
    this->_setItemPos(IDC_BC_PSRUN, pset_x, main_y - 1, 22, 22, true);
    this->_setItemPos(IDC_BC_PSNEW, this->cliWidth() - 72, main_y - 1, 22, 22, true);
    this->_setItemPos(IDC_BC_PSDEL, this->cliWidth() - 49, main_y - 1, 22, 22, true);
    this->_setItemPos(IDC_BC_PSEDI, this->cliWidth() - 26, main_y - 1, 22, 22, true);

    // Presets ListView
    this->_setItemPos(IDC_LV_PST, pset_x, main_y + 24, this->_lv_pst_span - RSIZE_SEP_W, (foot_y - main_y) - 28, true);
    this->_lv_pst_on_resize(); //< Resize the Mod Channel ListView column
  }

  // Library Frame
  this->_setChildPos(this->_UiManMain->hwnd(), RSIZE_SEP_W, main_y, pset_x - (RSIZE_SEP_W * 2), (foot_y - main_y) - (RSIZE_SEP_H), true);

  if(this->_ui_ovw_show) {
    // Overview Frame
    this->_setChildPos(this->_UiManFoot->hwnd(), RSIZE_SEP_W, foot_y, this->cliWidth() - (RSIZE_SEP_W * 2), this->_ui_ovw_span - RSIZE_BOT_H, true);
  }

  // Foot status bar
  this->_setItemPos(IDC_SC_STATUS, -1, this->cliHeight()-24, this->cliWidth()+2, 26, true);
  this->_setItemPos(IDC_SC_FILE, 9, this->cliHeight()-19, this->cliWidth()-100, 16, true);
  this->_setItemPos(IDC_SC_INFO, this->cliWidth()-97, this->cliHeight()-19, 90, 16, true);

  if(this->_split_captured) {

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

    // update the footer frame and area around the splitter, without erasing window background to reduce flickering.
    RedrawWindow(this->_hwnd, reinterpret_cast<RECT*>(&rc), nullptr, RDW_INVALIDATE|RDW_UPDATENOW);

    if(this->_split_hover_head) {
      rc[1] = this->_lv_chn_span - 80; rc[3] = this->_lv_chn_span + 50;
      RedrawWindow(this->_hwnd, reinterpret_cast<RECT*>(&rc), nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
    }
    if(this->_split_hover_side) {
      rc[0] = this->cliWidth() - (this->_lv_pst_span + 10); rc[2] = this->cliWidth() - (this->_lv_pst_span - 80);
      RedrawWindow(this->_hwnd, reinterpret_cast<RECT*>(&rc), nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
    }
    if(this->_split_hover_foot) {
      rc[3] = this->cliHeight() - this->_ui_ovw_span;
      RedrawWindow(this->_hwnd, reinterpret_cast<RECT*>(&rc), nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
    }
  } else {
    // simple resize, we can redraw entire window
    RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW);
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

  // enable or disable menu elements
  this->_menu_enable();

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
      ImageList_Add(himl, Om_getResImage(idb[i]+shift), nullptr);

    this->_listview_himl_size = ModMan->iconsSize();
  }

  bool has_modhub = (ModMan->activeHub() != nullptr);

  this->enableItem(IDC_LV_CHN, has_modhub);
  this->enableItem(IDC_LV_PST, has_modhub);
  this->enableItem(IDC_BC_PSNEW, has_modhub);
  this->enableItem(IDC_BC_CHADD, has_modhub);

  bool has_modchn = (ModMan->activeChannel() != nullptr);

  this->enableItem(IDC_BC_CHDEL, has_modchn);
  this->enableItem(IDC_BC_CHEDI, has_modchn);

  // rebuild the Recent Mod Hub menu
  this->_menu_recent_populate();

  // rebuild the Mod Hub list ComboBox
  this->_cb_hub_populate();

  // update window caption and status bar
  this->_caption_populate();
  this->_status_populate();

  // update Mod Hub Icon
  //this->_sb_hub_populate();

  // update Mod Channel ListView
  this->_lv_chn_populate();

  // update Mod Presets ListView
  this->_lv_pst_populate();

  // Load UI layout parameters
  this->_layout_load();
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
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMan::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // release the previously captured mouse for frames move and resize process
  if(uMsg == WM_LBUTTONUP) {
    if(this->_split_captured) {
      // release captured mouse
      ReleaseCapture();
      // quit the frame move and resize process
      this->_split_captured = false;
      // Save current UI layout parameters
      this->_layout_save();
    }
    return 0;
  }

  // if mouse cursor is hovering between frames, checks for left button click
  // by user to capture mouse and entering the frames move and resize process
  if(uMsg == WM_LBUTTONDOWN || (uMsg == WM_PARENTNOTIFY && wParam == WM_LBUTTONDOWN)) {

    if(this->_split_hover_foot || this->_split_hover_head || this->_split_hover_side) {

      long mx = LOWORD(lParam); //< Cursor X position relative to client area
      long my = HIWORD(lParam); //< Cursor Y position relative to client area

      // keeps mouse pointer position and foot frame height at
      // capture to later calculate relative moves and size changes

      // Save initial line position
      if(this->_split_hover_head) {
        this->_split_params[0] = my;
        this->_split_params[1] = this->_lv_chn_span;
      }

      if(this->_split_hover_side) {
        this->_split_params[0] = mx;
        this->_split_params[1] = this->_lv_pst_span;
      }

      if(this->_split_hover_foot) {
        this->_split_params[0] = my;
        this->_split_params[1] = this->_ui_ovw_span;
      }

      // capture the mouse
      SetCapture(this->_hwnd);
      // we now are in frame move and resize process
      this->_split_captured = true;
    }
    return 0;
  }

  // changes the default cursor arrow to north-south resize arrows according
  // cursor hovering between the frames.
  if(uMsg == WM_SETCURSOR) {
    // checks whether cursor is hovering resize anchor
    if(this->_split_hover_foot || this->_split_hover_head) {
      SetCursor(LoadCursor(0,IDC_SIZENS));
      return 1; //< bypass default process
    }
    if(this->_split_hover_side) {
      SetCursor(LoadCursor(0,IDC_SIZEWE));
      return 1; //< bypass default process
    }
  }

  // track mouse cursor position either to detect whether cursor hover
  // between the frames (to change cursor) or, if we captured cursor, to
  // process the move and resize of the frames
  if(uMsg == WM_MOUSEMOVE) {

    long mx = LOWORD(lParam); //< Cursor X position relative to client area
    long my = HIWORD(lParam); //< Cursor Y position relative to client area

    if(GetCapture() == this->_hwnd) {

      #ifdef DEBUG
      std::cout << "DEBUG => OmUiMan::_onMsg : WM_MOUSEMOVE : GetCapture() == this->_hwnd\n";
      #endif

      if(this->_split_hover_head) {
        // calculate new line position according new cursor moves
        long s = this->_split_params[1] + (my - this->_split_params[0]);
        // move the splitter / resize frames
        if(s != this->_lv_chn_span) {
          this->_split_params[2] = s;
          this->_onResize();
        }
      }

      if(this->_split_hover_side) {
        // calculate new line position according new cursor moves
        long s = this->_split_params[1] + (this->_split_params[0] - mx);
        // move the splitter / resize frames
        if(s != this->_lv_pst_span) {
          this->_split_params[2] = s;
          this->_onResize();
        }
      }

      if(this->_split_hover_foot) {
        // calculate new line position according new cursor moves
        long s = this->_split_params[1] + (this->_split_params[0] - my);
        // move the splitter / resize frames
        if(s != this->_ui_ovw_span) {
          this->_split_params[2] = s;
          this->_onResize();
        }
      }

    } else {
      // checks whether mouse cursor is hovering between frames, we take a
      // good margin around the gap to make it easier to catch.

      // check for head split
      if(this->_lv_chn_show) {
        this->_split_hover_head = (my > (this->_lv_chn_span - 3) && my < (this->_lv_chn_span + 8));
      } else {
        this->_split_hover_head = false;
      }

      // check for side split
      if(this->_lv_pst_show) {
        long top = this->_lv_chn_show ? this->_lv_chn_span : RSIZE_TOP_H;
        long bot = this->_ui_ovw_show ? this->cliHeight() - this->_ui_ovw_span : RSIZE_BOT_H;
        long sx = this->cliWidth() - (this->_lv_pst_span);
        this->_split_hover_side = (my > top) && (my < bot) && (mx > (sx - 5) && mx < sx);
      } else {
        this->_split_hover_side = false;
      }

      // check for foot split
      if(this->_ui_ovw_show) {
        long s = this->cliHeight() - this->_ui_ovw_span;
        this->_split_hover_foot = (my > (s - 8) && my < (s + 3));
      } else {
        this->_split_hover_foot = false;
      }
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

        OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

        OmResult result = ModMan->openArg(static_cast<char*>(pCd->lpData));
        if(result == OM_RESULT_OK || result == OM_RESULT_PENDING) {

          // refresh UI
          this->refresh();

          if(result == OM_RESULT_PENDING) {
            if(this->openArg(ModMan->pendingArg())) {
              ModMan->clearPendingArg();
            }
          }

        } else {
          Om_dlgBox_okl(this->_hwnd, L"file open", IDI_DLG_ERR, L"Error opening file", L"unable to open file:", ModMan->lastError());
        }
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
        this->_lv_chn_on_rclick();
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

    // Prevent command/shorcut execution when dialog is not active
    if(!this->active()) {

      OmDialog* UiTool;

      // Reroot to proper Tool dialog if any
      UiTool = this->childById(IDD_TOOL_PKG);
      if(UiTool->active()) UiTool->postMessage(uMsg, wParam, lParam);

      UiTool = this->childById(IDD_TOOL_REP);
      if(UiTool->active()) UiTool->postMessage(uMsg, wParam, lParam);

      return false;
    }

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMan::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

    // handle "File > Recent Files" path click
    if(LOWORD(wParam) >= IDM_FILE_RECENT_PATH) { // recent
      // subtract Command ID by the base resource ID to get real index
      this->openRecent(LOWORD(wParam) - IDM_FILE_RECENT_PATH);
    }

    // Menus and Shortcuts Messages
    switch(LOWORD(wParam))
    {

    case IDC_CB_HUB: //< Mod Hub ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->_cb_hub_on_selchange();
      break;

    case IDC_BC_CHEDI:  //< Channel 'Prop' button
      this->channelProperties();
      break;

    case IDC_BC_CHADD:  //< Channel '+' button
      this->createChannel();
      break;

    case IDC_BC_CHDEL:  //< Channel 'x' button
      this->deleteChannel();
      break;

    case IDC_BC_PSRUN:  //< Preset 'Run' button
      this->runPreset();
      break;

    case IDC_BC_PSNEW:  //< Preset '+' button
      this->createPreset();
      break;

    case IDC_BC_PSDEL:  //< Preset 'x' button
      this->deletePreset();
      break;

    case IDC_BC_PSEDI:  //< Preset 'Prop' button
      this->presetProperties();
      break;

    // Menu : File []
    case IDM_FILE_NEW:
      this->childById(IDD_WIZ_HUB)->open(); // New Mod Hub Wizard
      break;

    case IDM_FILE_OPEN:
      this->openFile();
      break;

    case IDM_FILE_CLOSE:
      this->closeHub();
      break;

    case IDM_FILE_CLEAR_HIST:
      this->clearRecents();
      break;

    case IDM_QUIT:
      this->quit();
      break;

    // Menu : Edit []
    case IDM_HUB_PROP:
      this->hubProperties();
      break;

    case IDM_CHN_ADD:
      this->createChannel();
      break;

    case IDM_CHN_DEL:
      this->deleteChannel();
      break;

    case IDM_CHN_PROP:
      this->channelProperties();
      break;

    // Menu : Edit > Preset > []
    case IDM_PST_RUN:
      this->runPreset();
      break;

    case IDM_PST_ADD:
      this->createPreset();
      break;

    case IDM_PST_DEL:
      this->deletePreset();
      break;

    case IDM_PST_PROP:
      this->presetProperties();
      break;


    case IDM_REP_QRY:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->queryRepositories();
      break;

    case IDM_REP_ADD:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->addRepository();
      break;

    case IDM_REP_DEL:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->deleteRepository();
      break;


    case IDM_MOD_ADD:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->addToLibrary();
      break;

    case IDM_MOD_IMP:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->importToLibrary();
      break;

    case IDM_MOD_INST:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->queueInstalls();
      break;

    case IDM_MOD_UINS:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->queueRestores();
      break;

    case IDM_MOD_CLNS:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->queueCleaning();
      break;

    case IDM_MOD_TRSH:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->deleteSources();
      break;

    case IDM_MOD_DISC:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->discardBackups();
      break;

    case IDM_MOD_OPEN:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->exploreSources();
      break;

    case IDM_MOD_EDIT:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->editSource();
      break;

    case IDM_MOD_INFO:
      static_cast<OmUiManMainLib*>(this->_UiManMain->childById(IDD_MGR_MAIN_LIB))->showProperties();
      break;


    // Menu : Edit > Network Mod > []
    case IDM_NET_DNWS:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->startDownloads(false);
      break;

    case IDM_NET_DNLD:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->startDownloads(true);
      break;

    case IDM_NET_STOP:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->stopDownloads();
      break;

    case IDM_NET_RVOK:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->revokeDownloads();
      break;

    case IDM_NET_FIXD:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->downloadDepends(false);
      break;

    case IDM_NET_INFO:
      static_cast<OmUiManMainNet*>(this->_UiManMain->childById(IDD_MGR_MAIN_NET))->showProperties();
      break;

    case IDM_MAN_PROP:
      this->childById(IDD_PROP_MGR)->open();
      break;

    // Menu : Tools > []
    case IDM_REPEDITOR:
      this->childById(IDD_TOOL_REP)->modeless();
      break;

    case IDM_PKGEDITOR:
      this->childById(IDD_TOOL_PKG)->modeless();
      break;

    // Menu : Help > []
    case IDM_DBGLOG:
      this->childById(IDD_HELP_LOG)->modeless();
      break;

    case IDM_ABOUT:
      this->childById(IDD_HELP_ABT)->open();
      break;

    // Common shortcut (accelerator) commands
    case IDM_COM_INFO: {

        OmDialog* UiManMain;

        UiManMain = this->_UiManMain->childById(IDD_MGR_MAIN_LIB);
        if(UiManMain->visible()) static_cast<OmUiManMainLib*>(UiManMain)->showProperties();

        UiManMain = this->_UiManMain->childById(IDD_MGR_MAIN_NET);
        if(UiManMain->visible()) static_cast<OmUiManMainNet*>(UiManMain)->showProperties();
      }
      break;

    // Debug Shortcut
    #ifdef DEBUG
    case IDA_DEBUG_LOCKMODE:
      std::wcout << L"DEBUG => OmUiMan::_onMsg : IDA_DEBUG_LOCKMODE\n";
      this->enableLockMode(!this->_lock_mode);
      break;

    case IDA_DEBUG_SAFEMODE:
      std::wcout << L"DEBUG => OmUiMan::_onMsg : IDA_DEBUG_SAFEMODE\n";
      this->enableSafeMode(!this->_safe_mode);
      break;
    #endif // DEBUG
    }
  }

  if(uMsg == WM_EXITSIZEMOVE) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMan::_onMsg : WM_EXITSIZEMOVE\n";
    #endif

  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
LRESULT WINAPI OmUiMan::_subMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
  // we forward WM_LBUTTONDOWN and WM_LBUTTONUP event to parent
  // window (UiMan) for proper resize controls
  if(uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP) {

    OmUiMan* UiMan = reinterpret_cast<OmUiMan*>(dwRefData);

    // send message to parent
    SendMessage(UiMan->hwnd(), uMsg, wParam, lParam);
  }

  // we forward WM_SETCURSOR, WM_LBUTTONDOWN and WM_LBUTTONUP event to parent
  // window (UiMan) for proper mouse cursor
  if(uMsg == WM_SETCURSOR) {

    OmUiMan* UiMan = reinterpret_cast<OmUiMan*>(dwRefData);

    // send message to parent
    SendMessage(UiMan->hwnd(), uMsg, wParam, lParam);
    return 1;
  }

  // we forward WM_MOUSEMOVE event to parent window (UiMan) to better catch the
  // mouse cursor when around the frame split.
  if(uMsg == WM_MOUSEMOVE) {

    OmUiMan* UiMan = reinterpret_cast<OmUiMan*>(dwRefData);

    // get current cursor position, relative to client
    long p[2] = {LOWORD(lParam), HIWORD(lParam)};

    // convert coordinate to relative to parent's client
    ClientToScreen(hWnd, reinterpret_cast<POINT*>(&p));
    ScreenToClient(UiMan->hwnd(), reinterpret_cast<POINT*>(&p));

    // send message to parent
    SendMessage(UiMan->hwnd(), WM_MOUSEMOVE, 0, MAKELPARAM(p[0], p[1]));
  }

  return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

