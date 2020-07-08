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

#include "gui/OmUiWizCtxCfg.h"
#include "gui/res/resource.h"
#include "gui/OmUiWizCtx.h"
#include "OmManager.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizCtxCfg::OmUiWizCtxCfg(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizCtxCfg::~OmUiWizCtxCfg()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizCtxCfg::id() const
{
  return IDD_WIZ_CTX_CFG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizCtxCfg::hasValidParams() const
{
  wchar_t inpt1[MAX_PATH];
  wchar_t inpt2[MAX_PATH];
  wchar_t inpt3[MAX_PATH];

  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, inpt1, MAX_PATH);
  if(wcslen(inpt1)) {
    if(!Om_isValidName(inpt1)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid Context title",
                                    L"The Context title contain "
                                    L"illegal character(s)");
      return false;
    }
  } else {
    Om_dialogBoxWarn(this->_hwnd, L"Invalid Context title",
                                  L"Please choose a Context title.");
    return false;
  }
  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, inpt2, MAX_PATH);
  if(wcslen(inpt2)) {
    if(!Om_isDir(inpt2)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid Context location folder",
                                    L"Please choose an existing folder "
                                    L"for new Context location");
      return false;
    }
  } else {
    Om_dialogBoxWarn(this->_hwnd, L"Invalid Context location folder",
                                  L"Please choose an existing folder "
                                  L"for new Context location");
    return false;
  }
  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, inpt3, MAX_PATH);
  if(wcslen(inpt3)) {
    if(!Om_isValidName(inpt3)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid Context definition file name",
                                    L"The Context definition file name "
                                    L"contain illegal character(s)");
      return false;
    }
  } else {
    Om_dialogBoxWarn(this->_hwnd, L"Invalid Context definition file name",
                                  L"Please enter a valid file name "
                                  L"for Context definition");
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxCfg::_onShow()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Indicative name");

  this->_createTooltip(IDC_EC_INPT2,  L"Context home folder location");
  this->_createTooltip(IDC_BC_BROW2,  L"Select context location");

  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  wchar_t inpt1[MAX_PATH];
  wchar_t inpt3[MAX_PATH];

  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, inpt1, MAX_PATH);

  // set default entry contents
  if(!wcslen(inpt1)) {
    wstring path;
    manager->getDefaultLocation(path);
    if(path.size()) {
      SetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, path.c_str());
    }
    SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, L"<invalid path>");
    reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(false);
  } else {
    // enable or disable the Wizard Next button
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, inpt3, MAX_PATH);
    if(wcslen(inpt3)) {
      if(Om_isValidName(inpt3)) {
        reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(true);
      } else {
        reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(false);
      }
    } else {
      reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(false);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxCfg::_onResize()
{
  // Introduction text
  this->_setControlPos(IDC_SC_TEXT1, 10, 5, 190, 25);
  // Context title Label & EditControl
  this->_setControlPos(IDC_SC_LBL01, 10, 40, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT1, 10, 50, this->width()-25, 13);
  // Context location Label & EditControl & Browse button
  this->_setControlPos(IDC_SC_LBL02, 10, 80, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT2, 10, 90, this->width()-45, 13);
  this->_setControlPos(IDC_BC_BROW2, this->width()-31, 90, 16, 13);
  // Result path Label & EditControl
  this->_setControlPos(IDC_SC_LBL03, 10, 150, this->width()-25, 9);
  this->_setControlPos(IDC_EC_INPT3, 10, 160, this->width()-25, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxCfg::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxCfg::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizCtxCfg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  wchar_t inpt1[MAX_PATH];
  wchar_t inpt2[MAX_PATH];
  wchar_t inpt3[MAX_PATH];

  if(uMsg == WM_COMMAND) {
    switch(LOWORD(wParam))
    {
    case IDC_EC_INPT1: // title
    case IDC_EC_INPT2: // Context location path
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, inpt1, MAX_PATH);
      if(wcslen(inpt1)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, inpt2, MAX_PATH);
        if(wcslen(inpt2)) {
          swprintf(inpt3, MAX_PATH, L"%ls\\%ls\\", inpt2, inpt1, OMM_CTX_FILE_EXT);
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, inpt3);
        } else {
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, L"<invalid path>");
        }
      } else {
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, L"<invalid path>");
      }
      break;

    case IDC_BC_BROW2:
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, inpt2, MAX_PATH);
      if(Om_dialogBrowseDir(inpt2, this->_hwnd, L"Select folder where to create Context home", inpt2)) {
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, inpt2);
        manager->saveDefaultLocation(inpt2);
      }
      break; // case BTN_BROWSE1:

    case IDC_EC_INPT3: // resulting Context path
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, inpt3, MAX_PATH);
      if(wcslen(inpt3)) {
        if(Om_isValidName(inpt3)) {
          reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(true);
        } else {
          reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(false);
        }
      } else {
        reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(false);
      }
      break;
    }
  }


  return false;
}
