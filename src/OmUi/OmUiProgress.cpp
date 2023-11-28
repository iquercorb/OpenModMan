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

#include "OmBaseWin.h"
#include <ShlObj.h>

#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiProgress.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiProgress::OmUiProgress(HINSTANCE hins) : OmDialog(hins),
  _abort(false)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiProgress::~OmUiProgress()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_SC_HEAD, WM_GETFONT));
  DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiProgress::id() const
{
  return IDD_PROGRESS;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::setPbRange(uint16_t min, uint16_t max) const
{
  this->msgItem(IDC_PB_COM, PBM_SETRANGE, 0, MAKELPARAM(min, max));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::setPbPos(uint16_t pos) const
{
  this->msgItem(IDC_PB_COM, PBM_SETPOS, pos);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::setScHeadText(const OmWString& text) const
{
  this->setItemText(IDC_SC_HEAD, text);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::setScItemText(const OmWString& text) const
{
  this->setItemText(IDC_SC_ITEM, text);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HWND OmUiProgress::hPb() const
{
  return GetDlgItem(this->_hwnd, IDC_PB_COM);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HWND OmUiProgress::hScHead() const
{
  return GetDlgItem(this->_hwnd, IDC_SC_HEAD);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HWND OmUiProgress::hScItem() const
{
  return GetDlgItem(this->_hwnd, IDC_SC_ITEM);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::abortSet(bool enable)
{
  this->_abort = enable;

  this->enableItem(IDC_BC_ABORT, !enable);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::_bc_abort_hit()
{
  this->_abort = true;

  this->enableItem(IDC_BC_ABORT, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::_onInit()
{
  // set dialog title icon
  this->setIcon(Om_getResIcon(this->_hins,IDI_APP,2),Om_getResIcon(this->_hins,IDI_APP,1));

  // set header text font
  HFONT hFt = Om_createFont(16, 600, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_HEAD, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  SetWindowTextW(this->_hwnd, L"");
  this->setItemText(IDC_SC_HEAD, L"");
  this->setItemText(IDC_SC_ITEM, L"");

  this->_abort = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::_onResize()
{
  int half_h = static_cast<int>(this->cliUnitY() * 0.5f);

  // Header text label
  this->_setItemPos(IDC_SC_HEAD, 5, 5, this->cliUnitX()-20, 12);
  // Detail or item text label
  this->_setItemPos(IDC_SC_ITEM, 5, half_h-15, this->cliUnitX()-10, 9);
  // Progress bar
  this->_setItemPos(IDC_PB_COM, 5, half_h, this->cliUnitX()-10, 11);
  // Abort button
  this->_setItemPos(IDC_BC_ABORT, this->cliUnitX()-55, this->cliUnitY()-20, 50, 14);

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiProgress::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_ABORT:
      this->_bc_abort_hit();
      break;
    }

  }

  return false;
}
