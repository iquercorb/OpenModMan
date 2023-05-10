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

#include "OmManager.h"

#include "OmUiPropLoc.h"

#include "OmUtilDlg.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropLocStg.h"


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

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Target path, where Mods/Packages are to be applied.", start))
    return;

  this->setItemText(IDC_EC_INP02, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onCkBoxLib()
{
  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->locCur();
  if(!pLoc) return;

  bool bm_chk = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_EC_INP03, bm_chk);
  this->enableItem(IDC_BC_BRW03, bm_chk);

  if(bm_chk && pLoc->hasCustLibDir()) {
    this->setItemText(IDC_EC_INP03, pLoc->libDir());
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

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Library folder, where Mods/Packages are stored.", start))
    return;

  this->setItemText(IDC_EC_INP03, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onCkBoxBck()
{
  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->locCur();
  if(!pLoc) return;

  bool bm_chk = this->msgItem(IDC_BC_CKBX2, BM_GETCHECK);

  this->enableItem(IDC_EC_INP04, bm_chk);
  this->enableItem(IDC_BC_BRW04, bm_chk);

  if(bm_chk && pLoc->hasCustBckDir()) {
    this->setItemText(IDC_EC_INP04, pLoc->bckDir());
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

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Backup folder, where backup data will be stored.", start))
    return;

  this->setItemText(IDC_EC_INP04, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Location name, to identify it");

  this->_createTooltip(IDC_EC_INP02,  L"Installation destination path, where Mods/Packages are to be installed");
  this->_createTooltip(IDC_BC_BRW02,  L"Browse to select destination folder");

  this->_createTooltip(IDC_BC_CKBX1,  L"Use a custom Library folder instead of default one");
  this->_createTooltip(IDC_EC_INP03,  L"Library folder path, where Mods/Packages are stored");
  this->_createTooltip(IDC_BC_BRW03,  L"Browse to select a custom Library folder");

  this->_createTooltip(IDC_BC_CKBX2,  L"Use a custom Backup folder instead of default one");
  this->_createTooltip(IDC_EC_INP04,  L"Backup folder path, where backup data will be stored");
  this->_createTooltip(IDC_BC_BRW04,  L"Browse to select a custom Backup folder");

  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onResize()
{
  // Location Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 50, 15, 220, 9);
  this->_setItemPos(IDC_EC_INP01, 50, 25, this->cliUnitX()-90, 13);

  // Location Location Label, EditControl and Browse button
  this->_setItemPos(IDC_SC_LBL02, 50, 50, 220, 9);
  this->_setItemPos(IDC_EC_INP02, 50, 60, this->cliUnitX()-108, 13);
  this->_setItemPos(IDC_BC_BRW02, this->cliUnitX()-55, 60, 16, 13);

  // Custom Library CheckBox
  this->_setItemPos(IDC_BC_CKBX1, 50, 90, 240, 9);
  // Location Library Label, EditControl and Browse button
  this->_setItemPos(IDC_EC_INP03, 50, 100, this->cliUnitX()-108, 13);
  this->_setItemPos(IDC_BC_BRW03, this->cliUnitX()-55, 100, 16, 13);

  // Custom Backup CheckBox
  this->_setItemPos(IDC_BC_CKBX2, 50, 120, 240, 9);
  // Location Backup Label, EditControl and Browse button
  this->_setItemPos(IDC_EC_INP04, 50, 130, this->cliUnitX()-108, 13);
  this->_setItemPos(IDC_BC_BRW04, this->cliUnitX()-55, 130, 16, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocStg::_onRefresh()
{
  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->locCur();

  if(pLoc == nullptr)
    return;

  this->setItemText(IDC_EC_INP01, pLoc->title());
  this->setItemText(IDC_EC_INP02, pLoc->dstDir());

  this->setItemText(IDC_EC_INP03, pLoc->libDir());
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, pLoc->hasCustLibDir());
  this->enableItem(IDC_EC_INP03, pLoc->hasCustLibDir());
  this->enableItem(IDC_BC_BRW03, pLoc->hasCustLibDir());

  this->setItemText(IDC_EC_INP04, pLoc->bckDir());
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, pLoc->hasCustBckDir());
  this->enableItem(IDC_EC_INP04, pLoc->hasCustBckDir());
  this->enableItem(IDC_BC_BRW04, pLoc->hasCustBckDir());

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropLocStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01: //< Title EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->setChParam(LOC_PROP_STG_TITLE, true);
      break;

    case IDC_BC_BRW02: //< Target path "..." (browse) Button
      this->_onBcBrwDst();
      break;

    case IDC_EC_INP02: //< Target path EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->setChParam(LOC_PROP_STG_INSTALL, true);
      break;

    case IDC_BC_CKBX1: //< Check Box for custom Library path
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

    case IDC_BC_CKBX2: //< Check Box for custom Backup path
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

