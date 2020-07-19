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
#include "OmManager.h"
#include "gui/OmUiPropLocStg.h"
#include "gui/OmUiPropLoc.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocStg::OmUiPropLocStg(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i)
    this->_chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocStg::~OmUiPropLocStg()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropLocStg::id() const
{
  return IDD_PROP_LOC_STG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::setChParam(unsigned i, bool en)
{
  _chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Indicative name");

  this->_createTooltip(IDC_EC_INPT2,  L"Package installation destination path");
  this->_createTooltip(IDC_BC_BROW2,  L"Select destination folder");

  this->_createTooltip(IDC_BC_CHK01,  L"Use custom Library folder");
  this->_createTooltip(IDC_EC_INPT3,  L"Custom Library folder path");
  this->_createTooltip(IDC_BC_BROW3,  L"Select custom Library folder");

  this->_createTooltip(IDC_BC_CHK02,  L"Use custom Backup folder");
  this->_createTooltip(IDC_EC_INPT4,  L"Custom Backup folder path");
  this->_createTooltip(IDC_BC_BROW4,  L"Select custom Backup folder");

  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onResize()
{
  // Location Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 20, 64, 9);
  this->_setItemPos(IDC_EC_INPT1, 70, 20, this->width()-90, 13);

  // Location Location Label, EditControl and Browse button
  this->_setItemPos(IDC_SC_LBL02, 5, 60, 64, 9);
  this->_setItemPos(IDC_EC_INPT2, 70, 60, this->width()-108, 13);
  this->_setItemPos(IDC_BC_BROW2, this->width()-35, 60, 16, 13);

  // Custom Library CheckBox
  this->_setItemPos(IDC_BC_CHK01, 70, 90, 120, 9);
  // Location Library Label, EditControl and Browse button
  this->_setItemPos(IDC_SC_LBL03, 5, 100, 64, 9);
  this->_setItemPos(IDC_EC_INPT3, 70, 100, this->width()-108, 13);
  this->_setItemPos(IDC_BC_BROW3, this->width()-35, 100, 16, 13);

  // Custom Backup CheckBox
  this->_setItemPos(IDC_BC_CHK02, 70, 120, 120, 9);
  // Location Backup Label, EditControl and Browse button
  this->_setItemPos(IDC_SC_LBL04, 5, 130, 64, 9);
  this->_setItemPos(IDC_EC_INPT4, 70, 130, this->width()-108, 13);
  this->_setItemPos(IDC_BC_BROW4, this->width()-35, 130, 16, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onRefresh()
{
  OmLocation* location = static_cast<OmUiPropLoc*>(this->_parent)->location();

  if(location == nullptr)
    return;

  this->setItemText(IDC_EC_INPT1, location->title());
  this->setItemText(IDC_EC_INPT2, location->installDir());

  this->setItemText(IDC_EC_INPT3, location->libraryDir());
  this->msgItem(IDC_BC_CHK01, BM_SETCHECK, location->hasCustLibraryDir());
  this->enableItem(IDC_EC_INPT3, location->hasCustLibraryDir());
  this->enableItem(IDC_BC_BROW3, location->hasCustLibraryDir());

  this->setItemText(IDC_EC_INPT4, location->backupDir());
  this->msgItem(IDC_BC_CHK02, BM_SETCHECK, location->hasCustBackupDir());
  this->enableItem(IDC_EC_INPT4, location->hasCustBackupDir());
  this->enableItem(IDC_BC_BROW4, location->hasCustBackupDir());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLocStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    OmLocation* location = static_cast<OmUiPropLoc*>(this->_parent)->location();

    if(location == nullptr)
      return false;

    bool bm_chk;

    wstring item_str, brow_str;

    switch(LOWORD(wParam))
    {
    case IDC_BC_CHK01: //< Check Box for custom Library path
      bm_chk = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);
      this->enableItem(IDC_EC_INPT3, bm_chk);
      this->enableItem(IDC_BC_BROW3, bm_chk);
      if(bm_chk && location->hasCustLibraryDir()) {
        this->setItemText(IDC_EC_INPT3, location->libraryDir());
      } else {
        this->setItemText(IDC_EC_INPT3, location->home() + L"\\library");
      }
      break;

    case IDC_BC_CHK02: //< Check Box for custom Backup path
      bm_chk = this->msgItem(IDC_BC_CHK02, BM_GETCHECK);
      this->enableItem(IDC_EC_INPT4, bm_chk);
      this->enableItem(IDC_BC_BROW4, bm_chk);
      if(bm_chk && location->hasCustBackupDir()) {
        this->setItemText(IDC_EC_INPT4, location->backupDir());
      } else {
        this->setItemText(IDC_EC_INPT4, location->home() + L"\\backup");
      }
      break;

    case IDC_EC_INPT1: //< Entry for Title
      // user modified parameter, notify it
      this->setChParam(LOC_PROP_STG_TITLE, true);
      break;

    case IDC_EC_INPT2: //< Entry for Install
      // user modified parameter, notify it
      this->setChParam(LOC_PROP_STG_INSTALL, true);
      break;

    case IDC_BC_BROW2: //< Browse Button for Install folder
      this->getItemText(IDC_EC_INPT2, item_str);
      if(Om_dialogBrowseDir(brow_str, this->_hwnd, L"Select packages Destination folder", item_str)) {
        this->setItemText(IDC_EC_INPT2, brow_str);
      }
      break;

    case IDC_EC_INPT3: //< Entry for Library
      // user modified parameter, notify it
      this->setChParam(LOC_PROP_STG_LIBRARY, true);
      break;

    case IDC_BC_BROW3: //< Browse Button for Library folder
      this->getItemText(IDC_EC_INPT3, item_str);
      if(Om_dialogBrowseDir(brow_str, this->_hwnd, L"Select custom packages Library folder", item_str)) {
        this->setItemText(IDC_EC_INPT3, brow_str);
      }
      break;

    case IDC_EC_INPT4: //< Entry for Backup
      // user modified parameter, notify it
      this->setChParam(LOC_PROP_STG_BACKUP, true);
      break;

    case IDC_BC_BROW4: //< Browse Button for Backup folder
      this->getItemText(IDC_EC_INPT4, item_str);
      if(Om_dialogBrowseDir(brow_str, this->_hwnd, L"Select custom Backups location", item_str)) {
        this->setItemText(IDC_EC_INPT4, brow_str);
      }
      break;
    }
  }

  return false;
}

