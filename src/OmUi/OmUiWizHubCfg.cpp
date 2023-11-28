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

#include "OmUiWizHub.h"

#include "OmUtilDlg.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizHubCfg.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizHubCfg::OmUiWizHubCfg(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizHubCfg::~OmUiWizHubCfg()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizHubCfg::id() const
{
  return IDD_WIZ_HUB_CFG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizHubCfg::hasValidParams() const
{
  OmWString item_str;

  this->getItemText(IDC_EC_INP01, item_str);
  if(!Om_dlgValidName(this->_hwnd, L"Mod Hub name", item_str))
    return false;

  this->getItemText(IDC_EC_INP02, item_str);
  if(Om_dlgValidPath(this->_hwnd, L"Mod Hub path", item_str)) {
    if(!Om_dlgCreateFolder(this->_hwnd, L"Mod Hub path", item_str))
      return false;
  } else {
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubCfg::_onPathChange()
{
  OmWString name, path;

  this->getItemText(IDC_EC_INP01, name);
  this->getItemText(IDC_EC_INP02, path);

  if(!Om_isValidName(name) || !Om_isValidPath(path)) {
    this->setItemText(IDC_EC_INP03, L"<invalid path>");
    return;
  }

  path += L"\\" + name + L"\\";
  this->setItemText(IDC_EC_INP03, path);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubCfg::_onBcBrwHome()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP02, start);

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Mod Hub path, where to create the Mod Hub home folder", start))
    return;

  this->setItemText(IDC_EC_INP02, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubCfg::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Mod Hub name, both to identify it and create home folder");

  this->_createTooltip(IDC_EC_INP02,  L"Mod Hub path, where to create the Mod Hub home folder");
  this->_createTooltip(IDC_BC_BRW02,  L"Browse to select Hub path");

  // set default start values
  this->setItemText(IDC_EC_INP01, L"New Mod Hub");
  this->setItemText(IDC_EC_INP02, L"");
  this->setItemText(IDC_EC_INP03, L"<invalid path>");

  // disable "next" button
  static_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubCfg::_onShow()
{
  OmWString item_str;

  // enable or disable "next" button according values
  bool allow = true;

  this->getItemText(IDC_EC_INP01, item_str);
  if(!item_str.empty()) {

    this->getItemText(IDC_EC_INP02, item_str);
    if(!item_str.empty()) {

      this->getItemText(IDC_EC_INP03, item_str);
      if(!Om_isValidPath(item_str)) allow = false;

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
void OmUiWizHubCfg::_onResize()
{
  // Introduction text
  this->_setItemPos(IDC_SC_INTRO, 10, 5, 190, 25);
  // Mod Hub title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 10, 40, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP01, 10, 50, this->cliUnitX()-25, 13);
  // Mod Hub location Label & EditControl & Browse button
  this->_setItemPos(IDC_SC_LBL02, 10, 80, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP02, 10, 90, this->cliUnitX()-45, 13);
  this->_setItemPos(IDC_BC_BRW02, this->cliUnitX()-31, 90, 16, 13);
  // Result path Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 10, 150, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP03, 10, 160, this->cliUnitX()-25, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubCfg::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubCfg::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiWizHubCfg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01: // Title
    case IDC_EC_INP02: // Mod Hub path
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        this->_onPathChange();
      break;

    case IDC_BC_BRW02:
      this->_onBcBrwHome();
      break;

    case IDC_EC_INP03: // Resulting Mod Hub home path
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

          this->getItemText(IDC_EC_INP03, item_str);
          if(!Om_isValidPath(item_str))
            allow = false;

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
