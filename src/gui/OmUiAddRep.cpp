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
#include "gui/OmUiAddRep.h"
#include "OmManager.h"
#include "OmSocket.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiAddRep::OmUiAddRep(HINSTANCE hins) : OmDialog(hins),
  _pLoc(nullptr),
  _testResult(0)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiAddRep::~OmUiAddRep()
{
  HFONT hFt;
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_SC_STATE, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_RESUL, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiAddRep::id() const
{
  return IDD_ADD_REP;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddRep::_testLog(const wstring& log)
{
  wstring entry;

  // get local time
  int t_h, t_m, t_s;
  Om_getTime(&t_s, &t_m, &t_h);

  wchar_t hour[32];
  swprintf(hour, 32, L"[%02d:%02d:%02d]  ", t_h, t_m, t_s);

  entry = hour + log;

  unsigned s = this->msgItem(IDC_EC_RESUL, WM_GETTEXTLENGTH, 0, 0);

  this->msgItem(IDC_EC_RESUL, EM_SETSEL, s, s);
  this->msgItem(IDC_EC_RESUL, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(entry.c_str()));
  this->msgItem(IDC_EC_RESUL, WM_VSCROLL, SB_BOTTOM, 0);
  this->msgItem(IDC_EC_RESUL, WM_VSCROLL, SB_BOTTOM, 0);

  RedrawWindow(this->getItem(IDC_EC_RESUL), nullptr, nullptr, RDW_ERASE|RDW_INVALIDATE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddRep::_onBcChk()
{
  OmSocket sock;

  this->setItemText(IDC_EC_RESUL, L"");
  this->setItemText(IDC_SC_STATE, L"Pending...");

  wstring rep_base;
  wstring rep_name;

  this->getItemText(IDC_EC_INP01, rep_base);
  this->getItemText(IDC_EC_INP02, rep_name);

  wstring url = rep_base + L"/";
  url += rep_name + L".xml";

  if(!Om_isValidFileUrl(url)) {
    Om_msgBox_okl(this->_hwnd, L"Add Network Repository - " OMM_APP_NAME, IDI_ERR,
                  L"Invalid Network Repository parameters", L"The specified "
                  "Network Repository base address and name combination make "
                  "no valid file access URL:", url);
  }

  this->_testResult = -1;

  string data;

  this->_testLog(L"HTTP GET request: "+url+L"\r\n");

  if(sock.httpGet(url, data)) {

    this->_testLog(L"HTTP GET succeed: "+to_wstring(data.size())+L" bytes received\r\n");

    OmConfig config;

    if(config.parse(Om_fromUtf8(data.c_str()), OMM_CFG_SIGN_REP)) {
      this->_testLog(L"XML parse succeed.\r\n");
      int n = config.xml().child(L"remotes").attrAsInt(L"count");
      this->_testLog(L"Repository provides " + std::to_wstring(n) + L" package(s)\r\n");
      this->setItemText(IDC_SC_STATE, L"The Repository appear to be valid !");
      this->_testResult = 1;
    } else {
      this->_testLog(L"XML parse failed.\r\n");
      this->setItemText(IDC_SC_STATE, L"Error: Invalid XML definition");
    }
  } else {
    this->_testLog(L"HTTP GET failed: "+sock.lastErrorStr()+L"\r\n");
    this->setItemText(IDC_SC_STATE, L"Error: HTTP request failed");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiAddRep::_onBcOk()
{
  if(!this->_pLoc) return false;

  if(this->_testResult == 0) {


    if(!Om_msgBox_yn(this->_hwnd, L"Add Network Repository - " OMM_APP_NAME, IDI_ERR,
                  L"Network Repository not tested", L"You did not tested the "
                  "Repository, it may be invalid or unavailable. "
                  "Do you want to add it anyway ?"))
       return false;


  } else if(this->_testResult == -1) {

    if(!Om_msgBox_yn(this->_hwnd, L"Add Network Repository - " OMM_APP_NAME, IDI_ERR,
                  L"Network Repository not tested", L"The last Repository test "
                  "failed, it appear to be invalid or unavailable. "
                  "Do you want to add it anyway ?"))
       return false;
  }

  wstring rep_base;
  wstring rep_name;

  this->getItemText(IDC_EC_INP01, rep_base);
  this->getItemText(IDC_EC_INP02, rep_name);

  wstring url = rep_base + L"/";
  url += rep_name + L".xml";

  if(!Om_isValidFileUrl(url)) {

    Om_msgBox_okl(this->_hwnd, L"Add Network Repository - " OMM_APP_NAME, IDI_ERR,
                  L"Invalid Network Repository parameters", L"The specified "
                  "Network Repository base address and name combination make "
                  "no valid file access URL:", url);

    return false;
  }

  // add new repository in Context
  if(!this->_pLoc->repAdd(rep_base, rep_name)) {

    Om_msgBox_okl(this->_hwnd, L"Add Network Repository - " OMM_APP_NAME, IDI_ERR,
                 L"Add Network Repository error", L"Network Repository "
                 "cannot be added because of the following error:",
                 this->_pLoc->lastError());
  }

  this->quit();

  // refresh parent dialog
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddRep::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins,IDI_APP,2),Om_getResIcon(this->_hins,IDI_APP,1));

  HFONT hFt;
  // set specific fonts
  hFt = Om_createFont(12, 800, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_STATE, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(12, 400, L"Consolas");
  this->msgItem(IDC_EC_RESUL, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Repository base URL, the root HTTP address");
  this->_createTooltip(IDC_EC_INP02,  L"Repository name, the repository identifier");
  this->_createTooltip(IDC_BC_QRY,    L"Query the Repository to check its availability");
  this->_createTooltip(IDC_SC_STATE,  L"Repository query test result");
  this->_createTooltip(IDC_EC_RESUL,  L"Repository query test logs");

  // set default start values
  this->setItemText(IDC_EC_INP01, L"http://");
  this->setItemText(IDC_EC_INP02, L"default");
  this->setItemText(IDC_SC_STATE, L"");

  // Set caret at end of string
  this->msgItem(IDC_EC_INP01, EM_SETSEL, 0, -1);
  this->msgItem(IDC_EC_INP01, EM_SETSEL, -1, -1);

  this->enableItem(IDC_BC_OK, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddRep::_onResize()
{
  // Repository address Label
  this->_setItemPos(IDC_SC_LBL01, 10, 10, 80, 9);

  // Repository URL Label & EditText
  this->_setItemPos(IDC_SC_LBL02, 10, 27, 22, 9);
  this->_setItemPos(IDC_EC_INP01, 31, 25, this->width()-155, 13);

  // Repository Name Label & EditText
  this->_setItemPos(IDC_EC_INP02, this->width()-89, 25, 79, 13); //< resize EditText first to prevent graphical glitchs
  this->_setItemPos(IDC_SC_LBL03, this->width()-115, 27, 23, 9);
  // force Label to repaint by invalidate rect
  InvalidateRect(this->getItem(IDC_SC_LBL03), nullptr, true);

  // Repository Test Label, Button and Result
  this->_setItemPos(IDC_SC_LBL04, 10, 55, 80, 9);
  this->_setItemPos(IDC_BC_QRY, 10, 70, 50, 14);
  this->_setItemPos(IDC_SC_STATE, 65, 73, this->width()-20, 12);

  // Repository Test content
  this->_setItemPos(IDC_EC_RESUL, 10, 90, this->width()-20, this->height()-130);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Ok and Cancel buttons
  this->_setItemPos(IDC_BC_OK, this->width()-110, this->height()-19, 50, 14);
  this->_setItemPos(IDC_BC_CANCEL, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiAddRep::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    wstring item_str;

    switch(LOWORD(wParam))
    {

    case IDC_EC_INP01: // Title
      if(HIWORD(wParam) == EN_CHANGE) {
        this->getItemText(IDC_EC_INP01, item_str);
        this->enableItem(IDC_BC_QRY, (item_str.size() > 7));
        has_changed = true;
      }
      break;

    case IDC_BC_QRY:
      this->_onBcChk();
      break;

    case IDC_BC_OK:
      this->_onBcOk();
      break;

    case IDC_BC_CANCEL:
      this->quit();
      break;
    }

    // enable or disable "OK" button according values
    if(has_changed) {
      bool allow = true;
      this->enableItem(IDC_BC_OK, allow);
    }
  }

  return false;
}

