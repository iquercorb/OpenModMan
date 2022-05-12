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

#include "OmManager.h"
#include "OmContext.h"

#include "OmUiWizCtxBeg.h"
#include "OmUiWizCtxCfg.h"
#include "OmUiWizCtxLoc.h"
#include "OmUiMgr.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizCtx.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizCtx::OmUiWizCtx(HINSTANCE hins) : OmDialogWiz(hins)
{
  // create wizard pages
  this->_addPage(new OmUiWizCtxBeg(hins));
  this->_addPage(new OmUiWizCtxCfg(hins));
  this->_addPage(new OmUiWizCtxLoc(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizCtx::~OmUiWizCtx()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizCtx::id() const
{
  return IDD_WIZ_CTX;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizCtx::_onWizNext()
{
  switch(this->_currPage)
  {
  case 1: // Context parameters Wizard page
    return static_cast<OmUiWizCtxCfg*>(this->childById(IDD_WIZ_CTX_CFG))->hasValidParams();
    break;
  case 2: // Location parameters Wizard page
    return static_cast<OmUiWizCtxLoc*>(this->childById(IDD_WIZ_CTX_LOC))->hasValidParams();
    break;
  default:
    return true;
    break;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtx::_onWizFinish()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmUiWizCtxCfg* pUiWizCtxCfg = static_cast<OmUiWizCtxCfg*>(this->childById(IDD_WIZ_CTX_CFG));
  OmUiWizCtxLoc* pUiWizLocCfg = static_cast<OmUiWizCtxLoc*>(this->childById(IDD_WIZ_CTX_LOC));

  // Retrieve Context parameters
  wstring ctx_name, ctx_home;
  pUiWizCtxCfg->getItemText(IDC_EC_INP01, ctx_name);
  pUiWizCtxCfg->getItemText(IDC_EC_INP02, ctx_home);

  // Retrieve Location parameters
  wstring loc_name, loc_dst, loc_lib, loc_bck;
  pUiWizLocCfg->getItemText(IDC_EC_INP01, loc_name);
  pUiWizLocCfg->getItemText(IDC_EC_INP02, loc_dst);

  if(pUiWizLocCfg->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
    pUiWizLocCfg->getItemText(IDC_EC_INP03, loc_lib);
  }

  if(pUiWizLocCfg->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
    pUiWizLocCfg->getItemText(IDC_EC_INP04, loc_bck);
  }

  this->quit();

  // create the new Context, if an error occur, error message
  if(pMgr->ctxNew(ctx_name, ctx_home)) {
    // get last created Context
    OmContext* pCtx = pMgr->ctxGet(pMgr->ctxCount()-1);
    // create new Location in Context
    if(!pCtx->locAdd(loc_name, loc_dst, loc_lib, loc_bck)) {
      Om_dlgBox_okl(this->_hwnd, L"Software Context Wizard", IDI_ERR,
                    L"Target Location creation error", L"Target Location "
                    "creation failed because of the following error:",
                    pCtx->lastError());
    }
  } else {
    Om_dlgBox_okl(this->_hwnd, L"Software Context Wizard", IDI_ERR,
                  L"Software Context creation error", L"Software Context "
                  "creation failed because of the following error:",
                  pMgr->lastError());
  }

  // Unselect current context, this will force to select last one at next refresh
  static_cast<OmUiMgr*>(this->root())->ctxSel(-1);

  // force parent dialog to refresh
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtx::_onWizInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));
}
