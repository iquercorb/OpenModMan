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
#include "gui/OmUiPropPkg.h"
#include "gui/OmUiPropPkgSrc.h"
#include "gui/OmUiPropPkgBck.h"



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPkg::OmUiPropPkg(HINSTANCE hins) : OmDialogProp(hins),
  _package(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"Source", new OmUiPropPkgSrc(hins));
  this->_addPage(L"Backup", new OmUiPropPkgBck(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropPkg::~OmUiPropPkg()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropPkg::id() const
{
  return IDD_PROP_PKG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkg::_onShow()
{
  // Initialize TabControl with pages dialogs
  this->_pagesOnShow(IDC_TC_TABS1);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkg::_onResize()
{
  // TabControl
  this->_setControlPos(IDC_TC_TABS1, 4, 5, this->width()-8, this->height()-28);
  // Resize page dialogs according IDC_TC_TABS1
  this->_pagesOnResize(IDC_TC_TABS1);
  // OK Button
  this->_setControlPos(IDC_BC_OK, this->width()-161, this->height()-19, 50, 14);
  // Cancel Button
  this->_setControlPos(IDC_BC_CANCEL, this->width()-108, this->height()-19, 50, 14);
  // Apply Button
  this->_setControlPos(IDC_BC_APPLY, this->width()-54, this->height()-19, 50, 14);

  // force buttons to redraw to prevent artifacts
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_OK), nullptr, true);
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_CANCEL), nullptr, true);
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_APPLY), nullptr, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkg::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropPkg::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropPkg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {
    // handle TabControl page selection change
    this->_pagesOnNotify(IDC_TC_TABS1, wParam, lParam);
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_APPLY:
      if(this->applyChanges()) {
        // refresh all tree from the main dialog
        this->root()->refresh();
      }
      break;

    case IDC_BC_OK:
      if(this->applyChanges()) {
        // quit the dialog
        this->quit();
        // refresh all tree from the main dialog
        this->root()->refresh();
      }
      break; // case BTN_OK:

    case IDC_BC_CANCEL:
      this->quit();
      break; // case BTN_CANCEL:
    }
  }

  return false;
}
