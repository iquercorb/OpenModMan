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

#include "OmModMan.h"

#include "OmArchive.h"          //< Archive compression methods / level

#include "OmUiMan.h"
#include "OmUiAddRep.h"
#include "OmUiPropChnStg.h"
#include "OmUiPropChnLib.h"
#include "OmUiPropChnBck.h"
#include "OmUiPropChnNet.h"

#include "OmUtilFs.h"
#include "OmUtilDlg.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropChn.h"


/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the moveBackup_fth
/// thread finished his job.
///
#define UWM_MOVEBACKUP_DONE     (WM_APP+4)


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChn::OmUiPropChn(HINSTANCE hins) : OmDialogProp(hins),
  _ModChan(nullptr)
{
  // create tab dialogs
  this->_addPage(L"General Settings", new OmUiPropChnStg(hins));
  this->_addPage(L"Mod Library", new OmUiPropChnLib(hins));
  this->_addPage(L"Backup storage", new OmUiPropChnBck(hins));
  this->_addPage(L"Network Repositories", new OmUiPropChnNet(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChn::~OmUiPropChn()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropChn::id()  const
{
  return IDD_PROP_CHN;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropChn::checkChanges()
{
  if(!this->_ModChan)
    return false;

  bool different, changed = false;

  OmWString item_str;

  // General Settings Tab
  OmUiPropChnStg* UiPropChnStg  = static_cast<OmUiPropChnStg*>(this->childById(IDD_PROP_CHN_STG));

  if(UiPropChnStg->paramChanged(CHN_PROP_STG_TITLE)) { //< parameter for Mod Channel title

    OmWString ec_entry;

    UiPropChnStg->getItemText(IDC_EC_INP01, ec_entry);

    if(this->_ModChan->title() != ec_entry) {
      changed = true;
    } else {
      UiPropChnStg->paramReset(CHN_PROP_STG_TITLE);
    }
  }

  if(UiPropChnStg->paramChanged(CHN_PROP_STG_TARGET)) { //< parameter for Target path

    OmWString ec_entry;

    UiPropChnStg->getItemText(IDC_EC_INP02, ec_entry);

    if(!Om_namesMatches(this->_ModChan->targetPath(),ec_entry)) {
      changed = true;
    } else {
      UiPropChnStg->paramReset(CHN_PROP_STG_TARGET);
    }
  }

  // Mods Library Tab
  OmUiPropChnLib* UiPropChnLib  = static_cast<OmUiPropChnLib*>(this->childById(IDD_PROP_CHN_LIB));

  if(UiPropChnLib->paramChanged(CHN_PROP_LIB_CUSTDIR)) { //< parameter for Library path

    different = false;

    if(UiPropChnLib->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {

      OmWString ec_entry;

      UiPropChnLib->getItemText(IDC_EC_INP01, ec_entry);

      if(!Om_namesMatches(this->_ModChan->libraryPath(), ec_entry) || !this->_ModChan->hasCustLibraryPath())
        different = true;

    } else {
      if(this->_ModChan->hasCustLibraryPath()) different = true;
    }

    if(different) {
      changed = true;
    } else {
      UiPropChnLib->paramReset(CHN_PROP_LIB_CUSTDIR);
    }
  }

  if(UiPropChnLib->paramChanged(CHN_PROP_LIB_DEVMODE)) {
    if(UiPropChnLib->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != this->_ModChan->libraryDevmod()) {
      changed = true;
    } else {
      UiPropChnLib->paramReset(CHN_PROP_LIB_DEVMODE);
    }
  }

  if(UiPropChnLib->paramChanged(CHN_PROP_LIB_WARNINGS)) {

    different = false;

    if(UiPropChnLib->msgItem(IDC_BC_CKBX3, BM_GETCHECK) != this->_ModChan->warnOverlaps())
      different = true;
    if(UiPropChnLib->msgItem(IDC_BC_CKBX4, BM_GETCHECK) != this->_ModChan->warnExtraInst())
      different = true;
    if(UiPropChnLib->msgItem(IDC_BC_CKBX5, BM_GETCHECK) != this->_ModChan->warnMissDeps())
      different = true;
    if(UiPropChnLib->msgItem(IDC_BC_CKBX6, BM_GETCHECK) != this->_ModChan->warnExtraUnin())
      different = true;

    if(different) {
      changed = true;
    } else {
      UiPropChnLib->paramReset(CHN_PROP_LIB_WARNINGS);
    }
  }

  if(UiPropChnLib->paramChanged(CHN_PROP_LIB_SHOWHIDDEN)) {
    if(UiPropChnLib->msgItem(IDC_BC_CKBX7, BM_GETCHECK) != this->_ModChan->libraryShowhidden()) {
      changed = true;
    } else {
      UiPropChnLib->paramReset(CHN_PROP_LIB_SHOWHIDDEN);
    }
  }

  // Data backup Tab
  OmUiPropChnBck* UiPropChnBck  = static_cast<OmUiPropChnBck*>(this->childById(IDD_PROP_CHN_BCK));

  if(UiPropChnBck->paramChanged(CHN_PROP_BCK_CUSTDIR)) { //< parameter for Backup path

    different = false;

    if(UiPropChnBck->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {

      OmWString ec_entry;

      UiPropChnBck->getItemText(IDC_EC_INP01, ec_entry);

      if(!Om_namesMatches(this->_ModChan->backupPath(), ec_entry) || !this->_ModChan->hasCustBackupPath())
        different = true;

    } else {
      if(this->_ModChan->hasCustBackupPath()) different = true;
    }

    if(different) {
      changed = true;
    } else {
      UiPropChnBck->paramReset(CHN_PROP_LIB_CUSTDIR);
    }
  }

  if(UiPropChnBck->paramChanged(CHN_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level

    different = false;

    if(UiPropChnBck->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {

      int32_t cb_sel;

      cb_sel = UiPropChnBck->msgItem(IDC_CB_ZMD, CB_GETCURSEL);
      int32_t comp_method = UiPropChnBck->msgItem(IDC_CB_ZMD, CB_GETITEMDATA, cb_sel);

      cb_sel = UiPropChnBck->msgItem(IDC_CB_ZLV, CB_GETCURSEL);
      int32_t comp_level = UiPropChnBck->msgItem(IDC_CB_ZLV, CB_GETITEMDATA, cb_sel);

      if(this->_ModChan->backupCompMethod() != comp_method)
        different = true;

      if(this->_ModChan->backupCompLevel() != comp_level)
        different = true;

    } else {
      if(this->_ModChan->backupCompMethod() != -1)
        different = true;
    }

    if(different) {
      changed = true;
    } else {
      UiPropChnBck->paramReset(CHN_PROP_BCK_COMP_LEVEL);
    }

  }

  // Mods Repositories Tab
  OmUiPropChnNet* UiPropChnNet  = static_cast<OmUiPropChnNet*>(this->childById(IDD_PROP_CHN_NET));

  if(UiPropChnNet->paramChanged(CHN_PROP_NET_WARNINGS)) {

     different = false;

    if(UiPropChnNet->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != this->_ModChan->warnExtraDnld())
      different = true;
    if(UiPropChnNet->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != this->_ModChan->warnMissDnld())
      different = true;
    if(UiPropChnNet->msgItem(IDC_BC_CKBX3, BM_GETCHECK) != this->_ModChan->warnUpgdBrkDeps())
      different = true;

    if(different) {
      changed = true;
    } else {
      UiPropChnNet->paramReset(CHN_PROP_NET_WARNINGS);
    }
  }

  if(UiPropChnNet->paramChanged(CHN_PROP_NET_ONUPGRADE)) {
    if(UiPropChnNet->msgItem(IDC_BC_RAD02, BM_GETCHECK) != this->_ModChan->upgdRename()) {
      changed = true;
    } else {
      UiPropChnNet->paramReset(CHN_PROP_NET_ONUPGRADE);
    }
  }

  // enable Apply button
  this->enableItem(IDC_BC_APPLY, changed);

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropChn::validChanges()
{
  if(!this->_ModChan)
    return true;

  // General Settings Tab
  OmUiPropChnStg* UiPropChnStg  = static_cast<OmUiPropChnStg*>(this->childById(IDD_PROP_CHN_STG));

  if(UiPropChnStg->paramChanged(CHN_PROP_STG_TITLE)) { //< parameter for Channel name

    OmWString ec_entry;

    UiPropChnStg->getItemText(IDC_EC_INP01, ec_entry);

    if(!Om_dlgValidName(this->_hwnd, L"Channel name", ec_entry))
      return false;
  }

  if(UiPropChnStg->paramChanged(CHN_PROP_STG_TARGET)) { //< parameter for Target path

    OmWString ec_entry;

    UiPropChnStg->getItemText(IDC_EC_INP02, ec_entry);

    if(!Om_dlgValidDir(this->_hwnd, L"Target directory", ec_entry))
      return false;
  }

  // Mods Library Tab
  OmUiPropChnLib* UiPropChnLib  = static_cast<OmUiPropChnLib*>(this->childById(IDD_PROP_CHN_LIB));

  if(UiPropChnLib->paramChanged(CHN_PROP_LIB_CUSTDIR)) { //< parameter for Library path

    if(UiPropChnLib->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) { //< Custom Library folder Check-Box checked

      OmWString ec_entry;

      UiPropChnLib->getItemText(IDC_EC_INP01, ec_entry);

      if(Om_dlgValidPath(this->_hwnd, L"Library directory path", ec_entry)) {

        if(!Om_dlgCreateFolder(this->_hwnd, L"Custom library directory", ec_entry))
          return false;

      } else {

        return false;
      }
    }
  }

  // Data backup Tab
  OmUiPropChnBck* UiPropChnBck  = static_cast<OmUiPropChnBck*>(this->childById(IDD_PROP_CHN_BCK));

  if(UiPropChnBck->paramChanged(CHN_PROP_BCK_CUSTDIR)) { //< parameter for Backup path

    if(UiPropChnBck->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) { //< Custom Backup folder Check-Box checked

      OmWString ec_entry;

      UiPropChnBck->getItemText(IDC_EC_INP01, ec_entry);

      if(Om_dlgValidPath(this->_hwnd, L"Backup directory path", ec_entry)) {

        if(!Om_dlgCreateFolder(this->_hwnd, L"Custom backup directory", ec_entry))
          return false;

      } else {

        return false;
      }
    }
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropChn::applyChanges()
{
  if(!this->_ModChan)
    return true;

  // General Settings Tab
  OmUiPropChnStg* UiPropChnStg  = static_cast<OmUiPropChnStg*>(this->childById(IDD_PROP_CHN_STG));

  if(UiPropChnStg->paramChanged(CHN_PROP_STG_TITLE)) { //< parameter for Mod Channel title

    OmWString ec_entry;

    UiPropChnStg->getItemText(IDC_EC_INP01, ec_entry);

    // set UI safe mode, this unselect everything and disable UI controls
    static_cast<OmUiMan*>(this->root())->enableSafeMode(true);

    OmResult result = this->_ModChan->rename(ec_entry);

    // back UI to normal state
    static_cast<OmUiMan*>(this->root())->enableSafeMode(false);

    if(result != OM_RESULT_OK) {
      Om_dlgBox_okl(this->_hwnd, L"Mod Channel properties", IDI_DLG_WRN, L"Channel rename error",
                   L"Unable to rename Channel:", this->_ModChan->lastError());
      return false;
    }

    // Reset parameter as unmodified
    UiPropChnStg->paramReset(CHN_PROP_STG_TITLE);
  }

  if(UiPropChnStg->paramChanged(CHN_PROP_STG_TARGET)) { //< parameter for Mod Channel Install path

    OmWString ec_entry;

    UiPropChnStg->getItemText(IDC_EC_INP02, ec_entry);

    OmResult result = this->_ModChan->setTargetPath(ec_entry);

    if(result != OM_RESULT_OK) {
      Om_dlgBox_okl(this->_hwnd, L"Mod Channel properties", IDI_DLG_WRN, L"Target directory change error",
                   L"Unable to change target directory:", this->_ModChan->lastError());
      return false;
    }

    // Reset parameter as unmodified
    UiPropChnStg->paramReset(CHN_PROP_STG_TARGET);
  }

  // Mod Library Tab
  OmUiPropChnLib* UiPropChnLib  = static_cast<OmUiPropChnLib*>(this->childById(IDD_PROP_CHN_LIB));

  if(UiPropChnLib->paramChanged(CHN_PROP_LIB_CUSTDIR)) { //< parameter for Mod Channel Library path

    OmResult result;

    if(UiPropChnLib->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {

      OmWString ec_entry;

      UiPropChnLib->getItemText(IDC_EC_INP01, ec_entry);

      result = this->_ModChan->setCustLibraryPath(ec_entry);

    } else {

      result = this->_ModChan->setDefLibraryPath();
    }

    if(result != OM_RESULT_OK) {
      Om_dlgBox_okl(this->_hwnd, L"Mod Channel properties", IDI_DLG_WRN, L"Library directory change error",
                   L"Unable to change library directory:", this->_ModChan->lastError());
      return false;
    }

    // Reset parameter as unmodified
    UiPropChnLib->paramReset(CHN_PROP_LIB_CUSTDIR);
  }

  if(UiPropChnLib->paramChanged(CHN_PROP_LIB_DEVMODE)) {

    this->_ModChan->setLibraryDevmod(UiPropChnLib->msgItem(IDC_BC_CKBX2, BM_GETCHECK));

    // Reset parameter as unmodified
    UiPropChnLib->paramReset(CHN_PROP_LIB_DEVMODE);
  }

  if(UiPropChnLib->paramChanged(CHN_PROP_LIB_WARNINGS)) {

    this->_ModChan->setWarnOverlaps(UiPropChnLib->msgItem(IDC_BC_CKBX3, BM_GETCHECK));
    this->_ModChan->setWarnExtraInst(UiPropChnLib->msgItem(IDC_BC_CKBX4, BM_GETCHECK));
    this->_ModChan->setWarnMissDeps(UiPropChnLib->msgItem(IDC_BC_CKBX5, BM_GETCHECK));
    this->_ModChan->setWarnExtraUnin(UiPropChnLib->msgItem(IDC_BC_CKBX6, BM_GETCHECK));

    // Reset parameter as unmodified
    UiPropChnLib->paramReset(CHN_PROP_LIB_WARNINGS);
  }

  if(UiPropChnLib->paramChanged(CHN_PROP_LIB_SHOWHIDDEN)) {

    this->_ModChan->setLibraryShowhidden(UiPropChnLib->msgItem(IDC_BC_CKBX7, BM_GETCHECK));

    // Reset parameter as unmodified
    UiPropChnLib->paramReset(CHN_PROP_LIB_SHOWHIDDEN);
  }

  // Data Backup Tab
  OmUiPropChnBck* UiPropChnBck  = static_cast<OmUiPropChnBck*>(this->childById(IDD_PROP_CHN_BCK));

  if(UiPropChnBck->paramChanged(CHN_PROP_BCK_CUSTDIR)) { //< parameter for Mod Channel Library path

    OmResult result;

    if(UiPropChnBck->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {

      OmWString ec_entry;

      UiPropChnBck->getItemText(IDC_EC_INP01, ec_entry);

      result = this->_ModChan->setCustBackupPath(ec_entry);

    } else {

      result = this->_ModChan->setDefBackupPath();
    }

    if(result != OM_RESULT_OK) {
      if(result == OM_RESULT_ERROR) {
        Om_dlgBox_okl(this->_hwnd, L"Mod Channel properties", IDI_DLG_WRN, L"Backup directory change error",
                     L"Backup data transfer encountered error:", this->_ModChan->lastError());
      } else {
        Om_dlgBox_okl(this->_hwnd, L"Mod Channel properties", IDI_DLG_WRN, L"Backup directory change error",
                     L"Unable to change backup directory:", this->_ModChan->lastError());
      }
      return false;
    }

    // Reset parameter as unmodified
    UiPropChnBck->paramReset(CHN_PROP_BCK_CUSTDIR);
  }

  if(UiPropChnBck->paramChanged(CHN_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level

    if(UiPropChnBck->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {

      int32_t cb_sel;

      cb_sel = UiPropChnBck->msgItem(IDC_CB_ZMD, CB_GETCURSEL);
      int32_t comp_method = UiPropChnBck->msgItem(IDC_CB_ZMD, CB_GETITEMDATA, cb_sel);

      cb_sel = UiPropChnBck->msgItem(IDC_CB_ZLV, CB_GETCURSEL);
      int32_t comp_level = UiPropChnBck->msgItem(IDC_CB_ZLV, CB_GETITEMDATA, cb_sel);

      this->_ModChan->setBackupComp(comp_method, comp_level);

    } else {

      // disable zipped backups
      this->_ModChan->setBackupComp(-1, 0);

    }

    // Reset parameter as unmodified
    UiPropChnBck->paramReset(CHN_PROP_BCK_COMP_LEVEL);
  }

  // Mods Repositories Tab
  OmUiPropChnNet* UiPropChnNet  = static_cast<OmUiPropChnNet*>(this->childById(IDD_PROP_CHN_NET));

  if(UiPropChnNet->paramChanged(CHN_PROP_NET_WARNINGS)) {

    this->_ModChan->setWarnExtraDnld(UiPropChnNet->msgItem(IDC_BC_CKBX1, BM_GETCHECK));
    this->_ModChan->setWarnMissDnld(UiPropChnNet->msgItem(IDC_BC_CKBX2, BM_GETCHECK));
    this->_ModChan->setWarnUpgdBrkDeps(UiPropChnNet->msgItem(IDC_BC_CKBX3, BM_GETCHECK));

    // Reset parameter as unmodified
    UiPropChnNet->paramReset(CHN_PROP_NET_WARNINGS);
  }

  if(UiPropChnNet->paramChanged(CHN_PROP_NET_ONUPGRADE)) {

    this->_ModChan->setUpgdRename(UiPropChnNet->msgItem(IDC_BC_RAD02, BM_GETCHECK));

    // Reset parameter as unmodified
    UiPropChnNet->paramReset(CHN_PROP_NET_ONUPGRADE);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChn::_onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(uMsg); OM_UNUSED(wParam); OM_UNUSED(lParam);

  return false;
}
