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

#include "OmUiAddRep.h"
#include "OmUiPropChn.h"

#include "OmUtilDlg.h"
#include "OmUtilStr.h"
#include "OmUtilSys.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropChnNet.h"


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
void OmUiPropChnNet::_query_result_fn(void* ptr, OmResult result, uint64_t code)
{
  OmUiPropChnNet* self = static_cast<OmUiPropChnNet*>(ptr);

  OmModChan* ModChan = static_cast<OmUiPropChn*>(self->_parent)->ModChan();
  if(!ModChan)
    return;

  OmNetRepo* NetRepo = ModChan->getRepository(code);

  switch(NetRepo->queryResult()) {
    case OM_RESULT_OK: self->setItemText(IDC_SC_STATE, L"Valid"); break;
    case OM_RESULT_PENDING: self->setItemText(IDC_SC_STATE, L"Pending..."); break;
    case OM_RESULT_UNKNOW: self->setItemText(IDC_SC_STATE, L"Unknow"); break;
    default: self->setItemText(IDC_SC_STATE, NetRepo->lastError()); break;
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_query_start(size_t i)
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan)
    return;

  this->enableItem(IDC_SC_STATE, true);
  this->setItemText(IDC_SC_STATE, L"Pending...");

  OmPNetRepoArray selection;
  selection.push_back(ModChan->getRepository(i));

  ModChan->queueQueries(selection, nullptr, OmUiPropChnNet::_query_result_fn, this);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onLbReplsSel()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan)
    return;

  int lb_sel = this->msgItem(IDC_LB_REP, LB_GETCURSEL);
  if(lb_sel >= 0) {

    OmNetRepo* ModRepo = ModChan->getRepository(lb_sel);

    this->enableItem(IDC_BC_DEL, true);
    this->enableItem(IDC_BC_QRY, true);
    this->enableItem(IDC_SC_STATE, true);

    if(ModRepo->queryResult() == OM_RESULT_PENDING) {
      this->setItemText(IDC_SC_STATE, L"Pending...");
    } else {
      if(ModRepo->queryResult() == OM_RESULT_OK) {
        this->setItemText(IDC_SC_STATE, L"Valid");
      } else if(ModRepo->queryResult() > 0) {
        this->setItemText(IDC_SC_STATE, ModRepo->lastError());
      } else {
        this->setItemText(IDC_SC_STATE, L"Unknown");
      }
    }

  } else {

    this->enableItem(IDC_SC_STATE, false);
    this->setItemText(IDC_SC_STATE, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onBcAddRepo()
{
  OmModChan* pModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!pModChan) return;

  // Open new Repository dialog
  OmUiAddRep* pUiNewRep = static_cast<OmUiAddRep*>(this->siblingById(IDD_ADD_REP));
  pUiNewRep->setModChan(pModChan);
  pUiNewRep->open(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onBcDelRepo()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan)
    return;

  int lb_sel = this->msgItem(IDC_LB_REP, LB_GETCURSEL);
  if(lb_sel >= 0) {

    OmNetRepo* ModRepo = ModChan->getRepository(lb_sel);

    OmWString repo_name;
    repo_name.assign(ModRepo->urlBase());
    repo_name.append(L" -- ");
    repo_name.append(ModRepo->urlName());

    // warns the user before committing the irreparable
    if(!Om_dlgBox_ynl(this->_hwnd, L"Channel properties", IDI_QRY,
                L"Remove Mod Repository", L"Remove the following Mod "
                "Repository from Mod Channel ?", repo_name))
    {
      return;
    }

    // refresh list and buttons
    this->_onTabRefresh();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onBcQryRepo()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan) return;

  int lb_sel = this->msgItem(IDC_LB_REP, LB_GETCURSEL);

  if(lb_sel >= 0)
    this->_query_start(lb_sel);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onCkBoxWrn()
{
  this->paramCheck(CHN_PROP_NET_WARNINGS);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onBcRadUpg()
{
  this->paramCheck(CHN_PROP_NET_ONUPGRADE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onTabInit()
{
  // Set buttons inner icons
  this->setBmIcon(IDC_BC_ADD, Om_getResIcon(this->_hins, IDI_BT_ADD));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDI_BT_REM));

  // define controls tool-tips
  this->_createTooltip(IDC_LB_CHN,    L"Mod Repositories list");

  this->_createTooltip(IDC_BC_DEL,    L"Remove selected Mod Repository");
  this->_createTooltip(IDC_BC_EDI,    L"Query selected Mod Repository");
  this->_createTooltip(IDC_BC_ADD,    L"Add a new Repository");

  this->_createTooltip(IDC_BC_CKBX1,  L"Warn if Mods download requires additional dependencies to download");
  this->_createTooltip(IDC_BC_CKBX2,  L"Warn if Mods to download have missing dependencies");
  this->_createTooltip(IDC_BC_CKBX3,  L"Warn if upgrading Mods will delete older versions required by other");

  this->_createTooltip(IDC_BC_RAD01,  L"On Mod upgrade, the older Mod is moved to recycle bin");
  this->_createTooltip(IDC_BC_RAD02,  L"On Mod upgrade, the older Mod is renamed with .old extension");

  this->enableItem(IDC_SC_STATE, false);

  // Update values
  this->_onTabRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onTabResize()
{
  // Mod Channel list Label & ListBox
  this->_setItemPos(IDC_SC_LBL01, 50, 15, 68, 9);
  this->_setItemPos(IDC_LB_REP, 50, 25, this->cliUnitX()-107, 20);

  // Remove Button
  this->_setItemPos(IDC_BC_DEL, 50, 48, 50, 14);
  this->_setItemPos(IDC_BC_QRY, 105, 48, 50, 14);
  // Add button
  this->_setItemPos(IDC_BC_ADD, this->cliUnitX()-107, 48, 50, 14);
  // Query Status Label & result static
  this->_setItemPos(IDC_SC_LBL04, 51, 64, 60, 9);
  this->_setItemPos(IDC_SC_STATE, 97, 64, this->cliUnitX()-180, 9);

  // Warnings label
  this->_setItemPos(IDC_SC_LBL02, 50, 85, 180, 9);
  // Warnings CheckBoxes
  this->_setItemPos(IDC_BC_CKBX1, 65, 95, 180, 9);
  this->_setItemPos(IDC_BC_CKBX2, 65, 105, 180, 9);
  this->_setItemPos(IDC_BC_CKBX3, 65, 115, 180, 9);

  // Package upgrade label
  this->_setItemPos(IDC_SC_LBL03, 50, 135, 180, 9);
  // Move to trash RadioButton
  this->_setItemPos(IDC_BC_RAD01, 65, 145, 180, 9);
  // Rename RadioButton
  this->_setItemPos(IDC_BC_RAD02, 65, 155, 180, 9);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnNet::_onTabRefresh()
{
  // Set controls default states and parameters
  this->enableItem(IDC_SC_STATE, false);
  //this->setItemText(IDC_SC_STATE, L"Unknown");

  this->enableItem(IDC_BC_DEL,  false);
  this->enableItem(IDC_BC_QRY,  false);

  this->msgItem(IDC_LB_REP, LB_RESETCONTENT);

  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan) return;

  OmWString lb_entry;

  for(unsigned i = 0; i < ModChan->repositoryCount(); ++i) {

    OmNetRepo* ModRepo = ModChan->getRepository(i);

    lb_entry = ModRepo->urlBase();
    lb_entry += L" -- ";
    lb_entry += ModRepo->urlName();

    this->msgItem(IDC_LB_REP, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(lb_entry.c_str()));
  }

  // set warning messages
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, ModChan->warnExtraDnld());
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, ModChan->warnMissDnld());
  this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, ModChan->warnUpgdBrkDeps());

  // set Upgrade Rename
  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, !ModChan->upgdRename());
  this->msgItem(IDC_BC_RAD02, BM_SETCHECK, ModChan->upgdRename());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChnNet::_onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_REP: //< Mod Channel(s) list ListBox
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_onLbReplsSel();
      break;

    case IDC_BC_ADD: //< New button for Mod Channel(s) list
      this->_onBcAddRepo();
      break;

    case IDC_BC_DEL: //< Remove button for Mod Channel(s) list
      this->_onBcDelRepo();
      break;

    case IDC_BC_QRY:
      this->_onBcQryRepo();
      break;

    case IDC_BC_CKBX1:
    case IDC_BC_CKBX2:
    case IDC_BC_CKBX3:
      this->_onCkBoxWrn();
      break;

    case IDC_BC_RAD01:
    case IDC_BC_RAD02:
      this->_onBcRadUpg();
      break;
    }
  }

  return false;
}
