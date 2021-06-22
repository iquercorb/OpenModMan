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
OmUiMainNet::OmUiMainNet(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainNet::~OmUiMainNet()
{
  HFONT hFt;
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_SC_TITLE, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_PKTXT, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
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
void OmUiMainNet::freeze(bool freeze)
{
  // TODO

  this->enableItem(IDC_BC_ABORT, freeze);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::safemode(bool enable)
{
  // TODO
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::locSel(int i)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // main window dialog
  OmUiMain* pUiMain = static_cast<OmUiMain*>(this->_parent);

  // disable "Edit > Package []" in main menu
  pUiMain->setPopupItem(1, 5, MF_GRAYED);

  // select the requested Location
  if(pCtx) {

    pCtx->locSel(i);

    OmLocation* pLoc = pCtx->locCur();

    if(pLoc) {

      // Check Location Destination folder access
      if(!pLoc->checkAccessDst()) {

        wstring wrn = L"Configured Location's destination folder \"";
        wrn += pLoc->dstDir()+L"\""; wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";

        Om_dialogBoxWarn(this->_hwnd, L"Destination folder access error", wrn);
      }

      // Check Location Backup folder access
      if(!pLoc->checkAccessBck()) {

        wstring wrn = L"Configured Location's backup folder \"";
        wrn += pLoc->bckDir()+L"\""; wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";

        Om_dialogBoxWarn(this->_hwnd, L"Backup folder access error", wrn);
      }

      // Check Location Library folder access
      if(!pLoc->checkAccessLib()) {

        wstring wrn = L"Configured Location's library folder \"";
        wrn += pLoc->bckDir()+L"\""; wrn += OMM_STR_ERR_DIRACCESS;
        wrn += L"\n\nPlease check Location's settings and folder permissions.";

        Om_dialogBoxWarn(this->_hwnd, L"Library folder access error", wrn);
      }

      // enable the "Edit > Location properties..." menu
      pUiMain->setPopupItem(1, 2, MF_ENABLED);

    } else {

      // disable the "Edit > Location properties..." menu
      pUiMain->setPopupItem(1, 2, MF_GRAYED);
    }
  }

  // refresh
  this->_buildLbRep();
  //this->_buildLvPkg();

  // forces control to select item
  this->msgItem(IDC_CB_LOC, CB_SETCURSEL, i);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_buildCbLoc()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();

  // check whether any context is selected
  if(!pCtx) {
    // empty the Combo-Box
    this->msgItem(IDC_CB_LOC, CB_RESETCONTENT);
    // disable Location ComboBox
    this->enableItem(IDC_CB_LOC, false);
    // force to reset current selection
    this->locSel(-1);
    // return now
    return;
  }

  // save current selection
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);

  // empty the Combo-Box
  this->msgItem(IDC_CB_LOC, CB_RESETCONTENT);

  // add Context(s) to Combo-Box
  if(pCtx->locCount()) {

    wstring label;

    for(unsigned i = 0; i < pCtx->locCount(); ++i) {

      // compose Location label
      label = pCtx->locGet(i)->title() + L" - ";

      if(pCtx->locGet(i)->checkAccessDst()) {
        label += pCtx->locGet(i)->dstDir();
      } else {
        label += L"<folder access error>";
      }

      this->msgItem(IDC_CB_LOC, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(label.c_str()));
    }

    // select the the previously selected Context
    if(cb_sel < 0) {
      // select the first Location by default
      this->locSel(0);
    } else {
      this->msgItem(IDC_CB_LOC, CB_SETCURSEL, cb_sel);
    }

    // enable the ComboBox control
    this->enableItem(IDC_CB_LOC, true);

  } else {

    // disable Location ComboBox
    this->enableItem(IDC_CB_LOC, false);
    // force to reset current selection
    this->locSel(-1);

    // ask user to create at least one Location in the Context
    wstring qry = L"The Context have not any configured "
                  L"Location, this does not make much sense."
                  L"\n\nDo you want to add a Location now ?";

    if(Om_dialogBoxQuerry(this->_hwnd, L"Context empty", qry)) {
      OmUiAddLoc* pUiAddLoc = static_cast<OmUiAddLoc*>(this->siblingById(IDD_ADD_LOC));
      pUiAddLoc->ctxSet(pCtx);
      pUiAddLoc->open(true);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_buildLbRep()
{
  // empty List-Box
  this->msgItem(IDC_LB_REP, LB_RESETCONTENT);

  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  if(pCtx->locCur()) {

    OmLocation* pLoc = pCtx->locCur();

    wstring label;
    OmRepository* pRep;

    for(unsigned i = 0; i < pLoc->repCount(); ++i) {
      pRep = pLoc->repGet(i);
      label = pRep->base() + L" - " + pRep->name();
      this->msgItem(IDC_LB_REP, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(label.c_str()));
    }

    this->enableItem(IDC_BC_CHK, (pLoc->repCount() > 0));
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onCbLocSel()
{
  int cb_sel = this->msgItem(IDC_CB_LOC, CB_GETCURSEL);
  this->locSel(cb_sel);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onLbRepSel()
{
  int lb_sel = this->msgItem(IDC_LB_BAT, LB_GETCURSEL);

  this->enableItem(IDC_BC_DEL, (lb_sel >= 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onBcNewRep()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  if(pCtx->locCur()) {
    OmUiAddRep* pUiNewRep = static_cast<OmUiAddRep*>(this->siblingById(IDD_ADD_REP));
    pUiNewRep->locSet(pCtx->locCur());
    pUiNewRep->open(true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onBcDelRep()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmContext* pCtx = pMgr->ctxCur();
  if(!pCtx) return;

  OmLocation* pLoc = pCtx->locCur();
  if(!pLoc) return;

  int lb_sel = this->msgItem(IDC_LB_REP, LB_GETCURSEL);
  if(lb_sel < 0) return;

  OmRepository* pRep = pLoc->repGet(lb_sel);

  // warns the user before committing the irreparable
  wstring qry = L"Are your sure you want to delete the Repository \"";
  qry += pRep->base()+L" - "+pRep->name()+L"\" ?";

  if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Repository", qry))
    return;

  pLoc->repRem(lb_sel);

  // reload the repository ListBox
  this->_buildLbRep();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onInit()
{
  // Defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(18, 800, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(14, 700, L"Consolas");
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  // Set batches New and Delete buttons icons
  this->setBmImage(IDC_BC_NEW, Om_getResImage(this->_hins, IDB_BTN_ADD));
  this->setBmImage(IDC_BC_DEL, Om_getResImage(this->_hins, IDB_BTN_REM));
  this->setBmImage(IDC_BC_CHK, Om_getResImage(this->_hins, IDB_BTN_REF));


  // define controls tool-tips
  this->_createTooltip(IDC_CB_LOC,  L"Select active location");
  this->_createTooltip(IDC_BC_UPGD,   L"Upgrade selected package(s)");
  this->_createTooltip(IDC_BC_SYNC,   L"Synchronize selected package(s)");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort current process");

  // Initialize the ListView control
  DWORD dwExStyle = LVS_EX_FULLROWSELECT|
                    LVS_EX_SUBITEMIMAGES|
                    LVS_EX_DOUBLEBUFFER;

  this->msgItem(IDC_LV_PKG, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwExStyle);

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
  this->msgItem(IDC_LV_PKG, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Name");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 550;
  lvCol.iSubItem = 1;
  this->msgItem(IDC_LV_PKG, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Version");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem = 2;
  this->msgItem(IDC_LV_PKG, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&lvCol));

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
  pUiMain->setPopupItem(1, 5, MF_GRAYED);

  // select location according current ComboBox selection
  this->locSel(this->msgItem(IDC_CB_LOC, CB_GETCURSEL));

  // refresh dialog
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainNet::_onResize()
{
  // Locations Combo-Box
  this->_setItemPos(IDC_CB_LOC, 5, 5, this->width()-10, 12);

  // Repositories label
  this->_setItemPos(IDC_SC_LBL01, 5, 24, 50, 12);
  // Repositories ProgressBar
  this->_setItemPos(IDC_PB_REP, 55, 22, this->width()-115, 11);
  this->_setItemPos(IDC_BC_CHK, this->width()-55, 20, 50, 14);
  // Repositories ListBox
  this->_setItemPos(IDC_LB_REP, 5, 37, this->width()-65, 29);
  // Repositories Apply, New.. and Delete buttons
  this->_setItemPos(IDC_BC_NEW, this->width()-55, 36, 50, 14);
  this->_setItemPos(IDC_BC_DEL, this->width()-55, 52, 50, 14);

  // Horizontal separator
  this->_setItemPos(IDC_SC_SEPAR, 5, 70, this->width()-10, 1);

  // Package List ListView
  this->_setItemPos(IDC_LV_PKG, 5, 75, this->width()-10, this->height()-191);
  // Resize the ListView column
  LONG size[4];
  GetClientRect(this->getItem(IDC_LV_PKG), reinterpret_cast<LPRECT>(&size));
  this->msgItem(IDC_LV_PKG, LVM_SETCOLUMNWIDTH, 1, size[2]-125);

  // Upgrade and Sync buttons
  this->_setItemPos(IDC_BC_UPGD, 5, this->height()-114, 50, 14);
  this->_setItemPos(IDC_BC_SYNC, 55, this->height()-114, 50, 14);
  // Progress bar
  this->_setItemPos(IDC_PB_PKG, 107, this->height()-113, this->width()-165, 12);
  // Abort button
  this->_setItemPos(IDC_BC_ABORT, this->width()-55, this->height()-114, 50, 14);

  // Package name/title
  this->_setItemPos(IDC_SC_TITLE, 5, this->height()-96, this->width()-161, 12);
  // Package snapshot
  this->_setItemPos(IDC_SB_PKG, 5, this->height()-83, 85, 78);
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
  ShowWindow(this->getItem(IDC_SB_PKG), false);

  // disable the Progress-Bar
  this->enableItem(IDC_PB_PKG, false);

  this->_buildCbLoc(); //< reload Location Combo-Box

  // if icon size changed, rebuild Package ListView
  //if(this->_buildLvPkg_icSize != pMgr->iconsSize()) {
    //this->_buildLvPkg();
  //}

  // disable all batches buttons
  this->enableItem(IDC_BC_NEW, (pMgr->ctxCur() != nullptr));
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
  // UWM_MAIN_CTX_CHANGED is a custom message sent from Main (parent) Dialog
  // to notify its child tab dialogs the Context selection changed.
  if(uMsg == UWM_MAIN_CTX_CHANGED) {
    // invalidate Location selection
    this->msgItem(IDC_CB_LOC, CB_SETCURSEL, -1);
    // Refresh the dialog
    this->_onRefresh();
    return false;
  }

  if(uMsg == WM_NOTIFY) {

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->ctxCur();

    if(!pCtx->locCur())
      return false;

    NMHDR* pNmhdr = reinterpret_cast<NMHDR*>(lParam);

    if(LOWORD(wParam) == IDC_LV_PKG) {

      // if thread is running we block all interaction
      /*
      if(this->_pkgInst_hth || this->_pkgUnin_hth)
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
          //pLoc->libSetSorting(PKG_SORTING_STAT);
          break;
        case 2:
          //pLoc->libSetSorting(PKG_SORTING_VERS);
          break;
        default:
          //pLoc->libSetSorting(PKG_SORTING_NAME);
          break;
        }
        this->refresh();
        return false;
      }
    }
  }

  if(uMsg == WM_COMMAND) {

    OmManager* pMgr = static_cast<OmManager*>(this->_data);
    OmContext* pCtx = pMgr->ctxCur();

    if(!pCtx->locCur())
      return false;

    switch(LOWORD(wParam))
    {

    case IDC_CB_LOC:
      if(HIWORD(wParam) == CBN_SELCHANGE) this->_onCbLocSel();
      break;

    case IDC_LB_REP: //< Location(s) list List-Box
      if(HIWORD(wParam) == LBN_SELCHANGE) this->_onLbRepSel();
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
      this->_onBcNewRep();
      break;

    case IDC_BC_CHK:
      break;

    case IDC_BC_DEL:
      this->_onBcDelRep();
      break;

    }
  }

  return false;
}
