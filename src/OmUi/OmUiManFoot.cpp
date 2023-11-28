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

#include "OmUiMan.h"
#include "OmUiManFootOvw.h"
#include "OmUiManFootDet.h"
#include "OmUiManFootGal.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiManFoot.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManFoot::OmUiManFoot(HINSTANCE hins) : OmDialog(hins),
  _UiMan(nullptr)
{
  // create child tab dialogs
  this->_tab_add_dialog(L"Mod Overview", new OmUiManFootOvw(hins));
  this->_tab_add_dialog(L"Mod Properties", new OmUiManFootDet(hins));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManFoot::~OmUiManFoot()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiManFoot::id() const
{
  return IDD_MGR_FOOT;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFoot::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFoot::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // disable Tab Control
  //this->enableItem(IDC_TC_MAIN, !enable);

  // passes the message to child tab dialog
  for(size_t i = 0; i < this->_tab_dialog.size(); ++i) {

    // TODO: update this if tab child dialog are added

    switch(this->_tab_dialog[i]->id())
    {
    case IDD_MGR_FOOT_OVW:
      static_cast<OmUiManFootOvw*>(this->_tab_dialog[i])->freeze(enable);
      break;
    case IDD_MGR_FOOT_DET:
      static_cast<OmUiManFootDet*>(this->_tab_dialog[i])->freeze(enable);
      break;
    }

  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFoot::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFoot::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // passes the message to child tab dialog
  for(size_t i = 0; i < this->_tab_dialog.size(); ++i) {

    // TODO: update this if tab child dialog are added

    switch(this->_tab_dialog[i]->id())
    {
    case IDD_MGR_FOOT_OVW:
      static_cast<OmUiManFootOvw*>(this->_tab_dialog[i])->safemode(enable);
      break;
    case IDD_MGR_FOOT_DET:
      static_cast<OmUiManFootDet*>(this->_tab_dialog[i])->safemode(enable);
      break;
    }

  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFoot::selectItem(OmModPack* ModPack)
{
  static_cast<OmUiManFootOvw*>(this->_tab_get_dialog(IDD_MGR_FOOT_OVW))->setPreview(ModPack);
  static_cast<OmUiManFootDet*>(this->_tab_get_dialog(IDD_MGR_FOOT_DET))->setDetails(ModPack);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFoot::selectItem(OmNetPack* NetPack)
{
  static_cast<OmUiManFootOvw*>(this->_tab_get_dialog(IDD_MGR_FOOT_OVW))->setPreview(NetPack);
  static_cast<OmUiManFootDet*>(this->_tab_get_dialog(IDD_MGR_FOOT_DET))->setDetails(NetPack);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFoot::clearItem()
{
  static_cast<OmUiManFootOvw*>(this->_tab_get_dialog(IDD_MGR_FOOT_OVW))->clearPreview();
  static_cast<OmUiManFootDet*>(this->_tab_get_dialog(IDD_MGR_FOOT_DET))->clearDetails();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFoot::_tab_add_dialog(const OmWString& title, OmDialog* dialog)
{
  this->addChild(dialog);
  this->_tab_dialog.push_back(dialog);
  this->_tab_name.push_back(title);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialog* OmUiManFoot::_tab_get_dialog(int32_t id)
{
  // passes the message to child tab dialog
  for(size_t i = 0; i < this->_tab_dialog.size(); ++i) {
    if(this->_tab_dialog[i]->id() == id)
      return this->_tab_dialog[i];
  }

  return nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFoot::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFoot::_onInit\n";
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

      // show dialog to force init
      this->_tab_dialog[i]->modeless(true);
      // set white background to fit tab background
      EnableThemeDialogTexture(this->_tab_dialog[i]->hwnd(), ETDT_ENABLETAB);
      // hide dialog
      this->_tab_dialog[i]->hide();
    }
  }

  // refresh all elements
  this->_onRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFoot::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFoot::_onShow\n";
  #endif

  // show the first tab page
  if(this->_tab_dialog.size())
    this->_tab_dialog[0]->show();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFoot::_onResize()
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
void OmUiManFoot::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFoot::_onRefresh\n";
  #endif

  //OmModMan* pMgr = static_cast<OmModMan*>(this->_data);
  //OmModHub* pModHub = pMgr->activeHub();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFoot::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFoot::_onQuit\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiManFoot::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // set cursor depending mouse hover between frames. The cursor switch is
  // 'decided' by main window (UiMgr) but must be done here since when cursor
  // does not hover the parent window, it does not receive WM_SETCURSOR.
  if(uMsg == WM_SETCURSOR) {
    // checks whether cursor is hovering between frames
    if(this->_UiMan->splitterCursorHover()) {
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
    //std::cout << "DEBUG => OmUiManFoot::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif
  }

  return false;
}
