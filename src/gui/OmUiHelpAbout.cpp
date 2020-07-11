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
#include "gui/OmUiHelpAbout.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiHelpAbout::OmUiHelpAbout(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiHelpAbout::~OmUiHelpAbout()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiHelpAbout::id() const
{
  return IDD_HELP_ABOUT;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpAbout::_onShow()
{
  HFONT hFont = CreateFont(14,0,0,0,400,false,false,false,1,0,0,5,0,"Consolas");
  SendMessage(GetDlgItem(this->_hwnd, IDC_EC_ENT01), WM_SETFONT, (WPARAM)hFont, 1);

  wchar_t wcbuf[OMM_MAX_PATH];
  swprintf(wcbuf, OMM_MAX_PATH,
      L"%ls (%ls)\nVersion %d.%d.%d (%ls) - %ls\nBy %ls\n\n"
      "This software is distributed under terms and conditions of the "
      "GNU GPL v3.0 license:",
      OMM_APP_NAME, OMM_APP_SHORT_NAME, OMM_APP_MAJ, OMM_APP_MIN, OMM_APP_REV,
      OMM_APP_ARCH, OMM_APP_DATE, OMM_APP_AUTHOR);

  SetDlgItemTextW(this->_hwnd, IDC_SC_TEXT1, wcbuf);

  string gpl = Om_loadPlainText(L"LICENSE.TXT");
  SetDlgItemText(this->_hwnd, IDC_EC_ENT01, gpl.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpAbout::_onResize()
{
  this->_setControlPos(IDC_SC_TEXT1, 5, 5, this->width()-10, 40);
  this->_setControlPos(IDC_EC_ENT01, 5, 55, this->width()-10, this->height()-80);
  this->_setControlPos(IDC_BC_CLOSE, (0.5f*this->width())-25, this->height()-20, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpAbout::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpAbout::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiHelpAbout::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_CLOSE:
      this->quit();
      break;
    }

  }

  return false;
}
