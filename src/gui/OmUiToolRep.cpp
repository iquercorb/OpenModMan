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
#include "gui/OmUiToolRep.h"
#include "OmPackage.h"
#include "OmManager.h"
#include "OmContext.h"
#include "gui/OmUiProgress.h"

#define REPO_DEFAULT_DOWLOAD  L"files/"

/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the _addDir_fth
/// thread finished his job.
///
#define UWM_ADDENTRIES_DONE   (WM_APP+1)

/// \brief Save repository remote package snapshot
///
/// Function to create and store (as Data URI) repository remote package snapshot
/// from the given image object. The image is:
///  1 - loaded as raw RGB(A) data.
///  2 - converted to its thumbnail version (128 x 128 pixels).
///  3 - encoded to JPEG image data.
///  3 - the result is encoded to Base64 then assembled in Data URI scheme.
///
/// \param[in]  xml_pic : <picture> child node of a <package> XML node.
/// \param[in]  image   : Image object to convert to repository snapshot.
///
/// \return True if operation succeed, false otherwise
///
static inline bool __save_snapshot(OmXmlNode& xml_pic, const OmImage& image)
{
  // Load image data to raw RGB
  unsigned w, h, c;
  uint8_t* rgb = Om_loadImage(&w, &h, &c, image.data(), image.data_size());

  if(rgb) {
    // Get thumbnail version
    uint8_t* thn = Om_thumbnailImage(128, rgb, w, h, c);
    Om_free(rgb);

    if(thn) {

      // Encode image to JPEG
      size_t jpg_size;
      uint8_t* jpg = Om_encodeJpg(&jpg_size, thn, 128, 128, c, 7);
      Om_free(thn);

      if(jpg) {

        // format jpeg to base64 encoded data URI
        wstring data_uri;
        Om_encodeDataUri(data_uri, L"image/jpeg", L"", jpg, jpg_size);
        Om_free(jpg);

        // set node content to data URI string
        xml_pic.setContent(data_uri);

        return true;
      }
    }
  }
  return false;
}

