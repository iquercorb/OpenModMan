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
  reinterpret_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onShow()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT3,  L"Indicative name");

  this->_createTooltip(IDC_BC_BROW1,  L"Select application or icon file");
  this->_createTooltip(IDC_BC_DEL,    L"Remove custom icon");

  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onResize()
{
  // home location Label & EditControl
  this->_setControlPos(IDC_SC_LBL01, 5, 20, 64, 9);
  this->_setControlPos(IDC_EC_INPT1, 70, 20, this->width()-90, 13);
  // Title Label & EditControl
  this->_setControlPos(IDC_SC_LBL03, 5, 60, 64, 9);
  this->_setControlPos(IDC_EC_INPT3, 70, 60, this->width()-90, 13);
  // Icon Label & placeholder
  this->_setControlPos(IDC_SC_LBL04, 5, 90, 64, 9);
  this->_setControlPos(IDC_EC_INPT4, 170, 90, 50, 13); // not visible
  this->_setControlPos(IDC_SB_CTICO, 70, 90, 30, 30);
  // Select & Remove Buttons
  this->_setControlPos(IDC_BC_BROW1, 110, 90, 50, 14);
  this->_setControlPos(IDC_BC_DEL, 110, 105, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onRefresh()
{
  OmContext* context = reinterpret_cast<OmUiPropCtx*>(this->_parent)->context();

  if(context == nullptr)
    return;

  SetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, context->home().c_str());
  SetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, context->uuid().c_str());
  SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, context->title().c_str());

  wchar_t icon_src[OMM_MAX_PATH];
  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, icon_src, OMM_MAX_PATH);

  // check if the path to icon is non empty
  if(wcslen(icon_src) && Om_isValidName(icon_src)) {
    // reload the last selected icon
    HICON hIcon = nullptr;
    ExtractIconExW(icon_src, 0, &hIcon, nullptr, 1);
    if(hIcon) {
      SendMessage(GetDlgItem(this->_hwnd, IDC_SB_CTICO), STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
      SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, icon_src);
      DeleteObject(hIcon);
    }
    // change browse button text
    SetDlgItemTextW(this->_hwnd, IDC_BC_BROW1, L"Change...");

  } else {
    // check whether Context already have an icon configured
    if(context->icon()) {
      SendMessage(GetDlgItem(this->_hwnd, IDC_SB_CTICO), STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)context->icon());
      // change browse button text
      SetDlgItemTextW(this->_hwnd, IDC_BC_BROW1, L"Change...");

    } else {

      //HICON hIcon = (HICON)LoadImage(this->_hins, MAKEINTRESOURCE(IDC_SB_CTICO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
      HICON hIcon = (HICON)Om_loadShellIcon(SIID_APPLICATION,true);
      SendMessage(GetDlgItem(this->_hwnd, IDC_SB_CTICO), STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
      // change browse button text
      SetDlgItemTextW(this->_hwnd, IDC_BC_BROW1, L"Select...");
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxStg::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtxStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    OmContext* context = reinterpret_cast<OmUiPropCtx*>(this->_parent)->context();

    if(context == nullptr)
      return false;

    HICON hIcon;
    wchar_t wcbuf[OMM_MAX_PATH];
    wchar_t sldir[OMM_MAX_PATH];

    switch(LOWORD(wParam))
    {
    case IDC_EC_INPT3: //< Entry for Context title
      // user modified parameter, notify it
      this->setChParam(CTX_PROP_STG_TITLE, true);
      break;

    case IDC_BC_BROW1: //< Brows Button for Context icon
      // get last valid path to start browsing
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf, OMM_MAX_PATH);
      wcscpy(sldir, Om_getDirPart(wcbuf).c_str());

      if(Om_dialogOpenFile(wcbuf, this->_parent->hwnd(), L"Select Context icon.", ICON_FILES_FILTER, sldir)) {
        ExtractIconExW(wcbuf, 0, &hIcon, nullptr, 1);
        if(hIcon) {
          SendMessage(GetDlgItem(this->_hwnd, IDC_SB_CTICO), STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
          RedrawWindow(GetDlgItem(this->_hwnd, IDC_SB_CTICO), nullptr, nullptr, RDW_INVALIDATE|RDW_ERASE); // force repaint
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf);
          DeleteObject(hIcon);
        }
        // user modified parameter, notify it
        this->setChParam(CTX_PROP_STG_ICON, true);
      }
      break;

    case IDC_BC_DEL: //< Remove Button for Context icon
      // load default icon
      hIcon = (HICON)Om_loadShellIcon(SIID_APPLICATION, true);
      SendMessage(GetDlgItem(this->_hwnd, IDC_SB_CTICO), STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
      RedrawWindow(GetDlgItem(this->_hwnd, IDC_SB_CTICO), nullptr, nullptr, RDW_INVALIDATE|RDW_ERASE); // force repaint
      // set invalid path, this will remove link at validation
      SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, L"<delete>");
      // change browse button text
      SetDlgItemTextW(this->_hwnd, IDC_BC_BROW1, L"Select...");
      // user modified parameter, notify it
      this->setChParam(CTX_PROP_STG_ICON, true);
      break;
    }
  }

  return false;
}
