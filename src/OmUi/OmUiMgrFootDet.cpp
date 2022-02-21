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
#include "OmUiMgrFootDet.h"

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
OmUiMgrFootDet::OmUiMgrFootDet(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr)
{

}

OmUiMgrFootDet::~OmUiMgrFootDet()
{
  HFONT hFt;
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMgrFootDet::id() const
{
  return IDD_MGR_FOOT_DET;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootDet::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootDet::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::setDetails(OmPackage* pPkg)
{
  if(pPkg) {

    OmManager* pMgr = static_cast<OmManager*>(this->_data);

    bool raw = pMgr->noMarkdown();

    wstring text;

    text.append(L"### ", 4);
    if(raw) text.append(pPkg->name()); else Om_escapeMarkdown(&text, pPkg->name());
    text.append(L"\r\n", 2);

    text.append(L"\r\n  - Identity : ", 17);
    if(raw) text.append(pPkg->ident()); else Om_escapeMarkdown(&text, pPkg->ident());

    text.append(L"\r\n  - Core name : ", 18);
    if(raw) text.append(pPkg->core()); else Om_escapeMarkdown(&text, pPkg->core());

    text.append(L"\r\n  - Version : ", 16);
    text.append(pPkg->version().asString());

    if(pPkg->category().size()) {
      text.append(L"\r\n  - Category : ", 17);
      text.append(pPkg->category());
    } else {
      text.append(L"\r\n  - Category : <undefined>", 28);
    }

    text.append(L"\r\n\r\n#### Source\r\n", 17);

    if(pPkg->isZip()) {
      text.append(L"\r\n  - Type : Zip file", 21);
    } else {
      text.append(L"\r\n  - Type : Folder", 19);
    }

    text.append(L"\r\n  - Location : ", 17);
    if(raw) text.append(pPkg->srcPath()); else Om_escapeMarkdown(&text, pPkg->srcPath());

    text.append(L"\r\n\r\n#### Dependencies\r\n", 23);

    if(pPkg->depCount()) {
      if(raw) {
        for(size_t i = 0; i < pPkg->depCount(); ++i) {
          text.append(L"\r\n  - ", 6); text.append(pPkg->depGet(i));
        }
      } else {
        for(size_t i = 0; i < pPkg->depCount(); ++i) {
          text.append(L"\r\n  - ", 6); Om_escapeMarkdown(&text, pPkg->depGet(i));
        }
      }
    } else {
      text.append(L"\r\n  - <none>", 12);
    }

    text.append(L"\r\n\r\n#### Installation files\r\n", 29);

    for(size_t i = 0; i < pPkg->srcItemCount(); ++i) {
      if(raw) {
        if(pPkg->srcItemGet(i).type == PKGITEM_TYPE_F) {
          text.append(L"\r\n  - ", 6); text.append(pPkg->srcItemGet(i).path);
        }
      } else {
        if(pPkg->srcItemGet(i).type == PKGITEM_TYPE_F) {
          text.append(L"\r\n  - ", 6); Om_escapeMarkdown(&text, pPkg->srcItemGet(i).path);
        }
      }
    }

    this->_renderText(text, true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::setDetails(OmRemote* pRmt)
{
  if(pRmt) {

    OmManager* pMgr = static_cast<OmManager*>(this->_data);

    bool raw = pMgr->noMarkdown();

    wstring text;

    text.append(L"### ", 4);
    if(raw) text.append(pRmt->name()); else Om_escapeMarkdown(&text, pRmt->name());
    text.append(L"\r\n", 2);

    text.append(L"\r\n  - Identity : ", 17);
    if(raw) text.append(pRmt->ident()); else Om_escapeMarkdown(&text, pRmt->ident());

    text.append(L"\r\n  - Core name : ", 18);
    if(raw) text.append(pRmt->core()); else Om_escapeMarkdown(&text, pRmt->core());

    text.append(L"\r\n  - Version : ", 16);
    text.append(pRmt->version().asString());

    if(pRmt->category().size()) {
      text.append(L"\r\n  - Category : ", 17);
      text.append(pRmt->category());
    } else {
      text.append(L"\r\n  - Category : <undefined>", 28);
    }

    text.append(L"\r\n\r\n#### Download\r\n", 19);

    text.append(L"\r\n  - URL : ", 12);
    text.append(pRmt->urlGet(0));

    text.append(L"\r\n  - Size : ", 13);
    wstring kbytes = Om_formatSizeSysStr(pRmt->bytes(), true);
    text.append(kbytes);

    switch(pRmt->checksumType()) {
      case RMT_CHECKSUM_MD5:  text.append(L"\r\n  - checksum (md5) : ", 23); break;
      case RMT_CHECKSUM_XXH:  text.append(L"\r\n  - checksum (xxh) : ", 23); break;
      default:                text.append(L"\r\n  - checksum : ", 17); break;
    }
    text.append(pRmt->checksum());

    text.append(L"\r\n\r\n#### Supersedes\r\n", 21);

    if(pRmt->supCount()) {
      if(raw) {
        for(size_t i = 0; i < pRmt->supCount(); ++i) {
          text.append(L"\r\n  - ", 6); text.append(pRmt->supGet(i)->ident());
        }
      } else {
        for(size_t i = 0; i < pRmt->supCount(); ++i) {
          text.append(L"\r\n  - ", 6); Om_escapeMarkdown(&text, pRmt->supGet(i)->ident());
        }
      }
    } else {
      text.append(L"\r\n  - <none>", 12);
    }

    text.append(L"\r\n\r\n#### Dependencies\r\n", 23);

    if(pRmt->depCount()) {
      if(raw) {
        for(size_t i = 0; i < pRmt->depCount(); ++i) {
          text.append(L"\r\n  - ", 6); text.append(pRmt->depGet(i));
        }
      } else {
        for(size_t i = 0; i < pRmt->depCount(); ++i) {
          text.append(L"\r\n  - ", 6); Om_escapeMarkdown(&text, pRmt->depGet(i));
        }
      }
    } else {
      text.append(L"\r\n  - <none>", 12);
    }

    this->_renderText(text, true);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::clearDetails()
{
  this->showItem(IDC_EC_DESC, false);
  this->showItem(IDC_FT_DESC, false);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::_renderText(const wstring& text, bool show)
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
void OmUiMgrFootDet::_onReLink(LPARAM lParam)
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
void OmUiMgrFootDet::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootDet::_onInit\n";
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

  this->showItem(IDC_SC_NAME, true);
  this->showItem(IDC_EC_DESC, false);
  this->showItem(IDC_FT_DESC, true);

  // set event mask for Rich Edit control to receive CFE_LINK notifications
  this->msgItem(IDC_FT_DESC, EM_SETEVENTMASK, 0, ENM_LINK);
  this->msgItem(IDC_FT_DESC, EM_AUTOURLDETECT,  AURL_ENABLEURL|AURL_ENABLEEAURLS|
                                                AURL_ENABLEEMAILADDR, 0);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootDet::_onShow\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::_onResize()
{
  // Package source files, (RTF then Raw)
  this->_setItemPos(IDC_FT_DESC, 4, 2, this->cliUnitX()-6, this->cliUnitY()-4);
  this->_setItemPos(IDC_EC_DESC, 4, 2, this->cliUnitX()-6, this->cliUnitY()-4);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootDet::_onRefresh\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrFootDet::_onQuit\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMgrFootDet::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
    //std::cout << "DEBUG => OmUiMgrFootDet::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif
  }

  return false;
}


