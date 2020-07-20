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
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_CB_ISIZE,  L"Packages list icon size");

  this->_createTooltip(IDC_BC_CHK01,  L"Automatically load context at application start");
  this->_createTooltip(IDC_LB_STRLS,  L"Context files");
  this->_createTooltip(IDC_BC_BROW1,  L"Select a context file");
  this->_createTooltip(IDC_BC_DEL,    L"Remove the selected entry");

  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  // add items in combo box
  HWND hCb = this->getItem(IDC_CB_ISIZE);

  unsigned cb_cnt = SendMessageW(hCb, CB_GETCOUNT, 0, 0);
  if(!cb_cnt) {
    SendMessageW(hCb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Small"));
    SendMessageW(hCb, CB_ADDSTRING, 1, reinterpret_cast<LPARAM>(L"Medium"));
    SendMessageW(hCb, CB_ADDSTRING, 2, reinterpret_cast<LPARAM>(L"Large"));
  }

  switch(pMgr->iconsSize()) {
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
  pMgr->getStartContexts(&enable, start_files);

  this->msgItem(IDC_BC_CHK01, BM_SETCHECK, enable);
  this->enableItem(IDC_BC_BROW1, enable);

  HWND hLb = this->getItem(IDC_LB_STRLS);
  EnableWindow(hLb, enable);
  SendMessageW(hLb, LB_RESETCONTENT, 0, 0);
  for(size_t i = 0; i < start_files.size(); ++i) {
    SendMessageW(hLb, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(start_files[i].c_str()));
  }

  this->enableItem(IDC_BC_DEL, false);

  SetFocus(hCb);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropManGle::_onResize()
{
  // Icon size Label & ComboBox
  this->_setItemPos(IDC_SC_LBL01, 50, 20, 100, 9);
  this->_setItemPos(IDC_CB_ISIZE, 50, 30, this->width()-100, 14);
  // force ComboBox to repaint by invalidate rect, else it randomly disappears on resize
  InvalidateRect(this->getItem(IDC_CB_ISIZE), nullptr, true);

  // Startup Contexts list CheckBox & ListBox
  this->_setItemPos(IDC_BC_CHK01, 50, 59, 100, 9);
  this->_setItemPos(IDC_LB_STRLS, 50, 70, this->width()-100, this->height()-130);

  // Startup Contexts list Add and Remove... buttons
  this->_setItemPos(IDC_BC_BROW1, 50, this->height()-58, 50, 14);
  this->_setItemPos(IDC_BC_DEL, 102, this->height()-58, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropManGle::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    int lb_sel;
    bool bm_chk;
    wstring item_str;
    wstring brow_str;

    switch(LOWORD(wParam))
    {
    case IDC_CB_ISIZE: //< Combo-Box for icons size
      // parameter modified, must be saved we parent dialog valid changes
      this->setChParam(MAN_PROP_GLE_ICON_SIZE, true);
      break;

    case IDC_BC_CHK01: //< Check-Box for Open Context(s) at startup
      bm_chk = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);
      this->enableItem(IDC_LB_STRLS, bm_chk);
      this->enableItem(IDC_BC_BROW1, bm_chk);
      // user modified parameter, notify it
      this->setChParam(MAN_PROP_GLE_STARTUP_CONTEXTS, true);
      break;

    case IDC_LB_STRLS: //< List-Box for startup Context(s) list
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        lb_sel = this->msgItem(IDC_LB_STRLS, LB_GETCURSEL);
        this->enableItem(IDC_BC_DEL, (lb_sel >= 0));
      }
      break;

    case IDC_BC_BROW1: //< Brows Button for startup Context
      this->getItemText(IDC_EC_INPT1, item_str);
      if(Om_dialogOpenFile(brow_str, this->_hwnd, L"Select Context file", OMM_CTX_DEF_FILE_FILER, item_str)) {
        // add file path to startup context list
        this->msgItem(IDC_LB_STRLS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(brow_str.c_str()));
      }
      // user modified parameter, notify it
      this->setChParam(MAN_PROP_GLE_STARTUP_CONTEXTS, true);
      break; // case BTN_BROWSE1:

    case IDC_BC_DEL: //< Remove Button for startup Context
      lb_sel = this->msgItem(IDC_LB_STRLS, LB_GETCURSEL);
      if(lb_sel >= 0) this->msgItem(IDC_LB_STRLS, LB_DELETESTRING, lb_sel);
      // user modified parameter, notify it
      this->setChParam(MAN_PROP_GLE_STARTUP_CONTEXTS, true);
      break; // case BTN_BROWSE1:
    }
  }

  return false;
}
