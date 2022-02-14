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
  #include <UxTheme.h>

#include "OmPackage.h"
#include "OmRemote.h"

#include "OmUiMgr.h"

#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiMgrFootSrc.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrFootSrc::OmUiMgrFootSrc(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr)
{

}

OmUiMgrFootSrc::~OmUiMgrFootSrc()
{
  /*
  HFONT hFt;
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_SC_NAME, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
  */
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMgrFootSrc::id() const
{
  return IDD_MGR_FOOT_SRC;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootSrc::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootSrc::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootSrc::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootSrc::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootSrc::setDetails(OmPackage* pPkg)
{
  if(pPkg) {
    this->showItem(IDC_SC_NAME, true);
    this->setItemText(IDC_SC_NAME, pPkg->name() + L" " + pPkg->version().asString());

    wstring src_lst;

    for(size_t i = 0; i < pPkg->srcItemCount(); ++i) {
      src_lst += pPkg->srcItemGet(i).path;
      src_lst += L"\r\n";
    }

    this->setItemText(IDC_EC_LISTE, src_lst);
    this->showItem(IDC_EC_LISTE, true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootSrc::setDetails(OmRemote* pRmt)
{
  if(pRmt) {
    this->showItem(IDC_SC_NAME, true);
    this->setItemText(IDC_SC_NAME, pRmt->name() + L" " + pRmt->version().asString());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootSrc::clearDetails()
{
  this->showItem(IDC_SC_NAME, false);
  this->showItem(IDC_EC_LISTE, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootSrc::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootSrc::_onInit\n";
  #endif

  // retrieve main dialog
  this->_pUiMgr = static_cast<OmUiMgr*>(this->root());

  // set white background to fit tab background
  EnableThemeDialogTexture(this->_hwnd, ETDT_ENABLETAB);

  // Defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(21, 400, L"Ms Dlg Font");
  this->msgItem(IDC_SC_NAME, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(16, 400, L"Consolas");
  this->msgItem(IDC_EC_LISTE, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  this->showItem(IDC_SC_NAME, true);
  this->showItem(IDC_EC_LISTE, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootSrc::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootSrc::_onShow\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootSrc::_onResize()
{
  // Package name/title
  this->_setItemPos(IDC_SC_NAME, 4, 2, this->cliUnitX()-58, 14);
  // Package source files
  this->_setItemPos(IDC_EC_LISTE, 4, 18, this->cliUnitX()-8, this->cliUnitY()-18);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootSrc::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootSrc::_onRefresh\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootSrc::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootSrc::_onQuit\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMgrFootSrc::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{

  if(uMsg == WM_COMMAND) {

    #ifdef DEBUG
    //std::cout << "DEBUG => OmUiMgrFootDsc::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

  }

  return false;
}


