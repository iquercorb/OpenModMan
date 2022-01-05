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
#include "gui/OmUiMain.h"
#include "gui/OmUiMainLib.h"
#include "gui/OmUiMainNet.h"
#include "gui/OmUiMainTst.h"
#include "gui/OmUiPropCtx.h"
#include "gui/OmUiPropLoc.h"
#include "gui/OmUiPropMan.h"
#include "gui/OmUiPropBat.h"
#include "gui/OmUiHelpLog.h"
#include "gui/OmUiHelpAbt.h"
#include "gui/OmUiWizCtx.h"
#include "gui/OmUiAddBat.h"
#include "gui/OmUiAddRep.h"
#include "gui/OmUiAddLoc.h"
#include "gui/OmUiToolPkg.h"
#include "gui/OmUiToolRep.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMain::OmUiMain(HINSTANCE hins) : OmDialog(hins),
  _pageName(),
  _pageDial(),
  _freeze_mode(false),
  _freeze_quit(false)
{
  // create child tab dialogs
  this->_addPage(L"Library", new OmUiMainLib(hins)); // Library Tab
  this->_addPage(L"Network", new OmUiMainNet(hins)); // Network Tab
  //this->_addPage(L"Test", new OmUiMainTst(hins)); // Test Tab, for development an debug purpose

  // add children dialogs
  this->addChild(new OmUiPropMan(hins));    //< Dialog for Manager Options
  this->addChild(new OmUiPropCtx(hins));    //< Dialog for Context Properties
  this->addChild(new OmUiPropLoc(hins));    //< Dialog for Location Properties
  this->addChild(new OmUiPropBat(hins));    //< Dialog for Batch Properties
  this->addChild(new OmUiHelpLog(hins));    //< Dialog for Help Debug log
  this->addChild(new OmUiHelpAbt(hins));  //< Dialog for Help About
  this->addChild(new OmUiWizCtx(hins));     //< Dialog for New Context Wizard
  this->addChild(new OmUiToolPkg(hins));     //< Dialog for New Package
  this->addChild(new OmUiAddBat(hins));     //< Dialog for New Batch
  this->addChild(new OmUiAddLoc(hins));     //< Dialog for Adding Location
  this->addChild(new OmUiAddRep(hins));     //< Dialog for Add Repository
  this->addChild(new OmUiToolRep(hins));     //< Dialog for Repository Editor

  // set the accelerator table for the dialog
  this->setAccel(IDR_ACCEL);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMain::~OmUiMain()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMain::id() const
{
  return IDD_MAIN;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMain::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  this->_freeze_mode = enable;

  // disable Context ComboBox
  this->enableItem(IDC_CB_CTX, !enable);
  // disable Tab Control
  this->enableItem(IDC_TC_MAIN, !enable);

  // disable menus
  int state = enable ? MF_GRAYED : MF_ENABLED;
  this->setPopupItem(this->_menu, 0, state); //< File menu
  this->setPopupItem(this->_menu, 1, state); //< Edit menu
  this->setPopupItem(this->_menu, 2, state); //< Tools menu

  // force menu bar to redraw so enabled/grayed state
  // is properly visually updated
  DrawMenuBar(this->_hwnd);

  // passes the message to child tab dialog
  for(size_t i = 0; i < this->_pageDial.size(); ++i) {
    if(this->_pageDial[i]->visible()) {
      // TODO: update this if tab child dialog are added
      switch(this->_pageDial[i]->id())
      {
      case IDD_MAIN_LIB:
        static_cast<OmUiMainLib*>(this->_pageDial[i])->freeze(enable);
        break;
      case IDD_MAIN_NET:
        static_cast<OmUiMainNet*>(this->_pageDial[i])->freeze(enable);
        break;
      }
      break;
    }
  }

  // check whether freeze was aborted, this happen if user
  // requested to close the dialog while in freeze mode.
  if(this->_freeze_quit && !enable) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMain::freeze (quit)\n";
    #endif

    this->quit();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMain::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // passes the message to child tab dialog
  for(size_t i = 0; i < this->_pageDial.size(); ++i) {
    if(this->_pageDial[i]->visible()) {
      // TODO: update this if tab child dialog are added
      switch(this->_pageDial[i]->id())
      {
      case IDD_MAIN_LIB:
        static_cast<OmUiMainLib*>(this->_pageDial[i])->safemode(enable);
        break;
      case IDD_MAIN_NET:
        static_cast<OmUiMainNet*>(this->_pageDial[i])->safemode(enable);
        break;
      }
      break;
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::ctxOpen(const wstring& path)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // Try to open Context
  if(pMgr->ctxOpen(path)) {

    // refresh
    this->refresh();

  } else {
    Om_dialogBoxErr(this->_hwnd, L"Unable to load Context", pMgr->lastError());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::ctxClose()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMain::ctxClose\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // close the current context
  pMgr->ctxClose();

  // refresh
  this->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::ctxSel(int id)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMain::ctxSel " << id << "\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // select the requested Context
  pMgr->ctxSel(id);

  // refresh all
  this->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_addPage(const wstring& title, OmDialog* dialog)
{
  this->addChild(dialog);
  this->_pageDial.push_back(dialog);
  this->_pageName.push_back(title);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_buildCaption()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // update dialog window title
  wstring caption;
  if(pCtx) caption = pCtx->title() + L" - ";

  this->setCaption(caption + OMM_APP_NAME);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_buildSbCtx()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // update the Context icon
  HICON hIc = nullptr;

  // get context icon
  if(pCtx)
    if(pCtx->icon())
      hIc = pCtx->icon();

  // Get default icon
  if(!hIc)
    hIc = Om_getShellIcon(SIID_APPLICATION, true);

  this->msgItem(IDC_SB_ICON, STM_SETICON, reinterpret_cast<WPARAM>(hIc));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_buildMnRct()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMain::_buildMnRct\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // handle to "File > Recent files" pop-up
  HMENU hMenu = this->getPopupItem(static_cast<unsigned>(0), 3); //< "File > Recent files" pop-up

  // remove all entry from "File > Recent files >" pop-up except the two last
  // ones which are the separator and and the "Clear history" menu-item
  unsigned n = GetMenuItemCount(hMenu) - 2;
  for(unsigned i = 0; i < n; ++i)
    RemoveMenu(hMenu, 0, MF_BYPOSITION);

  // get recent files path list from manager
  vector<wstring> path;
  pMgr->loadRecentFiles(path);

  // add the recent file path or disable popup
  if(path.size()) {

    wstring item_str;

    for(size_t i = 0; i < path.size(); ++i) {

      item_str = to_wstring(path.size() - i);
      item_str.append(L" "); item_str.append(path[i]);

      InsertMenuW(hMenu, 0, MF_BYPOSITION|MF_STRING, IDM_FILE_RECENT_PATH + i, item_str.c_str());
    }
    // enable the "File > Recent Files" popup
    this->setPopupItem(static_cast<int>(0), 3, MF_ENABLED);
  } else {
    // disable the "File > Recent Files" popup
    this->setPopupItem(static_cast<int>(0), 3, MF_GRAYED);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_buildCbCtx()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMain::_buildCbCtx\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // empty the Combo-Box
  this->msgItem(IDC_CB_CTX, CB_RESETCONTENT);

  // add Context(s) to Combo-Box
  if(pMgr->ctxCount()) {

    wstring item_str;

    for(unsigned i = 0; i < pMgr->ctxCount(); ++i) {

      item_str = pMgr->ctxGet(i)->title();
      item_str += L" - ";
      item_str += pMgr->ctxGet(i)->home();

      this->msgItem(IDC_CB_CTX, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));
    }

    // select context according current active one
    this->msgItem(IDC_CB_CTX, CB_SETCURSEL, pMgr->ctxCurId());

    // enable the ComboBox control
    this->enableItem(IDC_CB_CTX, true);

  } else {

    // no selection
    this->msgItem(IDC_CB_CTX, CB_SETCURSEL, -1);

    // disable the ComboBox control
    this->enableItem(IDC_CB_CTX, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onCbCtxSel()
{
  int cb_sel = this->msgItem(IDC_CB_CTX, CB_GETCURSEL);

  if(cb_sel >= 0) this->ctxSel(cb_sel);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMain::_onShow\n";
  #endif

  // show the first tab page
  this->_pageDial[0]->show();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMain::_onInit\n";
  #endif

  // set window icon
  this->setIcon(Om_getResIcon(this->_hins, IDB_APP_ICON, 2), Om_getResIcon(this->_hins, IDB_APP_ICON, 1));

  // Defines fonts for Context ComboBox
  HFONT hFt = Om_createFont(21, 200, L"Ms Shell Dlg");
  this->msgItem(IDC_CB_CTX, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  this->_createTooltip(IDC_CB_CTX, L"Select active context");

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  RECT rect = {0,0,0,0};
  // set window to saved position and size
  pMgr->loadWindowRect(rect);

  int x = rect.left;
  int y = rect.top;
  int w = rect.right - rect.left;
  int h = rect.bottom - rect.top;

  if(x >= 0 && y >= 0 && w > 0 && h > 0) {
    SetWindowPos(this->_hwnd, nullptr, x, y, w, h, SWP_NOZORDER);
  }

  // initialize TabControl with pages dialogs
  if(this->_pageDial.size() && this->_hwnd) {

    TCITEMW tcPage;
    tcPage.mask = TCIF_TEXT;

    for(size_t i = 0; i < this->_pageDial.size(); ++i) {

      tcPage.pszText = (LPWSTR)this->_pageName[i].c_str();
      this->msgItem(IDC_TC_MAIN, TCM_INSERTITEMW, i, reinterpret_cast<LPARAM>(&tcPage));

      this->_pageDial[i]->modeless(false);
      // set white background to fit tab background
      EnableThemeDialogTexture(this->_pageDial[i]->hwnd(), ETDT_ENABLETAB);
    }
  }

  // Set default context icon
  HICON hIc = Om_getShellIcon(SIID_APPLICATION, true);
  this->msgItem(IDC_SB_ICON, STM_SETICON, reinterpret_cast<WPARAM>(hIc));

  // refresh all elements
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onResize()
{
  // Context list ComboBox
  this->_setItemPos(IDC_CB_CTX, 4, 4, this->width()-32 , 19);
  // Context Icon
  this->_setItemPos(IDC_SB_ICON, this->width()-24, 3, 19, 19);
  // Main Tab Control
  this->_setItemPos(IDC_TC_MAIN, 4, 25, this->width()-8, this->height()-30);

  // Resize page dialogs according IDC_TC_MAIN
  if(this->_pageDial.size()) {

    LONG pos[4];

    // get TabControl local coordinates
    GetWindowRect(this->getItem(IDC_TC_MAIN), reinterpret_cast<LPRECT>(&pos));
    MapWindowPoints(HWND_DESKTOP, this->_hwnd, reinterpret_cast<LPPOINT>(&pos), 2);

    // convert into base unit and adjust to keep inside the TabControl
    pos[0] = MulDiv(pos[0], 4, this->unitX()) + 1;
    pos[1] = MulDiv(pos[1], 8, this->unitY()) + 15;
    pos[2] = MulDiv(pos[2], 4, this->unitX()) - 3;
    pos[3] = MulDiv(pos[3], 8, this->unitY()) - 3;

    // Map again in pixels
    MapDialogRect(this->_hwnd, reinterpret_cast<LPRECT>(&pos));
    pos[2] -= pos[0]; // width = right - left
    pos[3] -= pos[1]; // height = bottom - top

    // apply this for all dialogs
    for(size_t i = 0; i < this->_pageDial.size(); ++i) {
      SetWindowPos(this->_pageDial[i]->hwnd(), 0, pos[0], pos[1], pos[2], pos[3], SWP_NOZORDER|SWP_NOACTIVATE);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMain::_onRefresh\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // update menus
  int state = pCtx ? MF_ENABLED : MF_GRAYED;
  this->setPopupItem(static_cast<int>(0), 5, state); // File > Close
  this->setPopupItem(static_cast<int>(1), 0, state); // Edit > Context properties...
  this->setPopupItem(static_cast<int>(1), 3, state); // Edit > Add Location...
  if(pCtx) {
    // Edit > Location properties...
    this->setPopupItem(static_cast<int>(1), 2, pMgr->ctxCur()->locCur() ? MF_ENABLED : MF_GRAYED);
  } else {
    this->setPopupItem(static_cast<int>(1), 2, MF_GRAYED); // Edit > Location properties...
    this->setPopupItem(static_cast<int>(1), 5, MF_GRAYED); // Edit > Package []
  }

  // rebuild the Recent Contect menu
  this->_buildMnRct();

  // rebuild the Context list Combo-Box
  this->_buildCbCtx();

  // update window caption
  this->_buildCaption();

  // update Context Icon
  this->_buildSbCtx();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onClose()
{
  // check whether dialog is in freeze mode, in this case we cannot quit
  // right now, we need to carefully abort all running threads first
  if(this->_freeze_mode) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMain::_onClose (freeze_mode)\n";
    #endif

    // quit dialog on next unfreeze
    this->_freeze_quit = true;

    // disable the window to notify user its request to quit is acknowledged
    //EnableWindow(this->_hwnd, false);

    // send message to all dialog to request abort all their jobs
    for(size_t i = 0; i < this->_pageDial.size(); ++i) {
      this->_pageDial[i]->postMessage(UWM_MAIN_ABORT_REQUEST);
    }

  } else {
    this->quit();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMain::_onQuit\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  RECT rec;
  GetWindowRect(this->_hwnd, &rec);
  pMgr->saveWindowRect(rec);

  // Exist dialog thread
  PostQuitMessage(0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMain::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // The WM_COPYDATA is received if another instance was run
  // with arguments, in this case it pass the argument string
  // to initial instance using WM_COPYDATA
  if(uMsg == WM_COPYDATA) {

    COPYDATASTRUCT* pCd = reinterpret_cast<COPYDATASTRUCT*>(lParam);

    if(pCd->dwData == 42) { //< this mean this is argument line from another instance

      if(pCd->cbData > 3) { //< at least more than a nullchar and two quotes

        // convert ANSI string to proper wide string
        wstring path;
        Om_fromAnsiCp(path, reinterpret_cast<char*>(pCd->lpData));

        // check for quotes and removes them
        if(path.back() == L'"' && path.front() == L'"') {
          path.erase(0, 1);  path.pop_back();
        }

        // try to open
        this->ctxOpen(path);
      }
    }
  }

  if(uMsg == WM_NOTIFY) {
    // handle TabControl page selection change
    if(this->_pageDial.size()) {
      // check for notify from the specified TabControl
      if(LOWORD(wParam) == IDC_TC_MAIN) {
        // check for a "selection changed" notify
        if(((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
          // get TabControl current selection
          int tc_sel = this->msgItem(IDC_TC_MAIN, TCM_GETCURSEL);
          // change page dialog visibility according selection
          if(tc_sel >= 0) {
            for(int i = 0; i < static_cast<int>(this->_pageDial.size()); ++i) {
              if(i == tc_sel) {
                this->_pageDial[i]->show();
              } else {
                this->_pageDial[i]->hide();
              }
            }
          }
        }
      }
    }
  }

  if(uMsg == WM_COMMAND) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMain::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

    // Prevent command/shorcut execution when dialog is not active
    if(!this->active())
      return false;

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->ctxCur();

    wstring item_str;

    // handle "File > Recent Files" path click
    if(LOWORD(wParam) >= IDM_FILE_RECENT_PATH) { // recent

      vector<wstring> paths;
      pMgr->loadRecentFiles(paths);

      // subtract Command ID by the base resource ID to get real index
      this->ctxOpen(paths[LOWORD(wParam) - IDM_FILE_RECENT_PATH]);
    }

    // Menus and Shortcuts Messages
    switch(LOWORD(wParam))
    {

    case IDC_CB_CTX: //< Context ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->_onCbCtxSel();
      break;

    // Menu : File []
    case IDM_FILE_NEW_CTX:
      this->childById(IDD_WIZ_CTX)->open(); // New Context Wizard
      break;

    case IDM_FILE_OPEN:
      if(Om_dialogOpenFile(item_str, this->_hwnd, L"Open Context file", OMM_CTX_DEF_FILE_FILER, item_str)) {
        this->ctxOpen(item_str);
      }
      break;

    case IDM_FILE_CLOSE:
      this->ctxClose();
      break;

    case IDM_FILE_CLEAR_HIST:
      pMgr->clearRecentFiles();
      this->_buildMnRct();
      break;

    case IDM_FILE_QUIT:
      this->quit();
      break;

    // Menu : Edit []
    case IDM_EDIT_CTX_PROP: {
      OmUiPropCtx* pUiPropCtx = static_cast<OmUiPropCtx*>(this->childById(IDD_PROP_CTX));
      pUiPropCtx->ctxSet(pCtx);
      pUiPropCtx->open();
      break;
    }

    case IDM_EDIT_LOC_PROP: {
      OmUiPropLoc* pUiPropLoc = static_cast<OmUiPropLoc*>(this->childById(IDD_PROP_LOC));
      pUiPropLoc->locSet(pCtx->locCur());
      pUiPropLoc->open();
      break;
    }

    case IDM_EDIT_ADD_LOC: {
      OmUiAddLoc* pUiAddLoc = static_cast<OmUiAddLoc*>(this->childById(IDD_ADD_LOC));
      pUiAddLoc->ctxSet(pCtx);
      pUiAddLoc->open();
      break;
    }

    // Menu : Edit > Package > []
    case IDM_EDIT_PKG_INST:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->pkgInst();
      break;

    case IDM_EDIT_PKG_UINS:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->pkgUnin();
      break;

    case IDM_EDIT_PKG_TRSH:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->pkgTrsh();
      break;

    case IDM_EDIT_PKG_OPEN:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->pkgOpen();
      break;

    case IDM_EDIT_PKG_EDIT:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->pkgEdit();
      break;

    case IDM_EDIT_PKG_INFO:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->pkgProp();
      break;

    // Menu : Edit > Remote > []
    case IDM_EDIT_RMT_DOWN:
      static_cast<OmUiMainNet*>(this->childById(IDD_MAIN_NET))->rmtDown(false);
      break;

    case IDM_EDIT_RMT_UPGR:
      static_cast<OmUiMainNet*>(this->childById(IDD_MAIN_NET))->rmtDown(true);
      break;

    case IDM_EDIT_RMT_FIXD:
      static_cast<OmUiMainNet*>(this->childById(IDD_MAIN_NET))->rmtFixd(false);
      break;

    case IDM_EDIT_RMT_INFO:
      static_cast<OmUiMainNet*>(this->childById(IDD_MAIN_NET))->rmtProp();
      break;

    case IDM_EDIT_OPTIONS:
      this->childById(IDD_PROP_MAN)->open();
      break;


    // Menu : Tools > []
    case IDM_TOOLS_EDI_REP:
      this->childById(IDD_TOOL_REP)->modeless();
      break;

    case IDM_TOOLS_EDI_PKG:
      this->childById(IDD_TOOL_PKG)->modeless();
      break;


    // Menu : Help > []
    case IDM_HELP_LOG:
      this->childById(IDD_HELP_LOG)->modeless();
      break;

    case IDM_HELP_ABOUT:
      this->childById(IDD_HELP_ABT)->open();
      break;
    }
  }

  return false;
}

