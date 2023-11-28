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
#include "OmUiProgress.h"

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
  this->_addPage(L"Data Backup", new OmUiPropChnBck(hins));
  this->_addPage(L"Mod Repositories", new OmUiPropChnNet(hins));

  // creates child sub-dialogs
  this->addChild(new OmUiAddRep(hins));     //< Dialog for new Repository
  this->addChild(new OmUiProgress(hins));   //< for Mod Channel backup transfer or deletion
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
  OmUiPropChnStg* pUiPropLocStg  = static_cast<OmUiPropChnStg*>(this->childById(IDD_PROP_CHN_STG));

  if(pUiPropLocStg->paramChanged(CHN_PROP_STG_TITLE)) { //< parameter for Mod Channel title
    pUiPropLocStg->getItemText(IDC_EC_INP01, item_str);
    if(this->_ModChan->title() != item_str) {
      changed = true;
    } else {
      pUiPropLocStg->paramReset(CHN_PROP_STG_TITLE);
    }
  }

  if(pUiPropLocStg->paramChanged(CHN_PROP_STG_TARGET)) { //< parameter for Target path
    pUiPropLocStg->getItemText(IDC_EC_INP02, item_str);
    if(!Om_namesMatches(this->_ModChan->targetPath(),item_str)) {
      changed = true;
    } else {
      pUiPropLocStg->paramReset(CHN_PROP_STG_TARGET);
    }
  }

  // Mods Library Tab
  OmUiPropChnLib* pUiPropLocLib  = static_cast<OmUiPropChnLib*>(this->childById(IDD_PROP_CHN_LIB));

  if(pUiPropLocLib->paramChanged(CHN_PROP_LIB_CUSTDIR)) { //< parameter for Library path
    different = false;
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
      pUiPropLocLib->getItemText(IDC_EC_INP01, item_str);
      if(!Om_namesMatches(this->_ModChan->libraryPath(), item_str) || !this->_ModChan->hasCustomLibraryDir())
        different = true;
    } else {
      if(this->_ModChan->hasCustomLibraryDir()) different = true;
    }
    if(different) {
      changed = true;
    } else {
      pUiPropLocLib->paramReset(CHN_PROP_LIB_CUSTDIR);
    }
  }

  if(pUiPropLocLib->paramChanged(CHN_PROP_LIB_DEVMODE)) {
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != this->_ModChan->libraryDevmod()) {
      changed = true;
    } else {
      pUiPropLocLib->paramReset(CHN_PROP_LIB_DEVMODE);
    }
  }

  if(pUiPropLocLib->paramChanged(CHN_PROP_LIB_WARNINGS)) {

    different = false;

    if(pUiPropLocLib->msgItem(IDC_BC_CKBX3, BM_GETCHECK) != this->_ModChan->warnOverlaps())
      different = true;
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX4, BM_GETCHECK) != this->_ModChan->warnExtraInst())
      different = true;
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX5, BM_GETCHECK) != this->_ModChan->warnMissDeps())
      different = true;
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX6, BM_GETCHECK) != this->_ModChan->warnExtraUnin())
      different = true;

    if(different) {
      changed = true;
    } else {
      pUiPropLocLib->paramReset(CHN_PROP_LIB_WARNINGS);
    }
  }

  if(pUiPropLocLib->paramChanged(CHN_PROP_LIB_SHOWHIDDEN)) {
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX7, BM_GETCHECK) != this->_ModChan->libraryShowhidden()) {
      changed = true;
    } else {
      pUiPropLocLib->paramReset(CHN_PROP_LIB_SHOWHIDDEN);
    }
  }

  // Data backup Tab
  OmUiPropChnBck* pUiPropLocBck  = static_cast<OmUiPropChnBck*>(this->childById(IDD_PROP_CHN_BCK));

  if(pUiPropLocBck->paramChanged(CHN_PROP_BCK_CUSTDIR)) { //< parameter for Backup path

    different = false;

    if(pUiPropLocBck->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
      pUiPropLocBck->getItemText(IDC_EC_INP01, item_str);
      if(!Om_namesMatches(this->_ModChan->backupPath(), item_str) || !this->_ModChan->hasCustomBackupDir())
        different = true;
    } else {
      if(this->_ModChan->hasCustomBackupDir()) different = true;
    }

    if(different) {
      changed = true;
    } else {
      pUiPropLocBck->paramReset(CHN_PROP_LIB_CUSTDIR);
    }
  }

  if(pUiPropLocBck->paramChanged(CHN_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level

    different = false;

    if(pUiPropLocBck->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {

      if(this->_ModChan->backupCompMethod() != pUiPropLocBck->msgItem(IDC_CB_ZMD, CB_GETCURSEL))
        different = true;

      if(this->_ModChan->backupCompLevel() != pUiPropLocBck->msgItem(IDC_CB_ZLV, CB_GETCURSEL))
        different = true;

    } else {
      if(this->_ModChan->backupCompMethod() != -1) different = true;
    }

    if(different) {
      changed = true;
    } else {
      pUiPropLocBck->paramReset(CHN_PROP_BCK_COMP_LEVEL);
    }

  }

  // Mods Repositories Tab
  OmUiPropChnNet* pUiPropLocNet  = static_cast<OmUiPropChnNet*>(this->childById(IDD_PROP_CHN_NET));

  if(pUiPropLocNet->paramChanged(CHN_PROP_NET_WARNINGS)) {

     different = false;

    if(pUiPropLocNet->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != this->_ModChan->warnExtraDnld())
      different = true;
    if(pUiPropLocNet->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != this->_ModChan->warnMissDnld())
      different = true;
    if(pUiPropLocNet->msgItem(IDC_BC_CKBX3, BM_GETCHECK) != this->_ModChan->warnUpgdBrkDeps())
      different = true;

    if(different) {
      changed = true;
    } else {
      pUiPropLocNet->paramReset(CHN_PROP_NET_WARNINGS);
    }
  }

  if(pUiPropLocNet->paramChanged(CHN_PROP_NET_ONUPGRADE)) {
    if(pUiPropLocNet->msgItem(IDC_BC_RAD02, BM_GETCHECK) != this->_ModChan->upgdRename()) {
      changed = true;
    } else {
      pUiPropLocNet->paramReset(CHN_PROP_NET_ONUPGRADE);
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

  OmWString inp_str;

  // General Settings Tab
  OmUiPropChnStg* pUiPropLocStg  = static_cast<OmUiPropChnStg*>(this->childById(IDD_PROP_CHN_STG));

  if(pUiPropLocStg->paramChanged(CHN_PROP_STG_TITLE)) { //< parameter for Channel name

    pUiPropLocStg->getItemText(IDC_EC_INP01, inp_str);

    if(!Om_dlgValidName(this->_hwnd, L"Channel name", inp_str))
      return false;
  }

  if(pUiPropLocStg->paramChanged(CHN_PROP_STG_TARGET)) { //< parameter for Target path

    pUiPropLocStg->getItemText(IDC_EC_INP02, inp_str);

    if(!Om_dlgValidDir(this->_hwnd, L"Target path", inp_str))
      return false;
  }

  // Mods Library Tab
  OmUiPropChnLib* pUiPropLocLib  = static_cast<OmUiPropChnLib*>(this->childById(IDD_PROP_CHN_LIB));

  if(pUiPropLocLib->paramChanged(CHN_PROP_LIB_CUSTDIR)) { //< parameter for Library path

    if(pUiPropLocLib->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) { //< Custom Library folder Check-Box checked

      pUiPropLocLib->getItemText(IDC_EC_INP01, inp_str);

      if(Om_dlgValidPath(this->_hwnd, L"Library folder path", inp_str)) {

        if(!Om_dlgCreateFolder(this->_hwnd, L"Custom Library folder", inp_str))
          return false;

      } else {

        return false;
      }
    }
  }

  // Data backup Tab
  OmUiPropChnBck* pUiPropLocBck  = static_cast<OmUiPropChnBck*>(this->childById(IDD_PROP_CHN_BCK));

  if(pUiPropLocBck->paramChanged(CHN_PROP_BCK_CUSTDIR)) { //< parameter for Backup path

    if(pUiPropLocBck->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) { //< Custom Backup folder Check-Box checked

      pUiPropLocBck->getItemText(IDC_EC_INP01, inp_str);

      if(Om_dlgValidPath(this->_hwnd, L"Backup folder path", inp_str)) {

        if(!Om_dlgCreateFolder(this->_hwnd, L"Custom Backup folder", inp_str))
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

  OmWString inp_str;

  // General Settings Tab
  OmUiPropChnStg* pUiPropLocStg  = static_cast<OmUiPropChnStg*>(this->childById(IDD_PROP_CHN_STG));

  if(pUiPropLocStg->paramChanged(CHN_PROP_STG_TITLE)) { //< parameter for Mod Channel title

    pUiPropLocStg->getItemText(IDC_EC_INP01, inp_str);

    // To prevent crash during operation we unselect location in the main dialog
    static_cast<OmUiMan*>(this->root())->safemode(true);

    //TODO: a tester pour de vrai et voir à améliorer ce système de "safemode"

    if(!this->_ModChan->rename(inp_str)) {
      Om_dlgBox_okl(this->_hwnd, L"Mod Channel properties", IDI_WRN,
                   L"Mod Channel files rename error", L"Mod Channel "
                   "title changed but folder and definition file rename "
                   "failed because of the following error:", this->_ModChan->lastError());
    }

    // Back to main dialog window to normal state
    static_cast<OmUiMan*>(this->root())->safemode(false);

    // Reset parameter as unmodified
    pUiPropLocStg->paramReset(CHN_PROP_STG_TITLE);
  }

  if(pUiPropLocStg->paramChanged(CHN_PROP_STG_TARGET)) { //< parameter for Mod Channel Install path

    pUiPropLocStg->getItemText(IDC_EC_INP02, inp_str);

    this->_ModChan->setDstDir(inp_str);

    // Reset parameter as unmodified
    pUiPropLocStg->paramReset(CHN_PROP_STG_TARGET);
  }

  // Mod Library Tab
  OmUiPropChnLib* pUiPropLocLib  = static_cast<OmUiPropChnLib*>(this->childById(IDD_PROP_CHN_LIB));

  if(pUiPropLocLib->paramChanged(CHN_PROP_LIB_CUSTDIR)) { //< parameter for Mod Channel Library path

    if(pUiPropLocLib->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {

      pUiPropLocLib->getItemText(IDC_EC_INP01, inp_str);

      this->_ModChan->setCustomLibraryDir(inp_str);

    } else {

      this->_ModChan->setDefaultLibraryDir();
    }

    // Reset parameter as unmodified
    pUiPropLocLib->paramReset(CHN_PROP_LIB_CUSTDIR);
  }

  if(pUiPropLocLib->paramChanged(CHN_PROP_LIB_DEVMODE)) {

    this->_ModChan->setLibraryDevmod(pUiPropLocLib->msgItem(IDC_BC_CKBX2, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocLib->paramReset(CHN_PROP_LIB_DEVMODE);
  }

  if(pUiPropLocLib->paramChanged(CHN_PROP_LIB_WARNINGS)) {

    this->_ModChan->setWarnOverlaps(pUiPropLocLib->msgItem(IDC_BC_CKBX3, BM_GETCHECK));
    this->_ModChan->setWarnExtraInst(pUiPropLocLib->msgItem(IDC_BC_CKBX4, BM_GETCHECK));
    this->_ModChan->setWarnMissDeps(pUiPropLocLib->msgItem(IDC_BC_CKBX5, BM_GETCHECK));
    this->_ModChan->setWarnExtraUnin(pUiPropLocLib->msgItem(IDC_BC_CKBX6, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocLib->paramReset(CHN_PROP_LIB_WARNINGS);
  }

  if(pUiPropLocLib->paramChanged(CHN_PROP_LIB_SHOWHIDDEN)) {

    this->_ModChan->setLibraryShowhidden(pUiPropLocLib->msgItem(IDC_BC_CKBX7, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocLib->paramReset(CHN_PROP_LIB_SHOWHIDDEN);
  }

  // Data Backup Tab
  OmUiPropChnBck* pUiPropLocBck  = static_cast<OmUiPropChnBck*>(this->childById(IDD_PROP_CHN_BCK));

  if(pUiPropLocBck->paramChanged(CHN_PROP_BCK_CUSTDIR)) { //< parameter for Mod Channel Library path

    bool has_error = false;

    if(pUiPropLocBck->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {

      pUiPropLocBck->getItemText(IDC_EC_INP01, inp_str);

      if(!this->_ModChan->setCustomBackupDir(inp_str))
        has_error = true;

    } else {

      if(!this->_ModChan->setDefaultBackupDir())
        has_error = true;
    }

    if(has_error) {
      // an error occurred during backup purge
      Om_dlgBox_ok(this->_hwnd, L"Mod Channel properties", IDI_WRN,
                   L"Backup Directory transfer error", L"Mod Channel Backup "
                   "Data transfer encountered errors, some Backup Data may "
                   "had not properly transfered to new location.");
    }

    // Reset parameter as unmodified
    pUiPropLocBck->paramReset(CHN_PROP_BCK_CUSTDIR);
  }

  if(pUiPropLocBck->paramChanged(CHN_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level

    if(pUiPropLocBck->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {

      int32_t comp_method, comp_level;

      switch(pUiPropLocBck->msgItem(IDC_CB_ZMD, CB_GETCURSEL)) {
      case 1:   comp_method = OM_METHOD_DEFLATE; break; //< MZ_COMPRESS_METHOD_DEFLATE
      case 2:   comp_method = OM_METHOD_LZMA; break;    //< MZ_COMPRESS_METHOD_LZMA
      case 3:   comp_method = OM_METHOD_LZMA2; break;   //< MZ_COMPRESS_METHOD_XZ
      case 4:   comp_method = OM_METHOD_ZSTD; break;    //< MZ_COMPRESS_METHOD_ZSTD
      default:  comp_method = OM_METHOD_STORE; break;   //< MZ_COMPRESS_METHOD_STORE
      }

      switch(pUiPropLocBck->msgItem(IDC_CB_ZLV, CB_GETCURSEL)) {
      case 1:   comp_level = OM_LEVEL_FAST; break; //< MZ_COMPRESS_LEVEL_FAST
      case 2:   comp_level = OM_LEVEL_SLOW; break; //< MZ_COMPRESS_LEVEL_NORMAL
      case 3:   comp_level = OM_LEVEL_BEST; break; //< MZ_COMPRESS_LEVEL_BEST
      default:  comp_level = OM_LEVEL_NONE; break;
      }

      this->_ModChan->setBackupComp(comp_method, comp_level);

    } else {

      // disable zipped backups
      this->_ModChan->setBackupComp(-1, 0);

    }

    // Reset parameter as unmodified
    pUiPropLocBck->paramReset(CHN_PROP_BCK_COMP_LEVEL);
  }

  // Mods Repositories Tab
  OmUiPropChnNet* pUiPropLocNet  = static_cast<OmUiPropChnNet*>(this->childById(IDD_PROP_CHN_NET));

  if(pUiPropLocNet->paramChanged(CHN_PROP_NET_WARNINGS)) {

    this->_ModChan->setWarnExtraDnld(pUiPropLocNet->msgItem(IDC_BC_CKBX1, BM_GETCHECK));
    this->_ModChan->setWarnMissDnld(pUiPropLocNet->msgItem(IDC_BC_CKBX2, BM_GETCHECK));
    this->_ModChan->setWarnUpgdBrkDeps(pUiPropLocNet->msgItem(IDC_BC_CKBX3, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocNet->paramReset(CHN_PROP_NET_WARNINGS);
  }

  if(pUiPropLocNet->paramChanged(CHN_PROP_NET_ONUPGRADE)) {

    this->_ModChan->setUpgdRename(pUiPropLocNet->msgItem(IDC_BC_RAD02, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocNet->paramReset(CHN_PROP_NET_ONUPGRADE);
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
  return false;
}
