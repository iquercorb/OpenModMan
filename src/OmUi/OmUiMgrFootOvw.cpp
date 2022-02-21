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
  #include <RichEdit.h>

#include "OmManager.h"
#include "OmPackage.h"
#include "OmRemote.h"

#include "OmUiMgr.h"

#include "OmUtilWin.h"
#include "OmUtilStr.h"
#include "OmUtilDlg.h"

#include "md4c-rtf/md4c-rtf.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiMgrFootOvw.h"

/// \brief MD4C parser options
///
/// Global options flag set for MD4C parser
///
#define MD4C_OPTIONS  MD_FLAG_UNDERLINE|MD_FLAG_TABLES|MD_FLAG_PERMISSIVEAUTOLINKS|MD_FLAG_NOHTML

/// \brief Callback for Markdown to RTF parser
///
/// Callback for MD4C RTF parser/renderer used to receive and store
/// rendered RTF data
///
/// \param[in]  data  : Pointer to RTF data.
/// \param[in]  size  : Size of RTF data.
/// \param[in]  ptr   : Pointer to user data.
///
static void __md2rtf_cb(const uint8_t* data, unsigned size, void* ptr)
{
  // get pointer to string
  string* str = reinterpret_cast<string*>(ptr);

  // contact new data to string
  str->append(reinterpret_cast<const char*>(data), size);
}

