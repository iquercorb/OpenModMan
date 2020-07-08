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
#include "gui/OmUiPropManGle.h"
#include "OmDialogProp.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropManGle::OmUiPropManGle(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i)
    this->_chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropManGle::~OmUiPropManGle()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropManGle::id() const
{
  return IDD_PROP_MAN_GLE;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::setChParam(unsigned i, bool en)
{
  _chParam[i] = en;
  reinterpret_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onShow()
{
  // define controls tool-tips
  this->_createTooltip(IDC_CB_ISIZE,  L"Packages list icon size");

  this->_createTooltip(IDC_BC_CHK01,  L"Automatically load context at application start");
  this->_createTooltip(IDC_LB_STRLS,  L"Context files");
  this->_createTooltip(IDC_BC_BROW1,  L"Select a context file");
  this->_createTooltip(IDC_BC_DEL,    L"Remove the selected entry");

  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);

  // add items in combo box
  HWND hCb = GetDlgItem(this->_hwnd, IDC_CB_ISIZE);

  unsigned cb_cnt = SendMessageW(hCb, CB_GETCOUNT, 0, 0);
  if(!cb_cnt) {
    SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)"Small");
    SendMessage(hCb, CB_ADDSTRING, 1, (LPARAM)"Medium");
    SendMessage(hCb, CB_ADDSTRING, 2, (LPARAM)"Large");
  }

  switch(manager->iconsSize()) {
  case 16:
    SendMessageW(hCb, CB_SETCURSEL, 0, 0);
    break;
  case 32:
    SendMessageW(hCb, CB_SETCURSEL, 2, 0);
    break;
  default:
    SendMessageW(hCb, CB_SETCURSEL, 1, 0);
    break;
  }

  bool enable;
  vector<wstring> start_files;
  manager->getStartContexts(&enable, start_files);

  HWND hLb = GetDlgItem(this->_hwnd, IDC_LB_STRLS);

  if(enable) {
    SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01), BM_SETCHECK, 1, 0);
    EnableWindow(hLb, true);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW1), true);
  } else {
    SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01), BM_SETCHECK, 0, 0);
    EnableWindow(hLb, false);
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW1), false);
  }

  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);

  SendMessageW(hLb, LB_RESETCONTENT, 0, 0);
  for(unsigned i = 0; i < start_files.size(); ++i) {
    SendMessageW(hLb, LB_ADDSTRING, 0, (LPARAM)start_files[i].c_str());
  }

  SetFocus(GetDlgItem(this->_hwnd, IDC_CB_ISIZE));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onResize()
{
  // Icon size Label & ComboBox
  this->_setControlPos(IDC_SC_LBL01, 50, 20, 100, 9);
  this->_setControlPos(IDC_CB_ISIZE, 50, 30, this->width()-100, 14);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_CB_ISIZE), nullptr, true);

  // Startup Contexts list CheckBox & ListBox
  this->_setControlPos(IDC_BC_CHK01, 50, 59, 100, 9);
  this->_setControlPos(IDC_LB_STRLS, 50, 70, this->width()-100, this->height()-130);

  // Startup Contexts list Add and Remove... buttons
  this->_setControlPos(IDC_BC_BROW1, 50, this->height()-58, 50, 14);
  this->_setControlPos(IDC_BC_DEL, 102, this->height()-58, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropManGle::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    int lb_sel;
    wchar_t wcbuf[MAX_PATH];

    switch(LOWORD(wParam))
    {
    case IDC_CB_ISIZE: //< Combo-Box for icons size
      // parameter modified, must be saved we parent dialog valid changes
      this->setChParam(MAN_PROP_GLE_ICON_SIZE, true);
      break;

    case IDC_BC_CHK01: //< Check-Box for Open Context(s) at startup
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK01), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_STRLS), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW1), true);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_STRLS), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW1), false);
      }
      // user modified parameter, notify it
      this->setChParam(MAN_PROP_GLE_STARTUP_CONTEXTS, true);
      break;

    case IDC_LB_STRLS: //< List-Box for startup Context(s) list
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        if(SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_STRLS), LB_GETCURSEL, 0, 0) >= 0) {
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), true);
        } else {
          EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);
        }
      }
      break;

    case IDC_BC_BROW1: //< Brows Button for startup Context
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, MAX_PATH);
      if(Om_dialogOpenFile(wcbuf, this->_hwnd, L"Select file.", OMM_CTX_DEF_FILE_FILER, wcbuf)) {
        SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_STRLS), LB_ADDSTRING, 0, (LPARAM)wcbuf);
      }
      // user modified parameter, notify it
      this->setChParam(MAN_PROP_GLE_STARTUP_CONTEXTS, true);
      break; // case BTN_BROWSE1:

    case IDC_BC_DEL: //< Remove Button for startup Context
      lb_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_STRLS), LB_GETCURSEL, 0, 0);
      if(lb_sel >= 0) {
        SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_STRLS), LB_DELETESTRING, lb_sel, 0);
      }
      // user modified parameter, notify it
      this->setChParam(MAN_PROP_GLE_STARTUP_CONTEXTS, true);
      break; // case BTN_BROWSE1:

    case IDOK:
      this->quit();
      break;
    }
  }

  return false;
}
