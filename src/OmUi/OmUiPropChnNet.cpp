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
#include "OmNetRepo.h"

#include "OmUiMan.h"
#include "OmUiWizRep.h"
#include "OmUiPropChn.h"

#include "OmUtilDlg.h"
#include "OmUtilStr.h"
#include "OmUtilSys.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropChnNet.h"

#define QRY_UNKNOWN_STRING    L"Not tested"
#define QRY_PENDING_STRING    L"Sending request..."
#define QRY_VALID_STRING      L"Repository appear valid"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnNet::OmUiPropChnNet(HINSTANCE hins) : OmDialogPropTab(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnNet::~OmUiPropChnNet()
{
  HFONT hFt;
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_SC_STATE, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropChnNet::id() const
{
  return IDD_PROP_CHN_NET;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_query_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OM_UNUSED(result);

  OmUiPropChnNet* self = static_cast<OmUiPropChnNet*>(ptr);

  OmNetRepo* NetRepo = reinterpret_cast<OmNetRepo*>(param);

  switch(NetRepo->queryResult()) {

    case OM_RESULT_OK:
      self->setItemText(IDC_SC_STATE, QRY_VALID_STRING);
      break;

    case OM_RESULT_PENDING:
      self->setItemText(IDC_SC_STATE, QRY_PENDING_STRING);
      break;

    case OM_RESULT_UNKNOW:
      self->setItemText(IDC_SC_STATE, QRY_UNKNOWN_STRING);
      break;

    default:
      self->setItemText(IDC_SC_STATE, NetRepo->queryLastError());
      break;
  }

  // if any, print received data to log
  if(!NetRepo->queryResponseData().empty())
    self->setItemText(IDC_EC_RESUL, NetRepo->queryResponseData());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_query_start(size_t index)
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan) return;

  this->enableItem(IDC_SC_STATE, true);
  this->setItemText(IDC_SC_STATE, QRY_PENDING_STRING);

  OmPNetRepoArray selection;
  selection.push_back(ModChan->getRepository(index));

  ModChan->queueQueries(selection, nullptr, OmUiPropChnNet::_query_result_fn, nullptr, this);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_lb_rep_on_selchg()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan) return;

  int32_t lb_sel = this->msgItem(IDC_LB_REP, LB_GETCURSEL);

  bool has_select = (lb_sel >= 0);

  this->enableItem(IDC_BC_RPDEL, has_select);
  this->enableItem(IDC_SC_STATE, has_select);

  if(!has_select) {
    this->enableItem(IDC_BC_RPQRY, has_select);
    this->setItemText(IDC_SC_STATE, L"");
    return;
  }

  OmNetRepo* NetRepo = ModChan->getRepository(lb_sel);

  this->enableItem(IDC_BC_RPQRY, has_select && (NetRepo->queryResult() != OM_RESULT_PENDING));

  if(NetRepo->queryResult() == OM_RESULT_PENDING) {

    this->setItemText(IDC_SC_STATE, QRY_PENDING_STRING);

  } else {

    if(NetRepo->queryResult() == OM_RESULT_OK) {

      this->setItemText(IDC_SC_STATE, QRY_VALID_STRING);

    } else if(NetRepo->queryResult() > 0) {

      this->setItemText(IDC_SC_STATE, NetRepo->lastError());

    } else {

      this->setItemText(IDC_SC_STATE, QRY_UNKNOWN_STRING);
    }
  }

  // if any, print received data to log
  if(!NetRepo->queryResponseData().empty()) {
    this->setItemText(IDC_EC_RESUL, NetRepo->queryResponseData());
  } else {
    this->setItemText(IDC_EC_RESUL, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_bc_rpadd_pressed()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // not during query, please
  if(ModChan->queriesQueueSize())
    return;

  OmUiWizRep* UiWizRep = static_cast<OmUiWizRep*>(this->root()->childById(IDD_WIZ_REP));

  UiWizRep->setModChan(ModChan);

  UiWizRep->open(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_bc_rpdel_pressed()
{
  int32_t lb_sel = this->msgItem(IDC_LB_REP, LB_GETCURSEL);
  if(lb_sel < 0) return;

  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan) return;

  OmNetRepo* ModRepo = ModChan->getRepository(lb_sel);

  // TODO: Améliorer ça, supposons une URL ultra longue, que faire ?
  OmWString repo_name = ModRepo->base();
  repo_name += L" -- ";
  repo_name += ModRepo->name();

  // warns the user before committing the irreparable
  if(!Om_dlgBox_ynl(this->_hwnd, L"Channel properties", IDI_DLG_QRY, L"Remove Mod Repository",
                    L"Remove the following Mod Repository from Mod Channel ?", repo_name))
    return;

  // remove repository
  ModChan->removeRepository(lb_sel);

  // refresh all
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_bc_rpqry_pressed()
{
  int32_t lb_sel = this->msgItem(IDC_LB_REP, LB_GETCURSEL);
  if(lb_sel < 0) return;

  this->_query_start(lb_sel);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onTbInit()
{
  HFONT hFt;
  // set specific fonts
  hFt = Om_createFont(12, 800, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_STATE, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(12, 400, L"Consolas");
  this->msgItem(IDC_EC_RESUL, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  // Set buttons inner icons
  this->setBmIcon(IDC_BC_RPADD, Om_getResIcon(IDI_BT_ADD));
  this->setBmIcon(IDC_BC_RPDEL, Om_getResIcon(IDI_BT_REM));
  this->setBmIcon(IDC_BC_RPQRY, Om_getResIcon(IDI_BT_REF));

  // define controls tool-tips
  this->_createTooltip(IDC_LB_CHN,    L"repositories list");

  this->_createTooltip(IDC_BC_DEL,    L"remove Repository");
  this->_createTooltip(IDC_BC_EDI,    L"query Repository");
  this->_createTooltip(IDC_BC_ADD,    L"add Repository");

  this->_createTooltip(IDC_EC_RESUL,  L"Received query data");

  this->enableItem(IDC_SC_STATE, false);

  // Update values
  this->_onTbRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onTbResize()
{
  int32_t y_base = 40;

  // Repositories list Label & ListBox
  this->_setItemPos(IDC_SC_LBL01, 50, y_base, 150, 16, true);
  this->_setItemPos(IDC_LB_REP, 75, y_base+20, this->cliWidth()-125, 64, true);

  // Actions buttons
  this->_setItemPos(IDC_BC_RPADD, 50, y_base+19, 22, 22, true);
  this->_setItemPos(IDC_BC_RPDEL, 50, y_base+41, 22, 22, true);
  this->_setItemPos(IDC_BC_RPQRY, 50, y_base+63, 22, 22, true);

  // Query Status Label & static
  this->_setItemPos(IDC_SC_LBL04, 75, y_base+100, 70, 16, true);
  this->_setItemPos(IDC_SC_STATE, 148, y_base+100, this->cliWidth()-185, 16, true);

  // Query received data entry
  this->_setItemPos(IDC_EC_RESUL, 75, y_base+120, this->cliWidth()-125, this->cliHeight()-220, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onTbRefresh()
{
  // Set controls default states and parameters
  this->enableItem(IDC_SC_STATE, false);
  //this->setItemText(IDC_SC_STATE, L"Unknown");

  this->enableItem(IDC_BC_RPDEL,  false);
  this->enableItem(IDC_BC_RPQRY,  false);

  this->msgItem(IDC_LB_REP, LB_RESETCONTENT);

  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan) return;

  OmWString lb_entry;

  for(unsigned i = 0; i < ModChan->repositoryCount(); ++i) {

    OmNetRepo* ModRepo = ModChan->getRepository(i);

    // TODO: Améliorer ça, supposons une URL ultra longue, que faire ?
    lb_entry = ModRepo->base();
    lb_entry += L" -- ";
    lb_entry += ModRepo->name();

    this->msgItem(IDC_LB_REP, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(lb_entry.c_str()));
  }

  this->_lb_rep_on_selchg();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChnNet::_onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_REP: //< Mod Channel(s) list ListBox
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_lb_rep_on_selchg();
      break;

    case IDC_BC_RPADD: //< Button : Add
      if(HIWORD(wParam) == BN_CLICKED)
        this->_bc_rpadd_pressed();
      break;

    case IDC_BC_RPDEL: //< Button : Delete
      if(HIWORD(wParam) == BN_CLICKED)
        this->_bc_rpdel_pressed();
      break;

    case IDC_BC_RPQRY: //< Button : Query
      if(HIWORD(wParam) == BN_CLICKED)
        this->_bc_rpqry_pressed();
      break;
    }
  }

  return false;
}
