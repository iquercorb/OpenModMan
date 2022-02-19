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

#include "OmUiPropMgrGle.h"

#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropMgr.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropMgr::OmUiPropMgr(HINSTANCE hins) : OmDialogProp(hins)
{
  // create tab dialogs
  this->_addPage(L"General", new OmUiPropMgrGle(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropMgr::~OmUiPropMgr()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropMgr::id() const
{
  return IDD_PROP_MGR;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropMgr::checkChanges()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmUiPropMgrGle* pUiPropManGle  = static_cast<OmUiPropMgrGle*>(this->childById(IDD_PROP_MGR_GLE));

  bool changed = false;

  if(pUiPropManGle->hasChParam(MGR_PROP_GLE_ICON_SIZE)) {

    int cb_sel = pUiPropManGle->msgItem(IDC_CB_ICS, CB_GETCURSEL, 0, 0);

    switch(cb_sel)
    {
    case 0:
      if(pMgr->iconsSize() != 16)
        changed = true;
      break;
    case 2:
      if(pMgr->iconsSize() != 32)
        changed = true;
      break;
    default:
      if(pMgr->iconsSize() != 24)
        changed = true;
      break;
    }
  }

  if(pUiPropManGle->hasChParam(MGR_PROP_GLE_NO_MDPARSE)) {
    if(pUiPropManGle->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != pMgr->noMarkdown())
      changed = true;
  }

  if(pUiPropManGle->hasChParam(MGR_PROP_GLE_START_LIST)) {
    changed = true;
  }

  // enable Apply button
  this->enableItem(IDC_BC_APPLY, changed);

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropMgr::applyChanges()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmUiPropMgrGle* pUiPropManGle  = static_cast<OmUiPropMgrGle*>(this->childById(IDD_PROP_MGR_GLE));

  // Parameter: Icons size for packages List-View
  if(pUiPropManGle->hasChParam(MGR_PROP_GLE_ICON_SIZE)) {

    int cb_sel = pUiPropManGle->msgItem(IDC_CB_ICS, CB_GETCURSEL);

    switch(cb_sel)
    {
    case 0:
      pMgr->setIconsSize(16);
      break;
    case 2:
      pMgr->setIconsSize(32);
      break;
    default:
      pMgr->setIconsSize(24);
      break;
    }

    // Reset parameter as unmodified
    pUiPropManGle->setChParam(MGR_PROP_GLE_ICON_SIZE, false);
  }

  // Parameter No markdown parsing
  if(pUiPropManGle->hasChParam(MGR_PROP_GLE_NO_MDPARSE)) {

    pMgr->setNoMarkdown(pUiPropManGle->msgItem(IDC_BC_CKBX1, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropManGle->setChParam(MGR_PROP_GLE_NO_MDPARSE, false);
  }

  // Parameter: Open Context(s) at startup
  if(pUiPropManGle->hasChParam(MGR_PROP_GLE_START_LIST)) {

    bool bm_chk = pUiPropManGle->msgItem(IDC_BC_CKBX2, BM_GETCHECK);

    vector<wstring> path_ls;

    int lb_cnt =  pUiPropManGle->msgItem(IDC_LB_PATH, LB_GETCOUNT);

    if(lb_cnt > 0) {
      wchar_t item_buf[OMM_ITM_BUFF];
      for(int i = 0; i < lb_cnt; ++i) {
        pUiPropManGle->msgItem(IDC_LB_PATH, LB_GETTEXT, i, reinterpret_cast<LPARAM>(item_buf));
        path_ls.push_back(item_buf);
      }
    }

    pMgr->saveStartContexts(bm_chk, path_ls);

    // Reset parameter as unmodified
    pUiPropManGle->setChParam(MGR_PROP_GLE_START_LIST, false);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  // refresh all dialogs from root (Main dialog)
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropMgr::_onPropInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));
}
