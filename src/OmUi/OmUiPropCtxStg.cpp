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

#include "OmUiPropCtx.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropCtxStg.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtxStg::OmUiPropCtxStg(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i) {
    this->_chParam[i] = false;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtxStg::~OmUiPropCtxStg()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropCtxStg::id() const
{
  return IDD_PROP_CTX_STG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::setChParam(unsigned i, bool en)
{
  this->_chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onBcBrwIcon()
{
  wstring start, result;

  // get last valid path to start browsing
  this->getItemText(IDC_EC_INP04, start);
  start = Om_getDirPart(start);

  if(!Om_dlgOpenFile(result, this->_parent->hwnd(), L"Select icon", OMM_ICO_FILES_FILTER, start))
    return;

  HICON hIc = nullptr;

  // check if the path to icon is non empty
  if(Om_isValidPath(result))
    ExtractIconExW(result.c_str(), 0, &hIc, nullptr, 1);

  if(hIc) {
    this->setItemText(IDC_EC_INP04, result);
  } else {
    hIc = Om_getShellIcon(SIID_APPLICATION, true);
    this->setItemText(IDC_EC_INP04, L"<none>");
  }

  this->msgItem(IDC_SB_ICON, STM_SETICON, reinterpret_cast<WPARAM>(hIc));

  InvalidateRect(this->getItem(IDC_SB_ICON), nullptr, true);

  // user modified parameter, notify it
  this->setChParam(CTX_PROP_STG_ICON, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onBcDelIcon()
{
  HICON hIc = Om_getShellIcon(SIID_APPLICATION, true);
  this->msgItem(IDC_SB_ICON, STM_SETICON, reinterpret_cast<WPARAM>(hIc));

  this->setItemText(IDC_EC_INP04, L"<none>"); //< set invalid path

  // user modified parameter, notify it
  this->setChParam(CTX_PROP_STG_ICON, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onInit()
{
  // add icon to buttons
  this->setBmIcon(IDC_BC_BRW01, Om_getResIcon(this->_hins, IDI_BT_OPN));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDI_BT_REM));

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Context home folder path");
  this->_createTooltip(IDC_EC_INP03,  L"Context name, to identify it");
  this->_createTooltip(IDC_BC_BRW01,  L"Browse to select an icon to associate with Context");
  this->_createTooltip(IDC_BC_DEL,    L"Remove the associated icon");

  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  this->setItemText(IDC_EC_INP01, pCtx->home());
  this->setItemText(IDC_EC_INP02, pCtx->uuid());
  this->setItemText(IDC_EC_INP03, pCtx->title());

  this->setItemText(IDC_EC_INP04, L"<none>"); //< hidden icon path

  // refresh with default values
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onResize()
{
  // home location Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 50, 15, 180, 9);
  this->_setItemPos(IDC_EC_INP01, 50, 25, this->cliUnitX()-90, 13);
  // Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 50, 50, 180, 9);
  this->_setItemPos(IDC_EC_INP03, 50, 60, this->cliUnitX()-90, 13);
  // Icon Label & placeholder
  this->_setItemPos(IDC_SC_LBL04, 50, 90, 180, 9);
  this->_setItemPos(IDC_EC_INP04, 190, 90, 50, 13); //< hidden field
  this->_setItemPos(IDC_SB_ICON, 50, 100, 30, 30);

  // Select & Remove Buttons
  this->_setItemPos(IDC_BC_BRW01, 85, 100, 16, 14);
  this->_setItemPos(IDC_BC_DEL, 85, 115, 16, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onRefresh()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  HICON hIc = nullptr;
  wstring ico_path;

  // check if the path to icon is valid
  this->getItemText(IDC_EC_INP04, ico_path);
  if(Om_isValidPath(ico_path)) {
    // reload the last selected icon
    ExtractIconExW(ico_path.c_str(), 0, &hIc, nullptr, 1);
  } else {
    // check whether Context already have an icon configured
    if(pCtx->icon())
      hIc = pCtx->icon();
  }

  if(!hIc) {
    hIc = Om_getShellIcon(SIID_APPLICATION, true);
    this->setItemText(IDC_EC_INP04, L"<none>");
  }

  this->msgItem(IDC_SB_ICON, STM_SETICON, reinterpret_cast<WPARAM>(hIc));

  InvalidateRect(this->getItem(IDC_SB_ICON), nullptr, true);

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropCtxStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    wstring item_str, brow_str;

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP03: //< Entry for Context title
      // user modified parameter, notify it
      this->setChParam(CTX_PROP_STG_TITLE, true);
      break;

    case IDC_BC_BRW01: //< Brows Button for Context icon
      this->_onBcBrwIcon();
      break;

    case IDC_BC_DEL: //< Remove Button for Context icon
      this->_onBcDelIcon();
      break;
    }
  }

  return false;
}
