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
#include "gui/OmUiNewLoc.h"
#include "OmManager.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewLoc::OmUiNewLoc(HINSTANCE hins) : OmDialog(hins),
  _context(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewLoc::~OmUiNewLoc()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiNewLoc::id() const
{
  return IDD_NEW_LOC;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewLoc::_onShow()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Location main identifier");

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

  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_OK), allow);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewLoc::_onResize()
{
  // Location title Label & EditControl
  this->_setControlPos(IDC_SC_LBL01, 10, 10, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT1, 10, 20, this->width()-25, 13);

  // Location Install Label & EditControl & Browse button
  this->_setControlPos(IDC_SC_LBL02, 10, 50, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT2, 10, 60, this->width()-45, 13);
  this->_setControlPos(IDC_BC_BROW2, this->width()-31, 60, 16, 13);

  // Custom Library Label & EditControl & Browse buttonben ess
  this->_setControlPos(IDC_BC_CHK01, 10, 110, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT3, 10, 120, this->width()-45, 13);
  this->_setControlPos(IDC_BC_BROW3, this->width()-31, 120, 16, 13);

  // Custom Library Label & EditControl & Browse button
  this->_setControlPos(IDC_BC_CHK02, 10, 140, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT4, 10, 150, this->width()-45, 13);
  this->_setControlPos(IDC_BC_BROW4, this->width()-31, 150, 16, 13);

  // ---- separator
  this->_setControlPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Ok and Cancel buttons
  this->_setControlPos(IDC_BC_OK, this->width()-110, this->height()-19, 50, 14);
  this->_setControlPos(IDC_BC_CANCEL, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewLoc::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewLoc::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewLoc::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    case IDC_BC_OK:
      this->_apply();
      break; // case IDC_BC_OK:

    case IDC_BC_CANCEL:
      this->quit();
      break; // case IDC_BC_CANCEL:

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
      EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_OK), allow);
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewLoc::_apply()
{
  OmContext* context = this->_context;

  if(context == nullptr)
    return false;

  bool chk01, chk02;
  wchar_t loc_titl[OMM_MAX_PATH];
  wchar_t loc_inst[OMM_MAX_PATH];
  wchar_t loc_libd[OMM_MAX_PATH];
  wchar_t loc_bckd[OMM_MAX_PATH];

  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, loc_titl, OMM_MAX_PATH);
  if(wcslen(loc_titl)) {
    if(!Om_isValidName(loc_titl)) {
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
  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, loc_inst, OMM_MAX_PATH);
  if(wcslen(loc_inst)) {
    if(!Om_isDir(loc_inst)) {
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

  chk01 = SendMessage(GetDlgItem(this->_hwnd,IDC_BC_CHK01),BM_GETCHECK,0,0);
  if(chk01) {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, loc_libd, OMM_MAX_PATH);
    if(wcslen(loc_libd)) {
      if(!Om_isDir(loc_libd)) {
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
  } else {
    loc_libd[0] = L'\0'; //< disable custom library folder
  }

  chk02 = SendMessage(GetDlgItem(this->_hwnd,IDC_BC_CHK02),BM_GETCHECK,0,0);
  if(chk02) {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT5, loc_bckd, OMM_MAX_PATH);
    if(wcslen(loc_bckd)) {
      if(!Om_isDir(loc_bckd)) {
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
  } else {
    loc_bckd[0] = L'\0'; //< disable custom backup folder
  }

  this->quit();

  // create new Location in Context
  if(!context->makeLocation(loc_titl, loc_inst, loc_libd, loc_bckd)) {
    Om_dialogBoxErr(this->_hwnd, L"Location creation failed", context->lastError());
  }

  // refresh all tree from the main dialog
  this->root()->refresh();

  return true;
}

