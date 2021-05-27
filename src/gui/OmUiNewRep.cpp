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
#include "gui/OmUiNewRep.h"
#include "OmManager.h"
#include "OmSocket.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewRep::OmUiNewRep(HINSTANCE hins) : OmDialog(hins),
  _location(nullptr),
  _hFtMonos(Om_createFont(12, 400, L"Consolas")),
  _hFtHeavy(Om_createFont(12, 800, L"Ms Shell Dlg")),
  _check(0)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewRep::~OmUiNewRep()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiNewRep::id() const
{
  return IDD_NEW_REP;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewRep::_log(const wstring& log)
{
  wstring entry;

  // get local time
  int t_h, t_m, t_s;
  Om_getTime(&t_s, &t_m, &t_h);

  wchar_t hour[32];
  swprintf(hour, 32, L"[%02d:%02d:%02d]  ", t_h, t_m, t_s);

  entry = hour + log;

  unsigned s = this->msgItem(IDC_EC_ENT01, WM_GETTEXTLENGTH, 0, 0);

  this->msgItem(IDC_EC_ENT01, EM_SETSEL, s, s);
  this->msgItem(IDC_EC_ENT01, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(entry.c_str()));
  this->msgItem(IDC_EC_ENT01, WM_VSCROLL, SB_BOTTOM, 0);
  this->msgItem(IDC_EC_ENT01, WM_VSCROLL, SB_BOTTOM, 0);

  RedrawWindow(this->getItem(IDC_EC_ENT01), nullptr, nullptr, RDW_ERASE|RDW_INVALIDATE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewRep::_repoChk()
{
  OmSocket sock;

  this->setItemText(IDC_EC_ENT01, L"");
  this->setItemText(IDC_SC_DESC1, L"Pending...");

  wstring rep_base;
  wstring rep_name;

  this->getItemText(IDC_EC_INPT1, rep_base);
  this->getItemText(IDC_EC_INPT2, rep_name);

  wstring url = rep_base + L"/";
  url += rep_name + L".xml";

  if(!Om_isValidUrl(Om_toUtf8(url))) {

    wstring err = L"Supplied parameters cannot be used to "
                  L"create a valid HTTP address:";

    err += L"\n\n";
    err += url;

    Om_dialogBoxErr(this->_hwnd, L"Invalid parameters", err);
  }

  this->_check = -1;

  string rep_xml;

  this->_log(L"HTTP GET request: "+url+L"\r\n");

  if(sock.httpGet(Om_toUtf8(url), rep_xml)) {

    this->_log(L"HTTP GET succeed: "+to_wstring(rep_xml.size())+L" bytes received\r\n");

    OmConfig rep_def;

    if(rep_def.parse(Om_fromUtf8(rep_xml.c_str()), OMM_CFG_SIGN_REP)) {
      this->_log(L"XML parse succeed: "+to_wstring(rep_def.xml().childCount(L"package"))+L" package(s) provided\r\n");
      this->setItemText(IDC_SC_DESC1, L"The Repository appear to be valid !");
      this->_check = 1;
    } else {
      this->_log(L"XML parse failed.\r\n");
      this->setItemText(IDC_SC_DESC1, L"Error: Invalid XML definition");
    }
  } else {
    this->_log(L"HTTP GET failed: "+sock.lastErrorStr()+L"\r\n");
    this->setItemText(IDC_SC_DESC1, L"Error: HTTP request failed");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewRep::_apply()
{
  OmLocation* pLoc = this->_location;

  if(pLoc == nullptr)
    return false;

  if(this->_check == 0) {

    wstring wrn = L"You did not tested the Repository, it may be invalid "
                  L"or unavailable, do you want to continue anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"The Repository was not tested", wrn)) {
      return false;
    }

  } else if(this->_check == -1) {

    wstring wrn = L"The last Repository test failed, the Repository "
                  L"appear to be invalid or unavailable, do you want to "
                  L"continue anyway ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"The Repository appear invalid", wrn)) {
      return false;
    }
  }

  wstring rep_base;
  wstring rep_name;

  this->getItemText(IDC_EC_INPT1, rep_base);
  this->getItemText(IDC_EC_INPT2, rep_name);

  wstring url = rep_base + L"/";
  url += rep_name + L".xml";

  if(!Om_isValidUrl(Om_toUtf8(url))) {

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
void OmUiNewRep::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Repository base URL");
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Repository name");

  // set specific fonts
  this->msgItem(IDC_SC_DESC1, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtHeavy), true);
  this->msgItem(IDC_EC_ENT01, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtMonos), true);

  // set default start values
  this->setItemText(IDC_EC_INPT1, L"http://");
  this->setItemText(IDC_EC_INPT2, L"default");
  this->setItemText(IDC_SC_DESC1, L"");

  // Set caret at end of string
  this->msgItem(IDC_EC_INPT1, EM_SETSEL, 0, -1);
  this->msgItem(IDC_EC_INPT1, EM_SETSEL, -1, -1);

  this->enableItem(IDC_BC_OK, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewRep::_onResize()
{
  // Repository URL Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 10, 10, 80, 9);
  this->_setItemPos(IDC_EC_INPT1, 10, 20, this->width()-105, 13);

  // Repository Name Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, this->width()-90, 10, 80, 9);
  this->_setItemPos(IDC_EC_INPT2, this->width()-90, 20, 80, 13);

  // Repository Test Label, Button and Result
  this->_setItemPos(IDC_SC_LBL03, 10, 40, 80, 9);
  this->_setItemPos(IDC_BC_CHK, 10, 50, 50, 14);
  this->_setItemPos(IDC_SC_DESC1, 62, 53, this->width()-20, 12);

  // Repository Test content
  this->_setItemPos(IDC_EC_ENT01, 10, 70, this->width()-20, this->height()-110);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Ok and Cancel buttons
  this->_setItemPos(IDC_BC_OK, this->width()-110, this->height()-19, 50, 14);
  this->_setItemPos(IDC_BC_CANCEL, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewRep::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    wstring item_str, brow_str;

    switch(LOWORD(wParam))
    {

    case IDC_EC_INPT1: // Title
      this->getItemText(IDC_EC_INPT1, item_str);
      this->enableItem(IDC_BC_CHK, (item_str.size() > 7));
      has_changed = true;
      break;

    case IDC_BC_CHK:
      this->_repoChk();
      break;

    case IDC_BC_OK:
      this->_apply();
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

