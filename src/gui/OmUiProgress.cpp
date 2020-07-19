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
#include "OmManager.h"
#include "gui/OmUiProgress.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiProgress::OmUiProgress(HINSTANCE hins) : OmDialog(hins),
  _abort(false),
  _hFtDesc(Om_createFont(16, 700, L"Ms Shell Dlg"))
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiProgress::~OmUiProgress()
{
  DeleteObject(this->_hFtDesc);
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
void OmUiProgress::setTitle(const wchar_t* wstr) const
{
  SetWindowTextW(this->_hwnd, wstr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::setDesc(const wchar_t* wstr) const
{
  SetDlgItemTextW(this->_hwnd, IDC_SC_TITLE, wstr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::setDetail(const wchar_t* wstr) const
{
  SetDlgItemTextW(this->_hwnd, IDC_SC_DESC1, wstr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HWND OmUiProgress::getPbHandle() const
{
  return GetDlgItem(this->_hwnd, IDC_PB_PGRES);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HWND OmUiProgress::getDescScHandle() const
{
  return GetDlgItem(this->_hwnd, IDC_SC_TITLE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HWND OmUiProgress::getDetailScHandle() const
{
  return GetDlgItem(this->_hwnd, IDC_SC_DESC1);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::_onInit()
{
  this->msgItem(IDC_SC_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtDesc), true);
  SetWindowTextW(this->_hwnd, L"");
  this->setItemText(IDC_SC_TITLE, L"");
  this->setItemText(IDC_SC_DESC1, L"");

  this->_abort = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::_onResize()
{
  int half_height = static_cast<int>(this->height() * 0.5f);

  this->_setItemPos(IDC_SC_TITLE, 10, 8, this->width()-20, 12);
  this->_setItemPos(IDC_SC_DESC1, 10, half_height-15, this->width()-20, 9);
  this->_setItemPos(IDC_PB_PGRES, 10, half_height, this->width()-20, 11);
  this->_setItemPos(IDC_BC_ABORT, this->width()-70, this->height()-24, 60, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiProgress::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_ABORT:
      this->_abort = true;
      this->enableItem(IDC_BC_ABORT, false);
      break;
    }

  }

  return false;
}
