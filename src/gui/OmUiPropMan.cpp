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
  OmManager* manager = static_cast<OmManager*>(this->_data);
  OmUiPropManGle* uiPropManGle  = static_cast<OmUiPropManGle*>(this->childById(IDD_PROP_MAN_GLE));
  OmUiPropManPkg* uiPropManPkg  = static_cast<OmUiPropManPkg*>(this->childById(IDD_PROP_MAN_PKG));

  bool changed = false;

  if(uiPropManGle->hasChParam(MAN_PROP_GLE_ICON_SIZE)) {

    int cb_sel = uiPropManGle->msgItem(IDC_CB_ISIZE, CB_GETCURSEL, 0, 0);

    switch(cb_sel)
    {
    case 0:
      if(manager->iconsSize() != 16)
        changed = true;
      break;
    case 2:
      if(manager->iconsSize() != 32)
        changed = true;
      break;
    default:
      if(manager->iconsSize() != 24)
        changed = true;
      break;
    }
  }

  if(uiPropManGle->hasChParam(MAN_PROP_GLE_STARTUP_CONTEXTS)) {
    changed = true;
  }

  if(uiPropManPkg->hasChParam(MAN_PROP_PKG_PACKAGE_FLAGS)) {
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
  OmManager* manager = static_cast<OmManager*>(this->_data);
  OmUiPropManGle* uiPropManGle  = static_cast<OmUiPropManGle*>(this->childById(IDD_PROP_MAN_GLE));
  OmUiPropManPkg* uiPropManPkg  = static_cast<OmUiPropManPkg*>(this->childById(IDD_PROP_MAN_PKG));

  // Parameter: Icons size for packages List-View
  if(uiPropManGle->hasChParam(MAN_PROP_GLE_ICON_SIZE)) {

    int cb_sel = uiPropManGle->msgItem(IDC_CB_ISIZE, CB_GETCURSEL);

    switch(cb_sel)
    {
    case 0:
      manager->setIconsSize(16);
      break;
    case 2:
      manager->setIconsSize(32);
      break;
    default:
      manager->setIconsSize(24);
      break;
    }

    // Reset parameter as unmodified
    uiPropManGle->setChParam(MAN_PROP_GLE_ICON_SIZE, false);
  }

  // Parameter: Open Context(s) at startup
  if(uiPropManGle->hasChParam(MAN_PROP_GLE_STARTUP_CONTEXTS)) {

    HWND hLb = uiPropManGle->getItem(IDC_LB_STRLS);

    int lb_cnt =  SendMessageW(hLb, LB_GETCOUNT, 0, 0);

    wstring item_str;

    vector<wstring> start_files;

    for(int i = 0; i < lb_cnt; ++i) {
      uiPropManGle->getItemText(IDC_LB_STRLS, item_str);
      start_files.push_back(item_str);
    }

    bool bm_chk = uiPropManGle->msgItem(IDC_BC_CHK01, BM_GETCHECK);

    manager->saveStartContexts(bm_chk, start_files);

    // Reset parameter as unmodified
    uiPropManGle->setChParam(MAN_PROP_GLE_STARTUP_CONTEXTS, false);
  }

  // Parameter: Various Packages options
  if(uiPropManPkg->hasChParam(MAN_PROP_PKG_PACKAGE_FLAGS)) {

    manager->setLegacySupport(uiPropManPkg->msgItem(IDC_BC_CHK01, BM_GETCHECK));
    manager->setWarnOverlaps(uiPropManPkg->msgItem(IDC_BC_CHK02, BM_GETCHECK));
    manager->setWarnExtraInst(uiPropManPkg->msgItem(IDC_BC_CHK03, BM_GETCHECK));
    manager->setWarnMissDpnd(uiPropManPkg->msgItem(IDC_BC_CHK04, BM_GETCHECK));
    manager->setWarnExtraUnin(uiPropManPkg->msgItem(IDC_BC_CHK05, BM_GETCHECK));
    manager->setQuietBatches(uiPropManPkg->msgItem(IDC_BC_CHK06, BM_GETCHECK));

    // Reset parameter as unmodified
    uiPropManPkg->setChParam(MAN_PROP_PKG_PACKAGE_FLAGS, false);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  // refresh all tree from the main dialog
  this->root()->refresh();

  return true;
}
