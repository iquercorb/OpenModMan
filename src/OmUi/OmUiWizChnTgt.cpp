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
#include "OmUiWizChnTgt.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizChnTgt::OmUiWizChnTgt(HINSTANCE hins) : OmDialogWizPage(hins)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizChnTgt::~OmUiWizChnTgt()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizChnTgt::id() const
{
  return IDD_WIZ_CHN_TGT;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizChnTgt::validFields() const
{
  OmWString ec_entry;

  this->getItemText(IDC_EC_INP01, ec_entry);
  if(ec_entry.empty())
    return false;

  this->getItemText(IDC_EC_INP02, ec_entry);
  if(ec_entry.empty())
    return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizChnTgt::validParams() const
{
  OmWString item_str;

  this->getItemText(IDC_EC_INP01, item_str);
  if(!Om_dlgValidName(this->_hwnd, L"Channel name", item_str))
    return false;

  this->getItemText(IDC_EC_INP02, item_str);
  if(!Om_dlgValidPath(this->_hwnd, L"Target directory", item_str))
    return false;

  if(!Om_dlgValidDir(this->_hwnd, L"Target directory", item_str))
    return false;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChnTgt::_browse_dir_target()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP02, start);

  if(!Om_dlgOpenDir(result, this->_hwnd, L"Select modding target directory, where Mods are to be applied", start))
    return;

  this->setItemText(IDC_EC_INP02, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChnTgt::_onPgInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Channel name, both to identify it and create home directory");

  this->_createTooltip(IDC_EC_INP02,  L"Channel target directory, where Mods are to be applied");
  this->_createTooltip(IDC_BC_BRW02,  L"Browse to select target directory");

  // set default start values
  this->setItemText(IDC_EC_INP01, L"New Channel");
  this->setItemText(IDC_EC_INP02, L"");
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChnTgt::_onPgResize()
{
  int32_t y_base = 70;

  // Introduction text
  this->_setItemPos(IDC_SC_HELP, 0, 0, this->cliWidth(), 70, true);

  // Mod Channel title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 0, y_base, this->cliWidth()-10, 16, true);
  this->_setItemPos(IDC_EC_INP01, 0, y_base+20, this->cliWidth()-10, 21, true);

  // Mod Channel target directory Label & EditControl & Browse button
  this->_setItemPos(IDC_SC_LBL02, 0, y_base+80, this->cliWidth()-10, 16, true);
  this->_setItemPos(IDC_EC_INP02, 0, y_base+100, this->cliWidth()-35, 21, true);
  this->_setItemPos(IDC_BC_BRW02, this->cliWidth()-30, y_base+100, 22, 22, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiWizChnTgt::_onPgMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01: // Title
    case IDC_EC_INP02: // Target Directory
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        this->fieldsChanged();
      break;

    case IDC_BC_BRW02:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_dir_target();
      break;
    }
  }

  return false;
}
