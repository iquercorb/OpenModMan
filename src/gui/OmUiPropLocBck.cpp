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
#include "gui/OmUiPropLocBck.h"
#include "gui/OmUiPropLoc.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocBck::OmUiPropLocBck(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i)
    this->_chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocBck::~OmUiPropLocBck()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropLocBck::id() const
{
  return IDD_PROP_LOC_BCK;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::setChParam(unsigned i, bool en)
{
  _chParam[i] = en;
  reinterpret_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CHK01,  L"Store backup data as zip archives");
  this->_createTooltip(IDC_CB_LEVEL,  L"Compression level for backup zip files");

  OmLocation* location = reinterpret_cast<OmUiPropLoc*>(this->_parent)->location();

  // add items in combo box
  HWND hCb = this->getItem(IDC_CB_LEVEL);

  unsigned cb_cnt = SendMessageW(hCb, CB_GETCOUNT, 0, 0);
  if(!cb_cnt) {
    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)L"None ( very fast )");
    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)L"Low ( fast )");
    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)L"Normal ( slow )");
    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)L"Best ( very slow )");
  }

  if(location == nullptr)
    return;

  int comp_levl = location->backupZipLevel();

  if(comp_levl >= 0) {

    this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 1);

    EnableWindow(hCb, true);

    switch(comp_levl)
    {
    case 1:
      SendMessageW(hCb, CB_SETCURSEL, 1, 0);
      break;
    case 2:
      SendMessageW(hCb, CB_SETCURSEL, 2, 0);
      break;
    case 3:
      SendMessageW(hCb, CB_SETCURSEL, 3, 0);
      break;
    default:
      SendMessageW(hCb, CB_SETCURSEL, 0, 0);
      break;
    }

  } else {

    this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 0);

    EnableWindow(hCb, false);

    SendMessageW(hCb, CB_SETCURSEL, 0, 0);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocBck::_onResize()
{
  // Compressed Backup CheckBox
  this->_setItemPos(IDC_BC_CHK01, 50, 20, 120, 9);
  // Compression level Label & ComboBox
  this->_setItemPos(IDC_SC_LBL01, 50, 40, 120, 9);
  this->_setItemPos(IDC_CB_LEVEL, 50, 50, this->width()-100, 14);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_LEVEL), nullptr, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLocBck::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool bm_chk;

    switch(LOWORD(wParam))
    {
    case IDC_BC_CHK01:
      bm_chk = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);
      this->enableItem(IDC_CB_LEVEL, bm_chk);
      this->setChParam(LOC_PROP_BCK_COMP_LEVEL, true);
      break;

    case IDC_CB_LEVEL:
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        // user modified parameter, notify it
        this->setChParam(LOC_PROP_BCK_COMP_LEVEL, true);
      }
      break;
    }
  }

  return false;
}
