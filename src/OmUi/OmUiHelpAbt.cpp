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

#include "OmBaseApp.h"

#include "OmModMan.h"

#include "OmUtilWin.h"
#include "OmUtilFs.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiHelpAbt.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiHelpAbt::OmUiHelpAbt(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiHelpAbt::~OmUiHelpAbt()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_RESUL, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiHelpAbt::id() const
{
  return IDD_HELP_ABT;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpAbt::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins,IDI_APP,2),Om_getResIcon(this->_hins,IDI_APP,1));

  OmWString about = OM_APP_NAME;
  about.append(L" - version ");
  about.append(std::to_wstring(OM_APP_MAJ));
  about.push_back(L'.');
  about.append(std::to_wstring(OM_APP_MIN));
  #if (OM_APP_REV > 0)
  about.push_back(L'.');
  about.append(std::to_wstring(OM_APP_REV));
  #endif
  about.append(L" ( "); about.append(OM_APP_ARCH); about.append(L" )\n\n");
  about.append(OM_APP_DATE); about.append(L"  by ");
  about.append(OM_APP_AUTHOR);
  this->setItemText(IDC_SC_INTRO, about);

  OmWString home_url = L"<a href=\"";
  home_url.append(OM_APP_URL); home_url.append(L"\">");
  home_url.append(OM_APP_URL); home_url.append(L"</a>");
  this->setItemText(IDC_LM_LNK01, home_url);
/*
  OmWString repo_url = L"<a href=\"";
  repo_url.append(OM_APP_GIT); repo_url.append(L"\">");
  repo_url.append(OM_APP_GIT); repo_url.append(L"</a>");
  this->setItemText(IDC_LM_LNK02, repo_url);
*/
  HFONT hFt = Om_createFont(14, 400, L"Consolas");
  this->msgItem(IDC_EC_RESUL, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  OmCString txt;
  if(Om_loadPlainText(&txt, L"CREDITS")) {
    SetDlgItemText(this->_hwnd, IDC_EC_RESUL, txt.c_str());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpAbt::_onResize()
{
  unsigned half_width = static_cast<unsigned>(this->cliUnitX() * 0.5f);

  this->_setItemPos(IDC_SC_INTRO, 5, 5, this->cliUnitX()-10, 25);

  this->_setItemPos(IDC_SC_LBL01, half_width - 100, 35, 60, 9);
  this->_setItemPos(IDC_LM_LNK01, half_width - 35, 35, 200, 9);

  this->_setItemPos(IDC_SC_LBL02, half_width - 100, 50, 60, 9);
  this->_setItemPos(IDC_LM_LNK02, half_width - 35, 50, 200, 9);

  this->_setItemPos(IDC_EC_RESUL, 5, 65, this->cliUnitX()-10, this->cliUnitY()-100);

  // force buttons to redraw
  InvalidateRect(this->getItem(IDC_SC_INTRO), nullptr, true);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->cliUnitY()-25, this->cliUnitX()-10, 1);
  // Close button
  this->_setItemPos(IDC_BC_CLOSE, this->cliUnitX()-54, this->cliUnitY()-19, 50, 14);

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiHelpAbt::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {

    NMHDR* pNmhdr = reinterpret_cast<NMHDR*>(lParam);

    switch(LOWORD(wParam))
    {
    case IDC_LM_LNK01:
    case IDC_LM_LNK02:
      if(pNmhdr->code == NM_CLICK) {
        NMLINK* pNmlink = reinterpret_cast<NMLINK*>(lParam);
        ShellExecuteW(NULL, L"open", pNmlink->item.szUrl, nullptr, nullptr, SW_SHOW);
      }
      break;
    }
  }

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
