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
#include "OmDialogProp.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialogProp::OmDialogProp(HINSTANCE hins) : OmDialog(hins),
  _pageName(),
  _pageDial(),
  _hTab(nullptr),
  _hBcOk(nullptr),
  _hBcApply(nullptr),
  _hBcCancel(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialogProp::~OmDialogProp()
{

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
bool OmDialogProp::applyChanges()
{
  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_addPage(const wstring& title, OmDialog* dialog)
{
  this->addChild(dialog);
  this->_pageDial.push_back(dialog);
  this->_pageName.push_back(title);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_onInit()
{
  // Retrieve handle to common controls
  this->_hTab = this->getItem(IDC_TC_PROP);
  this->_hBcOk = this->getItem(IDC_BC_OK);
  this->_hBcApply = this->getItem(IDC_BC_APPLY);
  this->_hBcCancel = this->getItem(IDC_BC_CANCEL);

  if(this->_pageDial.size() && this->_hwnd) {

    if(this->_hTab == nullptr)
      return;

    TCITEMW tcPage;
    tcPage.mask = TCIF_TEXT;

    for(size_t i = 0; i < this->_pageDial.size(); ++i) {

      tcPage.pszText = (LPWSTR)this->_pageName[i].c_str();
      SendMessageW(this->_hTab, TCM_INSERTITEMW, i, (LPARAM)&tcPage);

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
  this->_setItemPos(IDC_TC_PROP, 4, 5, this->width()-8, this->height()-28);

  if(this->_pageDial.size() && this->_hTab) {

    LONG pos[4];

    // get TabControl local coordinates
    GetWindowRect(hTab, (LPRECT)&pos);
    MapWindowPoints(HWND_DESKTOP, this->_hwnd, (LPPOINT)&pos, 2);

    // convert into base unit and adjust to keep inside the TabControl
    pos[0] = MulDiv(pos[0], 4, this->unitX()) + 3;
    pos[1] = MulDiv(pos[1], 8, this->unitY()) + 14;
    pos[2] = MulDiv(pos[2], 4, this->unitX()) - 3;
    pos[3] = MulDiv(pos[3], 8, this->unitY()) - 3;

    // Map again in pixels
    MapDialogRect(this->_hwnd, (LPRECT)&pos);
    pos[2] -= pos[0]; // width = right - left
    pos[3] -= pos[1]; // height = bottom - top

    // apply this for all dialogs
    for(size_t i = 0; i < this->_pageDial.size(); ++i) {
      SetWindowPos(this->_pageDial[i]->hwnd(), 0, pos[0], pos[1], pos[2], pos[3], SWP_NOZORDER|SWP_NOACTIVATE);
    }
  }

  // OK Button
  this->_setItemPos(IDC_BC_OK, this->width()-161, this->height()-19, 50, 14);
  // Cancel Button
  this->_setItemPos(IDC_BC_CANCEL, this->width()-108, this->height()-19, 50, 14);
  // Apply Button
  this->_setItemPos(IDC_BC_APPLY, this->width()-54, this->height()-19, 50, 14);

  // force buttons to redraw to prevent artifacts
  InvalidateRect(this->_hBcOk, nullptr, true);
  InvalidateRect(this->_hBcCancel, nullptr, true);
  InvalidateRect(this->_hBcApply, nullptr, true);

  this->_onPropResize();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmDialogProp::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {

    if(this->_pageDial.size() && this->_hTab) {

      // check for notify from the specified TabControl
      if(LOWORD(wParam) == IDC_TC_PROP) {

        NMHDR* pNmhdr = reinterpret_cast<NMHDR*>(lParam);

        // check for a "selection changed" notify
        if(pNmhdr->code == TCN_SELCHANGE) {

          // get TabControl current selection
          int tab_sel = SendMessageW(this->_hTab, TCM_GETCURSEL, 0, 0);

          // change page dialog visibility according selection
          if(tab_sel >= 0) {
            for(int i = 0; i < static_cast<int>(this->_pageDial.size()); ++i) {
              if(i == tab_sel) {
                this->_pageDial[i]->show();
              } else {
                this->_pageDial[i]->hide();
              }
            }
          }
        }
      }
    }
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_APPLY:
      this->applyChanges();
      break;

    case IDC_BC_OK:
      if(this->applyChanges()) {
        // quit the dialog
        this->quit();
      }
      break; // case BTN_OK:

    case IDC_BC_CANCEL:
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
bool OmDialogProp::_onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return false;
}
