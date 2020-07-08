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
#include "gui/OmUiWizCtx.h"
#include "gui/OmUiWizCtxBeg.h"
#include "gui/OmUiWizCtxCfg.h"
#include "gui/OmUiWizCtxLoc.h"
#include "OmManager.h"
#include "OmContext.h"


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
    return reinterpret_cast<OmUiWizCtxCfg*>(this->childById(IDD_WIZ_CTX_CFG))->hasValidParams();
    break;

  case 2: // Location parameters Wizard page
    return reinterpret_cast<OmUiWizCtxLoc*>(this->childById(IDD_WIZ_LOC_CFG))->hasValidParams();
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
  OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
  OmUiWizCtxCfg* uiWizCtxCfg = reinterpret_cast<OmUiWizCtxCfg*>(this->childById(IDD_WIZ_CTX_CFG));
  OmUiWizCtxLoc* uiWizLocCfg = reinterpret_cast<OmUiWizCtxLoc*>(this->childById(IDD_WIZ_LOC_CFG));

  wchar_t ctx_titl[MAX_PATH];
  wchar_t ctx_locp[MAX_PATH];
  wchar_t loc_titl[MAX_PATH];
  wchar_t loc_inst[MAX_PATH];
  wchar_t loc_libd[MAX_PATH];
  wchar_t loc_bckd[MAX_PATH];

  GetDlgItemTextW(uiWizCtxCfg->hwnd(), IDC_EC_INPT1, ctx_titl, MAX_PATH);
  GetDlgItemTextW(uiWizCtxCfg->hwnd(), IDC_EC_INPT2, ctx_locp, MAX_PATH);

  GetDlgItemTextW(uiWizLocCfg->hwnd(), IDC_EC_INPT1, loc_titl, MAX_PATH);
  GetDlgItemTextW(uiWizLocCfg->hwnd(), IDC_EC_INPT2, loc_inst, MAX_PATH);

  if(SendMessage(GetDlgItem(uiWizLocCfg->hwnd(),IDC_BC_CHK01),BM_GETCHECK,0,0)) {
    GetDlgItemTextW(uiWizLocCfg->hwnd(), IDC_EC_INPT4, loc_libd, MAX_PATH);
  } else {
    loc_libd[0] = L'\0';
  }

  if(SendMessage(GetDlgItem(uiWizLocCfg->hwnd(),IDC_BC_CHK02),BM_GETCHECK,0,0)) {
    GetDlgItemTextW(uiWizLocCfg->hwnd(), IDC_EC_INPT5, loc_bckd, MAX_PATH);
  } else {
    loc_bckd[0] = L'\0';
  }

  this->quit();

  // create the new Context, if an error occur, error message
  if(manager->makeContext(ctx_titl, ctx_locp)) {
    // get last created Context
    OmContext* context = manager->context(manager->contextCount()-1);
    // create new Location in Context
    if(!context->makeLocation(loc_titl, loc_inst, loc_libd, loc_bckd)) {
      Om_dialogBoxErr(this->_hwnd, L"Location creation failed", context->lastError());
    }
  } else {
    Om_dialogBoxErr(this->_hwnd, L"Context creation failed", manager->lastError());
  }

  // refresh all tree from the main dialog
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtx::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtx::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizCtx::_onWizMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return false;
}
