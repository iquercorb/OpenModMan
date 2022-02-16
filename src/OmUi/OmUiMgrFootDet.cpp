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
#include "OmUtilStr.h"

#include "md4c-rtf/md4c-rtf.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiMgrFootDet.h"

/// \brief MD4C parser options
///
/// Global options flag set for MD4C parser
///
#define MD4C_OPTIONS  MD_FLAG_UNDERLINE|MD_FLAG_TABLES|MD_FLAG_PERMISSIVEAUTOLINKS|MD_FLAG_NOHTML

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrFootDet::OmUiMgrFootDet(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr),
  _rtfBuff(nullptr), _rtfSize(0), _rtfRead(0), _rtfWrit(0),
  _txtBuff(nullptr), _txtSize(0), _txtWrit(0)
{

}

OmUiMgrFootDet::~OmUiMgrFootDet()
{
  if(this->_rtfBuff)
    Om_free(this->_rtfBuff);

  if(this->_txtBuff)
    Om_free(this->_txtBuff);

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
    // reset text buffer
    this->_txtWrit = 0;
    this->_txtCat(L"### ", 4);
    this->_txtCat(pPkg->name());
    this->_txtCat(L"\r\n* Identity : **", 17); this->_txtCat(pPkg->ident()); this->_txtCat(L"**", 2);
    this->_txtCat(L"\r\n* Core name : **", 18); this->_txtCat(pPkg->core()); this->_txtCat(L"**", 2);
    this->_txtCat(L"\r\n* Version : **", 16); this->_txtCat(pPkg->version().asString()); this->_txtCat(L"**", 2);
    if(pPkg->category().size()) {
      this->_txtCat(L"\r\n* Category : **", 17); this->_txtCat(pPkg->category()); this->_txtCat(L"**", 2);
    } else {
      this->_txtCat(L"\r\n* Category : **<undefined>**", 30);
    }

    this->_txtCat(L"\r\n#### Source", 13);
    if(pPkg->isZip()) {
      this->_txtCat(L"\r\n* Type : **Zip file**", 23);
    } else {
      this->_txtCat(L"\r\n* Type : **Folder**", 21);
    }
    this->_txtCat(L"\r\n* Location : **", 17); this->_txtCat(pPkg->srcPath()); this->_txtCat(L"**", 2);

    this->_txtCat(L"\r\n#### Dependencies", 19);
    if(pPkg->depCount()) {
      for(size_t i = 0; i < pPkg->depCount(); ++i) {
        this->_txtCat(L"\r\n* ", 4); this->_txtCat(pPkg->depGet(i));
      }
    } else {
      this->_txtCat(L"\r\n* <none>", 10);
    }

    this->_txtCat(L"\r\n#### Installation files", 25);
    for(size_t i = 0; i < pPkg->srcItemCount(); ++i) {
      if(pPkg->srcItemGet(i).type == PKGITEM_TYPE_F) {
        this->_txtCat(L"\r\n* ", 4);
        this->_txtCat(pPkg->srcItemGet(i).path);
      }
    }

    this->_renderText(true, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::setDetails(OmRemote* pRmt)
{
  if(pRmt) {

    // reset text buffer
    this->_txtWrit = 0;
    this->_txtCat(L"### ", 4);
    this->_txtCat(pRmt->name());
    this->_txtCat(L"\r\n* Identity : **", 17); this->_txtCat(pRmt->ident()); this->_txtCat(L"**", 2);
    this->_txtCat(L"\r\n* Core name : **", 18); this->_txtCat(pRmt->core()); this->_txtCat(L"**", 2);
    this->_txtCat(L"\r\n* Version : **", 16); this->_txtCat(pRmt->version().asString()); this->_txtCat(L"**", 2);
    if(pRmt->category().size()) {
      this->_txtCat(L"\r\n* Category : **", 17); this->_txtCat(pRmt->category()); this->_txtCat(L"**", 2);
    } else {
      this->_txtCat(L"\r\n* Category : **<undefined>**", 30);
    }

    this->_txtCat(L"\r\n#### Download", 15);
    this->_txtCat(L"\r\n* URL : **", 12); this->_txtCat(pRmt->urlGet(0)); this->_txtCat(L"**", 2);
    wstring kbytes = Om_formatSizeSysStr(pRmt->bytes(), true);
    this->_txtCat(L"\r\n* Size : **", 13); this->_txtCat(kbytes); this->_txtCat(L"**", 2);
    switch(pRmt->checksumType()) {
      case RMT_CHECKSUM_MD5:  this->_txtCat(L"\r\n* checksum (md5) : **", 23); break;
      case RMT_CHECKSUM_XXH:  this->_txtCat(L"\r\n* checksum (xxh) : **", 23); break;
      default:                this->_txtCat(L"\r\n* checksum : **", 17); break;
    }
    this->_txtCat(pRmt->checksum()); this->_txtCat(L"**", 2);

    this->_txtCat(L"\r\n#### Supersedes", 17);
    if(pRmt->supCount()) {
      for(size_t i = 0; i < pRmt->supCount(); ++i) {
        this->_txtCat(L"\r\n* ", 4); this->_txtCat(pRmt->supGet(i)->ident());
      }
    } else {
      this->_txtCat(L"\r\n* <none>", 10);
    }

    this->_txtCat(L"\r\n#### Dependencies", 19);
    if(pRmt->depCount()) {
      for(size_t i = 0; i < pRmt->depCount(); ++i) {
        this->_txtCat(L"\r\n* ", 4); this->_txtCat(pRmt->depGet(i));
      }
    } else {
      this->_txtCat(L"\r\n* <none>", 10);
    }

    this->_renderText(true, false);
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
void OmUiMgrFootDet::_md2rtf_cb(const uint8_t* data, unsigned size, void* ptr)
{
  OmUiMgrFootDet* self = reinterpret_cast<OmUiMgrFootDet*>(ptr);

  // allocate new buffer if required
  if(self->_rtfWrit + size > self->_rtfSize) {

    self->_rtfSize *= 2;
    self->_rtfBuff = reinterpret_cast<uint8_t*>(Om_realloc(self->_rtfBuff, self->_rtfSize));

    if(!self->_rtfBuff) {
      // that is a bad alloc error...
      return;
    }

    #ifdef DEBUG
    std::cout << "OmUiMgrFootDet::_md2rtf_cb realloc="<< self->_rtfSize <<" bytes\n";
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
DWORD CALLBACK OmUiMgrFootDet::_rtf2re_cb(DWORD_PTR ptr, LPBYTE lpBuff, LONG rb, LONG* prb)
{
  OmUiMgrFootDet* self = reinterpret_cast<OmUiMgrFootDet*>(ptr);

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
  std::cout << "OmUiMgrFootDet::_rtf2re_cb rb="<< *prb <<"\n";
  #endif

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::_txtCat(const wchar_t* text, size_t len)
{
  // allocate new buffer if required
  if(this->_txtWrit + len > this->_txtSize) {

    this->_txtSize *= 2;
    this->_txtBuff = reinterpret_cast<wchar_t*>(Om_realloc(this->_txtBuff, this->_txtSize * sizeof(wchar_t)));

    if(!this->_txtBuff) {
      // that is a bad alloc error...
      return;
    }

    #ifdef DEBUG
    std::cout << "OmUiMgrFootDet::_txtCat realloc="<< this->_txtSize <<" bytes\n";
    #endif
  }

  // contact new data
  memcpy(this->_txtBuff + this->_txtWrit, text, len * sizeof(wchar_t));

  // increment string length
  this->_txtWrit += len;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::_txtCat(const wstring& text)
{
  wchar_t esc_buf[1024];
  size_t esc_len;

  // escape string for markdown
  esc_len = Om_escapeMarkdown(esc_buf, text);

  // allocate new buffer if required
  if(this->_txtWrit + esc_len > this->_txtSize) {

    this->_txtSize *= 2;
    this->_txtBuff = reinterpret_cast<wchar_t*>(Om_realloc(this->_txtBuff, this->_txtSize * sizeof(wchar_t)));

    if(!this->_txtBuff) {
      // that is a bad alloc error...
      return;
    }

    #ifdef DEBUG
    std::cout << "OmUiMgrFootDet::_txtCat realloc="<< this->_txtSize <<" bytes\n";
    #endif
  }

  // contact new data
  memcpy(this->_txtBuff + this->_txtWrit, esc_buf, esc_len * sizeof(wchar_t));

  // increment string length
  this->_txtWrit += esc_len;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrFootDet::_renderText(bool show, bool raw)
{
  if(raw) {

    // set raw description to edit control
    this->setItemText(IDC_EC_DESC, this->_txtBuff);
    if(show) this->showItem(IDC_EC_DESC, true);
    this->showItem(IDC_FT_DESC, false);

  } else {

    // reset RTF buffers parameters
    this->_rtfWrit = 0; //< bytes written by MD parser
    this->_rtfRead = 0; //< bytes read by Rich Edit stream

    // parse MD and render to RTF
    md_rtf(this->_txtBuff, this->_txtWrit, this->_md2rtf_cb, this, MD4C_OPTIONS, 0, 11, 300);

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

  // Allocate new buffer for RTF data
  this->_rtfSize = 4096;
  this->_rtfBuff = reinterpret_cast<uint8_t*>(Om_alloc(4096));

  // Allocate new buffer for Text data
  this->_txtSize = 4096;
  this->_txtBuff = reinterpret_cast<wchar_t*>(Om_alloc(this->_txtSize * sizeof(wchar_t)));
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

  if(uMsg == WM_COMMAND) {
    #ifdef DEBUG
    //std::cout << "DEBUG => OmUiMgrFootDet::_onMsg : WM_COMMAND=" << LOWORD(wParam) << "\n";
    #endif
  }

  return false;
}


