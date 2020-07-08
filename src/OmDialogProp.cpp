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

#include "OmDialogProp.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialogProp::OmDialogProp(HINSTANCE hins) : OmDialog(hins),
  _pageName(),
  _pageDial()
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
void OmDialogProp::_pagesOnShow(unsigned tab_id)
{
  if(this->_pageDial.size() && this->_hwnd) {

    TCITEMW tcPage;
    tcPage.mask = TCIF_TEXT;

    for(unsigned i = 0; i < this->_pageDial.size(); ++i) {

      tcPage.pszText = (LPWSTR)this->_pageName[i].c_str();
      SendMessageW(GetDlgItem(this->_hwnd, tab_id), TCM_INSERTITEMW, i, (LPARAM)&tcPage);

      this->_pageDial[i]->modeless(false);
      EnableThemeDialogTexture(this->_pageDial[i]->hwnd(), ETDT_ENABLETAB);
    }

    this->_pageDial[0]->show();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_pagesOnNotify(unsigned tab_id, WPARAM wParam, LPARAM lParam)
{
  if(this->_pageDial.size()) {

    // check for notify from the specified TabControl
    if(LOWORD(wParam) == tab_id) {
      // check for a "selection changed" notify
      if(((LPNMHDR)lParam)->code == TCN_SELCHANGE) {

        // get TabControl current selection
        int tab_sel = SendMessageW(GetDlgItem(this->_hwnd, tab_id), TCM_GETCURSEL, 0, 0);

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


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogProp::_pagesOnResize(unsigned tab_id)
{
  if(this->_pageDial.size()) {

    LONG pos[4];

    // get TabControl local coordinates
    GetWindowRect(GetDlgItem(this->_hwnd, tab_id), (LPRECT)&pos);
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
    for(unsigned i = 0; i < this->_pageDial.size(); ++i) {
      SetWindowPos(this->_pageDial[i]->hwnd(), 0, pos[0], pos[1], pos[2], pos[3], SWP_NOZORDER|SWP_NOACTIVATE);
    }
  }
}
