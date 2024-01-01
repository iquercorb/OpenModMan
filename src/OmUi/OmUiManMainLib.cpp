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
//#include "OmUiAddChn.h"
//#include "OmUiAddPst.h"
#include "OmUiPropMod.h"
//#include "OmUiPropPst.h"
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
  _import_build(false),
  _import_abort(false),
  _import_hdp(nullptr),
  _import_hth(nullptr),
  _import_hwo(nullptr),
  _modops_abort(false),
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
void OmUiManMainLib::addToLibrary()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  if(this->_import_hth)
    return;

  OmWString start;

  // new dialog to open file (allow multiple selection)
  this->_import_array.clear();
  if(!Om_dlgOpenFileMultiple(this->_import_array, this->_hwnd, L"Select Mod-package to add", OM_PKG_FILES_FILTER, start))
    return;

  // this is simple Mod add to library
  this->_import_build = false;

  this->_import_hth = Om_threadCreate(OmUiManMainLib::_import_run_fn, this);
  this->_import_hwo = Om_threadWaitEnd(this->_import_hth, OmUiManMainLib::_import_end_fn, this);

  // TODO: Implementer l'import de Mod-directory avec conversion automatique en Package
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::importToLibrary()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  if(this->_import_hth)
    return;

  OmWString start;

  // new dialog to open file (allow multiple selection)
  this->_import_array.clear();
  if(!Om_dlgOpenDirMultiple(this->_import_array, this->_hwnd, L"Select Mod-directories to import", start))
    return;

  // this is true Mod build/import
  this->_import_build = true;

  this->_import_hth = Om_threadCreate(OmUiManMainLib::_import_run_fn, this);
  this->_import_hwo = Om_threadWaitEnd(this->_import_hth, OmUiManMainLib::_import_end_fn, this);

  // TODO: Implementer l'import de Mod-directory avec conversion automatique en Package
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
  if(!ModPack) return;

  OmUiPropMod* UiPropPkg = static_cast<OmUiPropMod*>(this->_UiMan->childById(IDD_PROP_MOD));

  UiPropPkg->setModPack(ModPack);

  UiPropPkg->open(true);
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

  if(!Om_dlgBox_yn(this->_hwnd, L"Delete Mods", IDI_DLG_PKG_DEL, L"Delete Mods",
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

  // initialize progress bar
  this->enableItem(IDC_PB_MOD, true);
  this->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, MAKELPARAM(0, selection.size()));
  this->msgItem(IDC_PB_MOD, PBM_SETPOS, 0);

  // unselect all items
  LVITEMW lvI = {};
  lvI.mask = LVIF_STATE; lvI.stateMask = LVIS_SELECTED;
  this->msgItem(IDC_LV_MOD, LVM_SETITEMSTATE, -1, reinterpret_cast<LPARAM>(&lvI));

  for(size_t i = 0; i < selection.size(); ++i) {

    OmModPack* ModPack = selection[i];

    // remove Mod references from existing Presets
    for(size_t i = 0; i < ModHub->presetCount(); ++i)
      ModHub->getPreset(i)->deleteSetupEntry(ModChan, ModPack->iden());

    int32_t result = Om_moveToTrash(ModPack->sourcePath());
    if(result != 0) {
      Om_dlgBox_okl(this->_hwnd, L"Delete Mods", IDI_DLG_ERR, L"Delete Mod error",
                    L"Moving Mod \""+ModPack->iden()+L"\" to recycle bin failed:",
                    Om_errShell(L"", ModPack->sourcePath(), result));
    }

    this->msgItem(IDC_PB_MOD, PBM_SETPOS, i + 1);
  }

  // reset progress bar
  this->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, 0);
  this->msgItem(IDC_PB_MOD, PBM_SETPOS, 0);
  this->enableItem(IDC_PB_MOD, false);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::discardBackups()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  if(!Om_dlgBox_yn(this->_hwnd, L"Discard backup data", IDI_DLG_PKG_WRN, L"Deleting backup data",
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

  // initialize progress bar
  this->enableItem(IDC_PB_MOD, true);
  this->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, MAKELPARAM(0, selection.size()));
  this->msgItem(IDC_PB_MOD, PBM_SETPOS, 0);

  // for status icon update
  LVITEMW lvI = {};
  lvI.mask = LVIF_IMAGE; lvI.iSubItem = 0;

  for(size_t i = 0; i < selection.size(); ++i) {

    OmModPack* ModPack = selection[i];

    OmResult result = ModPack->discardBackup();

    // update status icon
    lvI.iItem = static_cast<int>(i); lvI.iImage = this->_lv_mod_get_status_icon(ModPack);
    this->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    if(result == OM_RESULT_ERROR) {
      Om_dlgBox_okl(this->_hwnd, L"Discard backup data", IDI_DLG_ERR, L"Backup data discord error",
                    L"Deleting backup data of \""+ModPack->name()+L"\" has failed:", ModPack->lastError());
    }

    this->msgItem(IDC_PB_MOD, PBM_SETPOS, i + 1);
  }

  // reset progress bar
  this->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, 0);
  this->msgItem(IDC_PB_MOD, PBM_SETPOS, 0);
  this->enableItem(IDC_PB_MOD, false);
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

      UiToolPkg->parseSource(ModPack->sourcePath());

    } else {

      UiToolPkg->setInitParse(ModPack->sourcePath());

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

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkTargetWrite(L"Install Mods"))
    return;

  if(!this->_UiMan->checkLibraryRead(L"Install Mods"))
    return;

  if(!this->_UiMan->checkBackupWrite(L"Install Mods"))
    return;

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
OmResult OmUiManMainLib::execInstalls(const OmPModPackArray& selection, bool silent)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return OM_RESULT_ABORT;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkTargetWrite(L"Install Mods"))
    return OM_RESULT_ERROR;

  if(!this->_UiMan->checkLibraryRead(L"Install Mods"))
    return OM_RESULT_ERROR;

  if(!this->_UiMan->checkBackupWrite(L"Install Mods"))
    return OM_RESULT_ERROR;

  OmPModPackArray installs;
  OmWStringArray overlaps, depends, missings;

  // prepare Mod installation
  ModChan->prepareInstalls(selection, &installs, &overlaps, &depends, &missings);

  // warn user for extra and missing stuff
  if(!silent) {

    if(!this->_UiMan->warnMissings(ModChan->warnMissDeps(), L"Install Mods", missings))
      return OM_RESULT_ABORT;

    if(!this->_UiMan->warnExtraInstalls(ModChan->warnExtraInst(), L"Install Mods", depends))
      return OM_RESULT_ABORT;

    if(!this->_UiMan->warnOverlaps(ModChan->warnOverlaps(), L"Install Mods", overlaps))
      return OM_RESULT_ABORT;
  }

  return this->_modops_exec(installs);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::queueRestores(bool silent)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // prevent useless processing
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkTargetWrite(L"Uninstall Mods"))
    return;

  if(!this->_UiMan->checkBackupWrite(L"Uninstall Mods"))
    return;

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
OmResult OmUiManMainLib::execRestores(const OmPModPackArray& selection, bool silent)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return OM_RESULT_ABORT;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkTargetWrite(L"Uninstall Mods"))
    return OM_RESULT_ERROR;

  if(!this->_UiMan->checkBackupWrite(L"Uninstall Mods"))
    return OM_RESULT_ERROR;

  OmPModPackArray restores;
  OmWStringArray overlappers, dependents;

  // prepare Mods uninstall and backups restoration
  ModChan->prepareRestores(selection, &restores, &overlappers, &dependents);

  // warn user for extra and missing stuff
  if(!silent) {
    if(!this->_UiMan->warnExtraRestores(ModChan->warnExtraUnin(), L"Uninstall Mods", overlappers, dependents))
      return OM_RESULT_ABORT;
  }

  return this->_modops_exec(restores);
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

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkTargetWrite(L"Clean uninstall Mods"))
    return;

  if(!this->_UiMan->checkBackupWrite(L"Clean uninstall Mods"))
    return;

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
    if(!Om_dlgBox_cal(this->_hwnd, L"Clean uninstall Mods", IDI_DLG_PKG_DEL, L"Unused dependency Mods",
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
  //this->_modops_abort();
  this->_modops_abort = true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_modlib_notify_fn(void* ptr, OmNotify notify, uint64_t param)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::_modlib_notify_fn\n";
  #endif

  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  if(notify == OM_NOTIFY_ALTERED || notify == OM_NOTIFY_DELETED)
    self->_lv_mod_alterate(notify, param);

  if(notify == OM_NOTIFY_REBUILD || notify == OM_NOTIFY_CREATED)
    self->_lv_mod_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_refresh_processing()
{
  OmModMan* ModMan = reinterpret_cast<OmModMan*>(this->_data);

  bool is_safe = true;

  // search in all opened Hubs for all channel
  for(size_t h = 0; h < ModMan->hubCount(); ++h) {

    OmModHub* ModHub = ModMan->getHub(h);

    for(size_t c = 0; c < ModHub->channelCount(); ++c) {

      bool has_queue = false;

      OmModChan* ModChan = ModHub->getChannel(c);

      if(ModChan->modOpsQueueSize()) {
        has_queue = true; is_safe = false;
      }

      // if this is current active channel set controls
      if(ModChan == ModHub->activeChannel()) {

        // enable 'kill-switch" abort button
        this->enableItem(IDC_BC_ABORT, has_queue);

        // reset progress bar, then enable or disable
        this->enableItem(IDC_PB_MOD, has_queue);
        if(has_queue) {
          this->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

          uint32_t progress = ModChan->modOpsProgress();

          this->msgItem(IDC_PB_MOD, PBM_SETPOS, progress + 1); //< this prevent transition
          this->msgItem(IDC_PB_MOD, PBM_SETPOS, progress);

        } else {
          this->msgItem(IDC_PB_MOD, PBM_SETPOS, 0);
        }
      }
    }
  }

  // set dialog safe to quit
  this->_setSafe(is_safe);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_abort_processing()
{
  // this function is the "kill switch" for all pending queues and threads
  // related to this dialog, this will make the dialog safe to quit

  OmModMan* ModMan = reinterpret_cast<OmModMan*>(this->_data);

  for(size_t h = 0; h < ModMan->hubCount(); ++h) {

    OmModHub* ModHub = ModMan->getHub(h);

    for(size_t c = 0; c < ModHub->channelCount(); ++c) {

      OmModChan* ModChan = ModHub->getChannel(c);

      ModChan->abortModOps();
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiManMainLib::_import_run_fn(void* ptr)
{
  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();

  // reset abort
  self->_import_abort = 0;

  // Open progress dialog
  HWND hParent = self->root()->hwnd(); //< we must provide main dialog handle or things goes wrong

  OmResult result;

  if(self->_import_build) {
    // true importation with mod building from directories
    self->_import_hdp = Om_dlgProgress(hParent, L"Import to Library", IDI_DLG_PKG_BLD, L"Importing Mods to Library", &self->_import_abort, OM_DLGBOX_DUAL_BARS);
    result = ModChan->importToLibrary(self->_import_array, OmUiManMainLib::_import_progress_fn, OmUiManMainLib::_import_compress_fn, self);
  } else {
    // simple importation, copying files to library
    self->_import_hdp = Om_dlgProgress(hParent, L"Add to Library", IDI_DLG_PKG_ADD, L"Adding Mods to Library", &self->_import_abort);
    result = ModChan->addToLibrary(self->_import_array, OmUiManMainLib::_import_progress_fn, self);
  }

  // quit the progress dialog (dialogs must be opened and closed within the same thread)
  Om_dlgProgressClose(static_cast<HWND>(self->_import_hdp));
  self->_import_hdp = nullptr;

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiManMainLib::_import_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  if(self->_import_build) {

    // preparing text
    self->_import_str = L"Building ";
    self->_import_str += Om_getFilePart(reinterpret_cast<wchar_t*>(param));
    self->_import_str += L": ";

    // update progress bar only
    Om_dlgProgressUpdate(static_cast<HWND>(self->_import_hdp), tot, cur, nullptr, 1); //< secondary bar

  } else {

    // preparing text
    self->_import_str = L"Copying Mod package: ";
    self->_import_str += Om_getFilePart(reinterpret_cast<wchar_t*>(param));

    // update progress bar + text
    Om_dlgProgressUpdate(static_cast<HWND>(self->_import_hdp), tot, cur, self->_import_str.c_str());

  }

  return (self->_import_abort != 1);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiManMainLib::_import_compress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  // preparing text
  OmWString progress_str = self->_import_str;
  progress_str += Om_getFilePart(reinterpret_cast<wchar_t*>(param));

  // update progress bar + text
  Om_dlgProgressUpdate(static_cast<HWND>(self->_import_hdp), tot, cur, progress_str.c_str(), 0); //< main bar

  return (self->_import_abort != 1);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmUiManMainLib::_import_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  OmResult result = static_cast<OmResult>(Om_threadExitCode(self->_import_hth));
  Om_threadClear(self->_import_hth, self->_import_hwo);

  self->_import_hth = nullptr;
  self->_import_hwo = nullptr;

  self->_import_array.clear();

  if(result == OM_RESULT_ERROR) {
    Om_dlgBox_ok(self->_hwnd, L"Import Mods", IDI_DLG_WRN, L"Mod importation error",
                L"One or more error occurred during Mod importation, see log for details.");
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
  LVITEMW lvI = {};
  lvI.mask = LVIF_IMAGE; lvI.iSubItem = 0; lvI.iImage = ICON_STS_QUE;
  for(size_t i = 0; i < selection.size(); ++i) {
    lvI.iItem = ModChan->indexOfModpack(selection[i]);
    this->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  // reset abort state
  this->_modops_abort = false;

  ModChan->queueModOps(selection,
                       OmUiManMainLib::_modops_begin_fn,
                       OmUiManMainLib::_modops_progress_fn,
                       OmUiManMainLib::_modops_result_fn,
                       OmUiManMainLib::_modops_ended_fn,
                       this);

  // Disable 'Install' and 'Uninstall'
  if(this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT)) {
    this->enableItem(IDC_BC_INST, false);
    this->enableItem(IDC_BC_UNIN, false);
  }

  // enter processing state
  this->_refresh_processing();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmResult OmUiManMainLib::_modops_exec(const OmPModPackArray& selection)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return OM_RESULT_ABORT;

  // change status icon
  LVITEMW lvI = {};
  lvI.mask = LVIF_IMAGE; lvI.iSubItem = 0; lvI.iImage = ICON_STS_QUE;
  for(size_t i = 0; i < selection.size(); ++i) {
    lvI.iItem = ModChan->indexOfModpack(selection[i]);
    this->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  // disable 'Install' and 'Uninstall' buttons
  if(this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT)) {
    this->enableItem(IDC_BC_INST, false);
    this->enableItem(IDC_BC_UNIN, false);
  }

  // reset abort state
  this->_modops_abort = false;

  // enter processing state
  this->_refresh_processing();

  // this is the synchronous version of the process
  OmResult result = ModChan->execModOps(selection,
                                        OmUiManMainLib::_modops_begin_fn,
                                        OmUiManMainLib::_modops_progress_fn,
                                        OmUiManMainLib::_modops_result_fn,
                                        this);

  // leaving processing state
  this->_refresh_processing();

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_modops_begin_fn(void* ptr, uint64_t param)
{
  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  OmModPack* ModPack = reinterpret_cast<OmModPack*>(param);

  // get ListView item index search using lparam, that is, Mod Pack hash value.
  int32_t item_id = self->findLvParam(IDC_LV_MOD, ModPack->hash());
  if(item_id < 0) return;

  // update ListView item
  LVITEMW lvI = {}; lvI.iItem = item_id;
  // change status icon
  lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE; lvI.iImage = ICON_STS_WIP;
  self->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiManMainLib::_modops_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OM_UNUSED(tot); OM_UNUSED(cur);

  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  OmModPack* ModPack = reinterpret_cast<OmModPack*>(param);

  OmModChan* ModChan = ModPack->ModChan();

  // check whether dialog is showing the proper Channel
  if(ModChan != static_cast<OmModMan*>(self->_data)->activeChannel())
    return true; //< wrong Channel, do not abort but ignore

  // get ListView item index search using lparam, that is, Mod Pack hash value.
  int32_t item_id = self->findLvParam(IDC_LV_MOD, ModPack->hash());

  // Invalidate ListView subitem rect to call custom draw (progress bar)
  RECT rect;
  self->getLvSubRect(IDC_LV_MOD, item_id, 4 /* 'Progress' column */, &rect);
  self->redrawItem(IDC_LV_MOD, &rect, RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);

  // update the general progress bar
  self->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
  self->msgItem(IDC_PB_MOD, PBM_SETPOS, ModChan->modOpsProgress()+1);
  self->msgItem(IDC_PB_MOD, PBM_SETPOS, ModChan->modOpsProgress());

  return !self->_modops_abort;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_modops_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  OmModMan* ModMan = static_cast<OmModMan*>(self->_data);

  OmModPack* ModPack = reinterpret_cast<OmModPack*>(param);

  OmModChan* ModChan = ModPack->ModChan();

  // check whether dialog is showing the proper Channel
  if(ModChan == ModMan->activeChannel()) {

    // get ListView item index search using lparam, that is, Mod Pack hash value.
    int32_t item_id = self->findLvParam(IDC_LV_MOD, ModPack->hash());

    // Set 'Install' and 'Uninstall" button if item currently selected
    if(self->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT) == 1) {
      if(self->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED) == item_id) {
        self->enableItem(IDC_BC_UNIN, ModPack->hasBackup());
        self->enableItem(IDC_BC_INST, ModPack->hasSource());
      }
    }

    // update ListView item
    LVITEMW lvI = {}; lvI.iItem = item_id;
    // change status icon
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE; lvI.iImage = self->_lv_mod_get_status_icon(ModPack);
    self->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Invalidate ListView subitem rect to call custom draw (progress bar)
    RECT rect;
    self->getLvSubRect(IDC_LV_MOD, item_id, 4 /* 'Progress' column */, &rect);
    self->redrawItem(IDC_LV_MOD, &rect, RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);
  }

  // show appropriate error message if any
  if(OM_HAS_BIT(result, OM_RESULT_ERROR)) {

    if(ModChan != ModMan->activeChannel()) {
      // switch to proper Hub and Channel to show error
      OmModHub* ModHub = ModChan->ModHub();
      int32_t hub_id = ModMan->indexOfHub(ModHub);
      self->_UiMan->selectHub(hub_id);
      int32_t chn_id = ModHub->indexOfChannel(ModChan);
      self->_UiMan->selectChannel(chn_id);
    }

    if(result == OM_RESULT_ERROR_APPLY || result == OM_RESULT_ERROR_BACKP) {
      Om_dlgBox_okl(self->_hwnd, L"Mod installation", IDI_DLG_PKG_ERR,
                  L"Mod install error", L"The installation of \""
                  +ModPack->name()+L"\" failed:", ModPack->lastError());
    } else {
      Om_dlgBox_okl(self->_hwnd, L"Mod backup restore", IDI_DLG_PKG_ERR,
                  L"Mod backup data restore error", L"Restoring backup data of \""
                  +ModPack->name()+L"\" failed:", ModPack->lastError());
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_modops_ended_fn(void* ptr, OmNotify notify, uint64_t param)
{
  OM_UNUSED(notify); OM_UNUSED(param);

  OmUiManMainLib* self = static_cast<OmUiManMainLib*>(ptr);

  self->_refresh_processing();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_ec_lib_populate()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainLib::_ec_lib_populate\n";
  #endif

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  OmWString item_str;

  if(ModChan) {
    if(ModChan->accessesLibrary(OM_ACCESS_DIR_READ)) { //< check only for reading
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

  LVITEMW lvI = {};

  // add item to list view
  for(size_t i = 0; i < ModChan->modpackCount(); ++i) {

    OmModPack* ModPack = ModChan->getModpack(i);

    lvI.iItem = static_cast<int>(i);

    // the first column, Mod status, here we INSERT the new item
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE|LVIF_PARAM; //< icon and special data
    lvI.iImage = this->_lv_mod_get_status_icon(ModPack);
    // notice for later : to work properly the lParam must be defined on the first SubItem (iSubItem = 0)
    lvI.lParam = static_cast<LPARAM>(ModPack->hash());
    this->msgItem(IDC_LV_MOD, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

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
void OmUiManMainLib::_lv_mod_alterate(OmNotify action, uint64_t param)
{

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // corresponding ListView item should be retrieved by index, but to be sure
  // we search using lparam, that is, Mod Pack hash value.
  int32_t item_id = this->findLvParam(IDC_LV_MOD, param);

  if(action == OM_NOTIFY_DELETED) {

    if(item_id < 0)
      return;

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiManMainLib::_lv_mod_alterate : DELETE\n";
    #endif

    // delete item
    this->msgItem(IDC_LV_MOD, LVM_DELETEITEM, item_id, 0);

  } else {

    // update item data
    OmModPack* ModPack = ModChan->findModpack(param);
    if(!ModPack) return; //< what ?!

    LVITEMW lvI = {};

    if(action == OM_NOTIFY_CREATED) {

      #ifdef DEBUG
      std::cout << "DEBUG => OmUiManMainLib::_lv_mod_alterate : CREATE\n";
      #endif

      // the first column, Mod status, here we INSERT the new item
      lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE|LVIF_PARAM; //< icon and special data
      lvI.iImage = this->_lv_mod_get_status_icon(ModPack);
      // notice for later : to work properly the lParam must be defined on the first SubItem (iSubItem = 0)
      lvI.lParam = static_cast<LPARAM>(ModPack->hash());
      lvI.iItem = this->msgItem(IDC_LV_MOD, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    } else {

      if(item_id < 0)
        return;

      #ifdef DEBUG
      std::cout << "DEBUG => OmUiManMainLib::_lv_mod_alterate : ALTER\n";
      #endif

      lvI.iItem = item_id;
      // update status icon
      lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE;
      lvI.iImage = this->_lv_mod_get_status_icon(ModPack);
      this->msgItem(IDC_LV_MOD, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
    }

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
  }
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
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  // get count of ListView selected item
  uint32_t lv_nsl = this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT);

  // handle to sub-menu
  //HMENU hPopup = this->_UiMan->getPopupItem(MNU_EDIT, MNU_EDIT_MOD);

  if((lv_nsl < 1) || (ModChan == nullptr)) {

    // disable buttons
    this->enableItem(IDC_BC_INST, false);
    this->enableItem(IDC_BC_UNIN, false);

    // disable all menu items
    /*
    for(uint32_t i = 0; i < 12; ++i)
      this->setPopupItem(hPopup, i, MF_GRAYED);
    */
    // show nothing in footer frame
    this->_UiMan->pUiMgrFoot()->clearItem();

  } else {

    bool can_install = false;
    bool can_restore = false;
    bool can_cleanng = false;

    OmModPack* ModPack = nullptr;

    // scan selection to check what can be done
    int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
    while(lv_sel != -1) {

      ModPack = ModChan->getModpack(lv_sel);

      if(ModPack->hasBackup()) {
        can_restore = true;
        if(ModPack->dependCount())
          can_cleanng = true;
      } else {
        can_install = true;
      }

      // next selected item
      lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
    }

    // if single selection show mod pack overview
    if(lv_nsl == 1) {
      this->_UiMan->pUiMgrFoot()->selectItem(ModPack);
    } else {
      this->_UiMan->pUiMgrFoot()->clearItem();
    }

    // enable / disable buttons
    this->enableItem(IDC_BC_INST, can_install);
    this->enableItem(IDC_BC_UNIN, can_restore);

    // enable / disable menu items
    /*
    this->setPopupItem(hPopup, MNU_MOD_INST, can_install ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, MNU_MOD_UINS, can_restore ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, MNU_MOD_CLNS, can_cleanng ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, MNU_MOD_DISC, MF_ENABLED);
    this->setPopupItem(hPopup, MNU_MOD_OPEN, MF_ENABLED);
    this->setPopupItem(hPopup, MNU_MOD_TRSH, MF_ENABLED);
    this->setPopupItem(hPopup, MNU_MOD_EDIT, (lv_nsl == 1)?MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, MNU_MOD_INFO, (lv_nsl == 1)?MF_ENABLED:MF_GRAYED);
    */
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_lv_mod_on_rclick()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  // get sub-menu from hidden context menu
  HMENU hPopup = this->_UiMan->getContextPopup(POP_MOD);

  // get count of ListView selected item
  uint32_t lv_nsl = this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT);

  // enable or disable menu-items according current state and selection
  if((lv_nsl < 1) || (ModChan == nullptr)) {

    // disable all menu items
    for(uint32_t i = 0; i < 12; ++i)
      this->setPopupItem(hPopup, i, MF_GRAYED);

  } else {

    bool can_install = false;
    bool can_restore = false;
    bool can_cleanng = false;

    // scan selection to check what can be done
    int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
    while(lv_sel != -1) {

      OmModPack* ModPack = ModChan->getModpack(lv_sel);

      if(ModPack->hasBackup()) {
        can_restore = true;
        if(ModPack->dependCount())
          can_cleanng = true;
      } else {
        can_install = true;
      }

      // next selected item
      lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
    }

    this->setPopupItem(hPopup, POP_MOD_INST, can_install ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, POP_MOD_UINS, can_restore ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, POP_MOD_CLNS, can_cleanng ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, POP_MOD_DISC, can_restore ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, POP_MOD_OPEN, MF_ENABLED);
    this->setPopupItem(hPopup, POP_MOD_TRSH, MF_ENABLED);
    this->setPopupItem(hPopup, POP_MOD_EDIT, (lv_nsl == 1)?MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, POP_MOD_INFO, (lv_nsl == 1)?MF_ENABLED:MF_GRAYED);
  }

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  // display popup menu along mouse cursor
  TrackPopupMenu(hPopup, TPM_LEFTALIGN|TPM_RIGHTBUTTON,  pt.x, pt.y, 0, this->_hwnd, nullptr);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_lv_mod_cdraw_progress(HDC hDc, uint64_t item, int32_t subitem)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmModPack* ModPack = ModChan->getModpack(item);
  if(!ModPack) return;

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
  if(ModPack->hasBackup()) {
    if(ModPack->isOverlapped()) {
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

  // Set buttons icons
  this->setBmIcon(IDC_BC_ADD, Om_getResIcon(IDI_BT_ADD_PKG));
  this->setBmIcon(IDC_BC_IMPORT, Om_getResIcon(IDI_BT_IMP));

  // define controls tool-tips
  this->_createTooltip(IDC_BC_ADD,    L"Add Mod(s) to library");
  this->_createTooltip(IDC_BC_IMPORT, L"Import Mod(s) to library");

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

  // "subscribe" to active channel library directory changes notifications
  static_cast<OmModMan*>(this->_data)->notifyModLibraryStart(OmUiManMainLib::_modlib_notify_fn, this);

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
  this->_refresh_processing();
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
  //this->_UiMan->setPopupItem(MNU_EDIT, MNU_EDIT_MOD, MF_GRAYED);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainLib::_onResize()
{
  // Library path EditControl
  this->_setItemPos(IDC_EC_INP01, 2, 0, this->cliWidth()-53, 21, true);
  // Import & Build button
  this->_setItemPos(IDC_BC_ADD, this->cliWidth()-24, -1, 24, 23, true);
  this->_setItemPos(IDC_BC_IMPORT, this->cliWidth()-49, -1, 24, 23, true);
  // Mods Library ListView
  this->_setItemPos(IDC_LV_MOD, 2, 24, this->cliWidth()-3, this->cliHeight()-50, true);
  this->_lv_mod_on_resize(); //< Resize the Mods ListView column
  // Install and Uninstall buttons
  this->_setItemPos(IDC_BC_INST, 2, this->cliHeight()-23, 78, 23, true);
  this->_setItemPos(IDC_BC_UNIN, 81, this->cliHeight()-23, 78, 23, true);
  // Progress bar
  this->_setItemPos(IDC_PB_MOD, 161, this->cliHeight()-22, this->cliWidth()-241, 21, true);
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

  //OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  // disable abort button
  this->enableItem(IDC_BC_ABORT, false);

  // disable or enable elements depending context
  this->enableItem(IDC_SC_LBL01, (ModChan != nullptr));
  this->enableItem(IDC_LV_MOD, (ModChan != nullptr));
  this->enableItem(IDC_BC_ADD, (ModChan != nullptr));
  this->enableItem(IDC_BC_IMPORT, (ModChan != nullptr));

  // load or reload theme for ListView custom draw (progress bar)
  if(this->_lv_mod_cdraw_htheme)
    CloseThemeData(this->_lv_mod_cdraw_htheme);
  this->_lv_mod_cdraw_htheme = OpenThemeData(this->_hwnd, L"Progress");

  // library directory path edit control
  this->_ec_lib_populate();

  // local library ListView
  this->_lv_mod_populate();

  // Display error dialog AFTER ListView refreshed its content
  if(ModChan) {
    this->_UiMan->checkTargetWrite(L"Mods Library");
    this->_UiMan->checkBackupWrite(L"Mods Library");
    this->_UiMan->checkLibraryRead(L"Mods Library");
  }

  this->_refresh_processing();
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
  this->_abort_processing();
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
        break;
      }
    }
    return false;
  }

  if(uMsg == WM_COMMAND) {

    // Prevent command/shorcut execution when main dialog is not active
    if(!this->root()->active())
      return false;

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiManMainLib::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

    switch(LOWORD(wParam))
    {
    case IDC_BC_ADD:
      if(HIWORD(wParam) == BN_CLICKED)
        this->addToLibrary();
      break;

    case IDC_BC_IMPORT:
      if(HIWORD(wParam) == BN_CLICKED)
        this->importToLibrary();
      break;

    case IDC_BC_INST:
      if(HIWORD(wParam) == BN_CLICKED)
        this->queueInstalls();
      break;

    case IDC_BC_UNIN:
      if(HIWORD(wParam) == BN_CLICKED)
        this->queueRestores();
      break;

    case IDC_BC_ABORT:
      if(HIWORD(wParam) == BN_CLICKED)
        this->abortAll();
      break;

    case IDM_MOD_INST:
      this->queueInstalls();
      break;

    case IDM_MOD_UINS:
      this->queueRestores();
      break;

    case IDM_MOD_CLNS:
      this->queueCleaning();
      break;

    case IDM_MOD_TRSH:
      this->deleteSources();
      break;

    case IDM_MOD_DISC:
      this->discardBackups();
      break;

    case IDM_MOD_OPEN:
      this->exploreSources();
      break;

    case IDM_MOD_EDIT:
      this->editSource();
      break;

    case IDM_MOD_INFO:
      this->showProperties();
      break;
    }
  }

  return false;
}
