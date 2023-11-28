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
#include "OmUiPropChnLib.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnLib::OmUiPropChnLib(HINSTANCE hins) : OmDialogPropTab(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnLib::~OmUiPropChnLib()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropChnLib::id() const
{
  return IDD_PROP_CHN_LIB;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onCkBoxLib()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan)
    return;

  bool bm_chk = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_EC_INP01, bm_chk);
  this->enableItem(IDC_BC_BRW01, bm_chk);

  this->setItemText(IDC_EC_INP01, ModChan->libraryPath());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onBcBrwLib()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP01, start);

  if(!Om_dlgBrowseDir(result, this->_hwnd, L"Select Library folder, where Mods/Packages are stored.", start))
    return;

  this->setItemText(IDC_EC_INP01, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onCkBoxDev()
{
  // user modified parameter, notify it
  this->paramCheck(CHN_PROP_LIB_DEVMODE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onCkBoxWrn()
{
  // user modified parameter, notify it
  this->paramCheck(CHN_PROP_LIB_WARNINGS);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onCkBoxHid()
{
  // user modified parameter, notify it
  this->paramCheck(CHN_PROP_LIB_SHOWHIDDEN);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onTabInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CKBX1,  L"Use a custom Library folder instead of default one");
  this->_createTooltip(IDC_EC_INP01,  L"Library folder path, where Mods/Packages are stored");
  this->_createTooltip(IDC_BC_BRW01,  L"Browse to select a custom Library folder");

  this->_createTooltip(IDC_BC_CKBX2,  L"Parse library subfolders as mod packages for development purpose or legacy support");

  this->_createTooltip(IDC_BC_CKBX3,  L"Warn when packages install will overlap any previously installed");
  this->_createTooltip(IDC_BC_CKBX4,  L"Warn when packages install require additional dependencies installation");
  this->_createTooltip(IDC_BC_CKBX5,  L"Warn when packages dependencies are missing");
  this->_createTooltip(IDC_BC_CKBX6,  L"Warn when packages uninstall require additional uninstallations");
  this->_createTooltip(IDC_BC_CKBX7,  L"Parse and show library hidden files and subfolders");

  this->_onTabRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onTabRefresh()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan)
    return;

  this->setItemText(IDC_EC_INP01, ModChan->libraryPath());
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, ModChan->hasCustomLibraryDir());
  this->enableItem(IDC_EC_INP01, ModChan->hasCustomLibraryDir());
  this->enableItem(IDC_BC_BRW01, ModChan->hasCustomLibraryDir());

  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, ModChan->libraryDevmod());
  this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, ModChan->warnOverlaps());
  this->msgItem(IDC_BC_CKBX4, BM_SETCHECK, ModChan->warnExtraInst());
  this->msgItem(IDC_BC_CKBX5, BM_SETCHECK, ModChan->warnMissDeps());
  this->msgItem(IDC_BC_CKBX6, BM_SETCHECK, ModChan->warnExtraUnin());
  this->msgItem(IDC_BC_CKBX7, BM_SETCHECK, ModChan->libraryShowhidden());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onTabResize()
{
  // Custom Library CheckBox
  this->_setItemPos(IDC_BC_CKBX1, 50, 15, 240, 9);
  // Mod Channel Library Label, EditControl and Browse button
  this->_setItemPos(IDC_EC_INP01, 50, 25, this->cliUnitX()-108, 13);
  this->_setItemPos(IDC_BC_BRW01, this->cliUnitX()-55, 25, 16, 13);

  // Library Dev Mode CheckBox
  this->_setItemPos(IDC_BC_CKBX2, 50, 50, 200, 9);

  // Enable warnings Label
  this->_setItemPos(IDC_SC_LBL01, 50, 70, 200, 9);

  // Enable warnings CheckBoxes
  this->_setItemPos(IDC_BC_CKBX3, 65, 80, 200, 9);
  this->_setItemPos(IDC_BC_CKBX4, 65, 90, 200, 9);
  this->_setItemPos(IDC_BC_CKBX5, 65, 100, 200, 9);
  this->_setItemPos(IDC_BC_CKBX6, 65, 110, 200, 9);

  // Show Hidden CheckBox
  this->_setItemPos(IDC_BC_CKBX7, 50, 130, 200, 9);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChnLib::_onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_CKBX1: //< Check Box for custom Library path
      this->_onCkBoxLib();
      break;

    case IDC_EC_INP01: //< Library EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->paramCheck(CHN_PROP_LIB_CUSTDIR);
      break;

    case IDC_BC_BRW01: //< Custom Library "..." (browse) Button
      this->_onBcBrwLib();
      break;

    case IDC_BC_CKBX2: //< Check box for Package legacy (folders) support
      this->_onCkBoxDev();
      break;

    case IDC_BC_CKBX3: //< Check box for Warn at Installation overlaps
    case IDC_BC_CKBX4: //< Check box for Warn at Additional installation due to dependencies
    case IDC_BC_CKBX5: //< Check box for Warn at Installation dependencies missing
    case IDC_BC_CKBX6: //< Check box for Warn at Additional restoration due to overlaps
      this->_onCkBoxWrn();
      break;

    case IDC_BC_CKBX7: //< Check box for Show hidden files/folders
      this->_onCkBoxHid();
      break;
    }
  }

  return false;
}
