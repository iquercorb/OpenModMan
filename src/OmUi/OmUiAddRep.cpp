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
#include "OmBase.h"
#include "OmBaseUi.h"
#include "OmBaseApp.h"

#include "OmModMan.h"

#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilSys.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiAddRep.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiAddRep::OmUiAddRep(HINSTANCE hins) : OmDialog(hins),
  _ModChan(nullptr),
  _qry_result(OM_RESULT_UNKNOW)
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
void OmUiAddRep::_qry_addlog(const OmWString& log)
{
  OmWString entry;

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
void OmUiAddRep::_qry_reponse_fn(void* ptr, uint8_t* buf, uint64_t len, uint64_t code)
{
  OmUiAddRep* self = static_cast<OmUiAddRep*>(ptr);

  OmWString log_entry;

  if(code == 200) {

    log_entry.assign(L"HTTP request success: "); log_entry.append(std::to_wstring(len)); log_entry.append(L" bytes received\r\n");
    self->_qry_addlog(log_entry);

    char* str = reinterpret_cast<char*>(buf);

    OmXmlConf xml_def;

    // try to parse received data as repository
    if(!xml_def.parse(Om_toUTF16(str), OM_XMAGIC_REP)) {

      log_entry.assign(L"Definition parse error: "); log_entry.append(xml_def.lastErrorStr()); log_entry.append(L"\r\n");
      self->_qry_addlog(log_entry);

      self->_qry_result = OM_RESULT_ERROR;

      self->setItemText(IDC_SC_STATE, L"XML definition parse error");

      return;
    }

    if(!xml_def.hasChild(L"remotes")) {

      log_entry.assign(L"Definition parse error: <remotes> node not found\r\n");
      self->_qry_addlog(log_entry);

      self->_qry_result = OM_RESULT_ERROR;

      self->setItemText(IDC_SC_STATE, L"XML definition parse error");

      return;
    }

    size_t mod_count = xml_def.child(L"remotes").attrAsInt(L"count");

    log_entry.assign(L"Definition parse success: "); log_entry.append(std::to_wstring(mod_count)); log_entry.append(L" Mods referenced\r\n");
    self->_qry_addlog(log_entry);

    self->_qry_result = OM_RESULT_OK;

    self->setItemText(IDC_SC_STATE, L"Valid");

  } else {
    self->_qry_result = OM_RESULT_ERROR;

    log_entry.assign(L"HTTP request failed: "); log_entry.append(self->_connect.lastError()); log_entry.append(L"\r\n");
    self->_qry_addlog(log_entry);

    self->setItemText(IDC_SC_STATE, self->_connect.lastError());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiAddRep::_onBcChk()
{
  OmWString url_base;
  OmWString url_name;
  OmWString url_full;

  this->getItemText(IDC_EC_INP01, url_base);
  this->getItemText(IDC_EC_INP02, url_name);

  url_full.assign(url_base);
  url_full.append(L"/");
  url_full.append(url_name);
  url_full.append(L".xml");

  if(!Om_isValidFileUrl(url_full)) {
    Om_dlgBox_okl(this->_hwnd, L"Add Mod Repository", IDI_ERR,
                  L"Invalid Mod Repository parameters", L"The specified "
                  "base address and definition name combination cannot make "
                  "valid access URL:", url_full);
  }

  this->setItemText(IDC_EC_RESUL, L"");
  this->setItemText(IDC_SC_STATE, L"Pending...");

  this->_qry_result = OM_RESULT_PENDING;

  this->_connect.requestHttpGet(url_full, OmUiAddRep::_qry_reponse_fn, this);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiAddRep::_onBcOk()
{
  if(!this->_ModChan)
    return false;

  if(this->_qry_result == OM_RESULT_UNKNOW) {


    if(!Om_dlgBox_yn(this->_hwnd, L"Add Mod Repository", IDI_ERR,
                  L"Mod Repository not tested", L"You did not query the "
                  "Repository, it may be not valid or unavailable. "
                  "Do you want to add it anyway ?"))
       return false;


  } else if(this->_qry_result != OM_RESULT_OK) {

    if(!Om_dlgBox_yn(this->_hwnd, L"Add Mod Repository", IDI_ERR,
                  L"Mod Repository not valid", L"The last Repository query "
                  "failed, it appear to be not valid or unavailable. "
                  "Do you want to add it anyway ?"))
       return false;
  }

  OmWString url_base;
  OmWString url_name;
  OmWString url_full;

  this->getItemText(IDC_EC_INP01, url_base);
  this->getItemText(IDC_EC_INP02, url_name);

  url_full.assign(url_base);
  url_full.append(L"/");
  url_full.append(url_name);
  url_full.append(L".xml");

  if(!Om_isValidFileUrl(url_full)) {

    Om_dlgBox_okl(this->_hwnd, L"Add Mod Repository", IDI_ERR,
                  L"Invalid Mod Repository parameters", L"The specified "
                  "base address and definition name combination cannot make "
                  "make valid access URL:", url_full);

    return false;
  }

  // add new repository in Mod Hub
  if(!this->_ModChan->addRepository(url_base, url_name)) {

    Om_dlgBox_okl(this->_hwnd, L"Add Mod Repository", IDI_ERR,
                 L"Add Mod Repository error", L"Repository cannot be added "
                 "to Mod Channel:", this->_ModChan->lastError());
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
  this->_createTooltip(IDC_EC_INP01,  L"Mod Repository base HTTP address");
  this->_createTooltip(IDC_EC_INP02,  L"Mod Repository identifier or definition name");
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
  this->_setItemPos(IDC_EC_INP01, 31, 25, this->cliUnitX()-155, 13);

  // Repository Name Label & EditText
  this->_setItemPos(IDC_EC_INP02, this->cliUnitX()-89, 25, 79, 13); //< resize EditText first to prevent graphical glitchs
  this->_setItemPos(IDC_SC_LBL03, this->cliUnitX()-115, 27, 23, 9);
  // force Label to repaint by invalidate rect
  InvalidateRect(this->getItem(IDC_SC_LBL03), nullptr, true);

  // Repository Test Label, Button and Result
  this->_setItemPos(IDC_SC_LBL04, 10, 55, 120, 9);
  this->_setItemPos(IDC_BC_QRY, 10, 70, 50, 14);
  this->_setItemPos(IDC_SC_STATE, 65, 73, this->cliUnitX()-20, 12);

  // Repository Test content
  this->_setItemPos(IDC_EC_RESUL, 10, 90, this->cliUnitX()-20, this->cliUnitY()-130);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->cliUnitY()-25, this->cliUnitX()-10, 1);
  // Ok and Cancel buttons
  this->_setItemPos(IDC_BC_OK, this->cliUnitX()-110, this->cliUnitY()-19, 50, 14);
  this->_setItemPos(IDC_BC_CANCEL, this->cliUnitX()-54, this->cliUnitY()-19, 50, 14);

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiAddRep::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    OmWString item_str;

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

