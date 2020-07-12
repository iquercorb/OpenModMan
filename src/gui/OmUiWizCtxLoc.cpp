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

#include "OmUiWizCtxLoc.h"
#include "gui/res/resource.h"
#include "gui/OmUiWizCtx.h"
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
  return IDD_WIZ_LOC_CFG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizCtxLoc::hasValidParams() const
{
  wchar_t wcbuf[OMM_MAX_PATH];

  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
  if(wcslen(wcbuf)) {
    if(!Om_isValidName(wcbuf)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid Location title",
                                    L"The Location title contain "
                                    L"illegal character(s)");
      return false;
    }
  } else {
    Om_dialogBoxWarn(this->_hwnd, L"Invalid Location title",
                                  L"Please choose a Location title.");
    return false;
  }
  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
  if(wcslen(wcbuf)) {
    if(!Om_isDir(wcbuf)) {
        Om_dialogBoxErr(this->_hwnd,  L"Invalid Location Destination folder",
                                      L"Please select an existing folder for "
                                      L"Location Destination.");
      return false;
    }
  } else {
      Om_dialogBoxErr(this->_hwnd,  L"Invalid Location Destination path",
                                    L"Please enter a valid path for Location "
                                    L"Destination folder.");
    return false;
  }
  if(SendMessage(GetDlgItem(this->_hwnd,IDC_BC_CHK01),BM_GETCHECK,0,0)) {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf, OMM_MAX_PATH);
    if(wcslen(wcbuf)) {
      if(!Om_isDir(wcbuf)) {
        Om_dialogBoxWarn(this->_hwnd, L"Invalid Location Library folder",
                                      L"Please choose an existing folder for "
                                      L"Location custom Library");
        return false;
      }
    } else {
        Om_dialogBoxWarn(this->_hwnd, L"Invalid Location Library folder",
                                      L"Please enter a valid path for Location "
                                      L"custom Library folder");
      return false;
    }
  }
  if(SendMessage(GetDlgItem(this->_hwnd,IDC_BC_CHK02),BM_GETCHECK,0,0)) {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT5, wcbuf, OMM_MAX_PATH);
    if(wcslen(wcbuf)) {
      if(!Om_isDir(wcbuf)) {
        Om_dialogBoxWarn(this->_hwnd, L"Invalid Location Backup folder",
                                      L"Please choose an existing folder for "
                                      L"Location custom Backup");
        return false;
      }
    } else {
        Om_dialogBoxWarn(this->_hwnd, L"Invalid Location Backup folder",
                                      L"Please enter a valid path for Location "
                                      L"custom Backup folder");
      return false;
    }
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onShow()
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

  wchar_t wcbuf[OMM_MAX_PATH];

  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
  if(!wcslen(wcbuf)) {

    SetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, L"Main Location");

    swprintf(wcbuf, OMM_MAX_PATH, L"Main Location\\library");
    SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, wcbuf);

    swprintf(wcbuf, OMM_MAX_PATH, L"Main Location\\backup");
    SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf);

    reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(false);

    return;
  }

  bool allow = true;

  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
  if(Om_isValidName(wcbuf)) {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
    if(wcslen(wcbuf)) {

      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01),BM_GETCHECK,0,0)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, wcbuf, OMM_MAX_PATH);
        if(!wcslen(wcbuf)) allow = false;
      }

      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK02),BM_GETCHECK,0,0)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf, OMM_MAX_PATH);
        if(!wcslen(wcbuf)) allow = false;
      }

    } else {
      allow = false;
    }
  } else {
    allow = false;
  }

  reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(allow);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxLoc::_onResize()
{
  // Introduction text
  this->_setControlPos(IDC_SC_TEXT1, 10, 5, 190, 25);
  // Location title Label & EditControl
  this->_setControlPos(IDC_SC_LBL01, 10, 40, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT1, 10, 50, this->width()-25, 13);
  // Location Install Label & EditControl & Browse button
  this->_setControlPos(IDC_SC_LBL02, 10, 80, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT2, 10, 90, this->width()-45, 13);
  this->_setControlPos(IDC_BC_BROW2, this->width()-31, 90, 16, 13);
  // Custom Library Label & EditControl & Browse buttonben ess
  this->_setControlPos(IDC_BC_CHK01, 10, 120, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT3, 10, 130, this->width()-45, 13);
  this->_setControlPos(IDC_BC_BROW3, this->width()-31, 130, 16, 13);
  // Custom Library Label & EditControl & Browse button
  this->_setControlPos(IDC_BC_CHK02, 10, 150, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT4, 10, 160, this->width()-45, 13);
  this->_setControlPos(IDC_BC_BROW4, this->width()-31, 160, 16, 13);
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

    wchar_t inpt1[OMM_MAX_PATH];
    wchar_t inpt3[OMM_MAX_PATH];
    wchar_t inpt2[OMM_MAX_PATH];
    wchar_t inpt4[OMM_MAX_PATH];

    switch(LOWORD(wParam))
    {

    case IDC_EC_INPT1: // Title
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, inpt1, OMM_MAX_PATH);
      if(wcslen(inpt1) && Om_isValidName(inpt1)) {
        if(!SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01),BM_GETCHECK,0,0)) {
          swprintf(inpt3, OMM_MAX_PATH, L"%ls\\library", inpt1);
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, inpt3);
        }
        if(!SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK02),BM_GETCHECK,0,0)) {
          swprintf(inpt4, OMM_MAX_PATH, L"%ls\\backup", inpt1);
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, inpt4);
        }
      } else {
        if(!SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01),BM_GETCHECK,0,0)) {
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, L"<invalid path>\\library");
        }
        if(!SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK02),BM_GETCHECK,0,0)) {
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, L"<invalid path>\\backup");
        }
      }
      has_changed = true;
      break;

    case IDC_BC_BROW2:
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, inpt2, OMM_MAX_PATH);
      if(Om_dialogBrowseDir(inpt2, this->_hwnd, L"Select installation Location folder", inpt2)) {
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, inpt2);
      }
      break;

    case IDC_BC_CHK01:
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW3), true);
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, L"");
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW3), false);
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, inpt1, OMM_MAX_PATH);
        if(wcslen(inpt1) && Om_isValidName(inpt1)) {
          swprintf(inpt3, OMM_MAX_PATH, L"%ls\\library", inpt1);
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, inpt3);
        } else {
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, L"<invalid path>\\library");
        }
      }
    break;

    case IDC_BC_BROW3:
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, inpt3, OMM_MAX_PATH);
      if(Om_dialogBrowseDir(inpt3, this->_hwnd, L"Select custom packages Library", inpt3)) {
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, inpt3);
      }
      break;

    case IDC_BC_CHK02:
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK02), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT4), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), true);
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, L"");
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT4), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), false);
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, inpt1, OMM_MAX_PATH);
        if(wcslen(inpt1) && Om_isValidName(inpt1)) {
          swprintf(inpt4, OMM_MAX_PATH, L"%ls\\backup", inpt1);
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, inpt4);
        } else {
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, L"<invalid path>\\backup");
        }
      }
    break;

    case IDC_BC_BROW4:
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, inpt4, OMM_MAX_PATH);
      if(Om_dialogBrowseDir(inpt4, this->_hwnd, L"Select custom Backups location", inpt4)) {
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, inpt4);
      }
      break;

    case IDC_EC_INPT2: //< Location
    case IDC_EC_INPT3: //< Library
    case IDC_EC_INPT4: //< backup
      has_changed = true;
      break;
    }

    if(has_changed) {
      bool allow = true;
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, inpt1, OMM_MAX_PATH);
      if(Om_isValidName(inpt1)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, inpt2, OMM_MAX_PATH);
        if(wcslen(inpt2)) {
          if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01),BM_GETCHECK,0,0)) {
            GetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, inpt3, OMM_MAX_PATH);
            if(!wcslen(inpt3)) allow = false;
          }
          if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK02),BM_GETCHECK,0,0)) {
            GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, inpt4, OMM_MAX_PATH);
            if(!wcslen(inpt4)) allow = false;
          }
        } else {
          allow = false;
        }
      } else {
        allow = false;
      }
      reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(allow);
    }

  }

  return false;
}
