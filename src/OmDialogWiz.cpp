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
#include "OmBase.h"

#include "OmBaseUi.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmDialogWiz.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialogWiz::OmDialogWiz(HINSTANCE hins) : OmDialog(hins),
  _pageDial(), _currPage(0),
  _hBmSplash(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_WIZARD),IMAGE_BITMAP,0,0,0)))
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialogWiz::~OmDialogWiz()
{
  DeleteObject(this->_hBmSplash);
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::setNextAllowed(bool allow)
{
  this->enableItem(IDC_BC_NEXT, allow);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_addPage(OmDialog* dialog)
{
  this->addChild(dialog);
  this->_pageDial.push_back(dialog);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onInit()
{
  // set splash image
  this->setStImage(IDC_SB_IMAGE, this->_hBmSplash);

  this->_currPage = 0;

  if(this->_pageDial.size() && this->_hwnd) {

    for(size_t i = 0; i < this->_pageDial.size(); ++i)
      this->_pageDial[i]->modeless(false);

    this->_pageDial[0]->show();
  }

  // change next button text if we are at the last page
  if((this->_currPage + 1) < static_cast<int>(this->_pageDial.size())) {
    this->setItemText(IDC_BC_NEXT, L"Next >");
  } else {
    this->setItemText(IDC_BC_NEXT, L"Finish");
  }

  // disable back button if we are at the first page
  this->enableItem(IDC_BC_BACK, (this->_currPage > 0));

  this->_onWizInit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onShow()
{
  this->_onWizShow();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onResize()
{
  // Lateral Banner
  this->_setItemPos(IDC_SB_IMAGE, 5, 5, 68, 189);
  SetWindowPos(this->getItem(IDC_SB_IMAGE), 0, 0, 0, 110, 310, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->cliUnitY()-25, this->cliUnitX()-10, 1);
  // Back, Next and Cancel buttons
  this->_setItemPos(IDC_BC_BACK, this->cliUnitX()-161, this->cliUnitY()-19, 50, 14);
  this->_setItemPos(IDC_BC_NEXT, this->cliUnitX()-110, this->cliUnitY()-19, 50, 14);
  this->_setItemPos(IDC_BC_CANCEL, this->cliUnitX()-54, this->cliUnitY()-19, 50, 14);

  // force buttons to redraw to prevent artifacts
  InvalidateRect(this->getItem(IDC_BC_BACK), nullptr, true);
  InvalidateRect(this->getItem(IDC_BC_NEXT), nullptr, true);
  InvalidateRect(this->getItem(IDC_BC_CANCEL), nullptr, true);

  if(this->_pageDial.size()) {

    LONG pos[4];

    // convert into base unit and adjust to keep inside the TabControl
    pos[0] = 80;
    pos[1] = 0;
    pos[2] = this->cliUnitX() - 80;
    pos[3] = this->cliUnitY() - 26;

    // Map in pixels
    MapDialogRect(this->_hwnd, reinterpret_cast<LPRECT>(&pos));

    // apply this for all dialogs
    for(size_t i = 0; i < this->_pageDial.size(); ++i)
      this->_setChildPos(this->_pageDial[i]->hwnd(), pos[0], pos[1], pos[2], pos[3], true);
  }

  this->_onWizResize();

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmDialogWiz::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool page_changed = false;

    switch(LOWORD(wParam))
    {
    case IDC_BC_NEXT:
      if((this->_currPage + 1) < static_cast<int>(this->_pageDial.size())) {
        if(this->_onWizNext()) {
          // jump to next page
          ++ this->_currPage;
          page_changed = true;
        }
      } else {
        this->_onWizFinish();
      }

      break;

    case IDC_BC_BACK:
      if(this->_currPage > 0) {
        -- this->_currPage;
        page_changed = true;
      }
      break; // case IDC_BC_OK:

    case IDC_BC_CANCEL:
      this->quit();
      break; // case IDC_BC_CANCEL:
    }

    if(page_changed) {

      // change next button text if we are at the last page
      if((this->_currPage + 1) < static_cast<int>(this->_pageDial.size())) {
        this->setItemText(IDC_BC_NEXT, L"Next >");
      } else {
        this->setItemText(IDC_BC_NEXT, L"Finish");
      }

      // disable back button if we are at the first page
      this->enableItem(IDC_BC_BACK, (this->_currPage > 0));

      // change page dialog visibility according selection
      if(this->_currPage >= 0) {

        for(size_t i = 0; i < this->_pageDial.size(); ++i)
          this->_pageDial[i]->hide();

        this->_pageDial[this->_currPage]->show();
      }
    }

  }

  return this->_onWizMsg(uMsg, wParam, lParam);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onWizInit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onWizShow()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onWizResize()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmDialogWiz::_onWizNext()
{
  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onWizFinish()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmDialogWiz::_onWizMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return false;
}
