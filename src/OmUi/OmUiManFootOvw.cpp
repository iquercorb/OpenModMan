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
  #include <cstdlib>     /* strtoul */

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
#include "OmUiManFootOvw.h"

/// \brief static MD2RTF Context
///
/// Static MD2RTF Context structure for Markdown to RTF parsing and render
///
static OM_MD2RTF_CTX __md2rtf_ctx;

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManFootOvw::OmUiManFootOvw(HINSTANCE hins) : OmDialog(hins),
  _UiMan(nullptr)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManFootOvw::~OmUiManFootOvw()
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
long OmUiManFootOvw::id() const
{
  return IDD_MGR_FOOT_OVW;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootOvw::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootOvw::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootOvw::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootOvw::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootOvw::setPreview(OmModPack* ModPack)
{
  if(ModPack)
    this->_overview_populate(ModPack->name(), ModPack->version(), ModPack->thumbnail(), ModPack->description(), ModPack->sourceIsDir());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootOvw::setPreview(OmNetPack* NetPack)
{
  if(NetPack)
    this->_overview_populate(NetPack->name(), NetPack->version(), NetPack->thumbnail(), NetPack->description(), false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootOvw::clearPreview()
{
  this->showItem(IDC_SB_SNAP, false);
  this->showItem(IDC_FT_DESC, false); //< Rich Edit (MD parsed)
  this->showItem(IDC_EC_DESC, false); //< raw (plain text)

  // Clear MD2RTF context, prevent working on invisible data
  Om_md2rtf_clear(&__md2rtf_ctx);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootOvw::_overview_populate(const OmWString& name, const OmVersion& vers, const OmImage& snap, const OmWString& desc, bool dir)
{
  if(desc.size()) {

    this->_desc_set_text(desc);

  } else {

    OmWString text;

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

    this->_desc_set_text(text);
  }

  this->showItem(IDC_SB_SNAP, true);

  HBITMAP hBm;

  if(snap.hbmp()) {
    hBm = snap.hbmp();
  } else {
    hBm = Om_getResImage(this->_hins, dir ? IDB_SNAP_DIR : IDB_SNAP_PKG);
  }

  // Update the selected picture
  hBm = this->setStImage(IDC_SB_SNAP, hBm);

  // Properly delete unused image
  if(hBm) {
    if(hBm != Om_getResImage(this->_hins, IDB_SNAP_DIR) &&
       hBm != Om_getResImage(this->_hins, IDB_SNAP_PKG)) DeleteObject(hBm);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootOvw::_desc_set_text(const OmWString& text)
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
void OmUiManFootOvw::_ft_desc_on_link(LPARAM lParam)
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
void OmUiManFootOvw::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootOvw::_onInit\n";
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

  this->showItem(IDC_SC_NAME, false);
  this->showItem(IDC_SB_SNAP, false);
  this->showItem(IDC_FT_DESC, false); //< Rich Edit (MD parsed)
  this->showItem(IDC_EC_DESC, false); //< raw (plain text)

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
void OmUiManFootOvw::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootOvw::_onShow\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootOvw::_onResize()
{
  // Package name/title
  //this->_setItemPos(IDC_SC_NAME, 4, 2, this->cliUnitX()-58, 14);
  // Package snapshot
  this->_setItemPos(IDC_SB_SNAP, 4, 2, 84, 84);
  // Package description, (RTF then Raw)
  this->_setItemPos(IDC_FT_DESC, 96, 2, this->cliUnitX()-98, this->cliUnitY()-4);
  this->_setItemPos(IDC_EC_DESC, 96, 2, this->cliUnitX()-98, this->cliUnitY()-4);

  // Adjust RTF document tables width to fit control new width
  HWND hEdit = this->getItem(IDC_FT_DESC);
  if(Om_md2rtf_autofit(&__md2rtf_ctx, hEdit)) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiManFootOvw::_onResize : Om_md2rtf_stream\n";
    #endif
    // we stream-in RTF data only if changes were made
    Om_md2rtf_stream(&__md2rtf_ctx, hEdit);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootOvw::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootOvw::_onRefresh\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootOvw::_onQuit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootOvw::_onQuit\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiManFootOvw::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
    //std::cout << "DEBUG => OmUiManFootOvw::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif
  }

  return false;
}
