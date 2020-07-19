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
  wstring item_str;

  this->getItemText(IDC_EC_INPT1, item_str);
  if(!Om_isValidName(item_str)) {
    Om_dialogBoxWarn(this->_hwnd, L"Invalid Context title", OMM_STR_ERR_VALIDNAME);
    return false;
  }

  this->getItemText(IDC_EC_INPT2, item_str);
  if(!Om_isValidPath(item_str)) {
    Om_dialogBoxWarn(this->_hwnd, L"Invalid Context home path", OMM_STR_ERR_VALIDPATH);
    return false;
  }

  this->getItemText(IDC_EC_INPT3, item_str);
  if(!Om_isValidPath(item_str)) {
    Om_dialogBoxWarn(this->_hwnd, L"Invalid Context definition filename", OMM_STR_ERR_VALIDPATH);
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxCfg::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Indicative name");

  this->_createTooltip(IDC_EC_INPT2,  L"Context home folder location");
  this->_createTooltip(IDC_BC_BROW2,  L"Select context location");

  // set default start values
  this->setItemText(IDC_EC_INPT1, L"New Context");
  this->setItemText(IDC_EC_INPT2, L"");
  this->setItemText(IDC_EC_INPT3, L"<invalid path>");

  // disable "next" button
  static_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxCfg::_onShow()
{
  wstring item_str;

  // enable or disable "next" button according values
  bool allow = true;

  this->getItemText(IDC_EC_INPT1, item_str);
  if(!item_str.empty()) {

    this->getItemText(IDC_EC_INPT2, item_str);
    if(!item_str.empty()) {

      this->getItemText(IDC_EC_INPT3, item_str);
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
void OmUiWizCtxCfg::_onResize()
{
  // Introduction text
  this->_setItemPos(IDC_SC_TEXT1, 10, 5, 190, 25);
  // Context title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 10, 40, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INPT1, 10, 50, this->width()-25, 13);
  // Context location Label & EditControl & Browse button
  this->_setItemPos(IDC_SC_LBL02, 10, 80, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INPT2, 10, 90, this->width()-45, 13);
  this->_setItemPos(IDC_BC_BROW2, this->width()-31, 90, 16, 13);
  // Result path Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 10, 150, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INPT3, 10, 160, this->width()-25, 13);
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
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    wstring item_st1, item_st2, brow_str;

    switch(LOWORD(wParam))
    {
    case IDC_EC_INPT1: // title
    case IDC_EC_INPT2: // Context location path
      this->getItemText(IDC_EC_INPT1, item_st1);
      if(Om_isValidName(item_st1)) {
        this->getItemText(IDC_EC_INPT2, item_st2);
        if(Om_isValidPath(item_st2)) {
          item_st2 += L"\\" + item_st1 + L"\\";
        } else {
          item_st2 = L"<invalid path>";
        }
      } else {
        item_st2 = L"<invalid path>";
      }
      this->setItemText(IDC_EC_INPT3, item_st2);
      break;

    case IDC_BC_BROW2:
      this->getItemText(IDC_EC_INPT2, item_st1);
      if(Om_dialogBrowseDir(brow_str, this->_hwnd, L"Select folder where to create Context home", item_st1)) {
        this->setItemText(IDC_EC_INPT2, brow_str);
        //manager->saveDefaultLocation(inpt2); //< TODO: ?
      }
      break;

    case IDC_EC_INPT3: // resulting Context path
      has_changed = true;
      break;
    }

    // enable or disable "next" button according values
    if(has_changed) {
      bool allow = true;

      this->getItemText(IDC_EC_INPT1, item_st1);
      if(!item_st1.empty()) {

        this->getItemText(IDC_EC_INPT2, item_st1);
        if(!item_st1.empty()) {

          this->getItemText(IDC_EC_INPT3, item_st1);
          if(!Om_isValidPath(item_st1)) allow = false;

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
