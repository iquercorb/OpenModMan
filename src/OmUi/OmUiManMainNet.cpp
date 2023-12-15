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
//#include "OmUiAddChn.h"
//#include "OmUiAddRep.h"
#include "OmUiWizRep.h"
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
  _download_upgrd(false),
  _upgrade_abort(false),
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
void OmUiManMainNet::addRepository()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // not during query, please
  if(ModChan->queriesQueueSize())
    return;

  OmUiWizRep* UiWizRep = static_cast<OmUiWizRep*>(this->root()->childById(IDD_WIZ_REP));

  UiWizRep->setModChan(ModChan);

  UiWizRep->open(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::queryRepositories()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  if(ModChan->queriesQueueSize()) {
    this->_query_abort();
    return;
  }

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

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::deleteRepository()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

   // not during query, please
  if(ModChan->queriesQueueSize())
    return;

  int lv_sel = this->msgItem(IDC_LV_REP, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0) return;

  OmNetRepo* NetRepo = ModChan->getRepository(lv_sel);

  OmWString repo_ref = NetRepo->base();
  if(!NetRepo->name().empty()) {
    repo_ref += L" -- "; repo_ref += NetRepo->name();
  }

  // warns the user before committing the irreparable
  if(!Om_dlgBox_ynl(this->_hwnd, L"Delete Mod Repository", IDI_QRY, L"Delete Mod Repository",
                    L"Delete the following Mod Repository from Mod Channel ?", repo_ref))
    return;

  ModChan->removeRepository(lv_sel);

  // reload the repository ListBox
  this->_lv_rep_populate();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::startDownloads(bool upgrade)
{
  // prevent useless processing
  if(!this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT))
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkLibraryWrite(L"Install Mods"))
    return;

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

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // checks for proper access on all required directories
  if(!this->_UiMan->checkLibraryWrite(L"Install Mods"))
    return;

  // get single selection
  int32_t lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0) return;

  OmNetPack* NetPack = ModChan->getNetpack(lv_sel);

  // this should never happen be we handle it
  if(!NetPack->hasLocal())
    return;

  // here we go like a full download
  OmPNetPackArray depends;  //< dependency download list
  OmWStringArray missings;    //< missing dependencies lists

  // Get remote package depdencies
  ModChan->getDepends(NetPack, &depends, &missings);

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

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // 1. get current selected Net Pack in ListView
  int lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  while(lv_sel != -1) {

    OmNetPack* NetPack = ModChan->getNetpack(lv_sel);

    NetPack->revokeDownload();

    // update ListView item
    this->_lv_net_alterate(OM_NOTIFY_ALTERED, NetPack->hash());

    // next selected item
    lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::showProperties() const
{
  // prevent useless processing
  if(this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT) != 1)
    return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0) return;

  OmNetPack* NetPack = ModChan->getNetpack(lv_sel);
  if(!NetPack) return;

  OmUiPropNet* UiPropRmt = static_cast<OmUiPropNet*>(this->_UiMan->childById(IDD_PROP_RMT));

  UiPropRmt->setNetPack(NetPack);

  UiPropRmt->open(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_netlib_notify_fn(void* ptr, OmNotify notify, uint64_t param)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::_netlib_notify_fn\n";
  #endif

  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  if(notify == OM_NOTIFY_REBUILD)
    self->_lv_net_populate();

  if(notify == OM_NOTIFY_CREATED || notify == OM_NOTIFY_DELETED || notify == OM_NOTIFY_ALTERED)
    self->_lv_net_alterate(notify, param);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_refresh_processing()
{
  OmModMan* ModMan = reinterpret_cast<OmModMan*>(this->_data);

  bool is_safe = true;

  // search in all opened Hubs for all channel
  for(size_t h = 0; h < ModMan->hubCount(); ++h) {

    OmModHub* ModHub = ModMan->getHub(h);

    for(size_t c = 0; c < ModHub->channelCount(); ++c) {

      bool has_queue = false;

      OmModChan* ModChan = ModHub->getChannel(c);

      if(ModChan->queriesQueueSize() || ModChan->downloadQueueSize() || ModChan->upgradesQueueSize()) {
        has_queue = true; is_safe = false;
      }

      // if this is current active channel set controls
      if(ModChan == ModHub->activeChannel()) {

        // enable 'kill-switch" abort button
        this->enableItem(IDC_BC_ABORT, has_queue);

        // reset progress bar, then enable or disable
        this->enableItem(IDC_PB_MOD, has_queue);
        if(has_queue) {
          this->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

          uint32_t progress;

          if(ModChan->upgradesQueueSize())
            progress = ModChan->upgradesProgress();

          if(ModChan->queriesQueueSize()) {
            progress = ModChan->queriesProgress();
            this->setBmIcon(IDC_BC_RPQRY, Om_getResIcon(IDI_BT_NOT));
          }

          if(ModChan->downloadQueueSize())
            progress = ModChan->downloadsProgress();

          this->msgItem(IDC_PB_MOD, PBM_SETPOS, progress + 1); //< this prevent transition
          this->msgItem(IDC_PB_MOD, PBM_SETPOS, progress);

        } else {
          this->msgItem(IDC_PB_MOD, PBM_SETPOS, 0);
          this->setBmIcon(IDC_BC_RPQRY, Om_getResIcon(IDI_BT_REF));
        }
      }
    }
  }

  // set dialog safe to quit
  this->_setSafe(is_safe);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_abort_processing()
{
  // this function is the "kill switch" for all pending queues and threads
  // related to this dialog, this will make the dialog safe to quit

  OmModMan* ModMan = reinterpret_cast<OmModMan*>(this->_data);

  for(size_t h = 0; h < ModMan->hubCount(); ++h) {

    OmModHub* ModHub = ModMan->getHub(h);

    for(size_t c = 0; c < ModHub->channelCount(); ++c) {

      OmModChan* ModChan = ModHub->getChannel(c);

      ModChan->abortQueries();
      ModChan->abortUpgrades();
      ModChan->stopDownloads();
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_query_abort()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // cancel queries
  ModChan->abortQueries();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_query_start(const OmPNetRepoArray& selection)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // change button image from refresh to stop
  this->setBmIcon(IDC_BC_RPQRY, Om_getResIcon(IDI_BT_NOT));

  // change status icon
  LVITEMW lvI = {};
  lvI.mask = LVIF_IMAGE; lvI.iSubItem = 0; lvI.iImage = ICON_STS_QUE;
  for(size_t i = 0; i < selection.size(); ++i) {
    lvI.iItem = ModChan->indexOfRepository(selection[i]);
    this->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  // send query for this Mod Repo
  ModChan->queueQueries(selection,
                        OmUiManMainNet::_query_begin_fn,
                        OmUiManMainNet::_query_result_fn,
                        OmUiManMainNet::_query_ended_fn,
                        this);

  // enter processing
  this->_refresh_processing();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_query_begin_fn(void* ptr, uint64_t param)
{
  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetRepo* NetRepo = reinterpret_cast<OmNetRepo*>(param);

  OmModChan* ModChan = NetRepo->ModChan();

  // check whether dialog is showing the proper Channel
  if(ModChan != static_cast<OmModMan*>(self->_data)->activeChannel())
    return;

  // update repository ListView item
  LVITEMW lvI = {}; lvI.iItem = ModChan->indexOfRepository(NetRepo);
  // change status image
  lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE; lvI.iImage = ICON_STS_WIP;
  self->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_query_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OM_UNUSED(result);

  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetRepo* NetRepo = reinterpret_cast<OmNetRepo*>(param);

  OmModChan* ModChan = NetRepo->ModChan();

  // check whether dialog is showing the proper Channel
  if(ModChan != static_cast<OmModMan*>(self->_data)->activeChannel())
    return;

  // update repository ListView item
  LVITEMW lvI = {}; lvI.iItem = ModChan->indexOfRepository(NetRepo);
  // change status image
  lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE;
  lvI.iImage = self->_lv_rep_get_status_icon(NetRepo);
  self->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  // set or update title
  lvI.iSubItem = 2; lvI.mask = LVIF_TEXT;
  lvI.pszText = const_cast<LPWSTR>(NetRepo->title().c_str());
  self->msgItem(IDC_LV_REP, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

  // update progression bar
  self->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
  self->msgItem(IDC_PB_MOD, PBM_SETPOS, ModChan->queriesProgress());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_query_ended_fn(void* ptr, OmNotify notify, uint64_t param)
{
  OM_UNUSED(notify); OM_UNUSED(param);

  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  // change button image from stop to refresh
  self->setBmIcon(IDC_BC_RPQRY, Om_getResIcon(IDI_BT_REF));

  // leaving processing
  self->_refresh_processing();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_download_abort()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  ModChan->stopDownloads();
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

  // update selected Net Pack ListView items
  LVITEMW lvI = {};
  // change status icon
  lvI.mask = LVIF_IMAGE; lvI.iSubItem = 0; lvI.iImage = ICON_STS_DNL;
  for(size_t i = 0; i < selection.size(); ++i) {
    lvI.iItem = ModChan->indexOfNetpack(selection[i]);
    this->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  // Enable 'Stop' and disable 'Download'
  if(this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT)) {
    this->enableItem(IDC_BC_STOP, true);
    this->enableItem(IDC_BC_DNLD, false);
  }

  // start or append downloads
  ModChan->startDownloads(selection,
                          OmUiManMainNet::_download_download_fn,
                          OmUiManMainNet::_download_result_fn,
                          OmUiManMainNet::_download_ended_fn,
                          this);

  // enter processing
  this->_refresh_processing();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiManMainNet::_download_download_fn(void* ptr, int64_t tot, int64_t cur, int64_t rate, uint64_t param)
{
  OM_UNUSED(tot); OM_UNUSED(cur); OM_UNUSED(rate);

  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  OmModChan* ModChan = NetPack->ModChan();

  // check whether dialog is showing the proper Channel
  if(ModChan != static_cast<OmModMan*>(self->_data)->activeChannel())
    return true; //< wrong Channel, do not abort but ignore

  // get ListView item index search using lparam, that is, Net Pack hash value.
  int32_t item_id = self->findLvParam(IDC_LV_NET, NetPack->hash());

  // Invalidate ListView subitem rect to call custom draw (progress bar)
  RECT rect = {};
  self->getLvSubRect(IDC_LV_NET, item_id, 5, &rect);
  self->redrawItem(IDC_LV_NET, &rect, RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);

  // update global progression
  self->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
  self->msgItem(IDC_PB_MOD, PBM_SETPOS, ModChan->downloadsProgress());

  return true; //< continue
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_download_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OM_UNUSED(result);

  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmModMan* ModMan = static_cast<OmModMan*>(self->_data);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  OmModChan* ModChan = NetPack->ModChan();

  // check whether dialog is showing the proper Channel
  if(ModChan == ModMan->activeChannel()) {

    // get ListView item index search using lparam, that is, Net Pack hash value.
    int32_t item_id = self->findLvParam(IDC_LV_NET, NetPack->hash());

    // Set 'Sotp' and 'Download" buttons if item currently selected
    if(self->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT) == 1) {
      if(self->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED) == item_id) {
        self->enableItem(IDC_BC_STOP, false);
        self->enableItem(IDC_BC_DNLD, !NetPack->hasLocal());
      }
    }

    // update ListView item
    LVITEMW lvI = {}; lvI.iItem = item_id;
    // change status icon
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE; lvI.iImage = self->_lv_net_get_status_icon(NetPack);
    self->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Invalidate ListView subitem rect to call custom draw (progress bar)
    RECT rect;
    self->getLvSubRect(IDC_LV_MOD, item_id, 5 /* 'Progress' column */, &rect);
    self->redrawItem(IDC_LV_MOD, &rect, RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);
  }

  // if an error occurred, display error dialog
  if(NetPack->hasError()) {

    if(ModChan != ModMan->activeChannel()) {
      // switch to proper Hub and Channel to show error
      OmModHub* ModHub = ModChan->ModHub();
      int32_t hub_id = ModMan->indexOfHub(ModHub);
      self->_UiMan->selectHub(hub_id);
      int32_t chn_id = ModHub->indexOfChannel(ModChan);
      self->_UiMan->selectChannel(chn_id);
    }

    Om_dlgBox_okl(self->_hwnd, L"Download Mods", IDI_MOD_ERR,
                L"Mod download error", L"The download of \""
                +NetPack->iden()+L"\" failed:", NetPack->lastError());
  }

  // if we are downloading for upgrade, here we go
  if(self->_download_upgrd) {
    if(NetPack->upgradableCount() && NetPack->hasLocal())
      self->_upgrade_start(NetPack->ModChan(), OmPNetPackArray(1, NetPack));
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_download_ended_fn(void* ptr, OmNotify notify, uint64_t param)
{
  OM_UNUSED(notify); OM_UNUSED(param);

  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  // leaving processing
  self->_refresh_processing();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_upgrade_start(OmModChan* ModChan, const OmPNetPackArray& selection)
{
  // since upgrade may be queued while active channel is different than
  // when download start, we use the passed pointer instead of current active one

  // reset abort status
  this->_upgrade_abort = false;

  ModChan->queueUpgrades(selection,
                         OmUiManMainNet::_upgrade_begin_fn,
                         OmUiManMainNet::_upgrade_progress_fn,
                         OmUiManMainNet::_upgrade_result_fn,
                         OmUiManMainNet::_upgrade_ended_fn,
                         this);

  // enter processing
  this->_refresh_processing();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_upgrade_begin_fn(void* ptr, uint64_t param)
{
  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  // get ListView item index search using lparam, that is, Net Pack hash value.
  int32_t item_id = self->findLvParam(IDC_LV_NET, NetPack->hash());
  if(item_id < 0) return;

  // update ListView item
  LVITEMW lvI = {}; lvI.iItem = item_id;
  // change status icon
  lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE; lvI.iImage = ICON_STS_WIP;
  self->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiManMainNet::_upgrade_progress_fn(void* ptr, size_t cur, size_t tot, uint64_t param)
{
  OM_UNUSED(cur); OM_UNUSED(tot);

  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  OmModChan* ModChan = NetPack->ModChan();

  // check whether dialog is showing the proper Channel
  if(ModChan != static_cast<OmModMan*>(self->_data)->activeChannel())
    return true; //< wrong Channel, do not abort but ignore

  // get ListView item index search using lparam, that is, Net Pack hash value.
  int32_t item_id = self->findLvParam(IDC_LV_NET, NetPack->hash());

  // Invalidate ListView subitem rect to call custom draw (progress bar)
  RECT rect;
  self->getLvSubRect(IDC_LV_NET, item_id, 5 /* 'Progress' column */, &rect);
  self->redrawItem(IDC_LV_NET, &rect, RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);

  // update global progression, but prevent interfering with current downloading
  if(!ModChan->downloadQueueSize()) {
    self->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    self->msgItem(IDC_PB_MOD, PBM_SETPOS, ModChan->upgradesProgress());
  }

  return !self->_upgrade_abort;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_upgrade_result_fn(void* ptr, OmResult result, uint64_t param)
{
  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  OmModMan* ModMan = static_cast<OmModMan*>(self->_data);

  OmNetPack* NetPack = reinterpret_cast<OmNetPack*>(param);

  OmModChan* ModChan = NetPack->ModChan();

  // check whether dialog is showing the proper Channel
  if(ModChan == ModMan->activeChannel()) {

    // get ListView item index search using lparam, that is, Net Pack hash value.
    int32_t item_id = self->findLvParam(IDC_LV_NET, NetPack->hash());

    // update ListView item
    LVITEMW lvI = {}; lvI.iItem = item_id;
    // change status icon
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE; lvI.iImage = self->_lv_net_get_status_icon(NetPack);
    self->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Invalidate ListView subitem rect to call custom draw (progress bar)
    RECT rect;
    self->getLvSubRect(IDC_LV_MOD, item_id, 5 /* 'Progress' column */, &rect);
    self->redrawItem(IDC_LV_MOD, &rect, RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);

  }

  if(result == OM_RESULT_ERROR) {

    if(ModChan != ModMan->activeChannel()) {
      // switch to proper Hub and Channel to show error
      OmModHub* ModHub = ModChan->ModHub();
      int32_t hub_id = ModMan->indexOfHub(ModHub);
      self->_UiMan->selectHub(hub_id);
      int32_t chn_id = ModHub->indexOfChannel(ModChan);
      self->_UiMan->selectChannel(chn_id);
    }

    Om_dlgBox_okl(self->_hwnd, L"Upgrade Mods", IDI_MOD_ERR,
                L"Mod upgrade error", L"The upgrading of \""
                +NetPack->core()+L"\" failed:", NetPack->lastError());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_upgrade_ended_fn(void* ptr, OmNotify notify, uint64_t param)
{
  OM_UNUSED(notify); OM_UNUSED(param);

  OmUiManMainNet* self = static_cast<OmUiManMainNet*>(ptr);

  // leaving processing
  self->_refresh_processing();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_rep_populate()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::_lv_rep_populate\n";
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
    this->enableItem(IDC_BC_RPQRY, false);
    // update Repositories ListView selection
    this->_lv_rep_on_selchg();
    // return now
    return;
  }

  OmWString lv_entry;
  LVITEMW lvI = {};

  for(size_t i = 0; i < ModChan->repositoryCount(); ++i) {

    OmNetRepo* NetRepo = ModChan->getRepository(i);

    lvI.iItem = static_cast<int>(i);

    // the first column, repository status, here we INSERT the new item
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE;
    lvI.iImage = this->_lv_rep_get_status_icon(NetRepo);
    this->msgItem(IDC_LV_REP, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    // Second column, the repository address, now empty, here we set the sub-item
    lvI.iSubItem = 1; lvI.mask = LVIF_TEXT|LVIF_IMAGE;
    lvI.iImage = ICON_REP;

    lv_entry = NetRepo->base();
    if(!NetRepo->name().empty()) {
      lv_entry += L" -- "; lv_entry += NetRepo->name();
    }

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

  // enable or disable query button and menu
  bool can_query = (ModChan->repositoryCount() > 0);
  this->enableItem(IDC_BC_RPQRY, can_query);
  this->_UiMan->setPopupItem(MNU_CHN, MNU_CHN_QRYREP, can_query ? MF_ENABLED : MF_GRAYED);

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
  uint32_t lv_nsl = this->msgItem(IDC_LV_REP, LVM_GETSELECTEDCOUNT);

  this->enableItem(IDC_BC_RPDEL, lv_nsl > 0);

  //HMENU hMnuRep = this->_UiMan->getPopupItem(MNU_EDIT, MNU_EDIT_REP);
  //this->_UiMan->setPopupItem(hMnuRep, MNU_EDIT_REP_DEL, lv_nsl ? MF_ENABLED : MF_GRAYED);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_rep_on_dblclk()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // prevent useless processing
  if(this->msgItem(IDC_LV_REP, LVM_GETSELECTEDCOUNT) != 1)
    return;

  // Get ListView unique selection
  int lv_sel = this->msgItem(IDC_LV_REP, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  if(lv_sel < 0) return;

  OmPNetRepoArray selection;

  selection.push_back(ModChan->getRepository(lv_sel));

  this->_query_start(selection);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_rep_on_rclick()
{
  // get Popup submenu from hidden context menu
  HMENU hPopup = this->_UiMan->getContextPopup(POP_REP);
  if(!hPopup) return;

  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // set 'Query' menu item text according current querying state
  this->setPopupItemText(hPopup, POP_REP_QRY, ModChan->queriesQueueSize() ? L"Abort query" : L"Query");

  // enable or disable menu-items according selection and lock mode
  uint32_t item_stat = this->msgItem(IDC_LV_REP, LVM_GETSELECTEDCOUNT) ? MF_ENABLED : MF_GRAYED;
  this->setPopupItem(hPopup, POP_REP_DEL, this->_UiMan->lockMode() ? MF_GRAYED : item_stat);

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  // display popup menu along mouse cursor
  TrackPopupMenu(hPopup, TPM_LEFTALIGN|TPM_RIGHTBUTTON,  pt.x, pt.y, 0, this->_hwnd, nullptr);
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
  std::cout << "DEBUG => OmUiManMainNet::_lv_net_populate\n";
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

  LVITEMW lvI = {};

  // add item to list view
  for(size_t i = 0; i < ModChan->netpackCount(); ++i) {

    OmNetPack* NetPack = ModChan->getNetpack(i);

    lvI.iItem = static_cast<int>(i);

    // the first column, mod status, here we INSERT the new item
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE|LVIF_PARAM; //< icon and special data
    lvI.iImage = this->_lv_net_get_status_icon(NetPack);
    // notice for later : to work properly the lParam must be defined on the first SubItem (iSubItem = 0)
    lvI.lParam = static_cast<LPARAM>(NetPack->hash());
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
    lvI.pszText = const_cast<LPWSTR>(NetPack->fileSizeStr().c_str());
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
void OmUiManMainNet::_lv_net_alterate(OmNotify action, uint64_t param)
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  // get ListView item index search using lparam, that is, Mod Pack hash value.
  int32_t item_id = this->findLvParam(IDC_LV_NET, param);

  if(action == OM_NOTIFY_DELETED) {

    if(item_id < 0)
      return;

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiManMainNet::_lv_net_alterate : DELETE\n";
    #endif

    // delete item
    this->msgItem(IDC_LV_NET, LVM_DELETEITEM, item_id, 0);

  } else {

    // update item data
    OmNetPack* NetPack = ModChan->findNetpack(param);
    if(!NetPack) return; //< what ?!

    LVITEMW lvI = {};

    if(action == OM_NOTIFY_CREATED) {

      #ifdef DEBUG
      std::cout << "DEBUG => OmUiManMainNet::_lv_net_alterate : CREATE\n";
      #endif

      // the first column, mod status, here we INSERT the new item
      lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE|LVIF_PARAM; //< icon and special data
      lvI.iImage = this->_lv_net_get_status_icon(NetPack);
      // notice for later : to work properly the lParam must be defined on the first SubItem (iSubItem = 0)
      lvI.lParam = static_cast<LPARAM>(NetPack->hash());
      lvI.iItem = this->msgItem(IDC_LV_NET, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));

    } else {

      if(item_id < 0)
        return;

      #ifdef DEBUG
      std::cout << "DEBUG => OmUiManMainNet::_lv_net_alterate : ALTER\n";
      #endif

      lvI.iItem = item_id;
      // update status icon
      lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE;
      lvI.iImage = this->_lv_net_get_status_icon(NetPack);
      this->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
    }

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
    lvI.pszText = const_cast<LPWSTR>(NetPack->fileSizeStr().c_str());
    this->msgItem(IDC_LV_NET, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }
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
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(!ModChan) return;

  OmNetPack* NetPack = ModChan->getNetpack(item);
  if(!NetPack) return;

  if(!NetPack->isDownloading() && !NetPack->isUpgrading())
    return;

  // get rectangle of subitem to draw
  RECT rect, bar_rect, txt_rect;
  this->getLvSubRect(IDC_LV_NET, item, subitem, &rect);

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
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  // handle to "Edit > Repository []
  //HMENU hPopup = this->_UiMan->getPopupItem(MNU_EDIT, MNU_EDIT_NET);

  // get count of ListView selected item
  uint32_t lv_nsl = this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT);

  // enable or disable menu-items according current state and selection
  if((lv_nsl < 1) || (ModChan == nullptr)) {

    this->enableItem(IDC_BC_DNLD, false);
    this->enableItem(IDC_BC_STOP, false);

    // disable all menu items
    /*
    for(uint32_t i = 0; i < 12; ++i)
      this->setPopupItem(hPopup, i, MF_GRAYED);
    */

    // show nothing in footer frame
    this->_UiMan->pUiMgrFoot()->clearItem();

  } else {

    bool can_down = false;
    bool can_upgd = false;
    bool can_stop = false;
    bool can_rvok = false;
    bool can_fixd = false;

    OmNetPack* NetPack = nullptr;

    // scan selection to check what can be done
    int32_t lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
    while(lv_sel != -1) {

      NetPack = ModChan->getNetpack(lv_sel);

      if(NetPack->hasLocal()) {
        if(NetPack->hasMissingDepend()) can_fixd = true;
      } else {
        if(!NetPack->isDownloading()) {
          can_down = true;
          if(NetPack->upgradableCount())  can_upgd = true;
          if(NetPack->isResumable())      can_rvok = true;
        } else {
          can_stop = true;
        }
      }

      // next selected item
      lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
    }

    // if single selection show mod pack overview
    if(lv_nsl == 1) {
      this->_UiMan->pUiMgrFoot()->selectItem(NetPack);
    } else {
      this->_UiMan->pUiMgrFoot()->clearItem();
    }

    OmWString down_text(can_rvok ? L"Resume" : L"Download");

    this->setItemText(IDC_BC_DNLD, down_text);
    this->enableItem(IDC_BC_DNLD, can_down);
    this->enableItem(IDC_BC_STOP, can_stop);
    /*
    this->setPopupItemText(hPopup, POP_NET_DNLD, down_text);
    this->setPopupItem(hPopup, MNU_NET_DNLD, can_down ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, MNU_NET_DNWS, can_upgd ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, MNU_NET_STOP, can_stop ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, MNU_NET_RVOK, can_rvok ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, MNU_NET_FIXD, can_fixd ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, MNU_NET_INFO, (lv_nsl == 1)?MF_ENABLED:MF_GRAYED);
    */
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
    this->startDownloads(true);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_lv_net_on_rclick()
{
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();

  // get sub-menu from hidden context menu
  HMENU hPopup = this->_UiMan->getContextPopup(POP_NET);

  // get count of ListView selected item
  uint32_t lv_nsl = this->msgItem(IDC_LV_NET, LVM_GETSELECTEDCOUNT);

  // enable or disable menu-items according current state and selection
  if((lv_nsl < 1) || (ModChan == nullptr)) {

    // disable all menu items
    for(uint32_t i = 0; i < 12; ++i)
      this->setPopupItem(hPopup, i, MF_GRAYED);

  } else {

    bool can_down = false;
    bool can_upgd = false;
    bool can_stop = false;
    bool can_rvok = false;
    bool can_fixd = false;

    // scan selection to check what can be done
    int32_t lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
    while(lv_sel != -1) {

      OmNetPack* NetPack = ModChan->getNetpack(lv_sel);

      if(NetPack->hasLocal()) {
        if(NetPack->hasMissingDepend()) can_fixd = true;
      } else {
        if(!NetPack->isDownloading()) {
          can_down = true;
          if(NetPack->upgradableCount())  can_upgd = true;
          if(NetPack->isResumable())      can_rvok = true;
        } else {
          can_stop = true;
        }
      }

      // next selected item
      lv_sel = this->msgItem(IDC_LV_NET, LVM_GETNEXTITEM, lv_sel, LVNI_SELECTED);
    }

    this->setPopupItemText(hPopup, POP_NET_DNLD, can_rvok ? L"Resume" : L"Download");
    this->setPopupItem(hPopup, POP_NET_DNLD, can_down ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, POP_NET_DNWS, can_upgd ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, POP_NET_STOP, can_stop ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, POP_NET_RVOK, can_rvok ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, POP_NET_FIXD, can_fixd ? MF_ENABLED:MF_GRAYED);
    this->setPopupItem(hPopup, POP_NET_INFO, (lv_nsl == 1)?MF_ENABLED:MF_GRAYED);
  }

  // get mouse cursor position
  POINT pt;
  GetCursorPos(&pt);

  // display popup menu along mouse cursor
  TrackPopupMenu(hPopup, TPM_LEFTALIGN|TPM_RIGHTBUTTON,  pt.x, pt.y, 0, this->_hwnd, nullptr);
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
    } else if(NetPack->downgradableCount()) {
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
void OmUiManMainNet::_bc_stop_clicked()
{
  // immediate response to user input
  this->enableItem(IDC_BC_STOP, false);

  // abort selected downloads
  this->stopDownloads();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_bc_abort_clicked()
{
  // immediate response to user input
  this->enableItem(IDC_BC_ABORT, false);

  // Abort all running operation
  this->_download_abort();
  this->_query_abort();
  this->_upgrade_abort = true;
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
  this->setBmIcon(IDC_BC_RPADD, Om_getResIcon(IDI_BT_ADD));
  this->setBmIcon(IDC_BC_RPDEL, Om_getResIcon(IDI_BT_REM));
  this->setBmIcon(IDC_BC_RPQRY, Om_getResIcon(IDI_BT_REF));

  SetWindowTheme(this->getItem(IDC_BC_RPADD),L"EXPLORER",nullptr);
  SetWindowTheme(this->getItem(IDC_BC_RPDEL),L"EXPLORER",nullptr);
  SetWindowTheme(this->getItem(IDC_BC_RPQRY),L"EXPLORER",nullptr);

  // define controls tool-tips
  this->_createTooltip(IDC_LV_REP,    L"Repositories list");
  this->_createTooltip(IDC_BC_RPQRY,  L"Query repositories");
  this->_createTooltip(IDC_BC_RPADD,  L"Add Repository");
  this->_createTooltip(IDC_BC_RPDEL,  L"Delete Repository");
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

  // "subscribe" to active channel library directory changes notifications
  static_cast<OmModMan*>(this->_data)->notifyNetLibraryStart(OmUiManMainNet::_netlib_notify_fn, this);

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
  this->_refresh_processing();
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
  //this->_UiMan->setPopupItem(MNU_EDIT, MNU_EDIT_REP, MF_GRAYED);
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
  this->_setItemPos(IDC_BC_RPQRY, 2, 0, 22, 22, true);
  this->_setItemPos(IDC_SC_SEPAR, 3, 29, 19, 1, true);
  this->_setItemPos(IDC_BC_RPADD, 2, 37, 22, 22, true);
  this->_setItemPos(IDC_BC_RPDEL, 2, 59, 22, 22, true);

  // Network Mods List ListView
  this->_setItemPos(IDC_LV_NET, 2, 84, this->cliWidth()-3, this->cliHeight()-110, true);
  this->_lv_net_on_resize(); //< resize ListView columns adapted to client area

  // Download & Pause buttons
  this->_setItemPos(IDC_BC_DNLD, 2, this->cliHeight()-23, 78, 23, true);
  this->_setItemPos(IDC_BC_STOP, 81, this->cliHeight()-23, 78, 23, true);
  // Progress bar
  this->_setItemPos(IDC_PB_MOD, 161, this->cliHeight()-22, this->cliWidth()-241, 21, true);
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
  this->enableItem(IDC_PB_MOD, false);

  // disable or enable elements depending context
  this->enableItem(IDC_LV_REP, (ModHub != nullptr));
  this->enableItem(IDC_BC_RPADD, (ModHub != nullptr));
  this->enableItem(IDC_LV_NET, (ModHub != nullptr));

  this->_lv_rep_populate();

  // load or reload theme for ListView custom draw (progress bar)
  if(this->_lv_net_cdraw_htheme)
    CloseThemeData(this->_lv_net_cdraw_htheme);
  this->_lv_net_cdraw_htheme = OpenThemeData(this->_hwnd, L"Progress");

  this->_lv_net_populate();

  // Display error dialog AFTER ListView refreshed its content
  if(ModChan) {
    this->_UiMan->checkLibraryWrite(L"Mods Library");
  }

  this->_refresh_processing();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManMainNet::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManMainNet::_onQuit\n";
  #endif

  // this is the 'kill-switch' for all pending queues related
  // to Network Library dialog, making it safe to quit
  this->_abort_processing();
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

  if(uMsg == WM_NOTIFY) {

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

    if(LOWORD(wParam) == IDC_LV_REP) {

      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case LVN_ITEMCHANGED: {
          NMLISTVIEW* nmLv = reinterpret_cast<NMLISTVIEW*>(lParam);
          // detect only selection changes
          if((nmLv->uNewState ^ nmLv->uOldState) & LVIS_SELECTED)
            this->_lv_rep_on_selchg();
          break;
        }

      case NM_DBLCLK:
        this->_lv_rep_on_dblclk();
        break;

      case NM_RCLICK:
        this->_lv_rep_on_rclick();
        break;
      }
    }

    if(LOWORD(wParam) == IDC_LV_NET) {

      OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
      if(!ModChan) return false;

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
        break;
      }
    }

  }

  if(uMsg == WM_COMMAND) {

    // Prevent command/shorcut execution when main dialog is not active
    if(!this->_UiMan->active())
      return false;

    #ifdef DEBUG
    //std::cout << "DEBUG => OmUiManMainNet::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif

    switch(LOWORD(wParam))
    {
    case IDC_BC_RPQRY: //< Repository "Query" button
      if(HIWORD(wParam) == BN_CLICKED)
        this->queryRepositories();
      break;

    case IDC_BC_RPADD: //< Repository "Add" button
      if(HIWORD(wParam) == BN_CLICKED)
        this->addRepository();
      break;

    case IDC_BC_RPDEL: //< Repository "Delete" button
      if(HIWORD(wParam) == BN_CLICKED)
        this->deleteRepository();
      break;

    case IDC_BC_DNLD: //< Main "Upgrade" button
      if(HIWORD(wParam) == BN_CLICKED)
        this->startDownloads(true);
      break;

    case IDC_BC_STOP: //< Main "Stop" button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_bc_stop_clicked();
      break;

    case IDC_BC_ABORT: //< Main "Abort all" button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_bc_abort_clicked();
      break;

    // Menu : Edit > Repository > []
    case IDM_REP_QRY:
      this->queryRepositories();
      break;

    case IDM_REP_ADD:
      this->addRepository();
      break;

    case IDM_REP_DEL:
      this->deleteRepository();
      break;

    // Menu : Edit > Remote > []
    case IDM_NET_DNWS:
      this->startDownloads(false);
      break;

    case IDM_NET_DNLD:
      this->startDownloads(true);
      break;

    case IDM_NET_STOP:
      this->stopDownloads();
      break;

    case IDM_NET_RVOK:
      this->revokeDownloads();
      break;

    case IDM_NET_FIXD:
      this->downloadDepends(false);
      break;

    case IDM_NET_INFO:
      this->showProperties();
      break;
    }
  }

  return false;
}