/// \brief Callback for Rich Edit stream
///
/// Callback Rich Edit input stream, used to send RTF data to
/// Rich Edit control.
///
/// \param[in]  ptr       : Pointer to user data.
/// \param[in]  buff      : Destination buffer where to write RTF data.
/// \param[in]  size      : Destination buffer size.
/// \param[out] writ      : Count of bytes actually written to buffer.
///
static DWORD CALLBACK __rtf2re_cb(DWORD_PTR ptr, LPBYTE buff, LONG size, LONG* writ)
{
  string* str = reinterpret_cast<string*>(ptr);

  LONG str_size = static_cast<LONG>(str->size());

  if(str_size) {

    if(size <= str_size) {

      memcpy(buff, str->data(), size);
      str->erase(0, size);
      *writ = size;

    } else {

      *writ = str_size;
      memcpy(buff, str->data(), str_size);
      str->clear();

    }

  } else {

    *writ = 0;
  }

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrFootOvw::OmUiMgrFootOvw(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrFootOvw::~OmUiMgrFootOvw()
{
  HFONT hFt;
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMgrFootOvw::id() const
{
  return IDD_MGR_FOOT_OVW;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootOvw::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootOvw::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::setPreview(OmPackage* pPkg)
{
  if(pPkg) {
    this->_showPreview(pPkg->name(), pPkg->version(), pPkg->image(), pPkg->desc(), !pPkg->isZip());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::setPreview(OmRemote* pRmt)
{
  if(pRmt) {
    this->_showPreview(pRmt->name(), pRmt->version(), pRmt->image(), pRmt->desc(), false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::clearPreview()
{
  this->showItem(IDC_SB_SNAP, false);
  this->showItem(IDC_FT_DESC, false); //< Rich Edit (MD parsed)
  this->showItem(IDC_EC_DESC, false); //< raw (plain text)
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::_showPreview(const wstring& name, const OmVersion& vers, const OmImage& snap, const wstring& desc, bool dir)
{
  if(desc.size()) {

    this->_renderText(desc, true);

  } else {

    wstring text;

    text.append(L"## ", 3);
    Om_escapeMarkdown(&text, name);
    if(!vers.isNull()) {
      text.append(L" - Version ", 11);
      Om_escapeMarkdown(&text, vers.asString());
    }

    if(dir) {
      text.append(L"\r\nThis folder does not have associated description text.");

    } else {
      text.append(L"\r\nThis Package does not embeds description text.");
    }

    this->_renderText(text, true);
  }

  this->showItem(IDC_SB_SNAP, true);

  HBITMAP hBm;

  if(snap.thumbnail()) {
    hBm = snap.thumbnail();
  } else {
    hBm = Om_getResImage(this->_hins, dir ? IDB_SNAP_DIR : IDB_SNAP_PKG);
  }

  // Update the selected picture
  hBm = this->setStImage(IDC_SB_SNAP, hBm);
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::_renderText(const wstring& text, bool show)
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);

  if(pMgr->noMarkdown()) {

    // set raw description to edit control
    this->setItemText(IDC_EC_DESC, text);
    if(show) this->showItem(IDC_EC_DESC, true);
    this->showItem(IDC_FT_DESC, false);

  } else {

    // string as RTF data buffer
    string rtf_data;

    // parse MD and render to RTF
    md_rtf(text.data(), text.size(), __md2rtf_cb, &rtf_data, MD4C_OPTIONS, MD_RTF_FLAG_SKIP_UTF8_BOM, 11, 300);

    // send RTF data to Rich Edit
    EDITSTREAM es = {};
    es.pfnCallback = __rtf2re_cb;
    es.dwCookie    = reinterpret_cast<DWORD_PTR>(&rtf_data);

    this->msgItem(IDC_FT_DESC, EM_STREAMIN, SF_RTF, reinterpret_cast<LPARAM>(&es));

    // reset scroll position once done
    long pt[2] = {};
    this->msgItem(IDC_FT_DESC, EM_SETSCROLLPOS, 0, reinterpret_cast<LPARAM>(&pt));

    if(show) this->showItem(IDC_FT_DESC, true);
    this->showItem(IDC_EC_DESC, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::_onReLink(LPARAM lParam)
{
  wchar_t url[512];

  ENLINK* el = reinterpret_cast<ENLINK*>(lParam);

  TEXTRANGEW trw;
  trw.chrg.cpMin = el->chrg.cpMin;
  trw.chrg.cpMax = el->chrg.cpMax;
  trw.lpstrText = url;

  this->msgItem(IDC_FT_DESC, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&trw));

  if(Om_dlgBox_ynl(this->_hwnd, L"Open link", IDI_QRY, L"Open link",
                   L"Do you want to open this link in browser ?", url)) {

      ShellExecuteW(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootOvw::_onInit\n";
  #endif

  // retrieve main dialog
  this->_pUiMgr = static_cast<OmUiMgr*>(this->root());

  // set white background to fit tab background
  EnableThemeDialogTexture(this->_hwnd, ETDT_ENABLETAB);

  // Defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(21, 400, L"Ms Dlg Font");
  this->msgItem(IDC_SC_NAME, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);
  hFt = Om_createFont(16, 400, L"Consolas");
  this->msgItem(IDC_EC_DESC, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  this->showItem(IDC_SC_NAME, false);
  this->showItem(IDC_SB_SNAP, false);
  this->showItem(IDC_FT_DESC, false); //< Rich Edit (MD parsed)
  this->showItem(IDC_EC_DESC, false); //< raw (plain text)

  // set event mask for Rich Edit control to receive CFE_LINK notifications
  this->msgItem(IDC_FT_DESC, EM_SETEVENTMASK, 0, ENM_LINK);
  this->msgItem(IDC_FT_DESC, EM_AUTOURLDETECT,  AURL_ENABLEURL|AURL_ENABLEEAURLS|
                                                AURL_ENABLEEMAILADDR, 0);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootOvw::_onShow\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::_onResize()
{
  // Package name/title
  //this->_setItemPos(IDC_SC_NAME, 4, 2, this->cliUnitX()-58, 14);
  // Package snapshot
  this->_setItemPos(IDC_SB_SNAP, 4, 2, 84, 84);
  // Package description, (RTF then Raw)
  this->_setItemPos(IDC_FT_DESC, 96, 2, this->cliUnitX()-98, this->cliUnitY()-4);
  this->_setItemPos(IDC_EC_DESC, 96, 2, this->cliUnitX()-98, this->cliUnitY()-4);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootOvw::_onRefresh\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootOvw::_onQuit\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMgrFootOvw::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {

    if(LOWORD(wParam) == IDC_FT_DESC) { //< Rich Edit (MD parsed)

      if(reinterpret_cast<NMHDR*>(lParam)->code == EN_LINK) {

        if(reinterpret_cast<ENLINK*>(lParam)->msg == WM_LBUTTONUP) {
          this->_onReLink(lParam);
        }
      }
    }
  }

  if(uMsg == WM_COMMAND) {
    #ifdef DEBUG
    //std::cout << "DEBUG => OmUiMgrFootOvw::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif
  }

  return false;
}
