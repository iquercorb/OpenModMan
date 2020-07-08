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
#include "gui/OmUiPropMan.h"
#include "gui/OmUiPropManGle.h"
#include "gui/OmUiPropManPkg.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropMan::OmUiPropMan(HINSTANCE hins) : OmDialogProp(hins)
{
  // create tab dialogs
  this->_addPage(L"General", new OmUiPropManGle(hins));
  this->_addPage(L"Packages", new OmUiPropManPkg(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropMan::~OmUiPropMan()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropMan::id() const
{
  return IDD_PROP_MAN;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropMan::checkChanges()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmUiPropManGle* uiPropManGle  = reinterpret_cast<OmUiPropManGle*>(this->childById(IDD_PROP_MAN_GLE));
  OmUiPropManPkg* uiPropManPkg  = reinterpret_cast<OmUiPropManPkg*>(this->childById(IDD_PROP_MAN_PKG));

  bool changed = false;

  if(uiPropManGle->hasChParam(MAN_PROP_GLE_ICON_SIZE)) {

    int cb_sel = SendMessageW(GetDlgItem(uiPropManGle->hwnd(), IDC_CB_ISIZE), CB_GETCURSEL, 0, 0);

    switch(cb_sel)
    {
    case 0:
      if(manager->iconsSize() != 16)
        changed = true;
      break;
    case 2:
      if(manager->iconsSize() != 32)
        changed = true;
      break;
    default:
      if(manager->iconsSize() != 24)
        changed = true;
      break;
    }
  }

  if(uiPropManGle->hasChParam(MAN_PROP_GLE_STARTUP_CONTEXTS)) {
    changed = true;
  }

  if(uiPropManPkg->hasChParam(MAN_PROP_PKG_PACKAGE_FLAGS)) {
    changed = true;
  }

  // enable Apply button
  if(IsWindowEnabled(GetDlgItem(this->_hwnd, IDC_BC_APPLY)) != changed) {
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), changed);
  }

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropMan::applyChanges()
{
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmUiPropManGle* uiPropManGle  = reinterpret_cast<OmUiPropManGle*>(this->childById(IDD_PROP_MAN_GLE));
  OmUiPropManPkg* uiPropManPkg  = reinterpret_cast<OmUiPropManPkg*>(this->childById(IDD_PROP_MAN_PKG));

  // Parameter: Icons size for packages List-View
  if(uiPropManGle->hasChParam(MAN_PROP_GLE_ICON_SIZE)) {

    int cb_sel = SendMessageW(GetDlgItem(uiPropManGle->hwnd(), IDC_CB_ISIZE), CB_GETCURSEL, 0, 0);

    switch(cb_sel)
    {
    case 0:
      manager->setIconsSize(16);
      break;
    case 2:
      manager->setIconsSize(32);
      break;
    default:
      manager->setIconsSize(24);
      break;
    }

    // Reset parameter as unmodified
    uiPropManGle->setChParam(MAN_PROP_GLE_ICON_SIZE, false);
  }

  // Parameter: Open Context(s) at startup
  if(uiPropManGle->hasChParam(MAN_PROP_GLE_STARTUP_CONTEXTS)) {

    HWND hLb = GetDlgItem(uiPropManGle->hwnd(), IDC_LB_STRLS);

    int lb_cnt =  SendMessage(hLb, LB_GETCOUNT, 0, 0);

    wchar_t wcbuf[MAX_PATH];

    vector<wstring> start_files;

    for(int i = 0; i < lb_cnt; ++i) {
      SendMessageW(hLb, LB_GETTEXT, i, (LPARAM)wcbuf);
      start_files.push_back(wcbuf);
    }

    bool chk01 = SendMessage(GetDlgItem(uiPropManGle->hwnd(), IDC_BC_CHK01), BM_GETCHECK, 0, 0);

    manager->saveStartContexts(chk01, start_files);

    // Reset parameter as unmodified
    uiPropManGle->setChParam(MAN_PROP_GLE_STARTUP_CONTEXTS, false);
  }

  // Parameter: Various Packages options
  if(uiPropManPkg->hasChParam(MAN_PROP_PKG_PACKAGE_FLAGS)) {
    manager->setLegacySupport(SendMessage(GetDlgItem(uiPropManPkg->hwnd(), IDC_BC_CHK01), BM_GETCHECK, 0, 0));
    manager->setWarnOverlaps(SendMessage(GetDlgItem(uiPropManPkg->hwnd(), IDC_BC_CHK02), BM_GETCHECK, 0, 0));
    manager->setWarnExtraInst(SendMessage(GetDlgItem(uiPropManPkg->hwnd(), IDC_BC_CHK03), BM_GETCHECK, 0, 0));
    manager->setWarnMissDpnd(SendMessage(GetDlgItem(uiPropManPkg->hwnd(), IDC_BC_CHK04), BM_GETCHECK, 0, 0));
    manager->setWarnExtraUnin(SendMessage(GetDlgItem(uiPropManPkg->hwnd(), IDC_BC_CHK05), BM_GETCHECK, 0, 0));
    manager->setQuietBatches(SendMessage(GetDlgItem(uiPropManPkg->hwnd(), IDC_BC_CHK06), BM_GETCHECK, 0, 0));

    // Reset parameter as unmodified
    uiPropManPkg->setChParam(MAN_PROP_PKG_PACKAGE_FLAGS, false);
  }

  // disable Apply button
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), false);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropMan::_onShow()
{
  // Initialize TabControl with pages dialogs
  this->_pagesOnShow(IDC_TC_TABS1);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropMan::_onResize()
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
void OmUiPropMan::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropMan::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropMan::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
      if(this->checkChanges()) {
        if(this->applyChanges()) {
          // quit the dialog
          this->quit();
          // refresh all tree from the main dialog
          this->root()->refresh();
        }
      } else {
        // quit the dialog
        this->quit();
      }
      break; // case BTN_OK:

    case IDC_BC_CANCEL:
      this->quit();
      break; // case BTN_CANCEL:
    }
  }

  return false;
}
