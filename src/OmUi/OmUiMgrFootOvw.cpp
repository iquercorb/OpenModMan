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

#include "OmPackage.h"
#include "OmRemote.h"

#include "OmUiMgr.h"

#include "OmUtilWin.h"

#include "md4c-rtf/md4c-rtf.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiMgrFootOvw.h"

/// \brief MD4C parser options
///
/// Global options flag set for MD4C parser
///
#define MD4C_OPTIONS  MD_FLAG_UNDERLINE|MD_FLAG_TABLES|MD_FLAG_PERMISSIVEAUTOLINKS

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrFootOvw::OmUiMgrFootOvw(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr),
  _rtfBuff(nullptr), _rtfSize(0), _rtfRead(0), _rtfWrit(0),
  _rawDesc(false)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrFootOvw::~OmUiMgrFootOvw()
{
  if(this->_rtfBuff)
    Om_free(this->_rtfBuff);

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
    this->_showPreview(pPkg->image(), pPkg->desc());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::setPreview(OmRemote* pRmt)
{
  if(pRmt) {
    this->_showPreview(pRmt->image(), pRmt->desc());
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
void OmUiMgrFootOvw::_md2rtf_cb(const uint8_t* data, unsigned size, void* ptr)
{
  OmUiMgrFootOvw* self = reinterpret_cast<OmUiMgrFootOvw*>(ptr);

  // allocate new buffer if required
  if(self->_rtfWrit + size > self->_rtfSize) {

    self->_rtfSize *= 2;
    self->_rtfBuff = reinterpret_cast<uint8_t*>(Om_realloc(self->_rtfBuff, self->_rtfSize));

    if(!self->_rtfBuff) {
      // that is a bad alloc error...
      return;
    }

    #ifdef DEBUG
    std::cout << "OmUiMgrFootOvw::_md2rtf_cb realloc="<< self->_rtfSize <<" bytes\n";
    #endif
  }

  // contact new data
  memcpy(self->_rtfBuff + self->_rtfWrit, data, size);

  // increment bytes written
  self->_rtfWrit += size;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD CALLBACK OmUiMgrFootOvw::_rtf2re_cb(DWORD_PTR ptr, LPBYTE lpBuff, LONG rb, LONG* prb)
{
  OmUiMgrFootOvw* self = reinterpret_cast<OmUiMgrFootOvw*>(ptr);

  if(self->_rtfRead < self->_rtfWrit) {
    if((self->_rtfRead + rb) <= self->_rtfWrit) {

      memcpy(lpBuff, self->_rtfBuff + self->_rtfRead, rb);
      self->_rtfRead += rb;
      *prb = rb;

    } else {

      *prb = self->_rtfWrit - self->_rtfRead;
      memcpy(lpBuff, self->_rtfBuff + self->_rtfRead, *prb);
      self->_rtfRead = self->_rtfWrit;
    }

  } else {
    *prb = 0;
  }

  #ifdef DEBUG
  std::cout << "OmUiMgrFootOvw::_rtf2re_cb rb="<< *prb <<"\n";
  #endif

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootOvw::_renderText(const wstring& text, bool show, bool raw)
{
  if(raw) {

    // set raw description to edit control
    this->setItemText(IDC_EC_DESC, text);
    if(show) this->showItem(IDC_EC_DESC, true);
    this->showItem(IDC_FT_DESC, false);

  } else {

    // reset RTF buffers parameters
    this->_rtfWrit = 0; //< bytes written by MD parser
    this->_rtfRead = 0; //< bytes read by Rich Edit stream

    // parse MD and render to RTF
    md_rtf(text.c_str(), text.size(), this->_md2rtf_cb, this, MD4C_OPTIONS, 0, 11, 300);

    // send RTF data to Rich Edit
    EDITSTREAM es = {};
    es.pfnCallback = this->_rtf2re_cb;
    es.dwCookie    = reinterpret_cast<DWORD_PTR>(this);

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
void OmUiMgrFootOvw::_showPreview(const OmImage& snap, const wstring& desc)
{
  if(desc.size())
    this->_renderText(desc, true, this->_rawDesc);

  this->showItem(IDC_SB_SNAP, true);

  HBITMAP hBm;

  if(snap.thumbnail()) {
    hBm = snap.thumbnail();
  } else {
    hBm = Om_getResImage(this->_hins, IDB_BLANK);
  }

  // Update the selected picture
  hBm = this->setStImage(IDC_SB_SNAP, hBm);
  if(hBm && hBm != Om_getResImage(this->_hins, IDB_BLANK)) DeleteObject(hBm);
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

  // Allocate new buffer for RTF data
  this->_rtfSize = 4096;
  this->_rtfBuff = reinterpret_cast<uint8_t*>(Om_alloc(4096));
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

        ENLINK* el = reinterpret_cast<ENLINK*>(lParam);

        if(el->msg == WM_LBUTTONUP) {

          wchar_t url_buf[256];

          TEXTRANGEW txr;
          txr.chrg.cpMin = el->chrg.cpMin;
          txr.chrg.cpMax = el->chrg.cpMax;
          txr.lpstrText = url_buf;

          this->msgItem(IDC_FT_DESC, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&txr));

          #ifdef DEBUG
          std::wcout << L"DEBUG => OmUiMgrFootOvw::_onMsg WM_NOTIFY-EN_LINK: " << url_buf << "\n";
          #endif
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
