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
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmUiPropManGle* UiPropManGle  = static_cast<OmUiPropManGle*>(this->childById(IDD_PROP_MGR_GLE));

  bool different, changed = false;

  if(UiPropManGle->paramChanged(MAN_PROP_GLE_ICON_SIZE)) {

    different = false;

    switch(UiPropManGle->msgItem(IDC_CB_ICS, CB_GETCURSEL))
    {
    case 0:
      different = (ModMan->iconsSize() != 16);
      break;

    default:
      different = (ModMan->iconsSize() != 24);
      break;
    }

    if(different) {
      changed = true;
    } else {
      UiPropManGle->paramReset(MAN_PROP_GLE_ICON_SIZE);
    }
  }

  if(UiPropManGle->paramChanged(MAN_PROP_GLE_NO_MDPARSE)) {
    if(UiPropManGle->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != ModMan->noMarkdown()) {
      changed = true;
    } else {
      UiPropManGle->paramReset(MAN_PROP_GLE_NO_MDPARSE);
    }
  }

  if(UiPropManGle->paramChanged(MAN_PROP_GLE_START_LIST)) {

    different = false;

    bool autoload;
    OmWStringArray paths;

    ModMan->getStartHubs(&autoload, paths);

    if(UiPropManGle->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != autoload)
      different = true;

    if(!different) {

      size_t lb_cnt = UiPropManGle->msgItem(IDC_LB_PATH, LB_GETCOUNT);

      if(lb_cnt == paths.size()) {

          OmWString lb_entry;

          for(size_t i = 0; i < lb_cnt; ++i) {
            UiPropManGle->getLbText(IDC_LB_PATH, i, lb_entry);
            if(!Om_namesMatches(paths[i], lb_entry)) {
              different = true; break;
            }
          }

      } else {
        different = true;
      }
    }

    if(different) {
      changed = true;
    } else {
      UiPropManGle->paramReset(MAN_PROP_GLE_START_LIST);
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
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmUiPropManGle* UiPropManGle  = static_cast<OmUiPropManGle*>(this->childById(IDD_PROP_MGR_GLE));

  // Parameter: Icons size for packages List-View
  if(UiPropManGle->paramChanged(MAN_PROP_GLE_ICON_SIZE)) {

    switch(UiPropManGle->msgItem(IDC_CB_ICS, CB_GETCURSEL))
    {
    case 0:
      ModMan->setIconsSize(16);
      break;

    default:
      ModMan->setIconsSize(24);
      break;
    }

    // Reset parameter as unmodified
    UiPropManGle->paramReset(MAN_PROP_GLE_ICON_SIZE);
  }

  // Parameter No markdown parsing
  if(UiPropManGle->paramChanged(MAN_PROP_GLE_NO_MDPARSE)) {

    ModMan->setNoMarkdown(UiPropManGle->msgItem(IDC_BC_CKBX1, BM_GETCHECK));

    // Reset parameter as unmodified
    UiPropManGle->paramReset(MAN_PROP_GLE_NO_MDPARSE);
  }

  // Parameter: Open Mod Hub(s) at startup
  if(UiPropManGle->paramChanged(MAN_PROP_GLE_START_LIST)) {

    OmWString lb_entry;
    OmWStringArray paths;

    int32_t lb_cnt = UiPropManGle->msgItem(IDC_LB_PATH, LB_GETCOUNT);
    for(int32_t i = 0; i < lb_cnt; ++i) {
      UiPropManGle->getLbText(IDC_LB_PATH, i, lb_entry);
      paths.push_back(lb_entry);
    }

    ModMan->saveStartHubs(UiPropManGle->msgItem(IDC_BC_CKBX2, BM_GETCHECK), paths);

    // Reset parameter as unmodified
    UiPropManGle->paramReset(MAN_PROP_GLE_START_LIST);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  return true;
}
