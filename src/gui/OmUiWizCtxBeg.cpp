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

#include "gui/OmUiWizCtxBeg.h"
#include "gui/res/resource.h"
#include "gui/OmUiWizCtx.h"
#include "OmManager.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizCtxBeg::OmUiWizCtxBeg(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizCtxBeg::~OmUiWizCtxBeg()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizCtxBeg::id() const
{
  return IDD_WIZ_CTX_BEG;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxBeg::_onShow()
{

  // set "help" icon
  //HICON hIcon = (HICON)Om_loadShellIcon(SIID_HELP, true);
  //SendMessage(GetDlgItem(this->_hwnd, IDC_SB_WHELP), STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
  //InvalidateRect(GetDlgItem(this->_hwnd, IDC_SB_WHELP), nullptr, true);

  // enable the Wizard Next button
  reinterpret_cast<OmDialogWiz*>(this->_parent)->setNextAllowed(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxBeg::_onResize()
{
  // Intro paragraph
  this->_setControlPos(IDC_SC_TEXT1, 10, 5, 190, 60);
  // Help Icon
  //this->_setControlPos(IDC_SB_WHELP, 10, 80, 21, 20);
  // Help paragraph
  //this->_setControlPos(IDC_SC_TEXT2, 10, 110, 190, 60);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxBeg::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtxBeg::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizCtxBeg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return false;
}
