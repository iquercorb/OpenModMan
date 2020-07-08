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
  _abort(false)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiProgress::~OmUiProgress()
{

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
void OmUiProgress::setCaption(const wchar_t* wstr) const
{
  SetWindowTextW(this->_hwnd, wstr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::setTitle(const wchar_t* wstr) const
{
  SetDlgItemTextW(this->_hwnd, IDC_SC_TITLE, wstr);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void* OmUiProgress::getProgressBar() const
{
  return GetDlgItem(this->_hwnd, IDC_PB_PGRES);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void* OmUiProgress::getStaticTitle() const
{
  return GetDlgItem(this->_hwnd, IDC_SC_TITLE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void* OmUiProgress::getStaticComment() const
{
  return GetDlgItem(this->_hwnd, IDC_SC_DESC1);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::_onShow()
{
  this->_abort = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::_onResize()
{
  SetWindowPos(GetDlgItem(this->_hwnd, IDC_SC_TITLE), nullptr, 20, 10, this->_rect.right-40, 16, SWP_NOZORDER);
  SetWindowPos(GetDlgItem(this->_hwnd, IDC_PB_PGRES), nullptr, 20, 35, this->_rect.right-40, 21, SWP_NOZORDER);
  SetWindowPos(GetDlgItem(this->_hwnd, IDC_SC_DESC1), nullptr, 20, 65, this->_rect.right-40, 16, SWP_NOZORDER);
  SetWindowPos(GetDlgItem(this->_hwnd, IDC_BC_ABORT),  nullptr, this->_rect.right-85, this->_rect.bottom-35, 75, 23, SWP_NOZORDER);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiProgress::_onQuit()
{

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
      EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ABORT), false);
      break;
    }

  }

  return false;
}
