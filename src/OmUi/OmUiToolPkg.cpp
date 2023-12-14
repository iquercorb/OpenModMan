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
#include "OmBaseApp.h"

#include "OmArchive.h"

#include "OmModMan.h"
#include "OmModChan.h"
#include "OmModPack.h"

#include "OmUtilWin.h"
#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilPkg.h"
#include "OmUtilAlg.h"

//#include <algorithm>            //< std::replace

/*
#include "OmBaseWin.h"
#include <ShlObj.h>

#include "OmBaseUi.h"

#include "OmBaseApp.h"

#include "OmArchive.h"

#include "OmModMan.h"
#include "OmModChan.h"
#include "OmImage.h"

#include "OmUtilFs.h"



*/

#include "OmUiMan.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiToolPkg.h"



/// \brief Mod categories list
///
/// List of predefined Mod categories
///
static const wchar_t __categ_list[][16] = {
  L"Generic",
  L"Texture",
  L"Skin",
  L"Model",
  L"Level",
  L"Mission",
  L"UI",
  L"Audio",
  L"Feature",
  L"Plugin",
  L"Script",
  L"Patch",
};

static const size_t __categ_count = 12;

/// \brief Custom "Package Save Done" Message
///
/// Custom "Package Save Done" window message to notify the dialog that the
/// running thread finished his job.
///
#define UWM_PKGSAVE_DONE    (WM_APP+1)

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolPkg::OmUiToolPkg(HINSTANCE hins) : OmDialog(hins),
  _ModPack(new OmModPack(nullptr)),
  _has_unsaved(false),
  _method_cache(-1),
  _modpack_save_abort(0),
  _modpack_save_hth(nullptr),
  _modpack_save_hwo(nullptr),
  _modpack_save_hdp(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolPkg::~OmUiToolPkg()
{
  if(this->_ModPack)
    delete this->_ModPack;

  HBITMAP hBm = this->setStImage(IDC_SB_SNAP, nullptr);
  if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);

  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiToolPkg::id() const
{
  return IDD_TOOL_PKG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_status_update_filename()
{
  OmWString file_path;

  // set definition file path to status bar
  if(!this->_ModPack->sourcePath().empty() && !this->_ModPack->sourceIsDir()) {
    file_path = this->_ModPack->sourcePath();
  } else {
    file_path = L"<unsaved package>";
  }

  this->setItemText(IDC_SC_FILE, file_path);

  OmWString caption = Om_getFilePart(file_path);
  caption += L" - Mod-Package editor";
  this->setCaption(caption);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_set_unsaved(bool enable)
{
  this->_has_unsaved = enable;

  bool file_exists = (!this->_ModPack->sourcePath().empty() && !this->_ModPack->sourceIsDir());

  // enable/disable and change tool bar 'save' button image
  TBBUTTONINFOA tbBi = {}; tbBi.cbSize = sizeof(TBBUTTONINFOA);
  tbBi.dwMask = TBIF_STATE;
  tbBi.fsState = (this->_has_unsaved && file_exists) ? TBSTATE_ENABLED : 0;
  this->msgItem(IDC_TB_TOOLS, TB_SETBUTTONINFO, IDC_BC_SAVE, reinterpret_cast<LPARAM>(&tbBi));

  // enable/disable and change menu 'save' item
  if(this->_has_unsaved && file_exists) {
    this->setPopupItem(MNU_ME_FILE, MNU_ME_FILE_SAVE, MF_ENABLED);
  } else {
    this->setPopupItem(MNU_ME_FILE, MNU_ME_FILE_SAVE, MF_GRAYED);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmUiToolPkg::_ask_unsaved()
{
  // Check and ask for unsaved changes
  if(this->_has_unsaved)
    return Om_dlgBox_ync(this->_hwnd, L"Mod-Package editor", IDI_QRY, L"Unsaved changes", L"Do you want to save changes before closing ?");

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_has_changes()
{
  // check for compression method difference
  if(this->_ModPack->hasSource()) {
    int32_t cb_sel = this->msgItem(IDC_CB_ZMD, CB_GETCURSEL);
    if(this->_method_cache != this->msgItem(IDC_CB_ZMD, CB_GETITEMDATA, cb_sel))
      return true;
  }

  // check for category difference
  if(this->_ModPack->category().empty()) {
    if(this->msgItem(IDC_CB_CAT, CB_GETCURSEL) != 0)
      return true;
  } else {
    if(!Om_namesMatches(this->_ModPack->category(), this->_categ_cache))
       return true;
  }

  // check for description difference
  if(this->_ModPack->description() != this->_desc_cache)
    return true;

  // check for dependencies differences
  if(this->_ModPack->dependCount() != this->_depend_cache.size()) {

    return true;

  } else {

    for(size_t i = 0; i < this->_ModPack->dependCount(); ++i)
      if(Om_arrayContain(this->_depend_cache, this->_ModPack->getDependIden(i)))
        return true;
  }

  // finally check for thumbnail difference (potentially the most costly)
  if(this->_ModPack->thumbnail() != this->_thumb_cache)
    return true;

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_reset_controls()
{
  // empty controls
  this->setItemText(IDC_EC_INP01, L"");
  this->enableItem(IDC_EC_INP01,  false);

  this->setItemText(IDC_EC_INP02, L"");
  this->enableItem(IDC_EC_INP02,  false);

  this->msgItem(IDC_CB_EXT, CB_SETCURSEL, 0);
  this->enableItem(IDC_CB_EXT,  false);

  this->setItemText(IDC_EC_RESUL, L"");
  this->enableItem(IDC_EC_RESUL,  false);

  //this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 4); //< FIXME : keep or reset ?
  //this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 2);
  this->enableItem(IDC_CB_ZMD,  false);
  this->enableItem(IDC_CB_ZLV,  false);

  this->enableItem(IDC_EC_READ1,  false);
  this->setItemText(IDC_EC_READ1, L"");

  //this->msgItem(IDC_CB_CAT, CB_SETCURSEL, 0);  //< FIXME : keep or reset ?
  //this->setItemText(IDC_EC_INP07, L"");  //< FIXME : keep or reset ?

  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CKBX1,  false);
  this->enableItem(IDC_BC_BRW03,  false);
  // set thumbnail placeholder
  HBITMAP hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_BLANK));
  if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);
  this->setPopupItem(MNU_ME_EDIT, MNU_ME_EDIT_THMBSEL, MF_GRAYED);

  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CKBX2,  false);
  this->enableItem(IDC_BC_BRW04,  false);
  this->setItemText(IDC_EC_DESC, L"");
  this->enableItem(IDC_EC_DESC,  false);
  this->setPopupItem(MNU_ME_EDIT, MNU_ME_EDIT_DESCSEL, MF_GRAYED);

  this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CKBX3,  false);
  this->enableItem(IDC_BC_DPADD,  false);
  this->enableItem(IDC_BC_DPBRW,  false);
  this->enableItem(IDC_BC_DPDEL,  false);
  this->msgItem(IDC_LB_DPN, LB_RESETCONTENT); //< empty ListBox
  this->enableItem(IDC_LB_DPN, false);
  this->setItemText(IDC_EC_INP08, L"");
  this->enableItem(IDC_EC_INP08,  false);
  this->setPopupItem(MNU_ME_EDIT, MNU_ME_EDIT_DEPIMP, MF_GRAYED);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_check_zip_method()
{
  int32_t cb_sel = this->msgItem(IDC_CB_ZMD, CB_GETCURSEL);
  int32_t method = this->msgItem(IDC_CB_ZMD, CB_GETITEMDATA, cb_sel);

  cb_sel = this->msgItem(IDC_CB_EXT, CB_GETCURSEL);
  OmWString cb_entry;
  this->getCbText(IDC_CB_EXT, cb_sel, cb_entry);

  if(Om_namesMatches(cb_entry, L".zip") && method != OM_METHOD_DEFLATE) {

    Om_dlgBox_ok(this->_hwnd, L"Mod-package editor", IDI_WRN, L"Non-standard Zip compression",
                 L"The selected compression method is not widely supported for Zip files, for "
                 "maximum compatibility prefer the \"Defalte\" method.");
  }

  // check for changes
  if(this->_ModPack->hasSource())
    this->_set_unsaved(this->_has_changes());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiToolPkg::_modpack_save_run_fn(void* ptr)
{
  OmUiToolPkg* self = static_cast<OmUiToolPkg*>(ptr);

  // set cached data to local instance
  self->_ModPack->setCategory(self->_categ_cache);
  self->_ModPack->setThumbnail(self->_thumb_cache);
  self->_ModPack->setDescription(self->_desc_cache);
  self->_ModPack->clearDepend();
  for(size_t i = 0; i < self->_depend_cache.size(); ++i)
    self->_ModPack->addDependIden(self->_depend_cache[i]);

  //get compression method and level
  int32_t method = self->msgItem(IDC_CB_ZMD, CB_GETITEMDATA, self->msgItem(IDC_CB_ZMD, CB_GETCURSEL));
  int32_t level = self->msgItem(IDC_CB_ZLV, CB_GETITEMDATA, self->msgItem(IDC_CB_ZLV, CB_GETCURSEL));

  // Open progress dialog
  self->_modpack_save_abort = 0;
  self->_modpack_save_hdp = Om_dlgProgress(self->_hwnd, L"Save Mod-Package", IDI_PKG_ADD, L"Saving Mod-Package", &self->_modpack_save_abort, OM_DLGBOX_DUAL_BARS);

  // and here we go for saving
  OmResult result = self->_ModPack->saveAs(self->_modpack_save_path, method, level,
                                          OmUiToolPkg::_modpack_save_progress_fn,
                                          OmUiToolPkg::_modpack_save_compress_fn, self);

  // quit the progress dialog (dialogs must be opened and closed within the same thread)
  Om_dlgProgressClose(static_cast<HWND>(self->_modpack_save_hdp));
  self->_modpack_save_hdp = nullptr;

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_modpack_save_progress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmUiToolPkg* self = static_cast<OmUiToolPkg*>(ptr);

  // update progress bar
  Om_dlgProgressUpdate(static_cast<HWND>(self->_modpack_save_hdp), tot, cur, nullptr, 1);

  return (self->_modpack_save_abort != 1);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_modpack_save_compress_fn(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OmUiToolPkg* self = static_cast<OmUiToolPkg*>(ptr);

  // update progress text
  OmWString progress_text = L"Compressing file: ";
  progress_text += Om_getFilePart(reinterpret_cast<wchar_t*>(param));
  Om_dlgProgressUpdate(static_cast<HWND>(self->_modpack_save_hdp), tot, cur, progress_text.c_str(), 0);

  return (self->_modpack_save_abort != 1);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmUiToolPkg::_modpack_save_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  OmUiToolPkg* self = static_cast<OmUiToolPkg*>(ptr);

  OmResult result = static_cast<OmResult>(Om_threadExitCode(self->_modpack_save_hth));
  Om_clearThread(self->_modpack_save_hth, self->_modpack_save_hwo);

  self->_modpack_save_hth = nullptr;
  self->_modpack_save_hwo = nullptr;

  if(result == OM_RESULT_OK) {

    // parse the new created package
    self->_modpack_parse(self->_modpack_save_path);

  } else {

    if(result == OM_RESULT_ERROR) {
      Om_dlgBox_okl(self->_hwnd, L"Mod-package editor", IDI_WRN, L"Mod-Package save error",
                   L"Mod-Package save failed:", self->_ModPack->lastError());
    }

    // re-parse previous package
    OmWString previous_path = self->_ModPack->sourcePath();
    self->_modpack_parse(previous_path);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_modpack_save_as()
{
  if(this->_modpack_save_hth)
    return;

  OmWString dlg_start, dlg_result;

  // if available, select current active channel library as start location
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) dlg_start = ModChan->libraryPath();

  // get default filename
  OmWString filename;
  this->getItemText(IDC_EC_RESUL, filename);

  OmWString extension;
  this->getCbText(IDC_CB_EXT, this->msgItem(IDC_CB_EXT, CB_GETCURSEL), extension);
  // remove the leading dot
  extension.erase(0, 1);

  // send save dialog to user
  if(!Om_dlgSaveFile(dlg_result, this->_hwnd, L"Save Mod-Package", OM_PKG_FILES_FILTER, extension.c_str(), filename.c_str(), dlg_start))
    return;

/*
  standard save dialog (IFileSaveDialog) already ask for overwrite

  // ask user for overwirte
  if(!Om_dlgOverwriteFile(this->_hwnd, dlg_result))
    return;
*/

  // start the "save" thread
  this->_modpack_save_path = dlg_result;

  this->_modpack_save_hth = Om_createThread(OmUiToolPkg::_modpack_save_run_fn, this);
  this->_modpack_save_hwo = Om_waitForThread(this->_modpack_save_hth, OmUiToolPkg::_modpack_save_end_fn, this);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_modpack_save()
{
  if(this->_modpack_save_hth)
    return;

  if(this->_ModPack->sourcePath().empty() || this->_ModPack->sourceIsDir()) {
    this->_modpack_save_as();
    return;
  }

  // start the "save" thread
  this->_modpack_save_path = this->_ModPack->sourcePath();

  this->_modpack_save_hth = Om_createThread(OmUiToolPkg::_modpack_save_run_fn, this);
  this->_modpack_save_hwo = Om_waitForThread(this->_modpack_save_hth, OmUiToolPkg::_modpack_save_end_fn, this);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_modpack_parse(const OmWString& path)
{
  this->_reset_controls();

  if(!this->_ModPack->parseSource(path)) {
    this->_reset_controls();
    return false;
  }

  // update status
  this->_status_update_filename();

  // enable controls
  this->enableItem(IDC_EC_INP01,  true);
  this->enableItem(IDC_EC_INP02,  true);
  this->enableItem(IDC_CB_EXT,    true);
  this->enableItem(IDC_EC_RESUL,  true);

  this->enableItem(IDC_CB_ZMD,    true);
  this->enableItem(IDC_CB_ZLV,    true);

  this->enableItem(IDC_EC_READ1,  true);

  this->enableItem(IDC_CB_CAT,    true);
  this->enableItem(IDC_BC_CKBX1,  true);
  this->enableItem(IDC_BC_CKBX2,  true);
  this->enableItem(IDC_BC_CKBX3,  true);

  this->setPopupItem(MNU_ME_EDIT, MNU_ME_EDIT_THMBSEL, MF_ENABLED);
  this->setPopupItem(MNU_ME_EDIT, MNU_ME_EDIT_DESCSEL, MF_ENABLED);
  this->setPopupItem(MNU_ME_EDIT, MNU_ME_EDIT_DEPIMP, MF_ENABLED);

  // reset all local copy of parameters
  this->_method_cache = -1;
  this->_categ_cache.clear();
  this->_thumb_cache.clear();
  this->_desc_cache.clear();
  this->_depend_cache.clear();

  // parse file name
  OmWString iden, core, vers, name;

  if(this->_ModPack->sourceIsDir()) {
    iden = Om_getFilePart(path);
  } else {
    iden = Om_getNamePart(path);
  }

  Om_parseModIdent(iden, &core, &vers, &name);

  this->setItemText(IDC_EC_INP01, name);
  this->setItemText(IDC_EC_INP02, vers);

  OmWString ext = Om_getFileExtPart(path);
  if(Om_namesMatches(ext, L"zip")) {
    this->msgItem(IDC_CB_EXT, CB_SETCURSEL, 1);
  } else {
    this->msgItem(IDC_CB_EXT, CB_SETCURSEL, 0);
  }

  // force filename preview to the current file
  this->setItemText(IDC_EC_RESUL, Om_getFilePart(path));

  // try to get a compression method
  this->_method_cache = this->_ModPack->getSourceCompMethod();

  if(this->_method_cache < 0) {
    this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 4);
  } else {
    uint32_t cb_count = this->msgItem(IDC_CB_ZMD, CB_GETCOUNT);
    for(uint32_t i = 0; i < cb_count; ++i) {
      if(this->msgItem(IDC_CB_ZMD, CB_GETITEMDATA, i) == this->_method_cache) {
        this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, i);
      }
    }
  }

  // set pack content
  if(!this->_ModPack->sourceEntryCount()) {
    this->setItemText(IDC_EC_READ1, L"<empty package>");
  } else {
    OmWString ec_content;

    size_t n = this->_ModPack->sourceEntryCount();
    for(size_t i = 0; i < n; ++i) {
      ec_content += this->_ModPack->getSourceEntry(i).path;
      if(i < n - 1) ec_content += L"\r\n";
    }

    this->setItemText(IDC_EC_READ1, ec_content);
  }

  // set category
  int32_t categ_id = -1;

  if(!this->_ModPack->category().empty()) {
    for(size_t i = 0; i < __categ_count; ++i) {
      if(Om_namesMatches(__categ_list[i], this->_ModPack->category())) {
        categ_id = i; break;
      }
    }
  } else {
    categ_id = 0; //< "Generic" is default category
  }

  if(categ_id >= 0) {
    this->enableItem(IDC_EC_INP07, false);
    this->msgItem(IDC_CB_CAT, CB_SETCURSEL, categ_id);
    this->getCbText(IDC_CB_CAT, categ_id, this->_categ_cache);
  } else {
    // copy to local cache
    this->_categ_cache = this->_ModPack->category();
    // set controls
    int32_t cb_last = this->msgItem(IDC_CB_CAT, CB_GETCOUNT) - 1;
    this->msgItem(IDC_CB_CAT, CB_SETCURSEL, cb_last);
    this->enableItem(IDC_EC_INP07, true);
    this->setItemText(IDC_EC_INP07, this->_ModPack->category());
  }

  // set thumbnail
  if(!this->_ModPack->thumbnail().valid()) {
    this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 0);
    //this->_thumb_toggle();
  } else {
    // copy to local cache
    this->_thumb_cache = this->_ModPack->thumbnail();
    // set controls
    this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 1);
    this->enableItem(IDC_BC_BRW03, true);
    HBITMAP hBm = this->setStImage(IDC_SB_SNAP, this->_thumb_cache.hbmp());
    if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);
  }

  // set description
  if(this->_ModPack->description().empty()) {
    this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 0);
    //this->_desc_toggle();
  } else {
    // copy to local cache
    this->_desc_cache = this->_ModPack->description();
    // set controls
    this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 1);
    this->enableItem(IDC_BC_BRW04, true);
    this->enableItem(IDC_EC_DESC, true);
    this->setItemText(IDC_EC_DESC, this->_desc_cache);
  }

  // set dependencies
  if(this->_ModPack->dependCount() == 0) {
    this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, 0);
    //this->_depend_toggle();
  } else {
    // copy to local cache
    for(size_t i = 0; i < this->_ModPack->dependCount(); ++i)
      this->_depend_cache.push_back(this->_ModPack->getDependIden(i));
    // set controls
    this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, 1);
    this->enableItem(IDC_LB_DPN, true);
    this->_depend_populate();
  }

  // nothing to save
  this->_set_unsaved(false);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_modpack_open()
{
  // if available, select current active channel library as start location
  OmWString start;
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) start = ModChan->libraryPath();

  // new dialog to open file (allow multiple selection)
  OmWString result;
  if(!Om_dlgOpenFile(result, this->_hwnd, L"Open Mod-Package(s)", OM_PKG_FILES_FILTER, start))
    return;

  // run add list thread
  if(!this->_modpack_parse(result)) {
    Om_dlgBox_okl(this->_hwnd, L"Mod-package editor", IDI_WRN, L"Mod-package parse error",
                 L"The following file parse failed, it is either corrupted or not a valid Mod-package",
                 result);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_modpack_build()
{
  // if available, select current active channel library as start location
  OmWString start;
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) start = ModChan->libraryPath();

  // new dialog to open file (allow multiple selection)
  OmWString result;
  if(!Om_dlgOpenDir(result, this->_hwnd, L"Open Mod directory", start))
    return;

  // run add list thread
  if(!this->_modpack_parse(result)) {
    Om_dlgBox_okl(this->_hwnd, L"Mod-package editor", IDI_WRN, L"Mod directory parse error",
                 L"The following directory parse failed, and this should never happen, so...",
                 result);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_name_compose()
{
  OmWString filename, name, vers, ext;

  this->getItemText(IDC_EC_INP01, name);

  if(name.empty())
    return;

  filename = Om_spacesToUnderscores(name);

  this->getItemText(IDC_EC_INP02, vers);

  OmVersion version(vers);
  if(!version.isNull()) {
    filename += L"_v";
    filename += version.asString();
  }

  int32_t cb_sel = this->msgItem(IDC_CB_EXT, CB_GETCURSEL);
  this->getCbText(IDC_CB_EXT, cb_sel, ext);

  filename += ext;

  this->setItemText(IDC_EC_RESUL, filename);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_categ_select()
{
  int32_t cb_sel = this->msgItem(IDC_CB_CAT, CB_GETCURSEL);

  // check whether user selected the last item (GENERIC)
  if(cb_sel == this->msgItem(IDC_CB_CAT, CB_GETCOUNT) - 1) {

    this->enableItem(IDC_EC_INP07, true);
    this->setItemText(IDC_EC_INP07, this->_categ_cache);

  } else {

    this->enableItem(IDC_EC_INP07, false);
    this->setItemText(IDC_EC_INP07, L"");

    // write changes to current Package
    this->getCbText(IDC_CB_CAT, cb_sel, this->_categ_cache);
  }

  // check for changes
  this->_set_unsaved(this->_has_changes());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_categ_changed()
{
  // we keep a local copy of the last edited text to restore it
  if(IsWindowEnabled(this->getItem(IDC_EC_INP07))) {

    this->getItemText(IDC_EC_INP07, this->_categ_cache);

    // check for changes
    this->_set_unsaved(this->_has_changes());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_thumb_toggle()
{
  bool is_enabled = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_BC_DPADD, is_enabled);
  this->enableItem(IDC_BC_BRW03, is_enabled);
  this->enableItem(IDC_SB_SNAP, is_enabled);

  HBITMAP hBm = nullptr;

  // clear local thumbnail
  this->_thumb_cache.clear();

  if(is_enabled) {

    // set thumbnail to current Mod Pack thumbnail
    if(this->_ModPack->thumbnail().valid()) {
      this->_thumb_cache = this->_ModPack->thumbnail();
      hBm = this->setStImage(IDC_SB_SNAP, this->_thumb_cache.hbmp());
    } else {
      hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_BLANK));
    }

  } else {

    // set thumbnail placeholder to static control
    hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_BLANK));

  }

  // check for changes
  this->_set_unsaved(this->_has_changes());

  if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_thumb_load()
{
  OmWString open_start, open_result;

  // if available, select current active channel library as start location
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) open_start = ModChan->libraryPath();

  // new dialog to open file
  if(!Om_dlgOpenFile(open_result, this->_hwnd, L"Open image file", OM_IMG_FILES_FILTER, open_start))
    return;

  // try to load image file
  if(this->_thumb_cache.loadThumbnail(open_result, OM_MODPACK_THUMB_SIZE, OM_SIZE_FILL)) {

    // set image to static control
    HBITMAP hBm = this->setStImage(IDC_SB_SNAP, this->_thumb_cache.hbmp());
    if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);
  }

  // check for changes
  this->_set_unsaved(this->_has_changes());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_desc_toggle()
{
  bool is_enabled = this->msgItem(IDC_BC_CKBX2, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW04, is_enabled);
  this->enableItem(IDC_EC_DESC, is_enabled);

  // reset description
  this->_desc_cache.clear();

  if(is_enabled) {
    if(!this->_ModPack->description().empty())
      this->_desc_cache = this->_ModPack->description();
  }

  // check for changes
  this->_set_unsaved(this->_has_changes());

  this->setItemText(IDC_EC_DESC, this->_desc_cache);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_desc_load()
{
  OmWString open_start, open_result;

  // if available, select current active channel library as start location
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) open_start = ModChan->libraryPath();

  // new dialog to open file
  if(!Om_dlgOpenFile(open_result, this->_hwnd, L"Open text file", OM_TXT_FILES_FILTER, open_start))
    return;

  // we keep local copy of loaded text
  this->_desc_cache = Om_toCRLF(Om_toUTF16(Om_loadPlainText(open_result)));

  // assign new description
  this->setItemText(IDC_EC_DESC, this->_desc_cache);

  // check for changes
  this->_set_unsaved(this->_has_changes());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_desc_changed()
{
  // we keep a local copy of the last edited text to restore it
  if(IsWindowEnabled(this->getItem(IDC_EC_DESC))) {

    this->getItemText(IDC_EC_DESC, this->_desc_cache);

    // check for changes
    this->_set_unsaved(this->_has_changes());
  }
}
///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_depend_populate()
{
  this->msgItem(IDC_LB_DPN, LB_RESETCONTENT); //< empty ListBox

  for(size_t i = 0; i < this->_depend_cache.size(); ++i)
    this->msgItem(IDC_LB_DPN, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(this->_depend_cache[i].c_str()));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_depend_toggle()
{
  bool is_enabled = this->msgItem(IDC_BC_CKBX3, BM_GETCHECK);

  this->enableItem(IDC_BC_DPADD, is_enabled);
  this->enableItem(IDC_BC_DPBRW, is_enabled);
  this->enableItem(IDC_LB_DPN, is_enabled);

  this->_depend_cache.clear();

  if(is_enabled) {
    if(this->_ModPack->dependCount()) {
      for(size_t i = 0; i < this->_ModPack->dependCount(); ++i)
        this->_depend_cache.push_back(this->_ModPack->getDependIden(i));
    }
  }

  // rebuild depend list
  this->_depend_populate();

  // check for changes
  this->_set_unsaved(this->_has_changes());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_depend_sel_changed()
{
  // get ListBox current selection
  int32_t lb_sel = this->msgItem(IDC_LB_DPN, LB_GETCURSEL);

  this->enableItem(IDC_BC_DPDEL, (lb_sel >= 0));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_depend_delete()
{
  // get ListBox current selection
  int32_t lb_sel = this->msgItem(IDC_LB_DPN, LB_GETCURSEL);
  if(lb_sel < 0) return;

  this->msgItem(IDC_LB_DPN, LB_DELETESTRING, lb_sel);
  this->_depend_cache.erase(this->_depend_cache.begin() + lb_sel);

  this->_depend_sel_changed();

  // check for changes
  this->_set_unsaved(this->_has_changes());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_depend_browse()
{
  // if available, select current active channel library as start location
  OmWString start;
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) start = ModChan->libraryPath();

  // new dialog to open file (allow multiple selection)
  OmWStringArray result;
  if(!Om_dlgOpenFileMultiple(result, this->_hwnd, L"Open Mod-Package(s)", OM_PKG_FILES_FILTER, start))
    return;

  OmWString identity;

  for(size_t i = 0; i < result.size(); ++i) {

    identity = Om_getNamePart(result[i]);

    this->msgItem(IDC_LB_DPN, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(identity.c_str()));
    this->_depend_cache.push_back(identity);
  }

  this->_depend_sel_changed();

  // check for changes
  this->_set_unsaved(this->_has_changes());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_depend_add_show(bool show)
{
  // Show/Hide the "Add Dependency" controls
  this->showItem(IDC_SC_LBL08, show);
  this->showItem(IDC_EC_INP08, show);
  this->showItem(IDC_BC_DPVAL, show);
  this->showItem(IDC_BC_DPABT, show);

  // Show/Hide regular Dependencies controls
  this->showItem(IDC_BC_DPADD, !show);
  this->showItem(IDC_BC_DPDEL, !show);
  this->showItem(IDC_BC_DPBRW, !show);
  this->showItem(IDC_LB_DPN,   !show);

  this->_depend_sel_changed();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_depend_add_valid()
{
  this->enableItem(IDC_BC_DPVAL, false);

  OmWString ec_content;
  this->getItemText(IDC_EC_INP08, ec_content);
  this->setItemText(IDC_EC_INP08, L"");

  this->msgItem(IDC_LB_DPN, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ec_content.c_str()));
  this->_depend_cache.push_back(ec_content);

  this->_depend_add_show(false);

  // check for changes
  this->_set_unsaved(this->_has_changes());
}

/*
///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::selectSource(const OmWString& path)
{
  bool is_dir = Om_isDir(path);

  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, is_dir);
  this->msgItem(IDC_BC_RAD02, BM_SETCHECK, !is_dir);

  if(is_dir) {
    this->_onBcBrwDir(path.c_str());
  } else {
    this->_onBcBrwPkg(path.c_str());
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_freeze(bool freeze)
{
  bool enable = !freeze;

  // disable all controls during creation
  this->enableItem(IDC_BC_RAD01, enable);
  this->enableItem(IDC_BC_RAD02, enable);
  this->enableItem(IDC_EC_INP03, enable);
  this->enableItem(IDC_EC_INP04, enable);
  this->enableItem(IDC_CB_EXT, enable);
  this->enableItem(IDC_CB_ZMD, enable);
  this->enableItem(IDC_CB_ZLV, enable);
  this->enableItem(IDC_EC_INP05, enable);
  this->enableItem(IDC_BC_BRW03, enable);
  this->enableItem(IDC_BC_CKBX1, enable);
  this->enableItem(IDC_BC_CKBX2, enable);
  this->enableItem(IDC_BC_CKBX3, enable);
  // disable according radios and check-boxes status
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->enableItem(IDC_EC_INP01, enable);
    this->enableItem(IDC_BC_BRW01, enable);
  } else {
    this->enableItem(IDC_EC_INP02, enable);
    this->enableItem(IDC_BC_BRW02, enable);
  }
  if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
    this->enableItem(IDC_EC_INP07, enable);
    if(enable) {
      OmWString ident;
      this->getItemText(IDC_EC_INP07, ident);
      this->enableItem(IDC_BC_ADD, !ident.empty());
    } else {
      this->enableItem(IDC_BC_ADD, false);
    }
    this->enableItem(IDC_BC_DEL, enable);
    this->enableItem(IDC_LB_DPN, enable);
  }
  if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
    this->enableItem(IDC_BC_BRW04, enable);
  }
  if(this->msgItem(IDC_BC_CKBX3, BM_GETCHECK)) {
    this->enableItem(IDC_BC_BRW05, enable);
    this->enableItem(IDC_EC_DESC, enable);
  }
  this->enableItem(IDC_BC_SAVE, enable);
  this->enableItem(IDC_BC_CLOSE, enable);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_parseSrc(const OmWString& path)
{
  HBITMAP hBm;

  // reset current package
  this->_ModPack.clearAll();

  // name and extension controls initial state
  this->setItemText(IDC_EC_INP03, L"");
  this->enableItem(IDC_EC_INP03, false);
  this->setItemText(IDC_EC_INP04, L"");
  this->enableItem(IDC_EC_INP04, false);
  this->enableItem(IDC_CB_EXT, false);
  this->enableItem(IDC_CB_ZMD, false);
  this->enableItem(IDC_CB_ZLV, false);
  this->setItemText(IDC_EC_READ1, L"");

  // destination folder disabled
  this->enableItem(IDC_EC_INP06, false);
  this->enableItem(IDC_BC_BRW03, false);

  // disable the save button
  this->enableItem(IDC_BC_SAVE, false);

  // category initial state
  this->msgItem(IDC_CB_CAT, CB_SETCURSEL, 0);
  this->enableItem(IDC_CB_CAT, false);
  this->setItemText(IDC_EC_INP09, L"");
  this->enableItem(IDC_EC_INP09, false);

  // Dependencies initial state
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CKBX1, false);
  this->enableItem(IDC_SC_LBL06, false); //< "Ident" label
  this->msgItem(IDC_LB_DPN, LB_RESETCONTENT);
  this->enableItem(IDC_LB_DPN, false);
  this->setItemText(IDC_EC_INP07, L"");
  this->enableItem(IDC_EC_INP07, false);
  this->enableItem(IDC_BC_ADD, false);
  this->enableItem(IDC_BC_DEL, false);

  // Snapshot initial states
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CKBX2, false);
  this->enableItem(IDC_BC_BRW04, false);
  hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_BLANK));
  if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);
  this->setItemText(IDC_EC_INP08, L"");

  // Description initial states
  this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, 0);
  this->enableItem(IDC_BC_CKBX3, false);
  this->enableItem(IDC_BC_BRW05, false);
  this->enableItem(IDC_EC_DESC, false);
  this->setItemText(IDC_EC_DESC, L"");

  // check whether source path is empty to
  // reset to initial state
  if(path.empty()) {
    this->_unsaved = false; //< reset unsaved changes
    return true;
  }

  // Try to parse the package
  if(!this->_ModPack.parseSource(path)) {
    return false;
  }

  // enable controls for package edition
  this->enableItem(IDC_EC_INP03, true);
  this->enableItem(IDC_EC_INP04, true);
  this->enableItem(IDC_CB_EXT, true);
  this->enableItem(IDC_CB_ZMD, true);
  this->enableItem(IDC_CB_ZLV, true);

  this->enableItem(IDC_EC_INP06, true);
  this->enableItem(IDC_BC_BRW03, true);

  this->enableItem(IDC_BC_CKBX1, true);
  this->enableItem(IDC_BC_CKBX2, true);
  this->enableItem(IDC_BC_CKBX3, true);

  this->enableItem(IDC_CB_CAT, true);

  // set source path to input EditText depending selected Radio
  this->enableItem(IDC_EC_INP01, true);
  this->enableItem(IDC_EC_INP02, true);
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->setItemText(IDC_EC_INP01, path);
    this->setItemText(IDC_EC_INP02, L"");
    this->enableItem(IDC_EC_INP02, false);
  } else {
    this->setItemText(IDC_EC_INP02, path);
    this->setItemText(IDC_EC_INP01, L"");
    this->enableItem(IDC_EC_INP01, false);
  }

  // check for package category
  if(!this->_ModPack.category().empty()) {
    int cb_idx = this->msgItem(IDC_CB_CAT, CB_FINDSTRING, -1, reinterpret_cast<LPARAM>(this->_ModPack.category().c_str()));
    if(cb_idx >= 0) {
      this->msgItem(IDC_CB_CAT, CB_SETCURSEL, cb_idx);
      this->setItemText(IDC_EC_INP09, L"");
      this->enableItem(IDC_EC_INP09, false);
    } else {
      this->msgItem(IDC_CB_CAT, CB_SETCURSEL, OmModCategoryCount);
      this->enableItem(IDC_EC_INP09, true);
      this->setItemText(IDC_EC_INP09, this->_ModPack.category());
    }
  } else {
    this->msgItem(IDC_CB_CAT, CB_SETCURSEL, 0);
    this->setItemText(IDC_EC_INP09, L"");
    this->enableItem(IDC_EC_INP09, false);
  }

  // check for package dependencies
  if(this->_ModPack.dependCount()) {
    this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 1);
    this->enableItem(IDC_SC_LBL06, true); //< "Ident" label
    this->enableItem(IDC_LB_DPN, true);
    this->enableItem(IDC_EC_INP07, true);
    for(unsigned i = 0; i < this->_ModPack.dependCount(); ++i) {
      this->msgItem(IDC_LB_DPN, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(this->_ModPack.getDependIden(i).c_str()));
    }
  }

  // check for package snapshot
  if(this->_ModPack.thumbnail().valid()) {
    this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 1);
    this->enableItem(IDC_BC_BRW04, true);
    hBm = this->setStImage(IDC_SB_SNAP, this->_ModPack.thumbnail().hbmp());
    if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);
  }

  // check for package description
  if(this->_ModPack.description().size()) {
    this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, 1);
    this->enableItem(IDC_BC_BRW05, true);
    this->enableItem(IDC_EC_DESC, true);
    this->setItemText(IDC_EC_DESC, this->_ModPack.description());
  }

  OmWString item_str, dst_path;

  // Add package content to output EditText
  unsigned n = this->_ModPack.sourceEntryCount();
  for(unsigned i = 0; i < this->_ModPack.sourceEntryCount(); ++i) {
    item_str.append(this->_ModPack.getSourceEntry(i).path);
    if(i < n - 1) item_str.append(L"\r\n");
  }
  this->setItemText(IDC_EC_READ1, item_str);

  // update name and version
  this->setItemText(IDC_EC_INP03, this->_ModPack.name());

  if(!this->_ModPack.version().isNull())
    this->setItemText(IDC_EC_INP04, this->_ModPack.version().asString());

  // get current source path
  item_str.clear();
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP01, item_str);
  } else {
    this->getItemText(IDC_EC_INP02, item_str);
  }

  if(!item_str.empty()) {

    // preselect file extention
    if(Om_extensionMatches(item_str, OM_PKG_FILE_EXT)) {
      this->msgItem(IDC_CB_EXT, CB_SETCURSEL, 1);
    } else {
      this->msgItem(IDC_CB_EXT, CB_SETCURSEL, 0);
    }

    // refresh final filename
    this->_onNameChange();

    // prefill destination path if empty
    if(this->getItemText(IDC_EC_INP06, dst_path) == 0) {

      dst_path = Om_getDirPart(item_str);

      if(Om_isDir(dst_path))
        this->setItemText(IDC_EC_INP06, dst_path);
    }
  }

  this->_unsaved = false; //< reset unsaved changes

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_save_init()
{
  // disable all dialog's controls
  this->_freeze(true);

  // enable progress bar and abort button
  this->enableItem(IDC_PB_MOD, true);
  this->enableItem(IDC_BC_ABORT, true);

  // start package building thread
  DWORD dWid;
  this->_save_hth = CreateThread(nullptr, 0, this->_save_fth, this, 0, &dWid);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_save_stop()
{
  DWORD exitCode;

  if(this->_save_hth) {
    WaitForSingleObject(this->_save_hth, INFINITE);
    GetExitCodeThread(this->_save_hth, &exitCode);
    CloseHandle(this->_save_hth);
    this->_save_hth = nullptr;
  }

  // enable all dialog's controls
  this->_freeze(false);

  // reset & disable progress bar & abort button
  this->msgItem(IDC_PB_MOD, PBM_SETPOS, 0, 0);
  this->enableItem(IDC_PB_MOD, false);
  this->enableItem(IDC_BC_ABORT, false);

  // show a reassuring dialog message
  if(exitCode == 0) {

    // get destination filename
    OmWString item_str;
    this->getItemText(IDC_EC_RESUL, item_str);

    // a reassuring message
    Om_dlgSaveSucces(this->_hwnd, L"Mod Pack Editor", L"Save Mod pack", L"Mod pack");
  }

  // refresh the main window dialog, this will also refresh this one
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_save_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t param)
{
  OM_UNUSED(param);

  OmUiToolPkg* self = reinterpret_cast<OmUiToolPkg*>(ptr);

  self->msgItem(IDC_PB_MOD, PBM_SETRANGE, 0, MAKELPARAM(0, tot));
  self->msgItem(IDC_PB_MOD, PBM_SETPOS, cur);

  return !self->_save_abort;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiToolPkg::_save_fth(void* arg)
{
  OmUiToolPkg* self = static_cast<OmUiToolPkg*>(arg);

  OmWString item_str;

  // get package category
  int32_t cb_sel = self->msgItem(IDC_CB_CAT, CB_GETCURSEL);
  if(cb_sel == self->msgItem(IDC_CB_CAT, CB_GETCOUNT)-1) {
    // get category from text field
    self->getItemText(IDC_EC_INP09, item_str);
    Om_strToUpper(&item_str); //< convert to upper case
  } else {
    // Get from ComboBox
    wchar_t cate[OM_MAX_ITEM];
    self->msgItem(IDC_CB_CAT, CB_GETLBTEXT, cb_sel, reinterpret_cast<LPARAM>(cate));
    item_str = cate;
  }
  self->_ModPack.setCategory(item_str);


  // get/update package dependencies list
  self->_ModPack.clearDepend();
  if(self->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
    int32_t lb_cnt = self->msgItem(IDC_LB_DPN, LB_GETCOUNT);
    if(lb_cnt) {
      wchar_t iden[OM_MAX_ITEM];
      for(int32_t i = 0; i < lb_cnt; ++i) {
        self->msgItem(IDC_LB_DPN, LB_GETTEXT, i, reinterpret_cast<LPARAM>(iden));
        self->_ModPack.addDependIden(iden);
      }
    }
  }

  // get package image data from specified file if any
  if(self->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
    self->getItemText(IDC_EC_INP08, item_str);
    if(!item_str.empty()) {
      self->_ModPack.setThumbnail(item_str);
    }
  } else {
    self->_ModPack.clearThumbnail();
  }

  // get package description text
  if(self->msgItem(IDC_BC_CKBX3, BM_GETCHECK)) {
    self->getItemText(IDC_EC_DESC, item_str);
    self->_ModPack.setDescription(item_str);
  } else {
    self->_ModPack.setDescription(L"");
  }

  // get package compression level
  int32_t comp_md, comp_lv;

  switch(self->msgItem(IDC_CB_ZMD, CB_GETCURSEL)) {
  case 1:   comp_md = OM_METHOD_DEFLATE; break; //< MZ_COMPRESS_METHOD_DEFLATE
  case 2:   comp_md = OM_METHOD_LZMA; break;    //< MZ_COMPRESS_METHOD_LZMA
  case 3:   comp_md = OM_METHOD_LZMA2; break;   //< MZ_COMPRESS_METHOD_XZ
  case 4:   comp_md = OM_METHOD_ZSTD; break;    //< MZ_COMPRESS_METHOD_ZSTD
  default:  comp_md = OM_METHOD_STORE; break;   //< MZ_COMPRESS_METHOD_STORE
  }

  switch(self->msgItem(IDC_CB_ZLV, CB_GETCURSEL)) {
  case 1:   comp_lv = OM_LEVEL_FAST; break; //< MZ_COMPRESS_LEVEL_FAST
  case 2:   comp_lv = OM_LEVEL_SLOW; break; //< MZ_COMPRESS_LEVEL_NORMAL
  case 3:   comp_lv = OM_LEVEL_BEST; break; //< MZ_COMPRESS_LEVEL_BEST
  default:  comp_lv = OM_LEVEL_NONE; break;
  }

  // get destination path & filename
  OmWString out_path, out_dir, out_name;
  self->getItemText(IDC_EC_INP06, out_dir);
  self->getItemText(IDC_EC_RESUL, out_name);
  Om_concatPaths(out_path, out_dir, out_name);

  self->_save_abort = false;
  self->enableItem(IDC_BC_ABORT, true);

  DWORD exitCode = 0;

  if(!self->_ModPack.saveAs(out_path, comp_md, comp_lv, &self->_save_progress_cb, self)) {

    // show error dialog box
    Om_dlgSaveError(self->_hwnd, L"Mod Pack Editor", L"Save Mod pack",
                    L"Mod pack", self->_ModPack.lastError());

    exitCode = 1;
  }

  // if user aborted, the was package not successfully created
  if(self->_save_abort) {
    self->_unsaved = true; //< still have unsaved changes
    exitCode = 1;
  }

  // send message to stop thread
  self->postMessage(UWM_PKGSAVE_DONE);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcRadSrc()
{
  // get checked Radio
  bool bm_chk = this->msgItem(IDC_BC_RAD01, BM_GETCHECK);

  // Check for unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgResetUnsaved(this->_hwnd, L"Mod Pack Editor")) {
      // undo the radio button changes by user
      this->msgItem(IDC_BC_RAD01, BM_SETCHECK, !bm_chk);
      this->msgItem(IDC_BC_RAD02, BM_SETCHECK, bm_chk);
      return; //< return now, don't change anything
    }
  }

  // reset source input EditText
  this->enableItem(IDC_EC_INP01, true);
  this->setItemText(IDC_EC_INP01, L"");
  this->enableItem(IDC_EC_INP02, true);
  this->setItemText(IDC_EC_INP02, L"");

  // enable or disable EditText and Buttons according selection
  this->enableItem(IDC_EC_INP01, bm_chk);
  this->enableItem(IDC_BC_BRW01, bm_chk);
  this->enableItem(IDC_EC_INP02, !bm_chk);
  this->enableItem(IDC_BC_BRW02, !bm_chk);

  // reset package source
  this->_parseSrc(L"");

  // reset unsaved changes
  this->_unsaved = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_onBcBrwDir(const wchar_t* path)
{
  // Check for unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgResetUnsaved(this->_hwnd, L"Mod Pack Editor"))
      return false; //< return now, don't change anything
  }

  OmWString result, start;

  if(path != nullptr) {

    result = path;

  } else {

    // Select start directory, either previous one or current location library
    this->getItemText(IDC_EC_INP01, start);

    if(start.empty()) {

      // select current Mod Channel library as default
      OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
      if(ModChan) {
        start = ModChan->libraryPath();
      }

    } else {
      start = Om_getDirPart(start);
    }

    if(!Om_dlgOpenDir(result, this->_hwnd, L"Select Mod source directory", start)) {
      this->setItemText(IDC_EC_INP01, L"");
      this->_parseSrc(L"");
      return false;
    }

  }

  if(!Om_isDir(result)) {
    this->setItemText(IDC_EC_INP01, L"");
    this->_parseSrc(L"");
    return false;
  }

  // check whether selected folder is empty
  if(Om_isDirEmpty(result)) {

    // show warning dialog box
    if(!Om_dlgBox_yn(this->_hwnd, L"Mod Pack Editor", IDI_QRY,
                L"Empty source directory", L"The selected source directory is "
                "empty, the resulting package will have no content to install."
                "Do you want to continue anyway ?"))
    {
      this->setItemText(IDC_EC_INP01, L"");
      this->_parseSrc(L"");
      return false;
    }

  }



  // Try to parse the folder (as package source)
  if(!this->_parseSrc(result)) {
    // this cannot happen at this stage... but...
    Om_dlgBox_okl(this->_hwnd, L"Mod Pack Editor", IDI_ERR,
                 L"Mod source parse error", L"Unable to parse "
                 "the specified directory Mod source:", result);

    this->setItemText(IDC_EC_INP01, L"");
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_onBcBrwPkg(const wchar_t* path)
{
  // Check for unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgResetUnsaved(this->_hwnd, L"Mod Pack Editor"))
      return false; //< return now, don't change anything
  }

  // reset unsaved changes
  this->_unsaved = false;

  OmWString result, start;

  if(path != nullptr) {

    result = path;

  } else {

    // Select start directory, either previous one or current location library
    this->getItemText(IDC_EC_INP02, start);

    if(start.empty()) {

      // select current Mod Channel library as default
      OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
      if(ModChan) {
        start = ModChan->libraryPath();
      }

    } else {
      start = Om_getDirPart(start);
    }

    // open select file dialog
    if(!Om_dlgOpenFile(result, this->_hwnd, L"Select Mod Pack file", OM_PKG_FILES_FILTER, start)) {
      this->setItemText(IDC_EC_INP02, L"");
      this->_parseSrc(L"");
      return false;
    }

  }

  if(!Om_isFile(result)) {
    this->setItemText(IDC_EC_INP02, L"");
    this->_parseSrc(L"");
    return false;
  }

  // parse this package source
  if(!this->_parseSrc(result)) {
    Om_dlgBox_okl(this->_hwnd, L"Mod Pack Editor", IDI_ERR,
                 L"Mod source parse error", L"Unable to parse "
                 "the specified file as Mod source:", result);

    this->setItemText(IDC_EC_INP02, L"");
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onNameChange()
{
  // get base name
  OmWString name_str;
  this->getItemText(IDC_EC_INP03, name_str);

  if(!name_str.empty()) {

    // Replace all spaces by underscores
    replace(name_str.begin(), name_str.end(), L' ', L'_');

    // get version
    OmWString vers_str;
    this->getItemText(IDC_EC_INP04, vers_str);

    // check if version string is valid
    OmVersion version(vers_str);
    if(!version.isNull()) {
      name_str += L"_v" + version.asString();
    }

    // get chosen file extension
    int cb_sel = this->msgItem(IDC_CB_EXT, CB_GETCURSEL, 0);
    wchar_t ext_str[6];
    this->msgItem(IDC_CB_EXT, CB_GETLBTEXT, cb_sel, reinterpret_cast<LPARAM>(ext_str));
    name_str += ext_str;

    // set final filename
    this->setItemText(IDC_EC_RESUL, name_str);

  } else {

    // reset output
    this->setItemText(IDC_EC_RESUL, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcBrwDest()
{
  OmWString result, start;

  // select start directory
  this->getItemText(IDC_EC_INP06, start);

  if(start.empty()) {
    if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
      this->getItemText(IDC_EC_INP01, start);
    } else {
      this->getItemText(IDC_EC_INP02, start);
    }
  }

  // open select folder dialog
  if(!Om_dlgOpenDir(result, this->_hwnd, L"Select destination location", Om_getDirPart(start)))
    return;

  if(!Om_isDir(result))
    return;

  // Set path to input EditText
  this->setItemText(IDC_EC_INP06, result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onLbDpnlsSel()
{
  // enable or disable Trash Button according selection
  int lb_sel = this->msgItem(IDC_LB_DPN, LB_GETCURSEL);

  this->enableItem(IDC_BC_DEL, (lb_sel >= 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onCbCatSel()
{
  int cb_sel = this->msgItem(IDC_CB_CAT, CB_GETCURSEL);

  // check whether user selected the last item (GENERIC)
  if(cb_sel == this->msgItem(IDC_CB_CAT, CB_GETCOUNT)-1) {
    // enable the text field
    this->enableItem(IDC_EC_INP09, true);
  } else {
    this->setItemText(IDC_EC_INP09, L"");
    this->enableItem(IDC_EC_INP09, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onCkBoxDep()
{
  bool bm_chk = this->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

  this->enableItem(IDC_SC_LBL06, bm_chk); //< "Ident" label
  this->enableItem(IDC_EC_INP07, bm_chk);
  this->enableItem(IDC_LB_DPN, bm_chk);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcAddDep()
{
  // Get identity string from EditText
  OmWString ident;
  this->getItemText(IDC_EC_INP07, ident);

  if(ident.empty())
    return;

  // Add string to dependencies
  this->msgItem(IDC_LB_DPN, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ident.c_str()));
  // Empty EditText
  this->setItemText(IDC_EC_INP07, L"");

  // disable Dependencies "+" Button
  this->enableItem(IDC_BC_ADD, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcDelDep()
{
  // get selected item index
  int lb_sel = this->msgItem(IDC_LB_DPN, LB_GETCURSEL);

  // remove from list
  if(lb_sel >= 0) {
    this->msgItem(IDC_LB_DPN, LB_DELETESTRING, lb_sel);
  }

  // disable Dependencies Trash Button
  this->enableItem(IDC_BC_DEL, false);
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onCkBoxSnap()
{
  if(this->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {

    this->enableItem(IDC_BC_BRW04, true);

  } else {

    this->enableItem(IDC_BC_BRW04, false);

    HBITMAP hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_BLANK));
    if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);

    this->setItemText(IDC_EC_INP08, L"");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_onBcBrwSnap()
{
  OmWString result, start;

  // select the start directory from package source path
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP01, start);
  } else {
    this->getItemText(IDC_EC_INP02, start);
  }

  // open file dialog
  if(!Om_dlgOpenFile(result, this->_hwnd, L"Open image file", OM_IMG_FILES_FILTER, Om_getDirPart(start)))
    return false;

  OmImage thumb;
  HBITMAP hBm;

  // Try to open image
  if(thumb.loadThumbnail(result, OM_MODPACK_THUMB_SIZE, OM_SIZE_FILL)) {

    // set thumbnail
    hBm = this->setStImage(IDC_SB_SNAP, thumb.hbmp());
    if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);

    // set EditText content to image path
    this->setItemText(IDC_EC_INP08, result);

  } else {

    // remove any thumbnail
    hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_BLANK));
    if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);

    // reset hidden EditText content
    this->setItemText(IDC_EC_INP08, L"");
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onCkBoxDesc()
{
  bool bm_chk = this->msgItem(IDC_BC_CKBX3, BM_GETCHECK);

  this->enableItem(IDC_BC_BRW05, bm_chk);
  this->enableItem(IDC_EC_DESC, bm_chk);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolPkg::_onBcBrwDesc()
{
  OmWString result, start;

  // select the start directory from package source path
  if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
    this->getItemText(IDC_EC_INP01, start);
  } else {
    this->getItemText(IDC_EC_INP02, start);
  }

  // open file dialog
  if(!Om_dlgOpenFile(result, this->_hwnd, L"Open text file", OM_TXT_FILES_FILTER, Om_getDirPart(start)))
    return false;

  if(!Om_isFile(result))
    return false;

  OmWString text_wcs;
  Om_loadToUTF16(&text_wcs, result);

  SetDlgItemTextW(this->_hwnd, IDC_EC_DESC, text_wcs.c_str());

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onBcSave()
{
  OmWString item_str, msg;

  // verify package has parsed source
  if(!this->_ModPack.hasSource()) {

    // show error dialog box
    Om_dlgBox_ok(this->_hwnd, L"Mod Pack Editor", IDI_ERR,
                 L"Invalid Mod source", L"Invalid or empty Mod source. "
                 "Please select a file or directory as Mod source.");
    return;
  }

  // verify the destination path and name
  OmWString out_name, out_dir;

  this->getItemText(IDC_EC_INP06, out_dir);
  this->getItemText(IDC_EC_RESUL, out_name);

  if(!Om_dlgValidName(this->_hwnd, L"Mod Pack filename", out_name))
    return;

  if(Om_dlgValidPath(this->_hwnd, L"Save Destination", out_dir)) {
    if(!Om_dlgCreateFolder(this->_hwnd, L"Save Destination", out_dir))
      return;
  } else {
    return;
  }

  if(!Om_dlgOverwriteFile(this->_hwnd, out_dir + L"\\" + out_name))
    return;

  // changes has been saved
  this->_unsaved = false;

  // disable the Save button
  this->enableItem(IDC_BC_SAVE, false);

  // here we go
  this->_save_init();
}

*/
///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(IDI_MOD_TOOL,2),Om_getResIcon(IDI_MOD_TOOL,1));

  // Set menu icons
  HMENU hMnuFile = this->getPopup(MNU_ME_FILE);
  this->setPopupItemIcon(hMnuFile, MNU_ME_FILE_NEW, Om_getResIconPremult(IDI_BT_NEW));
  this->setPopupItemIcon(hMnuFile, MNU_ME_FILE_OPEN, Om_getResIconPremult(IDI_BT_OPN));
  this->setPopupItemIcon(hMnuFile, MNU_ME_FILE_BUIL, Om_getResIconPremult(IDI_PKG_BLD));
  this->setPopupItemIcon(hMnuFile, MNU_ME_FILE_SAVE, Om_getResIconPremult(IDI_BT_SAV));
  this->setPopupItemIcon(hMnuFile, MNU_ME_FILE_SAVAS, Om_getResIconPremult(IDI_BT_SVA));
  this->setPopupItemIcon(hMnuFile, MNU_ME_FILE_QUIT, Om_getResIconPremult(IDI_QUIT));

  HMENU hMnuEdit = this->getPopup(MNU_ME_EDIT);
  this->setPopupItemIcon(hMnuEdit, MNU_ME_EDIT_THMBSEL, Om_getResIconPremult(IDI_PIC_ADD));
  this->setPopupItemIcon(hMnuEdit, MNU_ME_EDIT_DESCSEL, Om_getResIconPremult(IDI_TXT_ADD));
  this->setPopupItemIcon(hMnuEdit, MNU_ME_EDIT_DEPIMP, Om_getResIconPremult(IDI_PKG_IMP));

  // dialog is modeless so we set dialog title with app name
  this->setCaption(L"Mod-Package editor");

  // Create the toolbar.
  CreateWindowExW(WS_EX_LEFT, TOOLBARCLASSNAMEW, nullptr, WS_CHILD|TBSTYLE_WRAPABLE|TBSTYLE_TOOLTIPS, 0, 0, 0, 0,
                  this->_hwnd, reinterpret_cast<HMENU>(IDC_TB_TOOLS), this->_hins, nullptr);

  HIMAGELIST himl = static_cast<OmUiMan*>(this->root())->toolBarsImgList();
  this->msgItem(IDC_TB_TOOLS, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(himl));

  // Initialize button info.
  TBBUTTON tbButtons[4] = {
    {ICON_NEW, IDC_BC_NEW,  TBSTATE_ENABLED, 0/*BTNS_AUTOSIZE*/, {0}, 0, reinterpret_cast<INT_PTR>("New Package")},
    {ICON_OPN, IDC_BC_OPEN, TBSTATE_ENABLED, 0/*BTNS_AUTOSIZE*/, {0}, 0, reinterpret_cast<INT_PTR>("Open Package file")},
    {ICON_BLD, IDC_BC_OPEN2,TBSTATE_ENABLED, 0/*BTNS_AUTOSIZE*/, {0}, 0, reinterpret_cast<INT_PTR>("Build from directory")},
    {ICON_SAV, IDC_BC_SAVE, 0,               0/*BTNS_AUTOSIZE*/, {0}, 0, reinterpret_cast<INT_PTR>("Save Package file")}
  };

  // Add buttons
  this->msgItem(IDC_TB_TOOLS, TB_SETMAXTEXTROWS, 0); //< disable text under buttons
  this->msgItem(IDC_TB_TOOLS, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON));
  this->msgItem(IDC_TB_TOOLS, TB_ADDBUTTONS, 4, reinterpret_cast<LPARAM>(&tbButtons));

  this->msgItem(IDC_TB_TOOLS, TB_SETBUTTONSIZE, 0, MAKELPARAM(26, 22));
  //this->msgItem(IDC_TB_TOOLS, TB_SETLISTGAP, 25);                       //< this does not work
  //this->msgItem(IDC_TB_TOOLS, TB_SETPADDING, 0, MAKELPARAM(25, 20));    //< this does not work

  // Resize and show the toolbar
  this->msgItem(IDC_TB_TOOLS, TB_AUTOSIZE);
  this->showItem(IDC_TB_TOOLS, true);

  // add items to extension ComboBox
  this->msgItem(IDC_CB_EXT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"." OM_PKG_FILE_EXT));
  this->msgItem(IDC_CB_EXT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L".zip"));
  this->msgItem(IDC_CB_EXT, CB_SETCURSEL, 0, 0);

  // add items into Category ComboBox
  for(size_t i = 0; i < __categ_count; ++i) {
    this->msgItem(IDC_CB_CAT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(__categ_list[i]));
  }
  this->msgItem(IDC_CB_CAT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"- custom -"));
  this->msgItem(IDC_CB_CAT, CB_SETCURSEL, 0, 0);

  int32_t cb_id;

  // add items to Compression Method ComboBox
  cb_id = this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None (Store only)"));
  this->msgItem(IDC_CB_ZMD, CB_SETITEMDATA, cb_id, OM_METHOD_STORE);
  cb_id = this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Deflate (Legacy Zip)"));
  this->msgItem(IDC_CB_ZMD, CB_SETITEMDATA, cb_id, OM_METHOD_DEFLATE);
  cb_id = this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"LZMA"));
  this->msgItem(IDC_CB_ZMD, CB_SETITEMDATA, cb_id, OM_METHOD_LZMA);
  cb_id = this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"LZMA2"));
  this->msgItem(IDC_CB_ZMD, CB_SETITEMDATA, cb_id, OM_METHOD_LZMA2);
  cb_id = this->msgItem(IDC_CB_ZMD, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Zstandard"));
  this->msgItem(IDC_CB_ZMD, CB_SETITEMDATA, cb_id, OM_METHOD_ZSTD);
  this->msgItem(IDC_CB_ZMD, CB_SETCURSEL, 4);

  // add items into Compression Level ComboBox
  cb_id = this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"None (Store only)"));
  this->msgItem(IDC_CB_ZLV, CB_SETITEMDATA, cb_id, OM_LEVEL_NONE);
  cb_id = this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Fast"));
  this->msgItem(IDC_CB_ZLV, CB_SETITEMDATA, cb_id, OM_LEVEL_FAST);
  cb_id = this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal"));
  this->msgItem(IDC_CB_ZLV, CB_SETITEMDATA, cb_id, OM_LEVEL_SLOW);
  cb_id = this->msgItem(IDC_CB_ZLV, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Best"));
  this->msgItem(IDC_CB_ZLV, CB_SETITEMDATA, cb_id, OM_LEVEL_BEST);
  this->msgItem(IDC_CB_ZLV, CB_SETCURSEL, 2);

  // Set thumbnail placeholder image
  this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_BLANK));

  // Set buttons icons
  this->setBmIcon(IDC_BC_BRW03, Om_getResIcon(IDI_PIC_ADD));  //< Thumbnail Select
  this->setBmIcon(IDC_BC_BRW04, Om_getResIcon(IDI_TXT_ADD));  //< Description Load
  this->setBmIcon(IDC_BC_DPBRW, Om_getResIcon(IDI_PKG_IMP));  //< Dependencies Select
  this->setBmIcon(IDC_BC_DPADD, Om_getResIcon(IDI_BT_ADD));   //< Dependencies Add
  this->setBmIcon(IDC_BC_DPDEL, Om_getResIcon(IDI_BT_REM));   //< Dependencies Delete
  this->setBmIcon(IDC_BC_DPVAL, Om_getResIcon(IDI_BT_VAL));   //< Depend-Add Valid
  this->setBmIcon(IDC_BC_DPABT, Om_getResIcon(IDI_BT_ABT));   //< Depend-Add Abort

  // Hide the "Add Dependency" controls
  this->showItem(IDC_SC_LBL08, false);
  this->showItem(IDC_EC_INP08, false);
  this->showItem(IDC_BC_DPVAL, false);
  this->showItem(IDC_BC_DPABT, false);

  // set tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Display name");
  this->_createTooltip(IDC_EC_INP02,  L"Version string");
  this->_createTooltip(IDC_EC_RESUL,  L"Filename preview");

  this->_createTooltip(IDC_CB_ZMD,    L"Archive compression algorithm");
  this->_createTooltip(IDC_CB_ZLV,    L"Archive compression level");

  this->_createTooltip(IDC_EC_READ1,  L"List of Mod files");

  this->_createTooltip(IDC_CB_CAT,    L"Predefined categories");
  this->_createTooltip(IDC_EC_INP07,  L"Custom category");

  this->_createTooltip(IDC_BC_CKBX1,  L"Enable overview thumbnail");
  this->_createTooltip(IDC_BC_BRW03,  L"Load thumbnail image");

  this->_createTooltip(IDC_BC_CKBX2,  L"Enable overview description");
  this->_createTooltip(IDC_BC_BRW04,  L"Load description text");

  this->_createTooltip(IDC_BC_CKBX3,  L"Enable Mod dependencies");
  this->_createTooltip(IDC_BC_DPADD,  L"Add dependency Mod");
  this->_createTooltip(IDC_BC_DPBRW,  L"Select dependency Mod(s)");
  this->_createTooltip(IDC_BC_DPDEL,  L"Remove dependency");
  this->_createTooltip(IDC_EC_INP08,  L"Dependency Mod Identity string");
  this->_createTooltip(IDC_BC_DPVAL,  L"Valid");
  this->_createTooltip(IDC_BC_DPABT,  L"Abort");

  // update status
  this->_status_update_filename();

  // nothing to save
  this->_set_unsaved(false);

  // reset controls to initial states
  this->_reset_controls();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onResize()
{
  int32_t half_w = this->cliWidth() * 0.5f;
  int32_t quar_w = this->cliWidth() * 0.25f;
  int32_t base_y = 33;
  int32_t foot_y = this->cliHeight() - (base_y+28);

  // resize the toolbar
  this->msgItem(IDC_TB_TOOLS, TB_AUTOSIZE);

  // toolbar separator
  this->_setItemPos(IDC_SC_SEPAR, -1, 28, this->cliWidth()+2, 1, true);

  // -- Left Frame --

  // Name & Version Labels
  this->_setItemPos(IDC_SC_LBL01, 10, base_y, 50, 16, true);
  this->_setItemPos(IDC_SC_LBL02, half_w-125, base_y, 60, 16, true);
  // Name & Version EditControl + Extension ComboBox
  this->_setItemPos(IDC_EC_INP01, 10, base_y+20, half_w-140, 21, true);
  this->_setItemPos(IDC_EC_INP02, half_w-125, base_y+20, 60, 21, true);
  this->_setItemPos(IDC_CB_EXT,   half_w-60, base_y+20, 50, 21, true);
  //filename preview Label & EditControl
  //this->_setItemPos(IDC_SC_LBL03, 10, base_y+50, 200, 21, true);
  this->_setItemPos(IDC_EC_RESUL, 10, base_y+50, half_w-20, 21, true);

  // Compression Method / Level Labels
  this->_setItemPos(IDC_SC_LBL03, 10, base_y+90, 200, 21, true);
  //this->_setItemPos(IDC_SC_LBL04, 10, base_y+115, 50, 16, true);
  //this->_setItemPos(IDC_SC_LBL05, quar_w+10, base_y+115, 40, 16, true);
  // Compression Method /Level ComboBoxes
  this->_setItemPos(IDC_CB_ZMD, 10, base_y+110, quar_w-15, 21, true);
  this->_setItemPos(IDC_CB_ZLV, quar_w+5, base_y+110, quar_w-20, 21, true);

  // Content Label
  this->_setItemPos(IDC_SC_LBL06, 10, base_y+150, 200, 21, true);
  this->_setItemPos(IDC_EC_READ1, 10, base_y+170, half_w-20, foot_y-180, true);

  // -- Right Frame --

  // Category Label
  this->_setItemPos(IDC_SC_LBL07, half_w+10, base_y, 200, 16, true);
  // Category ComboBox
  this->_setItemPos(IDC_CB_CAT, half_w+10, base_y+20, half_w-20, 21, true);
  // Category EditControl
  this->_setItemPos(IDC_EC_INP07, half_w+10, base_y+50, half_w-20, 21, true);

  // Thumbnail CheckBox
  this->_setItemPos(IDC_BC_CKBX1, half_w+10, base_y+90, 100, 16, true);
  // Thumbnail notice text
  this->_setItemPos(IDC_SC_NOTES, half_w+120, base_y+91, 150, 16, true);
  // Thumbnail Action buttons
  this->_setItemPos(IDC_BC_BRW03, this->cliWidth()-32, base_y+87, 22, 22, true);
  // Thumbnail static bitmap
  this->_setItemPos(IDC_SB_SNAP, half_w+10, base_y+110, 128, 128, true);

  // Description CheckBox
  this->_setItemPos(IDC_BC_CKBX2, half_w+10, base_y+260, 100, 16, true);
  // Description Actions buttons
  this->_setItemPos(IDC_BC_BRW04, this->cliWidth()-32, base_y+257, 22, 22, true);
  // Description EditControl
  this->_setItemPos(IDC_EC_DESC, half_w+10, base_y+280, half_w-21, foot_y-(290+110), true);

  // Dependencies CheckBox
  this->_setItemPos(IDC_BC_CKBX3, half_w+10, foot_y-65, 90, 16, true);
  // Dependencies notice text
  this->_setItemPos(IDC_SC_HELP, half_w+120, foot_y-64, 140, 16, true);
  // Dependencies Actions buttons
  this->_setItemPos(IDC_BC_DPADD, this->cliWidth()-78, foot_y-68, 22, 22, true);
  this->_setItemPos(IDC_BC_DPBRW, this->cliWidth()-55, foot_y-68, 22, 22, true);
  this->_setItemPos(IDC_BC_DPDEL, this->cliWidth()-32, foot_y-68, 22, 22, true);
  // Dependencies ListBox
  this->_setItemPos(IDC_LB_DPN, half_w+10, foot_y-45, half_w-20, 68, true);

  // Dependencies Add Label
  this->_setItemPos(IDC_SC_LBL08, half_w+10, foot_y-35, 200, 16, true);
  // Dependencies Add Entry
  this->_setItemPos(IDC_EC_INP08, half_w+10, foot_y-15, half_w-20, 21, true);
  // Dependencies Add Buttons
  this->_setItemPos(IDC_BC_DPVAL, this->cliWidth()-55, foot_y-38, 22, 22, true);
  this->_setItemPos(IDC_BC_DPABT, this->cliWidth()-32, foot_y-38, 22, 22, true);
  /*
  // Ident Label, EditText & + Button
  this->_setItemPos(IDC_SC_LBL06, half_w+10, 87, 35, 9);
  this->_setItemPos(IDC_EC_INP07, half_w+40, 85, half_w-70, 13);
  this->_setItemPos(IDC_BC_ADD, this->cliUnitX()-25, 85, 16, 13);
  */



  /*
  unsigned half_w = static_cast<float>(this->cliUnitX()) * 0.5f;

  // -- Left Frame --

  // From folder RadioButton
  this->_setItemPos(IDC_BC_RAD01, 10, 10, 150, 9);
  // From Folder EditControl & Browse Button
  this->_setItemPos(IDC_EC_INP01, 10, 20, half_w-75, 13);
  this->_setItemPos(IDC_BC_BRW01, half_w-60, 19, 50, 14);

  // From existing Package RadioButton
  this->_setItemPos(IDC_BC_RAD02, 10, 40, 150, 9);
  // From existing Package EditControl & Browse Button
  this->_setItemPos(IDC_EC_INP02, 10, 50, half_w-75, 13);
  this->_setItemPos(IDC_BC_BRW02, half_w-60, 49, 50, 14);

  // [ - - - File name & Zip compression GroupBox - - -
  this->_setItemPos(IDC_GB_GRP01, 5, 65, half_w-10, this->cliUnitY()-147);
  // Package filename Label
  this->_setItemPos(IDC_SC_LBL01, 10, 75, 120, 9);
  // Name Label & EditText
  this->_setItemPos(IDC_SC_LBL02, 10, 90, 23, 9);
  this->_setItemPos(IDC_EC_INP03, 35, 88, half_w-145, 13);
  // Version label & EditText
  this->_setItemPos(IDC_SC_LBL03, half_w-105, 90, 27, 9);
  this->_setItemPos(IDC_EC_INP04, half_w-75, 88, 30, 13);
  // File extension ComboBox
  this->_setItemPos(IDC_CB_EXT, half_w-40, 88, 30, 13);
  // Output filname EditText
  this->_setItemPos(IDC_EC_RESUL, 10, 104, half_w-20, 13);
  // Compression Method Label & ComboBox
  this->_setItemPos(IDC_SC_LBL09, 10, 125, 90, 9);
  this->_setItemPos(IDC_CB_ZMD, 10, 137, half_w-140, 13);
  // Compression Level Label & ComboBox
  this->_setItemPos(IDC_SC_LBL04, 120, 125, 90, 9);
  this->_setItemPos(IDC_CB_ZLV, 120, 137, half_w-130, 13);
  // Package content Label & output EditText
  this->_setItemPos(IDC_SC_LBL07, 10, 158, 150, 13);
  this->_setItemPos(IDC_EC_READ1, 10, 170, half_w-20, this->cliUnitY()-258);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // Destination Label
  this->_setItemPos(IDC_SC_LBL05, 10, this->cliUnitY()-77, 120, 8);
  // Destination EditText & Browse Button
  this->_setItemPos(IDC_EC_INP06, 10, this->cliUnitY()-65, half_w-40, 13);
  this->_setItemPos(IDC_BC_BRW03, half_w-25, this->cliUnitY()-65, 15, 13);

  // Save Button
  this->_setItemPos(IDC_BC_SAVE, 10, this->cliUnitY()-45, 45, 14);
  // Progress Bar
  this->_setItemPos(IDC_PB_MOD, 57, this->cliUnitY()-44, half_w-114, 12);
  // Abort Button
  this->_setItemPos(IDC_BC_ABORT, half_w-55, this->cliUnitY()-45, 45, 14);

  // -- Right Frame --

  // [ - - -         Category GroupBox           - - -
  this->_setItemPos(IDC_GB_GRP02, half_w+5, 0, half_w-10, 60);
  // Category Label
  this->_setItemPos(IDC_SC_LBL08, half_w+10, 10, 120, 9);
  // Category ComboBox & EditText
  this->_setItemPos(IDC_CB_CAT, half_w+10, 25, half_w-20, 14);
  // Category ComboBox & EditText
  this->_setItemPos(IDC_EC_INP09, half_w+10, 40, half_w-20, 14);

  // [ - - -       Dependencies GroupBox          - - -
  this->_setItemPos(IDC_GB_GRP03, half_w+5, 60, half_w-10, 75);
  // Has Dependencies CheckBox
  this->_setItemPos(IDC_BC_CKBX1, half_w+10, 70, 120, 9);
  // Ident Label, EditText & + Button
  this->_setItemPos(IDC_SC_LBL06, half_w+10, 87, 35, 9);
  this->_setItemPos(IDC_EC_INP07, half_w+40, 85, half_w-70, 13);
  this->_setItemPos(IDC_BC_ADD, this->cliUnitX()-25, 85, 16, 13);
  // Depend ListBox & Trash Button
  this->_setItemPos(IDC_LB_DPN, half_w+10, 100, half_w-40, 30);
  this->_setItemPos(IDC_BC_DEL, this->cliUnitX()-25, 100, 16, 13);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Snapshot GroupBox             - - -
  this->_setItemPos(IDC_GB_GRP04, half_w+5, 135, half_w-10, 95);
  // Include snapshot CheckBox
  this->_setItemPos(IDC_BC_CKBX2, half_w+10, 145, 70, 9);
  // Snapshot Bitmap & Select... Button
  this->_setItemPos(IDC_SB_SNAP, this->cliUnitX()-160, 146, 86, 79);
  this->_setItemPos(IDC_BC_BRW04, this->cliUnitX()-50, 145, 40, 13);
  // Snapshot hidden EditText
  this->_setItemPos(IDC_EC_INP08, half_w+10, 160, 120, 13); // hidden
  // Snapshot helper Static text
  this->_setItemPos(IDC_SC_NOTES, this->cliUnitX()-230, 180, 60, 35);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Description GroupBox          - - -
  this->_setItemPos(IDC_GB_GRP05, half_w+5, 230, half_w-10, this->cliUnitY()-260);
  // Description CheckBox & Load.. Button
  this->_setItemPos(IDC_BC_CKBX3, half_w+10, 240, 100, 9);
  this->_setItemPos(IDC_BC_BRW05, this->cliUnitX()-50, 240, 40, 13);
  // Description EditText
  this->_setItemPos(IDC_EC_DESC, half_w+10, 255, half_w-20, this->cliUnitY()-290);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // ----- Separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->cliUnitY()-25, this->cliUnitX()-10, 1);
  // Close Button
  this->_setItemPos(IDC_BC_CLOSE, this->cliUnitX()-54, this->cliUnitY()-19, 50, 14);
  */

  // Foot status bar
  this->_setItemPos(IDC_SC_STATUS, 2, this->cliHeight()-24, this->cliWidth()-4, 22, true);
  this->_setItemPos(IDC_SC_FILE, 7, this->cliHeight()-20, this->cliWidth()-110, 16, true);
  this->_setItemPos(IDC_SC_INFO, this->cliWidth()-97, this->cliHeight()-20, 90, 16, true);

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolPkg::_onClose()
{
  /*
  // Check for unsaved changes
  if(this->_unsaved) {
    // ask user to save
    if(!Om_dlgCloseUnsaved(this->_hwnd, L"Mod Pack Editor")) {
      return; //< do NOT close
    }
  }

  // gracefully exit current thread before exist
  if(this->_save_hth != nullptr) {
    this->_save_abort = true;
    this->_save_stop();
  }
  */
  this->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiToolPkg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {
    switch(LOWORD(wParam))
    {
    case IDC_BC_OPEN:
    case IDM_FILE_OPEN:
      this->_modpack_open();
      break;

    case IDC_BC_OPEN2:
    case IDM_MOD_BUIL:
      this->_modpack_build();
      break;

    case IDC_BC_SAVE:
    case IDM_FILE_SAVE:
      this->_modpack_save();
      break;

    case IDM_FILE_SAVAS:
      this->_modpack_save_as();
      break;

    case IDC_EC_INP01: //< Entry : Name
    case IDC_EC_INP02: //< Entry : Version
      if(HIWORD(wParam) == EN_CHANGE) {
        this->_name_compose();
      }
      break;

    case IDC_CB_EXT:  //< ComboBox: File extension
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        this->_name_compose();
        this->_check_zip_method();
      }
      break;

    case IDC_CB_ZMD:  //< ComboBox: File extension
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->_check_zip_method();
      break;

    case IDC_CB_CAT:  //< ComboBox: Category
      if(HIWORD(wParam) == CBN_SELCHANGE)
        this->_categ_select();
      break;

    case IDC_EC_INP07: //< Entry : Custom Category
      //if(HIWORD(wParam) == EN_CHANGE)
      if(HIWORD(wParam) == EN_KILLFOCUS)
        this->_categ_changed();
      break;

    case IDC_BC_CKBX1: //< CheckBox: Description
      if(HIWORD(wParam) == BN_CLICKED)
        this->_thumb_toggle();
      break;

    case IDC_BC_BRW03: //< Button : Description: Load
      if(HIWORD(wParam) == BN_CLICKED)
        this->_thumb_load();
      break;

    case IDM_THMB_SEL: //< Menu: Add thumbnail
      {
        this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 1);
        this->_thumb_toggle();
        this->_thumb_load();
      }
      break;

    case IDC_BC_CKBX2: //< CheckBox: Description
      if(HIWORD(wParam) == BN_CLICKED)
        this->_desc_toggle();
      break;

    case IDC_BC_BRW04: //< Button : Description: Load
      if(HIWORD(wParam) == BN_CLICKED)
        this->_desc_load();
      break;

    case IDM_DESC_SEL: //< Menu: Add thumbnail
      {
        this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, 1);
        this->_desc_toggle();
        this->_desc_load();
      }
      break;

    case IDC_EC_DESC: //< Entry : Description entry
      //if(HIWORD(wParam) == EN_CHANGE)
      if(HIWORD(wParam) == EN_KILLFOCUS)
        this->_desc_changed();
      break;

    case IDC_BC_CKBX3: //< CheckBox: Dependencies
      if(HIWORD(wParam) == BN_CLICKED)
        this->_depend_toggle();
      break;

    case IDC_LB_DPN: //< Packages list ListBox
      // check for selection change
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        this->_depend_sel_changed();
      }
      break;

    case IDC_BC_DPADD: //< Button : Dependencies: Add
      if(HIWORD(wParam) == BN_CLICKED) {
        this->_depend_add_show(true);
        SetFocus(this->getItem(IDC_EC_INP08));
      }
      break;

    case IDC_BC_DPBRW: //< Button : Dependencies: Browse
      if(HIWORD(wParam) == BN_CLICKED)
        this->_depend_browse();
      break;

    case IDM_DEP_ADD: //< Menu: Add thumbnail
      {
        this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, 1);
        this->_depend_toggle();
        this->_depend_browse();
      }
      break;

    case IDC_BC_DPDEL: //< Button : Dependencies: Delete
      if(HIWORD(wParam) == BN_CLICKED)
        this->_depend_delete();
      break;

    case IDC_BC_DPABT: //< Button : Add-Depend Prompt: Abort
      if(HIWORD(wParam) == BN_CLICKED) {
        this->setItemText(IDC_EC_INP08, L"");
        this->_depend_add_show(false);
      }
      break;

    case IDC_EC_INP08: //< Entry : Add-Depend Prompt: entry
      if(HIWORD(wParam) == EN_CHANGE) {
        int32_t txt_len = GetWindowTextLengthW(this->getItem(IDC_EC_INP08));
        this->enableItem(IDC_BC_DPVAL, (txt_len > 1));
      }

    case IDC_BC_DPVAL: //< Button : Add-Depend Prompt: Abort
      if(HIWORD(wParam) == BN_CLICKED)
        this->_depend_add_valid();
      break;

    }
  }
