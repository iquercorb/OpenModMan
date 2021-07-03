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
#include "gui/OmUiAddLoc.h"
#include "gui/OmUiProgress.h"
#include "gui/OmUiPropCtx.h"
#include "gui/OmUiPropCtxLoc.h"
#include "gui/OmUiPropLoc.h"


/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the _delLoc_fth
/// thread finished his job.
///
#define UWM_BACKPURGE_DONE     (WM_APP+1)


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtxLoc::OmUiPropCtxLoc(HINSTANCE hins) : OmDialog(hins),
  _delLoc_hth(nullptr),
  _delLoc_id(-1)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i) {
    this->_chParam[i] = false;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtxLoc::~OmUiPropCtxLoc()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropCtxLoc::id() const
{
  return IDD_PROP_CTX_LOC;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::setChParam(unsigned i, bool en)
{
  this->_chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_delLoc_init(int id)
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  // store Location id
  this->_delLoc_id = id;
  OmLocation* pLoc = pCtx->locGet(id);

  // To prevent crash during operation we unselect location in the main dialog
  static_cast<OmUiMain*>(this->root())->safemode(true);

  // unselect location
  pCtx->locSel(-1);

  // if Location does not have backup data, we can bypass the purge and
  // do directly to the end
  if(pLoc->bckHasData()) {

    OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS));

    pUiProgress->open(true);
    pUiProgress->setCaption(L"Delete Target location");
    pUiProgress->setScHeadText(L"Restoring all backup data");

    DWORD dwId;
    this->_delLoc_hth = CreateThread(nullptr, 0, this->_delLoc_fth, this, 0, &dwId);

  } else {

    // directly delete the location
    this->_delLoc_stop();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_delLoc_stop()
{
  DWORD exitCode;

  if(this->_delLoc_hth) {
    WaitForSingleObject(this->_delLoc_hth, INFINITE);
    GetExitCodeThread(this->_delLoc_hth, &exitCode);
    CloseHandle(this->_delLoc_hth);
    this->_delLoc_hth = nullptr;
  }

  // quit the progress dialog
  static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS))->quit();

  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  wstring msg;

  // check whether purge succeed
  if(exitCode == 0) {

    // backup data purged, now delete Location
    if(!pCtx->locRem(this->_delLoc_id)) {
      msg = L"Errors occurred during Target location deletion process, "
            L"read debug log for more details.";
      Om_dialogBoxWarn(this->_hwnd, L"Target location deletion error", msg);
    }

  } else {
    // an error occurred during backup purge
    msg = L"Target location deletion aborted because errors occurred during "
          L"backup purge process, read debug log for more details.";
    Om_dialogBoxErr(this->_hwnd, L"Target location backup purge error", msg);
  }

  // select the first location in list
  pCtx->locSel(0);

  // Back to main dialog window to normal state
  static_cast<OmUiMain*>(this->root())->safemode(false);

  // refresh all dialogs from root
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropCtxLoc::_delLoc_fth(void* arg)
{
  OmUiPropCtxLoc* self = static_cast<OmUiPropCtxLoc*>(arg);

  OmContext* pCtx = static_cast<OmUiPropCtx*>(self->_parent)->ctxCur();
  if(!pCtx) return 1;
  OmLocation* pLoc = pCtx->locGet(self->_delLoc_id);
  if(!pLoc) return 1;

  DWORD exitCode = 0;

  if(!pLoc->bckPurge(&self->_delLoc_progress_cb, self->siblingById(IDD_PROGRESS))) {
    exitCode = 1; //< report error
  }

  // sends message to window to inform process ended
  PostMessage(self->_hwnd, UWM_BACKPURGE_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtxLoc::_delLoc_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t data)
{
  OmUiProgress* pUiProgress = reinterpret_cast<OmUiProgress*>(ptr);

  if(data) {
    pUiProgress->setScItemText(reinterpret_cast<wchar_t*>(data));
  }
  pUiProgress->setPbRange(0, tot);
  pUiProgress->setPbPos(cur);

  return !pUiProgress->abortGet();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onLbLoclsSel()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  int lb_sel = this->msgItem(IDC_LB_LOC, LB_GETCURSEL);
  int loc_id = this->msgItem(IDC_LB_LOC, LB_GETITEMDATA, lb_sel);

  if(loc_id >= 0) {

    OmLocation* pLoc = pCtx->locGet(loc_id);

    this->setItemText(IDC_EC_OUT01, pLoc->dstDir());
    this->setItemText(IDC_EC_OUT02, pLoc->libDir());
    this->setItemText(IDC_EC_OUT03, pLoc->bckDir());

    this->enableItem(IDC_SC_LBL02, true);
    this->enableItem(IDC_EC_OUT01, true);
    this->enableItem(IDC_SC_LBL03, true);
    this->enableItem(IDC_EC_OUT02, true);
    this->enableItem(IDC_SC_LBL04, true);
    this->enableItem(IDC_EC_OUT03, true);

    this->enableItem(IDC_BC_DEL, true);
    this->enableItem(IDC_BC_EDI, true);

    this->enableItem(IDC_BC_UP, (lb_sel > 0));
    int lb_max = this->msgItem(IDC_LB_LOC, LB_GETCOUNT) - 1;
    this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
  } else {
    this->setItemText(IDC_EC_OUT01, L"<no selection>");
    this->setItemText(IDC_EC_OUT02, L"<no selection>");
    this->setItemText(IDC_EC_OUT03, L"<no selection>");

    this->enableItem(IDC_SC_LBL02, false);
    this->enableItem(IDC_EC_OUT01, false);
    this->enableItem(IDC_SC_LBL03, false);
    this->enableItem(IDC_EC_OUT02, false);
    this->enableItem(IDC_SC_LBL04, false);
    this->enableItem(IDC_EC_OUT03, false);

    this->enableItem(IDC_BC_DEL, false);
    this->enableItem(IDC_BC_EDI, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onBcUpLoc()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_LOC, LB_GETCURSEL);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_LOC, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_LOC, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_LOC, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_LOC, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_LOC, LB_SETITEMDATA, lb_sel, idx);

  this->enableItem(IDC_BC_UP, (lb_sel > 1));
  this->enableItem(IDC_BC_DN, true);

  // user modified parameter, notify it
  this->setChParam(CTX_PROP_LOC_ORDER, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onBcDnLoc()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_LOC, LB_GETCURSEL);
  // get count of item in List-Box as index to for insertion
  int lb_max = this->msgItem(IDC_LB_LOC, LB_GETCOUNT) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  this->msgItem(IDC_LB_LOC, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_LOC, LB_GETITEMDATA, lb_sel);
  this->msgItem(IDC_LB_LOC, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_LOC, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_LOC, LB_SETITEMDATA, lb_sel, idx);
  this->msgItem(IDC_LB_LOC, LB_SETCURSEL, lb_sel);

  this->enableItem(IDC_BC_UP, true);
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));

  // user modified parameter, notify it
  this->setChParam(CTX_PROP_LOC_ORDER, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onBcDelLoc()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  int lb_sel = this->msgItem(IDC_LB_LOC, LB_GETCURSEL);
  int loc_id = this->msgItem(IDC_LB_LOC, LB_GETITEMDATA, lb_sel);

  if(loc_id < 0) return;

  OmLocation* pLoc = pCtx->locGet(loc_id);

  // warns the user before committing the irreparable
  wstring msg;
  msg =   L"The operation will permanently delete "
          L"the Target location and its associated data.";
  msg +=  L"\n\nDelete the Location \"" + pLoc->title() + L"\" ?";

  if(!Om_dialogBoxQuerry(this->_hwnd, L"Target location deletion", msg))
    return;

  if(pLoc->bckHasData()) {
    msg =   L"The Target location currently have installed packages, the "
            L"deletion process will uninstall them and restore all backup data.";
    msg +=  L"\n\nContinue ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Target location deletion", msg))
      return;
  }

  // here we go for Location delete
  this->_delLoc_init(loc_id);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onBcEdiLoc()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  int lb_sel = this->msgItem(IDC_LB_LOC, LB_GETCURSEL);
  int loc_id = this->msgItem(IDC_LB_LOC, LB_GETITEMDATA, lb_sel);

  if(loc_id >= 0) {
    // open the Location Properties dialog
    OmUiPropLoc* pUiPropLoc = static_cast<OmUiPropLoc*>(this->siblingById(IDD_PROP_LOC));
    pUiPropLoc->locSet(pCtx->locGet(loc_id));
    pUiPropLoc->open();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onBcAddLoc()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  // open add Location dialog
  OmUiAddLoc* pUiNewLoc = static_cast<OmUiAddLoc*>(this->siblingById(IDD_ADD_LOC));
  pUiNewLoc->ctxSet(pCtx);
  pUiNewLoc->open(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onInit()
{
  // Set buttons inner icons
  this->setBmIcon(IDC_BC_ADD, Om_getResIcon(this->_hins, IDB_BTN_ADD));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDB_BTN_REM));
  this->setBmIcon(IDC_BC_EDI, Om_getResIcon(this->_hins, IDB_BTN_MOD));
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(this->_hins, IDB_BTN_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(this->_hins, IDB_BTN_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_LB_LOC,  L"Target locations list");
  this->_createTooltip(IDC_BC_UP,   L"Move up in list");
  this->_createTooltip(IDC_BC_DN,   L"Move down in list");
  this->_createTooltip(IDC_BC_DEL,  L"Delete Target location and its associated data");
  this->_createTooltip(IDC_BC_ADD,  L"Configure a new Target location for this Context");
  this->_createTooltip(IDC_BC_EDI,  L"Modify Target location properties");

  // Update values
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onResize()
{
  // Locations list Label & ListBox
  this->_setItemPos(IDC_SC_LBL01, 50, 15, 64, 9);
  this->_setItemPos(IDC_LB_LOC, 50, 25, this->width()-107, 30);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->width()-55, 25, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->width()-55, 40, 16, 15);
  // Remove & Modify Buttons
  this->_setItemPos(IDC_BC_DEL, 50, 57, 50, 14);
  this->_setItemPos(IDC_BC_EDI, 105, 57, 50, 14);
  // Add button
  this->_setItemPos(IDC_BC_ADD, this->width()-108, 57, 50, 14);
  // Location Destination Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 50, 75, 110, 9);
  this->_setItemPos(IDC_EC_OUT01, 115, 75, this->width()-155, 13);
  // Location Library Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 50, 87, 110, 9);
  this->_setItemPos(IDC_EC_OUT02, 115, 87, this->width()-155, 13);
  // Location Backup Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 50, 99, 110, 9);
  this->_setItemPos(IDC_EC_OUT03, 115, 99, this->width()-155, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onRefresh()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  this->msgItem(IDC_LB_LOC, LB_RESETCONTENT);

  for(unsigned i = 0; i < pCtx->locCount(); ++i) {
    this->msgItem(IDC_LB_LOC, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(pCtx->locGet(i)->title().c_str()));
    this->msgItem(IDC_LB_LOC, LB_SETITEMDATA, i, i); // for Location index reordering
  }

  // Set controls default states and parameters
  this->setItemText(IDC_EC_OUT01, L"<no selection>");
  this->setItemText(IDC_EC_OUT02, L"<no selection>");
  this->setItemText(IDC_EC_OUT03, L"<no selection>");

  this->enableItem(IDC_SC_LBL02, false);
  this->enableItem(IDC_EC_OUT01, false);
  this->enableItem(IDC_SC_LBL03, false);
  this->enableItem(IDC_EC_OUT02, false);
  this->enableItem(IDC_SC_LBL04, false);
  this->enableItem(IDC_EC_OUT03, false);

  this->enableItem(IDC_BC_DEL,  false);
  this->enableItem(IDC_BC_EDI, false);

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtxLoc::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_BACKPURGE_DONE is a custom message sent from Location backups purge thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_BACKPURGE_DONE) {
    // end the removing Location process
    this->_delLoc_stop();
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_LOC: //< Location(s) list List-Box
      this->_onLbLoclsSel();
      break;

    case IDC_BC_UP: //< Up Buttn
      this->_onBcUpLoc();
      break;

    case IDC_BC_DN: //< Down Buttn
      this->_onBcDnLoc();
      break;

    case IDC_BC_DEL: //< "Remove" Button
      this->_onBcDelLoc();
      break;

    case IDC_BC_EDI: //< "Modify" Button
      this->_onBcEdiLoc();
      break;

    case IDC_BC_ADD: //< "New" Button
      this->_onBcAddLoc();
      break;
    }
  }

  return false;
}
