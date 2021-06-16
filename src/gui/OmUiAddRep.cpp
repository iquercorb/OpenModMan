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
  _location(nullptr),
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
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_OUT01, WM_GETFONT));
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

  unsigned s = this->msgItem(IDC_EC_OUT01, WM_GETTEXTLENGTH, 0, 0);

  this->msgItem(IDC_EC_OUT01, EM_SETSEL, s, s);
  this->msgItem(IDC_EC_OUT01, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(entry.c_str()));
  this->msgItem(IDC_EC_OUT01, WM_VSCROLL, SB_BOTTOM, 0);
  this->msgItem(IDC_EC_OUT01, WM_VSCROLL, SB_BOTTOM, 0);

  RedrawWindow(this->getItem(IDC_EC_OUT01), nullptr, nullptr, RDW_ERASE|RDW_INVALIDATE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddRep::_onBcChk()
{
  OmSocket sock;

  this->setItemText(IDC_EC_OUT01, L"");
  this->setItemText(IDC_SC_STATE, L"Pending...");

  wstring rep_base;
  wstring rep_name;

  this->getItemText(IDC_EC_INP01, rep_base);
  this->getItemText(IDC_EC_INP02, rep_name);

  wstring url = rep_base + L"/";
  url += rep_name + L".xml";

  if(!Om_isValidUrl(url)) {

    wstring err = L"Supplied parameters cannot be used to "
                  L"create a valid HTTP address:";

    err += L"\n\n";
    err += url;

    Om_dialogBoxErr(this->_hwnd, L"Invalid parameters", err);
  }

  this->_testResult = -1;

  string rep_xml;

  this->_testLog(L"HTTP GET request: "+url+L"\r\n");

  if(sock.httpGet(Om_toUtf8(url), rep_xml)) {

    this->_testLog(L"HTTP GET succeed: "+to_wstring(rep_xml.size())+L" bytes received\r\n");

    OmConfig rep_def;

    if(rep_def.parse(Om_fromUtf8(rep_xml.c_str()), OMM_CFG_SIGN_REP)) {
      this->_testLog(L"XML parse succeed.\r\n");
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
  OmLocation* pLoc = this->_location;

  if(pLoc == nullptr)
    return false;

  if(this->_testResult == 0) {

    wstring wrn = L"You did not tested the Repository, it may be invalid "
                  L"or unavailable, do you want to continue anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"The Repository was not tested", wrn)) {
      return false;
    }

  } else if(this->_testResult == -1) {

    wstring wrn = L"The last Repository test failed, the Repository "
                  L"appear to be invalid or unavailable, do you want to "
                  L"continue anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"The Repository appear invalid", wrn)) {
      return false;
    }
  }

  wstring rep_base;
  wstring rep_name;

  this->getItemText(IDC_EC_INP01, rep_base);
  this->getItemText(IDC_EC_INP02, rep_name);

  wstring url = rep_base + L"/";
  url += rep_name + L".xml";

  if(!Om_isValidUrl(url)) {

    wstring err = L"Supplied parameters cannot be used to "
                  L"create a valid HTTP address";

    Om_dialogBoxErr(this->_hwnd, L"Invalid parameters", err);

    return false;
  }

  // add new repository in Context
  if(!pLoc->addRepository(rep_base, rep_name)) {
    Om_dialogBoxErr(this->_hwnd, L"Repository creation failed", pLoc->lastError());
  }

  // refresh parent dialog
  this->parent()->refresh();

  this->quit();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddRep::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Repository base URL");
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Repository name");

  HFONT hFt;
  // set specific fonts
  hFt = Om_createFont(12, 800, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_STATE, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(12, 400, L"Consolas");
  this->msgItem(IDC_EC_OUT01, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

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
  this->_setItemPos(IDC_BC_CHK, 10, 70, 50, 14);
  this->_setItemPos(IDC_SC_STATE, 65, 73, this->width()-20, 12);

  // Repository Test content
  this->_setItemPos(IDC_EC_OUT01, 10, 90, this->width()-20, this->height()-130);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Ok and Cancel buttons
  this->_setItemPos(IDC_BC_OK, this->width()-110, this->height()-19, 50, 14);
  this->_setItemPos(IDC_BC_CANCEL, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiAddRep::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    wstring item_str;

    switch(LOWORD(wParam))
    {

    case IDC_EC_INP01: // Title
      if(HIWORD(wParam) == EN_CHANGE) {
        this->getItemText(IDC_EC_INP01, item_str);
        this->enableItem(IDC_BC_CHK, (item_str.size() > 7));
        has_changed = true;
      }
      break;

    case IDC_BC_CHK:
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

