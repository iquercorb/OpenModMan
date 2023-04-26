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

#include "OmManager.h"

#include "OmUiMgr.h"
#include "OmUiMgrMainLib.h"
#include "OmUiMgrMainNet.h"
#include "OmUiMgrMainTst.h" //< tab for test purposes

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiMgrMain.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrMain::OmUiMgrMain(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr), _tabName(), _tabDial()
{
  // create child tab dialogs
  this->_addTab(L"Packages Library", new OmUiMgrMainLib(hins)); // Library Tab
  this->_addTab(L"Network Repositories", new OmUiMgrMainNet(hins)); // Network Tab
  //this->_addTab(L"Work In Progress", new OmUiMgrMainTst(hins)); // Test Tab, for development an debug purpose

  // set the accelerator table for the dialog
  this->setAccel(IDR_ACCEL);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrMain::~OmUiMgrMain()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMgrMain::id() const
{
  return IDD_MGR_MAIN;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMain::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMain::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // disable Tab Control
  this->enableItem(IDC_TC_MAIN, !enable);

  // passes the message to child tab dialog
  for(size_t i = 0; i < this->_tabDial.size(); ++i) {
    if(this->_tabDial[i]->visible()) {
      // TODO: update this if tab child dialog are added
      switch(this->_tabDial[i]->id())
      {
      case IDD_MGR_MAIN_LIB:
        static_cast<OmUiMgrMainLib*>(this->_tabDial[i])->freeze(enable);
        break;
      case IDD_MGR_MAIN_NET:
        static_cast<OmUiMgrMainNet*>(this->_tabDial[i])->freeze(enable);
        break;
      }
      break;
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMain::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMain::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // passes the message to child tab dialog
  for(size_t i = 0; i < this->_tabDial.size(); ++i) {
    if(this->_tabDial[i]->visible()) {
      // TODO: update this if tab child dialog are added
      switch(this->_tabDial[i]->id())
      {
      case IDD_MGR_MAIN_LIB:
        static_cast<OmUiMgrMainLib*>(this->_tabDial[i])->safemode(enable);
        break;
      case IDD_MGR_MAIN_NET:
        static_cast<OmUiMgrMainNet*>(this->_tabDial[i])->safemode(enable);
        break;
      }
      break;
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMain::_addTab(const wstring& title, OmDialog* dialog)
{
  this->addChild(dialog);
  this->_tabDial.push_back(dialog);
  this->_tabName.push_back(title);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMain::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMain::_onInit\n";
  #endif

  // retrieve main dialog
  this->_pUiMgr = static_cast<OmUiMgr*>(this->root());

  // initialize TabControl with pages dialogs
  if(this->_tabDial.size() && this->_hwnd) {

    TCITEMW tcPage;
    tcPage.mask = TCIF_TEXT;

    for(size_t i = 0; i < this->_tabDial.size(); ++i) {

      tcPage.pszText = (LPWSTR)this->_tabName[i].c_str();
      this->msgItem(IDC_TC_MAIN, TCM_INSERTITEMW, i, reinterpret_cast<LPARAM>(&tcPage));

      this->_tabDial[i]->modeless(false);
      // set white background to fit tab background
      EnableThemeDialogTexture(this->_tabDial[i]->hwnd(), ETDT_ENABLETAB);
    }
  }

  // refresh all elements
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMain::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMain::_onShow\n";
  #endif

  // show the first tab page
  if(this->_tabDial.size())
    this->_tabDial[0]->show();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMain::_onResize()
{
  // Main Tab Control
  this->_setItemPos(IDC_TC_MAIN, -1, -1, this->width()+2, this->height()+2, true);

  // Resize page dialogs according IDC_TC_MAIN
  if(this->_tabDial.size()) {

    // apply this for all dialogs
    for(size_t i = 0; i < this->_tabDial.size(); ++i) {
      this->_setChildPos(this->_tabDial[i]->hwnd(), 2, 24, this->cliWidth()-5, this->cliHeight()-27, true);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMain::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMain::_onRefresh\n";
  #endif

  //OmManager* pMgr = static_cast<OmManager*>(this->_data);
  //OmContext* pCtx = pMgr->ctxCur();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMain::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMain::_onQuit\n";
  #endif

  // Exist dialog thread
  //PostQuitMessage(0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMgrMain::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
    ScreenToClient(this->_pUiMgr->hwnd(), reinterpret_cast<POINT*>(&p));
    // send message to parent
    SendMessage(this->_pUiMgr->hwnd(), WM_MOUSEMOVE, 0, MAKELPARAM(p[0], p[1]));
  }

  // UWM_MAIN_ABORT_REQUEST is a custom message sent from Main (parent) Dialog
  // to notify its child tab dialogs they must abort all running threaded jobs
  if(uMsg == UWM_MAIN_ABORT_REQUEST) {
    // send message to all dialog to request abort all their jobs
    for(size_t i = 0; i < this->_tabDial.size(); ++i) {
      this->_tabDial[i]->postMessage(UWM_MAIN_ABORT_REQUEST);
    }
    return false;
  }

  if(uMsg == WM_NOTIFY) {
    // handle TabControl page selection change
    if(this->_tabDial.size()) {
      // check for notify from the specified TabControl
      if(LOWORD(wParam) == IDC_TC_MAIN) {
        // check for a "selection changed" notify
        if(((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
          // get TabControl current selection
          int tc_sel = this->msgItem(IDC_TC_MAIN, TCM_GETCURSEL);
          // change page dialog visibility according selection
          if(tc_sel >= 0) {
            // hide all visible tables
            for(size_t i = 0; i < this->_tabDial.size(); ++i) {
              if(this->_tabDial[i]->visible()) {
                this->_tabDial[i]->hide(); break;
              }
            }
            // show selected tab
            if(!this->_tabDial[tc_sel]->visible())
              this->_tabDial[tc_sel]->show();
          }
        }
      }
    }
  }

  if(uMsg == WM_COMMAND) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMgrMain::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

    // Prevent command/shorcut execution when dialog is not active
    if(!this->active())
      return false;

    //OmManager* pMgr = static_cast<OmManager*>(this->_data);
    //OmContext* pCtx = pMgr->ctxCur();

    // Menus and Shortcuts Messages
    switch(LOWORD(wParam))
    {

    }
  }

  return false;
}
