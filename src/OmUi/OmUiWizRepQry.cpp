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
#include "OmBaseApp.h"
#include "OmBaseUi.h"

#include "OmUtilDlg.h"
#include "OmUtilStr.h"
#include "OmUtilSys.h"
#include "OmUtilWin.h"

#include "OmNetRepo.h"

#include "OmUiWizRepCfg.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizRepQry.h"

#define QRY_INVALID_STRING    L"Invalid parameters"
#define QRY_UNKNOWN_STRING    L"Not tested"
#define QRY_PENDING_STRING    L"Sending request..."
#define QRY_VALID_STRING      L"Repository appear valid"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizRepQry::OmUiWizRepQry(HINSTANCE hins) : OmDialogWizPage(hins),
  _NetRepo(new OmNetRepo(nullptr)),
  _query_result(OM_RESULT_UNKNOW),
  _query_hth(nullptr),
  _query_hwo(nullptr)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizRepQry::~OmUiWizRepQry()
{
  if(this->_NetRepo)
    delete this->_NetRepo;

  HFONT hFt;
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_SC_STATE, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_RESUL, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizRepQry::id() const
{
  return IDD_WIZ_REP_QRY;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizRepQry::validFields() const
{
  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizRepQry::validParams() const
{
  if(this->_query_result == OM_RESULT_UNKNOW) {
    if(!Om_dlgBox_yn(this->_hwnd, L"Repository Configuration Wizard", IDI_DLG_WRN, L"Repository not tested",
                     L"You did not query the Repository to verify parameters and availability. "
                     "Do you want to add it anyway ?"))
       return false;
  }

  if(this->_query_result != OM_RESULT_OK) {
    if(!Om_dlgBox_yn(this->_hwnd, L"Repository Configuration Wizard", IDI_DLG_WRN, L"Repository unavailable",
                    L"The last Repository query failed, either parameters are invalid or Repository is unavailable. "
                    "Do you want to add it anyway ?"))
       return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizRepQry::_query_abort()
{
  this->_NetRepo->abortQuery();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizRepQry::_query_start()
{
  if(this->_query_hth)
    this->_query_abort();

  OmUiWizRepCfg* UiWizRepCfg = static_cast<OmUiWizRepCfg*>(this->siblingById(IDD_WIZ_REP_CFG));

  OmWString base, name;

  if(UiWizRepCfg->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    UiWizRepCfg->getItemText(IDC_EC_INP01, base);
    UiWizRepCfg->getItemText(IDC_EC_INP02, name);
  } else {
    UiWizRepCfg->getItemText(IDC_EC_INP03, base);
  }

  // set Repository coordinates, they should be valid
  if(!this->_NetRepo->setCoordinates(base, name)) {
    this->setItemText(IDC_SC_STATE, QRY_INVALID_STRING);
    return;
  }

  // here we go
  this->_query_hth = Om_threadCreate(OmUiWizRepQry::_query_run_fn, this);
  this->_query_hwo = Om_threadWaitEnd(this->_query_hth, OmUiWizRepQry::_query_end_fn, this);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiWizRepQry::_query_run_fn(void* ptr)
{
  OmUiWizRepQry* self = static_cast<OmUiWizRepQry*>(ptr);

  self->setItemText(IDC_EC_RESUL, L"");
  self->setItemText(IDC_SC_STATE, QRY_PENDING_STRING);
  self->setItemText(IDC_BC_RPQRY, L"Abort query");

  self->_query_result = self->_NetRepo->query();

  // if any, print received data to log
  if(!self->_NetRepo->queryResponseData().empty()) {
    OmWString crlf_str = Om_toCRLF(self->_NetRepo->queryResponseData());
    size_t len = self->msgItem(IDC_EC_RESUL, WM_GETTEXTLENGTH);
    self->msgItem(IDC_EC_RESUL, EM_SETSEL, 0, len);
    self->msgItem(IDC_EC_RESUL, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(crlf_str.c_str()));
    self->msgItem(IDC_EC_RESUL, WM_VSCROLL, SB_TOP, 0);
    self->msgItem(IDC_EC_RESUL, 0, 0, RDW_ERASE|RDW_INVALIDATE);
  }

  if(self->_query_result == OM_RESULT_OK) {
    self->setItemText(IDC_SC_STATE, QRY_VALID_STRING);
  } else {
    if(self->_query_result != OM_RESULT_ABORT) {
      self->setItemText(IDC_SC_STATE, self->_NetRepo->queryLastError());
    }
  }

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmUiWizRepQry::_query_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  OmUiWizRepQry* self = static_cast<OmUiWizRepQry*>(ptr);

  Om_threadClear(self->_query_hth, self->_query_hwo);
  self->_query_hth = nullptr;
  self->_query_hwo = nullptr;

  // reset query button
  self->setItemText(IDC_BC_RPQRY, L"Send query");
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizRepQry::_onPgInit()
{
  HFONT hFt;
  // set specific fonts
  hFt = Om_createFont(12, 800, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_STATE, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(12, 400, L"Consolas");
  this->msgItem(IDC_EC_RESUL, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  this->_createTooltip(IDC_BC_RPQRY,  L"query the Repository to check parameters and availability");
  this->_createTooltip(IDC_SC_STATE,  L"Query test result");
  this->_createTooltip(IDC_EC_RESUL,  L"Query test logs");

  // set default start values
  this->setItemText(IDC_SC_STATE, QRY_UNKNOWN_STRING);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizRepQry::_onPgShow()
{
  // reset query result
  this->setItemText(IDC_SC_STATE, QRY_UNKNOWN_STRING);

  // erase edit control content
  size_t len = this->msgItem(IDC_EC_RESUL, WM_GETTEXTLENGTH);
  this->msgItem(IDC_EC_RESUL, EM_SETSEL, 0, len);
  this->msgItem(IDC_EC_RESUL, EM_REPLACESEL, 0, 0);

  // send new query
  this->_query_start();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizRepQry::_onPgResize()
{
  int32_t y_base = 100;

  // Introduction text
  this->_setItemPos(IDC_SC_HELP, 0, 0, this->cliWidth(), 70, true);

  this->_setItemPos(IDC_BC_RPQRY, 0, y_base, 78, 23, true);

  this->_setItemPos(IDC_SC_LBL01, 80, y_base+6, 40, 18, true);
  this->_setItemPos(IDC_SC_STATE, 122, y_base+6, this->cliWidth()-122, 18, true);

  this->_setItemPos(IDC_EC_RESUL, 1, y_base+30, this->cliWidth()-2, this->cliHeight()-140, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiWizRepQry::_onPgMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);


  if(uMsg == WM_COMMAND) {
    switch(LOWORD(wParam))
    {
    case IDC_BC_RPQRY:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_query_start();
      break;
    }
  }

  return 0;
}
