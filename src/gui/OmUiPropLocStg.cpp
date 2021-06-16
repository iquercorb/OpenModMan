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
#include "gui/OmUiPropLocStg.h"
#include "gui/OmUiPropLoc.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocStg::OmUiPropLocStg(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i)
    this->_chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocStg::~OmUiPropLocStg()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropLocStg::id() const
{
  return IDD_PROP_LOC_STG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::setChParam(unsigned i, bool en)
{
  _chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onBcBrwDst()
{
  wstring start, result;

  this->getItemText(IDC_EC_INP02, start);

  if(!Om_dialogBrowseDir(result, this->_hwnd, L"Select packages Destination folder", start))
    return;

  this->setItemText(IDC_EC_INP02, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onCkBoxLib()
{
  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->location();
  if(!pLoc) return;

  bool bm_chk = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);

  this->enableItem(IDC_EC_INP03, bm_chk);
  this->enableItem(IDC_BC_BRW03, bm_chk);

  if(bm_chk && pLoc->hasCustLibraryDir()) {
    this->setItemText(IDC_EC_INP03, pLoc->libraryDir());
  } else {
    this->setItemText(IDC_EC_INP03, pLoc->home() + L"\\Library");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onBcBrwLib()
{
  wstring start, result;

  this->getItemText(IDC_EC_INP03, start);

  if(!Om_dialogBrowseDir(result, this->_hwnd, L"Select custom packages Library folder", start))
    return;

  this->setItemText(IDC_EC_INP03, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onCkBoxBck()
{
  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->location();
  if(!pLoc) return;

  bool bm_chk = this->msgItem(IDC_BC_CHK02, BM_GETCHECK);

  this->enableItem(IDC_EC_INP04, bm_chk);
  this->enableItem(IDC_BC_BRW04, bm_chk);

  if(bm_chk && pLoc->hasCustBackupDir()) {
    this->setItemText(IDC_EC_INP04, pLoc->backupDir());
  } else {
    this->setItemText(IDC_EC_INP04, pLoc->home() + L"\\Backup");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onBcBrwBck()
{
  wstring start, result;

  this->getItemText(IDC_EC_INP04, start);

  if(!Om_dialogBrowseDir(result, this->_hwnd, L"Select custom Backups location", start))
    return;

  this->setItemText(IDC_EC_INP04, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Indicative name");

  this->_createTooltip(IDC_EC_INP02,  L"Package installation destination path");
  this->_createTooltip(IDC_BC_BRW02,  L"Select destination folder");

  this->_createTooltip(IDC_BC_CHK01,  L"Use custom Library folder");
  this->_createTooltip(IDC_EC_INP03,  L"Custom Library folder path");
  this->_createTooltip(IDC_BC_BRW03,  L"Select custom Library folder");

  this->_createTooltip(IDC_BC_CHK02,  L"Use custom Backup folder");
  this->_createTooltip(IDC_EC_INP04,  L"Custom Backup folder path");
  this->_createTooltip(IDC_BC_BRW04,  L"Select custom Backup folder");

  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onResize()
{
  // Location Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 20, 64, 9);
  this->_setItemPos(IDC_EC_INP01, 70, 20, this->width()-90, 13);

  // Location Location Label, EditControl and Browse button
  this->_setItemPos(IDC_SC_LBL02, 5, 60, 64, 9);
  this->_setItemPos(IDC_EC_INP02, 70, 60, this->width()-108, 13);
  this->_setItemPos(IDC_BC_BRW02, this->width()-35, 60, 16, 13);

  // Custom Library CheckBox
  this->_setItemPos(IDC_BC_CHK01, 70, 90, 120, 9);
  // Location Library Label, EditControl and Browse button
  this->_setItemPos(IDC_SC_LBL03, 5, 100, 64, 9);
  this->_setItemPos(IDC_EC_INP03, 70, 100, this->width()-108, 13);
  this->_setItemPos(IDC_BC_BRW03, this->width()-35, 100, 16, 13);

  // Custom Backup CheckBox
  this->_setItemPos(IDC_BC_CHK02, 70, 120, 120, 9);
  // Location Backup Label, EditControl and Browse button
  this->_setItemPos(IDC_SC_LBL04, 5, 130, 64, 9);
  this->_setItemPos(IDC_EC_INP04, 70, 130, this->width()-108, 13);
  this->_setItemPos(IDC_BC_BRW04, this->width()-35, 130, 16, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onRefresh()
{
  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->location();

  if(pLoc == nullptr)
    return;

  this->setItemText(IDC_EC_INP01, pLoc->title());
  this->setItemText(IDC_EC_INP02, pLoc->installDir());

  this->setItemText(IDC_EC_INP03, pLoc->libraryDir());
  this->msgItem(IDC_BC_CHK01, BM_SETCHECK, pLoc->hasCustLibraryDir());
  this->enableItem(IDC_EC_INP03, pLoc->hasCustLibraryDir());
  this->enableItem(IDC_BC_BRW03, pLoc->hasCustLibraryDir());

  this->setItemText(IDC_EC_INP04, pLoc->backupDir());
  this->msgItem(IDC_BC_CHK02, BM_SETCHECK, pLoc->hasCustBackupDir());
  this->enableItem(IDC_EC_INP04, pLoc->hasCustBackupDir());
  this->enableItem(IDC_BC_BRW04, pLoc->hasCustBackupDir());

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLocStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01: //< Title EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->setChParam(LOC_PROP_STG_TITLE, true);
      break;

    case IDC_BC_BRW02: //< Destination "..." (browse) Button
      this->_onBcBrwDst();
      break;

    case IDC_EC_INP02: //< Destination EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->setChParam(LOC_PROP_STG_INSTALL, true);
      break;

    case IDC_BC_CHK01: //< Check Box for custom Library path
      this->_onCkBoxLib();
      break;

    case IDC_BC_BRW03: //< Custom Library "..." (browse) Button
      this->_onBcBrwLib();
      break;

    case IDC_EC_INP03: //< Library EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->setChParam(LOC_PROP_STG_LIBRARY, true);
      break;

    case IDC_BC_CHK02: //< Check Box for custom Backup path
      this->_onCkBoxBck();
      break;

    case IDC_BC_BRW04: //< Custom Backup "..." (browse) Button
      this->_onBcBrwBck();
      break;

    case IDC_EC_INP04: //< Backup EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->setChParam(LOC_PROP_STG_BACKUP, true);
      break;
    }
  }

  return false;
}

