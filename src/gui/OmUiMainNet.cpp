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
#include "gui/OmUiMainNet.h"
#include "gui/OmUiAddLoc.h"
#include "gui/OmUiAddRep.h"
#include "gui/OmUiMain.h"



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainNet::OmUiMainNet(HINSTANCE hins) : OmDialog(hins),
  _hFtTitle(Om_createFont(18, 800, L"Ms Shell Dlg")),
  _hFtMonos(Om_createFont(14, 700, L"Consolas")),
  _hBmBlank(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_PKG_BLANK),0,0,0,0))),
  _hBmBcNew(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_BTN_ADD),0,0,0,0))),
  _hBmBcDel(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_BTN_REM),0,0,0,0))),
  _hBmBcRef(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_BTN_REF),0,0,0,0)))
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainNet::~OmUiMainNet()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMainNet::id() const
{
  return IDD_MAIN_NET;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::selLocation(int i)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // main window dialog
  OmUiMain* pUiMain = static_cast<OmUiMain*>(this->_parent);

  // select the requested Location
  if(pMgr->curContext()) {

    OmContext* pCtx = pMgr->curContext();

    pCtx->selLocation(i);

    if(pCtx->curLocation()) {

      // enable the "Edit > Location properties..." menu
      pUiMain->setMenuEdit(2, MF_BYPOSITION|MF_ENABLED);

    } else {

      // disable the "Edit > Location properties..." menu
      pUiMain->setMenuEdit(2, MF_BYPOSITION|MF_GRAYED);
    }
  }


  // refresh
  this->_reloadRepLb();

  //this->_reloadLibEc();

  //this->_reloadLibLv();

  // forces control to select item
  HWND hCb = this->getItem(IDC_CB_LOCLS);

  if(i != SendMessageW(hCb, CB_GETCURSEL, 0, 0))
    SendMessageW(hCb, CB_SETCURSEL, i, 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_repoDel()
{
  int lb_sel = this->msgItem(IDC_LB_REPLS, LB_GETCURSEL);

  if(lb_sel >= 0) {

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->curContext();
    OmLocation* pLoc = pCtx->curLocation();

    // warns the user before committing the irreparable
    wstring qry = L"Are your sure you want to delete the Repository \"";
    qry += pLoc->repository(lb_sel)->base() + L" - " + pLoc->repository(lb_sel)->name();
    qry += L"\" ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Repository", qry)) {
      return;
    }

    pLoc->remRepository(lb_sel);

    // reload the repository list-box
    this->_reloadRepLb();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onSelectRep()
{
  int lb_sel = this->msgItem(IDC_LB_BATLS, LB_GETCURSEL);

  this->enableItem(IDC_BC_DEL, (lb_sel >= 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_reloadLocCb()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->curContext();

  HWND hCb = this->getItem(IDC_CB_LOCLS);

  if(pCtx == nullptr) {
    // no Location, disable the List-Box
    EnableWindow(hCb, false);
    // unselect Location
    this->selLocation(-1);

    return;
  }

  // save current selection
  int cb_sel = SendMessageW(hCb, CB_GETCURSEL, 0, 0);

  // empty the Combo-Box
  SendMessageW(hCb, CB_RESETCONTENT, 0, 0);

  // add Context(s) to Combo-Box
  if(pCtx->locationCount()) {

    wstring label;

    EnableWindow(hCb, true);

    for(unsigned i = 0; i < pCtx->locationCount(); ++i) {

      label = pCtx->location(i)->title();
      label += L" - ";

      // checks whether installation destination path is valid
      if(pCtx->location(i)->checkAccessDst()) {
        label += pCtx->location(i)->installDir();
      } else {
        label += L"<folder access error>";

        wstring wrn = L"Configured Location's Destination folder \"";
        wrn += pCtx->location(i)->installDir()+L"\"";
        wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";
        Om_dialogBoxWarn(this->_hwnd, L"Destination folder access error", wrn);
      }

      SendMessageW(hCb, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(label.c_str()));
    }

    // select the the previously selected Context
    if(cb_sel >= 0) {
      SendMessageW(hCb, CB_SETCURSEL, cb_sel, 0);
      this->_reloadRepLb(); //< reload repositories list
      //this->_reloadLibEc(); //< reload displayed library path
      //this->_reloadLibLv(true); //< reload + reparse packages list
    } else {
      SendMessageW(hCb, CB_SETCURSEL, 0, 0);
      // select the first Location by default
      this->selLocation(0);
    }
  } else {
    // no Location, disable the List-Box
    EnableWindow(hCb, false);
    // unselect Location
    this->selLocation(-1);

    // if Context have no Location, we ask user to create at least one
    wstring qry = L"The current Context does not have any configured "
                  L"Location. A Context needs at least one Location.\n\n"
                  L"Do you want to configure a new Location now ?";

    if(Om_dialogBoxQuerry(this->_hwnd, L"No Location found", qry)) {
      OmUiAddLoc* pUiNewLoc = static_cast<OmUiAddLoc*>(this->siblingById(IDD_ADD_LOC));
      pUiNewLoc->setContext(pCtx);
      pUiNewLoc->open(true);
    }

  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_reloadRepLb()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->curContext();
  OmLocation* pLoc = (pCtx) ? pCtx->curLocation() : nullptr;

  HWND hLb = this->getItem(IDC_LB_REPLS);

  // empty List-Box
  SendMessageW(hLb, LB_RESETCONTENT, 0, 0);

  if(pLoc) {

    wstring str;
    OmRepository* pRep;

    for(unsigned i = 0; i < pLoc->repositoryCount(); ++i) {
      pRep = pLoc->repository(i);
      str = pRep->base() + L" - " + pRep->name();
      SendMessageW(hLb, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(str.c_str()));
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onInit()
{
  // Defines fonts for package description, title, and log output
  this->msgItem(IDC_SC_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtTitle), true);
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtMonos), true);
  // Set batches New and Delete buttons icons
  this->msgItem(IDC_BC_NEW, BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcNew));
  this->msgItem(IDC_BC_DEL, BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcDel));
  this->msgItem(IDC_BC_CHK, BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcRef));

  // define controls tool-tips
  this->_createTooltip(IDC_CB_LOCLS,  L"Select active location");
  this->_createTooltip(IDC_BC_UPGD,   L"Upgrade selected package(s)");
  this->_createTooltip(IDC_BC_SYNC,   L"Synchronize selected package(s)");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort current process");

  // Initialize the ListView control
  HWND hLv = this->getItem(IDC_LV_PKGLS);

  DWORD dwExStyle = LVS_EX_FULLROWSELECT|
                    LVS_EX_SUBITEMIMAGES|
                    LVS_EX_DOUBLEBUFFER;

  SendMessageW(hLv, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwExStyle);

  // we now add columns into our list-view control
  LVCOLUMNW lvCol;
  lvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;

  //  "The alignment of the leftmost column is always LVCFMT_LEFT; it
  // cannot be changed." says Mr Microsoft. Do not ask why, the Microsoft's
  // mysterious ways... So, don't try to fix this.

  lvCol.pszText = const_cast<LPWSTR>(L"Status");
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.cx = 43;
  lvCol.iSubItem = 0;
  SendMessageW(hLv, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Name");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 550;
  lvCol.iSubItem = 1;
  SendMessageW(hLv, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Version");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem = 2;
  SendMessageW(hLv, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&lvCol));

  // force refresh
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onShow()
{
  OmUiMain* pUiMain = static_cast<OmUiMain*>(this->_parent);

  // disable "Edit > Package" in main menu
  pUiMain->setMenuEdit(5, MF_BYPOSITION|MF_GRAYED);
  // disable the "Edit > Package > []" elements
  HMENU hMenu = pUiMain->getMenuEdit(5);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_INST, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_UINS, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_OPEN, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_TRSH, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_EDIT_PKG_INFO, MF_GRAYED);

  // select location according current ComboBox selection
  this->selLocation(this->msgItem(IDC_CB_LOCLS, CB_GETCURSEL));

  // refresh dialog
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onResize()
{
  // Locations Combo-Box
  this->_setItemPos(IDC_CB_LOCLS, 5, 5, this->width()-10, 12);

  // Repositories label
  this->_setItemPos(IDC_SC_LBL01, 5, 23, 136, 12);
  // Repositories List-Box
  this->_setItemPos(IDC_LB_REPLS, 5, 37, this->width()-34, 28);
  // Repositories Apply, New.. and Delete buttons
  this->_setItemPos(IDC_BC_CHK, this->width()-55, 20, 50, 14);
  this->_setItemPos(IDC_BC_NEW, this->width()-25, 36, 20, 14);
  this->_setItemPos(IDC_BC_DEL, this->width()-25, 52, 20, 14);

  // Horizontal separator
  this->_setItemPos(IDC_SC_SEPAR, 5, 70, this->width()-10, 1);

  // Library path EditControl
  //this->_setItemPos(IDC_EC_INPT1, 5, 120, this->width()-10, 12);

  // Package List ListView
  this->_setItemPos(IDC_LV_PKGLS, 5, 75, this->width()-10, this->height()-191);
  // Resize the ListView column
  LONG size[4];
  HWND hLv = this->getItem(IDC_LV_PKGLS);
  GetClientRect(hLv, reinterpret_cast<LPRECT>(&size));
  SendMessageW(hLv, LVM_SETCOLUMNWIDTH, 1, size[2]-125);

  // Upgrade and Sync buttons
  this->_setItemPos(IDC_BC_UPGD, 5, this->height()-114, 50, 14);
  this->_setItemPos(IDC_BC_SYNC, 55, this->height()-114, 50, 14);
  // Progress bar
  this->_setItemPos(IDC_PB_PGRES, 107, this->height()-113, this->width()-165, 12);
  // Abort button
  this->_setItemPos(IDC_BC_ABORT, this->width()-55, this->height()-114, 50, 14);

  // Package name/title
  this->_setItemPos(IDC_SC_TITLE, 5, this->height()-96, this->width()-161, 12);
  // Package snapshot
  this->_setItemPos(IDC_SB_PKIMG, 5, this->height()-83, 85, 78);
  // Package description
  this->_setItemPos(IDC_EC_PKTXT, 95, this->height()-83, this->width()-101, 78);

  InvalidateRect(this->_hwnd, nullptr, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onRefresh()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // disable all packages buttons
  this->enableItem(IDC_BC_ABORT, false);
  this->enableItem(IDC_BC_UPGD, false);
  this->enableItem(IDC_BC_SYNC, false);

  // hide package details
  ShowWindow(this->getItem(IDC_SC_TITLE), false);
  ShowWindow(this->getItem(IDC_EC_PKTXT), false);
  ShowWindow(this->getItem(IDC_SB_PKIMG), false);

  // disable the Progress-Bar
  this->enableItem(IDC_PB_PGRES, false);

  this->_reloadLocCb(); //< reload Location Combo-Box

  // disable all batches buttons
  this->enableItem(IDC_BC_CHK, false);
  this->enableItem(IDC_BC_NEW, (pMgr->curContext() != nullptr));
  this->enableItem(IDC_BC_DEL, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMainNet::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->curContext();
  OmLocation* pLoc = (pCtx) ? pCtx->curLocation() : nullptr;

  if(pLoc == nullptr)
    return false;

  if(uMsg == WM_NOTIFY) {

    NMHDR* pNmhdr = reinterpret_cast<NMHDR*>(lParam);

    if(LOWORD(wParam) == IDC_LV_PKGLS) {

      // if thread is running we block all interaction
      /*
      if(this->_install_hth || this->_uninstall_hth)
        return false;
      */

      if(pNmhdr->code == NM_DBLCLK) {
        //this->toggle();
        return false;
      }

      if(pNmhdr->code == LVN_ITEMCHANGED) {
        // update package(s) selection
        //this->_onSelectPkg();
        return false;
      }

      if(pNmhdr->code == NM_RCLICK) {
        // Open the popup menu
        //this->_showPkgPopup();
        return false;
      }

      if(pNmhdr->code == LVN_COLUMNCLICK) {

        NMLISTVIEW* pNmlv = reinterpret_cast<NMLISTVIEW*>(lParam);

        switch(pNmlv->iSubItem)
        {
        case 0:
          //pLoc->setPackageSorting(PKG_SORTING_STAT);
          break;
        case 2:
          //pLoc->setPackageSorting(PKG_SORTING_VERS);
          break;
        default:
          //pLoc->setPackageSorting(PKG_SORTING_NAME);
          break;
        }
        this->refresh();
        return false;
      }
    }
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {

    case IDC_CB_LOCLS:
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        this->selLocation(this->msgItem(IDC_CB_LOCLS, CB_GETCURSEL));
      }
      break;

    case IDC_LB_REPLS: //< Location(s) list List-Box
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        this->_onSelectRep();
      }
      if(HIWORD(wParam) == LBN_DBLCLK) {
        //...
      }
      break;

    case IDC_BC_INST:
      break;

    case IDC_BC_UNIN:
      break;

    case IDC_BC_ABORT:
      //this->_abortPending = true;
      this->enableItem(IDC_BC_ABORT, false);
      break;

    case IDC_BC_NEW:
      {
        OmUiAddRep* pUiNewRep = static_cast<OmUiAddRep*>(this->siblingById(IDD_ADD_REP));
        pUiNewRep->setLocation(pLoc);
        pUiNewRep->open(true);
      }
      break;

    case IDC_BC_CHK:
      break;

    case IDC_BC_DEL:
      this->_repoDel();
      break;

    }
  }

  return false;
}
