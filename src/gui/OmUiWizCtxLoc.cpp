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
#include "gui/OmUiWizCtx.h"
#include "gui/OmUiWizCtxLoc.h"
#include "OmManager.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizCtxLoc::OmUiWizCtxLoc(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizCtxLoc::~OmUiWizCtxLoc()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizCtxLoc::id() const
{
  return IDD_WIZ_CTX_LOC;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizCtxLoc::hasValidParams() const
{
  wstring item_str;

  this->getItemText(IDC_EC_INP01, item_str);
  if(!Om_isValidName(item_str)) {
    wstring wrn = L"Title";
    wrn += OMM_STR_ERR_VALIDNAME;
    Om_dialogBoxWarn(this->_hwnd, L"Invalid Location title", wrn);
    return false;
  }

  this->getItemText(IDC_EC_INP02, item_str);
  if(!Om_isDir(item_str)) {
    wstring wrn = L"The folder \""+item_str+L"\"";
    wrn += OMM_STR_ERR_ISDIR;
    Om_dialogBoxWarn(this->_hwnd, L"Invalid install destination folder", wrn);
    return false;
  }

  if(this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP03, item_str);
    if(!Om_isDir(item_str)) {
      wstring wrn = L"The folder \""+item_str+L"\"";
      wrn += OMM_STR_ERR_ISDIR;
      Om_dialogBoxWarn(this->_hwnd, L"Invalid custom library folder", wrn);
      return false;
    }
  }

  if(this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP04, item_str);
    if(!Om_isDir(item_str)) {
      wstring wrn = L"The folder \""+item_str+L"\"";
      wrn += OMM_STR_ERR_ISDIR;
      Om_dialogBoxWarn(this->_hwnd, L"Invalid custom backup folder", wrn);
      return false;
    }
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onTitleChange()
{
  wstring title;

  this->getItemText(IDC_EC_INP01, title);

  if(Om_isValidName(title)) {
    if(!this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
      this->setItemText(IDC_EC_INP03, title + L"\\Library");
    }
    if(!this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
      this->setItemText(IDC_EC_INP04, title + L"\\Backup");
    }
  } else {
    if(!this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
      this->setItemText(IDC_EC_INP03, L"<invalid path>\\Library");
    }
    if(!this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
      this->setItemText(IDC_EC_INP04, L"<invalid path>\\Backup");
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onBcBrwDst()
{
  wstring start, result;

  this->getItemText(IDC_EC_INP02, start);

  if(!Om_dialogBrowseDir(result, this->_hwnd, L"Select installation destination folder", start))
    return;

  this->setItemText(IDC_EC_INP02, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onCkBoxLib()
{
  bool bm_chk = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW03, bm_chk);
  this->enableItem(IDC_EC_INP03, bm_chk);

  wstring path;

  if(!bm_chk) {

    // set automatic default library path
    this->getItemText(IDC_EC_INP01, path);

    if(Om_isValidName(path)) {
      path += L"\\Library";
    } else {
      path = L"<invalid path>\\Library";
    }

  }

  this->setItemText(IDC_EC_INP03, path);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onBcBrwLib()
{
  wstring start, result;

  this->getItemText(IDC_EC_INP03, start);

  if(!Om_dialogBrowseDir(result, this->_hwnd, L"Select packages library custom folder", start))
    return;

  this->setItemText(IDC_EC_INP03, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onCkBoxBck()
{
  bool bm_chk = this->msgItem(IDC_BC_CHK02, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW04, bm_chk);
  this->enableItem(IDC_EC_INP04, bm_chk);

  wstring path;

  if(!bm_chk) {

    // set automatic default backup path
    this->getItemText(IDC_EC_INP01, path);

    if(Om_isValidName(path)) {
      path += L"\\Backup";
    } else {
      path = L"<invalid path>\\Backup";
    }
  }

  this->setItemText(IDC_EC_INP04, path);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onBcBrwBck()
{
  wstring start, result;

  this->getItemText(IDC_EC_INP04, start);

  if(!Om_dialogBrowseDir(result, this->_hwnd, L"Select backup data custom folder", start))
    return;

  this->setItemText(IDC_EC_INP04, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Indicative name");

  this->_createTooltip(IDC_EC_INP02,  L"Package installation destination path");
  this->_createTooltip(IDC_BC_BRW02,  L"Select destination folder");

  this->_createTooltip(IDC_BC_CHK01,  L"Use custom Library folder");
  this->_createTooltip(IDC_EC_INP03,  L"Custom Library folder path");
  this->_createTooltip(IDC_BC_BRW03,  L"Select custom Library folder");

  this->_createTooltip(IDC_BC_CHK02,  L"Use custom Backup folder");
  this->_createTooltip(IDC_EC_INP04,  L"Custom Backup folder path");
  this->_createTooltip(IDC_BC_BRW04,  L"Select custom Backup folder");

  // set default start values
  this->setItemText(IDC_EC_INP01, L"New Location");
  this->setItemText(IDC_EC_INP02, L"");
  this->setItemText(IDC_EC_INP03, L"New Location\\Library");
  this->setItemText(IDC_EC_INP04, L"New Location\\Backup");

  // disable "next" button
  static_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onShow()
{
  wstring item_str;

  // enable or disable "next" button according values
  bool allow = true;

  this->getItemText(IDC_EC_INP01, item_str);
  if(!item_str.empty()) {

    this->getItemText(IDC_EC_INP02, item_str);
    if(!item_str.empty()) {

      if(this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INP03, item_str);
        if(item_str.empty()) allow = false;
      }

      if(this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
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
void OmUiWizCtxLoc::_onResize()
{
  // Introduction text
  this->_setItemPos(IDC_SC_INTRO, 10, 5, 190, 25);
  // Location title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 10, 40, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INP01, 10, 50, this->width()-25, 13);
  // Location Install Label & EditControl & Browse button
  this->_setItemPos(IDC_SC_LBL02, 10, 80, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INP02, 10, 90, this->width()-45, 13);
  this->_setItemPos(IDC_BC_BRW02, this->width()-31, 90, 16, 13);
  // Custom Library Label & EditControl & Browse buttonben ess
  this->_setItemPos(IDC_BC_CHK01, 10, 120, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INP03, 10, 130, this->width()-45, 13);
  this->_setItemPos(IDC_BC_BRW03, this->width()-31, 130, 16, 13);
  // Custom Library Label & EditControl & Browse button
  this->_setItemPos(IDC_BC_CHK02, 10, 150, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INP04, 10, 160, this->width()-45, 13);
  this->_setItemPos(IDC_BC_BRW04, this->width()-31, 160, 16, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizCtxLoc::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    case IDC_BC_BRW02: //< Destination "..." (browse) Button
      this->_onBcBrwDst();
      break;

    case IDC_BC_CHK01: //< Custom Library CheckBox
      this->_onCkBoxLib();
      break;

    case IDC_BC_BRW03: //< Custom Library "..." (browse) Button
      this->_onBcBrwLib();
      break;

    case IDC_BC_CHK02: //< Custom Backup CheckBox
      this->_onCkBoxBck();
      break;

    case IDC_BC_BRW04: //< Custom Backup "..." (browse) Button
      this->_onBcBrwBck();
      break;

    case IDC_EC_INP02: //< Location
    case IDC_EC_INP03: //< Library
    case IDC_EC_INP04: //< backup
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
      break;
    }

    // enable or disable "next" button according values
    if(has_changed) {
      bool allow = true;

      wstring item_str;

      this->getItemText(IDC_EC_INP01, item_str);
      if(!item_str.empty()) {

        this->getItemText(IDC_EC_INP02, item_str);
        if(!item_str.empty()) {

          if(this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
            this->getItemText(IDC_EC_INP03, item_str);
            if(item_str.empty()) allow = false;
          }

          if(this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
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
