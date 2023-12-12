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

#include "OmUiPropHub.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropHubStg.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubStg::OmUiPropHubStg(HINSTANCE hins) : OmDialogPropTab(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHubStg::~OmUiPropHubStg()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropHubStg::id() const
{
  return IDD_PROP_HUB_STG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubStg::_browse_hub_icon()
{
  OmWString start, result;

  // get last valid path to start browsing
  this->getItemText(IDC_EC_INP04, start);
  start = Om_getDirPart(start);

  if(!Om_dlgOpenFile(result, this->_parent->hwnd(), L"Select icon", OM_ICO_FILES_FILTER, start))
    return;

  HICON hIc = nullptr;

  // check if the path to icon is non empty
  if(Om_hasLegalPathChar(result))
    ExtractIconExW(result.c_str(), 0, &hIc, nullptr, 1);

  if(hIc) {
    this->setItemText(IDC_EC_INP04, result);
    DeleteObject(hIc); //< delete temporary icon
  } else {
    this->setItemText(IDC_EC_INP04, L"");
  }

  // refresh icon
  this->_onTbRefresh();

  // user modified parameter, notify it
  this->paramCheck(HUB_PROP_STG_ICON);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubStg::_delete_hub_icon()
{
  this->setItemText(IDC_EC_INP04, L""); //< set invalid path

  // refresh icon
  this->_onTbRefresh();

  // user modified parameter, notify it
  this->paramCheck(HUB_PROP_STG_ICON);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubStg::_onTbInit()
{
  // add icon to buttons
  this->setBmIcon(IDC_BC_BRW01, Om_getResIcon(IDI_BT_OPN));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(IDI_BT_REM));

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Hub home directory path");
  this->_createTooltip(IDC_EC_INP03,  L"Hub title");
  this->_createTooltip(IDC_BC_BRW01,  L"Select icon or application");
  this->_createTooltip(IDC_BC_DEL,    L"Remove icon");

  OmModHub* ModHub = static_cast<OmUiPropHub*>(this->_parent)->ModHub();
  if(!ModHub) return;

  this->setItemText(IDC_EC_INP01, ModHub->home());
  this->setItemText(IDC_EC_INP03, ModHub->title());
  this->setItemText(IDC_EC_INP04, ModHub->iconSource()); //< hidden icon path

  // refresh with default values
  this->_onTbRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubStg::_onTbResize()
{
  int32_t y_base = 30;

  // home location Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 50, y_base, 300, 16, true);
  this->_setItemPos(IDC_EC_INP01, 50, y_base+20, this->cliWidth()-80, 21, true);
  // Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 50, y_base+50, 300, 16, true);
  this->_setItemPos(IDC_EC_INP03, 50, y_base+70, this->cliWidth()-80, 21, true);

  // Icon Label
  this->_setItemPos(IDC_SC_LBL04, 50, y_base+110, 300, 16, true);
  // Icon path (hidden)
  this->_setItemPos(IDC_EC_INP04, 350, y_base+110, 50, 21, true); //< hidden field
  // Icon image
  this->_setItemPos(IDC_SB_ICON, 75, y_base+130, 96, 96, true);

  // Select & Remove Buttons
  this->_setItemPos(IDC_BC_BRW01, 50, y_base+130, 22, 22, true);
  this->_setItemPos(IDC_BC_DEL, 50, y_base+153, 22, 22, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropHubStg::_onTbRefresh()
{
  OmWString icon_src;
  HICON hIc = nullptr;

  // check if the path to icon is valid
  this->getItemText(IDC_EC_INP04, icon_src);
  if(Om_hasLegalPathChar(icon_src)) {
    ExtractIconExW(icon_src.c_str(), 0, &hIc, nullptr, 1); //< large icon
  } else {
    hIc = Om_getShellIcon(SIID_APPLICATION, true);
  }

  // Update icon static control
  hIc = this->setStIcon(IDC_SB_ICON, hIc);

  // Properly delete unused icon
  if(hIc) {
    if(hIc != Om_getShellIcon(SIID_APPLICATION, true)) DeleteObject(hIc);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropHubStg::_onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP03:  //< Entry: title
      // notify parameter changes
      this->paramCheck(HUB_PROP_STG_TITLE);
      break;

    case IDC_BC_BRW01:  //< Button: Browse to select Icon/Application
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_hub_icon();
      break;

    case IDC_BC_DEL:    //< Button: Remove icon
      if(HIWORD(wParam) == BN_CLICKED)
        this->_delete_hub_icon();
      break;
    }
  }

  return false;
}
