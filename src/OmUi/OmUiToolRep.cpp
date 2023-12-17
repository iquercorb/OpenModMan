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

#include "OmBaseWin.h"
  #include <UxTheme.h>
//#include <ShlObj.h>

#include "OmBaseUi.h"

#include "OmBaseApp.h"

#include "OmModMan.h"
#include "OmModHub.h"
#include "OmModPack.h"

#include "OmUtilFs.h"
#include "OmUtilAlg.h"
#include "OmUtilDlg.h"
#include "OmUtilStr.h"
#include "OmUtilImg.h"
#include "OmUtilHsh.h"
#include "OmUtilB64.h"
#include "OmUtilZip.h"
#include "OmUtilWin.h"

#include "OmUiMan.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiToolRep.h"

#define REPO_DEFAULT_DOWLOAD  L"files/"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolRep::OmUiToolRep(HINSTANCE hins) : OmDialog(hins),
  _NetRepo(new OmNetRepo(nullptr)),
  _has_unsaved(false),
  _reflist_lastsel(-1),
  _reflist_add_abort(0),
  _reflist_add_hth(nullptr),
  _reflist_add_hwo(nullptr),
  _reflist_add_hpd(nullptr)
{
  // set the accelerator table for the dialog
  this->setAccel(IDR_ACCEL);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolRep::~OmUiToolRep()
{
  if(this->_NetRepo)
    delete this->_NetRepo;

  HBITMAP hBm = this->setStImage(IDC_SB_SNAP, nullptr);
  if(hBm && hBm != Om_getResImage(IDB_SC_THMB_BLANK)) DeleteObject(hBm);

  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  DeleteObject(hFt);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiToolRep::id() const
{
  return IDD_TOOL_REP;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_status_update_filename()
{
  OmWString file_path;

  // set definition file path to status bar
  if(!this->_NetRepo->path().empty()) {
    file_path = this->_NetRepo->path();
  } else {
    file_path = L"<unsaved definition>";
  }

  this->setItemText(IDC_SC_FILE, file_path);

  OmWString caption = Om_getFilePart(file_path);
  caption += L" - Repository editor";
  this->setCaption(caption);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_status_update_references()
{
  // set references count to status bar
  OmWString sc_entry = std::to_wstring(this->_NetRepo->referenceCount());
  sc_entry += L" reference(s)";

  this->setItemText(IDC_SC_INFO, sc_entry);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_set_unsaved(bool enable)
{
  this->_has_unsaved = enable;

  // enable/disable and change tool bar 'save' button image
  TBBUTTONINFOA tbBi = {}; tbBi.cbSize = sizeof(TBBUTTONINFOA);
  tbBi.dwMask = TBIF_STATE;
  //tbBi.dwMask = TBIF_IMAGE|TBIF_STATE;
  //tbBi.iImage = (this->_has_unsaved) ? ICON_SAV : ICON_SVD;
  tbBi.fsState = (this->_has_unsaved) ? TBSTATE_ENABLED : 0;
  this->msgItem(IDC_TB_TOOLS, TB_SETBUTTONINFO, IDC_BC_SAVE, reinterpret_cast<LPARAM>(&tbBi));

  // enable/disable and change menu 'save' item
  if(this->_has_unsaved && !this->_NetRepo->path().empty()) {
    this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVE, MF_ENABLED);
  } else {
    this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVE, MF_GRAYED);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t OmUiToolRep::_ask_unsaved()
{
  // Check and ask for unsaved changes
  if(this->_has_unsaved)
    return Om_dlgBox_ync(this->_hwnd, L"Repository editor", IDI_DLG_QRY, L"Unsaved changes", L"Do you want to save changes before closing ?");

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repository_init()
{
  // Check and ask for unsaved changes
  switch(this->_ask_unsaved()) {
    case  1: this->_repository_save(); break; //< 'Yes'
    case -1: return;                    //< 'Cancel'
  }

  this->_NetRepo->init(L"New Repository");

  // disable/enable proper menu items
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_FAD, MF_ENABLED);
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_DAD, MF_ENABLED);
  this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVE, MF_GRAYED);
  this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVAS, MF_ENABLED);

  // update status bar and caption
  this->_status_update_filename();
  this->_status_update_references();

  // populate title and download path
  this->enableItem(IDC_EC_INP01, true);
  this->setItemText(IDC_EC_INP01, this->_NetRepo->title());
  this->enableItem(IDC_EC_INP02, true);
  this->setItemText(IDC_EC_INP02, this->_NetRepo->downpath());

  this->enableItem(IDC_BC_BRW01, true);
  this->enableItem(IDC_BC_BRW02, true);

  this->enableItem(IDC_LV_MOD, true);
  this->_reflist_populate();

  // update selection
  this->_reflist_selchg();

  // reset unsaved changes
  this->_set_unsaved(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_repository_load(const OmWString& path)
{
  bool has_failed = false;

  // (try) load and parse repository XML file
  OmResult result = this->_NetRepo->load(path);
  if(result != OM_RESULT_OK) {
    Om_dlgBox_okl(this->_hwnd, L"Repository editor", IDI_DLG_ERR, L"Repository open error",
                  L"Unable to load Repository definition:", this->_NetRepo->lastError());
    has_failed = true;
  }

  // reset unsaved changes
  this->_set_unsaved(false);

  // update status bar and caption
  this->_status_update_filename();
  this->_status_update_references();

  if(has_failed)
    return false;

  // disable/enable proper menu items
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_FAD, MF_ENABLED);
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_DAD, MF_ENABLED);
  this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVE, MF_GRAYED);
  this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVAS, MF_ENABLED);

  // populate title and download path
  this->enableItem(IDC_EC_INP01, true);
  this->setItemText(IDC_EC_INP01, this->_NetRepo->title());
  this->enableItem(IDC_EC_INP02, true);
  this->setItemText(IDC_EC_INP02, this->_NetRepo->downpath());

  this->enableItem(IDC_BC_BRW01, true);
  this->enableItem(IDC_BC_BRW02, true);

  // populate references list
  this->enableItem(IDC_LV_MOD, true);
  this->_reflist_populate();

  // update selection
  this->_reflist_selchg();

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_repository_save(const OmWString& path)
{
  bool has_failed = false;

  // ensure last values saved
  this->_repository_save_title();
  this->_repository_save_downpath();
  // ensure current reference changes are saved
  this->_reference_url_changed();
  this->_reference_desc_changed();

  if(OM_RESULT_OK != this->_NetRepo->save(path)) {

    Om_dlgBox_okl(this->_hwnd, L"Repository editor", IDI_DLG_ERR, L"Save file error",
                  L"Unable to save file:", this->_NetRepo->lastError());

    has_failed = true;
  }

  // update status bar and caption
  this->_status_update_filename();

  // changes now saved
  this->_set_unsaved(false);

  return !has_failed;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repository_close()
{
  // Check and ask for unsaved changes
  switch(this->_ask_unsaved()) {
    case  1: this->_repository_save(); break; //< 'Yes'
    case -1: return;                    //< 'Cancel'
  }

  this->_NetRepo->clear();

  // disable/enable proper menu items
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_FAD, MF_GRAYED);
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_DAD, MF_GRAYED);
  this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVE, MF_GRAYED);
  this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVAS, MF_GRAYED);

  // update status bar and caption
  this->_status_update_filename();
  this->_status_update_references();

  // populate title and download path
  this->enableItem(IDC_EC_INP01, false);
  this->setItemText(IDC_EC_INP01, L"");
  this->enableItem(IDC_EC_INP02, false);
  this->setItemText(IDC_EC_INP02, L"");

  this->enableItem(IDC_BC_BRW01, false);
  this->enableItem(IDC_BC_BRW02, false);

  // disable and rebuild ListView
  this->enableItem(IDC_LV_MOD, false);
  this->_reflist_populate();

  // update selection
  this->_reflist_selchg();

  // reset unsaved changes
  this->_set_unsaved(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repository_open()
{
  // Check and ask for unsaved changes
  switch(this->_ask_unsaved()) {
    case  1: this->_repository_save(); break; //< 'Yes'
    case -1: return;                    //< 'Cancel'
  }

  OmWString dlg_start, dlg_result;

  // if available, select current active channel library as start location
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) dlg_start = ModChan->libraryPath();

  // new dialog to open file
  if(!Om_dlgOpenFile(dlg_result, this->_hwnd, L"Open Repository definition", OM_REP_FILES_FILTER, dlg_start))
    return;

  // load repository file
  this->_repository_load(dlg_result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repository_save()
{
  // Force Save As... if empty path
  if(this->_NetRepo->path().empty()) {

    this->_repository_save_as();

    return;
  }

  this->_repository_save(this->_NetRepo->path());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repository_save_as()
{
  OmWString dlg_start, dlg_result;

  // if available, select current active channel library as start location
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) dlg_start = ModChan->libraryPath();

  // send save dialog to user
  if(!Om_dlgSaveFile(dlg_result, this->_hwnd, L"Save Repository definition", OM_REP_FILES_FILTER, OM_XML_DEF_EXT, L"default.omx", dlg_start))
    return;

  // check for ".omx" extension, add it if needed
  if(!Om_extensionMatches(dlg_result, OM_XML_DEF_EXT)) {
    dlg_result += L"." OM_XML_DEF_EXT;
  }

  this->_repository_save(dlg_result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repository_save_title()
{
  // check for unsaved changes
  OmWString ec_content;
  this->getItemText(IDC_EC_INP01, ec_content);

  if(ec_content != this->_NetRepo->title()) {

    this->_NetRepo->setTitle(ec_content);

    this->_set_unsaved(true);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repository_save_downpath()
{
  // check for unsaved changes
  OmWString ec_content;
  this->getItemText(IDC_EC_INP02, ec_content);

  Om_trim(&ec_content);

  if(!ec_content.empty()) {
    if(!Om_hasLegalUrlChar(ec_content) && !Om_isUrl(ec_content)) {
      Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_DLG_ERR, L"Invalid default download link",
                    L"Download link path or URL is invalid or contain illegal character", ec_content);
      // force focus to entry
      SetFocus(this->getItem(IDC_EC_INP02));
    }
  }

  if(ec_content != this->_NetRepo->downpath()) {

    this->_NetRepo->setDownpath(ec_content);

    this->_set_unsaved(true);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reflist_resize()
{
  LONG size[4];

  // Resize the Mods ListView column
  GetClientRect(this->getItem(IDC_LV_MOD), reinterpret_cast<LPRECT>(&size));
  this->msgItem(IDC_LV_MOD, LVM_SETCOLUMNWIDTH, 0, size[2]-2);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reflist_populate()
{
  // Save list-view scroll position to lvRect
  RECT lvRec;
  this->msgItem(IDC_LV_MOD, LVM_GETVIEWRECT, 0, reinterpret_cast<LPARAM>(&lvRec));
  // empty list view
  this->msgItem(IDC_LV_MOD, LVM_DELETEALLITEMS);

  if(this->_NetRepo->referenceCount()) {

    OmXmlNode modref;

    // add item to list view
    LVITEMW lvI = {};
    for(size_t i = 0; i < this->_NetRepo->referenceCount(); ++i) {

      modref = this->_NetRepo->getReference(i);

      lvI.iItem = static_cast<int32_t>(i);

      // Single column, entry path, if it is a file
      lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE|LVIF_TEXT|LVIF_PARAM;
      lvI.iImage = ICON_MOD_PKG;
      lvI.lParam = i;
      lvI.pszText = const_cast<LPWSTR>(modref.attrAsString(L"ident"));
      this->msgItem(IDC_LV_MOD, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
    }

    // we enable the ListView
    this->enableItem(IDC_LV_MOD, true);

    // restore ListView scroll position from lvRec
    this->msgItem(IDC_LV_MOD, LVM_SCROLL, 0, -lvRec.top );
  }

  // adapt ListView column size to client area
  this->_reflist_resize();

  // update Mods ListView selection
  this->_reflist_selchg();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reflist_selchg(int32_t item, bool selected)
{
  bool has_url = false;
  bool has_depend = false;
  bool has_thumbnail = false;
  bool has_description = false;

  if(item >= 0) {

    // check whether this an item select or unselect
    if(!selected) {

      // this is a item unselect, we save custom URL and description before switching
      this->_reference_url_changed(item);
      this->_reference_desc_changed(item);

    } else {

      // store last selected item
      this->_reflist_lastsel = item;

      OmXmlNode ref_node = this->_NetRepo->getReference(item);

      // check for custom URL
      if(ref_node.hasChild(L"url")) {
        this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 1);
        this->enableItem(IDC_EC_INP03, true);
        this->setItemText(IDC_EC_INP03, ref_node.child(L"url").content());
        has_url = true;
      }

      // check for dependencies
      if(ref_node.hasChild(L"dependencies")) {
        this->enableItem(IDC_BC_CHECK, true);
        this->enableItem(IDC_EC_READ3, true);

        OmXmlNodeArray ident_nodes;
        ref_node.child(L"dependencies").children(ident_nodes, L"ident");
        OmWString ec_entry;
        for(unsigned i = 0; i < ident_nodes.size(); ++i) {
          ec_entry += ident_nodes[i].content();
          if(i < (ident_nodes.size() - 1))
            ec_entry += L"\r\n";
        }

        this->setItemText(IDC_EC_READ3, ec_entry);
        this->setPopupItem(MNU_RE_REF, MNU_RE_REF_DEPCHK, MF_ENABLED);
        has_depend = true;
      }

      // check for thumbnail
      if(ref_node.hasChild(L"thumbnail")) {

        // decode the DataURI
        size_t jpg_size;
        OmWString mimetype, charset;
        uint8_t* jpg_data = Om_decodeDataUri(&jpg_size, mimetype, charset, ref_node.child(L"thumbnail").content());

        // load Jpeg image
        if(jpg_data) {

          uint32_t w, h;

          uint8_t* rgb_data = Om_imgLoadData(&w, &h, jpg_data, jpg_size);

          Om_free(jpg_data);

          if(rgb_data) {

            HBITMAP hBm = Om_imgEncodeHbmp(rgb_data, w, h, 4);

            Om_free(rgb_data);

            // set image to dialog
            this->enableItem(IDC_SB_SNAP, true);
            hBm = this->setStImage(IDC_SB_SNAP, hBm);
            if(hBm && hBm != Om_getResImage(IDB_SC_THMB_BLANK)) DeleteObject(hBm);

            this->enableItem(IDC_BC_RESET, true);
            this->setPopupItem(MNU_RE_REF, MNU_RE_REF_THMBDEL, MF_ENABLED);
            has_thumbnail = true;
          }
        }
      }

      // check for <remote> description
      if(ref_node.hasChild(L"description")) {

        OmXmlNode description_node = ref_node.child(L"description");

        // decode the DataURI
        size_t dfl_size;
        OmWString mimetype, charset;
        uint8_t* dfl_data = Om_decodeDataUri(&dfl_size, mimetype, charset, description_node.content());

        if(dfl_data) {

          size_t txt_size = description_node.attrAsInt(L"bytes");

          uint8_t* txt_data = Om_zInflate(dfl_data, dfl_size, txt_size);

          Om_free(dfl_data);

          if(txt_data) {

            // set text to item
            this->setItemText(IDC_EC_DESC, Om_toUTF16(reinterpret_cast<char*>(txt_data)));

            Om_free(txt_data);

            has_description = true;
          }
        }
      }
    }
  }

  // check whether we still have one selection
  bool has_select = (this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT) > 0);

  this->setPopupItem(MNU_RE_REF, MNU_RE_REF_THMBSEL, has_select ? MF_ENABLED : MF_GRAYED);
  this->setPopupItem(MNU_RE_REF, MNU_RE_REF_DESCSEL, has_select ? MF_ENABLED : MF_GRAYED);
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_FRM, has_select ? MF_ENABLED : MF_GRAYED);

  // enable or disable the Package "Remove" Button
  this->enableItem(IDC_BC_DEL, has_select);

  // enable or disable references parameters controls
  this->enableItem(IDC_BC_CKBX1, has_select);

  this->enableItem(IDC_BC_RESET, has_select);
  this->enableItem(IDC_BC_BRW03, has_select);
  this->enableItem(IDC_SB_SNAP, has_select);

  this->enableItem(IDC_BC_BRW04, has_select);
  this->enableItem(IDC_BC_SAV02, has_select);
  this->enableItem(IDC_EC_DESC, has_select);

  if(!has_url) {
    this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 0);
    this->enableItem(IDC_EC_INP03, false);
    this->setItemText(IDC_EC_INP03, L"");
  }

  if(!has_depend) {
    this->enableItem(IDC_BC_CHECK, false);
    this->enableItem(IDC_EC_READ3, false);
    this->setItemText(IDC_EC_READ3, L"");
    this->setPopupItem(MNU_RE_REF, MNU_RE_REF_DEPCHK, MF_GRAYED);
  }

  if(!has_thumbnail) {
    this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_SC_THMB_BLANK));
    this->enableItem(IDC_SB_SNAP, false);
    this->enableItem(IDC_BC_RESET, false);
    this->setPopupItem(MNU_RE_REF, MNU_RE_REF_THMBDEL, MF_GRAYED);
  }

  if(!has_description) {
    this->setItemText(IDC_EC_DESC, L"");
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_repo_add_ref(const OmWString& path, bool select)
{
  // try to parse package
  OmModPack ModPack;
  if(!ModPack.parseSource(path))
    return false;

  bool is_update = false;

  // check if reference with same identity already exists
  if(this->_NetRepo->hasReference(ModPack.iden())) {
    if(!Om_dlgBox_ynl(this->_hwnd, L"Repository Editor", IDI_DLG_QRY, L"Duplicate reference",
                     L"This Mod is already referenced, do you want to update the existing one ?", ModPack.iden()))
      return true;

    is_update = true;
  }

  // add reference to Repository
  this->_NetRepo->addReference(&ModPack);

  // append to ListBox
  if(!is_update) {
    // add item to list view
    LVITEMW lvI = {}; lvI.iItem = this->_NetRepo->referenceCount(); // insert at end of list
    // Single column, entry path, if it is a file
    lvI.iSubItem = 0; lvI.mask = LVIF_IMAGE|LVIF_TEXT;
    lvI.iImage = ICON_MOD_PKG; lvI.pszText = const_cast<LPWSTR>(ModPack.iden().c_str());
    this->msgItem(IDC_LV_MOD, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvI));
  }

  if(select) {
    // select the last ListView entry
    uint32_t lv_last = this->msgItem(IDC_LV_MOD, LVM_GETITEMCOUNT) - 1;
    LVITEMW lvI = {}; lvI.state = LVIS_SELECTED;
    this->msgItem(IDC_LV_MOD, LVM_SETITEMSTATE, lv_last, reinterpret_cast<LPARAM>(&lvI));
  }

  // update status bar
  this->_status_update_references();

  // set unsaved changes
  this->_set_unsaved(true);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_repo_del_ref()
{
  // check for ListView selection
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return false;

  // get single selection index
  int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  this->_NetRepo->deleteReference(lv_sel);

  // delete ListView item
  this->msgItem(IDC_LV_MOD, LVM_DELETEITEM, lv_sel);

  // update reference parameters controls
  this->_reflist_selchg();

  // update status bar
  this->_status_update_references();

  // set unsaved changes
  this->_set_unsaved(true);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reference_url_toggle()
{
  if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {

    this->enableItem(IDC_EC_INP03, true);

  } else {

    this->setItemText(IDC_EC_INP03, L"");

    this->_reference_url_changed();
    this->enableItem(IDC_EC_INP03, false);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reference_url_changed(int32_t item)
{
  int32_t lv_sel;

  if(item < 0) {
    // check for ListView selection
    if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
      return;
    // get single selection index
    lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  } else {
    lv_sel = item;
  }

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lv_sel);

  bool has_changes = false;

  if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {

    OmWString ec_content;

    this->getItemText(IDC_EC_INP03, ec_content);

    Om_trim(&ec_content);

    if(ec_content.empty()) {
      if(ref_node.hasChild(L"url")) {
        ref_node.remChild(L"url");
        has_changes = true;
      }
    } else {

      if(!Om_hasLegalUrlChar(ec_content) && !Om_isUrl(ec_content)) {
        Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_DLG_ERR, L"Invalid custom download link",
                      L"Download link path or URL is invalid or contain illegal character", ec_content);
        // force ListView selection and focus to entry
        LVITEMW lvI = {}; lvI.state = LVIS_SELECTED;
        this->msgItem(IDC_LV_MOD, LVM_SETITEMSTATE, lv_sel, reinterpret_cast<LPARAM>(&lvI));
        SetFocus(this->getItem(IDC_EC_INP03));
      }

      if(ref_node.hasChild(L"url")) {
        if(ec_content != ref_node.child(L"url").content()) {
          ref_node.child(L"url").setContent(ec_content);
          has_changes = true;
        }
      } else {
        ref_node.addChild(L"url").setContent(ec_content);
        has_changes = true;
      }

    }
  } else {
    if(ref_node.hasChild(L"url")) {
      ref_node.remChild(L"url");
      has_changes = true;
    }
  }

  if(has_changes)
    this->_set_unsaved(true);

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reference_desc_load()
{
  // check for ListView selection
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  // get single selection index
  int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lv_sel);

  OmWString open_start, open_result;

  // if available, select current active channel library as start location
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) open_start = ModChan->libraryPath();

  // new dialog to open file
  if(!Om_dlgOpenFile(open_result, this->_hwnd, L"Open text file", OM_TXT_FILES_FILTER, open_start))
    return;

  // set loaded text as description
  OmCString text = Om_toCRLF(Om_loadPlainText(open_result));
  SetDlgItemTextA(this->_hwnd, IDC_EC_DESC, text.c_str());

  // save description
  this->_reference_desc_changed();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reference_desc_changed(int32_t item)
{
  int32_t lv_sel;

  if(item < 0) {
    // check for ListView selection
    if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
      return;
    // get single selection index
    lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
  } else {
    lv_sel = item;
  }

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lv_sel);

  // get current reference description data URI
  OmWString ref_uri;
  if(ref_node.hasChild(L"description"))
    ref_uri = ref_node.child(L"description").content();

  bool has_changes = false;

  // get current description text
  OmWString ec_content;
  this->getItemText(IDC_EC_DESC, ec_content);

  // immediately check case where everything is empty
  if(ec_content.empty()) {

    if(ref_node.hasChild(L"description")) {
      ref_node.remChild(L"description");
      has_changes = true;
    }

  } else {

    OmCString utf8 = Om_toUTF8(ec_content);

    // text buffer size with null char
    size_t txt_size = utf8.size() + 1;

    // compress data using defalte
    size_t dfl_size;
    uint8_t* dfl_data = Om_zDeflate(&dfl_size, reinterpret_cast<const uint8_t*>(utf8.c_str()), txt_size, 9);

    if(dfl_data) {

      // encode raw data to data URI Base64
      OmWString data_uri;
      Om_encodeDataUri(data_uri, L"application/octet-stream", L"", dfl_data, dfl_size);
      // free deflated data
      Om_free(dfl_data);

      OmXmlNode description_node;

      if(ref_node.hasChild(L"description")) {
        description_node = ref_node.child(L"description");
        // compare deflated size
        if(txt_size != static_cast<uint32_t>(description_node.attrAsInt(L"bytes"))) {
          has_changes = true;
        } else {
          // compare data URI strings
          if(data_uri != description_node.content())
            has_changes = true;
        }
      } else {
        description_node = ref_node.addChild(L"description");
        has_changes = true;
      }

      if(has_changes) {
        description_node.setAttr(L"bytes", static_cast<int>(txt_size));
        description_node.setContent(data_uri);
      }
    }
  }

  if(has_changes)
    this->_set_unsaved(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reference_thumb_load()
{
  // check for ListView selection
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  // get single selection index
  int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lv_sel);

  OmWString open_start, open_result;

  // if available, select current active channel library as start location
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) open_start = ModChan->libraryPath();

  // new dialog to open file
  if(!Om_dlgOpenFile(open_result, this->_hwnd, L"Open image file", OM_IMG_FILES_FILTER, open_start))
    return;

  OmImage image;

  bool has_changes = false;

  // try to load image file
  if(image.loadThumbnail(open_result, OM_MODPACK_THUMB_SIZE, OM_SIZE_FILL)) {

    // Encode RGBA to JPEG
    uint64_t jpg_size;
    uint8_t* jpg_data = Om_imgEncodeJpg(&jpg_size, image.data(), image.width(), image.height(), image.bpp());

    if(jpg_data) {
      // format jpeg to base64 encoded data URI
      OmWString data_uri;
      Om_encodeDataUri(data_uri, L"image/jpeg", L"", jpg_data, jpg_size);

      // set node content to data URI string
      if(ref_node.hasChild(L"thumbnail")) {
        ref_node.child(L"thumbnail").setContent(data_uri);
      } else {
        ref_node.addChild(L"thumbnail").setContent(data_uri);
      }

      has_changes = true;

      // free allocated data
      Om_free(jpg_data);

      // set thumbnail bitmap to static control
      HBITMAP hBm = this->setStImage(IDC_SB_SNAP, image.hbmp());
      if(hBm && hBm != Om_getResImage(IDB_SC_THMB_BLANK)) DeleteObject(hBm);

      // enable Snapshot "Delete" Button
      this->enableItem(IDC_BC_RESET, true);
    }
  }

  if(has_changes)
    this->_set_unsaved(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reference_thumb_delete()
{
  // check for ListView selection
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  // get single selection index
  int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lv_sel);

  bool has_changes = false;

  // set node content to data URI string
  if(ref_node.hasChild(L"thumbnail")) {

    ref_node.remChild(L"thumbnail");
    has_changes = true;

    // set thumbnail placeholder image
    HBITMAP hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_SC_THMB_BLANK));
    if(hBm && hBm != Om_getResImage(IDB_SC_THMB_BLANK)) DeleteObject(hBm);
  }

  if(has_changes)
    this->_set_unsaved(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reference_deps_get(const OmWString& iden, OmWStringArray* missings)
{
  // retrieve reference from identity
  int32_t ref_index = this->_NetRepo->indexOfReference(iden);
  if(ref_index < 0) return;

  OmXmlNode ref_node = this->_NetRepo->getReference(ref_index);

  // get dependencies list
  if(ref_node.hasChild(L"dependencies")) {

    OmXmlNodeArray ident_nodes;
    ref_node.child(L"dependencies").children(ident_nodes, L"ident");

    OmWString dep_iden;

    for(size_t i = 0; i < ident_nodes.size(); ++i) {

      dep_iden = ident_nodes[i].content();

      int32_t dep_index = this->_NetRepo->indexOfReference(dep_iden);
      if(dep_index < 0) {
        Om_push_backUnique(*missings, dep_iden);
        continue;
      }

      // recursive check
      this->_reference_deps_get(dep_iden, missings);
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reference_deps_check()
{
  // check for ListView selection
  if(!this->msgItem(IDC_LV_MOD, LVM_GETSELECTEDCOUNT))
    return;

  // get single selection index
  int32_t lv_sel = this->msgItem(IDC_LV_MOD, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lv_sel);

  OmWStringArray missings;

  // recursive check for dependencies within the repository references
  this->_reference_deps_get(ref_node.attrAsString(L"ident"), &missings);

  if(!missings.empty()) {

    Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_DLG_WRN, L"Missing dependencies",
                  L"The following declared Mod dependencies are not referenced in the repository",
                  Om_concatStrings(missings, L"\r\n"));
  } else {

    Om_dlgBox_ok(this->_hwnd, L"Repository Editor", IDI_DLG_NFO, L"Satisfied dependencies ",
                 L"All declared Mod dependencies are referenced in the repository");
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reflist_add_files()
{
  // if available, select current active channel library as start location
  OmWString start;
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) start = ModChan->libraryPath();

  // new dialog to open file (allow multiple selection)
  OmWStringArray result;
  if(!Om_dlgOpenFileMultiple(result, this->_hwnd, L"Open Mod-Package(s)", OM_PKG_FILES_FILTER, start))
    return;

  // run add list thread
  this->_reflist_add_start(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reflist_add_directory()
{
  // if available, select current active channel library as start location
  OmWString start;
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) start = ModChan->libraryPath();

  // open dialog to select folder
  OmWString result;
  if(!Om_dlgOpenDir(result, this->_hwnd, L"Select directory with Mod packages to add", start))
    return;

  // get list of potential Mods
  OmWStringArray paths;
  Om_lsFileFiltered(&paths, result, L"*.zip", true, true);
  Om_lsFileFiltered(&paths, result, L"*." OM_PKG_FILE_EXT, true, true);

  // run add list thread
  this->_reflist_add_start(paths);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_reflist_add_start(const OmWStringArray& paths)
{
  // add paths to queue
  for(size_t i = 0; i < paths.size(); ++i)
    this->_reflist_add_queue.push_back(paths[i]);

  // launch thread if not already running
  if(!this->_reflist_add_hth) {

    // disable buttons
    this->enableItem(IDC_BC_BRW01, false);
    this->enableItem(IDC_BC_BRW02, false);

    this->_reflist_add_abort = 0;

    // launch thread
    this->_reflist_add_hth = Om_threadCreate(OmUiToolRep::_reflist_add_run_fn, this);
    this->_reflist_add_hwo = Om_threadWaitEnd(this->_reflist_add_hth, OmUiToolRep::_reflist_add_end_fn, this);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiToolRep::_reflist_add_run_fn(void* ptr)
{
  OmUiToolRep* self = static_cast<OmUiToolRep*>(ptr);

  // Open progress dialog
  self->_reflist_add_abort = 0;
  self->_reflist_add_hpd = Om_dlgProgress(self->_hwnd, L"add Mod references", IDI_DLG_PKG_BLD, L"Parsing Mod packages", &self->_reflist_add_abort);

  // stuff for progress dialog
  OmWString progress_text;
  int32_t progress_tot = self->_reflist_add_queue.size();
  int32_t progress_cur = 0;

  OmWString file_path;

  // try to add each file, silently fail
  while(self->_reflist_add_queue.size()) {

    // check for abort
    if(self->_reflist_add_abort != 0)
      break;

    // get next file to proceed
    file_path = self->_reflist_add_queue.front();

    // update progress text
    progress_text = L"Computing checksum: ";
    progress_text += Om_getFilePart(file_path);
    Om_dlgProgressUpdate(static_cast<HWND>(self->_reflist_add_hpd), -1, -1, progress_text.c_str());

    // proceed this file
    self->_repo_add_ref(file_path);

    // update progress bar
    Om_dlgProgressUpdate(static_cast<HWND>(self->_reflist_add_hpd), progress_tot, ++progress_cur, nullptr);

    #ifdef DEBUG
    Sleep(50); //< for debug
    #endif

    self->_reflist_add_queue.pop_front();
  }

  // quit the progress dialog (dialogs must be opened and closed within the same thread)
  Om_dlgProgressClose(static_cast<HWND>(self->_reflist_add_hpd));
  self->_reflist_add_hpd = nullptr;

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmUiToolRep::_reflist_add_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  OmUiToolRep* self = static_cast<OmUiToolRep*>(ptr);

  Om_threadClear(self->_reflist_add_hth, self->_reflist_add_hwo);

  self->_reflist_add_hth = nullptr;
  self->_reflist_add_hwo = nullptr;

  self->_reflist_add_queue.clear();

  // enable buttons
  self->enableItem(IDC_BC_BRW01, true);
  self->enableItem(IDC_BC_BRW02, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onClose()
{
  switch(this->_ask_unsaved()) {
    case  1: this->_repository_save(); break; //< 'Yes'
    case -1: return;                    //< 'Cancel'
  }

  // quit this dialog
  this->quit();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(IDI_BT_TOOLREP,2),Om_getResIcon(IDI_BT_TOOLREP,1));

  // dialog is modeless so we set dialog title with app name
  this->setCaption(L"Repository editor ");

  // Set font for description
  HFONT hFt = Om_createFont(14, 400, L"Consolas");
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  // Create the toolbar.
  CreateWindowExW(WS_EX_LEFT, TOOLBARCLASSNAMEW, nullptr, WS_CHILD|TBSTYLE_WRAPABLE|TBSTYLE_TOOLTIPS, 0, 0, 0, 0,
                  this->_hwnd, reinterpret_cast<HMENU>(IDC_TB_TOOLS), this->_hins, nullptr);

  HIMAGELIST himl = static_cast<OmUiMan*>(this->root())->toolBarsImgList();
  this->msgItem(IDC_TB_TOOLS, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(himl));

  // Initialize button info.
  TBBUTTON tbButtons[3] = {
    {ICON_NEW, IDC_BC_NEW,  TBSTATE_ENABLED, 0/*BTNS_AUTOSIZE*/, {0}, 0, reinterpret_cast<INT_PTR>("New repository")},
    {ICON_OPN, IDC_BC_OPEN, TBSTATE_ENABLED, 0/*BTNS_AUTOSIZE*/, {0}, 0, reinterpret_cast<INT_PTR>("Open definition file")},
    {ICON_SAV, IDC_BC_SAVE, 0,               0/*BTNS_AUTOSIZE*/, {0}, 0, reinterpret_cast<INT_PTR>("Save definition file")}
  };

  // Add buttons
  this->msgItem(IDC_TB_TOOLS, TB_SETMAXTEXTROWS, 0); //< disable text under buttons
  this->msgItem(IDC_TB_TOOLS, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON));
  this->msgItem(IDC_TB_TOOLS, TB_ADDBUTTONS, 3, reinterpret_cast<LPARAM>(&tbButtons));

  this->msgItem(IDC_TB_TOOLS, TB_SETBUTTONSIZE, 0, MAKELPARAM(26, 22));
  //this->msgItem(IDC_TB_TOOLS, TB_SETLISTGAP, 25);                       //< this does not work
  //this->msgItem(IDC_TB_TOOLS, TB_SETPADDING, 0, MAKELPARAM(25, 20));    //< this does not work

  // Resize and show the toolbar
  this->msgItem(IDC_TB_TOOLS, TB_AUTOSIZE);
  this->showItem(IDC_TB_TOOLS, true);

  // Shared Image list for ListView controls
  himl = static_cast<OmUiMan*>(this->root())->listViewImgList();

  // Initialize Mod content ListView control with explorer theme
  this->msgItem(IDC_LV_MOD, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_DOUBLEBUFFER);
  SetWindowTheme(this->getItem(IDC_LV_MOD),L"EXPLORER",nullptr);
  // add column into ListView
  LVCOLUMNW lvC = {}; lvC.mask = LVCF_WIDTH|LVCF_FMT;
  // Single column for paths
  lvC.fmt = LVCFMT_LEFT;
  lvC.iSubItem = 0; lvC.cx = 300;
  this->msgItem(IDC_LV_MOD, LVM_INSERTCOLUMNW, lvC.iSubItem, reinterpret_cast<LPARAM>(&lvC));
  // set shared ImageList
  this->msgItem(IDC_LV_MOD, LVM_SETIMAGELIST, LVSIL_SMALL, reinterpret_cast<LPARAM>(himl));
  this->msgItem(IDC_LV_MOD, LVM_SETIMAGELIST, LVSIL_NORMAL, reinterpret_cast<LPARAM>(himl));

  // Set default package picture
  this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_SC_THMB_BLANK));

  // Set buttons icons
  this->setBmIcon(IDC_BC_BRW01, Om_getResIcon(IDI_BT_FAD));
  this->setBmIcon(IDC_BC_BRW02, Om_getResIcon(IDI_BT_DAD));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(IDI_BT_FRM));
  this->setBmIcon(IDC_BC_BRW03, Om_getResIcon(IDI_BT_ADD_IMG)); //< Thumbnail Select
  this->setBmIcon(IDC_BC_RESET, Om_getResIcon(IDI_BT_REM)); //< Thumbnail Erase
  //this->setBmIcon(IDC_BC_SAV02, Om_getResIcon(IDI_BT_SVD)); //< Description Save
  this->setBmIcon(IDC_BC_BRW04, Om_getResIcon(IDI_BT_ADD_TXT)); //< Description Load
  this->setBmIcon(IDC_BC_CHECK, Om_getResIcon(IDI_BT_DPN));

  // Set menu icons
  HMENU hMnuFile = this->getPopup(MNU_RE_FILE);
  this->setPopupItemIcon(hMnuFile, MNU_RE_FILE_NEW, Om_getResIconPremult(IDI_BT_NEW));
  this->setPopupItemIcon(hMnuFile, MNU_RE_FILE_OPEN, Om_getResIconPremult(IDI_BT_OPN));
  this->setPopupItemIcon(hMnuFile, MNU_RE_FILE_SAVE, Om_getResIconPremult(IDI_BT_SAV));
  this->setPopupItemIcon(hMnuFile, MNU_RE_FILE_SAVAS, Om_getResIconPremult(IDI_BT_SVA));
  this->setPopupItemIcon(hMnuFile, MNU_RE_FILE_QUIT, Om_getResIconPremult(IDI_BT_EXI));

  HMENU hMnuEdit = this->getPopup(MNU_RE_EDIT);
  this->setPopupItemIcon(hMnuEdit, MNU_RE_EDIT_FAD, Om_getResIconPremult(IDI_BT_FAD));
  this->setPopupItemIcon(hMnuEdit, MNU_RE_EDIT_DAD, Om_getResIconPremult(IDI_BT_DAD));
  this->setPopupItemIcon(hMnuEdit, MNU_RE_EDIT_FRM, Om_getResIconPremult(IDI_BT_FRM));

  HMENU hMnuRef = this->getPopup(MNU_RE_REF);
  this->setPopupItemIcon(hMnuRef, MNU_RE_REF_THMBSEL, Om_getResIconPremult(IDI_BT_ADD_IMG));
  this->setPopupItemIcon(hMnuRef, MNU_RE_REF_THMBDEL, Om_getResIconPremult(IDI_BT_REM));
  this->setPopupItemIcon(hMnuRef, MNU_RE_REF_DESCSEL, Om_getResIconPremult(IDI_BT_ADD_TXT));
  this->setPopupItemIcon(hMnuRef, MNU_RE_REF_DEPCHK, Om_getResIconPremult(IDI_BT_DPN));

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Repository short description");
  this->_createTooltip(IDC_EC_INP02,  L"Relative or absolute link to downloadable files");

  this->_createTooltip(IDC_LV_MOD,    L"Repository referenced Mod list");

  this->_createTooltip(IDC_BC_BRW01,  L"Add from files");
  this->_createTooltip(IDC_BC_BRW02,  L"Add from directory");
  this->_createTooltip(IDC_BC_DEL,    L"Delete reference");

  this->_createTooltip(IDC_BC_CKBX1,  L"Use custom download link");
  this->_createTooltip(IDC_EC_INP03,  L"Relative or absolute link to downloadable file");

  this->_createTooltip(IDC_BC_BRW03,  L"Load thumbnail image");
  this->_createTooltip(IDC_BC_RESET,  L"Delete thumbnail image");

  this->_createTooltip(IDC_BC_BRW04,  L"Load text file");
  this->_createTooltip(IDC_EC_DESC,   L"Description");

  this->_createTooltip(IDC_BC_CHECK,  L"Check for dependencies availability");

  // disable/enable proper menu items
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_FAD, MF_GRAYED);
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_DAD, MF_GRAYED);
  this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVE, MF_GRAYED);
  this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVAS, MF_GRAYED);

  // update selection to enable menu/buttons
  this->_reflist_selchg();

  // Initialize new Repository definition XML scheme
  //this->_repository_init();

  // Load initial repository
  if(!this->_init_path.empty()) {

    this->_repository_load(this->_init_path);

    this->_init_path.clear();

  } else {

    // reset unsaved changes
    this->_set_unsaved(false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onResize()
{
  int32_t half_w = this->cliWidth() * 0.5f;
  int32_t base_y = 33;
  int32_t foot_y = this->cliHeight() - (base_y+28);

  // resize the toolbar
  this->msgItem(IDC_TB_TOOLS, TB_AUTOSIZE);

  // toolbar separator
  this->_setItemPos(IDC_SC_SEPAR, -1, 28, this->cliWidth()+2, 1, true);

  // -- Left Frame --

  // [ - - -       Repository config GroupBox        - - -
  this->_setItemPos(IDC_GB_GRP01, 5, base_y, half_w-8, foot_y, true);
  // Title Label & EditText
  this->_setItemPos(IDC_SC_LBL01, 10, base_y+30, 200, 16, true);
  this->_setItemPos(IDC_EC_INP01, 10, base_y+50, half_w-20, 21, true);

  // Download path Label & EditText
  this->_setItemPos(IDC_SC_LBL02, 10, base_y+90, 200, 16, true);
  this->_setItemPos(IDC_EC_INP02, 10, base_y+110, half_w-20, 21, true);

  // Referenced Mods label
  this->_setItemPos(IDC_SC_LBL03, 10, base_y+150, 200, 16, true);
  // Referenced Mods Actions buttons
  this->_setItemPos(IDC_BC_BRW01, half_w-78, base_y+145, 22, 22, true);
  this->_setItemPos(IDC_BC_BRW02, half_w-55, base_y+145, 22, 22, true);
  this->_setItemPos(IDC_BC_DEL,   half_w-32, base_y+145, 22, 22, true);
  // Referenced Mods list
  this->_setItemPos(IDC_LV_MOD, 10, base_y+168, half_w-20, (foot_y-179), true);
  this->_reflist_resize();
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]


  // -- Right Frame --

  // [ - - -     Package Name & Url GroupBox      - - -
  this->_setItemPos(IDC_GB_GRP02, half_w+3, base_y, half_w-8, foot_y, true);

  // Custom Url CheckBox & EditText
  this->_setItemPos(IDC_BC_CKBX1, half_w+10, base_y+30, 200, 16, true);
  this->_setItemPos(IDC_EC_INP03, half_w+10, base_y+50, half_w-20, 21, true);
  // Custom url Action button
  //this->_setItemPos(IDC_BC_SAV01, this->cliWidth()-50, this->cliUnitY()-48, 22, 22, true);

  // Thumbnail Label
  this->_setItemPos(IDC_SC_LBL04, half_w+10, base_y+90, 90, 16, true);
    // Thumbnail notice text
  this->_setItemPos(IDC_SC_NOTES, half_w+110, base_y+90, 150, 16, true);
  // Thumbnail Action buttons
  this->_setItemPos(IDC_BC_RESET, this->cliWidth()-55, base_y+87, 22, 22, true);
  this->_setItemPos(IDC_BC_BRW03, this->cliWidth()-32, base_y+87, 22, 22, true);
  // Thumbnail static bitmap
  this->_setItemPos(IDC_SB_SNAP, half_w+10, base_y+110, 128, 128, true);

  // Description label
  this->_setItemPos(IDC_SC_LBL05, half_w+10, base_y+260, 90, 16, true);
  // Description Actions buttons
  this->_setItemPos(IDC_BC_BRW04, this->cliWidth()-32, base_y+257, 22, 22, true);
  // Description EditText
  this->_setItemPos(IDC_EC_DESC, half_w+10, base_y+280, half_w-21, foot_y-(290+85), true);

  // Dependencies Label
  this->_setItemPos(IDC_SC_LBL06, half_w+10, foot_y-45, 100, 16, true);
  // Dependencies Check button
  this->_setItemPos(IDC_BC_CHECK, this->cliWidth()-32, foot_y-48, 22, 22, true);
  // Dependencies EditText & Check Button
  this->_setItemPos(IDC_EC_READ3, half_w+10, foot_y-25, half_w-20, 48, true);

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
INT_PTR OmUiToolRep::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_NOTIFY) {

    if(LOWORD(wParam) == IDC_LV_MOD) {
      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      /*
      case NM_DBLCLK:
        break;

      case NM_RCLICK:
        break;
      */
      case LVN_ITEMCHANGED: {
          NMLISTVIEW* nmLv = reinterpret_cast<NMLISTVIEW*>(lParam);
          // detect only selection changes
          if((nmLv->uNewState ^ nmLv->uOldState) & LVIS_SELECTED)
            this->_reflist_selchg(nmLv->iItem, (nmLv->uNewState & LVIS_SELECTED));
          break;
        }
      }
    }

    return false;
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_NEW:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_repository_init();
      break;

    case IDC_BC_OPEN:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_repository_open();
      break;

    case IDC_BC_SAVE: //< General "Save as.." Button
      this->_repository_save();
      break;

    case IDC_EC_INP01: //< Repository title EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE) {
        // check for unsaved changes
        OmWString ec_content;
        this->getItemText(IDC_EC_INP01, ec_content);

        if(ec_content != this->_NetRepo->title())
          this->_set_unsaved(true);
      }
      if(HIWORD(wParam) == EN_KILLFOCUS)
        this->_repository_save_title();
      break;

    case IDC_EC_INP02: //< Download path EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE) {
        // check for unsaved changes
        OmWString ec_content;
        this->getItemText(IDC_EC_INP02, ec_content);

        if(ec_content != this->_NetRepo->downpath())
          this->_set_unsaved(true);
      }
      if(HIWORD(wParam) == EN_KILLFOCUS)
        this->_repository_save_downpath();
      break;

    case IDC_BC_BRW01: //< Button : Mod references Add Files
      if(HIWORD(wParam) == BN_CLICKED)
        this->_reflist_add_files();
      break;

    case IDC_BC_BRW02: //< Button : Mod references Add Directory
      if(HIWORD(wParam) == BN_CLICKED)
        this->_reflist_add_directory();
      break;

    case IDC_BC_DEL:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_repo_del_ref();
      break;

    case IDC_BC_CKBX1: // Custom Url CheckBox
      if(HIWORD(wParam) == BN_CLICKED)
        this->_reference_url_toggle();
      break;

    case IDC_EC_INP03: //< Custon URL EditText
      //if(HIWORD(wParam) == EN_KILLFOCUS)
      if(HIWORD(wParam) == EN_CHANGE)
        this->_reference_url_changed();
      break;

    case IDC_BC_BRW03: //< Thumbnail "Select..." Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_reference_thumb_load();
      break;

    case IDC_BC_RESET: //< Thumbnail "Delete" Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_reference_thumb_delete();
      break;

    case IDC_BC_BRW04: //< Description "Load.." Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_reference_desc_load();
      break;

    case IDC_EC_DESC: //< Description EditText
      // check for content changes
      //if(HIWORD(wParam) == EN_KILLFOCUS)
      if(HIWORD(wParam) == EN_CHANGE)
        this->_reference_desc_changed();
      break;

    case IDC_BC_CHECK: //< Dependencies "Check" Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_reference_deps_check();
      break;

    case IDM_FILE_NEW:
      this->_repository_init();
      break;

    case IDM_FILE_OPEN:
      this->_repository_open();
      break;

    case IDM_FILE_SAVE:
      this->_repository_save();
      break;

    case IDM_FILE_SAVAS:
      this->_repository_save_as();
      break;

    case IDM_QUIT:
      this->_onClose();
      break;

    case IDM_ENTRY_FADD:
      this->_reflist_add_files();
      break;

    case IDM_ENTRY_DADD:
      this->_reflist_add_directory();
      break;

    case IDM_ENTRY_DEL:
      this->_repo_del_ref();
      break;

    case IDM_THMB_SEL:
      this->_reference_thumb_load();
      break;

    case IDM_THMB_DEL:
      this->_reference_thumb_delete();
      break;

    case IDM_DESC_SEL:
      this->_reference_desc_load();
      break;

    case IDM_DEP_CHK:
      this->_reference_deps_check();
      break;
    }
  }

  return false;
}

