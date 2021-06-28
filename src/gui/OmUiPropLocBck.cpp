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
#include "gui/OmUiProgress.h"
#include "gui/OmUiPropLocBck.h"
#include "gui/OmUiPropLoc.h"

/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the backupDcard_fth
/// thread finished his job.
///
#define UWM_BACKDISCARD_DONE     (WM_APP+1)

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocBck::OmUiPropLocBck(HINSTANCE hins) : OmDialog(hins),
  _delBck_hth(nullptr)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i)
    this->_chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocBck::~OmUiPropLocBck()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropLocBck::id() const
{
  return IDD_PROP_LOC_BCK;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::setChParam(unsigned i, bool en)
{
  _chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_onCkBoxZip()
{
  bool bm_chk = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_SC_LBL01, bm_chk);
  this->enableItem(IDC_CB_LVL, bm_chk);

  this->setChParam(LOC_PROP_BCK_COMP_LEVEL, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_onBcDelBck()
{
  // warns the user before committing the irreparable
  wstring wrn = L"This will permanently delete all existing "
                L"backups data without restoring them (which should "
                "never be done except in emergency situation)."

                L"\n\nDiscard all backups data for this Location ?";

  if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Discard backups data", wrn))
    return;

  // Launch backup discard process
  this->_delBck_init();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_delBck_init()
{
  // To prevent crash during operation we unselect location in the main dialog
  static_cast<OmUiMain*>(this->root())->safemode(true);

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS));

  pUiProgress->open(true);
  pUiProgress->setCaption(L"Discard Location backups data");
  pUiProgress->setScHeadText(L"Backups data deletion");

  DWORD dwId;
  this->_delBck_hth = CreateThread(nullptr, 0, this->_delBck_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_delBck_stop()
{
  DWORD exitCode;

  if(this->_delBck_hth) {
    WaitForSingleObject(this->_delBck_hth, INFINITE);
    GetExitCodeThread(this->_delBck_hth, &exitCode);
    CloseHandle(this->_delBck_hth);
    this->_delBck_hth = nullptr;
  }

  // quit the progress dialog
  static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS))->quit();

  // Back to main dialog window to normal state
  static_cast<OmUiMain*>(this->root())->safemode(false);

  if(exitCode == 1) {
    wstring msg = L"Errors occurred during backup discard process, "
                  L"read debug log for more details.";
    Om_dialogBoxWarn(this->_hwnd, L"Backup discard error", msg);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropLocBck::_delBck_fth(void* arg)
{
  OmUiPropLocBck* self = static_cast<OmUiPropLocBck*>(arg);

  OmLocation* pLoc = static_cast<OmUiPropLoc*>(self->_parent)->locCur();

  if(pLoc == nullptr)
    return 1;

  DWORD exitCode = 0;

  // launch backups data deletion process
  if(!pLoc->bckDcard(&self->_delBck_progress_cb, self->siblingById(IDD_PROGRESS))) {
    exitCode = 1;
  }

  // sends message to window to inform process ended
  PostMessage(self->_hwnd, UWM_BACKDISCARD_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLocBck::_delBck_progress_cb(void* ptr, size_t tot, size_t cur, const wchar_t* str)
{
  OmUiProgress* pUiProgress = reinterpret_cast<OmUiProgress*>(ptr);

  if(str) {
    pUiProgress->setScItemText(str);
  }
  pUiProgress->setPbRange(0, tot);
  pUiProgress->setPbPos(cur);

  return !pUiProgress->abortGet();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CKBX1,  L"Store backup data as zip archives");
  this->_createTooltip(IDC_CB_LVL,  L"Compression level for backup zip files");

  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->locCur();

  // add items to Zip Level ComboBox
  this->msgItem(IDC_CB_LVL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None ( very fast )"));
  this->msgItem(IDC_CB_LVL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Low ( fast )"));
  this->msgItem(IDC_CB_LVL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal ( slow )"));
  this->msgItem(IDC_CB_LVL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Best ( very slow )"));

  if(!pLoc) return;

  int comp_levl = pLoc->bckZipLevel();

  if(comp_levl >= 0) {

    this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 1);

    switch(comp_levl)
    {
    case 1:
      this->msgItem(IDC_CB_LVL, CB_SETCURSEL, 1, 0);
      break;
    case 2:
      this->msgItem(IDC_CB_LVL, CB_SETCURSEL, 2, 0);
      break;
    case 3:
      this->msgItem(IDC_CB_LVL, CB_SETCURSEL, 3, 0);
      break;
    default:
      this->msgItem(IDC_CB_LVL, CB_SETCURSEL, 0, 0);
      break;
    }

    this->enableItem(IDC_CB_LVL, true);

  } else {

    this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 0);
    this->msgItem(IDC_CB_LVL, CB_SETCURSEL, 0, 0);
    this->enableItem(IDC_CB_LVL, false);
  }

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_onResize()
{
  // Compressed Backup CheckBox
  this->_setItemPos(IDC_BC_CKBX1, 50, 20, 120, 9);
  // Compression level Label & ComboBox
  this->_setItemPos(IDC_SC_LBL01, 50, 40, 120, 9);
  this->_setItemPos(IDC_CB_LVL, 50, 50, this->width()-100, 14);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_LVL), nullptr, true);
  // Maintenance operations Label
  this->_setItemPos(IDC_SC_LBL02, 50, 80, 120, 9);
  // Discard backups Button
  this->_setItemPos(IDC_BC_DEL, 50, 90, 80, 15);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLocBck::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_BACKDISCARD_DONE is a custom message sent from Location backups discard thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_BACKDISCARD_DONE) {
    // end the Location backups deletion process
    this->_delBck_stop();
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_CKBX1: //< Compress backup data CheckBox
      this->_onCkBoxZip();
      break;

    case IDC_CB_LVL: //< Backup compression level ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->setChParam(LOC_PROP_BCK_COMP_LEVEL, true);
      break;

    case IDC_BC_DEL: //< "Discard backups" Button
      this->_onBcDelBck();
      break;
    }
  }

  return false;
}
