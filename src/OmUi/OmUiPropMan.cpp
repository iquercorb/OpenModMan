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

#include "OmModMan.h"

#include "OmUiPropManGle.h"

#include "OmUtilWin.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropMan.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropMan::OmUiPropMan(HINSTANCE hins) : OmDialogProp(hins)
{
  // create tab dialogs
  this->_addPage(L"General Settings", new OmUiPropManGle(hins));
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
  return IDD_PROP_MGR;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropMan::checkChanges()
{
  OmModMan* pMgr = static_cast<OmModMan*>(this->_data);

  OmUiPropManGle* pUiPropManGle  = static_cast<OmUiPropManGle*>(this->childById(IDD_PROP_MGR_GLE));

  bool different, changed = false;

  OmWString itm_str;

  if(pUiPropManGle->paramChanged(MAN_PROP_GLE_ICON_SIZE)) {

    bool diff = false;

    switch(pUiPropManGle->msgItem(IDC_CB_ICS, CB_GETCURSEL))
    {
    case 0:   diff = (pMgr->iconsSize() != 16); break;
    case 2:   diff = (pMgr->iconsSize() != 32); break;
    default:  diff = (pMgr->iconsSize() != 24); break;
    }

    if(diff) {
      changed = true;
    } else {
      pUiPropManGle->paramReset(MAN_PROP_GLE_ICON_SIZE);
    }
  }

  if(pUiPropManGle->paramChanged(MAN_PROP_GLE_NO_MDPARSE)) {
    if(pUiPropManGle->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != pMgr->noMarkdown()) {
      changed = true;
    } else {
      pUiPropManGle->paramReset(MAN_PROP_GLE_NO_MDPARSE);
    }
  }

  if(pUiPropManGle->paramChanged(MAN_PROP_GLE_START_LIST)) {

    different = false;

    bool autoload;
    OmWStringArray path_ls;

    pMgr->loadStartHubs(&autoload, path_ls);

    if(pUiPropManGle->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != autoload) {
      different = true;
    }

    if(!different) {
      size_t lb_cnt = pUiPropManGle->msgItem(IDC_LB_PATH, LB_GETCOUNT);
      if(lb_cnt == path_ls.size()) {
          for(size_t i = 0; i < lb_cnt; ++i) {
            pUiPropManGle->getLbText(IDC_LB_PATH, i, itm_str);
            if(!Om_namesMatches(path_ls[i], itm_str)) {
              different = true;
              break;
            }
          }
      } else {
        different = true;
      }
    }

    if(different) {
      changed = true;
    } else {
      pUiPropManGle->paramReset(MAN_PROP_GLE_START_LIST);
    }
  }

  // enable Apply button
  this->enableItem(IDC_BC_APPLY, changed);

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropMan::validChanges()
{
  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropMan::applyChanges()
{
  OmModMan* pMgr = static_cast<OmModMan*>(this->_data);

  OmWString itm_str;

  OmUiPropManGle* pUiPropManGle  = static_cast<OmUiPropManGle*>(this->childById(IDD_PROP_MGR_GLE));

  // Parameter: Icons size for packages List-View
  if(pUiPropManGle->paramChanged(MAN_PROP_GLE_ICON_SIZE)) {

    switch(pUiPropManGle->msgItem(IDC_CB_ICS, CB_GETCURSEL))
    {
    case 0:   pMgr->setIconsSize(16); break;
    default:  pMgr->setIconsSize(24); break;
    }

    // Reset parameter as unmodified
    pUiPropManGle->paramReset(MAN_PROP_GLE_ICON_SIZE);
  }

  // Parameter No markdown parsing
  if(pUiPropManGle->paramChanged(MAN_PROP_GLE_NO_MDPARSE)) {

    pMgr->setNoMarkdown(pUiPropManGle->msgItem(IDC_BC_CKBX1, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropManGle->paramReset(MAN_PROP_GLE_NO_MDPARSE);
  }

  // Parameter: Open Mod Hub(s) at startup
  if(pUiPropManGle->paramChanged(MAN_PROP_GLE_START_LIST)) {

    OmWStringArray path_ls;

    int lb_cnt =  pUiPropManGle->msgItem(IDC_LB_PATH, LB_GETCOUNT);

    if(lb_cnt > 0) {

      for(int i = 0; i < lb_cnt; ++i) {
        pUiPropManGle->getLbText(IDC_LB_PATH, i, itm_str);
        path_ls.push_back(itm_str);
      }
    }

    pMgr->saveStartHubs(pUiPropManGle->msgItem(IDC_BC_CKBX2, BM_GETCHECK), path_ls);

    // Reset parameter as unmodified
    pUiPropManGle->paramReset(MAN_PROP_GLE_START_LIST);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  return true;
}
