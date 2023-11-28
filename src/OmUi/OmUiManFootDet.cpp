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

#include "OmModMan.h"
#include "OmModPack.h"
#include "OmNetPack.h"

#include "OmUiMan.h"

#include "OmUtilWin.h"
#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilRtf.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiManFootDet.h"

/// \brief static MD2RTF Context
///
/// Static MD2RTF Context structure for Markdown to RTF parsing and render
///
static OM_MD2RTF_CTX __md2rtf_ctx;

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManFootDet::OmUiManFootDet(HINSTANCE hins) : OmDialog(hins),
  _UiMan(nullptr)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManFootDet::~OmUiManFootDet()
{
  HFONT hFt;
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_DESC, WM_GETFONT));
  if(hFt) DeleteObject(hFt);

  // Free Markdown render and RTF allocated data
  Om_md2rtf_free(&__md2rtf_ctx);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiManFootDet::id() const
{
  return IDD_MGR_FOOT_DET;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootDet::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootDet::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::setDetails(OmModPack* ModPack)
{
  if(ModPack) {

    OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

    bool raw = ModMan->noMarkdown();

    OmWString text;

    text.append(L"### ", 4);
    if(raw) text.append(ModPack->name()); else Om_escapeMarkdown(&text, ModPack->name());
    text.append(L"\r\n", 2);

    text.append(L"\r\n  - Identity : ", 17);
    if(raw) text.append(ModPack->iden()); else Om_escapeMarkdown(&text, ModPack->iden());

    text.append(L"\r\n  - Core name : ", 18);
    if(raw) text.append(ModPack->core()); else Om_escapeMarkdown(&text, ModPack->core());

    text.append(L"\r\n  - Version : ", 16);
    text.append(ModPack->version().asString());

    if(ModPack->category().size()) {
      text.append(L"\r\n  - Category : ", 17);
      text.append(ModPack->category());
    } else {
      text.append(L"\r\n  - Category : <undefined>", 28);
    }

    text.append(L"\r\n\r\n#### Source\r\n", 17);

    if(ModPack->sourceIsDir()) {
      text.append(L"\r\n  - Type : Directory", 22);
    } else {
      text.append(L"\r\n  - Type : Archive file", 25);
    }

    text.append(L"\r\n  - Location : ", 17);
    if(raw) text.append(ModPack->sourcePath()); else Om_escapeMarkdown(&text, ModPack->sourcePath());

    text.append(L"\r\n\r\n#### Dependencies\r\n", 23);

    if(ModPack->dependCount()) {
      if(raw) {
        for(size_t i = 0; i < ModPack->dependCount(); ++i) {
          text.append(L"\r\n  - ", 6); text.append(ModPack->getDependIden(i));
        }
      } else {
        for(size_t i = 0; i < ModPack->dependCount(); ++i) {
          text.append(L"\r\n  - ", 6); Om_escapeMarkdown(&text, ModPack->getDependIden(i));
        }
      }
    } else {
      text.append(L"\r\n  - <none>", 12);
    }

    text.append(L"\r\n\r\n#### Installation files\r\n", 29);

    for(size_t i = 0; i < ModPack->sourceEntryCount(); ++i) {
      bool isdir = OM_HAS_BIT(ModPack->getSourceEntry(i).attr, OM_MODENTRY_DIR);
      if(raw) {
        if(!isdir) {
          text.append(L"\r\n  - ", 6); text.append(ModPack->getSourceEntry(i).path);
        }
      } else {
        if(!isdir) {
          text.append(L"\r\n  - ", 6); Om_escapeMarkdown(&text, ModPack->getSourceEntry(i).path);
        }
      }
    }

    this->_desc_set_text(text);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::setDetails(OmNetPack* NetPack)
{
  if(NetPack) {

    OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

    bool raw = ModMan->noMarkdown();

    OmWString text;

    text.append(L"### ", 4);
    if(raw) text.append(NetPack->name()); else Om_escapeMarkdown(&text, NetPack->name());
    text.append(L"\r\n", 2);

    text.append(L"\r\n  - Identity : ", 17);
    if(raw) text.append(NetPack->iden()); else Om_escapeMarkdown(&text, NetPack->iden());

    text.append(L"\r\n  - Core name : ", 18);
    if(raw) text.append(NetPack->core()); else Om_escapeMarkdown(&text, NetPack->core());

    text.append(L"\r\n  - Version : ", 16);
    text.append(NetPack->version().asString());

    if(NetPack->category().size()) {
      text.append(L"\r\n  - Category : ", 17);
      text.append(NetPack->category());
    } else {
      text.append(L"\r\n  - Category : <undefined>", 28);
    }

    text.append(L"\r\n\r\n#### Download\r\n", 19);

    text.append(L"\r\n  - URL : ", 12);
    text.append(NetPack->fileUrl());

    text.append(L"\r\n  - Size : ", 13);
    OmWString kbytes = Om_formatSizeSysStr(NetPack->fileSize(), true);
    text.append(kbytes);

    if(NetPack->checksumIsMd5()) {
      text.append(L"\r\n  - checksum (md5) : ", 23);
    } else {
      text.append(L"\r\n  - checksum (xxh) : ", 23);
    }
    text.append(NetPack->fileChecksum());

    text.append(L"\r\n\r\n#### Supersedes\r\n", 21);

    if(NetPack->upgradableCount()) {
      if(raw) {
        for(size_t i = 0; i < NetPack->upgradableCount(); ++i) {
          text.append(L"\r\n  - ", 6); text.append(NetPack->getUpgradable(i)->iden());
        }
      } else {
        for(size_t i = 0; i < NetPack->upgradableCount(); ++i) {
          text.append(L"\r\n  - ", 6); Om_escapeMarkdown(&text, NetPack->getUpgradable(i)->iden());
        }
      }
    } else {
      text.append(L"\r\n  - <none>", 12);
    }

    text.append(L"\r\n\r\n#### Dependencies\r\n", 23);

    if(NetPack->dependCount()) {
      if(raw) {
        for(size_t i = 0; i < NetPack->dependCount(); ++i) {
          text.append(L"\r\n  - ", 6); text.append(NetPack->getDependIden(i));
        }
      } else {
        for(size_t i = 0; i < NetPack->dependCount(); ++i) {
          text.append(L"\r\n  - ", 6); Om_escapeMarkdown(&text, NetPack->getDependIden(i));
        }
      }
    } else {
      text.append(L"\r\n  - <none>", 12);
    }

    this->_desc_set_text(text);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::clearDetails()
{
  this->showItem(IDC_EC_DESC, false);
  this->showItem(IDC_FT_DESC, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::_desc_set_text(const OmWString& text)
{
  OmModMan* ModMan = static_cast<OmModMan*>(this->_data);

  if(ModMan->noMarkdown()) {

    // set raw description to edit control
    this->setItemText(IDC_EC_DESC, text);

    this->showItem(IDC_EC_DESC, true);

    this->showItem(IDC_FT_DESC, false);

  } else {

    long rect[4];
    GetClientRect(this->getItem(IDC_FT_DESC), reinterpret_cast<LPRECT>(&rect));

    // Parse Markdown and render to RTF document
    Om_md2rtf_render(&__md2rtf_ctx, text, 11, rect[2]);

    // Stream-In RTF data to Rich Edit Control
    Om_md2rtf_stream(&__md2rtf_ctx, this->getItem(IDC_FT_DESC));

    // reset scroll position once done
    long pt[2] = {};
    this->msgItem(IDC_FT_DESC, EM_SETSCROLLPOS, 0, reinterpret_cast<LPARAM>(&pt));

    this->showItem(IDC_FT_DESC, true);

    this->showItem(IDC_EC_DESC, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::_ft_desc_on_link(LPARAM lParam)
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
void OmUiManFootDet::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootDet::_onInit\n";
  #endif

  // retrieve main dialog
  this->_UiMan = static_cast<OmUiMan*>(this->root());

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

  // Initialize Markdown To RTF Context
  Om_md2rtf_init(&__md2rtf_ctx);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootDet::_onShow\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::_onResize()
{
  // Package source files, (RTF then Raw)
  this->_setItemPos(IDC_FT_DESC, 4, 2, this->cliUnitX()-6, this->cliUnitY()-4);
  this->_setItemPos(IDC_EC_DESC, 4, 2, this->cliUnitX()-6, this->cliUnitY()-4);

  // Adjust RTF document tables width to fit control new width
  HWND hEdit = this->getItem(IDC_FT_DESC);
  if(Om_md2rtf_autofit(&__md2rtf_ctx, hEdit)) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiManFootDet::_onResize : RTF data changed.\n";
    #endif
    // we stream-in RTF data only if changes were made
    Om_md2rtf_stream(&__md2rtf_ctx, hEdit);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootDet::_onRefresh\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootDet::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootDet::_onQuit\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiManFootDet::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {

    if(LOWORD(wParam) == IDC_FT_DESC) { //< Rich Edit (MD parsed)

      if(reinterpret_cast<NMHDR*>(lParam)->code == EN_LINK) {

        if(reinterpret_cast<ENLINK*>(lParam)->msg == WM_LBUTTONUP) {
          this->_ft_desc_on_link(lParam);
        }
      }
    }
  }

  if(uMsg == WM_COMMAND) {
    #ifdef DEBUG
    //std::cout << "DEBUG => OmUiManFootDet::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif
  }

  return false;
}


