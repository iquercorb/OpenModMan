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
/// Custom window message to notify the dialog window that the _backupPurge_fth
/// thread finished his job.
///
#define UWM_BACKPURGE_DONE     (WM_APP+1)


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtxLoc::OmUiPropCtxLoc(HINSTANCE hins) : OmDialog(hins),
  _hBmBcNew(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_ADD), IMAGE_BITMAP, 0, 0, 0))),
  _hBmBcDel(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_REM), IMAGE_BITMAP, 0, 0, 0))),
  _hBmBcMod(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_MOD), IMAGE_BITMAP, 0, 0, 0))),
  _hBmBcUp(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_UP), IMAGE_BITMAP, 0, 0, 0))),
  _hBmBcDn(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_DN), IMAGE_BITMAP, 0, 0, 0))),
  _backupPurge_hth(nullptr)
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
  DeleteObject(this->_hBmBcNew);
  DeleteObject(this->_hBmBcDel);
  DeleteObject(this->_hBmBcMod);
  DeleteObject(this->_hBmBcUp);
  DeleteObject(this->_hBmBcDn);
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
void OmUiPropCtxLoc::_locationUp()
{
  HWND hLb = this->getItem(IDC_LB_LOCLS);

  // get selected item (index)
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  // retrieve the package List-Box label
  SendMessageW(hLb, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  idx = SendMessageW(hLb, LB_GETITEMDATA, lb_sel - 1, 0);

  SendMessageW(hLb, LB_DELETESTRING, lb_sel - 1, 0);

  SendMessageW(hLb, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  SendMessageW(hLb, LB_SETITEMDATA, lb_sel, idx);

  this->enableItem(IDC_BC_UP, (lb_sel > 1));
  this->enableItem(IDC_BC_DN, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_locationDn()
{
  HWND hLb = this->getItem(IDC_LB_LOCLS);

  // get selected item (index)
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);
  // get count of item in List-Box as index to for insertion
  int lb_max = SendMessageW(hLb, LB_GETCOUNT, 0, 0) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  SendMessageW(hLb, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  idx = SendMessageW(hLb, LB_GETITEMDATA, lb_sel, 0);
  SendMessageW(hLb, LB_DELETESTRING, lb_sel, 0);

  lb_sel++;

  SendMessageW(hLb, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  SendMessageW(hLb, LB_SETITEMDATA, lb_sel, idx);
  SendMessageW(hLb, LB_SETCURSEL, true, lb_sel);

  this->enableItem(IDC_BC_UP, true);
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_locationDel()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->context();

  if(pCtx == nullptr)
    return;

  int lb_sel = this->msgItem(IDC_LB_LOCLS, LB_GETCURSEL);
  int loc_id = this->msgItem(IDC_LB_LOCLS, LB_GETITEMDATA, lb_sel);

  if(loc_id >= 0) {

    OmLocation* pLoc = pCtx->location(loc_id);

    // check whether we need to purge backup data
    if(!pLoc->hasBackupData()) {

      // To prevent crash during operation we unselect location in the main dialog
      static_cast<OmUiMain*>(this->root())->setSafeEdit(true);

      // no backup to purge, delete Location now
      if(!pCtx->remLocation(loc_id)) {

        wstring wrn = L"One or more error occurred during Location deletion "
                      L"deletion process.\n\n"
                      L"Please read debug log output for more details.";

        Om_dialogBoxWarn(this->_hwnd, L"Delete Location error", wrn);
      }

      // refresh the dialog to reflect changes
      this->refresh();

      // To prevent crash during operation we unselect location in the main dialog
      static_cast<OmUiMain*>(this->root())->setSafeEdit(false);

    } else {

      // we need to purge backups data before delete Location
      this->_backupPurge_init();
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_backupPurge_init()
{
  // To prevent crash during operation we unselect location in the main dialog
  static_cast<OmUiMain*>(this->root())->setSafeEdit(true);

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS));

  pUiProgress->open(true);
  pUiProgress->setTitle(L"Purge Location backups data");
  pUiProgress->setDesc(L"Backups data restoration");

  DWORD dwId;
  this->_backupPurge_hth = CreateThread(nullptr, 0, this->_backupPurge_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_backupPurge_stop()
{
  DWORD exitCode;

  if(this->_backupPurge_hth) {
    WaitForSingleObject(this->_backupPurge_hth, INFINITE);
    GetExitCodeThread(this->_backupPurge_hth, &exitCode);
    CloseHandle(this->_backupPurge_hth);
    this->_backupPurge_hth = nullptr;
  }

  // quit the progress dialog
  static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS))->quit();

  // check whether purge succeed
  if(exitCode == 0) {
    // backup data purged, now delete Location
    this->_locationDel();
  }

  // Back to main dialog window to normal state
  static_cast<OmUiMain*>(this->root())->setSafeEdit(false);

  // refresh this dialog
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropCtxLoc::_backupPurge_fth(void* arg)
{
  OmUiPropCtxLoc* self = static_cast<OmUiPropCtxLoc*>(arg);

  OmContext* pCtx = static_cast<OmUiPropCtx*>(self->_parent)->context();

  if(pCtx == nullptr)
    return 1;

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(self->siblingById(IDD_PROGRESS));

  HWND hPb = pUiProgress->getPbHandle();
  HWND hSc = pUiProgress->getDetailScHandle();

  int lb_sel = self->msgItem(IDC_LB_LOCLS, LB_GETCURSEL);
  int loc_id = self->msgItem(IDC_LB_LOCLS, LB_GETITEMDATA, lb_sel);

  DWORD exitCode = 0;

  if(loc_id >= 0) {

    OmLocation* pLoc = pCtx->location(loc_id);

    // launch backups data purge process
    if(!pLoc->backupsPurge(hPb, hSc, pUiProgress->getAbortPtr())) {
      // we encounter error during backup data purge
      Om_dialogBoxErr(pUiProgress->hwnd(), L"Backups data purge error", pLoc->lastError());
      exitCode = 1;
    }
  }

  // sends message to window to inform process ended
  PostMessage(self->_hwnd, UWM_BACKPURGE_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onInit()
{
  // Set buttons inner icons
  this->msgItem(IDC_BC_ADD,   BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcNew));
  this->msgItem(IDC_BC_DEL,   BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcDel));
  this->msgItem(IDC_BC_EDI,  BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcMod));
  this->msgItem(IDC_BC_UP,    BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcUp));
  this->msgItem(IDC_BC_DN,    BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcDn));

  // define controls tool-tips
  this->_createTooltip(IDC_LB_LOCLS,  L"Context's locations");

  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");

  this->_createTooltip(IDC_BC_DEL,    L"Remove and purge location");
  this->_createTooltip(IDC_BC_ADD,    L"Add new location");
  this->_createTooltip(IDC_BC_EDI,   L"Location properties");

  this->enableItem(IDC_EC_INP02, false);
  this->enableItem(IDC_EC_INP03, false);
  this->enableItem(IDC_EC_INP04, false);

  // Update values
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onResize()
{
  // Locations list Label & ListBox
  this->_setItemPos(IDC_SC_LBL01, 5, 20, 64, 9);
  this->_setItemPos(IDC_LB_LOCLS, 70, 20, this->width()-107, 30);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->width()-35, 20, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->width()-35, 36, 16, 15);
  // Location Destination Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 71, 60, 40, 9);
  this->_setItemPos(IDC_EC_INP02, 115, 60, this->width()-125, 13);
  // Location Library Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 71, 75, 40, 9);
  this->_setItemPos(IDC_EC_INP03, 115, 75, this->width()-125, 13);
  // Location Backup Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 71, 90, 40, 9);
  this->_setItemPos(IDC_EC_INP04, 115, 90, this->width()-125, 13);
  // Remove & Modify Buttons
  this->_setItemPos(IDC_BC_DEL, 70, 110, 50, 14);
  this->_setItemPos(IDC_BC_EDI, 122, 110, 50, 14);
  // Add button
  this->_setItemPos(IDC_BC_ADD, this->width()-87, 110, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onRefresh()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->context();

  if(pCtx == nullptr)
    return;

  HWND hLb = this->getItem(IDC_LB_LOCLS);

  SendMessageW(hLb, LB_RESETCONTENT, 0, 0);
  if(pCtx) {
    for(unsigned i = 0; i < pCtx->locationCount(); ++i) {
      SendMessageW(hLb, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(pCtx->location(i)->title().c_str()));
      SendMessageW(hLb, LB_SETITEMDATA, i, i); // for Location index reordering
    }
  }

  // Set controls default states and parameters
  this->setItemText(IDC_EC_INP02, L"<no Location selected>");
  this->setItemText(IDC_EC_INP03, L"<no Location selected>");
  this->setItemText(IDC_EC_INP04, L"<no Location selected>");

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
    this->_backupPurge_stop();
  }

  if(uMsg == WM_COMMAND) {

    OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->context();

    if(pCtx == nullptr)
      return false;

    int lb_sel, loc_id;

    switch(LOWORD(wParam))
    {

    case IDC_LB_LOCLS: //< Location(s) list List-Box
      lb_sel = this->msgItem(IDC_LB_LOCLS, LB_GETCURSEL);
      loc_id = this->msgItem(IDC_LB_LOCLS, LB_GETITEMDATA, lb_sel);
      if(loc_id >= 0) {
        OmLocation* pLoc = pCtx->location(loc_id);
        this->setItemText(IDC_EC_INP02, pLoc->installDir());
        this->setItemText(IDC_EC_INP03, pLoc->libraryDir());
        this->setItemText(IDC_EC_INP04, pLoc->backupDir());
        this->enableItem(IDC_BC_DEL, true);
        this->enableItem(IDC_BC_EDI, true);
        this->enableItem(IDC_BC_UP, (lb_sel > 0));
        int lb_max = this->msgItem(IDC_LB_LOCLS, LB_GETCOUNT) - 1;
        this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
      }
      break;

    case IDC_BC_ADD: //< New button for Location(s) list
      {
        OmUiAddLoc* pUiNewLoc = static_cast<OmUiAddLoc*>(this->siblingById(IDD_ADD_LOC));
        pUiNewLoc->setContext(pCtx);
        pUiNewLoc->open(true);
      }
      break;

    case IDC_BC_EDI:
      lb_sel = this->msgItem(IDC_LB_LOCLS, LB_GETCURSEL);
      loc_id = this->msgItem(IDC_LB_LOCLS, LB_GETITEMDATA, lb_sel);
      if(loc_id >= 0) {
        // open the Location Properties dialog
        OmUiPropLoc* pUiPropLoc = static_cast<OmUiPropLoc*>(this->siblingById(IDD_PROP_LOC));
        pUiPropLoc->setLocation(pCtx->location(loc_id));
        pUiPropLoc->open();
      }
      break;

    case IDC_BC_DEL: //< Remove button for Location(s) list
      lb_sel = this->msgItem(IDC_LB_LOCLS, LB_GETCURSEL);
      loc_id = this->msgItem(IDC_LB_LOCLS, LB_GETITEMDATA, lb_sel);
      if(loc_id >= 0) {

        OmLocation* pLoc = pCtx->location(loc_id);

        // warns the user before committing the irreparable
        wstring wrn = L"The operation will permanently delete the Location "
                      L"definition file and related configuration.";

        wrn += L"\n\nDelete the Location \"";
        wrn += pLoc->title();
        wrn += L"\" ?";

        if(Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Location", wrn)) {
          // delete the Location
          this->_locationDel();
        }
      }
      break;


    case IDC_BC_UP:
      this->_locationUp();
      // user modified parameter, notify it
      this->setChParam(CTX_PROP_LOC_ORDER, true);
      break;

    case IDC_BC_DN:
      this->_locationDn();
      // user modified parameter, notify it
      this->setChParam(CTX_PROP_LOC_ORDER, true);
      break;
    }
  }

  return false;
}
