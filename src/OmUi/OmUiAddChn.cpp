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
#include "OmBaseApp.h"
#include "OmBaseUi.h"

#include "OmModMan.h"

#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiAddChn.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiAddChn::OmUiAddChn(HINSTANCE hins) : OmDialog(hins),
  _modHub(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiAddChn::~OmUiAddChn()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiAddChn::id() const
{
  return IDD_ADD_CHN;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddChn::_onTitleChange()
{
  OmWString title;

  this->getItemText(IDC_EC_INP01, title);

  if(!Om_isValidName(title))
    title = L"<invalid path>";

  if(!this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
    this->setItemText(IDC_EC_INP03, title + OM_MODCHAN_MODLIB_DIR);
  }
  if(!this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
    this->setItemText(IDC_EC_INP04, title + OM_MODCHAN_BACKUP_DIR);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddChn::_onBcBrwDst()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP02, start);

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Target path, where Mods/Packages are to be applied.", start))
    return;

  this->setItemText(IDC_EC_INP02, result);

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddChn::_onCkBoxLib()
{
  OmWString title;

  int bm_chk = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW03, bm_chk);
  this->enableItem(IDC_EC_INP03, bm_chk);

  if(!bm_chk) {
    this->getItemText(IDC_EC_INP01, title);
    if(!Om_isValidName(title)) {
      title = L"<invalid path>";
    }
  }

  this->setItemText(IDC_EC_INP03, title + OM_MODCHAN_MODLIB_DIR);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddChn::_onBcBrwLib()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP03, start);

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Library folder, where Mods/Packages are stored.", start))
    return;

  this->setItemText(IDC_EC_INP03, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddChn::_onCkBoxBck()
{
  OmWString title;

  int bm_chk = this->msgItem(IDC_BC_CKBX2, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW04, bm_chk);
  this->enableItem(IDC_EC_INP04, bm_chk);

  if(!bm_chk) {
    this->getItemText(IDC_EC_INP01, title);
    if(!Om_isValidName(title)) {
      title = L"<invalid path>";
    }
  }

  this->setItemText(IDC_EC_INP04, title + OM_MODCHAN_BACKUP_DIR);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddChn::_onBcBrwBck()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP04, start);

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Backup folder, where backup data will be stored.", start))
    return;

  this->setItemText(IDC_EC_INP04, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiAddChn::_onBcOk()
{
  if(!this->_modHub)
    return false;

  OmWString chn_name, chn_dst, chn_lib, chn_bck, msg;

  this->getItemText(IDC_EC_INP01, chn_name);
  if(!Om_dlgValidName(this->_hwnd, L"Channel name", chn_name))
    return false;

  this->getItemText(IDC_EC_INP02, chn_dst);
  if(!Om_dlgValidDir(this->_hwnd, L"Target path", chn_dst))
    return false;

  if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) { //< Custom Library CheckBox
    this->getItemText(IDC_EC_INP03, chn_lib);
    if(Om_dlgValidPath(this->_hwnd, L"Library folder path", chn_lib)) {
      if(!Om_dlgCreateFolder(this->_hwnd, L"Custom Library folder", chn_lib))
        return false;
    } else {
      return false;
    }
  }

  if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) { //< Custom Backup CheckBox
    this->getItemText(IDC_EC_INP04, chn_bck);
    if(Om_dlgValidPath(this->_hwnd, L"Backup folder path", chn_bck)) {
      if(!Om_dlgCreateFolder(this->_hwnd, L"Custom Backup folder", chn_bck))
        return false;
    } else {
      return false;
    }
  }

  this->quit();

  // create new Mod Channel in Mod Hub
  if(!this->_modHub->createChannel(chn_name, chn_dst, chn_lib, chn_bck)) {

    Om_dlgBox_okl(this->_hwnd, L"New Mod Channel", IDI_ERR,
                  L"Mod Channel creation error", L"Mod Channel "
                  "creation failed because of the following error:",
                  this->_modHub->lastError());
  }

  // refresh all tree from the main dialog
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddChn::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins,IDI_APP,2),Om_getResIcon(this->_hins,IDI_APP,1));

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Mod Channel name, to identify it and create folder");

  this->_createTooltip(IDC_EC_INP02,  L"Target path, where Mods/Packages are to be installed");
  this->_createTooltip(IDC_BC_BRW02,  L"Browse to select destination folder");

  this->_createTooltip(IDC_BC_CKBX1,  L"Use a custom Library folder instead of default one");
  this->_createTooltip(IDC_EC_INP03,  L"Library folder path, where Mods/Packages are stored");
  this->_createTooltip(IDC_BC_BRW03,  L"Browse to select a custom Library folder");

  this->_createTooltip(IDC_BC_CKBX2,  L"Use a custom Backup folder instead of default one");
  this->_createTooltip(IDC_EC_INP04,  L"Backup folder path, where backup data will be stored");
  this->_createTooltip(IDC_BC_BRW04,  L"Browse to select a custom Backup folder");

  // set default start values
  this->setItemText(IDC_EC_INP01, L"New Mod Channel");
  this->setItemText(IDC_EC_INP02, L"");
  this->setItemText(IDC_EC_INP03, L"New Mod Channel\\Library");
  this->setItemText(IDC_EC_INP04, L"New Mod Channel\\Backup");

  OmWString item_str;

  // disable OK button
  this->enableItem(IDC_BC_OK, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddChn::_onResize()
{
  // Mod Channel title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 10, 10, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP01, 10, 25, this->cliUnitX()-25, 13);

  // Mod Channel Install Label & EditControl & Browse button
  this->_setItemPos(IDC_SC_LBL02, 10, 50, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP02, 10, 65, this->cliUnitX()-45, 13);
  this->_setItemPos(IDC_BC_BRW02, this->cliUnitX()-31, 65, 16, 13);

  // Custom Library Label & EditControl & Browse buttonben ess
  this->_setItemPos(IDC_BC_CKBX1, 10, 100, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP03, 10, 115, this->cliUnitX()-45, 13);
  this->_setItemPos(IDC_BC_BRW03, this->cliUnitX()-31, 115, 16, 13);

  // Custom Library Label & EditControl & Browse button
  this->_setItemPos(IDC_BC_CKBX2, 10, 140, this->cliUnitX()-25, 9);
  this->_setItemPos(IDC_EC_INP04, 10, 155, this->cliUnitX()-45, 13);
  this->_setItemPos(IDC_BC_BRW04, this->cliUnitX()-31, 155, 16, 13);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->cliUnitY()-25, this->cliUnitX()-10, 1);
  // Ok and Cancel buttons
  this->_setItemPos(IDC_BC_OK, this->cliUnitX()-110, this->cliUnitY()-19, 50, 14);
  this->_setItemPos(IDC_BC_CANCEL, this->cliUnitX()-54, this->cliUnitY()-19, 50, 14);

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiAddChn::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    OmWString item_str;

    switch(LOWORD(wParam))
    {

    case IDC_EC_INP01: // Title
      if(HIWORD(wParam) == EN_CHANGE) {
        this->_onTitleChange();
        has_changed = true;
      }
      break;

    case IDC_BC_BRW02: // browse destination
      this->_onBcBrwDst();
      break;

    case IDC_BC_CKBX1: //< Custom Library Checkbox
      this->_onCkBoxLib();
      break;

    case IDC_BC_BRW03: //< Custom Library "..." (browse) Button
      this->_onBcBrwLib();
      break;

    case IDC_BC_CKBX2: //< Custom Backup Checkbox
      this->_onCkBoxBck();
      break;

    case IDC_BC_BRW04: //< Custom Backup "..." (browse) Button
      this->_onBcBrwBck();
      break;

    case IDC_EC_INP02: //< Target path EditText
    case IDC_EC_INP03: //< Library EditText
    case IDC_EC_INP04: //< Backup EditText
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
      break;

    case IDC_BC_OK:
      this->_onBcOk();
      break;

    case IDC_BC_CANCEL:
      this->quit();
      break;
    }

    // enable or disable "OK" button according values
    if(has_changed) {

      bool allow = true;

      this->getItemText(IDC_EC_INP01, item_str);
      if(!item_str.empty()) {

        this->getItemText(IDC_EC_INP02, item_str);
        if(!item_str.empty()) {

          if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
            this->getItemText(IDC_EC_INP03, item_str);
            if(item_str.empty()) allow = false;
          }

          if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
            this->getItemText(IDC_EC_INP04, item_str);
            if(item_str.empty()) allow = false;
          }

        } else {
          allow = false;
        }

      } else {
        allow = false;
      }

      this->enableItem(IDC_BC_OK, allow);
    }
  }

  return false;
}
