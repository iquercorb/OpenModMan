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

#include "OmUtilDlg.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizHubCfg.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizHubCfg::OmUiWizHubCfg(HINSTANCE hins) : OmDialogWizPage(hins)
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
bool OmUiWizHubCfg::validFields() const
{
  OmWString ec_entry;

  this->getItemText(IDC_EC_INP01, ec_entry);
  if(ec_entry.empty())
    return false;

  this->getItemText(IDC_EC_INP02, ec_entry);
  if(ec_entry.empty())
    return false;

  this->getItemText(IDC_EC_INP03, ec_entry);
  if(!Om_hasLegalPathChar(ec_entry))
    return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizHubCfg::validParams() const
{
  OmWString ec_entry;

  this->getItemText(IDC_EC_INP01, ec_entry);
  if(!Om_dlgValidName(this->_hwnd, L"Hub name", ec_entry))
    return false;

  this->getItemText(IDC_EC_INP02, ec_entry);
  if(!Om_dlgValidPath(this->_hwnd, L"Home directory", ec_entry))
    return false;

  if(!Om_dlgCreateFolder(this->_hwnd, L"Home directory", ec_entry))
    return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubCfg::_update_hub_path()
{
  OmWString name, path;

  this->getItemText(IDC_EC_INP01, name);
  this->getItemText(IDC_EC_INP02, path);

  if(!Om_hasLegalSysChar(name) || !Om_hasLegalPathChar(path)) {
    this->setItemText(IDC_EC_INP03, L"<invalid path>");
    return;
  }

  path += L"\\" + name + L"\\";
  this->setItemText(IDC_EC_INP03, path);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubCfg::_browse_dir_home()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP02, start);

  if(!Om_dlgOpenDir(result, this->_hwnd, L"Select Mod Hub home location, where to create Hub home directory", start))
    return;

  this->setItemText(IDC_EC_INP02, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubCfg::_onPgInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Hub name, both to identify it and create home directory");

  this->_createTooltip(IDC_EC_INP02,  L"Hub home location, where to create Hub home directory");
  this->_createTooltip(IDC_BC_BRW02,  L"Browse to select Hub home location");

  // set default start values
  this->setItemText(IDC_EC_INP01, L"New Mod Hub");
  this->setItemText(IDC_EC_INP02, L"");
  this->setItemText(IDC_EC_INP03, L"<invalid path>");
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizHubCfg::_onPgResize()
{
  int32_t y_base = 90;

  // Introduction text
  this->_setItemPos(IDC_SC_HELP, 0, 0, this->cliWidth(), 70, true);

  // Mod Hub title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 0, y_base, this->cliWidth()-10, 16, true);
  this->_setItemPos(IDC_EC_INP01, 0, y_base+20, this->cliWidth()-10, 21, true);

  // Mod Hub location Label & EditControl & Browse button
  this->_setItemPos(IDC_SC_LBL02, 0, y_base+60, this->cliWidth()-10, 16, true);
  this->_setItemPos(IDC_EC_INP02, 0, y_base+80, this->cliWidth()-35, 21, true);
  this->_setItemPos(IDC_BC_BRW02, this->cliWidth()-30, y_base+80, 22, 22, true);

  // Result path Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 0, y_base+140, this->cliWidth()-10, 16, true);
  this->_setItemPos(IDC_EC_INP03, 0, y_base+160, this->cliWidth()-10, 21, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiWizHubCfg::_onPgMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01: // Title
    case IDC_EC_INP02: // Mod Hub path
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        this->_update_hub_path();
      break;

    case IDC_BC_BRW02:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_dir_home();
      break;

    case IDC_EC_INP03: // Resulting Mod Hub home path
      if(HIWORD(wParam) == EN_CHANGE)
        this->fieldsChanged();
      break;
    }
  }

  return false;
}
