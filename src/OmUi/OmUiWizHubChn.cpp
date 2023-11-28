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
#include "OmBaseApp.h"

#include "OmModMan.h"

#include "OmUiWizHub.h"

#include "OmUtilDlg.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizHubChn.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizHubChn::OmUiWizHubChn(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizHubChn::~OmUiWizHubChn()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizHubChn::id() const
{
  return IDD_WIZ_HUB_CHN;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizHubChn::hasValidParams() const
{
  OmWString item_str, msg;

  this->getItemText(IDC_EC_INP01, item_str);
  if(!Om_dlgValidName(this->_hwnd, L"Mod Channel name", item_str))
    return false;

  this->getItemText(IDC_EC_INP02, item_str);
  if(!Om_dlgValidDir(this->_hwnd, L"Target path", item_str))
    return false;

  if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) { //< Custom Library CheckBox
    this->getItemText(IDC_EC_INP03, item_str);
    if(Om_dlgValidPath(this->_hwnd, L"Library folder path", item_str)) {
      if(!Om_dlgCreateFolder(this->_hwnd, L"Custom Library folder", item_str))
        return false;
    } else {
      return false;
    }
  }

  if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) { //< Custom Backup CheckBox
    this->getItemText(IDC_EC_INP04, item_str);
    if(Om_dlgValidPath(this->_hwnd, L"Backup folder path", item_str)) {
      if(!Om_dlgCreateFolder(this->_hwnd, L"Custom Backup folder", item_str))
        return false;
    } else {
      return false;
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onTitleChange()
{
  OmWString title;

  this->getItemText(IDC_EC_INP01, title);

  if(!Om_isValidName(title))
    title = L"<invalid path>";

  if(!this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
    this->setItemText(IDC_EC_INP03, title + OM_MODCHAN_MODLIB_DIR);
  }
  if(!this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
    this->setItemText(IDC_EC_INP04, title + OM_MODCHAN_BACKUP_DIR);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onBcBrwDst()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP02, start);

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Target path, where Mods/Packages are to be installed.", start))
    return;

  this->setItemText(IDC_EC_INP02, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onCkBoxLib()
{
  OmWString title;

  int bm_chk = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW03, bm_chk);
  this->enableItem(IDC_EC_INP03, bm_chk);

  if(!bm_chk) {
    this->getItemText(IDC_EC_INP01, title);
    if(!Om_isValidName(title)) {
      title = L"<invalid path>";
    }
  }

  this->setItemText(IDC_EC_INP03, title + OM_MODCHAN_MODLIB_DIR);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onBcBrwLib()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP03, start);

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Library folder, where Mods/Packages are stored.", start))
    return;

  this->setItemText(IDC_EC_INP03, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onCkBoxBck()
{
  OmWString title;

  int bm_chk = this->msgItem(IDC_BC_CKBX2, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW04, bm_chk);
  this->enableItem(IDC_EC_INP04, bm_chk);

  if(!bm_chk) {
    this->getItemText(IDC_EC_INP01, title);
    if(!Om_isValidName(title)) {
      title = L"<invalid path>";
    }
  }

  this->setItemText(IDC_EC_INP04, title + OM_MODCHAN_BACKUP_DIR);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onBcBrwBck()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP04, start);

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Backup folder, where backup data will be stored.", start))
    return;

  this->setItemText(IDC_EC_INP04, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Mod Channel name, to identify it and create folder");

  this->_createTooltip(IDC_EC_INP02,  L"Installation destination path, where Mods/Packages are to be installed");
  this->_createTooltip(IDC_BC_BRW02,  L"Browse to select destination folder");

  this->_createTooltip(IDC_BC_CKBX1,  L"Use a custom Library folder instead of default one");
  this->_createTooltip(IDC_EC_INP03,  L"Library folder path, where Mods/Packages are stored");
  this->_createTooltip(IDC_BC_BRW03,  L"Browse to select a custom Library folder");

  this->_createTooltip(IDC_BC_CKBX2,  L"Use a custom Backup folder instead of default one");
  this->_createTooltip(IDC_EC_INP04,  L"Backup folder path, where backup data will be stored");
  this->_createTooltip(IDC_BC_BRW04,  L"Browse to select a custom Backup folder");

  // set default start values
  this->setItemText(IDC_EC_INP01, L"New Mod Channel");
  this->setItemText(IDC_EC_INP02, L"");
  this->setItemText(IDC_EC_INP03, L"New Mod Channel\\Library");
  this->setItemText(IDC_EC_INP04, L"New Mod Channel\\Backup");

  // disable "next" button
  static_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onShow()
{
  OmWString item_str;

  // enable or disable "next" button according values
  bool allow = true;

  this->getItemText(IDC_EC_INP01, item_str);
  if(!item_str.empty()) {

    this->getItemText(IDC_EC_INP02, item_str);
    if(!item_str.empty()) {

      if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INP03, item_str);
        if(item_str.empty()) allow = false;
      }

      if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INP04, item_str);
        if(item_str.empty()) allow = false;
      }

    } else {
      allow = false;
    }

  } else {
    allow = false;
  }

  static_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(allow);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onResize()
{
  // Introduction text
  this->_setItemPos(IDC_SC_INTRO, 10, 5, 190, 25);
  // Mod Channel title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 10, 40, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP01, 10, 50, this->cliUnitX()-25, 13);
  // Mod Channel Install Label & EditControl & Browse button
  this->_setItemPos(IDC_SC_LBL02, 10, 80, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP02, 10, 90, this->cliUnitX()-45, 13);
  this->_setItemPos(IDC_BC_BRW02, this->cliUnitX()-31, 90, 16, 13);
  // Custom Library Label & EditControl & Browse buttonben ess
  this->_setItemPos(IDC_BC_CKBX1, 10, 120, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP03, 10, 130, this->cliUnitX()-45, 13);
  this->_setItemPos(IDC_BC_BRW03, this->cliUnitX()-31, 130, 16, 13);
  // Custom Library Label & EditControl & Browse button
  this->_setItemPos(IDC_BC_CKBX2, 10, 150, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP04, 10, 160, this->cliUnitX()-45, 13);
  this->_setItemPos(IDC_BC_BRW04, this->cliUnitX()-31, 160, 16, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubChn::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiWizHubChn::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    switch(LOWORD(wParam))
    {

    case IDC_EC_INP01: //< Title EditText
      if(HIWORD(wParam) == EN_CHANGE) {
        this->_onTitleChange();
        has_changed = true;
      }
      break;

    case IDC_BC_BRW02: //< Target "..." (browse) Button
      this->_onBcBrwDst();
      break;

    case IDC_BC_CKBX1: //< Custom Library CheckBox
      this->_onCkBoxLib();
      break;

    case IDC_BC_BRW03: //< Custom Library "..." (browse) Button
      this->_onBcBrwLib();
      break;

    case IDC_BC_CKBX2: //< Custom Backup CheckBox
      this->_onCkBoxBck();
      break;

    case IDC_BC_BRW04: //< Custom Backup "..." (browse) Button
      this->_onBcBrwBck();
      break;

    case IDC_EC_INP02: //< Mod Channel
    case IDC_EC_INP03: //< Library
    case IDC_EC_INP04: //< Backup
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
      break;
    }

    // enable or disable "next" button according values
    if(has_changed) {
      bool allow = true;

      OmWString item_str;

      this->getItemText(IDC_EC_INP01, item_str);
      if(!item_str.empty()) {

        this->getItemText(IDC_EC_INP02, item_str);
        if(!item_str.empty()) {

          if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
            this->getItemText(IDC_EC_INP03, item_str);
            if(item_str.empty()) allow = false;
          }

          if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
            this->getItemText(IDC_EC_INP04, item_str);
            if(item_str.empty()) allow = false;
          }

        } else {
          allow = false;
        }

      } else {
        allow = false;
      }

      static_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(allow);
    }
  }

  return false;
}
