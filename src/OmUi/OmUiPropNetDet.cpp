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

#include "OmNetPack.h"
#include "OmModPack.h"

#include "OmUiPropNet.h"

#include "OmUtilHsh.h"
#include "OmUtilWin.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropNetDet.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropNetDet::OmUiPropNetDet(HINSTANCE hins) : OmDialogPropTab(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropNetDet::~OmUiPropNetDet()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropNetDet::id() const
{
  return IDD_PROP_RMT_DET;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropNetDet::_onTabInit()
{
  // defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(14,400,L"Consolas");
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), 1);

  // define controls tool-tips
  this->_createTooltip(IDC_EC_READ1,  L"Mod identity, the raw filename without extension");
  this->_createTooltip(IDC_EC_READ2,  L"Mod core (or master) name, raw filename without version");
  this->_createTooltip(IDC_EC_READ3,  L"Mod version number");
  this->_createTooltip(IDC_EC_READ4,  L"Mod filename hash value, an unique file/directory identifier");
  this->_createTooltip(IDC_EC_READ5,  L"Mod download URLs");
  this->_createTooltip(IDC_EC_READ6,  L"Mod file size");
  this->_createTooltip(IDC_EC_READ7,  L"Mod file MD5 checksum");
  this->_createTooltip(IDC_EC_READ8,  L"List of Mods that this one supersedes as newer version");
  this->_createTooltip(IDC_EC_READ9,  L"List of Mods dependencies");
  this->_createTooltip(IDC_SB_SNAP,   L"Mod snapshot image");
  this->_createTooltip(IDC_EC_DESC,   L"Mod description text");

  OmNetPack* NetPack = static_cast<OmUiPropNet*>(this->_parent)->NetPack();
  if(!NetPack)
    return;

  // Ident
  this->setItemText(IDC_EC_READ1, NetPack->iden());

  // Core
  this->setItemText(IDC_EC_READ2, NetPack->core());

  // Parsed Version
  if(NetPack->version().valid()) {
    this->enableItem(IDC_EC_READ3, true);
    this->setItemText(IDC_EC_READ3, NetPack->version().asString());
  } else {
    this->enableItem(IDC_EC_READ3, false);
    this->setItemText(IDC_EC_READ3, L"N/A");
  }

  // Mod Category
  if(NetPack->category().size()) {
    this->enableItem(IDC_EC_READ4, true);
    this->setItemText(IDC_EC_READ4, NetPack->category());
  } else {
    this->enableItem(IDC_EC_READ4, false);
    this->setItemText(IDC_EC_READ4, L"<not defined>");
  }

  // Urls
  this->enableItem(IDC_EC_READ5, true);
  this->setItemText(IDC_EC_READ5, NetPack->fileUrl());

  // Size
  this->setItemText(IDC_EC_READ6, Om_formatSizeSysStr(NetPack->fileSize(), true));

  // Checksum
  if(NetPack->checksumIsMd5()) {
    this->setItemText(IDC_SC_LBL07, L"Checksum (md5) :");
  } else {
    this->setItemText(IDC_SC_LBL07, L"Checksum (xxh) :");
  }

  this->setItemText(IDC_EC_READ7, NetPack->fileChecksum());

  // Supersedes
  if(NetPack->upgradableCount()) {

    this->enableItem(IDC_EC_READ8, true);

    OmWString replace_list;

    for(unsigned i = 0; i < NetPack->upgradableCount(); ++i) {

      replace_list += NetPack->getUpgradable(i)->iden();

      if(i < (NetPack->upgradableCount() - 1))
        replace_list += L"\r\n";
    }

    this->setItemText(IDC_EC_READ8, replace_list);

  } else {
    this->enableItem(IDC_EC_READ8, false);
    this->setItemText(IDC_EC_READ8, L"None");
  }

  // Dependencies

  if(NetPack->dependCount()) {

    this->enableItem(IDC_EC_READ9, true);

    OmWString depend_list;

    for(unsigned i = 0; i < NetPack->dependCount(); ++i) {

      depend_list += NetPack->getDependIden(i);

      if(i < (NetPack->dependCount() - 1))
        depend_list += L"\r\n";
    }

    this->setItemText(IDC_EC_READ9, depend_list);

  } else {
    this->enableItem(IDC_EC_READ9, false);
    this->setItemText(IDC_EC_READ9, L"None");
  }

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropNetDet::_onTabResize()
{
  // Identity Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 5, 64, 9);
  this->_setItemPos(IDC_EC_READ1, 70, 5, this->cliUnitX()-90, 13);
  // Core Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 5, 20, 64, 9);
  this->_setItemPos(IDC_EC_READ2, 70, 20, this->cliUnitX()-90, 13);
  // Version Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 5, 35, 64, 9);
  this->_setItemPos(IDC_EC_READ3, 70, 35, this->cliUnitX()-90, 13);
  // Hash Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 5, 50, 64, 9);
  this->_setItemPos(IDC_EC_READ4, 70, 50, this->cliUnitX()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP01, 5, 66, this->cliUnitX()-25, 1);

  // Dowload URLs Label & EditControl
  this->_setItemPos(IDC_SC_LBL05, 5, 75, 64, 9);
  this->_setItemPos(IDC_EC_READ5, 70, 75, this->cliUnitX()-90, 13);
  // Size Label & EditControl
  this->_setItemPos(IDC_SC_LBL06, 5, 90, 64, 9);
  this->_setItemPos(IDC_EC_READ6, 70, 90, this->cliUnitX()-90, 13);
  // Md5sum Label & EditControl
  this->_setItemPos(IDC_SC_LBL07, 5, 105, 64, 9);
  this->_setItemPos(IDC_EC_READ7, 70, 105, this->cliUnitX()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP02, 5, 120, this->cliUnitX()-25, 1);

  // Supersedes Label & EditControl
  this->_setItemPos(IDC_SC_LBL08, 5, 125, 64, 9);
  this->_setItemPos(IDC_EC_READ8, 70, 125, this->cliUnitX()-90, 26);

  // separator
  this->_setItemPos(IDC_SC_SEP03, 5, 155, this->cliUnitX()-25, 1);

  // Dependencies Label & EditControl
  this->_setItemPos(IDC_SC_LBL09, 5, 160, 64, 9);
  this->_setItemPos(IDC_EC_READ9, 70, 160, this->cliUnitX()-90, 26);
  /*
  // separator
  this->_setItemPos(IDC_SC_SEP04, 5, 190, this->cliUnitX()-25, 1);

  // Package Source Snapshot Label
  this->_setItemPos(IDC_SC_LBL10, 5, 195, 64, 9);
  // Package Source Snapshot Image
  this->_setItemPos(IDC_SB_SNAP, 70, 195, 85, 78);

  // Package Source Description Label
  this->_setItemPos(IDC_SC_LBL11, 5, 278, 64, 9);
  // Package Source Description EditControl
  this->_setItemPos(IDC_EC_DESC, 70, 278, this->cliUnitX()-90, this->cliUnitY()-285);
  */
}
