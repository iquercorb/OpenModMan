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
#include "gui/OmUiNewLoc.h"
#include "gui/OmUiProgress.h"
#include "gui/OmUiPropCtx.h"
#include "gui/OmUiPropCtxLoc.h"
#include "gui/OmUiPropLoc.h"


/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the remLocation_fth
/// thread finished his job.
///
#define UWM_REMLOCATION_DONE     (WM_APP+1)


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtxLoc::OmUiPropCtxLoc(HINSTANCE hins) : OmDialog(hins),
  _remLocation_hth(nullptr)
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
  reinterpret_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_locationUp()
{
  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_LOCLS);

  // get selected item (index)
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t buf[512];
  int idx;

  // retrieve the package List-Box label
  SendMessageW(hLb, LB_GETTEXT, lb_sel - 1, (LPARAM)buf);
  idx = SendMessageW(hLb, LB_GETITEMDATA, lb_sel - 1, 0);

  SendMessageW(hLb, LB_DELETESTRING, lb_sel - 1, 0);

  SendMessageW(hLb, LB_INSERTSTRING, lb_sel, (LPARAM)buf);
  SendMessageW(hLb, LB_SETITEMDATA, lb_sel, (LPARAM)idx);

  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), (lb_sel > 1));
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_locationDn()
{
  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_LOCLS);

  // get selected item (index)
  int lb_sel = SendMessageW(hLb, LB_GETCURSEL, 0, 0);
  // get count of item in List-Box as index to for insertion
  int lb_max = SendMessageW(hLb, LB_GETCOUNT, 0, 0) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t buf[512];
  int idx;

  SendMessageW(hLb, LB_GETTEXT, lb_sel, (LPARAM)buf);
  idx = SendMessageW(hLb, LB_GETITEMDATA, lb_sel, 0);
  SendMessageW(hLb, LB_DELETESTRING, lb_sel, 0);

  lb_sel++;

  SendMessageW(hLb, LB_INSERTSTRING, lb_sel, (LPARAM)buf);
  SendMessageW(hLb, LB_SETITEMDATA, lb_sel, (LPARAM)idx);
  SendMessageW(hLb, LB_SETCURSEL , true, (LPARAM)(lb_sel));

  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), true);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), (lb_sel < lb_max));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_remLocation_init()
{
  // To prevent crash during operation we unselect location in the main dialog
  reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(true);

  OmUiProgress* uiProgress = reinterpret_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS));

  uiProgress->open(true);
  uiProgress->setCaption(L"Location cleaning");
  uiProgress->setTitle(L"Cleaning Location backups data...");

  DWORD dwId;
  this->_remLocation_hth = CreateThread(nullptr, 0, this->_remLocation_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_remLocation_stop()
{
  if(this->_remLocation_hth) {
    WaitForSingleObject(this->_remLocation_hth, INFINITE);
    CloseHandle(this->_remLocation_hth);
    this->_remLocation_hth = nullptr;
  }

  // Back to main dialog window to normal state
  reinterpret_cast<OmUiMain*>(this->root())->setSafeEdit(false);

  // Close progress dialog
  reinterpret_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS))->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropCtxLoc::_remLocation_fth(void* arg)
{
  OmUiPropCtxLoc* self = reinterpret_cast<OmUiPropCtxLoc*>(arg);

  OmContext* context = reinterpret_cast<OmUiPropCtx*>(self->_parent)->context();

  if(context == nullptr)
    return 0;

  OmUiProgress* uiProgress = reinterpret_cast<OmUiProgress*>(self->siblingById(IDD_PROGRESS));

  HWND hPb = (HWND)uiProgress->getProgressBar();
  HWND hSc = (HWND)uiProgress->getStaticComment();

  int lb_sel = SendMessageW(GetDlgItem(self->_hwnd, IDC_LB_LOCLS), LB_GETCURSEL, 0, 0);
  int loc_id = SendMessageW(GetDlgItem(self->_hwnd, IDC_LB_LOCLS), LB_GETITEMDATA, lb_sel, 0);

  if(!context->purgeLocation(loc_id, uiProgress->hwnd(), hPb, hSc, uiProgress->getAbortPtr())) {

    wstring str = L"An error occurred during Location deletion:\n";
    str += context->lastError();

    Om_dialogBoxErr(uiProgress->hwnd(), L"Location deletion error", str);
  }

  PostMessage(self->_hwnd, UWM_REMLOCATION_DONE, 0, 0);

  return 0;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onShow()
{
  // define controls tool-tips
  this->_createTooltip(IDC_LB_LOCLS,  L"Context's locations");

  this->_createTooltip(IDC_BC_UP,     L"Move up");
  this->_createTooltip(IDC_BC_DN,     L"Move down");

  this->_createTooltip(IDC_BC_DEL,    L"Remove and purge location");
  this->_createTooltip(IDC_BC_ADD,    L"Add new location");
  this->_createTooltip(IDC_BC_EDIT,   L"Location properties");


  HBITMAP hBm;

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_ADD), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_ADD), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_REM), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_DEL), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_MOD), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_EDIT), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_UP), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_UP), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  hBm = (HBITMAP)LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_DN), IMAGE_BITMAP, 0, 0, 0);
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_DN), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBm);

  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onResize()
{
  // Locations list Label & ListBox
  this->_setControlPos(IDC_SC_LBL01, 5, 20, 64, 9);
  this->_setControlPos(IDC_LB_LOCLS, 70, 20, this->width()-107, 30);
  // Up and Down buttons
  this->_setControlPos(IDC_BC_UP, this->width()-35, 20, 16, 15);
  this->_setControlPos(IDC_BC_DN, this->width()-35, 36, 16, 15);
  // Location Destination Label & EditControl
  this->_setControlPos(IDC_SC_LBL02, 71, 60, 40, 9);
  this->_setControlPos(IDC_EC_INPT2, 115, 60, this->width()-125, 13);
  // Location Library Label & EditControl
  this->_setControlPos(IDC_SC_LBL03, 71, 75, 40, 9);
  this->_setControlPos(IDC_EC_INPT3, 115, 75, this->width()-125, 13);
  // Location Backup Label & EditControl
  this->_setControlPos(IDC_SC_LBL04, 71, 90, 40, 9);
  this->_setControlPos(IDC_EC_INPT4, 115, 90, this->width()-125, 13);
  // Remove & Modify Buttons
  this->_setControlPos(IDC_BC_DEL, 70, 110, 50, 14);
  this->_setControlPos(IDC_BC_EDIT, 122, 110, 50, 14);
  // Add button
  this->_setControlPos(IDC_BC_ADD, this->width()-70, 110, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onRefresh()
{
  SetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, L"<no Location selected>");
  SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, L"<no Location selected>");
  SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, L"<no Location selected>");

  EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT2), false);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), false);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT4), false);

  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_EDIT), false);

  OmContext* context = reinterpret_cast<OmUiPropCtx*>(this->_parent)->context();

  if(context == nullptr)
    return;

  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_LOCLS);

  SendMessage(hLb, LB_RESETCONTENT, 0, 0);
  if(context) {
    for(unsigned i = 0; i < context->locationCount(); ++i) {
      SendMessageW(hLb, LB_ADDSTRING, i, (LPARAM)context->location(i)->title().c_str());
      SendMessageW(hLb, LB_SETITEMDATA, i, i); // for Location index reordering
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxLoc::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtxLoc::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_REMLOCATION_DONE is a custom message sent from Location deletion thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_REMLOCATION_DONE) {
    // end the removing Location process
    this->_remLocation_stop();
    // refresh the main window dialog, this will also refresh this one
    this->root()->refresh();
  }

  if(uMsg == WM_COMMAND) {

    OmContext* context = reinterpret_cast<OmUiPropCtx*>(this->_parent)->context();

    if(context == nullptr)
      return false;

    int lb_sel, lb_max;

    switch(LOWORD(wParam))
    {

    case IDC_LB_LOCLS: //< Location(s) list List-Box
      lb_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_LOCLS), LB_GETCURSEL, 0, 0);
      if(lb_sel >= 0) {
        int loc_id = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_LOCLS), LB_GETITEMDATA, lb_sel, 0);
        OmLocation* location = context->location(loc_id);
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, location->installDir().c_str());
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, location->libraryDir().c_str());
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, location->backupDir().c_str());
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_EDIT), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_UP), (lb_sel > 0));
        lb_max = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_LOCLS), LB_GETCOUNT, 0, 0) - 1;
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DN), (lb_sel < lb_max));
      }
      break;

    case IDC_BC_ADD: //< New button for Location(s) list
      {
        OmUiNewLoc* uiNewLoc = reinterpret_cast<OmUiNewLoc*>(this->siblingById(IDD_NEW_LOC));
        uiNewLoc->setContext(context);
        uiNewLoc->open(true);
      }
      break;

    case IDC_BC_EDIT:
      lb_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_LOCLS), LB_GETCURSEL, 0, 0);
      if(lb_sel >= 0 && lb_sel < (int)context->locationCount()) {
        int loc_id = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_LOCLS), LB_GETITEMDATA, lb_sel, 0);
        // open the Location Properties dialog
        OmUiPropLoc* uiPropLoc = reinterpret_cast<OmUiPropLoc*>(this->siblingById(IDD_PROP_LOC));
        uiPropLoc->setLocation(context->location(loc_id));
        uiPropLoc->open();
      }
      break;

    case IDC_BC_DEL: //< Remove button for Location(s) list
      lb_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_LOCLS), LB_GETCURSEL, 0, 0);
      if(lb_sel >= 0) {
        int loc_id = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_LOCLS), LB_GETITEMDATA, lb_sel, 0);
        // warns the user before committing the irreparable
        wstring msg;
        msg = L"The operation will permanently delete the Location "
              L"definition file and related configuration.";

        msg += L"\n\nDelete the Location '";
        msg += context->location(loc_id)->title();
        msg += L"' ?";

        if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Location", msg)) {
          return false;
        }
        // launch the removing Location process
        this->_remLocation_init();
      }
      break;


    case IDC_BC_UP:
      this->_locationUp();
      // user modified parameter, notify it
      this->setChParam(CTX_PROP_LOC_, true);
      break;

    case IDC_BC_DN:
      this->_locationDn();
      // user modified parameter, notify it
      this->setChParam(CTX_PROP_LOC_, true);
      break;
    }
  }

  return false;
}
