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
    return static_cast<OmUiWizCtxCfg*>(this->childById(IDD_WIZ_CTX_CFG))->hasValidParams();
    break;

  case 2: // Location parameters Wizard page
    return static_cast<OmUiWizCtxLoc*>(this->childById(IDD_WIZ_LOC_CFG))->hasValidParams();
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
  OmManager* manager = static_cast<OmManager*>(this->_data);
  OmUiWizCtxCfg* uiWizCtxCfg = static_cast<OmUiWizCtxCfg*>(this->childById(IDD_WIZ_CTX_CFG));
  OmUiWizCtxLoc* uiWizLocCfg = static_cast<OmUiWizCtxLoc*>(this->childById(IDD_WIZ_LOC_CFG));

  // Retrieve Context parameters
  wstring ctx_name, ctx_home;
  uiWizCtxCfg->getItemText(IDC_EC_INPT1, ctx_name);
  uiWizCtxCfg->getItemText(IDC_EC_INPT2, ctx_home);

  // Retrieve Location parameters
  wstring loc_name, loc_dst, loc_lib, loc_bck;
  uiWizLocCfg->getItemText(IDC_EC_INPT1, loc_name);
  uiWizLocCfg->getItemText(IDC_EC_INPT2, loc_dst);

  if(uiWizLocCfg->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
    uiWizLocCfg->getItemText(IDC_EC_INPT3, loc_lib);
  }

  if(uiWizLocCfg->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
    uiWizLocCfg->getItemText(IDC_EC_INPT4, loc_bck);
  }

  this->quit();

  // create the new Context, if an error occur, error message
  if(manager->makeContext(ctx_name, ctx_home)) {
    // get last created Context
    OmContext* context = manager->context(manager->contextCount()-1);
    // create new Location in Context
    if(!context->addLocation(loc_name, loc_dst, loc_lib, loc_bck)) {
      Om_dialogBoxErr(this->_hwnd, L"Location creation failed", context->lastError());
    }
  } else {
    Om_dialogBoxErr(this->_hwnd, L"Context creation failed", manager->lastError());
  }

  // refresh all tree from the main dialog
  this->root()->refresh();
}
