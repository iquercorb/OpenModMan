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
#include "OmBaseWin.h"
#include <UxTheme.h>        //< ETDT_ENABLETAB

#include "OmBaseUi.h"

#include "OmUtilWin.h"      //< Om_getResIcon, etc.

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmDialogProp.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialogProp::OmDialogProp(HINSTANCE hins) : OmDialog(hins),
  _noChanges(false)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialogProp::~OmDialogProp()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_TC_PROP, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmDialogProp::checkChanges()
{
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmDialogProp::validChanges()
{
  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmDialogProp::applyChanges()
{
  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_addPage(const OmWString& title, OmDialogPropTab* dialog)
{
  this->addChild(dialog);
  this->_pageDial.push_back(dialog);
  this->_pageName.push_back(title);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_setNoChange(bool enable)
{
  this->_noChanges = enable;

  this->showItem(IDC_BC_APPLY, !enable);
  this->showItem(IDC_BC_CANCEL, !enable);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(IDI_APP, 2), Om_getResIcon(IDI_APP, 1));

  DWORD ComStyle = WS_CHILD|WS_VISIBLE;

  CreateWindowExW(WS_EX_LEFT, WC_TABCONTROLW, L"", ComStyle|WS_CLIPSIBLINGS|WS_TABSTOP,
        5, 5, this->cliWidth()-10, this->cliHeight()-40,
        this->_hwnd, reinterpret_cast<HMENU>(IDC_TC_PROP), this->_hins, nullptr);

  CreateWindowExW(WS_EX_LEFT, L"BUTTON", L"Apply", ComStyle|WS_DISABLED|WS_TABSTOP,
        this->cliWidth()-248, this->cliHeight()-30, 78, 23,
        this->_hwnd, reinterpret_cast<HMENU>(IDC_BC_APPLY), this->_hins, nullptr);

  CreateWindowExW(WS_EX_LEFT, L"BUTTON", L"Cancel", ComStyle|WS_TABSTOP,
        this->cliWidth()-166, this->cliHeight()-30, 78, 23,
        this->_hwnd, reinterpret_cast<HMENU>(IDC_BC_CANCEL), this->_hins, nullptr);

  CreateWindowExW(WS_EX_LEFT, L"BUTTON", L"Close", ComStyle|WS_TABSTOP,
        this->cliWidth()-84, this->cliHeight()-30, 78, 23,
        this->_hwnd, reinterpret_cast<HMENU>(IDC_BC_CLOSE), this->_hins, nullptr);

  // Defines fonts for Mod Hub ComboBox
  HFONT hFt = Om_createFont(12, 200, L"Ms Shell Dlg");

  this->msgItem(IDC_TC_PROP, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  this->msgItem(IDC_BC_APPLY, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  this->msgItem(IDC_BC_CANCEL, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  this->msgItem(IDC_BC_CLOSE, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  // Retrieve handle to common controls
  this->_hTab = this->getItem(IDC_TC_PROP);

  this->showItem(IDC_BC_APPLY, !this->_noChanges);
  this->showItem(IDC_BC_CANCEL, !this->_noChanges);

  if(this->_pageDial.size() && this->_hwnd) {

    if(this->_hTab == nullptr)
      return;

    TCITEMW tcPage;
    tcPage.mask = TCIF_TEXT;

    for(size_t i = 0; i < this->_pageDial.size(); ++i) {

      tcPage.pszText = (LPWSTR)this->_pageName[i].c_str();
      SendMessageW(this->_hTab, TCM_INSERTITEMW, i, reinterpret_cast<LPARAM>(&tcPage));

      this->_pageDial[i]->modeless(false);
      EnableThemeDialogTexture(this->_pageDial[i]->hwnd(), ETDT_ENABLETAB);
    }

    this->_pageDial[0]->show();
  }

  this->_onPropInit();
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_onShow()
{
  this->_onPropShow();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_onResize()
{
  // Find the first TABCONTROL child of this dialog
  HWND hTab = FindWindowEx(this->_hwnd, nullptr, WC_TABCONTROL, nullptr);

  // TabControl
  this->_setItemPos(IDC_TC_PROP, 5, 5, this->cliWidth()-10, this->cliHeight()-40, true);

  if(this->_pageDial.size() && this->_hTab) {

    LONG pos[4];

    // get TabControl local coordinates
    GetWindowRect(hTab, reinterpret_cast<LPRECT>(&pos));
    MapWindowPoints(HWND_DESKTOP, this->_hwnd, reinterpret_cast<LPPOINT>(&pos), 2);

    pos[0] += 1;  //< add 1 pixel margin
    pos[1] += 21; //< add 21 pixel for tabs
    pos[2] -= pos[0] + 3; //< width = (right - left) - margin
    pos[3] -= pos[1] + 2; //< height = (bottom - top) - margin

    // apply this for all dialogs
    for(size_t i = 0; i < this->_pageDial.size(); ++i) {
      this->_setChildPos(this->_pageDial[i]->hwnd(), pos[0], pos[1], pos[2], pos[3], true);
    }
  }

  // Apply Button
  this->_setItemPos(IDC_BC_APPLY, this->cliWidth()-248, this->cliHeight()-30, 78, 23, true);
  // Cancel Button
  this->_setItemPos(IDC_BC_CANCEL, this->cliWidth()-166, this->cliHeight()-30, 78, 23, true);
  // Close Button
  this->_setItemPos(IDC_BC_CLOSE, this->cliWidth()-84, this->cliHeight()-30, 78, 23, true);

  this->_onPropResize();

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_onQuit()
{
  this->_onPropQuit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmDialogProp::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {

    if(this->_pageDial.size() && this->_hTab) {

      // check for notify from the specified TabControl
      if(LOWORD(wParam) == IDC_TC_PROP) {

        NMHDR* pNmhdr = reinterpret_cast<NMHDR*>(lParam);

        // check for a "selection changed" notify
        if(pNmhdr->code == TCN_SELCHANGE) {

          // get TabControl current selection
          int tc_sel = SendMessageW(this->_hTab, TCM_GETCURSEL, 0, 0);

          // change page dialog visibility according selection
          if(tc_sel >= 0) {
            // hide all visible tables
            for(size_t i = 0; i < this->_pageDial.size(); ++i) {
              if(this->_pageDial[i]->visible()) {
                this->_pageDial[i]->hide(); break;
              }
            }
            // show selected tab
            if(!this->_pageDial[tc_sel]->visible())
              this->_pageDial[tc_sel]->show();
          }
        }
      }
    }
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_APPLY:
      if(HIWORD(wParam) == BN_CLICKED)
        if(this->validChanges()) {
          if(this->applyChanges()) {
            // refresh all dialogs from root (Main dialog)
            this->root()->refresh();
          }
        }
      break;

    case IDC_BC_CLOSE:
      if(HIWORD(wParam) == BN_CLICKED)
        if(this->validChanges()) {
          if(this->applyChanges()) {
            // quit the dialog
            this->quit();
            // refresh all dialogs from root (Main dialog)
            this->root()->refresh();
          }
        }
      break; // case BTN_OK:

    case IDC_BC_CANCEL:
      if(HIWORD(wParam) == BN_CLICKED)
        this->quit();
      break; // case BTN_CANCEL:
    }
  }

  return this->_onPropMsg(uMsg, wParam, lParam);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_onPropInit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_onPropShow()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_onPropResize()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_onPropQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmDialogProp::_onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(uMsg); OM_UNUSED(wParam);  OM_UNUSED(lParam);

  return false;
}
