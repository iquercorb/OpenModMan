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

#include "OmUtilWin.h"
#include "OmUtilDlg.h"

#include "OmModMan.h"

#include "OmUiMan.h"
#include "OmUiManMainLib.h"
#include "OmUiManMainNet.h"
#include "OmUiAddChn.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiManMain.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManMain::OmUiManMain(HINSTANCE hins) : OmDialog(hins),
  _UiMan(nullptr)
{
  // create child tab dialogs
  this->_tab_add_dialog(L"Local Library", new OmUiManMainLib(hins)); // Library Tab
  this->_tab_add_dialog(L"Network Library", new OmUiManMainNet(hins)); // Network Tab
  //this->_tab_add_dialog(L"Work In Progress", new OmUiManMainTst(hins)); // Test Tab, for development an debug purpose

  // set the accelerator table for the dialog
  this->setAccel(IDR_ACCEL);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManMain::~OmUiManMain()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiManMain::id() const
{
  return IDD_MGR_MAIN;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMain::_tab_add_dialog(const OmWString& title, OmDialog* dialog)
{
  this->addChild(dialog);
  this->_tab_dialog.push_back(dialog);
  this->_tab_name.push_back(title);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMain::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMain::_onInit\n";
  #endif

  // retrieve main dialog
  this->_UiMan = static_cast<OmUiMan*>(this->root());

  // initialize TabControl with pages dialogs
  if(this->_tab_dialog.size() && this->_hwnd) {

    TCITEMW tcPage;
    tcPage.mask = TCIF_TEXT;

    for(size_t i = 0; i < this->_tab_dialog.size(); ++i) {

      tcPage.pszText = (LPWSTR)this->_tab_name[i].c_str();
      this->msgItem(IDC_TC_MAIN, TCM_INSERTITEMW, i, reinterpret_cast<LPARAM>(&tcPage));

      this->_tab_dialog[i]->modeless(false);
      // set white background to fit tab background
      EnableThemeDialogTexture(this->_tab_dialog[i]->hwnd(), ETDT_ENABLETAB);

      // force initialization
      this->_tab_dialog[i]->show();
      this->_tab_dialog[i]->hide();
    }
  }

  // refresh all elements
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMain::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMain::_onShow\n";
  #endif

  // show the first tab page
  if(this->_tab_dialog.size())
    this->_tab_dialog[0]->show();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMain::_onResize()
{
  // Main Tab Control
  this->_setItemPos(IDC_TC_MAIN, -1, -1, this->width()+2, this->height()+2, true);

  // Resize page dialogs according IDC_TC_MAIN
  if(this->_tab_dialog.size()) {

    // apply this for all dialogs
    for(size_t i = 0; i < this->_tab_dialog.size(); ++i) {
      this->_setChildPos(this->_tab_dialog[i]->hwnd(), 2, 24, this->cliWidth()-5, this->cliHeight()-27, true);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMain::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMain::_onRefresh\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMain::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMain::_onQuit\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiManMain::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // set cursor depending mouse hover between frames. The cursor switch is
  // 'decided' by main window (UiMgr) but must be done here since when cursor
  // does not hover the parent window, it does not receive WM_SETCURSOR.
  if(uMsg == WM_SETCURSOR) {
    // checks whether cursor is hovering between frames
    if(this->_UiMan->cursorResizeHor()) {
      SetCursor(LoadCursor(0,IDC_SIZENS));
      return 1; //< bypass default process
    }
    if(this->_UiMan->cursorResizeVer()) {
      SetCursor(LoadCursor(0,IDC_SIZEWE));
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
    ScreenToClient(this->_UiMan->hwnd(), reinterpret_cast<POINT*>(&p));
    // send message to parent
    SendMessage(this->_UiMan->hwnd(), WM_MOUSEMOVE, 0, MAKELPARAM(p[0], p[1]));
  }

  // UWM_MAIN_ABORT_REQUEST is a custom message sent from Main (parent) Dialog
  // to notify its child tab dialogs they must abort all running threaded jobs
  if(uMsg == UWM_MAIN_ABORT_REQUEST) {
    // send message to all dialog to request abort all their jobs
    for(size_t i = 0; i < this->_tab_dialog.size(); ++i) {
      this->_tab_dialog[i]->postMessage(UWM_MAIN_ABORT_REQUEST);
    }
    return false;
  }

  if(uMsg == WM_NOTIFY) {
    // handle TabControl page selection change
    if(this->_tab_dialog.size()) {
      // check for notify from the specified TabControl
      if(LOWORD(wParam) == IDC_TC_MAIN) {
        // check for a "selection changed" notify
        if(((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
          // get TabControl current selection
          int tc_sel = this->msgItem(IDC_TC_MAIN, TCM_GETCURSEL);
          // change page dialog visibility according selection
          if(tc_sel >= 0) {
            // hide all visible tables
            for(size_t i = 0; i < this->_tab_dialog.size(); ++i) {
              if(this->_tab_dialog[i]->visible()) {
                this->_tab_dialog[i]->hide(); break;
              }
            }
            // show selected tab
            if(!this->_tab_dialog[tc_sel]->visible())
              this->_tab_dialog[tc_sel]->show();
          }
        }
      }
    }
  }

  if(uMsg == WM_COMMAND) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiManMain::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

    // Prevent command/shorcut execution when dialog is not active
    if(!this->active())
      return false;

    // Menus and Shortcuts Messages
    switch(LOWORD(wParam))
    {

    }
  }

  return false;
}
