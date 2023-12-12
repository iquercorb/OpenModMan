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
void OmUiPropChnLib::_cust_library_toggle()
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
void OmUiPropChnLib::_browse_dir_library()
{
  OmWString start, result;

  this->getItemText(IDC_EC_INP01, start);

  if(!Om_dlgOpenDir(result, this->_hwnd, L"Select Library folder, where Mods/Packages are stored.", start))
    return;

  this->setItemText(IDC_EC_INP01, result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onTbInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CKBX1,  L"Use custom directory as Mod Library");
  this->_createTooltip(IDC_EC_INP01,  L"Mod Library directory, where Mods are stored");
  this->_createTooltip(IDC_BC_BRW01,  L"Select custom Mod Library directory");

  this->_createTooltip(IDC_BC_CKBX2,  L"Parse Library's sub-directories as Mods for development purpose or legacy support");

  this->_createTooltip(IDC_BC_CKBX7,  L"Parse and show Library's hidden files and sub-directories");

  this->_createTooltip(IDC_BC_CKBX3,  L"Warn if Mod installation will overlap any previously installed");
  this->_createTooltip(IDC_BC_CKBX4,  L"Warn if Mod install require additional dependencies installation");
  this->_createTooltip(IDC_BC_CKBX5,  L"Warn if Mod dependencies are missing");
  this->_createTooltip(IDC_BC_CKBX6,  L"Warn if Mod uninstall require additional uninstallations");


  this->_onTbRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onTbRefresh()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan)
    return;

  this->setItemText(IDC_EC_INP01, ModChan->libraryPath());
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, ModChan->hasCustLibraryPath());
  this->enableItem(IDC_EC_INP01, ModChan->hasCustLibraryPath());
  this->enableItem(IDC_BC_BRW01, ModChan->hasCustLibraryPath());

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
void OmUiPropChnLib::_onTbResize()
{
  int32_t y_base = 30;

  // Custom Library CheckBox
  this->_setItemPos(IDC_BC_CKBX1, 50, y_base, 240, 16, true);
  // Mod Channel Library Label, EditControl and Browse button
  this->_setItemPos(IDC_EC_INP01, 50, y_base+20, this->cliWidth()-130, 20, true);
  this->_setItemPos(IDC_BC_BRW01, this->cliWidth()-75, y_base+19, 25, 22, true);

  // Library Dev Mode CheckBox
  this->_setItemPos(IDC_BC_CKBX2, 50, y_base+60, 300, 16, true);

  // Show Hidden CheckBox
  this->_setItemPos(IDC_BC_CKBX7, 50, y_base+100, 300, 16, true);

  // Enable warnings Label
  this->_setItemPos(IDC_SC_LBL01, 50, y_base+140, 300, 16, true);

  // Enable warnings CheckBoxes
  this->_setItemPos(IDC_BC_CKBX3, 75, y_base+160, 300, 16, true);
  this->_setItemPos(IDC_BC_CKBX4, 75, y_base+180, 300, 16, true);
  this->_setItemPos(IDC_BC_CKBX5, 75, y_base+200, 300, 16, true);
  this->_setItemPos(IDC_BC_CKBX6, 75, y_base+220, 300, 16, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChnLib::_onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_CKBX1: //< CheckBox: use custom library
      if(HIWORD(wParam) == BN_CLICKED)
        this->_cust_library_toggle();
      break;

    case IDC_EC_INP01: //< Library EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->paramCheck(CHN_PROP_LIB_CUSTDIR);
      break;

    case IDC_BC_BRW01: //< Button: Browse library directory
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_dir_library();
      break;

    case IDC_BC_CKBX2: //< CheckBox: development mode
      if(HIWORD(wParam) == BN_CLICKED)
        // notify parameter changes
        this->paramCheck(CHN_PROP_LIB_DEVMODE);
      break;

    case IDC_BC_CKBX7: //< CheckBox : show hidden files/directories
      if(HIWORD(wParam) == BN_CLICKED)
        // notify parameter changes
        this->paramCheck(CHN_PROP_LIB_SHOWHIDDEN);
      break;

    case IDC_BC_CKBX3: //< CheckBox : Warn overlapping
    case IDC_BC_CKBX4: //< CheckBox : Warn extra installs
    case IDC_BC_CKBX5: //< CheckBox : Warn missing dependency
    case IDC_BC_CKBX6: //< CheckBox : Warn extra uninstalls
      // notify parameter changes
      this->paramCheck(CHN_PROP_LIB_WARNINGS);
      break;
    }
  }

  return false;
}
