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

#include "OmBaseApp.h"

#include "OmManager.h"

#include "Ui/OmUiMgr.h"

#include "Util/OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "Ui/OmUiMgrFoot.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrFoot::OmUiMgrFoot(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrFoot::~OmUiMgrFoot()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMgrFoot::id() const
{
  return IDD_MGR_FOOT;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFoot::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFoot::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFoot::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFoot::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFoot::viewPackage(const wstring& name, const OmVersion& vers, const OmImage& snap, const wstring& desc)
{
  this->showItem(IDC_SC_TITLE, true);
  this->setItemText(IDC_SC_TITLE, name + L" " + vers.asString());

  if(desc.size()) {
    this->showItem(IDC_EC_DESC, true);
    this->setItemText(IDC_EC_DESC, desc);
  } else {
    this->showItem(IDC_EC_DESC, false);
  }

  this->showItem(IDC_SB_SNAP, true);

  HBITMAP hBm;

  if(snap.thumbnail()) {
    hBm = snap.thumbnail();
  } else {
    hBm = Om_getResImage(this->_hins, IDB_BLANK);
  }

  // Update the selected picture
  hBm = this->setStImage(IDC_SB_SNAP, hBm);
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFoot::viewReset()
{
  this->showItem(IDC_SB_SNAP, false);
  this->showItem(IDC_EC_DESC, false);
  this->showItem(IDC_SC_TITLE, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFoot::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFoot::_onShow\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFoot::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFoot::_onInit\n";
  #endif

  // retrieve main dialog
  this->_pUiMgr = static_cast<OmUiMgr*>(this->root());

  // set white background to fit tab background
  EnableThemeDialogTexture(this->_hwnd, ETDT_ENABLETAB);

  // Defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(21, 400, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(14, 700, L"Consolas");
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  this->showItem(IDC_SB_SNAP, false);
  this->showItem(IDC_EC_DESC, false);
  this->showItem(IDC_SC_TITLE, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFoot::_onResize()
{
  // Package name/title
  this->_setItemPos(IDC_SC_TITLE, 4, 4, this->cliUnitX()-8, 14);
  // Package snapshot
  this->_setItemPos(IDC_SB_SNAP, 4, 18, 84, 84);
  // Package description
  this->_setItemPos(IDC_EC_DESC, 92, 18, this->cliUnitX()-92, this->cliUnitY()-18);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFoot::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFoot::_onRefresh\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFoot::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFoot::_onQuit\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMgrFoot::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // set cursor depending mouse hover between frames. The cursor switch is
  // 'decided' by main window (UiMgr) but must be done here since when cursor
  // does not hover the parent window, it does not receive WM_SETCURSOR.
  if(uMsg == WM_SETCURSOR) {
    // checks whether cursor is hovering between frames
    if(this->_pUiMgr->divIsHovr()) {
      SetCursor(LoadCursor(0,IDC_SIZENS));
      return 1; //< bypass default process
    }
  }

  // we forward WM_MOUSEMOVE event to parent window (UiMgr) to better catch the
  // mouse cursor when around the frame split.
  if(uMsg == WM_MOUSEMOVE) {
    // get current cursor position, relative to client
    long p[2] = {LOWORD(lParam), HIWORD(lParam)};
    // convert coordinate to relative to parent's client
    ClientToScreen(this->_hwnd, reinterpret_cast<POINT*>(&p));
    ScreenToClient(this->_parent->hwnd(), reinterpret_cast<POINT*>(&p));
    // send message to parent
    SendMessage(this->_parent->hwnd(), WM_MOUSEMOVE, 0, MAKELPARAM(p[0], p[1]));
  }

  if(uMsg == WM_COMMAND) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMgrFoot::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

  }



  return false;
}
