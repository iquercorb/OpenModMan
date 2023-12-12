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

#include "OmUiWizRep.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizRepCfg.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizRepCfg::OmUiWizRepCfg(HINSTANCE hins) : OmDialogWizPage(hins)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizRepCfg::~OmUiWizRepCfg()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizRepCfg::id() const
{
  return IDD_WIZ_REP_CFG;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizRepCfg::validFields() const
{
  OmWString ec_entry;

  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {

    this->getItemText(IDC_EC_INP01, ec_entry);

    Om_trim(&ec_entry); //< trim string

    if(ec_entry.empty())
      return false;

    this->getItemText(IDC_EC_INP02, ec_entry);

    Om_trim(&ec_entry); //< trim string

    if(ec_entry.empty())
      return false;
  }

  if(this->msgItem(IDC_BC_RAD02, BM_GETCHECK)) {

    this->getItemText(IDC_EC_INP03, ec_entry);

    Om_trim(&ec_entry); //< trim string

    if(ec_entry.empty())
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizRepCfg::validParams() const
{
  OmWString ec_entry;

  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {

    this->getItemText(IDC_EC_INP01, ec_entry);

    Om_trim(&ec_entry); //< trim string

    if(!Om_dlgValidUrl(this->_hwnd, L"Repository base address", ec_entry))
      return false;

    this->getItemText(IDC_EC_INP02, ec_entry);

    Om_trim(&ec_entry); //< trim string

    if(!Om_dlgValidUrlPath(this->_hwnd, L"Repository name", ec_entry))
      return false;
  }

  if(this->msgItem(IDC_BC_RAD02, BM_GETCHECK)) {

    this->getItemText(IDC_EC_INP03, ec_entry);

    Om_trim(&ec_entry); //< trim string

    if(!Om_dlgValidUrl(this->_hwnd, L"Repository base address", ec_entry))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizRepCfg::_coord_type_toggle()
{
  int32_t rd1_chk = false;
  int32_t rd2_chk = false;

  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->msgItem(IDC_BC_RAD02, BM_SETCHECK, 0);
    rd1_chk = true;
  }

  if(this->msgItem(IDC_BC_RAD02, BM_GETCHECK)) {
    this->msgItem(IDC_BC_RAD01, BM_SETCHECK, 0);
    rd2_chk = true;
  }

  this->enableItem(IDC_EC_INP01, rd1_chk);
  this->enableItem(IDC_EC_INP02, rd1_chk);
  this->enableItem(IDC_EC_INP03, rd2_chk);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizRepCfg::_onPgInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_RAD01,  L"configure access via named Repository coordinates");
  this->_createTooltip(IDC_EC_INP01,  L"Repository base address");
  this->_createTooltip(IDC_EC_INP02,  L"Repository name");

  this->_createTooltip(IDC_BC_RAD02,  L"configure access via raw URL web address");
  this->_createTooltip(IDC_EC_INP03,  L"Repository data URL");

  // set default start values
  this->setItemText(IDC_EC_INP01, L"http://");
  this->setItemText(IDC_EC_INP02, L"default");
  this->setItemText(IDC_EC_INP03, L"http://");

  // set default states
  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, 1);
  this->msgItem(IDC_BC_RAD02, BM_SETCHECK, 0);

  this->enableItem(IDC_EC_INP01, true);
  this->enableItem(IDC_EC_INP02, true);
  this->enableItem(IDC_EC_INP03, false);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizRepCfg::_onPgResize()
{
  int32_t y_base = 90;
  int32_t tier_w = (static_cast<float>(this->cliWidth()) * 0.333f) * 2;

  // Introduction text
  this->_setItemPos(IDC_SC_HELP, 0, 0, this->cliWidth(), 70, true);

  // Named Repository Radio Button
  this->_setItemPos(IDC_BC_RAD01, 0, y_base, this->cliWidth()-10, 16, true);
  // Named Repository labels
  this->_setItemPos(IDC_SC_LBL01, 16, y_base+20, tier_w-25, 21, true);
  this->_setItemPos(IDC_SC_LBL02, tier_w+1, y_base+20, this->cliWidth()-tier_w, 21, true);
  // Named Repository Fields
  this->_setItemPos(IDC_EC_INP01, 15, y_base+40, tier_w-25, 21, true);
  this->_setItemPos(IDC_EC_INP02, tier_w-1, y_base+40, this->cliWidth()-tier_w, 21, true);

  // Raw URL Radio Button
  this->_setItemPos(IDC_BC_RAD02, 0, y_base+90, this->cliWidth()-10, 16, true);
  // Named Repository label and Entry
  this->_setItemPos(IDC_EC_INP03, 15, y_base+110, this->cliWidth()-16, 21, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiWizRepCfg::_onPgMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {
    switch(LOWORD(wParam))
    {
    case IDC_BC_RAD01:
    case IDC_BC_RAD02:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_coord_type_toggle();
      break;

    case IDC_EC_INP01:
    case IDC_EC_INP02:
    case IDC_EC_INP03:
      if(HIWORD(wParam) == EN_CHANGE)
        this->fieldsChanged();
      break;
    }
  }

  return 0;
}
