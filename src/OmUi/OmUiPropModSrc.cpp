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

#include "OmModPack.h"

#include "OmUiPropMod.h"

#include "OmUtilHsh.h"
#include "OmUtilWin.h"
#include "OmUtilFs.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropModSrc.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropModSrc::OmUiPropModSrc(HINSTANCE hins) : OmDialogPropTab(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropModSrc::~OmUiPropModSrc()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropModSrc::id() const
{
  return IDD_PROP_PKG_SRC;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropModSrc::_onTabInit()
{
  // defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(14,400,L"Consolas");
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), 1);

  // define controls tool-tips
  this->_createTooltip(IDC_EC_READ1,  L"Mod source type, either ZIP archive or Directory (developer mode)");
  this->_createTooltip(IDC_EC_READ2,  L"Mod source location path");
  this->_createTooltip(IDC_EC_READ3,  L"List of Mod dependencies");

 OmModPack* ModPack = static_cast<OmUiPropMod*>(this->_parent)->getModPack();
  if(!ModPack)
    return;

  if(ModPack->hasSource()) {

    // Type
    if(ModPack->sourceIsDir()) {
      this->setItemText(IDC_EC_READ1, L"Directory");
    } else {
      this->setItemText(IDC_EC_READ1, L"Archive file");
    }

    // Location
    this->enableItem(IDC_EC_READ2, true);
    this->setItemText(IDC_EC_READ2, ModPack->sourcePath());

    // Dependencies
    if(ModPack->dependCount()) {

      this->enableItem(IDC_EC_READ3, true);

      OmWString depend_list;

      size_t depend_count = ModPack->dependCount();

      for(size_t i = 0; i < depend_count; ++i) {

        depend_list += ModPack->getDependIden(i);

        if(i < (depend_count - 1))
          depend_list += L"\r\n";
      }

      this->setItemText(IDC_EC_READ3, depend_list);
    } else {
      this->setItemText(IDC_EC_READ3, L"None");
      this->enableItem(IDC_EC_READ3, false);
    }

    // Total Size
    this->enableItem(IDC_EC_READ4, true);
    this->setItemText(IDC_EC_READ4, Om_formatSizeSysStr(Om_itemSize(ModPack->sourcePath()), true));

    // Source Files
    if(ModPack->sourceEntryCount()) {

      this->enableItem(IDC_EC_READ5, true);

      OmWString path_list;

      for(size_t i = 0; i < ModPack->sourceEntryCount(); ++i) {
        path_list += ModPack->getSourceEntry(i).path;
        path_list += L"\r\n";
      }

      this->setItemText(IDC_EC_READ5, path_list);
    } else {
      this->setItemText(IDC_EC_READ5, L"None");
      this->enableItem(IDC_EC_READ5, false);
    }
  } else {
    // Type
    this->setItemText(IDC_EC_READ1, L"None");
    this->enableItem(IDC_EC_READ1, false);

    // Location
    this->setItemText(IDC_EC_READ2, L"N/A");
    this->enableItem(IDC_EC_READ2, false);

    // Dependencies
    this->setItemText(IDC_EC_READ3, L"N/A");
    this->enableItem(IDC_EC_READ3, false);

    // Total Size
    this->setItemText(IDC_EC_READ4, L"N/A");
    this->enableItem(IDC_EC_READ4, false);

    // Installed Files
    this->setItemText(IDC_EC_READ5, L"");
    this->enableItem(IDC_EC_READ5, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropModSrc::_onTabResize()
{
  // Source Type Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 10, 64, 9);
  this->_setItemPos(IDC_EC_READ1, 70, 10, this->cliUnitX()-90, 13);
  // Source Location Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 5, 26, 64, 9);
  this->_setItemPos(IDC_EC_READ2, 70, 26, this->cliUnitX()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP01, 5, 47, this->cliUnitX()-25, 1);

  // Dependencies Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 5, 58, 64, 9);
  this->_setItemPos(IDC_EC_READ3, 70, 58, this->cliUnitX()-90, 50);

  // separator
  this->_setItemPos(IDC_SC_SEP02, 5, 115, this->cliUnitX()-25, 1);

  // Package Source Total size Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 5, 126, 64, 9);
  this->_setItemPos(IDC_EC_READ4, 70, 126, this->cliUnitX()-90, 13);
  // Package Source Files Label
  this->_setItemPos(IDC_SC_LBL05, 5, 142, 64, 9);
  // Package Source Files EditControl
  this->_setItemPos(IDC_EC_READ5, 70, 144, this->cliUnitX()-90, this->cliUnitY()-155);
}
