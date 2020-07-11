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
  reinterpret_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onShow()
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
  this->_setControlPos(IDC_SC_LBL01, 5, 20, 64, 9);
  this->_setControlPos(IDC_EC_INPT1, 70, 20, this->width()-90, 13);

  // Location Location Label, EditControl and Browse button
  this->_setControlPos(IDC_SC_LBL02, 5, 60, 64, 9);
  this->_setControlPos(IDC_EC_INPT2, 70, 60, this->width()-108, 13);
  this->_setControlPos(IDC_BC_BROW2, this->width()-35, 60, 16, 13);

  // Custom Library CheckBox
  this->_setControlPos(IDC_BC_CHK01, 70, 90, 120, 9);
  // Location Library Label, EditControl and Browse button
  this->_setControlPos(IDC_SC_LBL03, 5, 100, 64, 9);
  this->_setControlPos(IDC_EC_INPT3, 70, 100, this->width()-108, 13);
  this->_setControlPos(IDC_BC_BROW3, this->width()-35, 100, 16, 13);

  // Custom Backup CheckBox
  this->_setControlPos(IDC_BC_CHK02, 70, 120, 120, 9);
  // Location Backup Label, EditControl and Browse button
  this->_setControlPos(IDC_SC_LBL04, 5, 130, 64, 9);
  this->_setControlPos(IDC_EC_INPT4, 70, 130, this->width()-108, 13);
  this->_setControlPos(IDC_BC_BROW4, this->width()-35, 130, 16, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onRefresh()
{
  OmLocation* location = reinterpret_cast<OmUiPropLoc*>(this->_parent)->location();

  if(location == nullptr)
    return;

  SetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, location->title().c_str());
  SetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, location->installDir().c_str());

  SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, location->libraryDir().c_str());
  if(location->hasCustLibraryDir()) {
    SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01), BM_SETCHECK, 1, 0);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), true);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW3), true);
  } else {
    SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01), BM_SETCHECK, 0, 0);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW3), false);
  }

  SetDlgItemTextW(_hwnd, IDC_EC_INPT4, location->backupDir().c_str());
  if(location->hasCustBackupDir()) {
    SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK02), BM_SETCHECK, 1, 0);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT4), true);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), true);
  } else {
    SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK02), BM_SETCHECK, 0, 0);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT4), false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), false);
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLocStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    OmLocation* location = reinterpret_cast<OmUiPropLoc*>(this->_parent)->location();

    if(location == nullptr)
      return false;

    bool chk01;
    wchar_t wcbuf[OMM_MAX_PATH];

    switch(LOWORD(wParam))
    {
    case IDC_BC_CHK01: //< Check Box for custom Library path
      chk01 = SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01), BM_GETCHECK, 0, 0);

      EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), chk01);
      EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW3), chk01);

      if(chk01 && location->hasCustLibraryDir()) {
        swprintf(wcbuf, L"%ls", location->libraryDir().c_str());
      } else {
        swprintf(wcbuf, L"%ls\\library", location->home().c_str());
      }
      SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, wcbuf);
      break;

    case IDC_BC_CHK02: //< Check Box for custom Backup path
      chk01 = SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK02), BM_GETCHECK, 0, 0);

      EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT4), chk01);
      EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), chk01);

      if(chk01 && location->hasCustBackupDir()) {
        swprintf(wcbuf, L"%ls", location->backupDir().c_str());
      } else {
        swprintf(wcbuf, L"%ls\\backup", location->home().c_str());
      }
      SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf);
      break;

    case IDC_EC_INPT1: //< Entry for Title
      // user modified parameter, notify it
      this->setChParam(LOC_PROP_STG_TITLE, true);
      break;

    case IDC_EC_INPT2: //< Entry for Install
      // user modified parameter, notify it
      this->setChParam(LOC_PROP_STG_INSTALL, true);
      break;

    case IDC_EC_INPT3: //< Entry for Library
      // user modified parameter, notify it
      this->setChParam(LOC_PROP_STG_LIBRARY, true);
      break;

    case IDC_EC_INPT4: //< Entry for Backup
      // user modified parameter, notify it
      this->setChParam(LOC_PROP_STG_BACKUP, true);
      break;

    case IDC_BC_BROW2: //< Browse Button for Install folder
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);

      if(Om_dialogBrowseDir(wcbuf, this->_hwnd, L"Select packages Destination folder", wcbuf)) {
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf);
      }
      break;

    case IDC_BC_BROW3: //< Browse Button for Library folder
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, wcbuf, OMM_MAX_PATH);

      if(Om_dialogBrowseDir(wcbuf, this->_hwnd, L"Select custom packages Library folder", wcbuf)) {
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, wcbuf);
      }
      break;

    case IDC_BC_BROW4: //< Browse Button for Backup folder
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf, OMM_MAX_PATH);

      if(Om_dialogBrowseDir(wcbuf, this->_hwnd, L"Select custom Backups location", wcbuf)) {
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf);
      }
      break;
    }
  }

  return false;
}

