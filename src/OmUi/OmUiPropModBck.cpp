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

#include "OmModChan.h"
#include "OmModPack.h"

#include "OmUiPropMod.h"

#include "OmUtilFs.h"
#include "OmUtilHsh.h"
#include "OmUtilWin.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropModBck.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropModBck::OmUiPropModBck(HINSTANCE hins) : OmDialogPropTab(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropModBck::~OmUiPropModBck()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropModBck::id() const
{
  return IDD_PROP_PKG_BCK;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropModBck::_onTabInit()
{
  // defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(14, 400, L"Consolas");
  this->msgItem(IDC_EC_READ4, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  // define controls tool-tips
  this->_createTooltip(IDC_EC_READ1,  L"Mod backup packing type, either ZIP archive or Directory");
  this->_createTooltip(IDC_EC_READ2,  L"Mod backup pack location path");
  this->_createTooltip(IDC_EC_READ3,  L"List of Mods that the installation of this one overlaps");
  this->_createTooltip(IDC_EC_READ4,  L"If any, the total size of current backup data");
  this->_createTooltip(IDC_EC_READ5,  L"If any, list of currently installed files");

 OmModPack* ModPack = static_cast<OmUiPropMod*>(this->_parent)->getModPack();
  if(!ModPack)
    return;

  if(ModPack->hasBackup()) {

    // Type
    if(Om_isDir(ModPack->backupPath())) {
      this->setItemText(IDC_EC_READ1, L"Directory");
    } else {
      this->setItemText(IDC_EC_READ1, L"Compressed archive file");
    }

    // Location
    this->enableItem(IDC_EC_READ2, true);
    this->setItemText(IDC_EC_READ2, ModPack->backupPath());

    // Overlap by
    if(ModPack->overlapCount()) {

      this->enableItem(IDC_EC_READ3, true);

      OmModChan* ModChan = ModPack->ModChan();

      OmWString overlap_list;

      size_t overlap_count = ModPack->overlapCount();
      for(size_t i = 0; i < overlap_count; ++i) {

        uint64_t overlap_hash = ModPack->getOverlapHash(i);

        OmModPack* overlap_pack = ModChan->findModpack(overlap_hash);

        if(overlap_pack) {
          overlap_list += overlap_pack->iden();
        } else {
          overlap_list += L'<';
          overlap_list += Om_uint64ToStr(overlap_hash);
          overlap_list += L'>';
        }

        if(i < (overlap_count - 1))
          overlap_list += L"\r\n";

      }


      this->setItemText(IDC_EC_READ3, overlap_list);
    } else {
      this->setItemText(IDC_EC_READ3, L"None");
      this->enableItem(IDC_EC_READ3, false);
    }

    // Total Size
    this->enableItem(IDC_EC_READ4, true);
    this->setItemText(IDC_EC_READ4, Om_formatSizeSysStr(Om_itemSize(ModPack->backupPath()), true));

    // Installed Files
    if(ModPack->backupEntryCount()) {

      this->enableItem(IDC_EC_READ5, true);

      OmWString entry_list;

      for(size_t i = 0; i < ModPack->backupEntryCount(); ++i) {

        if(OM_HAS_BIT(ModPack->getBackupEntry(i).attr, OM_MODENTRY_DEL)) {
          entry_list += L"+  ";
        } else {
          entry_list += L"≠  ";
        }

        entry_list += ModPack->getBackupEntry(i).path;
        entry_list += L"\r\n";
      }

      this->setItemText(IDC_EC_READ5, entry_list);
    } else {
      this->setItemText(IDC_EC_READ5, L"None");
      this->enableItem(IDC_EC_READ5, false);
    }

  } else {

    // Type
    this->setItemText(IDC_EC_READ1, L"None");

    // Location
    this->setItemText(IDC_EC_READ2, L"N/A");
    this->enableItem(IDC_EC_READ2, false);

    // Overlap by
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
void OmUiPropModBck::_onTabResize()
{
  // Package Backup Type Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 5, 10, 64, 9);
  this->_setItemPos(IDC_EC_READ1, 70, 10, this->cliUnitX()-90, 13);
  // Package Backup Location Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 5, 26, 64, 9);
  this->_setItemPos(IDC_EC_READ2, 70, 26, this->cliUnitX()-90, 13);

  // separator
  this->_setItemPos(IDC_SC_SEP01, 5, 47, this->cliUnitX()-25, 1);

  // Package Backup Overlap Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 5, 58, 64, 9);
  this->_setItemPos(IDC_EC_READ3, 70, 58, this->cliUnitX()-90, 47);

  // separator
  this->_setItemPos(IDC_SC_SEP02, 5, 115, this->cliUnitX()-25, 1);

  // Package Backup Total size Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 5, 126, 64, 9);
  this->_setItemPos(IDC_EC_READ4, 70, 126, this->cliUnitX()-90, 13);
  // Package Backup Installed Files Label
  this->_setItemPos(IDC_SC_LBL05, 5, 142, 64, 9);
  // Package Backup Installed Files EditControl
  this->_setItemPos(IDC_EC_READ5, 70, 144, this->cliUnitX()-90, this->cliUnitY()-155);
}
