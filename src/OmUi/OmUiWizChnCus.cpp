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
#include "OmBaseUi.h"
#include "OmBaseApp.h"

#include "OmModHub.h"

#include "OmUtilDlg.h"
#include "OmUtilStr.h"

#include "OmUiWizChn.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizChnCus.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizChnCus::OmUiWizChnCus(HINSTANCE hins) : OmDialogWizPage(hins)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizChnCus::~OmUiWizChnCus()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizChnCus::id() const
{
  return IDD_WIZ_CHN_CUS;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizChnCus::validFields() const
{
  OmWString ec_entry;

  if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {

    this->getItemText(IDC_EC_INP01, ec_entry);

    if(ec_entry.empty())
      return false;
  }

  if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {

    this->getItemText(IDC_EC_INP02, ec_entry);

    if(ec_entry.empty())
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizChnCus::validParams() const
{
  OmWString ec_entry;

  if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {

    this->getItemText(IDC_EC_INP01, ec_entry);

    if(!Om_dlgValidPath(this->_hwnd, L"Mod Library path", ec_entry))
      return false;

    if(!Om_dlgValidDir(this->_hwnd, L"Mod Library directory", ec_entry))
      return false;
  }

  if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {

    this->getItemText(IDC_EC_INP02, ec_entry);

    if(!Om_dlgValidPath(this->_hwnd, L"Backup storage path", ec_entry))
      return false;

    if(!Om_dlgValidDir(this->_hwnd, L"Backup storage directory", ec_entry))
      return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChnCus::_library_cust_toggle()
{
  int bm_chk = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW01, bm_chk);
  this->enableItem(IDC_EC_INP01, bm_chk);

  if(bm_chk) {
    this->setItemText(IDC_EC_INP01, L"");
  } else {
    this->setItemText(IDC_EC_INP01, L"<hub home>\\<channel name>" OM_MODCHAN_MODLIB_DIR);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChnCus::_browse_dir_library()
{
  OmWString result;

  if(!Om_dlgOpenDir(result, this->_hwnd, L"Select directory for Mod Library, where Mods are to be found", L""))
    return;

  this->setItemText(IDC_EC_INP01, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChnCus::_backup_cust_toggle()
{
  int bm_chk = this->msgItem(IDC_BC_CKBX2, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW02, bm_chk);
  this->enableItem(IDC_EC_INP02, bm_chk);

  if(bm_chk) {
    this->setItemText(IDC_EC_INP02, L"");
  } else {
    this->setItemText(IDC_EC_INP02, L"<hub home>\\<channel name>" OM_MODCHAN_BACKUP_DIR);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChnCus::_browse_dir_backup()
{
  OmWString result;

  if(!Om_dlgOpenDir(result, this->_hwnd, L"Select directory for Backup storage, where backup data will be stored", L""))
    return;

  this->setItemText(IDC_EC_INP02, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChnCus::_onPgInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CKBX1,  L"Use a custom location for Mod Library");
  this->_createTooltip(IDC_EC_INP01,  L"Mod library directory, where Mods are to be found");
  this->_createTooltip(IDC_BC_BRW01,  L"Select custom Library directory");

  this->_createTooltip(IDC_BC_CKBX2,  L"Use a custom location for Backup storage");
  this->_createTooltip(IDC_EC_INP02,  L"Backup storage directory, where backup data will be stored");
  this->_createTooltip(IDC_BC_BRW02,  L"Select custom backup directory");

  // set default start values
  this->setItemText(IDC_EC_INP01, L"<hub home>\\<channel name>" OM_MODCHAN_MODLIB_DIR);
  this->setItemText(IDC_EC_INP02, L"<hub home>\\<channel name>" OM_MODCHAN_BACKUP_DIR);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizChnCus::_onPgResize()
{
  int32_t y_base = 120;

  // Introduction text
  this->_setItemPos(IDC_SC_HELP, 0, 0, this->cliWidth(), 90, true);

  // Mod Channel title Label & EditControl
  this->_setItemPos(IDC_BC_CKBX1, 0, y_base, this->cliWidth()-10, 16, true);
  this->_setItemPos(IDC_EC_INP01, 0, y_base+20, this->cliWidth()-35, 21, true);
  this->_setItemPos(IDC_BC_BRW01, this->cliWidth()-30, y_base+20, 22, 22, true);

  // Mod Channel target directory Label & EditControl & Browse button
  this->_setItemPos(IDC_BC_CKBX2, 0, y_base+70, this->cliWidth()-10, 16, true);
  this->_setItemPos(IDC_EC_INP02, 0, y_base+90, this->cliWidth()-35, 21, true);
  this->_setItemPos(IDC_BC_BRW02, this->cliWidth()-30, y_base+90, 22, 22, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiWizChnCus::_onPgMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_CKBX1: //< Custom Library CheckBox
      if(HIWORD(wParam) == BN_CLICKED)
        this->_library_cust_toggle();
      break;

    case IDC_BC_BRW01: //< Custom Library "..." (browse) Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_dir_library();
      break;

    case IDC_BC_CKBX2: //< Custom Backup CheckBox
      if(HIWORD(wParam) == BN_CLICKED)
        this->_backup_cust_toggle();
      break;

    case IDC_BC_BRW02: //< Custom Backup "..." (browse) Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_dir_backup();
      break;

    case IDC_EC_INP01: //< Custom Library path
    case IDC_EC_INP02: //< Custom Backup path
      if(HIWORD(wParam) == EN_CHANGE)
        this->fieldsChanged();
      break;
    }
  }

  return 0;
}
