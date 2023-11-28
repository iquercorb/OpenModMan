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
#include "OmUiPropChn.h"
#include "OmUtilDlg.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropChnStg.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnStg::OmUiPropChnStg(HINSTANCE hins) : OmDialogPropTab(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnStg::~OmUiPropChnStg()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropChnStg::id() const
{
  return IDD_PROP_CHN_STG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnStg::_onBcBrwDst()
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
void OmUiPropChnStg::_onTabInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Mod Channel name, to identify it");

  this->_createTooltip(IDC_EC_INP02,  L"Installation destination path, where Mods/Packages are to be installed");
  this->_createTooltip(IDC_BC_BRW02,  L"Browse to select destination folder");

  this->_onTabRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnStg::_onTabResize()
{
  // Mod Channel Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 50, 15, 220, 9);
  this->_setItemPos(IDC_EC_INP01, 50, 25, this->cliUnitX()-90, 13);

  // Mod Channel Label, EditControl and Browse button
  this->_setItemPos(IDC_SC_LBL02, 50, 50, 220, 9);
  this->_setItemPos(IDC_EC_INP02, 50, 60, this->cliUnitX()-108, 13);
  this->_setItemPos(IDC_BC_BRW02, this->cliUnitX()-55, 60, 16, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnStg::_onTabRefresh()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();

  if(ModChan == nullptr)
    return;

  this->setItemText(IDC_EC_INP01, ModChan->title());
  this->setItemText(IDC_EC_INP02, ModChan->targetPath());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChnStg::_onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01: //< Title EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->paramCheck(CHN_PROP_STG_TITLE);
      break;

    case IDC_BC_BRW02: //< Target path "..." (browse) Button
      this->_onBcBrwDst();
      break;

    case IDC_EC_INP02: //< Target path EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->paramCheck(CHN_PROP_STG_TARGET);
      break;
    }
  }

  return false;
}

