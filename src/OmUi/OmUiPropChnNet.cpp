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
      self->setItemText(IDC_SC_STATE, NetRepo->lastError());
      break;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_query_start(size_t i)
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan) return;

  this->enableItem(IDC_SC_STATE, true);
  this->setItemText(IDC_SC_STATE, QRY_PENDING_STRING);

  OmPNetRepoArray selection;
  selection.push_back(ModChan->getRepository(i));

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

  OmNetRepo* ModRepo = ModChan->getRepository(lb_sel);

  this->enableItem(IDC_BC_RPQRY, has_select && (ModRepo->queryResult() != OM_RESULT_PENDING));

  if(ModRepo->queryResult() == OM_RESULT_PENDING) {

    this->setItemText(IDC_SC_STATE, QRY_PENDING_STRING);

  } else {

    if(ModRepo->queryResult() == OM_RESULT_OK) {

      this->setItemText(IDC_SC_STATE, QRY_VALID_STRING);

    } else if(ModRepo->queryResult() > 0) {

      this->setItemText(IDC_SC_STATE, ModRepo->lastError());

    } else {

      this->setItemText(IDC_SC_STATE, QRY_UNKNOWN_STRING);
    }
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
  if(!Om_dlgBox_ynl(this->_hwnd, L"Channel properties", IDI_QRY, L"Remove Mod Repository",
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
  // Set buttons inner icons
  this->setBmIcon(IDC_BC_RPADD, Om_getResIcon(IDI_BT_ADD));
  this->setBmIcon(IDC_BC_RPDEL, Om_getResIcon(IDI_BT_REM));
  this->setBmIcon(IDC_BC_RPQRY, Om_getResIcon(IDI_BT_REF));

  // define controls tool-tips
  this->_createTooltip(IDC_LB_CHN,    L"Repositories list");

  this->_createTooltip(IDC_BC_DEL,    L"Remove Repository");
  this->_createTooltip(IDC_BC_EDI,    L"Query Repository");
  this->_createTooltip(IDC_BC_ADD,    L"Add Repository");

  this->_createTooltip(IDC_BC_CKBX1,  L"Warn if Mods download requires additional dependencies to be downloaded");
  this->_createTooltip(IDC_BC_CKBX2,  L"Warn if Mods to download have missing dependencies");
  this->_createTooltip(IDC_BC_CKBX3,  L"Warn if upgrading Mods will delete older versions required by other");

  this->_createTooltip(IDC_BC_RAD01,  L"On Mod upgrade, the older Mod is moved to recycle bin");
  this->_createTooltip(IDC_BC_RAD02,  L"On Mod upgrade, the older Mod is renamed with .old extension");

  this->enableItem(IDC_SC_STATE, false);

  // Update values
  this->_onTbRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onTbResize()
{
  // Repositories list Label & ListBox
  this->_setItemPos(IDC_SC_LBL01, 50, 20, 68, 16, true);
  this->_setItemPos(IDC_LB_REP, 75, 40, this->cliWidth()-125, 64, true);

  // Actions buttons
  this->_setItemPos(IDC_BC_RPADD, 50, 39, 22, 22, true);
  this->_setItemPos(IDC_BC_RPDEL, 50, 61, 22, 22, true);
  this->_setItemPos(IDC_BC_RPQRY, 50, 83, 22, 22, true);

  // Query Status Label & result static
  this->_setItemPos(IDC_SC_LBL04, 75, 110, 120, 16, true);
  this->_setItemPos(IDC_SC_STATE, 145, 110, this->cliWidth()-185, 16, true);

  // Warnings label
  this->_setItemPos(IDC_SC_LBL02, 50, 150, 300, 16, true);
  // Warnings CheckBoxes
  this->_setItemPos(IDC_BC_CKBX1, 75, 170, 300, 16, true);
  this->_setItemPos(IDC_BC_CKBX2, 75, 190, 300, 16, true);
  this->_setItemPos(IDC_BC_CKBX3, 75, 210, 300, 16, true);

  // Package upgrade label
  this->_setItemPos(IDC_SC_LBL03, 50, 250, 300, 16, true);
  // Move to trash RadioButton
  this->_setItemPos(IDC_BC_RAD01, 75, 270, 300, 16, true);
  // Rename RadioButton
  this->_setItemPos(IDC_BC_RAD02, 75, 290, 300, 16, true);
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

  // set warning messages
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, ModChan->warnExtraDnld());
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, ModChan->warnMissDnld());
  this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, ModChan->warnUpgdBrkDeps());

  // set Upgrade Rename
  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, !ModChan->upgdRename());
  this->msgItem(IDC_BC_RAD02, BM_SETCHECK, ModChan->upgdRename());

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

    case IDC_BC_CKBX1: //< CheckBox: warn extra downloads
    case IDC_BC_CKBX2: //< CheckBox: warn missing dependency
    case IDC_BC_CKBX3: //< CheckBox: warn upgrade breaks depends
      if(HIWORD(wParam) == BN_CLICKED)
        // notify parameters changes
        this->paramCheck(CHN_PROP_NET_WARNINGS);
      break;

    case IDC_BC_RAD01: //< Radio: on upgrade move to recycle bin
    case IDC_BC_RAD02: //< Radio: on upgrade rename to .old
      if(HIWORD(wParam) == BN_CLICKED)
        // notify parameters changes
        this->paramCheck(CHN_PROP_NET_ONUPGRADE);
      break;
    }
  }

  return false;
}
