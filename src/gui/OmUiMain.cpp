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
#include "gui/OmUiPropCtx.h"
#include "gui/OmUiPropLoc.h"
#include "gui/OmUiPropMan.h"
#include "gui/OmUiHelpLog.h"
#include "gui/OmUiHelpAbout.h"
#include "gui/OmUiWizCtx.h"
#include "gui/OmUiNewBat.h"
#include "gui/OmUiNewLoc.h"
#include "gui/OmUiNewPkg.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMain::OmUiMain(HINSTANCE hins) : OmDialog(hins),
  _pageName(),
  _pageDial(),
  _quitPending(false),
  _onProcess(false),
  _safeEdit(false),
  _hMenuFile(nullptr),
  _hMenuEdit(nullptr),
  _hMenuHelp(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"Library", new OmUiMainLib(hins));

  // add children dialogs
  this->addChild(new OmUiPropMan(hins));    //< Dialog for general settings
  this->addChild(new OmUiPropCtx(hins));    //< Dialog for Context properties
  this->addChild(new OmUiPropLoc(hins));    //< Dialog for Location properties
  this->addChild(new OmUiHelpLog(hins));    //< Dialog for help debug log
  this->addChild(new OmUiHelpAbout(hins));  //< Dialog for help debug log
  this->addChild(new OmUiWizCtx(hins));     //< Dialog for New Context Wizard
  this->addChild(new OmUiNewPkg(hins));     //< Dialog for new Package
  this->addChild(new OmUiNewBat(hins));     //< Dialog for new Batch
  this->addChild(new OmUiNewLoc(hins));     //< Dialog for adding Location

  // set the accelerator table for the dialog
  this->setAccelerator(IDR_ACCEL);
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
void OmUiMain::setOnProcess(bool enable)
{
  if(enable) {
    this->_onProcess = true;
    // disable the Tab_Control
    EnableWindow(GetDlgItem(this->_hwnd, IDC_TC_TABS1), false);
    // disable the Context Combo_Box
    EnableWindow(GetDlgItem(this->_hwnd, IDC_CB_CTXLS), false);
    // disable the Location List_Box
    EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_LOCLS), false);
  } else {
    this->_onProcess = false;
    // if close was requested, we quit the dialog
    if(this->_quitPending) {
      this->quit();
      return;
    }
    // enable the Tab_Control
    EnableWindow(GetDlgItem(this->_hwnd, IDC_TC_TABS1), true);
    // enable the Context Combo_Box
    EnableWindow(GetDlgItem(this->_hwnd, IDC_CB_CTXLS), true);
    // enable the Location List_Box
    EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_LOCLS), true);
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
void OmUiMain::selContext(int i)
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  // select the requested Context
  manager->selContext(i);

  // update the Context icon
  this->_reloadCtxIcon();
  // update menus
  if(manager->curContext()) {
    this->setMenuFile(IDM_FILE_CLOSE, MF_ENABLED); // File > Close
    this->setMenuEdit(0, MF_BYPOSITION|MF_ENABLED); // Edit > Context

  } else {
    this->setMenuFile(IDM_FILE_CLOSE, MF_GRAYED); // File > Close
    this->setMenuEdit(0, MF_BYPOSITION|MF_GRAYED); // Edit > Context
  }
  // update dialog window title
  this->_reloadCaption();

  // refresh library tab
  OmUiMainLib* uiMainLib = reinterpret_cast<OmUiMainLib*>(this->childById(IDD_MAIN_LIB));
  if(uiMainLib) uiMainLib->refresh();

  // forces control to select item
  HWND hCb = GetDlgItem(this->_hwnd,IDC_CB_CTXLS);
  if(i != SendMessageW(hCb,CB_GETCURSEL,0,0)) {
    SendMessageW(hCb,CB_SETCURSEL,i,0);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::openContext(const wstring& path)
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  // Try to open Context
  if(manager->openContext(path)) {
    this->selContext(-1);   //< unselect Context
    this->_reloadCtxCb();  //< rebuild Combo-Box
    this->_reloadMenu();    //< refresh "Recent Files" popup list
  } else {
    Om_dialogBoxErr(this->_hwnd, L"Context loading failed", manager->lastError());
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
void OmUiMain::_pagesOnShow(unsigned tab_id)
{
  if(this->_pageDial.size() && this->_hwnd) {

    TCITEMW tcPage;
    tcPage.mask = TCIF_TEXT;

    for(size_t i = 0; i < this->_pageDial.size(); ++i) {

      tcPage.pszText = (LPWSTR)this->_pageName[i].c_str();
      SendMessageW(GetDlgItem(this->_hwnd, tab_id), TCM_INSERTITEMW, i, (LPARAM)&tcPage);

      this->_pageDial[i]->modeless(false);
      EnableThemeDialogTexture(this->_pageDial[i]->hwnd(), ETDT_ENABLETAB);
    }

    this->_pageDial[0]->show();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_pagesOnNotify(unsigned tab_id, WPARAM wParam, LPARAM lParam)
{
  if(this->_pageDial.size()) {

    // check for notify from the specified TabControl
    if(LOWORD(wParam) == tab_id) {
      // check for a "selection changed" notify
      if(((LPNMHDR)lParam)->code == TCN_SELCHANGE) {

        // get TabControl current selection
        int tab_sel = SendMessageW(GetDlgItem(this->_hwnd, tab_id), TCM_GETCURSEL, 0, 0);

        // change page dialog visibility according selection
        if(tab_sel >= 0) {
          for(int i = 0; i < static_cast<int>(this->_pageDial.size()); ++i) {
            if(i == tab_sel) {
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


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_pagesOnResize(unsigned tab_id)
{
  if(this->_pageDial.size()) {

    LONG pos[4];

    // get TabControl local coordinates
    GetWindowRect(GetDlgItem(this->_hwnd, tab_id), (LPRECT)&pos);
    MapWindowPoints(HWND_DESKTOP, this->_hwnd, (LPPOINT)&pos, 2);

    // convert into base unit and adjust to keep inside the TabControl
    pos[0] = MulDiv(pos[0], 4, this->unitX()) + 1;
    pos[1] = MulDiv(pos[1], 8, this->unitY()) + 14;
    pos[2] = MulDiv(pos[2], 4, this->unitX()) - 2;
    pos[3] = MulDiv(pos[3], 8, this->unitY()) - 2;

    // Map again in pixels
    MapDialogRect(this->_hwnd, (LPRECT)&pos);
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
void OmUiMain::_pagesOnClose()
{
  // send an abort message to all Tab child dialogs
  for(size_t i = 0; i < this->_pageDial.size(); ++i) {
    PostMessage(this->_pageDial[i]->hwnd(), WM_COMMAND, MAKEWPARAM(IDC_BC_ABORT,0), 0);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_reloadCaption()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  wstring title = L"";
  if(manager->curContext()) {
    title += manager->curContext()->title() + L" - ";
  }

  title += OMM_APP_NAME;
  SetWindowTextW(this->_hwnd, title.c_str());
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_reloadCtxIcon()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  if(manager->curContext()) {
    if(manager->curContext()->icon()) {
      SendMessage(GetDlgItem(this->_hwnd, IDC_SB_CTICO), STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)manager->curContext()->icon());
    } else {
      HICON hicon = (HICON)Om_loadShellIcon(SIID_APPLICATION, true);
      SendMessage(GetDlgItem(this->_hwnd, IDC_SB_CTICO), STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hicon);
    }
  } else {
    HICON hicon = (HICON)Om_loadShellIcon(SIID_APPLICATION, true);
    SendMessage(GetDlgItem(this->_hwnd, IDC_SB_CTICO), STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hicon);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_reloadMenu()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

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
  manager->loadRecentFiles(path);

  // add the recent file path or disable popup
  if(path.size()) {
    wchar_t wcbuf[OMM_MAX_PATH];
    for(size_t i = 0; i < path.size(); ++i) {
      swprintf(wcbuf, OMM_MAX_PATH, L"&%d %ls", path.size() - i, path[i].c_str());
      InsertMenuW(hMenu, 0, MF_BYPOSITION|MF_STRING, IDM_FILE_RECENT_PATH + i, wcbuf);
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
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  HWND hCb = GetDlgItem(this->_hwnd, IDC_CB_CTXLS);

  // save current selection
  int cb_sel = SendMessageW(hCb, CB_GETCURSEL, 0, 0);

  // empty the Combo-Box
  SendMessageW(hCb, CB_RESETCONTENT, 0, 0);

  wstring label;

  // add Context(s) to Combo-Box
  if(manager->contextCount()) {

    EnableWindow(hCb, true);

    for(unsigned i = 0; i < manager->contextCount(); ++i) {

      label = manager->context(i)->title();
      label += L" - ";
      label += manager->context(i)->home();

      SendMessageW(hCb, CB_ADDSTRING, i, (LPARAM)label.c_str());
    }

    // select the the previously selected Context
    if(cb_sel >= 0 && cb_sel < static_cast<int>(manager->contextCount())) {
      SendMessageW(hCb, CB_SETCURSEL, cb_sel, 0);
    } else {
      SendMessageW(hCb, CB_SETCURSEL, 0, 0);
      // select the last Context by default
      this->selContext(manager->contextCount()-1);
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
void OmUiMain::_onShow()
{
  this->_createTooltip(IDC_CB_CTXLS, L"Select active context");

  // Get handle to menu
  HMENU hMenu = GetMenu(this->_hwnd);
  this->_hMenuFile = GetSubMenu(hMenu, 0);
  this->_hMenuEdit = GetSubMenu(hMenu, 1);
  this->_hMenuHelp = GetSubMenu(hMenu, 2);

  // set window icon
  HICON hIcon = (HICON)LoadImage(this->_hins,MAKEINTRESOURCE(IDB_APP_ICON),IMAGE_ICON,24,24,0);
  SendMessage(this->_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

  hIcon = (HICON)LoadImage(this->_hins,MAKEINTRESOURCE(IDB_APP_ICON),IMAGE_ICON,32,32,0);
  SendMessage(this->_hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  RECT rect = {0,0,0,0};
  // set window to saved position and size
  manager->loadWindowRect(rect);

  int x = rect.left;
  int y = rect.top;
  int w = rect.right - rect.left;
  int h = rect.bottom - rect.top;

  if(x >= 0 && y >= 0 && w > 0 && h > 0) {
    SetWindowPos(this->_hwnd, nullptr, x, y, w, h, SWP_NOZORDER);
  }

  // initialize TabControl with pages dialogs
  this->_pagesOnShow(IDC_TC_TABS1);

  // refresh all elements
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onResize()
{
  // Context Icon
  this->_setControlPos(IDC_SB_CTICO, 6, 3, 19, 19);
  // Context list ComboBox
  this->_setControlPos(IDC_CB_CTXLS, 32, 6, this->width()-38 , 12);
  // Main Tab Control
  this->_setControlPos(IDC_TC_TABS1, 5, 25, this->width()-9, this->height()-30);
  // Resize page dialogs according IDC_TC_TABS1
  this->_pagesOnResize(IDC_TC_TABS1);
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
    this->_pagesOnClose();
  } else {
    this->quit();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMain::_onQuit()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  RECT rec;
  GetWindowRect(this->_hwnd, &rec);
  manager->saveWindowRect(rec);

  // Exist dialog thread
  PostQuitMessage(0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMain::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  if(uMsg == WM_NOTIFY) {
    // handle TabControl page selection change
    this->_pagesOnNotify(IDC_TC_TABS1, wParam, lParam);
  }

  if(uMsg == WM_COMMAND) {

    // handle "File > Recent Files" path click
    if(LOWORD(wParam) >= IDM_FILE_RECENT_PATH) { // recent

      vector<wstring> paths;
      manager->loadRecentFiles(paths);

      // subtract Command ID by the base resource ID to get real index
      this->openContext(paths[LOWORD(wParam) - IDM_FILE_RECENT_PATH]);
    }

    wchar_t wcbuf[OMM_MAX_PATH];

    switch(LOWORD(wParam))
    {

    case IDC_CB_CTXLS:
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        this->selContext(SendMessageW(GetDlgItem(this->_hwnd, IDC_CB_CTXLS), CB_GETCURSEL, 0, 0));
      }
      break;

    case IDM_FILE_NEW_CTX:
      this->childById(IDD_WIZ_CTX)->open(); // New Context Wizard
      break;

    case IDM_FILE_NEW_PKG:
      this->childById(IDD_NEW_PKG)->open();
      break;

    case IDM_FILE_OPEN:
      if(Om_dialogOpenFile(wcbuf, this->_hwnd, L"Select file.", OMM_CTX_DEF_FILE_FILER, wcbuf)) {
        this->openContext(wcbuf);
      }
      break;

    case IDM_FILE_CLOSE:
      manager->closeCurrContext();
      this->selContext(-1);
      this->_reloadCtxCb();
      break;

    case IDM_FILE_CLEAR_HIST:
      manager->clearRecentFiles();
      this->_reloadMenu();
      break;

    case IDM_FILE_QUIT:
      this->quit();
      break;

    case IDM_EDIT_CTX_PROP:
      if(manager->curContext()) {
        OmUiPropCtx* uiPropCtx = reinterpret_cast<OmUiPropCtx*>(this->childById(IDD_PROP_CTX));
        uiPropCtx->setContext(manager->curContext());
        uiPropCtx->open(true);
      }
      break;

    case IDM_EDIT_CTX_ADDL:
      if(manager->curContext()) {
        OmUiNewLoc* uiNewLoc = reinterpret_cast<OmUiNewLoc*>(this->childById(IDD_NEW_LOC));
        uiNewLoc->setContext(manager->curContext());
        uiNewLoc->open(true);
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

    case IDM_EDIT_OPTIONS:
      this->childById(IDD_PROP_MAN)->open();
      break;

    case IDM_HELP_LOG:
      this->childById(IDD_HELP_LOG)->modeless();
      break;

    case IDM_HELP_ABOUT:
      this->childById(IDD_HELP_ABOUT)->open();
      break;
    }
  }

  return false;
}

