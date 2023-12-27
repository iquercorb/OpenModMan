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

#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropChnBck.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnBck::OmUiPropChnBck(HINSTANCE hins) : OmDialogPropTab(hins)
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
void OmUiPropChnBck::_cust_backup_toggle()
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
void OmUiPropChnBck::_browse_dir_backup()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP01, start);

  if(!Om_dlgOpenDir(result, this->_hwnd, L"Select Backup data directory, where Backup data will be stored", start))
    return;

  this->setItemText(IDC_EC_INP01, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnBck::_comp_backup_toggle()
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
void OmUiPropChnBck::_onTbInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CKBX1,  L"Use custom directory for Backup data storage");
  this->_createTooltip(IDC_EC_INP01,  L"Backup data directory, where Backup data will be stored");
  this->_createTooltip(IDC_BC_BRW01,  L"Select custom Backup data directory");

  this->_createTooltip(IDC_BC_CKBX2,  L"Use compressed archive instead of directories trees");
  this->_createTooltip(IDC_CB_ZMD,    L"Compression method for backup archives");
  this->_createTooltip(IDC_CB_ZLV,    L"Compression level for backup archives");

  // Set buttons inner icons
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(IDI_BT_WRN));

  int32_t cb_id;
  // add items to Compression Method ComboBox
  cb_id = this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None (Store only)"));
  this->msgItem(IDC_CB_ZMD, CB_SETITEMDATA, cb_id, OM_METHOD_STORE);
  cb_id = this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Deflate (Legacy Zip)"));
  this->msgItem(IDC_CB_ZMD, CB_SETITEMDATA, cb_id, OM_METHOD_DEFLATE);
  cb_id = this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"LZMA"));
  this->msgItem(IDC_CB_ZMD, CB_SETITEMDATA, cb_id, OM_METHOD_LZMA);
  cb_id = this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"LZMA2"));
  this->msgItem(IDC_CB_ZMD, CB_SETITEMDATA, cb_id, OM_METHOD_LZMA2);
  cb_id = this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Zstandard"));
  this->msgItem(IDC_CB_ZMD, CB_SETITEMDATA, cb_id, OM_METHOD_ZSTD);
  this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 4);

  // add items into Compression Level ComboBox
  cb_id = this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None (Store only)"));
  this->msgItem(IDC_CB_ZLV, CB_SETITEMDATA, cb_id, OM_LEVEL_NONE);
  cb_id = this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Fast"));
  this->msgItem(IDC_CB_ZLV, CB_SETITEMDATA, cb_id, OM_LEVEL_FAST);
  cb_id = this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal"));
  this->msgItem(IDC_CB_ZLV, CB_SETITEMDATA, cb_id, OM_LEVEL_SLOW);
  cb_id = this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Best"));
  this->msgItem(IDC_CB_ZLV, CB_SETITEMDATA, cb_id, OM_LEVEL_BEST);
  this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 2);

  this->_onTbRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnBck::_onTbRefresh()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan)
    return;

  this->setItemText(IDC_EC_INP01, ModChan->backupPath());
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, ModChan->hasCustBackupPath());
  this->enableItem(IDC_EC_INP01, ModChan->hasCustBackupPath());
  this->enableItem(IDC_BC_BRW01, ModChan->hasCustBackupPath());

  int32_t comp_level = ModChan->backupCompLevel();
  int32_t comp_method = ModChan->backupCompMethod();

  if(comp_method >= 0) {

    this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 1);

    uint32_t cb_count;

    // select proper compression method
    cb_count = this->msgItem(IDC_CB_ZMD, CB_GETCOUNT);
    for(uint32_t i = 0; i < cb_count; ++i) {
      if(this->msgItem(IDC_CB_ZMD, CB_GETITEMDATA, i) == comp_method) {
        this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, i); break;
      }
    }
/*
    switch(comp_method)
    {
    case OM_METHOD_DEFLATE: this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 1); break; //< MZ_COMPRESS_METHOD_DEFLATE
    case OM_METHOD_LZMA:    this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 2); break; //< MZ_COMPRESS_METHOD_LZMA
    case OM_METHOD_LZMA2:   this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 3); break; //< MZ_COMPRESS_METHOD_XZ
    case OM_METHOD_ZSTD:    this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 4); break; //< MZ_COMPRESS_METHOD_ZSTD
    default:                this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 0); break; //< MZ_COMPRESS_METHOD_STORE
    }
*/
    this->enableItem(IDC_CB_ZMD, true);

    // select proper compression level
    cb_count = this->msgItem(IDC_CB_ZLV, CB_GETCOUNT);
    for(uint32_t i = 0; i < cb_count; ++i) {
      if(this->msgItem(IDC_CB_ZLV, CB_GETITEMDATA, i) == comp_level) {
        this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, i); break;
      }
    }
/*
    switch(comp_level)
    {
    case OM_LEVEL_FAST:   this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 1); break; //< MZ_COMPRESS_LEVEL_FAST
    case OM_LEVEL_SLOW:   this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 2); break; //< MZ_COMPRESS_LEVEL_NORMAL
    case OM_LEVEL_BEST:   this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 3); break; //< MZ_COMPRESS_LEVEL_BEST
    default:              this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 0); break;
    }
*/
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
void OmUiPropChnBck::_onTbResize()
{
  int32_t y_base = 40;

  // Custom Backup CheckBox
  this->_setItemPos(IDC_BC_CKBX1, 50, y_base, 240, 16, true);

  // Mod Channel Backup Label, EditControl and Browse button
  this->_setItemPos(IDC_EC_INP01, 50, y_base+20, this->cliWidth()-130, 20, true);
  this->_setItemPos(IDC_BC_BRW01, this->cliWidth()-75, y_base+19, 25, 22, true);

  // Compressed Backup CheckBox
  this->_setItemPos(IDC_BC_CKBX2, 50, y_base+80, 300, 16, true);

  // Compression Method label & ComboBox
  this->_setItemPos(IDC_SC_LBL01, 75, y_base+113, 110, 16, true);
  this->_setItemPos(IDC_CB_ZMD, 190, y_base+110, this->cliWidth()-270, 21, true);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_ZMD), nullptr, true);

  // Compression Level label & ComboBox
  this->_setItemPos(IDC_SC_LBL02, 75, y_base+138, 110, 16, true);
  this->_setItemPos(IDC_CB_ZLV, 190, y_base+135, this->cliWidth()-270, 21, true);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_ZLV), nullptr, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChnBck::_onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {

    case IDC_BC_CKBX1: //< CheckBox: use custom backup dir
      if(HIWORD(wParam) == BN_CLICKED)
        this->_cust_backup_toggle();
      break;

    case IDC_BC_BRW01: //< Button: Browse backup directory
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_dir_backup();
      break;

    case IDC_EC_INP01: //< Backup EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->paramCheck(CHN_PROP_BCK_CUSTDIR);
      break;

    case IDC_BC_CKBX2: //< CheckBox: compress backup data
      if(HIWORD(wParam) == BN_CLICKED)
        this->_comp_backup_toggle();
      break;

    case IDC_CB_ZMD: //< ComboBox: compression Method
    case IDC_CB_ZLV: //< ComboBox: compression level
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->paramCheck(CHN_PROP_BCK_COMP_LEVEL);
      break;
    }
  }

  return false;
}