/// \brief Save repository remote package description
///
/// Function to create and store (as Data URI) repository remote package description
/// from the given image object. The text is:
///  1 - encoded to UTF-8 text.
///  2 - compressed using Deflate algorithm.
///  3 - the result is encoded to Base64 then assembled in Data URI scheme.
///
/// \param[in]  xml_des : <description> child node of a <package> XML node.
/// \param[in]  text    : Description string to convert to repository description.
///
/// \return True if operation succeed, false otherwise
///
static inline bool __save_description(OmXmlNode& xml_des, const wstring& text)
{
  // convert to UTF-8
  string utf8 = Om_toUtf8(text);
  size_t txt_size = utf8.size() + 1; // we include the nullchar

  if(txt_size > 1) {

    // set original data size in xml
    xml_des.setAttr(L"bytes", static_cast<int>(txt_size));

    // compress data
    size_t zip_size;
    uint8_t* zip = Om_zDeflate(&zip_size, reinterpret_cast<const uint8_t*>(utf8.c_str()), txt_size, 9);

    if(zip) {
      // Encode to data URI Base64
      wstring data_uri;
      Om_encodeDataUri(data_uri, L"application/octet-stream", L"", zip, zip_size);
      Om_free(zip);

      // set node content to data URI string
      xml_des.setContent(data_uri);

      return true;
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolRep::OmUiToolRep(HINSTANCE hins) : OmDialog(hins),
  _condig(),
  _rmtCur(),
  _addDir_hth(nullptr),
  _addDir_path()
{
  this->addChild(new OmUiProgress(hins));   //< for Location backup cleaning

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolRep::~OmUiToolRep()
{
  HBITMAP hBm = this->setStImage(IDC_SB_PKG, nullptr);
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);

  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_PKTXT, WM_GETFONT));
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
void OmUiToolRep::_repInit()
{
  // Initialize new Repository definition XML scheme
  this->_condig.init(OMM_CFG_SIGN_REP);

  // Generate a new UUID for this Repository
  wstring uuid = Om_genUUID();

  // Create repository base scheme
  OmXmlNode xml_def = this->_condig.xml();
  xml_def.addChild(L"uuid").setContent(uuid);
  xml_def.addChild(L"title").setContent(L"New Repository");
  xml_def.addChild(L"downpath").setContent(REPO_DEFAULT_DOWLOAD);
  xml_def.addChild(L"remotes").setAttr(L"count", 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_repOpen(const wstring& path)
{
  // Initialize new Repository definition XML scheme
  if(!this->_condig.open(path, OMM_CFG_SIGN_REP)) {
    wstring err = L"The file \""+path+L"\" is not valid Repository file.";
    Om_dialogBoxErr(this->_hwnd, L"Error parsing Repository file", err);
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmUiToolRep::_rmtGet(const wstring& ident)
{
  OmXmlNode result;

  // Get the package list XML node
  OmXmlNode xml_rmts = this->_condig.xml().child(L"remotes");

  // Get all <remote> children
  std::vector<OmXmlNode> xml_rmt_ls;
  xml_rmts.children(xml_rmt_ls, L"remote");

  // search <remote> with specified identity
  for(size_t i = 0; i < xml_rmt_ls.size(); ++i) {
    if(ident == xml_rmt_ls[i].attrAsString(L"ident")) {
      result = xml_rmt_ls[i]; break;
    }
  }

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_rmtAdd(const wstring& path)
{
  // parse the package
  OmXmlNode xml_node, xml_rmt, xml_rmts;

  // try to parse package
  OmPackage pkg;
  if(!pkg.srcParse(path))
    return false;

  // Search for already existing <remote>
  xml_rmt = this->_rmtGet(pkg.ident());

  // if <remote> with same identity already exist, user have to choose
  if(!xml_rmt.empty()) {

    wstring qry = L"Repository package with identity \""+pkg.ident()+L"\" already exists.";
    qry += L"\n\nDo you want to replace the existing one ?";

    if(!Om_dialogBoxQuerryWarn(this->_hwnd, L"Duplicated entry", qry))
      return true; // cancel operation

    // check whether the current duplicate <remote> is the current selected one
    if(!this->_rmtCur.empty()) {
      // unselect package in list, to prevent inconsistent control contents
      if(this->_rmtCur.attrAsString(L"ident") == pkg.ident()) {
        this->_rmtSel(L"");
      }
    }

    // reset the current Repository <remote>
    xml_rmt.remChild(L"dependencies");
    xml_rmt.remChild(L"picture");
    xml_rmt.remChild(L"description");
    xml_rmt.setAttr(L"file", Om_getFilePart(pkg.srcPath()));
    xml_rmt.setAttr(L"bytes", static_cast<int>(Om_itemSize(path)));
    xml_rmt.setAttr(L"checksum",Om_getChecksum(path));


  } else {

    // Get the remote package list XML node
    xml_rmts = this->_condig.xml().child(L"remotes");

    // create new <remote> in repository
    xml_rmt = xml_rmts.addChild(L"remote");
    xml_rmt.setAttr(L"ident", pkg.ident());
    xml_rmt.setAttr(L"file", Om_getFilePart(pkg.srcPath()));
    xml_rmt.setAttr(L"bytes", static_cast<int>(Om_itemSize(path)));
    xml_rmt.setAttr(L"checksum",Om_getChecksum(path));

    // Add package to ListBox
    this->msgItem(IDC_LB_PKG, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pkg.ident().c_str()));

    // increment <remote> count
    int n = xml_rmts.attrAsInt(L"count");
    xml_rmts.setAttr(L"count", n + 1);
  }

  if(pkg.depCount()) {
    xml_node = xml_rmt.addChild(L"dependencies");
    for(size_t i = 0; i < pkg.depCount(); ++i) {
      xml_node.addChild(L"ident").setContent(pkg.depGet(i));
    }
  }

  if(pkg.image().valid()) {
    xml_node = xml_rmt.addChild(L"picture");
    __save_snapshot(xml_node, pkg.image());
  }

  if(!pkg.desc().empty()) {
    xml_node = xml_rmt.addChild(L"description");
    __save_description(xml_node, pkg.desc());
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_rmtRem(const wstring& ident)
{
  // Get the package list XML node
  OmXmlNode xml_rmts = this->_condig.xml().child(L"remotes");

  // Get all <remote> children
  std::vector<OmXmlNode> xml_rmt_ls;
  xml_rmts.children(xml_rmt_ls, L"remote");

  // search <remote> with specified identity
  for(size_t i = 0; i < xml_rmt_ls.size(); ++i) {
    if(ident == xml_rmt_ls[i].attrAsString(L"ident")) {
      xml_rmts.remChild(xml_rmt_ls[i]);

      // decrement package count
      int n = xml_rmts.attrAsInt(L"count");
      xml_rmts.setAttr(L"count", n - 1);

      return true;
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_rmtSel(const wstring& ident)
{
  // clear current selected <remote>
  this->_rmtCur.clear();

  // check for empty selection
  if(ident.empty()) {

    // unselect all in ListBox
    this->msgItem(IDC_LB_PKG, LB_SETCURSEL, -1, 0);

    // no package selected, disable and reset all related controls
    this->setItemText(IDC_EC_OUT02, L"");
    this->setItemText(IDC_EC_INP03, L"");
    this->enableItem(IDC_EC_INP03, false);
    this->enableItem(IDC_BC_SAV01, false);
    this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 0);
    this->setItemText(IDC_EC_OUT04, L"");
    this->enableItem(IDC_BC_CHK, false);
    this->enableItem(IDC_BC_BRW08, false);
    this->enableItem(IDC_BC_DEL, false);
    HBITMAP hBm = this->setStImage(IDC_SB_PKG, Om_getResImage(this->_hins, IDB_PKG_THN));
    if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);
    this->enableItem(IDC_BC_BRW09, false);
    this->enableItem(IDC_BC_SAV02, false);
    this->setItemText(IDC_EC_PKTXT, L"");
    this->enableItem(IDC_EC_PKTXT, false);

    return true;
  }

  // Get the remote packages list XML node
  OmXmlNode xml_rmts = this->_condig.xml().child(L"remotes");

  // Get all <remote> children
  std::vector<OmXmlNode> xml_rmt_ls;
  xml_rmts.children(xml_rmt_ls, L"remote");

  // search <remote> with specified identity
  for(size_t i = 0; i < xml_rmt_ls.size(); ++i) {
    if(ident == xml_rmt_ls[i].attrAsString(L"ident")) {
      this->_rmtCur = xml_rmt_ls[i]; break;
    }
  }

  // verify we found something
  if(this->_rmtCur.empty())
    return false;

  OmXmlNode xml_node;
  wstring tmp_str1, tmp_str2;
  HBITMAP hBm_new, hBm_old;

  this->setItemText(IDC_EC_OUT02, this->_rmtCur.attrAsString(L"file"));

  // allow custom URL CheckBox
  this->enableItem(IDC_BC_CHK01, true);

  // Check whether this <remote> have a Custom URL
  if(this->_rmtCur.hasChild(L"url")) {

    this->enableItem(IDC_EC_INP03, true);
    this->setItemText(IDC_EC_INP03, this->_rmtCur.child(L"url").content());
    this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 1);

  } else {

    this->setItemText(IDC_EC_INP03, L"");
    this->enableItem(IDC_EC_INP03, false);
    this->msgItem(IDC_BC_CHK01, BM_SETCHECK, 0);
  }

  // custom URL save button disabled by default
  this->enableItem(IDC_BC_SAV01, false);

  // check for <remote> dependencies
  if(this->_rmtCur.hasChild(L"dependencies")) {

    xml_node = this->_rmtCur.child(L"dependencies");
    std::vector<OmXmlNode> xml_ls;
    xml_node.children(xml_ls, L"ident");

    wstring dpn_str;
    for(unsigned i = 0; i < xml_ls.size(); ++i) {
      dpn_str += xml_ls[i].content();
      if(i < (xml_ls.size() - 1)) {
        dpn_str += L"; ";
      }
    }

    this->enableItem(IDC_BC_CHK, true);
    this->enableItem(IDC_EC_OUT04, true);
    this->setItemText(IDC_EC_OUT04, dpn_str);

  } else {

    this->enableItem(IDC_BC_CHK, false);
    this->enableItem(IDC_EC_OUT04, false);
    this->setItemText(IDC_EC_OUT04, L"");
  }

  // enable Snapshot "Select..." Button
  this->enableItem(IDC_BC_BRW08, true);

  // for default controls state
  bool has_snap = false;

  // check for <remote> snapshot
  if(this->_rmtCur.hasChild(L"picture")) {
    xml_node = this->_rmtCur.child(L"picture");

    // decode the DataURI
    size_t jpg_size;
    uint8_t* jpg = Om_decodeDataUri(&jpg_size, tmp_str1, tmp_str2, xml_node.content());

    // load Jpeg image
    if(jpg) {

      // decode jpeg to rgb data
      unsigned w, h, c;
      uint8_t* rgb = Om_loadImage(&w, &h, &c, jpg, jpg_size);
      Om_free(jpg);

      if(rgb) {

        // convert to HBITMAP
        hBm_new = Om_hbitmapImage(rgb, w, h, c);
        Om_free(rgb);

        // set image to dialog
        hBm_old = this->setStImage(IDC_SB_PKG, hBm_new);
        if(hBm_old && hBm_old != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm_old);
        this->enableItem(IDC_BC_DEL, true);
        this->setItemText(IDC_BC_OPEN1, L"Change...");
        has_snap = true;
      }
    }

    if(!has_snap) {
      // TODO: do something in case snapshot exist but is corrupted or invalid
    }

  }

  // no snapshot, reset controls to default
  if(!has_snap) {
    hBm_old = this->setStImage(IDC_SB_PKG, Om_getResImage(this->_hins, IDB_PKG_THN));
    if(hBm_old && hBm_old != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm_old);
    this->enableItem(IDC_BC_DEL, false);
  }

  // allow user to edit a description
  this->enableItem(IDC_BC_BRW09, true);
  this->enableItem(IDC_BC_SAV02, false);
  this->enableItem(IDC_EC_PKTXT, true);

  // for default controls state
  bool has_desc = false;

  // check for <remote> description
  if(this->_rmtCur.hasChild(L"description")) {
    xml_node = this->_rmtCur.child(L"description");

    // decode the DataURI
    size_t zip_size;
    uint8_t* zip = Om_decodeDataUri(&zip_size, tmp_str1, tmp_str2, xml_node.content());

    if(zip) {
      // get string original size
      size_t txt_size = xml_node.attrAsInt(L"bytes");

      // decompress data
      uint8_t* txt = Om_zInflate(zip, zip_size, txt_size);
      Om_free(zip);

      if(txt) {
        // set text to item
        this->setItemText(IDC_EC_PKTXT, Om_fromUtf8(reinterpret_cast<char*>(txt)));
        Om_free(txt);
        has_desc = true;
      }
    }

    if(!has_desc) {
      // TODO: do something in case description exist but is corrupted or invalid
    }
  }

  if(!has_desc) {
    this->setItemText(IDC_EC_PKTXT, L"");
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmUiToolRep::_rmtGetDeps(vector<wstring>& miss_list, const wstring& ident)
{
  // our missing result
  int miss = 0;

  // get <remote> XML node by ident
  OmXmlNode xml_rmt = this->_rmtGet(ident);

  // get list of dependencies
  if(xml_rmt.hasChild(L"dependencies")) {

    std::vector<OmXmlNode> xml_dep_ls;
    xml_rmt.child(L"dependencies").children(xml_dep_ls, L"ident");

    bool unique;
    wstring dep_idt;
    OmXmlNode xml_dep;

    for(unsigned i = 0; i < xml_dep_ls.size(); ++i) {

      // get dependency identity
      dep_idt = xml_dep_ls[i].content();

      // try to get <remote> with this identity
      xml_dep = this->_rmtGet(dep_idt);

      // if <remote> not found, dependency is missing
      if(xml_dep.empty()) {

        // we add only if unique
        unique = true;
        for(size_t k = 0; k < miss_list.size(); ++k) {
          if(miss_list[i] == dep_idt) {
            unique = false; break;
          }
        }
        if(unique) {
          miss_list.push_back(dep_idt);
          miss++;
        }
        // next item
        continue;
      }
      // recurse process with dependency node
      miss += this->_rmtGetDeps(miss_list, dep_idt);
    }
  }
  return miss;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_addDir_init(const wstring& path)
{
  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));

  pUiProgress->open(true);
  pUiProgress->setCaption(L"Add multiples package entries");
  pUiProgress->setScHeadText(L"Computes packages checksum");

  // keep path to folder to scan
  this->_addDir_path = path;

  // start package building thread
  DWORD dWid;
  this->_addDir_hth = CreateThread(nullptr, 0, this->_addDir_fth, this, 0, &dWid);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_addDir_stop()
{
  DWORD exitCode;

  if(this->_addDir_hth) {
    WaitForSingleObject(this->_addDir_hth, INFINITE);
    GetExitCodeThread(this->_addDir_hth, &exitCode);
    CloseHandle(this->_addDir_hth);
    this->_addDir_hth = nullptr;
  }

  this->_addDir_path.clear();

  // quit the progress dialog
  static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS))->quit();

  // Enable or Disable Save as... button according ListBox content
  int lb_cnt = this->msgItem(IDC_LB_PKG, LB_GETCOUNT, 0, 0);
  this->enableItem(IDC_BC_SAVE, (lb_cnt > 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiToolRep::_addDir_fth(void* arg)
{
  OmUiToolRep* self = static_cast<OmUiToolRep*>(arg);

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(self->childById(IDD_PROGRESS));

  HWND hPb = pUiProgress->hPb();
  const bool* abort = pUiProgress->abortPtr();

  // get all file from given path
  vector<wstring> ls;
  Om_lsFile(&ls, self->_addDir_path, true);

  // initialize the progress bar
  if(hPb) {
    SendMessageW(hPb, PBM_SETRANGE, 0, MAKELPARAM(0, ls.size()));
    SendMessageW(hPb, PBM_SETSTEP, 1, 0);
    SendMessageW(hPb, PBM_SETPOS, 0, 0);
  }

  // try to add each file, silently fail
  for(size_t i = 0; i < ls.size(); ++i) {

    // set dialog progress detail text
    pUiProgress->setScItemText(Om_getFilePart(ls[i]).c_str());

    // proceed this package
    self->_rmtAdd(ls[i]);

    // step progress bar
    if(hPb) SendMessageW(hPb, PBM_STEPIT, 0, 0);
    #ifdef DEBUG
    Sleep(OMM_DEBUG_SLOW); //< for debug
    #endif

    if(*abort) break;
  }


  // sends message to window to inform process ended
  PostMessage(self->_hwnd, UWM_ADDENTRIES_DONE, 0, 0);

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcNew()
{
  // Check for unsaved changes
  if(this->itemEnabled(IDC_BC_SAVE)) {

    wstring qry = L"Do you want to save Repository before continue ?";

    if(Om_dialogBoxQuerryWarn(this->_hwnd, L"Unsaved changes", qry)) {
      this->_onBcSave();
    }
  }

  // Unselect <remote>
  this->_rmtSel(L"");

  // Initialize a new Repository XML def
  this->_repInit();

  // reset ListBox content
  this->msgItem(IDC_LB_PKG, LB_RESETCONTENT, 0, 0);

  // Set default title and download path to controls
  OmXmlNode xml_def = this->_condig.xml();
  this->setItemText(IDC_EC_INP01, xml_def.child(L"title").content());
  this->setItemText(IDC_EC_INP02, xml_def.child(L"downpath").content());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcOpen()
{
  OmContext* pCtx = static_cast<OmManager*>(this->_data)->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  wstring start, result;

  // select the initial location for browsing start
  if(pLoc) start = pLoc->libDir();

  // new dialog to open file
  if(!Om_dialogOpenFile(result, this->_hwnd, L"Select XML repository file", OMM_XML_FILES_FILTER, start))
    return;

  if(!Om_isFile(result))
    return;

  // unselect all
  this->_rmtSel(L"");

  // reset ListBox content
  this->msgItem(IDC_LB_PKG, LB_RESETCONTENT, 0, 0);

  if(!this->_repOpen(result))
    return;

  // Get Repository Title
  this->setItemText(IDC_EC_INP01, this->_condig.xml().child(L"title").content());
  // Get Download path
  this->setItemText(IDC_EC_INP02, this->_condig.xml().child(L"downpath").content());

  OmXmlNode xml_rmts = this->_condig.xml().child(L"remotes");

  // get all <remote> nodes within <remotes>
  std::vector<OmXmlNode> xml_rmt_ls;
  xml_rmts.children(xml_rmt_ls, L"remote");

  // Add each <remote> to ListBox
  for(size_t i = 0; i < xml_rmt_ls.size(); ++i) {
    this->msgItem(IDC_LB_PKG, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(xml_rmt_ls[i].attrAsString(L"ident")));
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcBrwPkg()
{
  OmContext* pCtx = static_cast<OmManager*>(this->_data)->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  wstring start, result;

  // select the initial location for browsing start
  if(pLoc) start = pLoc->libDir();

  // open file dialog
  if(!Om_dialogOpenFile(result, this->_hwnd, L"Select Package file", OMM_PKG_FILES_FILTER, start))
    return;

  if(!Om_isFile(result))
    return;

  // add package to repository
  if(!this->_rmtAdd(result)) {
    wstring err = L"The file \""+result+L"\" is not valid Package file.";
    Om_dialogBoxErr(this->_hwnd, L"Error parsing Package file", err);
  }

  // Enable or Disable Save as... button according ListBox content
  int lb_cnt = this->msgItem(IDC_LB_PKG, LB_GETCOUNT, 0, 0);
  this->enableItem(IDC_BC_SAVE, (lb_cnt > 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcBrwDir()
{
  OmContext* pCtx = static_cast<OmManager*>(this->_data)->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  wstring start, result;

  // select the initial location for browsing start
  if(pLoc) start = pLoc->libDir();

  // open dialog to select folder
  if(!Om_dialogBrowseDir(result, this->_hwnd, L"Select folder to search for packages", start))
    return;

  if(!Om_isDir(result))
    return;

  // each add need to compute the file checksum, this operation can take
  // long time specially with huge files, to prevent unpleasant freeze of
  // dialog, we proceed through progress dialog within new thread.
  this->_addDir_init(result);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcRemPkg()
{
  // get current selected item string in ListBox
  int lb_sel = this->msgItem(IDC_LB_PKG, LB_GETCURSEL);

  if(lb_sel < 0)
    return;

  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  // get identity from ListBox string
  wchar_t iden_buf[OMM_ITM_BUFF];
  this->msgItem(IDC_LB_PKG, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(iden_buf));

  // remove <remote> from XML definition
  this->_rmtRem(iden_buf);

  // Unselect remote
  this->_rmtSel(L"");

  // Remove remote from ListBox
  this->msgItem(IDC_LB_PKG, LB_DELETESTRING, lb_sel, 0);

  // Enable or Disable Save as... button according ListBox content
  int lb_cnt = this->msgItem(IDC_LB_PKG, LB_GETCOUNT, 0, 0);
  this->enableItem(IDC_BC_SAVE, (lb_cnt > 0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onLbPkglsSel()
{
  // get ListBox current selection
  int lb_sel = this->msgItem(IDC_LB_PKG, LB_GETCURSEL);

  // enable or disable the Package "Remove" Button
  this->enableItem(IDC_BC_REM, (lb_sel >= 0));

  // if any selection, change current remote
  if(lb_sel >= 0) {

    // Get identity to select
    wchar_t iden_buf[OMM_ITM_BUFF];
    this->msgItem(IDC_LB_PKG, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(iden_buf));

    // Select remote by identity
    this->_rmtSel(iden_buf);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcSavUrl()
{
  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  // Get URL from EditText
  wstring url_str;
  this->getItemText(IDC_EC_INP03, url_str);

  // Check for empty string
  if(!url_str.empty()) {

    // add a lasting / if needed
    if(url_str.back() != L'/')
      url_str.push_back(L'/');

    // Check for valid URL
    if(Om_isValidUrl(url_str)) {

      // Create node if needed
      if(!this->_rmtCur.hasChild(L"url"))
        this->_rmtCur.addChild(L"url");

      // Defile new URL string
      this->_rmtCur.child(L"url").setContent(url_str);

      // echo changes in EditText
      this->setItemText(IDC_EC_INP03, url_str);

    } else {

      wstring err = L"\""+url_str+L"\" is not a valid URL.";
      Om_dialogBoxErr(this->_hwnd, L"Invalid URL", err);

      // return now
      return;
    }

  } else {

    // Remove <url> node
    if(this->_rmtCur.hasChild(L"url")) {
      this->_rmtCur.remChild(L"url");
    }
  }

  // disable Custom Url "Save" Button
  this->enableItem(IDC_BC_SAV01, false);
  // enable Main "Save as..." Button
  this->enableItem(IDC_BC_SAVE, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcChkDeps()
{
  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  // get identity for this remote
  wstring ident = this->_rmtCur.attrAsString(L"ident");

  // Dependencies missing list
  vector<wstring> miss_ls;

  // go for recursive dependencies search
  int miss_cnt = this->_rmtGetDeps(miss_ls, ident);

  if(miss_cnt > 0) {
    // Warning message
    wstring wrn =   L"The package has dependencies which was "
                    "not found in the repository:\n\n";

    for(unsigned i = 0; i < miss_ls.size(); ++i)
      wrn += L"    " + miss_ls[i] + L"\n";

    Om_dialogBoxWarn(this->_hwnd, L"Missing package dependencies", wrn);

  } else {

    //Peaceful message
    wstring msg =   L"All dependencies for this package "
                    "was found in the repository.";
    Om_dialogBoxInfo(this->_hwnd, L"Dependencies package satisfied", msg);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcBrwSnap()
{
  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  OmContext* pCtx = static_cast<OmManager*>(this->_data)->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  wstring start, result;

  // select the initial location for browsing start
  if(pLoc) start = pLoc->libDir();

  // open file dialog
  if(!Om_dialogOpenFile(result, this->_hwnd, L"Select image file", OMM_IMG_FILES_FILTER, start))
    return;

  OmImage image;

  // try to load image file
  if(image.open(result, OMM_PKG_THMB_SIZE)) {

    OmXmlNode xml_pic;

    // retrieve or create <picture> child in <package>
    if(this->_rmtCur.hasChild(L"picture")) {
      xml_pic = this->_rmtCur.child(L"picture");
    } else {
      xml_pic = this->_rmtCur.addChild(L"picture");
    }

    // save snapshot data to <picture>
    if(__save_snapshot(xml_pic, image)) {

      HBITMAP hBm = this->setStImage(IDC_SB_PKG, image.thumbnail());
      if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);

      // enable General "Save as..." Button
      this->enableItem(IDC_BC_SAVE, true);

      // enable Snapshot "Delete" Button
      this->enableItem(IDC_BC_DEL, true);

      // return now
      return;
    }
  }

  // this what happen if something went wrong
  this->_onBcDelSnap(); //< delete Snapshot and reset controls
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcDelSnap()
{
  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  // retrieve or create <picture> child in <remote>
  if(this->_rmtCur.hasChild(L"picture")) {
    this->_rmtCur.remChild(L"picture");
  }

  HBITMAP hBm = this->setStImage(IDC_SB_PKG, Om_getResImage(this->_hins, IDB_PKG_THN));
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_PKG_THN)) DeleteObject(hBm);

  // disable Snapshot "Delete" Button
  this->enableItem(IDC_BC_DEL, false);
  // Enable Save as... button
  this->enableItem(IDC_BC_SAVE, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcBrwDesc()
{
  OmContext* pCtx = static_cast<OmManager*>(this->_data)->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  wstring start, result;

  // select the initial location for browsing start
  if(pLoc) start = pLoc->libDir();

  // open file dialog
  if(!Om_dialogOpenFile(result, this->_hwnd, L"Select text file", OMM_TXT_FILES_FILTER, start))
    return;

  if(!Om_isFile(result))
    return;

  // set loaded text as description
  string text_str = Om_loadPlainText(result);
  SetDlgItemTextA(this->_hwnd, IDC_EC_PKTXT, text_str.c_str());

  // enable Description "Save" Button
  this->enableItem(IDC_BC_SAV02, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcSavDesc()
{
  // check whether any <remote> is selected
  if(this->_rmtCur.empty())
    return;

  // get description string
  wstring desc_str;
  this->getItemText(IDC_EC_PKTXT, desc_str);

  if(!desc_str.empty()) {

    OmXmlNode xml_des;

    // retrieve or create <description> child in <remote>
    if(this->_rmtCur.hasChild(L"description")) {
      xml_des = this->_rmtCur.child(L"description");
    } else {
      xml_des = this->_rmtCur.addChild(L"description");
    }

    // save description string to <description>
    __save_description(xml_des, desc_str);

  } else {

    // retrieve or create <description> child in <remote>
    if(this->_rmtCur.hasChild(L"description")) {
      this->_rmtCur.remChild(L"description");
    }
  }

  // disable Description "Save" button
  this->enableItem(IDC_BC_SAV02, false);
  // enable General "Save as.." button
  this->enableItem(IDC_BC_SAVE, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcSave()
{
  OmContext* pCtx = static_cast<OmManager*>(this->_data)->ctxCur();
  OmLocation* pLoc = pCtx ? pCtx->locCur() : nullptr;

  // Repository XML node
  OmXmlNode xml_def = this->_condig.xml();

  // Before saving, update the download path and title
  wstring item_str;
  this->getItemText(IDC_EC_INP01, item_str);

  // set title
  xml_def.child(L"title").setContent(item_str);

  this->getItemText(IDC_EC_INP02, item_str);

  // add a lasting / if needed
  if(item_str.back() != L'/')
    item_str.push_back(L'/');

  // check whether path is valid
  if(!Om_isValidUrlPath(item_str)) {

    wstring err = L"Download path \""+item_str+L"\" is not a valid URL path.";
    Om_dialogBoxErr(this->_hwnd, L"Invalid download URL path", err);
    return;
  }

  // set download path
  xml_def.child(L"downpath").setContent(item_str);
  // echo changes in EditText
  this->setItemText(IDC_EC_INP02, item_str);

  wstring start, result;

  // select the initial location for browsing start
  if(pLoc) start = pLoc->libDir();

  // send save dialog to user
  if(!Om_dialogSaveFile(result, this->_hwnd, L"Save Repository definition...", OMM_XML_FILES_FILTER, start))
    return;

  // check for ".xml" extension, add it if needed
  if(!Om_extensionMatches(result, L"xml")) {
    result += L".xml";
  }

  if(Om_isValidPath(result)) {
    if(Om_isFile(result)) {
      wstring qry = L"The file \""+Om_getFilePart(result)+L"\"";
      qry += OMM_STR_QRY_OVERWRITE;
      if(!Om_dialogBoxQuerry(this->_hwnd, L"File already exists", qry)) {
        return;
      }
    }
  } else {
    wstring err = L"File name ";
    err += OMM_STR_ERR_VALIDNAME;
    Om_dialogBoxErr(this->_hwnd, L"Invalid file name", err);
    return;
  }

  if(!this->_condig.save(result)) {
    wstring err = L"Failed to save file "+Om_getFilePart(result)+L"\", ";
    err += this->_condig.lastErrorStr();
    Om_dialogBoxErr(this->_hwnd, L"Unable to save file", err);
    return;
  }

  // Disable Save as... button
  this->enableItem(IDC_BC_SAVE, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onBcClose()
{
  // check whether there is unsaved changes
  if(this->itemEnabled(IDC_BC_SAVE)) {

    // ask user to save
    wstring qry = L"Do you want to save Repository before closing ?";

    if(Om_dialogBoxQuerryWarn(this->_hwnd, L"Unsaved changes", qry)) {
      this->_onBcSave(); //< emulate Save as.. button click
    }
  }

  // quit this dialog
  this->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_BRW01, L"Select Repository XML file");
  this->_createTooltip(IDC_BC_NEW, L"Initialize new repository");

  this->_createTooltip(IDC_EC_INP01, L"Indicative title");
  this->_createTooltip(IDC_EC_INP02, L"Default download path");

  this->_createTooltip(IDC_LB_PKG, L"Repository remote package list");

  this->_createTooltip(IDC_BC_BRW02, L"Add package");
  this->_createTooltip(IDC_BC_BRW03, L"Add all packages from folder");
  this->_createTooltip(IDC_BC_REM, L"Remove selected package");

  this->_createTooltip(IDC_BC_CHK01, L"Use custom URL for download");
  this->_createTooltip(IDC_EC_INP03, L"Download URL prefix");
  this->_createTooltip(IDC_BC_SAV01, L"Save custom URL");

  this->_createTooltip(IDC_BC_CHK, L"Check dependencies availability");

  this->_createTooltip(IDC_BC_BRW08, L"Select new package snapshot");
  this->_createTooltip(IDC_BC_DEL, L"Remove package snapshot");

  this->_createTooltip(IDC_BC_BRW09, L"Load description text");
  this->_createTooltip(IDC_BC_SAV02, L"Save descriptions changes");

  // Set font for description
  HFONT hFt = Om_createFont(14, 400, L"Consolas");
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  // Set default package picture
  this->setStImage(IDC_SB_PKG, Om_getResImage(this->_hins, IDB_PKG_THN));
  // Set buttons icons
  this->setBmImage(IDC_BC_BRW02, Om_getResImage(this->_hins, IDB_BTN_ADD));
  this->setBmImage(IDC_BC_BRW03, Om_getResImage(this->_hins, IDB_BTN_DIR));
  this->setBmImage(IDC_BC_REM, Om_getResImage(this->_hins, IDB_BTN_REM));

  // Set snapshot format advice
  this->setItemText(IDC_SC_NOTES, L"Optimal format:\nSquare image of 128 x 128 pixels");

  // Initialize new Repository definition XML scheme
  this->_repInit();

  // Set default title and download path to controls
  OmXmlNode xml_def = this->_condig.xml();
  this->setItemText(IDC_EC_INP01, xml_def.child(L"title").content());
  this->setItemText(IDC_EC_INP02, xml_def.child(L"downpath").content());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onResize()
{
  unsigned half_w = static_cast<float>(this->width()) * 0.5f;

  // -- Left Frame --

  // New and Open.. Buttons
  this->_setItemPos(IDC_BC_NEW, 10, 10, 50, 14);
  this->_setItemPos(IDC_BC_BRW01, 65, 10, 50, 14);

  // [ - - -       Packages Title GroupBox        - - -
  this->_setItemPos(IDC_GB_GRP01, 5, 30, half_w-10, 45);
  // Title Label & EditText
  this->_setItemPos(IDC_SC_LBL01, 10, 40, 120, 9);
  this->_setItemPos(IDC_EC_INP01, 10, 55, half_w-20, 12);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Download path GroupBox        - - -
  this->_setItemPos(IDC_GB_GRP02, 5, 80, half_w-10, 45);
  // Download path Label & EditText
  this->_setItemPos(IDC_SC_LBL03, 10, 90, 120, 9);
  this->_setItemPos(IDC_EC_INP02, 10, 105, half_w-20, 12);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -       Packages List GroupBox        - - -
  this->_setItemPos(IDC_GB_GRP03, 5, 130, half_w-10, this->height()-185);
  // Packages list Label
  this->_setItemPos(IDC_SC_LBL02, 10, 147, 120, 9);
  // Packages list ListBox
  this->_setItemPos(IDC_LB_PKG, 10, 160, half_w-20, this->height()-223);
  // Add folder.. , Add... & Remove Buttons
  this->_setItemPos(IDC_BC_BRW02, half_w-125, 144, 40, 14);
  this->_setItemPos(IDC_BC_BRW03, half_w-80, 144, 40, 14);
  this->_setItemPos(IDC_BC_REM, half_w-26, 144, 16, 14);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]


  // Save As.. Button
  this->_setItemPos(IDC_BC_SAVE, 10, this->height()-45, 50, 14);

  // -- Right Frame --

  // [ - - -     Package Name & Url GroupBox      - - -
  this->_setItemPos(IDC_GB_GRP06, half_w+5, 5, half_w-10, 55);
  // Filename Label & EditText
  this->_setItemPos(IDC_SC_LBL06, half_w+10, 17, 54, 9);
  this->_setItemPos(IDC_EC_OUT02, half_w+65, 15, half_w-75, 12);
  // Custom Url CheckBox & EditText
  this->_setItemPos(IDC_BC_CHK01, half_w+10, 42, 54, 9);
  this->_setItemPos(IDC_EC_INP03, half_w+65, 40, half_w-120, 12);
  // Custom url Save Button
  this->_setItemPos(IDC_BC_SAV01, this->width()-50, 40, 40, 13);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Dependencies GroupBox         - - -
  this->_setItemPos(IDC_GB_GRP07, half_w+5, 65, half_w-10, 50);
  // Dependencies Label
  this->_setItemPos(IDC_SC_LBL07, half_w+10, 75, 54, 9);
  // Dependencies EditText & Check Button
  this->_setItemPos(IDC_EC_OUT04, half_w+65, 76, half_w-120, 30);
  this->_setItemPos(IDC_BC_CHK, this->width()-50, 75, 40, 13);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -          Snapshot GroupBox           - - -
  this->_setItemPos(IDC_GB_GRP08, half_w+5, 120, half_w-10, 95);
  // Snapshot Label
  this->_setItemPos(IDC_SC_LBL08, half_w+10, 130, 54, 9);
  // Snapshot Static Bitmap
  this->_setItemPos(IDC_SB_PKG, half_w+65, 131, 85, 78);
  // Change.. & Delete Buttons
  this->_setItemPos(IDC_BC_BRW08, this->width()-50, 130, 40, 13);
  this->_setItemPos(IDC_BC_DEL, this->width()-50, 145, 40, 13);
  // Snapshot helper Static text
  this->_setItemPos(IDC_SC_NOTES, half_w+165, 170, 60, 30);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // [ - - -        Description GroupBox          - - -
  this->_setItemPos(IDC_GB_GRP09, half_w+5, 220, half_w-10, this->height()-250);
  // Description Label
  this->_setItemPos(IDC_SC_LBL09, half_w+10, 232, 54, 9);
  // Load.. & Save Buttons
  this->_setItemPos(IDC_BC_BRW09, this->width()-95, 230, 40, 13);
  this->_setItemPos(IDC_BC_SAV02, this->width()-50, 230, 40, 13);
  // Description EditText
  this->_setItemPos(IDC_EC_PKTXT, half_w+10, 245, half_w-20, this->height()-280);
  // - - - - - - - - - - - - - - - - - - - - - - - - - ]

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Close button
  this->_setItemPos(IDC_BC_CLOSE, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_ADDENTRIES_DONE is a custom message sent from add entries thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_ADDENTRIES_DONE) {
    // end the removing Location process
    this->_addDir_stop();
  }

  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    switch(LOWORD(wParam))
    {
    case IDC_BC_NEW:
      this->_onBcNew();
      break;

    case IDC_BC_BRW01:
      this->_onBcOpen();
      break;

    case IDC_EC_INP01: //< Repository title EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
      break;

    case IDC_EC_INP02: //< Download path EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        has_changed = true;
      break;

    case IDC_BC_BRW02: //< Add Package Button
      this->_onBcBrwPkg();
      break;

    case IDC_BC_BRW03: //< Add Package folder Button
      this->_onBcBrwDir();
      break;

    case IDC_BC_REM:
      this->_onBcRemPkg();
      break;

    case IDC_LB_PKG: //< Packages list ListBox
      // check for selection change
      if(HIWORD(wParam) == LBN_SELCHANGE)
        this->_onLbPkglsSel();
      break;

    case IDC_BC_CHK01: // Custom Url CheckBox
      if(this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
        this->enableItem(IDC_EC_INP03, true);
      } else {
        this->setItemText(IDC_EC_INP03, L"");
        this->_onBcSavUrl();
        this->enableItem(IDC_EC_INP03, false);
      }
      break;

    case IDC_EC_INP03:
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        this->enableItem(IDC_BC_SAV01, true);
      break;

    case IDC_BC_SAV01: //< Custom Url "Save" Button
      this->_onBcSavUrl();
      break;

    case IDC_BC_CHK: //< Dependencies "Check" Button
      this->_onBcChkDeps();
      break;

    case IDC_BC_BRW08: //< Snapshot "Select..." Button
      this->_onBcBrwSnap();
      break;

    case IDC_BC_DEL: //< Snapshot "Delete" Button
      this->_onBcDelSnap();
      break;

    case IDC_BC_BRW09: //< Description "Load.." Button
      this->_onBcBrwDesc();
      break;

    case IDC_EC_PKTXT: //< Description EditText
      // check for content changes
      if(HIWORD(wParam) == EN_CHANGE)
        this->enableItem(IDC_BC_SAV02, true); //< enable Description "Save" Button
      break;

    case IDC_BC_SAV02: //< Description "Save" Button
      this->_onBcSavDesc();
      break;

    case IDC_BC_SAVE: //< General "Save as.." Button
      this->_onBcSave();
      break;

    case IDC_BC_CLOSE: //< General "Close" Button
      this->_onBcClose();
      break;
    }

    if(has_changed) {
      // check whether ListBox have remote to be saved
      int lb_cnt = this->msgItem(IDC_LB_PKG, LB_GETCOUNT, 0, 0);
      this->enableItem(IDC_BC_SAVE, (lb_cnt > 0));
    }
  }

  return false;
}

