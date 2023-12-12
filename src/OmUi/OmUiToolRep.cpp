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
#include <ShlObj.h>

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
  _xml(),
  _NetRepo(new OmNetRepo(nullptr)),
  _has_unsaved(false),
  _repo_addlist_abort(0),
  _repo_addlist_hth(nullptr),
  _repo_addlist_hwo(nullptr),
  _repo_addlist_hpd(nullptr)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolRep::~OmUiToolRep()
{
  if(this->_NetRepo)
    delete this->_NetRepo;

  HBITMAP hBm = this->setStImage(IDC_SB_SNAP, nullptr);
  if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);

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
void OmUiToolRep::_repo_init()
{
  // Check and ask for unsaved changes
  switch(this->_ask_unsaved()) {
    case  1: this->_repo_save(); break; //< 'Yes'
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

  // empty referenced Mods ListBox
  this->enableItem(IDC_LB_MOD, true);
  this->msgItem(IDC_LB_MOD, LB_RESETCONTENT, 0, 0);

  // update selection
  this->_ref_selected();

  // reset unsaved changes
  this->_set_unsaved(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repo_open()
{
  // Check and ask for unsaved changes
  switch(this->_ask_unsaved()) {
    case  1: this->_repo_save(); break; //< 'Yes'
    case -1: return;                    //< 'Cancel'
  }

  OmWString dlg_start, dlg_result;

  // if available, select current active channel library as start location
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) dlg_start = ModChan->libraryPath();

  // new dialog to open file
  if(!Om_dlgOpenFile(dlg_result, this->_hwnd, L"Open Repository definition", OM_XML_FILES_FILTER, dlg_start))
    return;

  // (try) load and parse repository XML file
  OmResult result = this->_NetRepo->load(dlg_result);
  if(result != OM_RESULT_OK) {
    Om_dlgBox_okl(this->_hwnd, L"Repository editor", IDI_ERR, L"Repository open error",
                  L"Unable to load Repository definition:", this->_NetRepo->lastError());
    return;
  }

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

  // populate referenced Mods ListBox
  this->enableItem(IDC_LB_MOD, true);
  this->msgItem(IDC_LB_MOD, LB_RESETCONTENT, 0, 0); //< empty ListBox

  OmXmlNode modref;

  for(size_t i = 0; i < this->_NetRepo->referenceCount(); ++i) {
    modref = this->_NetRepo->getReference(i);
    this->msgItem(IDC_LB_MOD, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(modref.attrAsString(L"ident")));
  }

  // update selection
  this->_ref_selected();

  // reset unsaved changes
  this->_set_unsaved(false);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repo_close()
{
  // Check and ask for unsaved changes
  switch(this->_ask_unsaved()) {
    case  1: this->_repo_save(); break; //< 'Yes'
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

    // empty referenced Mods ListBox
  this->msgItem(IDC_LB_MOD, LB_RESETCONTENT, 0, 0);
  this->enableItem(IDC_LB_MOD, false);

  // update selection
  this->_ref_selected();

  // reset unsaved changes
  this->_set_unsaved(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repo_save()
{
  // Force Save As... if empty path
  if(this->_NetRepo->path().empty()) {

    this->_repo_save_as();

    return;
  }

  OmResult result = this->_NetRepo->save(this->_NetRepo->path());
  if(result != OM_RESULT_OK) {
    Om_dlgBox_okl(this->_hwnd, L"Repository editor", IDI_ERR, L"Save file error",
                  L"Unable to save file:", this->_NetRepo->lastError());
  }

  // update status bar and caption
  this->_status_update_filename();

  // changes now saved
  this->_set_unsaved(false);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repo_save_as()
{
  OmWString dlg_start, dlg_result;

  // if available, select current active channel library as start location
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) dlg_start = ModChan->libraryPath();

  // send save dialog to user
  if(!Om_dlgSaveFile(dlg_result, this->_hwnd, L"Save Repository definition", OM_XML_FILES_FILTER, L"xml", L"default.xml", dlg_start))
    return;

  // check for ".xml" extension, add it if needed
  if(!Om_extensionMatches(dlg_result, L"xml")) {
    dlg_result += L".xml";
  }

  // ask user for overwirte
  if(!Om_dlgOverwriteFile(this->_hwnd, dlg_result))
    return;

  OmResult result = this->_NetRepo->save(dlg_result);
  if(result != OM_RESULT_OK) {
    Om_dlgBox_okl(this->_hwnd, L"Repository editor", IDI_ERR, L"Save file error",
                  L"Unable to save file:", this->_NetRepo->lastError());
    return;
  }

  // update status bar and caption
  this->_status_update_filename();

  // changes now saved
  this->_set_unsaved(false);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repo_save_title()
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
void OmUiToolRep::_repo_save_downpath()
{
  // check for unsaved changes
  OmWString ec_content;
  this->getItemText(IDC_EC_INP02, ec_content);

  Om_trim(&ec_content);

  if(!ec_content.empty()) {
    if(!Om_hasLegalUrlChar(ec_content) && !Om_isUrl(ec_content)) {
      Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_ERR, L"Invalid default download link",
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
bool OmUiToolRep::_repo_add_ref(const OmWString& path, bool select)
{
  // try to parse package
  OmModPack ModPack;
  if(!ModPack.parseSource(path))
    return false;

  bool is_update = false;

  // check if reference with same identity already exists
  if(this->_NetRepo->hasReference(ModPack.iden())) {
    if(!Om_dlgBox_ynl(this->_hwnd, L"Repository Editor", IDI_QRY, L"Duplicate reference",
                     L"This Mod is already referenced, do you want to update the existing one ?", ModPack.iden()))
      return true;

    is_update = true;
  }

  // add reference to Repository
  this->_NetRepo->addReference(&ModPack);

  // append to ListBox
  if(!is_update)
    this->msgItem(IDC_LB_MOD, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ModPack.iden().c_str()));

  if(select) {
    // select the last ListBox entry
    uint32_t lb_count = this->msgItem(IDC_LB_MOD, LB_GETCOUNT);
    this->msgItem(IDC_LB_MOD, LB_SETSEL, true, lb_count - 1);
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
  // get ListBox current selection
  int32_t lb_sel = this->msgItem(IDC_LB_MOD, LB_GETCURSEL);
  if(lb_sel < 0) return false;

  this->_NetRepo->deleteReference(lb_sel);

  // unselect and remove ListBox entry
  this->msgItem(IDC_LB_MOD, LB_SETSEL, true, -1);
  this->msgItem(IDC_LB_MOD, LB_DELETESTRING, lb_sel);

  // update reference parameters controls
  this->_ref_selected();

  // update status bar
  this->_status_update_references();

  // set unsaved changes
  this->_set_unsaved(true);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_ref_selected()
{
  // get ListBox current selection
  int32_t lb_sel = this->msgItem(IDC_LB_MOD, LB_GETCURSEL);
  bool has_select = (lb_sel >= 0);

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

  bool has_cust_url = false;
  bool has_depend = false;
  bool has_thumbnail = false;
  bool has_description = false;

  if(has_select) {

    OmXmlNode ref_node = this->_NetRepo->getReference(lb_sel);

    // check for custom URL
    if(ref_node.hasChild(L"url")) {
      this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, 1);
      this->enableItem(IDC_EC_INP03, true);
      this->setItemText(IDC_EC_INP03, ref_node.child(L"url").content());
      has_cust_url = true;
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
    if(ref_node.hasChild(L"picture")) {

      // decode the DataURI
      size_t jpg_size;
      OmWString mimetype, charset;
      uint8_t* jpg_data = Om_decodeDataUri(&jpg_size, mimetype, charset, ref_node.child(L"picture").content());

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
          if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);

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

  if(!has_cust_url) {
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
    this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_BLANK));
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
void OmUiToolRep::_ref_url_save()
{
  // get ListBox current selection
  int32_t lb_sel = this->msgItem(IDC_LB_MOD, LB_GETCURSEL);
  if(lb_sel < 0) return;

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lb_sel);

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
        Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_ERR, L"Invalid custom download link",
                      L"Download link path or URL is invalid or contain illegal character", ec_content);
        // force ListBox selection and focus to entry
        this->msgItem(IDC_LB_MOD, LB_SETCURSEL, lb_sel);
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
void OmUiToolRep::_ref_desc_load()
{
  // get ListBox current selection
  int32_t lb_sel = this->msgItem(IDC_LB_MOD, LB_GETCURSEL);
  if(lb_sel < 0) return;

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lb_sel);

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
  this->_ref_desc_save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_ref_desc_save()
{
  // get ListBox current selection
  int32_t lb_sel = this->msgItem(IDC_LB_MOD, LB_GETCURSEL);
  if(lb_sel < 0) return;

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lb_sel);

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
        if(txt_size != description_node.attrAsInt(L"bytes")) {
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
void OmUiToolRep::_ref_thumb_set()
{
  // get ListBox current selection
  int32_t lb_sel = this->msgItem(IDC_LB_MOD, LB_GETCURSEL);
  if(lb_sel < 0) return;

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lb_sel);

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
    uint8_t* jpg_data = Om_imgEncodeJpg(&jpg_size, image.data(), image.width(), image.height(), 4, 7);

    if(jpg_data) {
      // format jpeg to base64 encoded data URI
      OmWString data_uri;
      Om_encodeDataUri(data_uri, L"image/jpeg", L"", jpg_data, jpg_size);

      // set node content to data URI string
      if(ref_node.hasChild(L"picture")) {
        ref_node.child(L"picture").setContent(data_uri);
      } else {
        ref_node.addChild(L"picture").setContent(data_uri);
      }

      has_changes = true;

      // free allocated data
      Om_free(jpg_data);

      // set thumbnail bitmap to static control
      HBITMAP hBm = this->setStImage(IDC_SB_SNAP, image.hbmp());
      if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);

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
void OmUiToolRep::_ref_thumb_del()
{
  // get ListBox current selection
  int32_t lb_sel = this->msgItem(IDC_LB_MOD, LB_GETCURSEL);
  if(lb_sel < 0) return;

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lb_sel);

  bool has_changes = false;

  // set node content to data URI string
  if(ref_node.hasChild(L"picture")) {

    ref_node.remChild(L"picture");
    has_changes = true;

    // set thumbnail placeholder image
    HBITMAP hBm = this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_BLANK));
    if(hBm && hBm != Om_getResImage(IDB_BLANK)) DeleteObject(hBm);
  }

  if(has_changes)
    this->_set_unsaved(true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_ref_depends_get(const OmWString& iden, OmWStringArray* missings)
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
      this->_ref_depends_get(dep_iden, missings);
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_ref_depends_check()
{
  // get ListBox current selection
  int32_t lb_sel = this->msgItem(IDC_LB_MOD, LB_GETCURSEL);
  if(lb_sel < 0) return;

  // get corresponding reference node
  OmXmlNode ref_node = this->_NetRepo->getReference(lb_sel);

  OmWStringArray missings;

  // recursive check for dependencies within the repository references
  this->_ref_depends_get(ref_node.attrAsString(L"ident"), &missings);

  if(!missings.empty()) {

    Om_dlgBox_okl(this->_hwnd, L"Repository Editor", IDI_WRN, L"Missing dependencies",
                  L"The following declared Mod dependencies are not referenced in the repository",
                  Om_concatStrings(missings, L"\r\n"));
  } else {

    Om_dlgBox_ok(this->_hwnd, L"Repository Editor", IDI_NFO, L"Satisfied dependencies ",
                 L"All declared Mod dependencies are referenced in the repository");
  }
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
  this->msgItem(IDC_TB_HBAR, TB_SETBUTTONINFO, IDC_BC_SAVE, reinterpret_cast<LPARAM>(&tbBi));

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
    return Om_dlgBox_ync(this->_hwnd, L"Repository editor", IDI_QRY, L"Unsaved changes", L"Do you want to save changes before closing ?");

  return 0;
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
void OmUiToolRep::_browse_add_files()
{
  // if available, select current active channel library as start location
  OmWString start;
  OmModChan* ModChan = static_cast<OmModMan*>(this->_data)->activeChannel();
  if(ModChan) start = ModChan->libraryPath();

  // new dialog to open file (allow multiple selection)
  OmWStringArray result;
  if(!Om_dlgOpenFileMultiple(result, this->_hwnd, L"Open Mod package", OM_PKG_FILES_FILTER, start))
    return;

  // run add list thread
  this->_repo_addlist_start(result);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_browse_add_directory()
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
  this->_repo_addlist_start(paths);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_repo_addlist_start(const OmWStringArray& paths)
{
  // add paths to queue
  for(size_t i = 0; i < paths.size(); ++i)
    this->_repo_addlist_queue.push_back(paths[i]);

  // launch thread if not already running
  if(!this->_repo_addlist_hth) {

    // disable buttons
    this->enableItem(IDC_BC_BRW01, false);
    this->enableItem(IDC_BC_BRW02, false);

    this->_repo_addlist_abort = 0;

    // launch thread
    this->_repo_addlist_hth = Om_createThread(OmUiToolRep::_repo_addlist_run_fn, this);
    this->_repo_addlist_hwo = Om_waitForThread(this->_repo_addlist_hth, OmUiToolRep::_repo_addlist_end_fn, this);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiToolRep::_repo_addlist_run_fn(void* ptr)
{
  OmUiToolRep* self = static_cast<OmUiToolRep*>(ptr);

  // Open progress dialog
  self->_repo_addlist_abort = 0;
  self->_repo_addlist_hpd = Om_dlgProgress(self->_hwnd, L"add Mod references", IDI_PKG_ADD, L"Parsing Mod packages", &self->_repo_addlist_abort);

  // stuff for progress dialog
  OmWString progress_text;
  int32_t progress_tot = self->_repo_addlist_queue.size();
  int32_t progress_cur = 0;

  OmWString file_path;

  // try to add each file, silently fail
  while(self->_repo_addlist_queue.size()) {

    // check for abort
    if(self->_repo_addlist_abort != 0)
      break;

    // get next file to proceed
    file_path = self->_repo_addlist_queue.front();

    // update progress text
    progress_text = L"Computing checksum: ";
    progress_text += Om_getFilePart(file_path);
    Om_dlgProgressUpdate(static_cast<HWND>(self->_repo_addlist_hpd), -1, -1, progress_text.c_str());

    // proceed this file
    self->_repo_add_ref(file_path);

    // update progress bar
    Om_dlgProgressUpdate(static_cast<HWND>(self->_repo_addlist_hpd), progress_tot, ++progress_cur, nullptr);

    #ifdef DEBUG
    Sleep(50); //< for debug
    #endif

    self->_repo_addlist_queue.pop_front();
  }

  // quit the progress dialog (dialogs must be opened and closed within the same thread)
  Om_dlgProgressClose(static_cast<HWND>(self->_repo_addlist_hpd));
  self->_repo_addlist_hpd = nullptr;

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
VOID WINAPI OmUiToolRep::_repo_addlist_end_fn(void* ptr, uint8_t fired)
{
  OM_UNUSED(fired);

  OmUiToolRep* self = static_cast<OmUiToolRep*>(ptr);

  Om_clearThread(self->_repo_addlist_hth, self->_repo_addlist_hwo);

  self->_repo_addlist_hth = nullptr;
  self->_repo_addlist_hwo = nullptr;

  self->_repo_addlist_queue.clear();

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
    case  1: this->_repo_save(); break; //< 'Yes'
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
  this->setIcon(Om_getResIcon(IDI_REP_TOOL,2),Om_getResIcon(IDI_REP_TOOL,1));

  // dialog is modeless so we set dialog title with app name
  this->setCaption(L"Repository editor ");

  // Set menu icons
  HMENU hMnuFile = this->getPopup(MNU_RE_FILE);
  this->setPopupItemIcon(hMnuFile, MNU_RE_FILE_NEW, Om_getResIconPremult(IDI_BT_NEW));
  this->setPopupItemIcon(hMnuFile, MNU_RE_FILE_OPEN, Om_getResIconPremult(IDI_BT_OPN));
  this->setPopupItemIcon(hMnuFile, MNU_RE_FILE_SAVE, Om_getResIconPremult(IDI_BT_SAV));
  this->setPopupItemIcon(hMnuFile, MNU_RE_FILE_SAVAS, Om_getResIconPremult(IDI_BT_SVA));
  this->setPopupItemIcon(hMnuFile, MNU_RE_FILE_QUIT, Om_getResIconPremult(IDI_QUIT));

  HMENU hMnuEdit = this->getPopup(MNU_RE_EDIT);
  this->setPopupItemIcon(hMnuEdit, MNU_RE_EDIT_FAD, Om_getResIconPremult(IDI_BT_FAD));
  this->setPopupItemIcon(hMnuEdit, MNU_RE_EDIT_DAD, Om_getResIconPremult(IDI_BT_DAD));
  this->setPopupItemIcon(hMnuEdit, MNU_RE_EDIT_FRM, Om_getResIconPremult(IDI_BT_FRM));

  HMENU hMnuRef = this->getPopup(MNU_RE_REF);
  this->setPopupItemIcon(hMnuRef, MNU_RE_REF_THMBSEL, Om_getResIconPremult(IDI_PIC_ADD));
  this->setPopupItemIcon(hMnuRef, MNU_RE_REF_THMBDEL, Om_getResIconPremult(IDI_BT_REM));
  this->setPopupItemIcon(hMnuRef, MNU_RE_REF_DESCSEL, Om_getResIconPremult(IDI_TXT_ADD));
  this->setPopupItemIcon(hMnuRef, MNU_RE_REF_DEPCHK, Om_getResIconPremult(IDI_DEP_CHK));

  // Set font for description
  HFONT hFt = Om_createFont(14, 400, L"Consolas");
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  // Set default package picture
  this->setStImage(IDC_SB_SNAP, Om_getResImage(IDB_BLANK));

  // Set buttons icons
  this->setBmIcon(IDC_BC_BRW01, Om_getResIcon(IDI_BT_FAD));
  this->setBmIcon(IDC_BC_BRW02, Om_getResIcon(IDI_BT_DAD));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(IDI_BT_FRM));
  this->setBmIcon(IDC_BC_BRW03, Om_getResIcon(IDI_PIC_ADD)); //< Thumbnail Select
  this->setBmIcon(IDC_BC_RESET, Om_getResIcon(IDI_BT_REM)); //< Thumbnail Erase
  //this->setBmIcon(IDC_BC_SAV02, Om_getResIcon(IDI_BT_SVD)); //< Description Save
  this->setBmIcon(IDC_BC_BRW04, Om_getResIcon(IDI_TXT_ADD)); //< Description Load
  this->setBmIcon(IDC_BC_CHECK, Om_getResIcon(IDI_DEP_CHK));

  // Create the toolbar.
  HWND hWndToolbar = CreateWindowExW(WS_EX_LEFT, TOOLBARCLASSNAMEW, nullptr, WS_CHILD|TBSTYLE_WRAPABLE|TBSTYLE_TOOLTIPS, 0, 0, 0, 0,
                                    this->_hwnd, reinterpret_cast<HMENU>(IDC_TB_HBAR), this->_hins, nullptr);

  HIMAGELIST himl = static_cast<OmUiMan*>(this->root())->toolBarsImgList();
  this->msgItem(IDC_TB_HBAR, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(himl));

  // Initialize button info.
  TBBUTTON tbButtons[3] = {
    {ICON_NEW, IDC_BC_NEW,  TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, reinterpret_cast<INT_PTR>("New repository")},
    {ICON_OPN, IDC_BC_OPEN, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, reinterpret_cast<INT_PTR>("Open definition file")},
    {ICON_SAV, IDC_BC_SAVE, 0,               BTNS_AUTOSIZE, {0}, 0, reinterpret_cast<INT_PTR>("Save definition file")}
  };

  // Add buttons
  this->msgItem(IDC_TB_HBAR, TB_SETMAXTEXTROWS, 0); //< disable text under buttons
  this->msgItem(IDC_TB_HBAR, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON));
  this->msgItem(IDC_TB_HBAR, TB_ADDBUTTONS, 3, reinterpret_cast<LPARAM>(&tbButtons));

  // Resize the toolbar, and then show it.
  SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
  ShowWindow(hWndToolbar,  TRUE);

  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Repository short description");
  this->_createTooltip(IDC_EC_INP02,  L"Relative or absolute link to downloadable files");

  this->_createTooltip(IDC_LB_MOD,    L"Repository referenced Mod list");

  this->_createTooltip(IDC_BC_BRW01,  L"Add from files");
  this->_createTooltip(IDC_BC_BRW02,  L"Add from directory");
  this->_createTooltip(IDC_BC_DEL,    L"Delete reference");

  this->_createTooltip(IDC_BC_CKBX1,  L"Use custom download link");
  this->_createTooltip(IDC_EC_INP03,  L"Relative or absolute link to downloadable file");

  this->_createTooltip(IDC_BC_BRW03,  L"Load image file");
  this->_createTooltip(IDC_BC_RESET,  L"Delete thumbnail image");

  this->_createTooltip(IDC_BC_BRW04,  L"Load text file");
  this->_createTooltip(IDC_EC_DESC,   L"Description");

  this->_createTooltip(IDC_BC_CHECK,  L"Check for dependencies availability");

  // Initialize new Repository definition XML scheme
  //this->_repo_init();

  // disable/enable proper menu items
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_FAD, MF_GRAYED);
  this->setPopupItem(MNU_RE_EDIT, MNU_RE_EDIT_DAD, MF_GRAYED);
  this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVE, MF_GRAYED);
  this->setPopupItem(MNU_RE_FILE, MNU_RE_FILE_SAVAS, MF_GRAYED);

  // update selection to enable menu/buttons
  this->_ref_selected();

  // reset unsaved changes
  this->_set_unsaved(false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onResize()
{
  int32_t half_w = this->cliWidth() * 0.5f;
  int32_t base_y = 30;
  int32_t foot_y = this->cliHeight() - (base_y+28);

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
  this->_setItemPos(IDC_BC_BRW01, half_w-83, base_y+145, 23, 22, true);
  this->_setItemPos(IDC_BC_BRW02, half_w-58, base_y+145, 23, 22, true);
  this->_setItemPos(IDC_BC_DEL,   half_w-33, base_y+145, 23, 22, true);
  // Referenced Mods list
  this->_setItemPos(IDC_LB_MOD, 10, base_y+170, half_w-20, (foot_y-177), true);
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
  this->_setItemPos(IDC_SC_NOTES, half_w+100, base_y+90, 180, 16, true);
  // Thumbnail Action buttons
  this->_setItemPos(IDC_BC_RESET, this->cliWidth()-58, base_y+87, 23, 22, true);
  this->_setItemPos(IDC_BC_BRW03, this->cliWidth()-33, base_y+87, 23, 22, true);
  // Thumbnail static bitmap
  this->_setItemPos(IDC_SB_SNAP, half_w+10, base_y+110, 128, 128, true);

  // Description label
  this->_setItemPos(IDC_SC_LBL05, half_w+10, base_y+260, 90, 16, true);
  // Description Actions buttons
  this->_setItemPos(IDC_BC_SAV02, this->cliWidth()-58, base_y+257, 23, 22, true);
  this->_setItemPos(IDC_BC_BRW04, this->cliWidth()-33, base_y+257, 23, 22, true);
  // Description EditText
  this->_setItemPos(IDC_EC_DESC, half_w+10, base_y+280, half_w-21, foot_y-(290+85), true);

  // Dependencies Label
  this->_setItemPos(IDC_SC_LBL06, half_w+10, foot_y-45, 100, 16, true);
  // Dependencies Check button
  this->_setItemPos(IDC_BC_CHECK, this->cliWidth()-33, foot_y-48, 23, 22, true);
  // Dependencies EditText & Check Button
  this->_setItemPos(IDC_EC_READ3, half_w+10, foot_y-25, half_w-20, 48, true);

  // ---- Foot status bar
  this->_setItemPos(IDC_SC_STATUS, 2, this->cliHeight()-24, this->cliWidth()-4, 22, true);
  this->_setItemPos(IDC_SC_FILE, 7, this->cliHeight()-20, half_w, 16, true);
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

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_MOD: //< Packages list ListBox
      // check for selection change
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        #ifdef DEBUG
        std::wcout << "DEBUG => OmUiToolRep::_onMsg : IDC_LB_MOD : LBN_SELCHANGE\n";
        #endif // DEBUG
        this->_ref_selected();
      }
      break;

    case IDC_BC_NEW:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_repo_init();
      break;

    case IDC_BC_OPEN:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_repo_open();
      break;

    case IDC_BC_SAVE: //< General "Save as.." Button
      this->_repo_save();
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
        this->_repo_save_title();
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
        this->_repo_save_downpath();
      break;

    case IDC_BC_BRW01: //< Button : Mod references Add Files
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_add_files();
      break;

    case IDC_BC_BRW02: //< Button : Mod references Add Directory
      if(HIWORD(wParam) == BN_CLICKED)
        this->_browse_add_directory();
      break;

    case IDC_BC_DEL:
      if(HIWORD(wParam) == BN_CLICKED)
        this->_repo_del_ref();
      break;

    case IDC_BC_CKBX1: // Custom Url CheckBox
      if(this->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
        this->enableItem(IDC_EC_INP03, true);
      } else {
        this->setItemText(IDC_EC_INP03, L"");
        this->_ref_url_save();
        this->enableItem(IDC_EC_INP03, false);
      }
      break;

    case IDC_EC_INP03: //< Custon URL EditText
      if(HIWORD(wParam) == EN_KILLFOCUS)
        this->_ref_url_save();
      break;

    case IDC_BC_BRW03: //< Thumbnail "Select..." Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_ref_thumb_set();
      break;

    case IDC_BC_RESET: //< Thumbnail "Delete" Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_ref_thumb_del();
      break;

    case IDC_BC_BRW04: //< Description "Load.." Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_ref_desc_load();
      break;

    case IDC_EC_DESC: //< Description EditText
      // check for content changes
      if(HIWORD(wParam) == EN_KILLFOCUS)
        this->_ref_desc_save();
      break;

    case IDC_BC_CHECK: //< Dependencies "Check" Button
      if(HIWORD(wParam) == BN_CLICKED)
        this->_ref_depends_check();
      break;

    case IDM_FILE_NEW:
      this->_repo_init();
      break;

    case IDM_FILE_OPEN:
      this->_repo_open();
      break;

    case IDM_FILE_SAVE:
      this->_repo_save();
      break;

    case IDM_FILE_SAVAS:
      this->_repo_save_as();
      break;

    case IDM_QUIT:
      this->_onClose();
      break;

    case IDM_ENTRY_FADD:
      this->_browse_add_files();
      break;

    case IDM_ENTRY_DADD:
      this->_browse_add_directory();
      break;

    case IDM_ENTRY_DEL:
      this->_repo_del_ref();
      break;

    case IDM_REF_THMBSEL:
      this->_ref_thumb_set();
      break;

    case IDM_REF_THMBDEL:
      this->_ref_thumb_del();
      break;

    case IDM_REF_DESCSEL:
      this->_ref_desc_load();
      break;

    case IDM_REF_DEPCHK:
      this->_ref_depends_check();
      break;
    }
  }

  return false;
}

