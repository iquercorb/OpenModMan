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

#include "OmBaseWin.h"
  #include <UxTheme.h>
  #include <vssym32.h>      //< for custom draw
  #include <ShlWApi.h>      //< StrFromTimeIntervalW


#include "OmModMan.h"
#include "OmNetRepo.h"
#include "OmNetPack.h"
#include "OmModPack.h"

#include "OmUiMan.h"
#include "OmUiManMain.h"
#include "OmUiManFoot.h"
#include "OmUiAddChn.h"
#include "OmUiAddRep.h"
#include "OmUiPropNet.h"

#include "OmUtilFs.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiManMainNet.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManMainNet::OmUiManMainNet(HINSTANCE hins) : OmDialog(hins),
  _UiMan(nullptr),
  _query_count(0),
  _download_upgrd(false),
  _download_count(0),
  _upgrade_count(0),
  _lv_rep_icons_size(0),
  _lv_net_icons_size(0),
  _lv_net_cdraw_htheme(nullptr)
{
  // set the accelerator table for the dialog
  this->setAccel(IDR_ACCEL);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManMainNet::~OmUiManMainNet()
{
  // Get the previous Image List to be destroyed (Small and Normal uses the same)
  HIMAGELIST hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_NET, LVM_GETIMAGELIST, LVSIL_NORMAL));
  if(hImgLs) ImageList_Destroy(hImgLs);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiManMainNet::id() const
{
  return IDD_MGR_MAIN_NET;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  // Repository Label ListBox & buttons
  this->enableItem(IDC_SC_LBL01, !enable);
  this->enableItem(IDC_LV_REP, !enable);

  // Repository Buttons
  this->enableItem(IDC_BC_NEW, !enable);
  this->enableItem(IDC_BC_DEL, false);

  // then, user still can use Remote ListView
  // to watch, add or cancel downloads
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif

  if(!enable) {
    if(this->visible())
      this->_onRefresh();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::refreshLibrary()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::refreshLibrary\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  if(ModMan)  //< this should be always the case
    if(ModMan->activeChannel()) //< this should also be always the case
      this->_lv_net_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::queueDownloads(bool upgrade)
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT))
    return;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkLibraryWrite(L"Install Mods"))
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // get current selected Net Pack in ListView
  OmPNetPackArray selection, downloads;

  int32_t lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    OmNetPack* NetPack = ModChan->getNetpack(lv_sel);

    if(!NetPack->hasLocal() && !NetPack->isDownloading())
      selection.push_back(NetPack);

    // next selected item
    lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  OmWStringArray depends, missings, breakings;

  // prepare download
  ModChan->prepareDownloads(selection, &downloads, &depends, &missings, &breakings);

  if(!this->_UiMan->warnMissings(ModChan->warnMissDnld(), L"Download Mods", missings))
    return;

  if(!this->_UiMan->warnBreakings(ModChan->warnUpgdBrkDeps(), L"Download Mods", breakings))
    return;

  if(!this->_UiMan->warnExtraDownloads(ModChan->warnExtraDnld(), L"Download Mods", depends))
    return;

  this->_download_start(upgrade, downloads);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::downloadDepends(bool upgrade)
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkLibraryWrite(L"Install Mods"))
    return;

  // get single selection
  int32_t lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0) return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmNetPack* NetPack = ModChan->getNetpack(lv_sel);

  // this should never happen be we handle it
  if(!NetPack->hasLocal())
    return;

  // here we go like a full download
  OmPNetPackArray depends;  //< dependency download list
  OmWStringArray missings;    //< missing dependencies lists

  // Get remote package depdencies
  ModChan->getNetpackDepends(NetPack, &depends, &missings);

  // warn user for missing dependencies
  if(!this->_UiMan->warnMissings(ModChan->warnMissDnld(), L"Download Mods", missings))
    return;

  if(depends.empty())
    return;

  this->_download_start(upgrade, depends);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::stopDownloads()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT))
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // 1. get current selected Net Pack in ListView
  int lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    // stop single download
    ModChan->stopDownload(lv_sel);

    // next selected item
    lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::revokeDownloads()
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT))
    return;

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmModChan* ModChan = ModMan->activeChannel();
  if(!ModChan)
    return;

  // 1. get current selected Net Pack in ListView
  int lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    OmNetPack* NetPack = ModChan->getNetpack(lv_sel);

    NetPack->revokeDownload();

    // next selected item
    lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }

  this->_lv_net_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::showProperties() const
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmModChan* ModChan = ModMan->activeChannel();
  if(!ModChan)
    return;

  OmNetPack* NetPack = ModChan->getNetpack(lv_sel);

  if(NetPack) {

    OmUiPropNet* UiPropRmt = static_cast<OmUiPropNet*>(this->_UiMan->childById(IDD_PROP_RMT));

    UiPropRmt->setNetPack(NetPack);

    UiPropRmt->open(true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_update_safemode_status()
{
  bool threading = this->_query_count || this->_download_count || this->_upgrade_count;

  this->_setSafe(!threading);

  this->_UiMan->freeze(threading);

  this->enableItem(IDC_BC_ABORT, threading);

  this->msgItem(IDC_PB_PKG, PBM_SETPOS, 0, 0);
  this->enableItem(IDC_PB_PKG, threading);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_query_abort()
{
  if(this->_query_count) {

    OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

    OmModChan* ModChan = ModMan->activeChannel();
    if(!ModChan)
      return;

    // cancel queries
    ModChan->abortQueries();

    // change button image from stop to refresh
    this->setBmIcon(IDC_BC_QRY, Om_getResIcon(this->_hins, IDI_BT_REF));
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_query_start(const OmPNetRepoArray& selection)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // change button image from refresh to stop
  this->setBmIcon(IDC_BC_QRY, Om_getResIcon(this->_hins, IDI_BT_NOT));

  // increase count of queued queries
  this->_query_count += selection.size();

  // operation is running
  this->_update_safemode_status();

  // change status icon
  LVITEMW lvI = {LVIF_IMAGE, 0, 0}; lvI.iImage = ICON_STS_QUE;
  for(size_t i = 0; i < selection.size(); ++i) {
    lvI.iItem = ModChan->indexOfRepository(selection[i]);
    this->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  // send query for this Mod Repo
  ModChan->queueQueries(selection, OmUiManMainNet::_query_begin_fn, OmUiManMainNet::_query_result_fn, this);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_query_begin_fn(void* ptr, uint64_t param)
{
  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetRepo* NetRepo = reinterpret_cast<OmNetRepo*>(param);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();
  if(!ModChan)
    return;

  // change status icon
  LVITEMW lvI = {LVIF_IMAGE, ModChan->indexOfRepository(NetRepo), 0}; lvI.iImage = ICON_STS_WIP;
  self->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_query_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetRepo* NetRepo = reinterpret_cast<OmNetRepo*>(param);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();
  if(!ModChan) return;

  LVITEMW lvI = {LVIF_IMAGE, ModChan->indexOfRepository(NetRepo), 0};
  lvI.iImage = self->_lv_rep_get_status_icon(NetRepo);
  self->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

  // update progression bar
  self->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
  self->msgItem(IDC_PB_PKG, PBM_SETPOS, ModChan->queriesProgress());

  // refresh network library list
  self->_lv_net_populate();

  // decrease concurrent query count
  self->_query_count--;

  if(!self->_query_count) {

    // change button image from stop to refresh
    self->setBmIcon(IDC_BC_QRY, Om_getResIcon(self->_hins, IDI_BT_REF));

    // operation is stopping
    self->_update_safemode_status();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_download_abort()
{
  if(this->_download_count) {

    OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
    if(!ModChan)
      return;

    ModChan->stopDownloads();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_download_start(bool upgrade, const OmPNetPackArray& selection)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // do (or do not) upgrade at download end
  this->_download_upgrd = upgrade;

  // increase count of queued download
  this->_download_count += selection.size();

  // operation is running
  this->_update_safemode_status();

  // change status icon
  LVITEMW lvI = {LVIF_IMAGE, 0, 0}; lvI.iImage = ICON_STS_DNL;
  for(size_t i = 0; i < selection.size(); ++i) {
    lvI.iItem = ModChan->indexOfNetpack(selection[i]);
    this->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  // start or append downloads
  ModChan->queueDownloads(selection, OmUiManMainNet::_download_download_fn, OmUiManMainNet::_download_result_fn, this);

  // Enable 'Abort' and disable 'Download'
  if(this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT)) {
    this->enableItem(IDC_BC_STOP, true);
    this->enableItem(IDC_BC_DNLD, false);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiManMainNet::_download_download_fn(void* ptr, int64_t tot, int64_t cur, int64_t rate, uint64_t param)
{
  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();
  if(!ModChan)
    return true;

  // retrieve index of Net Pack in Mod Channel and therefor in ListView
  int32_t item_id = ModChan->indexOfNetpack(NetPack);

  // update global progression
  self->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
  self->msgItem(IDC_PB_PKG, PBM_SETPOS, ModChan->downloadsProgress());

  // Invalidate ListView subitem rect to call custom draw (progress bar)
  RECT rect;
  self->getLvSubRect(IDC_LV_NET, item_id, 5 /* 'Progress' column */, &rect);
  self->redrawItem(IDC_LV_NET, &rect, RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);

  return true; //< continue
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_download_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();
  if(!ModChan)
    return;

  // retrieve index of Net Pack in Mod Channel and therefor in ListView
  int32_t item_id = ModChan->indexOfNetpack(NetPack);

  // Set 'Stp' and 'Download" buttons if item currently selected
  if(self->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT) == 1) {
    if(self->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED) == item_id) {
      self->enableItem(IDC_BC_STOP, false);
      self->enableItem(IDC_BC_DNLD, !NetPack->hasLocal());
    }
  }

  // change status icon
  LVITEMW lvI = {LVIF_IMAGE, item_id, 0}; lvI.iImage = self->_lv_net_get_status_icon(NetPack);
  self->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

  // if an error occurred, display error dialog
  if(NetPack->hasError()) {
      Om_dlgBox_okl(self->_hwnd, L"Download Mods", IDI_PKG_ERR,
                  L"Mod download error", L"The download of \""
                  +NetPack->iden()+L"\" failed:", NetPack->lastError());
  }

  // decrease concurrent download count
  self->_download_count--;

  if(!self->_download_count)
    // operation is ending
    self->_update_safemode_status();

  // if we are downloading for upgrade, here we go
  if(self->_download_upgrd) {
    if(NetPack->upgradableCount())
      self->_upgrade_start(OmPNetPackArray(1, NetPack));
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_upgrade_abort()
{
  if(this->_download_count) {

    OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
    if(!ModChan) return;

    ModChan->abortUpgrades();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_upgrade_start(const OmPNetPackArray& selection)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  this->_upgrade_count += selection.size();

  // properly uninstall and replace old Mods, then install this one
  ModChan->queueUpgrades(selection, OmUiManMainNet::_upgrade_begin_fn, OmUiManMainNet::_upgrade_progress_fn, OmUiManMainNet::_upgrade_result_fn, this);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_upgrade_begin_fn(void* ptr, uint64_t param)
{
  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();
  if(!ModChan) return;

  // change status icon
  LVITEMW lvI = {LVIF_IMAGE, ModChan->indexOfNetpack(NetPack), 0}; lvI.iImage = ICON_STS_WIP;
  self->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiManMainNet::_upgrade_progress_fn(void* ptr, size_t cur, size_t tot, uint64_t param)
{
  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();
  if(!ModChan) return true;

  // retrieve NetPack index in list
  int32_t item_id = ModChan->indexOfNetpack(NetPack);

  // Invalidate ListView subitem rect to call custom draw (progress bar)
  RECT rect;
  self->getLvSubRect(IDC_LV_NET, item_id, 5 /* 'Progress' column */, &rect);
  self->redrawItem(IDC_LV_NET, &rect, RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);

  // update global progression, but prevent interfering with current downloading
  if(!self->_download_count) {
    self->msgItem(IDC_PB_PKG, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    self->msgItem(IDC_PB_PKG, PBM_SETPOS, ModChan->upgradesProgress());
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_upgrade_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  OmModChan* ModChan = static_cast<OmModMan*>(self->_data)->activeChannel();
  if(!ModChan)
    return;

  // structure for ListView update
  LVITEMW lvI = {LVIF_IMAGE, ModChan->indexOfNetpack(NetPack), 0};
  lvI.iImage = self->_lv_net_get_status_icon(NetPack);
  self->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

  if(result == OM_RESULT_ERROR) {
    Om_dlgBox_okl(self->_hwnd, L"Upgrade Mods", IDI_PKG_ERR,
                L"Mod upgrade error", L"The upgrading of \""
                +NetPack->core()+L"\" failed:", NetPack->lastError());
  }

  // decrease pending upgrade count
  self->_upgrade_count--;

  if(!self->_upgrade_count) {
    // operation is ending
    self->_update_safemode_status();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_rep_populate()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::_buildLvRep\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // if icon size changed we must 'reload' ImageList
  if(this->_lv_rep_icons_size != ModMan->iconsSize()) {
    LPARAM lparam = reinterpret_cast<LPARAM>(this->_UiMan->listViewImgList());
    this->msgItem(IDC_LV_REP, LVM_SETIMAGELIST, LVSIL_SMALL, lparam);
    this->msgItem(IDC_LV_REP, LVM_SETIMAGELIST, LVSIL_NORMAL, lparam);
    this->_lv_rep_icons_size = ModMan->iconsSize(); //< update size
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_REP, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty list view
  this->msgItem(IDC_LV_REP, LVM_DELETEALLITEMS);

  // get current context and location
  OmModChan* ModChan = ModMan->activeChannel();

  if(!ModChan) {
    // disable ListView
    this->enableItem(IDC_LV_REP, false);
    // disable query button
    this->enableItem(IDC_BC_QRY, false);
    // update Repositories ListView selection
    this->_lv_rep_on_selchg();
    // return now
    return;
  }

  OmWString lv_entry;

  for(size_t i = 0; i < ModChan->repositoryCount(); ++i) {

    OmNetRepo* NetRepo = ModChan->getRepository(i);

    LVITEMW lvI = {0, static_cast<int>(i)};

    // the first column, repository status, here we INSERT the new item
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE;
    lvI.iImage = this->_lv_rep_get_status_icon(NetRepo);
    this->msgItem(IDC_LV_REP, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Second column, the repository address, now empty, here we set the sub-item
    lvI.iSubItem = 1; lvI.mask = LVIF_TEXT|LVIF_IMAGE;
    lvI.iImage = ICON_REP;

    lv_entry = NetRepo->urlBase();
    lv_entry += L" -- ";
    lv_entry += NetRepo->urlName();

    lvI.pszText = const_cast<LPWSTR>(lv_entry.c_str());
    this->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Third column, the repository title
    lvI.iSubItem = 2; lvI.mask = LVIF_TEXT;
    lvI.pszText = const_cast<LPWSTR>(NetRepo->title().c_str());
    this->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  // we enable the List-View
  this->enableItem(IDC_LV_REP, true);

  // restore list-view scroll position from lvRec
  this->msgItem(IDC_LV_REP, LVM_SCROLL, 0, -lvRec.top );

  // enable or disable query button
  this->enableItem(IDC_BC_QRY, (ModChan->repositoryCount() > 0));

  // resize ListView columns adapted to client area
  this->_lv_rep_on_resize();

  // update ListView selection
  this->_lv_rep_on_selchg();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_rep_on_resize()
{
  LONG size[4], half_s;

  GetClientRect(this->getItem(IDC_LV_REP), reinterpret_cast<LPRECT>(&size));
  half_s = static_cast<float>(size[2]) * 0.5f;
  this->msgItem(IDC_LV_REP, LVM_SETCOLUMNWIDTH, 1, half_s - 27);
  this->msgItem(IDC_LV_REP, LVM_SETCOLUMNWIDTH, 2, half_s - 26);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_rep_on_selchg()
{
  // get count of selected item
  unsigned lv_nsl = this->msgItem(IDC_LV_REP, LVM_GETSELECTEDCOUNT);

  this->enableItem(IDC_BC_DEL, (lv_nsl > 0));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_rep_on_dblclk()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan)
    return;

  // prevent useless processing
  if(this->msgItem(IDC_LV_REP, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_REP, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0)
    return;

  OmPNetRepoArray selection;

  selection.push_back(ModChan->getRepository(lv_sel));

  this->_query_start(selection);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmUiManMainNet::_lv_rep_get_status_icon(const OmNetRepo* NetRepo)
{
  if(NetRepo->queryResult() >= OM_RESULT_OK) {
    if(NetRepo->queryResult() == OM_RESULT_OK) {
      return ICON_STS_CHK;
    } else if(OM_HAS_BIT(NetRepo->queryResult(), OM_RESULT_ERROR)) {
      return ICON_STS_ERR;
    }
  } else if(NetRepo->queryResult() == OM_RESULT_PENDING) {
    return ICON_STS_WIP;
  }

  return ICON_NONE;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_net_populate()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::_buildLvNet\n";
  #endif

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  // if icon size changed we must 'reload' ImageList
  if(this->_lv_net_icons_size != ModMan->iconsSize()) {
    LPARAM lparam = reinterpret_cast<LPARAM>(this->_UiMan->listViewImgList());
    this->msgItem(IDC_LV_NET, LVM_SETIMAGELIST, LVSIL_SMALL, lparam);
    this->msgItem(IDC_LV_NET, LVM_SETIMAGELIST, LVSIL_NORMAL, lparam);
    this->_lv_net_icons_size = ModMan->iconsSize(); //< update size
  }

  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_NET, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));

  // empty list view
  this->msgItem(IDC_LV_NET, LVM_DELETEALLITEMS);

  // get current context and location
  OmModChan* ModChan = ModMan->activeChannel();

  if(!ModChan) {
    // disable ListView
    this->enableItem(IDC_LV_NET, false);
    // update Package ListView selection
    this->_lv_net_on_selchg();
    // return now
    return;
  }

  // add item to list view
  for(size_t i = 0; i < ModChan->netpackCount(); ++i) {

    OmNetPack* NetPack = ModChan->getNetpack(i);

    LVITEMW lvI = {0, static_cast<int>(i)};

    // the first column, mod status, here we INSERT the new item
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE|LVIF_PARAM; //< icon and special data
    lvI.iImage = this->_lv_net_get_status_icon(NetPack);
    // notice for later : to work properly the lParam must be
    // defined on the first SubItem (iSubItem = 0)
    lvI.lParam = reinterpret_cast<LPARAM>(NetPack);
    this->msgItem(IDC_LV_NET, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Second column, the mod name and type, here we set the sub-item
    lvI.iSubItem = 1; lvI.mask = LVIF_TEXT|LVIF_IMAGE; // "Name" Collumn
    lvI.iImage = NetPack->dependCount() ? ICON_MOD_DEP : ICON_MOD_PKG;
    lvI.pszText = const_cast<LPWSTR>(NetPack->name().c_str());
    this->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Third column, the mod version, we set the sub-item
    lvI.iSubItem = 2; lvI.mask = LVIF_TEXT;
    lvI.pszText = const_cast<LPWSTR>(NetPack->version().asString().c_str());
    this->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Fourth column, the mod category, we set the sub-item
    lvI.iSubItem = 3; lvI.mask = LVIF_TEXT;
    lvI.pszText = const_cast<LPWSTR>(NetPack->category().c_str());
    this->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Fifth column, the mod size, we set the sub-item
    lvI.iSubItem = 4; lvI.mask = LVIF_TEXT;
    lvI.pszText = const_cast<LPWSTR>(Om_formatSizeSysStr(NetPack->fileSize(), true).c_str());
    this->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Sixth column, the operation progress
    // this sub-item is handled via custom draw routine
  }

  // we enable the ListView
  this->enableItem(IDC_LV_NET, true);

  // restore ListView scroll position from lvRec
  this->msgItem(IDC_LV_NET, LVM_SCROLL, 0, -lvRec.top );

  // resize ListView columns adapted to client area
  this->_lv_net_on_resize();

  // update Package ListView selection
  this->_lv_net_on_selchg();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_net_on_resize()
{
  LONG size[4];
  // Resize the ListView column
  GetClientRect(this->getItem(IDC_LV_NET), reinterpret_cast<LPRECT>(&size));
  this->msgItem(IDC_LV_NET, LVM_SETCOLUMNWIDTH, 1, size[2]-413);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_net_cdraw_progress(HDC hDc, uint64_t item, int32_t subitem)
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmModChan* ModChan = ModMan->activeChannel();
  if(!ModChan) return;

  OmNetPack* NetPack = ModChan->getNetpack(item);

  if(!NetPack->isDownloading() && !NetPack->isUpgrading())
    return;

  // get rectangle of subitem to draw
  RECT rect, bar_rect, txt_rect;
  this->getLvSubRect(IDC_LV_NET, item, subitem /* 'Progress' column */, &rect);

  double progress = 0.0;
  wchar_t item_str[OM_MAX_ITEM];

  if(NetPack->isDownloading()) {

    // progress ratio from download percent
    progress = (double)NetPack->downloadProgress() * 0.01;

    // create remaining time string
    StrFromTimeIntervalW(item_str, OM_MAX_ITEM, static_cast<uint32_t>(NetPack->downloadRemain()) * 1000, 3);
  }

  if(NetPack->isUpgrading()) {

    // progress ratio from upgrading percent
    progress = (double)NetPack->upgradeProgress() * 0.01;

    // laconic text...
    swprintf(item_str, OM_MAX_ITEM, L"Cleaning for upgrade");
  }

  // all drawing rectangle base on subitem rectangle
  bar_rect = txt_rect = rect;

  // give some padding
  bar_rect.left += 1; bar_rect.right -= 2; bar_rect.top += 1; bar_rect.bottom -= 1;

  // resize horizontally according download percentage
  bar_rect.right = bar_rect.left + ((bar_rect.right - bar_rect.left) * progress);

  // draw rectangle using "Progress" theme
  DrawThemeBackground(this->_lv_net_cdraw_htheme, hDc, PP_CHUNK, 0, &bar_rect, nullptr);

  // add padding
  txt_rect.left +=7;

  // draw text left-aligned
  DrawTextW(hDc, item_str, -1, &txt_rect, DT_LEFT|DT_VCENTER|DT_WORD_ELLIPSIS|DT_SINGLELINE);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_net_on_selchg()
{
  // get count of selected item
  int32_t lv_nsl = this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT);

  // handle to "Edit > Remote >" sub-menu
  HMENU hPopup = this->_UiMan->getPopupItem(MNU_EDIT, MNU_EDIT_NET);

  if(!lv_nsl) {

    this->enableItem(IDC_BC_DNLD, false);
    this->enableItem(IDC_BC_STOP, false);

    // disable "Edit > Remote []" pop-up menu
    this->_UiMan->setPopupItem(MNU_EDIT, MNU_EDIT_NET, MF_GRAYED);

    // disable all menu-item (for right click menu)
    for(int32_t i = 0; i < 6; ++i)
      this->_UiMan->setPopupItem(hPopup, i, MF_GRAYED);

    // show nothing in footer frame
    this->_UiMan->pUiMgrFoot()->clearItem();

    // return now
    return;
  }

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // at least one selected, enable "Edit > Remote []" pop-up menu
  this->_UiMan->setPopupItem(MNU_EDIT, MNU_EDIT_NET, MF_ENABLED);

  if(lv_nsl > 1) {

    // multiple selection, we allow more than one download at a time
    this->enableItem(IDC_BC_DNLD, true); //< enable anyway when multiple selection

    // enable and disable "Edit > Remote" menu-items
    this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_DNLD, MF_ENABLED); //< "Download" menu-item
    this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_DNWS, MF_ENABLED); //< "Download without supersede" menu-item
    this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_ABRT, MF_ENABLED); //< "Abort/Pause download" menu-item
    this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_RVOK, MF_ENABLED); //< "Revoke partial download" menu-item
    this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_FIXD, MF_GRAYED); //< "Fix dependencies" menu-item
    this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_INFO, MF_GRAYED); //< "View detail..." menu-item

    // on multiple selection, we hide package description
    this->_UiMan->pUiMgrFoot()->clearItem();

  } else {

    // enable the "Edit > Remote > .. " menu-item
    this->_UiMan->setPopupItem(hPopup, 5, MF_ENABLED); //< "View details" menu-item

    // get the selected item id (only one, no need to iterate)
    int32_t lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
    if(lv_sel >= 0) {

      OmNetPack* NetPack = ModChan->getNetpack(lv_sel);

      // show packages info in footer frame
      this->_UiMan->pUiMgrFoot()->selectItem(NetPack);

      // get remote package states
      bool can_download = !NetPack->hasLocal() && !NetPack->isDownloading();

      // enable and disable "Edit > Remote" menu-items
      this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_DNLD, can_download ? MF_ENABLED : MF_GRAYED); //< "Download" menu-item
      this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_DNWS, can_download ? MF_ENABLED : MF_GRAYED); //< "Download without supersede" menu-item
      this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_ABRT, NetPack->isDownloading() ? MF_ENABLED : MF_GRAYED); //< "Abort/Pause download" menu-item
      this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_RVOK, NetPack->isResumable() ? MF_ENABLED : MF_GRAYED); //< "Revoke partial download" menu-item
      this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_FIXD, NetPack->hasMissingDepend() ? MF_ENABLED : MF_GRAYED); //< "Fix dependencies" menu-item
      this->_UiMan->setPopupItem(hPopup, MNU_EDIT_NET_INFO, MF_ENABLED); //< "View detail..." menu-item

      this->enableItem(IDC_BC_DNLD, can_download);
      this->enableItem(IDC_BC_STOP, NetPack->isDownloading());

    } else {

      // reset footer frame
      this->_UiMan->pUiMgrFoot()->clearItem();
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_net_on_dblclk()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // prevent useless processing
  if(this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int32_t lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0) return;

  // check whether we must start or stop download
  if(ModChan->getNetpack(lv_sel)->isDownloading()) {
    this->stopDownloads();
  } else {
    this->queueDownloads(true);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmUiManMainNet::_lv_net_get_status_icon(const OmNetPack* NetPack)
{
  // select proper status icon
  if(NetPack->hasError()) {
    return ICON_STS_ERR;
  } else if(NetPack->hasLocal()) {
    if(NetPack->isUpgrading()) {
      return ICON_STS_WIP;
    } else if(NetPack->hasMissingDepend()) {
      return ICON_STS_WRN;
    } else {
      return ICON_STS_CHK;
    }
  } else if(NetPack->isDownloading()) {
    return ICON_STS_DNL;
  } else {
    if(NetPack->isResumable()) {
      return ICON_STS_RES;
    } else if(NetPack->upgradableCount()) {
      return ICON_STS_UPG;
    } else if(NetPack->upgradableCount()) {
      return ICON_STS_DNG;
    } else {
      return ICON_STS_NEW;
    }
  }

  return ICON_NONE;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_net_on_rclick()
{
  // get handle to "Edit > Remote..." sub-menu
  HMENU hMenu = this->_UiMan->getPopupItem(MNU_EDIT, MNU_EDIT_NET);

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  TrackPopupMenu(hMenu, TPM_TOPALIGN|TPM_LEFTALIGN, pt.x, pt.y, 0, this->_hwnd, nullptr);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_onBcQryRep()
{
  if(this->_query_count) {
    this->_query_abort();
  } else {

    OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
    if(!ModChan)
      return;

    OmPNetRepoArray selection;

    // get count of selected item
    int32_t lv_nsl = this->msgItem(IDC_LV_REP, LVM_GETSELECTEDCOUNT);

    if(lv_nsl > 0) {

      // query selected repositories

      int32_t lv_sel = this->msgItem(IDC_LV_REP, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
      while(lv_sel != -1) {

        // add to selection
        selection.push_back(ModChan->getRepository(lv_sel));

        // next selected item
        lv_sel = this->msgItem(IDC_LV_REP, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
      }

    } else {

      // query all repositories

      for(size_t i = 0; i < ModChan->repositoryCount(); ++i)
        selection.push_back(ModChan->getRepository(i));
    }

    this->_query_start(selection);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_onBcNewRep()
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmModChan* ModChan = ModMan->activeChannel();

  if(ModChan) {

    OmUiAddRep* pUiNewRep = static_cast<OmUiAddRep*>(this->_UiMan->childById(IDD_ADD_REP));

    pUiNewRep->setModChan(ModChan);

    pUiNewRep->open(true);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_onBcDelRep()
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmModChan* ModChan = ModMan->activeChannel();
  if(!ModChan) return;

  int lb_sel = this->msgItem(IDC_LB_REP, LB_GETCURSEL);
  if(lb_sel < 0) return;

  OmNetRepo* NetRepo = ModChan->getRepository(lb_sel);

  OmWString repo_name;
  repo_name.assign(NetRepo->urlBase());
  repo_name.append(L" -- ");
  repo_name.append(NetRepo->urlName());

  // warns the user before committing the irreparable
  if(!Om_dlgBox_ynl(this->_hwnd, L"Delete Mod Repository", IDI_QRY, L"Delete Mod Repository",
                    L"Delete the following Mod Repository from Mod Channel ?", repo_name))
    return;

  ModChan->removeRepository(lb_sel);

  // reload the repository ListBox
  this->_lv_rep_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_bc_stop_hit()
{
  // immediate response to user input
  this->enableItem(IDC_BC_STOP, false);

  // abort selected downloads
  this->stopDownloads();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_bc_abort_hit()
{
  // immediate response to user input
  this->enableItem(IDC_BC_ABORT, false);

  // Abort all running operation
  this->_download_abort();
  this->_query_abort();
  this->_upgrade_abort();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::_onInit\n";
  #endif

  // retrieve main dialog
  this->_UiMan = static_cast<OmUiMan*>(this->root());

  // Set batches New and Delete buttons icons
  this->setBmIcon(IDC_BC_NEW, Om_getResIcon(this->_hins, IDI_BT_ADD));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDI_BT_REM));
  this->setBmIcon(IDC_BC_QRY, Om_getResIcon(this->_hins, IDI_BT_REF));

  SetWindowTheme(this->getItem(IDC_BC_NEW),L"EXPLORER",nullptr);
  SetWindowTheme(this->getItem(IDC_BC_DEL),L"EXPLORER",nullptr);
  SetWindowTheme(this->getItem(IDC_BC_QRY),L"EXPLORER",nullptr);

  // define controls tool-tips
  this->_createTooltip(IDC_LB_REP,    L"Repositories list");
  this->_createTooltip(IDC_BC_QRY,    L"Query repositories");
  this->_createTooltip(IDC_BC_NEW,    L"Add Repository");
  this->_createTooltip(IDC_BC_DEL,    L"Delete Repository");
  this->_createTooltip(IDC_LV_NET,    L"Downloadable Mod list");
  this->_createTooltip(IDC_BC_DNLD,   L"Download selected Mods");
  this->_createTooltip(IDC_BC_ABORT,  L"Abort Mod download");

  // Shared Image list for ListView controls
  LPARAM himl = reinterpret_cast<LPARAM>(this->_UiMan->listViewImgList());

  // ListView style
  DWORD lvStyle = LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_DOUBLEBUFFER;

  // Initialize Repository ListView control
  this->msgItem(IDC_LV_REP, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lvStyle);
  // set explorer theme
  SetWindowTheme(this->getItem(IDC_LV_REP),L"EXPLORER",nullptr);

  // we now add columns into our list-view control
  LVCOLUMNW lvCol = {};
  lvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
  //  "The alignment of the leftmost column is always LVCFMT_LEFT; it
  // cannot be changed." says Mr Microsoft. Do not ask why, the Microsoft's
  // mysterious ways... So, don't try to fix this.
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.pszText = const_cast<LPWSTR>(L"");
  lvCol.cx = 30;
  lvCol.iSubItem = 0;
  this->msgItem(IDC_LV_REP, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.fmt = LVCFMT_LEFT;
  lvCol.pszText = const_cast<LPWSTR>(L"Repository");
  lvCol.cx = 300;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_REP, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.fmt = LVCFMT_LEFT;
  lvCol.pszText = const_cast<LPWSTR>(L"Description");
  lvCol.cx = 300;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_REP, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  // set image list for this ListView
  this->msgItem(IDC_LV_REP, LVM_SETIMAGELIST, LVSIL_SMALL, himl);
  this->msgItem(IDC_LV_REP, LVM_SETIMAGELIST, LVSIL_NORMAL, himl);

  // Initialize Remote Packages ListView control
  this->msgItem(IDC_LV_NET, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lvStyle);
  // set explorer theme
  SetWindowTheme(this->getItem(IDC_LV_NET),L"EXPLORER",nullptr);

  // we now add columns into our list-view control
  lvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
  //  "The alignment of the leftmost column is always LVCFMT_LEFT; it
  // cannot be changed." says Mr Microsoft. Do not ask why, the Microsoft's
  // mysterious ways... So, don't try to fix this.
  lvCol.pszText = const_cast<LPWSTR>(L"");
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.cx = 30;
  lvCol.iSubItem = 0;
  this->msgItem(IDC_LV_NET, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Name");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 300;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_NET, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Version");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_NET, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Category");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 80;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_NET, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Size");
  lvCol.fmt = LVCFMT_RIGHT;
  lvCol.cx = 80;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_NET, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  lvCol.pszText = const_cast<LPWSTR>(L"Progress");
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.cx = 120;
  lvCol.iSubItem++;
  this->msgItem(IDC_LV_NET, LVM_INSERTCOLUMNW, lvCol.iSubItem, reinterpret_cast<LPARAM>(&lvCol));

  // set image list for this ListView
  this->msgItem(IDC_LV_NET, LVM_SETIMAGELIST, LVSIL_SMALL, himl);
  this->msgItem(IDC_LV_NET, LVM_SETIMAGELIST, LVSIL_NORMAL, himl);

  this->_onRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::_onShow\n";
  #endif

  // refresh dialog
  //this->_onRefresh();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_onHide()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::_onHide\n";
  #endif

  // disable "Edit > Remote" in main menu
  this->_UiMan->setPopupItem(MNU_EDIT, MNU_EDIT_NET, MF_GRAYED);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_onResize()
{
  // Repositories ListBox
  this->_setItemPos(IDC_LV_REP, 28, 0, this->cliWidth()-29, 80, true);
  this->_lv_rep_on_resize(); //< resize ListView columns adapted to client area

  // Repositories Apply, New.. and Delete buttons
  this->_setItemPos(IDC_BC_QRY, 2, 0, 22, 22, true);
  this->_setItemPos(IDC_BC_NEW, 2, 37, 22, 22, true);
  this->_setItemPos(IDC_BC_DEL, 2, 59, 22, 22, true);
  //this->_setItemPos(IDC_SC_SEPAR, 2, 35, 20, 1, true);

  // Horizontal separator
  //this->_setItemPos(IDC_SC_SEPAR, 2, 54, this->cliUnitX()-4, 1);

  // Network Mods List ListView
  this->_setItemPos(IDC_LV_NET, 2, 84, this->cliWidth()-3, this->cliHeight()-110, true);
  this->_lv_net_on_resize(); //< resize ListView columns adapted to client area

  // Download & Pause buttons
  this->_setItemPos(IDC_BC_DNLD, 2, this->cliHeight()-23, 78, 23, true);
  this->_setItemPos(IDC_BC_STOP, 81, this->cliHeight()-23, 78, 23, true);
  // Progress bar
  this->_setItemPos(IDC_PB_PKG, 161, this->cliHeight()-22, this->cliWidth()-241, 21, true);
  // Abort button
  this->_setItemPos(IDC_BC_ABORT, this->cliWidth()-78, this->cliHeight()-23, 78, 23, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::_onRefresh\n";
  #endif

  OmModHub* ModHub = static_cast<OmModMan*>(this->_data)->activeHub();
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  // disable the Progress-Bar
  this->enableItem(IDC_PB_PKG, false);

  // reload Repository ListBox
  this->_lv_rep_populate();

  // disable or enable elements depending context
  this->enableItem(IDC_SC_LBL01, (ModHub != nullptr));
  this->enableItem(IDC_LV_NET, (ModHub != nullptr));
  this->enableItem(IDC_LB_REP, (ModHub != nullptr));
  this->enableItem(IDC_BC_NEW, (ModHub != nullptr));
/*
  if(ModChan)
    ModChan->refreshNetLibrary();
*/
  // load or reload theme for ListView custom draw (progress bar)
  if(this->_lv_net_cdraw_htheme)
    CloseThemeData(this->_lv_net_cdraw_htheme);
  this->_lv_net_cdraw_htheme = OpenThemeData(this->_hwnd, L"Progress");

  this->_lv_net_populate();

  // Display error dialog AFTER ListView refreshed its content
  if(ModChan) {
    this->_UiMan->checkLibraryWrite(L"Mods Library");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::_onQuit\n";
  #endif

  this->_download_abort();
  this->_query_abort();
  this->_upgrade_abort();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiManMainNet::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_MAIN_HUB_CHANGED is a custom message sent from Main (parent) Dialog
  // to notify its child tab dialogs the Mod Hub selection changed.
  if(uMsg == UWM_MAIN_HUB_CHANGED) {
    // Refresh the dialog
    this->_onRefresh();
    return false;
  }

  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  OmModHub* ModHub = ModMan->activeHub();
  if(!ModHub)
    return false;

  if(uMsg == WM_NOTIFY) {

    OmModChan* ModChan = ModHub->activeChannel();
    if(!ModChan)
      return false;

    if(reinterpret_cast<NMHDR*>(lParam)->code == NM_CUSTOMDRAW) {

      if(reinterpret_cast<NMHDR*>(lParam)->idFrom == IDC_LV_NET) {

        NMLVCUSTOMDRAW* lvCustomDraw = reinterpret_cast<NMLVCUSTOMDRAW*>(lParam);

        switch(lvCustomDraw->nmcd.dwDrawStage)
        {
        case CDDS_PREPAINT :
          // Request to receive the CDDS_ITEMPREPAINT notification, we must use
          // SetWindowLongPtr() instead of return value because we are in DialogProc
          SetWindowLongPtr(this->_hwnd, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW); //< ie. return CDRF_NOTIFYITEMDRAW;
          return true;

        case CDDS_ITEMPREPAINT:
          // Request to receive the CDRF_NOTIFYSUBITEMDRAW notification, we must use
          // SetWindowLongPtr() instead of return value because we are in DialogProc
          SetWindowLongPtr(this->_hwnd, DWLP_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW); //< ie. return CDRF_NOTIFYSUBITEMDRAW;
          return true;

        case CDDS_SUBITEM | CDDS_ITEMPREPAINT:

          // We seek only for the 'Progress' (#5) column of the ListView
          if(lvCustomDraw->iSubItem == 5) {
            // send this to the custom draw function
            this->_lv_net_cdraw_progress(lvCustomDraw->nmcd.hdc, lvCustomDraw->nmcd.dwItemSpec, lvCustomDraw->iSubItem);
            // Prevent system to redraw the default background by returning CDRF_SKIPDEFAULT
            // We must use SetWindowLongPtr() instead of value because we are in DialogProc
            SetWindowLongPtr(this->_hwnd, DWLP_MSGRESULT, CDRF_SKIPDEFAULT); //< ie. return CDRF_SKIPDEFAULT;
            return true;
          }
        }
      }
    }

    // if repositories query is running we block all interaction
    if(this->_query_count)
      return false;

    if(LOWORD(wParam) == IDC_LV_REP) {

      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case NM_DBLCLK:
        this->_lv_rep_on_dblclk();
        break;

      case LVN_ITEMCHANGED: {
          NMLISTVIEW* nmLv = reinterpret_cast<NMLISTVIEW*>(lParam);
          // detect only selection changes
          if((nmLv->uNewState ^ nmLv->uOldState) & LVIS_SELECTED)
            this->_lv_rep_on_selchg();
          break;
        }
      }
    }

    if(LOWORD(wParam) == IDC_LV_NET) {

      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case LVN_ITEMCHANGED: {
          NMLISTVIEW* nmLv = reinterpret_cast<NMLISTVIEW*>(lParam);
          // detect only selection changes
          if((nmLv->uNewState ^ nmLv->uOldState) & LVIS_SELECTED)
            this->_lv_net_on_selchg();
          break;
        }

      case NM_DBLCLK:
        this->_lv_net_on_dblclk();
        break;

      case NM_RCLICK:
        this->_lv_net_on_rclick();
        break;

      case LVN_COLUMNCLICK:
        switch(reinterpret_cast<NMLISTVIEW*>(lParam)->iSubItem)
        {
        case 0:
          ModChan->setNetLibrarySort(OM_SORT_STAT);
          break;
        case 2:
          ModChan->setNetLibrarySort(OM_SORT_VERS);
          break;
        case 3:
          ModChan->setNetLibrarySort(OM_SORT_CATE);
          break;
        case 4:
          ModChan->setNetLibrarySort(OM_SORT_SIZE);
          break;
        case 5:
          return false; // ignore action
        default:
          ModChan->setNetLibrarySort(OM_SORT_NAME);
          break;
        }
        this->_lv_net_populate(); //< rebuild ListView
        break;
      }
    }

  }

  if(uMsg == WM_COMMAND) {

    // Prevent command/shorcut execution when main dialog is not active
    if(!this->_UiMan->active())
      return false;

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiManMainNet::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

    switch(LOWORD(wParam))
    {
    case IDC_BC_QRY: //< Repository "Refresh" button
      this->_onBcQryRep();
      break;

    case IDC_BC_NEW: //< Repository "Add" button
      this->_onBcNewRep();
      break;

    case IDC_BC_DEL: //< Repository "Delete" button
      this->_onBcDelRep();
      break;

    case IDC_BC_DNLD: //< Main "Upgrade" button
      this->queueDownloads(true);
      break;

    case IDC_BC_STOP: //< Main "Stop" button
      this->_bc_stop_hit();
      break;

    case IDC_BC_ABORT: //< Main "Abort all" button
      this->_bc_abort_hit();
      break;

    // Menu : Edit > Remote > []
    case IDM_EDIT_NET_DNWS:
      this->queueDownloads(false);
      break;

    case IDM_EDIT_NET_DNLD:
      this->queueDownloads(true);
      break;

    case IDM_EDIT_NET_STOP:
      this->stopDownloads();
      break;

    case IDM_EDIT_NET_RVOK:
      this->revokeDownloads();
      break;

    case IDM_EDIT_NET_FIXD:
      this->downloadDepends(false);
      break;

    case IDM_EDIT_NET_INFO:
      this->showProperties();
      break;
    }
  }

  return false;
}
