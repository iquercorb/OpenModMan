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
#include "gui/OmUiPropCtxStg.h"
#include "gui/OmUiPropCtx.h"


#define ICON_FILES_FILTER L"Icon files (*.ico,*.exe)      \
                          \0*.ICO;*.EXE;                  \
                          \0Programs (*.exe)              \
                          \0*.EXE;                         \
                          \0Icons (*.ico)                 \
                          \0*.ICO;\0"                      \


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
  HICON hIc = reinterpret_cast<HICON>(this->msgItem(IDC_SB_CTICO, STM_GETICON));
  if(hIc) DestroyIcon(hIc);
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

  if(!Om_dialogOpenFile(result, this->_parent->hwnd(), L"Select Context icon.", ICON_FILES_FILTER, start))
    return;

  HICON hIc;

  // check if the path to icon is non empty
  if(Om_isValidPath(result)) {
    ExtractIconExW(result.c_str(), 0, &hIc, nullptr, 1);
  } else {
    hIc = Om_loadShellIcon(SIID_APPLICATION, true);
  }

  hIc = reinterpret_cast<HICON>(this->msgItem(IDC_SB_CTICO, STM_SETICON, reinterpret_cast<WPARAM>(hIc)));
  if(hIc) DestroyIcon(hIc);

  InvalidateRect(this->getItem(IDC_SB_CTICO), nullptr, true);

  this->setItemText(IDC_EC_INP04, result);
  this->setItemText(IDC_BC_BRW01, L"Change..."); //< change browse button text

  // user modified parameter, notify it
  this->setChParam(CTX_PROP_STG_ICON, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onBcDelIcon()
{
  HICON hIc = Om_loadShellIcon(SIID_APPLICATION, true);
  hIc = reinterpret_cast<HICON>(this->msgItem(IDC_SB_CTICO, STM_SETICON, reinterpret_cast<WPARAM>(hIc)));
  if(hIc) DestroyIcon(hIc);

  this->setItemText(IDC_EC_INP04, L"<delete>"); //< set invalid path
  this->setItemText(IDC_BC_BRW01, L"Select..."); //< change browse button text

  // user modified parameter, notify it
  this->setChParam(CTX_PROP_STG_ICON, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP03,  L"Indicative name");

  this->_createTooltip(IDC_BC_BRW01,  L"Select application or icon file");
  this->_createTooltip(IDC_BC_DEL,    L"Remove custom icon");

  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->context();
  if(!pCtx) return;

  this->setItemText(IDC_EC_INP01, pCtx->home());
  this->setItemText(IDC_EC_INP02, pCtx->uuid());
  this->setItemText(IDC_EC_INP03, pCtx->title());

  this->setItemText(IDC_EC_INP04, L"<invalid>"); //< hidden icon path

  // refresh with default values
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onResize()
{
  // home location Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 20, 64, 9);
  this->_setItemPos(IDC_EC_INP01, 70, 20, this->width()-90, 13);
  // Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 5, 60, 64, 9);
  this->_setItemPos(IDC_EC_INP03, 70, 60, this->width()-90, 13);
  // Icon Label & placeholder
  this->_setItemPos(IDC_SC_LBL04, 5, 90, 64, 9);
  this->_setItemPos(IDC_EC_INP04, 170, 90, 50, 13); // not visible
  this->_setItemPos(IDC_SB_CTICO, 70, 90, 30, 30);
  // Select & Remove Buttons
  this->_setItemPos(IDC_BC_BRW01, 110, 90, 50, 14);
  this->_setItemPos(IDC_BC_DEL, 110, 105, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onRefresh()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->context();
  if(!pCtx) return;

  wstring ctx_icon;

  this->getItemText(IDC_EC_INP04, ctx_icon);

  HICON hIc;

  // check if the path to icon is non empty
  if(Om_isValidPath(ctx_icon)) {

    // reload the last selected icon
    ExtractIconExW(ctx_icon.c_str(), 0, &hIc, nullptr, 1);
    this->setItemText(IDC_BC_BRW01, L"Change...");

  } else {

    // check whether Context already have an icon configured
    if(pCtx->icon()) {
      hIc = pCtx->icon();
      this->setItemText(IDC_BC_BRW01, L"Change...");
    } else {
      hIc = Om_loadShellIcon(SIID_APPLICATION, true);
      this->setItemText(IDC_BC_BRW01, L"Select...");
    }

  }

  hIc = reinterpret_cast<HICON>(this->msgItem(IDC_SB_CTICO, STM_SETICON, reinterpret_cast<WPARAM>(hIc)));
  if(hIc) DestroyIcon(hIc);

  InvalidateRect(this->getItem(IDC_SB_CTICO), nullptr, true);

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtxStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
