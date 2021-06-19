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
#include "OmManager.h"
#include "gui/OmUiPropMan.h"
#include "gui/OmUiPropManGle.h"
#include "gui/OmUiPropManPkg.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropMan::OmUiPropMan(HINSTANCE hins) : OmDialogProp(hins)
{
  // create tab dialogs
  this->_addPage(L"General", new OmUiPropManGle(hins));
  this->_addPage(L"Packages", new OmUiPropManPkg(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropMan::~OmUiPropMan()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropMan::id() const
{
  return IDD_PROP_MAN;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropMan::checkChanges()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmUiPropManGle* pUiPropManGle  = static_cast<OmUiPropManGle*>(this->childById(IDD_PROP_MAN_GLE));
  OmUiPropManPkg* pUiPropManPkg  = static_cast<OmUiPropManPkg*>(this->childById(IDD_PROP_MAN_PKG));

  bool changed = false;

  if(pUiPropManGle->hasChParam(MAN_PROP_GLE_ICON_SIZE)) {

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

  if(pUiPropManGle->hasChParam(MAN_PROP_GLE_STARTUP_CONTEXTS)) {
    changed = true;
  }

  if(pUiPropManPkg->hasChParam(MAN_PROP_PKG_PACKAGE_FLAGS)) {
    changed = true;
  }

  // enable Apply button
  this->enableItem(IDC_BC_APPLY, changed);

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropMan::applyChanges()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmUiPropManGle* pUiPropManGle  = static_cast<OmUiPropManGle*>(this->childById(IDD_PROP_MAN_GLE));
  OmUiPropManPkg* pUiPropManPkg  = static_cast<OmUiPropManPkg*>(this->childById(IDD_PROP_MAN_PKG));

  // Parameter: Icons size for packages List-View
  if(pUiPropManGle->hasChParam(MAN_PROP_GLE_ICON_SIZE)) {

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
    pUiPropManGle->setChParam(MAN_PROP_GLE_ICON_SIZE, false);
  }

  // Parameter: Open Context(s) at startup
  if(pUiPropManGle->hasChParam(MAN_PROP_GLE_STARTUP_CONTEXTS)) {

    bool bm_chk = pUiPropManGle->msgItem(IDC_BC_CHK01, BM_GETCHECK);

    vector<wstring> path_ls;

    int lb_cnt =  this->msgItem(IDC_LB_PATH, LB_GETCOUNT);
    if(lb_cnt > 0) {
      wchar_t item_buf[OMM_ITM_BUFF];
      for(int i = 0; i < lb_cnt; ++i) {
        this->msgItem(IDC_LB_PATH, LB_GETTEXT, i, reinterpret_cast<LPARAM>(item_buf));
        path_ls.push_back(item_buf);
      }
    }

    pMgr->saveStartContexts(bm_chk, path_ls);

    // Reset parameter as unmodified
    pUiPropManGle->setChParam(MAN_PROP_GLE_STARTUP_CONTEXTS, false);
  }

  // Parameter: Various Packages options
  if(pUiPropManPkg->hasChParam(MAN_PROP_PKG_PACKAGE_FLAGS)) {

    pMgr->setLegacySupport(pUiPropManPkg->msgItem(IDC_BC_CHK01, BM_GETCHECK));
    pMgr->setWarnOverlaps(pUiPropManPkg->msgItem(IDC_BC_CHK02, BM_GETCHECK));
    pMgr->setWarnExtraInst(pUiPropManPkg->msgItem(IDC_BC_CHK03, BM_GETCHECK));
    pMgr->setWarnMissDpnd(pUiPropManPkg->msgItem(IDC_BC_CHK04, BM_GETCHECK));
    pMgr->setWarnExtraUnin(pUiPropManPkg->msgItem(IDC_BC_CHK05, BM_GETCHECK));
    pMgr->setQuietBatches(pUiPropManPkg->msgItem(IDC_BC_CHK06, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropManPkg->setChParam(MAN_PROP_PKG_PACKAGE_FLAGS, false);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  // refresh all dialogs from root (Main dialog)
  this->root()->refresh();

  return true;
}
