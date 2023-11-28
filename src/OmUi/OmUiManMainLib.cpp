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
  #include <algorithm>    // std::find

#include "OmBaseUi.h"

#include "OmBaseWin.h"
  #include <UxTheme.h>
  #include <vssym32.h>      //< for custom draw

#include "OmModMan.h"

#include "OmUiMan.h"
#include "OmUiManMain.h"
#include "OmUiManFoot.h"
#include "OmUiAddChn.h"
#include "OmUiAddPst.h"
#include "OmUiPropMod.h"
#include "OmUiPropPst.h"
#include "OmUiToolPkg.h"

#include "OmUtilFs.h"
#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"
#include "OmUtilErr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiManMainLib.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManMainLib::OmUiManMainLib(HINSTANCE hins) : OmDialog(hins),
  _UiMan(nullptr),
  _modops_count(0),
  _lv_mod_icons_size(0)
{
  // set the accelerator table for the dialog
  this->setAccel(IDR_ACCEL);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManMainLib::~OmUiManMainLib()
{
  // Get the previous Image List to be destroyed (Small and Normal uses the same)
  HIMAGELIST hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_MOD, LVM_GETIMAGELIST, LVSIL_NORMAL));
  if(hImgLs) ImageList_Destroy(hImgLs);

  // Get the previous Image List to be destroyed (Small and Normal uses the same)
  hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_PST, LVM_GETIMAGELIST, LVSIL_NORMAL));
  if(hImgLs) ImageList_Destroy(hImgLs);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiManMainLib::id() const
{
  return IDD_MGR_MAIN_LIB;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // Packages ListView & buttons
  this->enableItem(IDC_LV_MOD, !enable);
  this->enableItem(IDC_BC_INST, !enable);
  this->enableItem(IDC_BC_UNIN, !enable);

  // Batches Label, ListBox & Buttons
  this->enableItem(IDC_SC_LBL01, !enable);
  this->enableItem(IDC_LB_PST, !enable);
  this->enableItem(IDC_BC_NEW, !enable);
  this->enableItem(IDC_BC_EDI, !enable);
  // Batch Launch & Delete Buttons
  if(enable) {
    this->enableItem(IDC_BC_DEL, false);
    this->enableItem(IDC_BC_RUN, false);
  } else {
    int lb_sel = this->msgItem(IDC_LB_PST, LB_GETCURSEL);
    this->enableItem(IDC_BC_DEL, (lb_sel >= 0));
    this->enableItem(IDC_BC_RUN, (lb_sel >= 0));
  }

  // Abort Button
  this->enableItem(IDC_BC_ABORT, enable);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  if(!enable) {
    if(this->visible())
      this->_onRefresh();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::refreshLibrary()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::refreshLibrary\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  if(ModMan)  //< this should be always the case
    if(ModMan->activeChannel()) //< this should also be always the case
      this->_lv_mod_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::showProperties()
{
  // ignore non-single selection
  if(this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT) != 1)
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmModPack* ModPack = ModChan->getModpack(this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED));

  if(ModPack) {

    OmUiPropMod* UiPropPkg = static_cast<OmUiPropMod*>(this->_UiMan->childById(IDD_PROP_PKG));

    UiPropPkg->setModPack(ModPack);

    UiPropPkg->open(true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::deleteSources()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  if(!Om_dlgBox_yn(this->_hwnd, L"Delete Mods", IDI_PKG_DEL, L"Delete Mods",
                    L"Move the selected Mods to recycle bin ?")) return;

  OmPModPackArray selection;

  int lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    OmModPack* ModPack = ModChan->getModpack(lv_sel);

    if(ModPack->hasSource())
      selection.push_back(ModPack);

    // next selected item
    lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  // unselect all items
  LVITEMW lvI = {LVIF_STATE, 0, 0}; lvI.stateMask = LVIS_SELECTED;
  this->msgItem(IDC_LV_MOD, LVM_SETITEMSTATE, -1, reinterpret_cast<LPARAM>(&lvI));

  // freeze dialog so user cannot interact
  this->_UiMan->freeze(true);

  for(size_t i = 0; i < selection.size(); ++i) {

    this->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, selection.size()));
    this->msgItem(IDC_PB_PKG, PBM_SETPOS, i + 1);

    OmModPack* ModPack = selection[i];

    // remove Mod references from existing Presets
    for(size_t i = 0; i < ModHub->presetCount(); ++i)
      ModHub->getPreset(i)->deleteSetupEntry(ModChan, ModPack->iden());

    int32_t result = Om_moveToTrash(ModPack->sourcePath());
    if(result != 0) {
      Om_dlgBox_okl(this->_hwnd, L"Delete Mods", IDI_ERR, L"Delete Mod error",
                    L"Moving Mod \""+ModPack->iden()+L"\" to recycle bin failed:",
                    Om_errShell(L"", ModPack->sourcePath(), result));
    }
  }

  // unfreeze dialog to allow user to interact
  this->_UiMan->freeze(false);

  // rebuild the ListView
  this->_lv_mod_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::discardBackups()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  if(!ModHub) return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  if(!Om_dlgBox_yn(this->_hwnd, L"Discard backup data", IDI_PKG_WRN, L"Deleting backup data",
                  L"Selected Mods backup data will be deleted so no longer can be restored, continue anyway ?"))
                  return;

  OmPModPackArray selection;

  int lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    OmModPack* ModPack = ModChan->getModpack(lv_sel);

    if(ModPack->hasBackup())
      selection.push_back(ModPack);

    // next selected item
    lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  // freeze dialog so user cannot interact
  this->_UiMan->freeze(true);

  // for status icon change
  LVITEMW lvI = {LVIF_IMAGE, 0, 0};

  for(size_t i = 0; i < selection.size(); ++i) {

    this->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, selection.size()));
    this->msgItem(IDC_PB_PKG, PBM_SETPOS, i + 1);

    OmModPack* ModPack = selection[i];

    OmResult result = ModPack->discardBackup();

    // change status icon
    lvI.iItem = static_cast<int>(i); lvI.iImage = this->_lv_mod_get_status_icon(ModPack);
    this->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    if(result == OM_RESULT_ERROR) {
      Om_dlgBox_okl(this->_hwnd, L"Discard backup data", IDI_ERR, L"Backup data discord error",
                    L"Deleting backup data of \""+ModPack->name()+L"\" has failed:", ModPack->lastError());
    }
  }

  // unfreeze dialog to allow user to interact
  this->_UiMan->freeze(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::exploreSources()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmPModPackArray selection;

  int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    OmModPack* ModPack = ModChan->getModpack(lv_sel);

    if(ModPack->hasSource())
      selection.push_back(ModPack);

    // next selected item
    lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  for(size_t i = 0; i < selection.size(); ++i) {

    // the default behavior is to explore (open explorer with deployed folders)
    // however, it may happen that zip file are handled by an application
    // (typically, WinRar or 7zip) and the "explore" command may fail, in this
    // case, we call the "open" command.

    if(ShellExecuteW(this->_hwnd, L"explore", selection[i]->sourcePath().c_str(), nullptr, nullptr, SW_NORMAL ) <= (HINSTANCE)32) {
      ShellExecuteW(this->_hwnd, L"open", selection[i]->sourcePath().c_str(), nullptr, nullptr, SW_NORMAL );
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::editSource()
{
  // ignore non-single selection
  if(this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT) != 1)
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmModPack* ModPack = ModChan->getModpack(this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED));

  if(ModPack) {

    OmUiToolPkg* UiToolPkg = static_cast<OmUiToolPkg*>(this->_UiMan->childById(IDD_TOOL_PKG));

    if(UiToolPkg->visible()) {

      UiToolPkg->selectSource(ModPack->sourcePath());

    } else {

      UiToolPkg->setSource(ModPack->sourcePath());

      UiToolPkg->modeless(true);
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::queueInstalls(bool silent)
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkTargetWrite(L"Install Mods"))
    return;

  if(!this->_UiMan->checkLibraryRead(L"Install Mods"))
    return;

  if(!this->_UiMan->checkBackupWrite(L"Install Mods"))
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmPModPackArray selection, installs;

  int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    OmModPack* ModPack = ModChan->getModpack(lv_sel);

    if(ModPack->hasSource() && !ModPack->hasBackup())
      selection.push_back(ModPack);

    // next selected item
    lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  OmWStringArray overlaps, depends, missings;

  // prepare Mod installation
  ModChan->prepareInstalls(selection, &installs, &overlaps, &depends, &missings);

  // warn user for extra and missing stuff
  if(!silent) {

    if(!this->_UiMan->warnMissings(ModChan->warnMissDeps(), L"Install Mods", missings))
      return;

    if(!this->_UiMan->warnExtraInstalls(ModChan->warnExtraInst(), L"Install Mods", depends))
      return;

    if(!this->_UiMan->warnOverlaps(ModChan->warnOverlaps(), L"Install Mods", overlaps))
      return;
  }

  this->_modops_add(installs);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::execInstalls(const OmPModPackArray& selection, bool silent)
{
  // checks for proper access on all required directories
  if(!this->_UiMan->checkTargetWrite(L"Install Mods"))
    return;

  if(!this->_UiMan->checkLibraryRead(L"Install Mods"))
    return;

  if(!this->_UiMan->checkBackupWrite(L"Install Mods"))
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmPModPackArray installs;
  OmWStringArray overlaps, depends, missings;

  // prepare Mod installation
  ModChan->prepareInstalls(selection, &installs, &overlaps, &depends, &missings);

  // warn user for extra and missing stuff
  if(!silent) {

    if(!this->_UiMan->warnMissings(ModChan->warnMissDeps(), L"Install Mods", missings))
      return;

    if(!this->_UiMan->warnExtraInstalls(ModChan->warnExtraInst(), L"Install Mods", depends))
      return;

    if(!this->_UiMan->warnOverlaps(ModChan->warnOverlaps(), L"Install Mods", overlaps))
      return;
  }

  this->_modops_exec(installs);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::queueRestores(bool silent)
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkTargetWrite(L"Uninstall Mods"))
    return;

  if(!this->_UiMan->checkBackupWrite(L"Uninstall Mods"))
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmPModPackArray selection, restores;

  int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    OmModPack* ModPack = ModChan->getModpack(lv_sel);

    if(ModPack->hasBackup())
      selection.push_back(ModPack);

    // next selected item
    lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  OmWStringArray overlappers, dependents;

  // prepare Mods uninstall and backups restoration
  ModChan->prepareRestores(selection, &restores, &overlappers, &dependents);

  // warn user for extra and missing stuff
  if(!silent) {
    if(!this->_UiMan->warnExtraRestores(ModChan->warnExtraUnin(), L"Uninstall Mods", overlappers, dependents))
      return;
  }

  this->_modops_add(restores);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::execRestores(const OmPModPackArray& selection, bool silent)
{
  // checks for proper access on all required directories
  if(!this->_UiMan->checkTargetWrite(L"Uninstall Mods"))
    return;

  if(!this->_UiMan->checkBackupWrite(L"Uninstall Mods"))
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmPModPackArray restores;
  OmWStringArray overlappers, dependents;

  // prepare Mods uninstall and backups restoration
  ModChan->prepareRestores(selection, &restores, &overlappers, &dependents);

  // warn user for extra and missing stuff
  if(!silent) {
    if(!this->_UiMan->warnExtraRestores(ModChan->warnExtraUnin(), L"Uninstall Mods", overlappers, dependents))
      return;
  }

  this->_modops_exec(restores);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::queueAuto(bool silent)
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT) != 1)
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // Get ListView single selection
  int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  if(ModChan->getModpack(lv_sel)->hasBackup()) {
    this->queueRestores(silent);
  } else {
    this->queueInstalls(silent);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::queueCleaning(bool silent)
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkTargetWrite(L"Clean uninstall Mods"))
    return;

  if(!this->_UiMan->checkBackupWrite(L"Clean uninstall Mods"))
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmPModPackArray selection, restores;

  int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    OmModPack* ModPack = ModChan->getModpack(lv_sel);

    if(ModPack->hasBackup())
      selection.push_back(ModPack);

    // next selected item
    lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  OmWStringArray depends, overlappers, dependents;

  // prepare Mods uninstall and backups restoration
  ModChan->prepareCleaning(selection, &restores, &depends, &overlappers, &dependents);

  // check and warn for extra uninstall due to dependencies
  if(!silent && depends.size() && ModChan->warnExtraUnin()) {
    if(!Om_dlgBox_cal(this->_hwnd, L"Clean uninstall Mods", IDI_PKG_DEL, L"Unused dependency Mods",
                      L"The following dependency Mods will no longer be used by any another so will be also uninstalled:",
                      Om_concatStrings(depends, L"\r\n")))
      return;
  }

  // warn user for extra and missing stuff
  if(!silent)
    if(!this->_UiMan->warnExtraRestores(ModChan->warnExtraUnin(), L"Clean uninstall Mods", overlappers, dependents))
      return;

  this->_modops_add(restores);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::abortAll()
{
  // disable Abort button to acknowledge input
  this->enableItem(IDC_BC_ABORT, false);

  // abort all running jobs
  this->_modops_abort();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_update_safemode_status()
{
  bool threading = this->_modops_count;

  this->_setSafe(!threading);

  this->_UiMan->freeze(threading);

  this->enableItem(IDC_BC_ABORT, threading);

  this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0, 0);
  this->enableItem(IDC_PB_PKG, threading);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_modops_abort()
{
  if(this->_modops_count) {

    OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
    if(!ModChan) return;

    ModChan->abortInstalls();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_modops_add(const OmPModPackArray& selection)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // change status icon
  LVITEMW lvI = {LVIF_IMAGE, 0, 0}; lvI.iImage = ICON_STS_QUE;
  for(size_t i = 0; i < selection.size(); ++i) {
    lvI.iItem = ModChan->indexOfModpack(selection[i]);
    this->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  // if no queued operation, this is global start
  if(!this->_modops_count)
    this->_update_safemode_status();

  // increase count of queued mod operations
  this->_modops_count += selection.size();

  ModChan->queueInstalls(selection, OmUiManMainLib::_modops_begin_fn, OmUiManMainLib::_modops_progress_fn, OmUiManMainLib::_modops_result_fn, this);

  // Enable 'Abort' and disable 'Download'
  if(this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT)) {
    this->enableItem(IDC_BC_INST, false);
    this->enableItem(IDC_BC_UNIN, false);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_modops_exec(const OmPModPackArray& selection)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // change status icon
  LVITEMW lvI = {LVIF_IMAGE, 0, 0}; lvI.iImage = ICON_STS_QUE;
  for(size_t i = 0; i < selection.size(); ++i) {
    lvI.iItem = ModChan->indexOfModpack(selection[i]);
    this->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  // if no queued operation, this is global start
  if(!this->_modops_count)
    this->_update_safemode_status();

  // increase count of queued mod operations
  this->_modops_count += selection.size();

  // Enable 'Abort' and disable 'Download'
  if(this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT)) {
    this->enableItem(IDC_BC_INST, false);
    this->enableItem(IDC_BC_UNIN, false);
  }

  // this is the synchronous version of the process
  ModChan->execInstalls(selection, OmUiManMainLib::_modops_begin_fn, OmUiManMainLib::_modops_progress_fn, OmUiManMainLib::_modops_result_fn, this);

  // process endend
  this->_update_safemode_status();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_modops_begin_fn(void* ptr, uint64_t param)
{
  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  OmModPack* ModPack = reinterpret_cast<OmModPack*>(param);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();
  if(!ModChan) return;

  // keep list of overlapped Mods before restoration so we can refresh and
  // maybe remove their overlapped status after Mod restoration
  if(ModPack->hasBackup()) {
    for(size_t i = 0; i < ModPack->overlapCount(); ++i) {
      OmModPack* OvrPack = ModChan->findModpack(ModPack->getOverlapHash(i));
      if(OvrPack) self->_modops_ovr_queue.push_back(OvrPack);
    }
  }

  // change status icon
  LVITEMW lvI = {LVIF_IMAGE, ModChan->indexOfModpack(ModPack), 0}; lvI.iImage = ICON_STS_WIP;
  self->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiManMainLib::_modops_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  OmModPack* ModPack = reinterpret_cast<OmModPack*>(param);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();
  if(!ModChan) return true;

  // retrieve index of Mod Pack in Mod channel and therefore in ListView
  int32_t item_id = ModChan->indexOfModpack(ModPack);

  // Invalidate ListView subitem rect to call custom draw (progress bar)
  RECT rect;
  self->getLvSubRect(IDC_LV_MOD, item_id, 4 /* 'Progress' column */, &rect);
  self->redrawItem(IDC_LV_MOD, &rect, RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);

  // update the general progress bar
  self->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
  self->msgItem(IDC_PB_PKG, PBM_SETPOS, ModChan->installsProgress());

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_modops_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  OmModPack* ModPack = reinterpret_cast<OmModPack*>(param);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();
  if(!ModChan) return;

  // retrieve index of Mod Pack in Mod channel and therefore in ListView
  int32_t item_id = ModChan->indexOfModpack(ModPack);

  // Set 'Install' and 'Uninstall" button if item currently selected
  if(self->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT) == 1) {
    if(self->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED) == item_id) {
      self->enableItem(IDC_BC_UNIN, ModPack->hasBackup());
      self->enableItem(IDC_BC_INST, ModPack->hasSource());
    }
  }

  // change ListView status icon
  LVITEMW lvI = {LVIF_IMAGE, item_id, 0}; lvI.iImage = self->_lv_mod_get_status_icon(ModPack);
  self->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

  // update overlapping state of other Mods
  if(ModPack->hasBackup()) {
    lvI.iImage = ICON_STS_OVR;

    // following Mod install, update new overlapped (if any) mods of this one
    for(size_t i = 0; i < ModPack->overlapCount(); ++i) {

      lvI.iItem = ModChan->indexOfModpack(ModPack->getOverlapHash(i));
      if(lvI.iItem >= 0)
        self->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
    }

  } else {
    lvI.iImage = ICON_STS_CHK;

    // following Mod restore, update previously overlapped Mods that may no longer been
    while(self->_modops_ovr_queue.size()) {

      OmModPack* OvrPack = self->_modops_ovr_queue.front();

      if(!ModChan->isOverlapped(OvrPack)) {
        lvI.iItem = ModChan->indexOfModpack(OvrPack);
        self->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
      }

      self->_modops_ovr_queue.pop_front();
    }
  }

  // show appropriate error message if any
  if(result == OM_RESULT_ERROR) {
    if(ModPack->backupHasError() || ModPack->applyHasError()) {
      Om_dlgBox_okl(self->_hwnd, L"Mod installation", IDI_PKG_ERR,
                  L"Mod install error", L"The installation of \""
                  +ModPack->name()+L"\" failed:", ModPack->lastError());
    } else {
      Om_dlgBox_okl(self->_hwnd, L"Mod backup restore", IDI_PKG_ERR,
                  L"Mod backup data restore error", L"Restoring backup data of \""
                  +ModPack->name()+L"\" failed:", ModPack->lastError());
    }
  }

  // clear analytical status about install/restore
  ModPack->clearAnalytics();

  self->_modops_count--;

  // if no operation left in queue, this is global stop
  if(!self->_modops_count)
    self->_update_safemode_status();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_ec_lib_populate()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::_ec_lib_populate\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmModChan* ModChan = ModMan->activeChannel();

  OmWString item_str;

  if(ModChan) {
    if(ModChan->libraryDirAccess(false)) { //< check only for reading
      item_str = ModChan->libraryPath();
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
void OmUiManMainLib::_lv_mod_populate()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::_lv_mod_populate\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // if icon size changed we must 'reload' ImageList
  if(this->_lv_mod_icons_size != ModMan->iconsSize()) {
    LPARAM lparam = reinterpret_cast<LPARAM>(this->_UiMan->listViewImgList());
    this->msgItem(IDC_LV_MOD, LVM_SETIMAGELIST, LVSIL_SMALL, lparam);
    this->msgItem(IDC_LV_MOD, LVM_SETIMAGELIST, LVSIL_NORMAL, lparam);
    this->_lv_mod_icons_size = ModMan->iconsSize(); //< update size
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_MOD, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty list view
  this->msgItem(IDC_LV_MOD, LVM_DELETEALLITEMS);

  // get current context and location
  OmModChan* ModChan = ModMan->activeChannel();

  if(!ModChan) {
    // disable ListView
    this->enableItem(IDC_LV_MOD, false);
    // update Mods ListView selection
    this->_lv_mod_on_selchg();
    // return now
    return;
  }

  // add item to list view
  for(size_t i = 0; i < ModChan->modpackCount(); ++i) {

    OmModPack* ModPack = ModChan->getModpack(i);

    LVITEMW lvI = {0, static_cast<int>(i)};

    // the first column, Mod status, here we INSERT the new item
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE;
    if(ModPack->hasBackup()) {
      lvI.iImage = ModChan->isOverlapped(i) ? ICON_STS_OVR : ICON_STS_CHK;
    } else {
      lvI.iImage = ICON_NONE;
    }
    lvI.iItem = this->msgItem(IDC_LV_MOD, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Second column, the Mod name and type, here we set the sub-item
    lvI.iSubItem = 1; lvI.mask = LVIF_TEXT|LVIF_IMAGE;
    if(ModPack->hasSource()) {
      lvI.iImage = ModPack->sourceIsDir() ? ICON_MOD_DIR : (ModPack->dependCount() ? ICON_MOD_DEP : ICON_MOD_PKG);
    } else {
      lvI.iImage = ICON_MOD_ERR;
    }
    lvI.pszText = const_cast<LPWSTR>(ModPack->name().c_str());
    this->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Third column, the Mod version, we set the sub-item
    lvI.iSubItem = 2; lvI.mask = LVIF_TEXT;
    lvI.pszText = const_cast<LPWSTR>(ModPack->version().asString().c_str());
    this->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Fourth column, the Mod category, we set the sub-item
    lvI.iSubItem = 3; lvI.mask = LVIF_TEXT;
    lvI.pszText = const_cast<LPWSTR>(ModPack->category().c_str());
    this->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Fifth column, the operation progress
    // this sub-item is handled via custom draw routine
  }

  // we enable the ListView
  this->enableItem(IDC_LV_MOD, true);

  // restore ListView scroll position from lvRec
  this->msgItem(IDC_LV_MOD, LVM_SCROLL, 0, -lvRec.top );

  // adapt ListView column size to client area
  this->_lv_mod_on_resize();

  // update Mods ListView selection
  this->_lv_mod_on_selchg();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_lv_mod_on_resize()
{
  LONG size[4];

  // Resize the Mods ListView column
  GetClientRect(this->getItem(IDC_LV_MOD), reinterpret_cast<LPRECT>(&size));
  this->msgItem(IDC_LV_MOD, LVM_SETCOLUMNWIDTH, 1, size[2]-326);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_lv_mod_on_selchg()
{
  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT);

  // handle to "Edit > Mod Pack >" sub-menu
  HMENU hPopup = this->_UiMan->getPopupItem(MNU_EDIT, MNU_EDIT_MOD);

  // check count of selected item
  if(!lv_nsl) {

    // disable install, uninstall abort and progress bar
    this->enableItem(IDC_BC_INST, false);
    this->enableItem(IDC_BC_UNIN, false);
    this->enableItem(IDC_PB_PKG, false);

    // disable "Edit > Mod Pack" in main menu
    this->_UiMan->setPopupItem(MNU_EDIT, MNU_EDIT_MOD, MF_GRAYED);

    // disable all menu-item (for right click menu)
    for(unsigned i = 0; i < 10; ++i)
      this->_UiMan->setPopupItem(hPopup, i, MF_GRAYED);

    // show nothing in footer frame
    this->_UiMan->pUiMgrFoot()->clearItem();

    // return now
    return;
  }

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmModChan* ModChan = ModMan->activeChannel();
  if(!ModChan)
    return;

  // at least one selected, enable "Edit > Mod Pack []" pop-up menu
  this->_UiMan->setPopupItem(MNU_EDIT, MNU_EDIT_MOD, MF_ENABLED);

  // Check whether we have multiple selection
  if(lv_nsl > 1) {

    // Enable both buttons
    this->enableItem(IDC_BC_INST, true);
    this->enableItem(IDC_BC_UNIN, true);

    // enable menu-items from "install" to "Move to recycle bin"
    for(unsigned i = 0; i < 6; ++i)
      this->_UiMan->setPopupItem(hPopup, i, MF_ENABLED);

    // disable proper menu-items
    this->_UiMan->setPopupItem(hPopup, MNU_EDIT_MOD_EDIT, MF_GRAYED); //< "Load in Mod Pack Editor" menu-item
    this->_UiMan->setPopupItem(hPopup, MNU_EDIT_MOD_INFO, MF_GRAYED); //< "View detail..." menu-item

    // on multiple selection, we hide Mod description
    this->_UiMan->pUiMgrFoot()->clearItem();

  } else {

    // enable menu-items from "Uninstall all" to "View details..."
    for(unsigned i = 3; i < 10; ++i)
      this->_UiMan->setPopupItem(hPopup, i, MF_ENABLED);

    OmModPack* ModPack;

    // get the selected item id (only one, no need to iterate)
    int lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
    if(lv_sel >= 0) {

      ModPack = ModChan->getModpack(lv_sel);

      // show Mods info in footer frame
      this->_UiMan->pUiMgrFoot()->selectItem(ModPack);

      // enable proper menu-item and buttons
      if(ModPack->hasBackup()) {

        // Enable and disable proper buttons
        this->enableItem(IDC_BC_INST, false);
        this->enableItem(IDC_BC_UNIN, true);

        // enable and disable proper menu-items
        this->_UiMan->setPopupItem(hPopup, MNU_EDIT_MOD_INST, MF_GRAYED);  //< "Install"
        this->_UiMan->setPopupItem(hPopup, MNU_EDIT_MOD_UINS, MF_ENABLED); //< "Uninstall"
        this->_UiMan->setPopupItem(hPopup, MNU_EDIT_MOD_CLNS, MF_ENABLED); //< "Uninstall tree"
        this->_UiMan->setPopupItem(hPopup, MNU_EDIT_MOD_DISC, MF_ENABLED); //< "Discard Backup data"

      } else {

        // Enable and disable proper buttons
        this->enableItem(IDC_BC_INST, true);
        this->enableItem(IDC_BC_UNIN, false);

        // enable and disable proper menu-items
        this->_UiMan->setPopupItem(hPopup, MNU_EDIT_MOD_INST, MF_ENABLED); //< "Install"
        this->_UiMan->setPopupItem(hPopup, MNU_EDIT_MOD_UINS, MF_GRAYED);  //< "Uninstall"
        this->_UiMan->setPopupItem(hPopup, MNU_EDIT_MOD_CLNS, MF_GRAYED);  //< "Uninstall tree"
        this->_UiMan->setPopupItem(hPopup, MNU_EDIT_MOD_DISC, MF_GRAYED);  //< "Discard Backup data"
      }

    } else {
      // reset footer frame
      this->_UiMan->pUiMgrFoot()->clearItem();
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_lv_mod_on_rclick()
{
  // Refresh pop-up menu according selection
  this->_lv_mod_on_selchg();

  // get handle to "Edit > Mod Pack..." sub-menu
  HMENU hMenu = this->_UiMan->getPopupItem(MNU_EDIT, MNU_EDIT_MOD);

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  TrackPopupMenu(hMenu, TPM_TOPALIGN|TPM_LEFTALIGN, pt.x, pt.y, 0, this->_hwnd, nullptr);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_lv_mod_cdraw_progress(HDC hDc, uint64_t item, int32_t subitem)
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmModChan* ModChan = ModMan->activeChannel();
  if(!ModChan) return;

  OmModPack* ModPack = ModChan->getModpack(item);

  if(!ModPack->isBackuping() && !ModPack->isRestoring() && !ModPack->isApplying())
    return;

  // get rectangle of subitem to draw
  RECT rect, bar_rect, txt_rect;
  this->getLvSubRect(IDC_LV_MOD, item, subitem /* 'Progress' column */, &rect);

  double progress = (double)ModPack->operationProgress() * 0.01;
  wchar_t item_str[OM_MAX_ITEM];

  // laconic text...
  if(ModPack->isBackuping())
    swprintf(item_str, OM_MAX_ITEM, L"Backuping...");

  if(ModPack->isRestoring())
    swprintf(item_str, OM_MAX_ITEM, L"Restoring...");

  if(ModPack->isApplying())
    swprintf(item_str, OM_MAX_ITEM, L"Installing...");

  // all drawing rectangle base on subitem rectangle
  bar_rect = txt_rect = rect;

  // give some padding
  bar_rect.left += 1; bar_rect.right -= 2; bar_rect.top += 1; bar_rect.bottom -= 1;

  // resize horizontally according download percentage
  bar_rect.right = bar_rect.left + ((bar_rect.right - bar_rect.left) * progress);

  // draw rectangle using "Progress" theme
  DrawThemeBackground(this->_lv_mod_cdraw_htheme, hDc, PP_CHUNK, 0, &bar_rect, nullptr);

  // add padding
  txt_rect.left +=7;

  // draw text left-aligned
  DrawTextW(hDc, item_str, -1, &txt_rect, DT_LEFT|DT_VCENTER|DT_WORD_ELLIPSIS|DT_SINGLELINE);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmUiManMainLib::_lv_mod_get_status_icon(const OmModPack* ModPack)
{
  // select proper status icon
  if(ModPack->hasError()) {
    return ICON_STS_ERR;
  } else if(ModPack->hasBackup()) {

    OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
    if(!ModChan) return ICON_NONE;

    if(ModChan->isOverlapped(ModPack)) {
      return ICON_STS_OVR;
    } else {
      return ICON_STS_CHK;
    }
  }

  return ICON_NONE;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::_onInit\n";
  #endif

  // retrieve main dialog
  this->_UiMan = static_cast<OmUiMan*>(this->root());

  // define controls tool-tips
  this->_createTooltip(IDC_BC_INST,   L"Install selected Mod(s)");
  this->_createTooltip(IDC_BC_UNIN,   L"Uninstall selected Mod(s)");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort current process");

  // Shared Image list for ListView controls
  LPARAM himl = reinterpret_cast<LPARAM>(this->_UiMan->listViewImgList());

  // Initialize Mods ListView control
  DWORD lvStyle = LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_DOUBLEBUFFER;

  this->msgItem(IDC_LV_MOD, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lvStyle);
  // set explorer theme
  SetWindowTheme(this->getItem(IDC_LV_MOD),L"EXPLORER",nullptr);

  // we now add columns into Mods list-view control
  LVCOLUMNW lvCol = {};
  lvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
  //  "The alignment of the leftmost column is always LVCFMT_LEFT; it
  // cannot be changed." says Mr Microsoft. Do not ask why, the Microsoft's
  // mysterious ways... So, don't try to fix this.
  lvCol.pszText = const_cast<LPWSTR>(L"");
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.cx = 30;
  lvCol.iSubItem = 0;
  this->msgItem(IDC_LV_MOD, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Name");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 440;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_MOD, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Version");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 70;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_MOD, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Category");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_MOD, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Progress");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 120;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_MOD, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  // set image list for this ListView
  this->msgItem(IDC_LV_MOD, LVM_SETIMAGELIST, LVSIL_SMALL, himl);
  this->msgItem(IDC_LV_MOD, LVM_SETIMAGELIST, LVSIL_NORMAL, himl);

  this->_onRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::_onShow\n";
  #endif

  // refresh dialog
  //this->_onRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_onHide()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::_onHide\n";
  #endif

  // disable "Edit > Mod Pack" in main menu
  this->_UiMan->setPopupItem(MNU_EDIT, MNU_EDIT_MOD, MF_GRAYED);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_onResize()
{
  // Library path EditControl
  this->_setItemPos(IDC_EC_INP01, 2, 0, this->cliWidth()-3, 21, true);
  // Mods Library ListView
  this->_setItemPos(IDC_LV_MOD, 2, 24, this->cliWidth()-3, this->cliHeight()-50, true);
  this->_lv_mod_on_resize(); //< Resize the Mods ListView column
  // Install and Uninstall buttons
  this->_setItemPos(IDC_BC_INST, 2, this->cliHeight()-23, 78, 23, true);
  this->_setItemPos(IDC_BC_UNIN, 81, this->cliHeight()-23, 78, 23, true);
  // Progress bar
  this->_setItemPos(IDC_PB_PKG, 161, this->cliHeight()-22, this->cliWidth()-241, 21, true);
  // Abort button
  this->_setItemPos(IDC_BC_ABORT, this->cliWidth()-78, this->cliHeight()-23, 78, 23,  true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::_onRefresh\n";
  #endif

  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  // disable abort button
  this->enableItem(IDC_BC_ABORT, false);

  // disable or enable elements depending context
  this->enableItem(IDC_SC_LBL01, (ModHub != nullptr));
  this->enableItem(IDC_LV_MOD, (ModHub != nullptr));
/*
  if(ModChan)
    ModChan->refreshModLibrary();
*/
  // load or reload theme for ListView custom draw (progress bar)
  if(this->_lv_mod_cdraw_htheme)
    CloseThemeData(this->_lv_mod_cdraw_htheme);
  this->_lv_mod_cdraw_htheme = OpenThemeData(this->_hwnd, L"Progress");

  this->_ec_lib_populate();

  this->_lv_mod_populate();

  // Display error dialog AFTER ListView refreshed its content
  if(ModChan) {
    this->_UiMan->checkTargetWrite(L"Mods Library");
    this->_UiMan->checkBackupWrite(L"Mods Library");
    this->_UiMan->checkLibraryRead(L"Mods Library");
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::_onQuit\n";
  #endif

  // abort all running jobs
  this->_modops_abort();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiManMainLib::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_MAIN_HUB_CHANGED is a custom message sent from Main (parent) Dialog
  // to notify its child tab dialogs the Mod Hub selection changed.
  if(uMsg == UWM_MAIN_HUB_CHANGED) {
    // Refresh the dialog
    this->_onRefresh();
    return false;
  }

  if(uMsg == WM_NOTIFY) {

    if(reinterpret_cast<NMHDR*>(lParam)->code == NM_CUSTOMDRAW) {

      if(reinterpret_cast<NMHDR*>(lParam)->idFrom == IDC_LV_MOD) {

        NMLVCUSTOMDRAW* lvCustomDraw = reinterpret_cast<NMLVCUSTOMDRAW*>(lParam);

        switch(lvCustomDraw->nmcd.dwDrawStage)
        {
        case CDDS_PREPAINT :
          // Request to receive the CDDS_ITEMPREPAINT notification, we must use
          // SetWindowLongPtr() instead of return value because we are in DialogProc
          SetWindowLongPtr(this->_hwnd, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW); //< ie. return CDRF_NOTIFYITEMDRAW;
          return true;

        case CDDS_ITEMPREPAINT:
          // Request to receive the CDRF_NOTIFYSUBITEMDRAW notification, we must use
          // SetWindowLongPtr() instead of return value because we are in DialogProc
          SetWindowLongPtr(this->_hwnd, DWLP_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW); //< ie. return CDRF_NOTIFYSUBITEMDRAW;
          return true;

        case CDDS_SUBITEM | CDDS_ITEMPREPAINT:

          // We seek only for the 'Progress' (#4) column of the ListView
          if(lvCustomDraw->iSubItem == 4) {
            // send this to the custom draw function
            this->_lv_mod_cdraw_progress(lvCustomDraw->nmcd.hdc, lvCustomDraw->nmcd.dwItemSpec, lvCustomDraw->iSubItem);
            // Prevent system to redraw the default background by returning CDRF_SKIPDEFAULT
            // We must use SetWindowLongPtr() instead of value because we are in DialogProc
            SetWindowLongPtr(this->_hwnd, DWLP_MSGRESULT, CDRF_SKIPDEFAULT); //< ie. return CDRF_SKIPDEFAULT;
            return true;
          }
        }
      }
    }

    if(LOWORD(wParam) == IDC_LV_MOD) {

      OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
      if(!ModChan) return false;

      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case NM_DBLCLK:
        this->queueAuto();
        break;

      case NM_RCLICK:
        this->_lv_mod_on_rclick();
        break;

      case LVN_ITEMCHANGED: {
          NMLISTVIEW* nmLv = reinterpret_cast<NMLISTVIEW*>(lParam);
          // detect only selection changes
          if((nmLv->uNewState ^ nmLv->uOldState) & LVIS_SELECTED)
            this->_lv_mod_on_selchg();
          break;
        }

      case LVN_COLUMNCLICK:
        switch(reinterpret_cast<NMLISTVIEW*>(lParam)->iSubItem)
        {
        case 0:
          ModChan->setModLibrarySort(OM_SORT_STAT);
          break;
        case 2:
          ModChan->setModLibrarySort(OM_SORT_VERS);
          break;
        case 3:
          ModChan->setModLibrarySort(OM_SORT_CATE);
          break;
        default:
          ModChan->setModLibrarySort(OM_SORT_NAME);
          break;
        }
        this->_lv_mod_populate(); //< rebuild ListView
        break;
      }
    }
    return false;
  }

  if(uMsg == WM_COMMAND) {

    // Prevent command/shorcut execution when main dialog is not active
    if(!this->_UiMan->active())
      return false;

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiManMainLib::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

    switch(LOWORD(wParam))
    {
    case IDC_BC_INST:
      this->queueInstalls();
      break;

    case IDC_BC_UNIN:
      this->queueRestores();
      break;

    case IDC_BC_ABORT:
      this->abortAll();
      break;

    case IDM_EDIT_MOD_INST:
      this->queueInstalls();
      break;

    case IDM_EDIT_MOD_UINS:
      this->queueRestores();
      break;

    case IDM_EDIT_MOD_CLNS:
      this->queueCleaning();
      break;

    case IDM_EDIT_MOD_TRSH:
      this->deleteSources();
      break;

    case IDM_EDIT_MOD_DISC:
      this->discardBackups();
      break;

    case IDM_EDIT_MOD_OPEN:
      this->exploreSources();
      break;

    case IDM_EDIT_MOD_EDIT:
      this->editSource();
      break;

    case IDM_EDIT_MOD_INFO:
      this->showProperties();
      break;
    }
  }

  return false;
}
