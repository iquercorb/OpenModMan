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

#include "OmModMan.h"

#include "OmArchive.h"          //< Archive compression methods / level

#include "OmUiMan.h"
#include "OmUiPropChn.h"
#include "OmUiProgress.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropChnBck.h"


/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the backupDcard_fth
/// thread finished his job.
///
#define UWM_BACKDISCARD_DONE     (WM_APP+1)

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnBck::OmUiPropChnBck(HINSTANCE hins) : OmDialogPropTab(hins),
  _delBck_hth(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnBck::~OmUiPropChnBck()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropChnBck::id() const
{
  return IDD_PROP_CHN_BCK;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnBck::_onCkBoxBck()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan)
    return;

  bool bm_chk = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_EC_INP01, bm_chk);
  this->enableItem(IDC_BC_BRW01, bm_chk);

  this->setItemText(IDC_EC_INP01, ModChan->backupPath());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnBck::_onBcBrwBck()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP01, start);

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Backup folder, where backup data will be stored.", start))
    return;

  this->setItemText(IDC_EC_INP01, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnBck::_onCkBoxZip()
{
  bool bm_chk = this->msgItem(IDC_BC_CKBX2, BM_GETCHECK);

  this->enableItem(IDC_SC_LBL01, bm_chk);
  this->enableItem(IDC_CB_ZMD, bm_chk);
  this->enableItem(IDC_SC_LBL02, bm_chk);
  this->enableItem(IDC_CB_ZLV, bm_chk);

  this->paramCheck(CHN_PROP_BCK_COMP_LEVEL);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnBck::_onBcDelBck()
{
  // warns the user before committing the irreparable
  if(!Om_dlgBox_ca(this->_hwnd, L"Mod Channel properties", IDI_QRY,
            L"Discard Mod Channel backup data", L"This will permanently "
            "delete all existing backup data without restoring them (which "
            "should never be done except in emergency situation)."))
  {
    return;
  }

  // Launch backup discard process
  this->_delBck_init();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnBck::_delBck_init()
{
  // To prevent crash during operation we unselect location in the main dialog
  static_cast<OmUiMan*>(this->root())->safemode(true);

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS));

  pUiProgress->open(true);
  pUiProgress->setCaption(L"Mod Channel backup data discard");
  pUiProgress->setScHeadText(L"Deleting backup data");

  DWORD dwId;
  this->_delBck_hth = CreateThread(nullptr, 0, this->_delBck_fth, this, 0, &dwId);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnBck::_delBck_stop()
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
  static_cast<OmUiMan*>(this->root())->safemode(false);

  if(exitCode == 1) {
    Om_dlgBox_ok(this->_hwnd, L"Mod Channel properties", IDI_PKG_ERR,
              L"Backup data discard error", L"The Backup data discarding "
              "process encountered error(s), some backup data may "
              "had not properly deleted. Please read debug log for details.");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropChnBck::_delBck_fth(void* arg)
{
  OmUiPropChnBck* self = static_cast<OmUiPropChnBck*>(arg);

  OmModChan* ModChan = static_cast<OmUiPropChn*>(self->_parent)->ModChan();
  if(!ModChan)
    return 1;

  DWORD exitCode = 0;

  // launch backups data deletion process
  /* TODO: refaire ça avec la nouvelle API
  if(!ModChan->discardBackupData(&self->_delBck_progress_cb, self->siblingById(IDD_PROGRESS))) {
    exitCode = 1;
  }
  */

  // sends message to window to inform process ended
  PostMessage(self->_hwnd, UWM_BACKDISCARD_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropChnBck::_delBck_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t data)
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
void OmUiPropChnBck::_onTabInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CKBX1,  L"Use a custom Backup folder instead of default one");
  this->_createTooltip(IDC_EC_INP01,  L"Backup folder path, where backup data will be stored");
  this->_createTooltip(IDC_BC_BRW01,  L"Browse to select a custom Backup folder");

  this->_createTooltip(IDC_BC_CKBX2,  L"Store backup data as zip archives");
  this->_createTooltip(IDC_CB_ZMD,    L"Compression method for backup zip archives");
  this->_createTooltip(IDC_CB_ZLV,    L"Compression level for backup zip archives");
  this->_createTooltip(IDC_BC_DEL,    L"Delete all backup data without restoring it (emergency use only)");

  // Set buttons inner icons
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDI_BT_WRN));

  // add items to Compression Method ComboBox
  this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None (Store only)"));
  this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Deflate (Legacy Zip)"));
  this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"LZMA"));
  this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"LZMA2"));
  this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Zstandard"));

  // add items to Compression Level ComboBox
  this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None (Store only)"));
  this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Fast"));
  this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal"));
  this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Best"));

  this->_onTabRefresh();
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnBck::_onTabRefresh()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan)
    return;

  this->setItemText(IDC_EC_INP01, ModChan->backupPath());
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, ModChan->hasCustomBackupDir());
  this->enableItem(IDC_EC_INP01, ModChan->hasCustomBackupDir());
  this->enableItem(IDC_BC_BRW01, ModChan->hasCustomBackupDir());

  int32_t comp_level = ModChan->backupCompLevel();
  int32_t comp_method = ModChan->backupCompMethod();

  if(comp_method >= 0) {

    this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 1);

    switch(comp_method)
    {
    case OM_METHOD_DEFLATE: this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 1); break; //< MZ_COMPRESS_METHOD_DEFLATE
    case OM_METHOD_LZMA:    this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 2); break; //< MZ_COMPRESS_METHOD_LZMA
    case OM_METHOD_LZMA2:   this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 3); break; //< MZ_COMPRESS_METHOD_XZ
    case OM_METHOD_ZSTD:    this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 4); break; //< MZ_COMPRESS_METHOD_ZSTD
    default:                this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 0); break; //< MZ_COMPRESS_METHOD_STORE
    }

    this->enableItem(IDC_CB_ZMD, true);

    switch(comp_level)
    {
    case OM_LEVEL_FAST:   this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 1); break; //< MZ_COMPRESS_LEVEL_FAST
    case OM_LEVEL_SLOW:   this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 2); break; //< MZ_COMPRESS_LEVEL_NORMAL
    case OM_LEVEL_BEST:   this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 3); break; //< MZ_COMPRESS_LEVEL_BEST
    default:              this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 0); break;
    }

    this->enableItem(IDC_CB_ZLV, true);

  } else {

    this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 0);
    this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 4);
    this->enableItem(IDC_CB_ZMD, false);
    this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 2);
    this->enableItem(IDC_CB_ZLV, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnBck::_onTabResize()
{

  // Custom Backup CheckBox
  this->_setItemPos(IDC_BC_CKBX1, 50, 15, 240, 9);
  // Mod Channel Backup Label, EditControl and Browse button
  this->_setItemPos(IDC_EC_INP01, 50, 25, this->cliUnitX()-108, 13);
  this->_setItemPos(IDC_BC_BRW01, this->cliUnitX()-55, 25, 16, 13);

  // Compressed Backup CheckBox
  this->_setItemPos(IDC_BC_CKBX2, 50, 50, 140, 9);

  // Compression Method label & ComboBox
  this->_setItemPos(IDC_SC_LBL01, 65, 67, 70, 9);
  this->_setItemPos(IDC_CB_ZMD, 140, 65, this->cliUnitX()-180, 14);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_ZMD), nullptr, true);

  // Compression Level label & ComboBox
  this->_setItemPos(IDC_SC_LBL02, 65, 82, 70, 9);
  this->_setItemPos(IDC_CB_ZLV, 140, 80, this->cliUnitX()-180, 14);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_ZLV), nullptr, true);

  // Maintenance operations Label
  this->_setItemPos(IDC_SC_LBL03, 50, 110, 120, 9);
  // Discard backups Button
  this->_setItemPos(IDC_BC_DEL, 65, 125, 90, 15);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChnBck::_onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_BACKDISCARD_DONE is a custom message sent from Mod Channel backups discard thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_BACKDISCARD_DONE) {
    // end the Mod Channel backups deletion process
    this->_delBck_stop();
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {

    case IDC_BC_CKBX1: //< Check Box for custom Backup path
      this->_onCkBoxBck();
      break;

    case IDC_BC_BRW01: //< Custom Backup "..." (browse) Button
      this->_onBcBrwBck();
      break;

    case IDC_EC_INP01: //< Backup EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->paramCheck(CHN_PROP_BCK_CUSTDIR);
      break;

    case IDC_BC_CKBX2: //< Compress backup data CheckBox
      this->_onCkBoxZip();
      break;

    case IDC_CB_ZMD: //< Backup compression Method ComboBox
    case IDC_CB_ZLV: //< Backup compression level ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->paramCheck(CHN_PROP_BCK_COMP_LEVEL);
      break;

    case IDC_BC_DEL: //< "Discard backups" Button
      this->_onBcDelBck();
      break;
    }
  }

  return false;
}