/*
  // UWM_PKGSAVE_DONE is a custom message sent from Package Save
  // thread function, to notify the thread ended is job.
  if(uMsg == UWM_PKGSAVE_DONE) {
    // properly stop the running thread and finish process
    this->_save_stop();
    return false;
  }

  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    OmWString item_str;

    switch(LOWORD(wParam))
    {
    case IDC_BC_RAD01:
    case IDC_BC_RAD02:
      this->_onBcRadSrc();
      break;

    case IDC_BC_BRW01: //< Create from folder "Select..." Button
      has_changed = this->_onBcBrwDir();
      break;

    case IDC_EC_INP01:
      // check for content changes
      //if(HIWORD(wParam) == EN_CHANGE)
        //has_changed = true;
      break;

    case IDC_BC_BRW02: //< Edit from existing "Open..." Button
      has_changed = this->_onBcBrwPkg();
      break;

    case IDC_EC_INP02:
      // check for content changes
      //if(HIWORD(wParam) == EN_CHANGE)
        //has_changed = true;
      break;

    case IDC_EC_INP03: //< Package Name input EditText
    case IDC_EC_INP04: //< Package Version input EditText
      if(HIWORD(wParam) == EN_CHANGE) {
        this->_onNameChange();
        has_changed = true;
      }
      break;
    case IDC_CB_EXT: //< File Extension ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE) {
        this->_onNameChange();
        has_changed = true;
      }
      break;

    case IDC_BC_BRW03:  // Destination folder "..." Button
      this->_onBcBrwDest();
      break;

    case IDC_EC_INP06: //< Destination folder input EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
      break;

    case IDC_CB_CAT: //< Location ComboBox
      if(HIWORD(wParam) == CBN_SELCHANGE) this->_onCbCatSel();
      break;

    case IDC_BC_CKBX1: //< Has Dependencies CheckBox
      this->_onCkBoxDep();
    break;

    case IDC_EC_INP07: //< Dependencies input EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE) {
        this->getItemText(IDC_EC_INP07, item_str);
        this->enableItem(IDC_BC_ADD, !item_str.empty());
      }
      break;

    case IDC_LB_DPN: //< Dependencies ListBox
      // check for selection change
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_onLbDpnlsSel();
      break;

    case IDC_BC_ADD: //< Dependencies "+" Button
      this->_onBcAddDep();
      has_changed = true;
      break;

    case IDC_BC_DEL: //< Dependencies "-" Button
      this->_onBcDelDep();
      has_changed = true;
      break;

    case IDC_BC_CKBX2:  //< Include snapshot CheckBox
      this->_onCkBoxSnap();
      break;

    case IDC_BC_BRW04: //< Snapshot "Select..." Button
      has_changed = this->_onBcBrwSnap();
      break;

    case IDC_BC_CKBX3: //< Include Description CheckBox
      this->_onCkBoxDesc();
    break;

    case IDC_BC_BRW05: //< Description "Load..." Button
      has_changed = this->_onBcBrwDesc();
      break;

    case IDC_EC_DESC: //< Description EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE) {
        this->enableItem(IDC_BC_SAVE, true); //< enable "Save" Button
        has_changed = true;
      }
      break;

    case IDC_BC_SAVE: //< Main "Save" Button
      this->_onBcSave();
      break;

    case IDC_BC_ABORT: //< Main "Abort" Button
      this->_save_abort = true;
      break;

    case IDC_BC_CLOSE: //< Main "Close" Button
      this->_onClose();
      break;
    }

    if(has_changed) {
      bool allow = false;

      // we have unsaved changes
      this->_unsaved = true;

      if(this->msgItem(IDC_BC_RAD01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INP01, item_str);
      } else {
        this->getItemText(IDC_EC_INP02, item_str);
      }

      if(!item_str.empty()) {
        this->getItemText(IDC_EC_INP06, item_str);
        if(!item_str.empty()) allow = true;
      }

      this->enableItem(IDC_BC_SAVE, allow);
    }
  }
  */

  return false;
}
