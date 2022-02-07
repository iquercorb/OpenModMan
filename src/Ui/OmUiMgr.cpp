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

#include "Ui/OmUiMgrMain.h"
#include "Ui/OmUiMgrFoot.h"
#include "Ui/OmUiMgrMainLib.h"
#include "Ui/OmUiMgrMainNet.h"
#include "Ui/OmUiMgrMainTst.h" //< tab for test purposes
#include "Ui/OmUiAddBat.h"
#include "Ui/OmUiAddRep.h"
#include "Ui/OmUiAddLoc.h"
#include "Ui/OmUiPropCtx.h"
#include "Ui/OmUiPropLoc.h"
#include "Ui/OmUiPropMan.h"
#include "Ui/OmUiPropBat.h"
#include "Ui/OmUiHelpLog.h"
#include "Ui/OmUiHelpAbt.h"
#include "Ui/OmUiWizCtx.h"
#include "Ui/OmUiToolPkg.h"
#include "Ui/OmUiToolRep.h"

#include "Util/OmUtilStr.h"
#include "Util/OmUtilDlg.h"
#include "Util/OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "Ui/OmUiMgr.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgr::OmUiMgr(HINSTANCE hins) : OmDialog(hins),
  /*_pageName(), _pageDial(), */
  _pUiMgrMain(nullptr), _pUiMgrFoot(nullptr),
  _divIsHovr(false), _divIsCapt(false), _divMove{},
  _freeze_mode(false), _freeze_quit(false)
{
  // add main frames
  this->_pUiMgrMain = new OmUiMgrMain(hins);
  this->addChild(this->_pUiMgrMain);
  this->_pUiMgrFoot = new OmUiMgrFoot(hins);
  this->addChild(this->_pUiMgrFoot);

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
OmUiMgr::~OmUiMgr()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMgr::id() const
{
  return IDD_MGR;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgr::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgr::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  this->_freeze_mode = enable;

  // disable Context ComboBox
  this->enableItem(IDC_CB_CTX, !enable);

  // disable menus
  int state = enable ? MF_GRAYED : MF_ENABLED;
  this->setPopupItem(this->_menu, 0, state); //< File menu
  this->setPopupItem(this->_menu, 1, state); //< Edit menu
  this->setPopupItem(this->_menu, 2, state); //< Tools menu

  // force menu bar to redraw so enabled/grayed state
  // is properly visually updated
  DrawMenuBar(this->_hwnd);

  // passes the message to child tab dialog
  this->_pUiMgrMain->freeze(enable);
  this->_pUiMgrFoot->freeze(enable);

  // check whether freeze was aborted, this happen if user
  // requested to close the dialog while in freeze mode.
  if(this->_freeze_quit && !enable) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMgr::freeze (quit)\n";
    #endif

    this->quit();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgr::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgr::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  this->_pUiMgrMain->safemode(enable);
  this->_pUiMgrFoot->safemode(enable);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgr::ctxOpen(const wstring& path)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // Try to open Context
  if(pMgr->ctxOpen(path)) {

    // refresh
    this->refresh();

  } else {

    Om_dlgBox_okl(this->_hwnd, L"Open Software Context", IDI_ERR,
                 L"Software Context open error", L"Software Context "
                 "loading failed because of the following error:",
                 pMgr->lastError());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgr::ctxClose()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgr::ctxClose\n";
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
void OmUiMgr::ctxSel(int id)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgr::ctxSel " << id << "\n";
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
void OmUiMgr::_buildCaption()
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
void OmUiMgr::_buildSbCtx()
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
void OmUiMgr::_buildMnRct()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgr::_buildMnRct\n";
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
void OmUiMgr::_buildCbCtx()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgr::_buildCbCtx\n";
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
void OmUiMgr::_onCbCtxSel()
{
  int cb_sel = this->msgItem(IDC_CB_CTX, CB_GETCURSEL);

  if(cb_sel >= 0) this->ctxSel(cb_sel);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgr::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgr::_onShow\n";
  #endif

  this->_pUiMgrMain->show();
  //this->_pUiMgrFoot->show();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgr::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgr::_onInit\n";
  #endif

  // set window icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));

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

  // create frames dialogs
  this->_pUiMgrMain->modeless(true);
  this->_pUiMgrFoot->modeless(true);

  // initialize frames to the proper size and position
  h = -1;
  pMgr->loadWindowFoot(&h);
  // we emulate frame resize by user
  this->_divIsCapt = true;
  this->_divMove[2] = h;
  this->_onResize();
  this->_divIsCapt = false;

  // Set default context icon
  HICON hIc = Om_getShellIcon(SIID_APPLICATION, true);
  this->msgItem(IDC_SB_ICON, STM_SETICON, reinterpret_cast<WPARAM>(hIc));

  // refresh all elements
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgr::_onResize()
{
  #ifdef DEBUG
  //std::cout << "DEBUG => OmUiMgr::_onResize\n";
  #endif

  long y, h, rc[4];

  if(!this->_divIsCapt) {

    // Context list ComboBox
    this->_setItemPos(IDC_CB_CTX, 4, 3, this->cliWidth()-42 , 28, true);
    // Context Icon
    this->_setItemPos(IDC_SB_ICON, this->cliWidth()-33, 3, 28, 28, true);

  } else {

    // store old foot frame rect for future redraw
    GetWindowRect(this->_pUiMgrFoot->hwnd(), reinterpret_cast<LPRECT>(&rc));
    MapWindowPoints(HWND_DESKTOP, this->_hwnd, reinterpret_cast<LPPOINT>(&rc), 2);

  }

  // get foot frame height, if we are in frame resize process we get it
  // from the temporary value stored after WM_MOUSEMOVE message
  h = (this->_divIsCapt) ? this->_divMove[2] : this->_pUiMgrFoot->height();

  // clamp foot frame height to prevent covering entire screen on resize
  if(h > this->cliHeight() - 200) h = this->cliHeight() - 200;

  // foot frame top position, relative to client
  y = this->cliHeight() - h;

  // resize and move frames
  this->_setChildPos(this->_pUiMgrFoot->hwnd(), 4, y - 4 , this->cliWidth() - 8, h, true);
  this->_setChildPos(this->_pUiMgrMain->hwnd(), 4, 36, this->cliWidth() - 8, y - 44, true);

  if(!this->_divIsCapt) {

    // simple resize, we can redraw entire window
    RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW);

  } else {

    // Calling SetWindowPos outside a resize modal loop (standard resize by user),
    // causes insane amount of flickering, probably due to suboptimal erase and
    // redraw sequences from child to parent.
    //
    // I tested almost every approach, handling WM_NCCALCSIZE and WM_ERASEBKGND
    // window messages, implementing a hierachical DeferWindowPos mechanism,
    // NOTHING WORKED ! The resize modal loop seem to use some internal bypass to
    // gracefully redraw the whole window at once without flickering. Thing that I
    // cannot reproduce (but THIS is what should be done), since this is
    // documented nowhere.
    //
    // The only thing that reduce significantly the flickering where to prevent
    // ALL call to SetWindowPos to redraw by including the SWP_NOREDRAW flag, then
    // handling redraw 'manually' from the parent window, using only RedrawWindow
    // with the proper flags.

    // update the footer frame and area around the splitter, without
    // erasing window background to reduce flickering.
    rc[1] -= 56; //< encompasses the bottom row of buttons and h-scroll of the main frame
    RedrawWindow(this->_hwnd, reinterpret_cast<RECT*>(&rc), nullptr, RDW_INVALIDATE|RDW_UPDATENOW);

    // force redraw only the area between the two frames, now we erase
    // the window background
    rc[1] = y - 8; rc[3] = y - 4;
    RedrawWindow(this->_hwnd, reinterpret_cast<RECT*>(&rc), nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgr::_onRefresh()
{
  #ifdef DEBUG
  //std::cout << "DEBUG => OmUiMgr::_onRefresh\n";
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
void OmUiMgr::_onClose()
{
  // check whether dialog is in freeze mode, in this case we cannot quit
  // right now, we need to carefully abort all running threads first
  if(this->_freeze_mode) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMgr::_onClose (freeze_mode)\n";
    #endif

    // quit dialog on next unfreeze
    this->_freeze_quit = true;

    // disable the window to notify user its request to quit is acknowledged
    EnableWindow(this->_hwnd, false);

    // send message to all frames to request abort all their jobs
    this->_pUiMgrMain->postMessage(UWM_MAIN_ABORT_REQUEST);
    this->_pUiMgrFoot->postMessage(UWM_MAIN_ABORT_REQUEST);

  } else {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMgr::_onClose\n";
    #endif

    this->quit();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgr::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgr::_onQuit\n";
  #endif

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  RECT rec;
  GetWindowRect(this->_hwnd, &rec);
  pMgr->saveWindowRect(rec);
  pMgr->saveWindowFoot(this->_pUiMgrFoot->height());

  // Exist dialog thread
  PostQuitMessage(0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMgr::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // release the previously captured mouse for frames move and resize process
  if(uMsg == WM_LBUTTONUP) {
    if(this->_divIsCapt) {
      // release captured mouse
      ReleaseCapture();
      // quit the frame move and resize process
      this->_divIsCapt = false;
    }
    return 0;
  }

  // if mouse cursor is hovering between frames, checks for left button click
  // by user to capture mouse and entering the frames move and resize process
  if(uMsg == WM_LBUTTONDOWN || (uMsg == WM_PARENTNOTIFY && wParam == WM_LBUTTONDOWN)) {
    if(this->_divIsHovr) {
      // keeps mouse pointer position and foot frame height at
      // capture to later calculate relative moves and size changes
      this->_divMove[0] = HIWORD(lParam);
      this->_divMove[1] = this->_pUiMgrFoot->height();
      // capture the mouse
      SetCapture(this->_hwnd);
      // we now are in frame move and resize process
      this->_divIsCapt = true;    }
    return 0;
  }

  // changes the default cursor arrow to north-south resize arrows according
  // cursor hovering between the frames.
  if(uMsg == WM_SETCURSOR) {
    // checks whether cursor is hovering between frames
    if(this->_divIsHovr) {
      SetCursor(LoadCursor(0,IDC_SIZENS));
      return 1; //< bypass default process
    }
  }

  // track mouse cursor position either to detect whether cursor hover
  // between the frames (to change cursor) or, if we captured cursor, to
  // process the move and resize of the frames
  if(uMsg == WM_MOUSEMOVE) {
    long p = HIWORD(lParam);
    if(GetCapture() == this->_hwnd) {
      // calculate new foot height according new cursor position
      long h = this->_divMove[1] + this->_divMove[0] - p;
      // clamp to reasonable values
      if(h < 170) h = 170;
      if(h > this->cliHeight()-200) h = this->cliHeight()-200;
      // move the splitter / resize frames
      if(h != this->_pUiMgrFoot->height()) {
        this->_divMove[2] = h;
        this->_onResize();
      }
    } else {
      // checks whether mouse cursor is hovering between frames, we take a
      // good margin around the gap to make it easier to catch.
      long y = this->cliHeight() - this->_pUiMgrFoot->height();
      this->_divIsHovr = (p > (y - 10) && p < (y - 1));
    }
    return 0;
  }

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

    return 0;
  }

  if(uMsg == WM_COMMAND) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMgr::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
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
      if(Om_dlgOpenFile(item_str, this->_hwnd, L"Open Context file", OMM_CTX_DEF_FILE_FILER, item_str)) {
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
      static_cast<OmUiMgrMainLib*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_LIB))->pkgInst();
      break;

    case IDM_EDIT_PKG_UINS:
      static_cast<OmUiMgrMainLib*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_LIB))->pkgUnin();
      break;

    case IDM_EDIT_PKG_CLNS:
      static_cast<OmUiMgrMainLib*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_LIB))->pkgClns();
      break;

    case IDM_EDIT_PKG_TRSH:
      static_cast<OmUiMgrMainLib*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_LIB))->pkgTrsh();
      break;

    case IDM_EDIT_PKG_OPEN:
      static_cast<OmUiMgrMainLib*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_LIB))->pkgOpen();
      break;

    case IDM_EDIT_PKG_EDIT:
      static_cast<OmUiMgrMainLib*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_LIB))->pkgEdit();
      break;

    case IDM_EDIT_PKG_INFO:
      static_cast<OmUiMgrMainLib*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_LIB))->pkgProp();
      break;

    // Menu : Edit > Remote > []
    case IDM_EDIT_RMT_DOWN:
      static_cast<OmUiMgrMainNet*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_NET))->rmtDown(false);
      break;

    case IDM_EDIT_RMT_UPGR:
      static_cast<OmUiMgrMainNet*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_NET))->rmtDown(true);
      break;

    case IDM_EDIT_RMT_FIXD:
      static_cast<OmUiMgrMainNet*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_NET))->rmtFixd(false);
      break;

    case IDM_EDIT_RMT_INFO:
      static_cast<OmUiMgrMainNet*>(this->_pUiMgrMain->childById(IDD_MGR_MAIN_NET))->rmtProp();
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

