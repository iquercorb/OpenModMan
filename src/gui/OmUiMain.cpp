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
  _hIcAppS(static_cast<HICON>(LoadImage(hins,MAKEINTRESOURCE(IDB_APP_ICON),IMAGE_ICON,24,24,0))),
  _hIcAppL(static_cast<HICON>(LoadImage(hins,MAKEINTRESOURCE(IDB_APP_ICON),IMAGE_ICON,32,32,0))),
  _pageName(),
  _pageDial(),
  _quitPending(false),
  _onProcess(false),
  _safeEdit(false),
  _hIcBlank(Om_loadShellIcon(SIID_APPLICATION, true)),
  _hMenuFile(nullptr),
  _hMenuEdit(nullptr),
  _hMenuHelp(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"Library", new OmUiMainLib(hins)); // Library Tab
  this->_addPage(L"Network", new OmUiMainNet(hins)); // Network Tab

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
  this->setAccelerator(IDR_ACCEL);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMain::~OmUiMain()
{
  DestroyIcon(this->_hIcAppS);
  DestroyIcon(this->_hIcAppL);
  DestroyIcon(this->_hIcBlank);
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
void OmUiMain::setOnProcess(bool enable)
{
  this->enableItem(IDC_TC_MAIN, !enable);
  this->enableItem(IDC_CB_CTXLS, !enable);
  this->enableItem(IDC_LB_LOCLS, !enable);

  if(enable) {
    this->_onProcess = true;
  } else {
    this->_onProcess = false;
    // if close was requested, we quit the dialog
    if(this->_quitPending) {
      this->quit();
      return;
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::setSafeEdit(bool enable)
{
  if(enable) {
    // Enters safe edit state
    this->_safeEdit = true;
    // Unselect current Location in Library tab dialog
    static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->selLocation(-1);
    // Set to process state
    this->setOnProcess(true);
  } else {
    // Unselect current Location in Library tab dialog
    static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->refresh();
    // Return to normal process state
    this->setOnProcess(false);
    // Exit safe edit state
    this->_safeEdit = false;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::openContext(const wstring& path)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // Try to open Context
  if(pMgr->openContext(path)) {
    this->selContext(-1);   //< unselect Context
    this->_reloadCtxCb();  //< rebuild Combo-Box
    this->_reloadMenu();    //< refresh "Recent Files" popup list
  } else {
    Om_dialogBoxErr(this->_hwnd, L"Context loading failed", pMgr->lastError());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::selContext(int id)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // select the requested Context
  pMgr->selContext(id);

  // update the Context icon
  this->_reloadCtxIcon();

  // update menus
  if(pMgr->curContext()) {
    this->setMenuFile(IDM_FILE_CLOSE, MF_ENABLED); // File > Close
    this->setMenuEdit(0, MF_BYPOSITION|MF_ENABLED); // Edit > Context properties...
    this->setMenuEdit(3, MF_BYPOSITION|MF_ENABLED); // Edit > Add Location...
  } else {
    this->setMenuFile(IDM_FILE_CLOSE, MF_GRAYED); // File > Close
    this->setMenuEdit(0, MF_BYPOSITION|MF_GRAYED); // Edit > Context properties...
    this->setMenuEdit(2, MF_BYPOSITION|MF_GRAYED); // Edit > Location properties...
    this->setMenuEdit(3, MF_BYPOSITION|MF_GRAYED); // Edit > Add Location...
    this->setMenuEdit(5, MF_BYPOSITION|MF_GRAYED); // Edit > Package []
  }

  // update dialog window title
  this->_reloadCaption();

  // refresh visible tab
  for(size_t i = 0; i < this->_pageDial.size(); ++i) {
    if(this->_pageDial[i]->visible()) {
      this->_pageDial[i]->refresh();
      break;
    }
  }

  // forces control to select (or unselect) item
  HWND hCb = this->getItem(IDC_CB_CTXLS);
  if(id != SendMessageW(hCb, CB_GETCURSEL, 0, 0)) {
    SendMessageW(hCb, CB_SETCURSEL, id, 0);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HMENU OmUiMain::getMenuFile(unsigned pos)
{
  return GetSubMenu(this->_hMenuFile, pos);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HMENU OmUiMain::getMenuEdit(unsigned pos)
{
  return GetSubMenu(this->_hMenuEdit, pos);
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HMENU OmUiMain::getMenuHelp(unsigned pos)
{
  return GetSubMenu(this->_hMenuHelp, pos);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::setMenuFile(unsigned item, unsigned enable)
{
  // Set enable flag
  EnableMenuItem(this->_hMenuFile, item, enable);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::setMenuEdit(unsigned item, unsigned enable)
{
  EnableMenuItem(this->_hMenuEdit, item, enable);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::setMenuHelp(unsigned item, unsigned enable)
{
  EnableMenuItem(this->_hMenuHelp, item, enable);
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
void OmUiMain::_reloadCaption()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  wstring title = L"";
  if(pMgr->curContext()) {
    title += pMgr->curContext()->title() + L" - ";
  }

  title += OMM_APP_NAME;
  SetWindowTextW(this->_hwnd, title.c_str());
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_reloadCtxIcon()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  HICON hIcon;

  if(pMgr->curContext()) {
    if(pMgr->curContext()->icon()) {
      hIcon = pMgr->curContext()->icon();
    } else {
      hIcon = this->_hIcBlank;
    }
  } else {
    hIcon = this->_hIcBlank;
  }

  this->msgItem(IDC_SB_CTICO, STM_SETICON, reinterpret_cast<WPARAM>(hIcon));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_reloadMenu()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // handle to "File > Recent files" popup
  HMENU hMenu = this->getMenuFile(3); //< "File > Recent files" Menu-item

  // remove all entry from "File > Recent files >" popup except the two last
  // ones which are the separator and and the "Clear history" menuitem
  unsigned nb_item = GetMenuItemCount(hMenu) - 2;
  if(nb_item) {
    for(unsigned i = 0; i < nb_item; ++i)
      RemoveMenu(hMenu, 0, MF_BYPOSITION);
  }

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
    this->setMenuFile(3, MF_BYPOSITION|MF_ENABLED);
  } else {
    // disable the "File > Recent Files" popup
    this->setMenuFile(3, MF_BYPOSITION|MF_GRAYED);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_reloadCtxCb()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  HWND hCb = this->getItem(IDC_CB_CTXLS);

  // save current selection
  int cb_sel = SendMessageW(hCb, CB_GETCURSEL, 0, 0);

  // empty the Combo-Box
  SendMessageW(hCb, CB_RESETCONTENT, 0, 0);

  wstring item_str;

  // add Context(s) to Combo-Box
  if(pMgr->contextCount()) {

    EnableWindow(hCb, true);

    for(unsigned i = 0; i < pMgr->contextCount(); ++i) {

      item_str = pMgr->context(i)->title();
      item_str += L" - ";
      item_str += pMgr->context(i)->home();

      SendMessageW(hCb, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(item_str.c_str()));
    }

    // select the the previously selected Context
    if(cb_sel >= 0) {
      SendMessageW(hCb, CB_SETCURSEL, cb_sel, 0);
    } else {
      SendMessageW(hCb, CB_SETCURSEL, 0, 0);
      // select the last Context by default
      this->selContext(pMgr->contextCount()-1);
    }

  } else {
    // no Context disable the Combo-Box
    EnableWindow(hCb, false);
    // unselect all
    this->selContext(-1);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onInit()
{
  // set window icon
  SendMessageW(this->_hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(this->_hIcAppS));
  SendMessageW(this->_hwnd, WM_SETICON, ICON_BIG,   reinterpret_cast<LPARAM>(this->_hIcAppL));

  this->_createTooltip(IDC_CB_CTXLS, L"Select active context");

  // Get handle to menu
  HMENU hMenu = GetMenu(this->_hwnd);
  this->_hMenuFile = GetSubMenu(hMenu, 0);
  this->_hMenuEdit = GetSubMenu(hMenu, 1);
  this->_hMenuHelp = GetSubMenu(hMenu, 2);

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
      EnableThemeDialogTexture(this->_pageDial[i]->hwnd(), ETDT_ENABLETAB);
    }

    this->_pageDial[0]->show();
  }

  // refresh all elements
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onResize()
{
  // Context Icon
  this->_setItemPos(IDC_SB_CTICO, 6, 3, 19, 19);
  // Context list ComboBox
  this->_setItemPos(IDC_CB_CTXLS, 32, 6, this->width()-38 , 12);

  // Main Tab Control
  this->_setItemPos(IDC_TC_MAIN, 5, 25, this->width()-9, this->height()-30);

  // Resize page dialogs according IDC_TC_MAIN
  if(this->_pageDial.size()) {

    LONG pos[4];

    // get TabControl local coordinates
    GetWindowRect(this->getItem(IDC_TC_MAIN), reinterpret_cast<LPRECT>(&pos));
    MapWindowPoints(HWND_DESKTOP, this->_hwnd, reinterpret_cast<LPPOINT>(&pos), 2);

    // convert into base unit and adjust to keep inside the TabControl
    pos[0] = MulDiv(pos[0], 4, this->unitX()) + 1;
    pos[1] = MulDiv(pos[1], 8, this->unitY()) + 14;
    pos[2] = MulDiv(pos[2], 4, this->unitX()) - 2;
    pos[3] = MulDiv(pos[3], 8, this->unitY()) - 2;

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
  // rebuild the Context list Combo-Box
  this->_reloadCtxCb();

  // update the Context icon
  this->_reloadCtxIcon();

  // Update menus to have proper actives or grayed menus
  this->_reloadMenu();

  // update dialog window title
  this->_reloadCaption();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onClose()
{
  if(this->_onProcess) {
    // disable the window, first to notify user the application heard his request
    EnableWindow(this->_hwnd, false);
    // notify a close was requested
    this->_quitPending = true;
    // send an abort message to all Tab child dialogs
    for(size_t i = 0; i < this->_pageDial.size(); ++i) {
      PostMessage(this->_pageDial[i]->hwnd(), WM_COMMAND, MAKEWPARAM(IDC_BC_ABORT,0), 0);
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
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

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

    // handle "File > Recent Files" path click
    if(LOWORD(wParam) >= IDM_FILE_RECENT_PATH) { // recent

      vector<wstring> paths;
      pMgr->loadRecentFiles(paths);

      // subtract Command ID by the base resource ID to get real index
      this->openContext(paths[LOWORD(wParam) - IDM_FILE_RECENT_PATH]);
    }

    wstring item_str;

    switch(LOWORD(wParam))
    {

    case IDC_CB_CTXLS:
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        this->selContext(this->msgItem(IDC_CB_CTXLS, CB_GETCURSEL));
      }
      break;

    case IDM_FILE_NEW_CTX:
      this->childById(IDD_WIZ_CTX)->open(); // New Context Wizard
      break;

    case IDM_TOOLS_EDI_PKG:
      this->childById(IDD_TOOL_PKG)->open();
      break;

    case IDM_FILE_OPEN:
      if(Om_dialogOpenFile(item_str, this->_hwnd, L"Select Context file.", OMM_CTX_DEF_FILE_FILER, item_str)) {
        this->openContext(item_str);
      }
      break;

    case IDM_FILE_CLOSE:
      pMgr->closeCurrContext();
      this->selContext(-1);
      this->_reloadCtxCb();
      break;

    case IDM_FILE_CLEAR_HIST:
      pMgr->clearRecentFiles();
      this->_reloadMenu();
      break;

    case IDM_FILE_QUIT:
      this->quit();
      break;

    case IDM_EDIT_CTX_PROP:
      if(pMgr->curContext()) {
        OmUiPropCtx* pUiPropCtx = static_cast<OmUiPropCtx*>(this->childById(IDD_PROP_CTX));
        pUiPropCtx->setContext(pMgr->curContext());
        pUiPropCtx->open(true);
      }
      break;

    case IDM_EDIT_LOC_PROP:
      if(pMgr->curContext()) {
        OmUiPropLoc* pUiPropLoc = static_cast<OmUiPropLoc*>(this->childById(IDD_PROP_LOC));
        pUiPropLoc->setLocation(pMgr->curContext()->curLocation());
        pUiPropLoc->open(true);
      }
      break;

    case IDM_EDIT_CTX_ADDL:
      if(pMgr->curContext()) {
        OmUiAddLoc* pUiNewLoc = static_cast<OmUiAddLoc*>(this->childById(IDD_ADD_LOC));
        pUiNewLoc->setContext(pMgr->curContext());
        pUiNewLoc->open(true);
      }
      break;

    case IDM_EDIT_PKG_INST:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->install();
      break;

    case IDM_EDIT_PKG_UINS:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->uninstall();
      break;

    case IDM_EDIT_PKG_TRSH:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->moveTrash();
      break;

    case IDM_EDIT_PKG_OPEN:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->openExplore();
      break;

    case IDM_EDIT_PKG_INFO:
      static_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB))->viewDetails();
      break;

    case IDM_TOOLS_EDI_REP:
      this->childById(IDD_TOOL_REP)->open();
      break;

    case IDM_EDIT_OPTIONS:
      this->childById(IDD_PROP_MAN)->open();
      break;

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

