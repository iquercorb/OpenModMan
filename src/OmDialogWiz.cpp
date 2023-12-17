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
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmDialogWiz.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialogWiz::OmDialogWiz(HINSTANCE hins) : OmDialog(hins),
  _currPage(0)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialogWiz::~OmDialogWiz()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_BC_BACK, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::checkNextAllowed()
{
  // enable 'Next' button according valid fields
  if(this->_currPage >= 0)
    this->enableItem(IDC_BC_NEXT, this->_pageDial[this->_currPage]->validFields());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_addPage(OmDialogWizPage* dialog)
{
  this->addChild(dialog);
  this->_pageDial.push_back(dialog);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_changePage(int32_t index)
{
  if(this->_pageDial.empty())
    return;

  // jump to next page
  this->_currPage = index;

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
      if(i != static_cast<uint32_t>(this->_currPage))
        this->_pageDial[i]->hide();

    this->_pageDial[this->_currPage]->show();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onNext()
{
  if(this->_pageDial.empty())
    return;

  // check for current page valid parameters
  if(this->_pageDial[this->_currPage]->validParams()) {

    // check whether we are on the last page
    if((this->_currPage + 1) < static_cast<int>(this->_pageDial.size())) {

      this->_changePage(this->_currPage + 1);

      this->_onWizNext();

    } else {

      this->_onWizFinish();
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onBack()
{
  if(this->_currPage > 0)
    this->_changePage(this->_currPage - 1);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(IDI_APP,2), Om_getResIcon(IDI_APP,1));

  DWORD ComStyle = WS_CHILD|WS_VISIBLE;

  CreateWindowExW(WS_EX_LEFT, WC_STATICW, L"", ComStyle|SS_BITMAP|SS_CENTERIMAGE|SS_RIGHTJUST,
         10, 10, 110, this->cliHeight()-58,
        this->_hwnd, reinterpret_cast<HMENU>(IDC_SB_IMAGE), this->_hins, nullptr);

  CreateWindowExW(WS_EX_LEFT, WC_STATICW, L"", ComStyle|SS_ETCHEDHORZ,
        4, this->cliHeight()-38, this->cliWidth()-8, 1,
        this->_hwnd, reinterpret_cast<HMENU>(IDC_SC_SEPAR), this->_hins, nullptr);

  CreateWindowExW(WS_EX_LEFT, L"BUTTON", L"< Back", ComStyle|WS_DISABLED|WS_TABSTOP,
        this->cliWidth()-248, this->cliHeight()-30, 78, 23,
        this->_hwnd, reinterpret_cast<HMENU>(IDC_BC_BACK), this->_hins, nullptr);

  CreateWindowExW(WS_EX_LEFT, L"BUTTON", L"Next >", ComStyle|WS_TABSTOP,
        this->cliWidth()-166, this->cliHeight()-30, 78, 23,
        this->_hwnd, reinterpret_cast<HMENU>(IDC_BC_NEXT), this->_hins, nullptr);

  CreateWindowExW(WS_EX_LEFT, L"BUTTON", L"Cancel", ComStyle|BS_DEFPUSHBUTTON|WS_TABSTOP,
        this->cliWidth()-84, this->cliHeight()-30, 78, 23,
        this->_hwnd, reinterpret_cast<HMENU>(IDC_BC_CANCEL), this->_hins, nullptr);

  // Defines fonts for Mod Hub ComboBox
  HFONT hFt = Om_createFont(12, 200, L"Ms Shell Dlg");

  this->msgItem(IDC_BC_BACK, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  this->msgItem(IDC_BC_NEXT, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  this->msgItem(IDC_BC_CANCEL, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  for(size_t i = 0; i < this->_pageDial.size(); ++i)
    this->_pageDial[i]->modeless(false);

  this->_onWizInit();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onShow()
{
  // set first page
  this->_changePage(0);

  this->_onWizShow();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialogWiz::_onResize()
{
  // Lateral Banner
  this->_setItemPos(IDC_SB_IMAGE, 10, 10, 110, this->cliHeight()-58, true);

  if(this->_pageDial.size()) {
    // apply this for all dialogs
    for(size_t i = 0; i < this->_pageDial.size(); ++i)
      this->_setChildPos(this->_pageDial[i]->hwnd(), 130, 10, this->cliWidth()-140, this->cliHeight()-50, true);
  }

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 4, this->cliHeight()-38, this->cliWidth()-8, 1, true);

  // Apply Button
  this->_setItemPos(IDC_BC_BACK, this->cliWidth()-248, this->cliHeight()-30, 78, 23, true);
  // Cancel Button
  this->_setItemPos(IDC_BC_NEXT, this->cliWidth()-166, this->cliHeight()-30, 78, 23, true);
  // Close Button
  this->_setItemPos(IDC_BC_CANCEL, this->cliWidth()-84, this->cliHeight()-30, 78, 23, true);
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

    switch(LOWORD(wParam))
    {
    case IDC_BC_NEXT:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_onNext();
      break;

    case IDC_BC_BACK:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_onBack();
      break;

    case IDC_BC_CANCEL:
      if(HIWORD(wParam) == BN_CLICKED)
        this->quit();
      break;
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
  return false;
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
  OM_UNUSED(uMsg); OM_UNUSED(wParam);  OM_UNUSED(lParam);

  return false;
}
