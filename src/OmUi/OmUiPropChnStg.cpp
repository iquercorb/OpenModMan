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
void OmUiPropChnStg::_browse_dir_target()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP02, start);

  if(!Om_dlgOpenDir(result, this->_hwnd, L"Select Target directory, where Mods are to be applied", start))
    return;

  this->setItemText(IDC_EC_INP02, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnStg::_onTbInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Channel title");

  this->_createTooltip(IDC_EC_INP02,  L"Target directory path, where Mods are to be applied");
  this->_createTooltip(IDC_BC_BRW02,  L"Select target directory");

  this->_onTbRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnStg::_onTbResize()
{
  int32_t y_base = 40;

  // Mod Channel Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 50, y_base, 220, 16, true);
  this->_setItemPos(IDC_EC_INP01, 50, y_base+20, this->cliWidth()-100, 20, true);

  // Mod Channel Label, EditControl and Browse button
  this->_setItemPos(IDC_SC_LBL02, 50, y_base+80, 220, 16, true);
  this->_setItemPos(IDC_EC_INP02, 50, y_base+100, this->cliWidth()-130, 20, true);
  this->_setItemPos(IDC_BC_BRW02, this->cliWidth()-75, y_base+99, 25, 22, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnStg::_onTbRefresh()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan) return;

  this->setItemText(IDC_EC_INP01, ModChan->title());
  this->setItemText(IDC_EC_INP02, ModChan->targetPath());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChnStg::_onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01: //< Title EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->paramCheck(CHN_PROP_STG_TITLE);
      break;

    case IDC_BC_BRW02: //< Target path "..." (browse) Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_dir_target();
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

