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
#include "OmBase.h"           //< string, vector, Om_alloc, OM_MAX_PATH, etc.

#include "OmBaseWin.h"
#include <ShlObj.h>           //< BROWSEINFOW, etc.
#include <ShlWApi.h>          //< PathFileExistW, etc.

#include "OmBaseUi.h"
#include "OmBaseApp.h"        //< OM_APP_NAME, etc.

#include "OmUtilWin.h"   //< Om_getErrorStr
#include "OmUtilStr.h"   //< Om_isValidName, etc.

/// \brief IDs for message box.
///
/// Custom IDs for message box dialog controls.
///
#define OM_DLGBOX_BTN0      200
#define OM_DLGBOX_BTN1      201
#define OM_DLGBOX_SC_RECT   300
#define OM_DLGBOX_SC_ICON   301
#define OM_DLGBOX_SC_HEAD   400
#define OM_DLGBOX_SC_MESG   401
#define OM_DLGBOX_SC_LIST   402

/// \brief Fonts for custom message box.
///
/// Custom fonts for message box dialog controls.
///
static HFONT __Om_dlgBox_FontB = nullptr;
static HFONT __Om_dlgBox_FontS = nullptr;
static HICON __Om_dlgBox_TIcon = nullptr;

/// \brief Dialog Procedure function for message box.
///
/// Custom Dialog Procedure function for custom message box.
///
static INT_PTR CALLBACK __Om_dlgBox_dlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_INITDIALOG) {

    HWND hItem;
    wchar_t* str_buf;
    int str_len;
    long rect[4] = {0,0,0,0};
    long X, Y, xalign, yalign = 25;
    HWND hParent = GetParent(hWnd);
    UINT uSwp;

    if(__Om_dlgBox_TIcon == nullptr)
      __Om_dlgBox_TIcon = Om_getResIcon(reinterpret_cast<HINSTANCE>(lParam), IDI_APP, 1);

    // set title bar icon
    SendMessageW(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(__Om_dlgBox_TIcon));

    // make sure icon is 32 pixels wide
    hItem = GetDlgItem(hWnd, OM_DLGBOX_SC_ICON);
    SetWindowPos(hItem, nullptr, 25, yalign, 32, 32, SWP_NOZORDER|SWP_NOACTIVATE);

    // create header font
    if(__Om_dlgBox_FontB == nullptr)
      __Om_dlgBox_FontB = CreateFontW(20,0,0,0,400,false,false,false,
                                      ANSI_CHARSET,OUT_TT_PRECIS,0,
                                      CLEARTYPE_QUALITY,0,L"Ms Shell Dlg");

    // create general font
    if(__Om_dlgBox_FontS == nullptr)
      __Om_dlgBox_FontS = CreateFontW(16,0,0,0,400,false,false,false,
                                      ANSI_CHARSET,OUT_TT_PRECIS,0,
                                      CLEARTYPE_QUALITY,0,L"Ms Shell Dlg");

    // create and setup HDC for DrawText
    HDC hDc = GetDC(hWnd);
    HGDIOBJ hOldFont = SelectObject(hDc, __Om_dlgBox_FontS);

    // Static Control - Header/Title
    hItem = GetDlgItem(hWnd, OM_DLGBOX_SC_HEAD);
    if(hItem) {
      // set font for this control
      SendMessageW(hItem, WM_SETFONT, reinterpret_cast<WPARAM>(__Om_dlgBox_FontB), true);
      // move and resize control
      SetWindowPos(hItem, nullptr, 70, yalign, 400, 20, SWP_NOZORDER|SWP_NOACTIVATE);
      yalign += 30;
    }

    // Static Control - Main message
    hItem = GetDlgItem(hWnd, OM_DLGBOX_SC_MESG);
    if(hItem) {

      // set font for this control
      SendMessageW(hItem, WM_SETFONT, reinterpret_cast<WPARAM>(__Om_dlgBox_FontS), true);

      // retrieve control inner text string
      str_len = SendMessageW(hItem, WM_GETTEXTLENGTH, 0, 0);
      str_buf = new wchar_t[str_len+1];
      SendMessageW(hItem, WM_GETTEXT , str_len+1, reinterpret_cast<LPARAM>(str_buf));

      // use DrawText to calculate proper control height
      rect[2] = 400; //< initial width
      DrawTextW(hDc, str_buf, -1, reinterpret_cast<LPRECT>(&rect), DT_WORDBREAK|DT_CALCRECT);
      delete[] str_buf;

      // move and resize control
      SetWindowPos(hItem, nullptr, 70, yalign, 400, rect[3], SWP_NOZORDER|SWP_NOACTIVATE);
      yalign += rect[3] + 10;
    }

    // Static Control - Item List
    hItem = GetDlgItem(hWnd, OM_DLGBOX_SC_LIST);
    if(hItem) {

      // set font for this control
      SendMessageW(hItem, WM_SETFONT, reinterpret_cast<WPARAM>(__Om_dlgBox_FontS), true);

      // retrieve control inner text string
      str_len = SendMessageW(hItem, WM_GETTEXTLENGTH, 0, 0);
      str_buf = new wchar_t[str_len+1];
      SendMessageW(hItem, WM_GETTEXT , str_len+1, reinterpret_cast<LPARAM>(str_buf));

      // use DrawText to calculate proper control height
      rect[2] = 370; //< initial width
      DrawTextW(hDc, str_buf, -1, reinterpret_cast<LPRECT>(&rect), DT_CALCRECT);
      delete[] str_buf;

      // if list exceed size, we clamp and add a vertical scroll
      if(rect[3] > 64) {
        rect[3] = 64;
        SetWindowLongPtr(hItem,GWL_STYLE,GetWindowLongPtr(hItem,GWL_STYLE)|WS_VSCROLL);
      }

      // move and resize control
      SetWindowPos(hItem, nullptr, 80, yalign, 370, rect[3], SWP_NOZORDER|SWP_NOACTIVATE);
      yalign += rect[3] + 10;
    }

    // we do not need the HDC anymore
    SelectObject(hDc, hOldFont);
    ReleaseDC(hWnd, hDc);

    // resize white rect
    hItem = GetDlgItem(hWnd, OM_DLGBOX_SC_RECT);
    yalign += 20;
    SetWindowPos(hItem, nullptr, 0, 0, 500, yalign, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);

    // button initial position
    yalign += 11;
    xalign = 394;

    // if present, move the No/Cancel button to the left of dialog
    hItem = GetDlgItem(hWnd, OM_DLGBOX_BTN0);
    if(hItem) {
      SetWindowPos(hItem, nullptr, xalign, yalign, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
      xalign -= 95; // shift position for the next button
    }

    // move the OK/Yes button button
    hItem = GetDlgItem(hWnd, OM_DLGBOX_BTN1);
    SetWindowPos(hItem, nullptr, xalign, yalign, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);

    // finaly resize the dialog and center to parent client
    yalign += 64;
    uSwp = SWP_NOZORDER|SWP_NOACTIVATE;
    if(hParent) {
      // center to parent client
      GetClientRect(hParent, reinterpret_cast<LPRECT>(&rect));
      ClientToScreen(hParent, reinterpret_cast<LPPOINT>(&rect));
      X = rect[0] + (rect[2]*0.5f) - 250;
      Y = rect[1] + (rect[3]*0.5f) - (yalign * 0.5f);
    } else {
      // ignore position
      uSwp |= SWP_NOMOVE;
    }
    SetWindowPos(hWnd, nullptr, X, Y, 500, yalign, uSwp);
  }

  if(uMsg == WM_CTLCOLORSTATIC) {
    // set white background for static controls
    SetBkMode(reinterpret_cast<HDC>(wParam),TRANSPARENT);
    SetBkColor(reinterpret_cast<HDC>(wParam), 0x00FFFFFF);

    if(reinterpret_cast<HWND>(lParam) == GetDlgItem(hWnd, OM_DLGBOX_SC_HEAD)) {
      SetTextColor(reinterpret_cast<HDC>(wParam), 0x00993322);
    }

    return (INT_PTR)GetStockObject(WHITE_BRUSH);
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case OM_DLGBOX_BTN1:
      // exit dialog and return 1
      EndDialog(hWnd, 1);
      break;

    case OM_DLGBOX_BTN0:
      // exit dialog and return 0
      EndDialog(hWnd, 0);
      break;
    }
  }

  if(uMsg == WM_CLOSE) {
    // exit dialog and return 0
    EndDialog(hWnd, 0);
  }

  return 0;
}

/// \brief Constant strings for message box.
///
/// Constant string for message box buttons and template.
///
static const wchar_t* __Om_dlgBox_str_OK = L"OK";
static const wchar_t* __Om_dlgBox_str_YE = L"Yes";
static const wchar_t* __Om_dlgBox_str_CO = L"Continue";
static const wchar_t* __Om_dlgBox_str_NO = L"No";
static const wchar_t* __Om_dlgBox_str_CA = L"Cancel";
static const wchar_t* __Om_dlgBox_str_AB = L"Abort";
static const wchar_t* __Om_dlgBox_str_FNT = L"Ms Shell Dlg";

/// \brief Message dialog box.
///
/// Create message dialog box with custom parameters. The function returns
/// value depending clicked button by user.
///
/// \param[in] hins   : Handle to instance.
/// \param[in] hwnd   : Handle to parent/owner window.
/// \param[in] cpt    : Caption string
/// \param[in] ico    : Message icon resource ID.
/// \param[in] hdr    : Message header/title string.
/// \param[in] msg    : Message body string.
/// \param[in] lst    : Optional message list of element.
/// \param[in] flags  : Parameters flags.
///
/// \return Zero if NO or CANCEL button was clicked, 1 if OK or YES button was clicked.
///
static INT_PTR __Om_dlgBox(HINSTANCE hins, HWND hwnd, const wchar_t* cpt, uint16_t ico, const wchar_t* hdr, const wchar_t* msg, const wchar_t* lst, unsigned flags)
{
  // configure buttons according params
  const wchar_t* fnt = __Om_dlgBox_str_FNT;
  const wchar_t* bt1 = __Om_dlgBox_str_OK;
  const wchar_t* bt0 = nullptr;
  if(flags & 0x1) { //< OM_DLGBOX_OC: OK - Cancel
    bt0 = __Om_dlgBox_str_CA;
  } else if(flags & 0x2) { //< OM_DLGBOX_YN: Yes - No
    bt1 = __Om_dlgBox_str_YE; bt0 = __Om_dlgBox_str_NO;
  } else if(flags & 0x4) { //< OM_DLGBOX_CA: Continue - Abort
    bt1 = __Om_dlgBox_str_CO; bt0 = __Om_dlgBox_str_AB;
  }
  bool has_bt0 = bt0 != nullptr;
  bool has_hdr = hdr ? wcslen(hdr) : false;
  bool has_msg = msg ? wcslen(msg) : false;
  bool has_lst = lst ? wcslen(lst) : false;

  // compute template size
  size_t tplSize = sizeof(DLGTEMPLATE) + 10;
  tplSize += (3 * sizeof(DLGITEMTEMPLATE)) + 32;
  tplSize += sizeof(wchar_t) * wcslen(cpt);
  tplSize += sizeof(wchar_t) * wcslen(fnt);
  tplSize += sizeof(wchar_t) * wcslen(bt1);
  if(has_bt0) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(bt0);
  if(has_hdr) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(hdr);
  if(has_msg) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(msg);
  if(has_lst) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(lst);

  DLGTEMPLATE* dlgt;
  DLGITEMTEMPLATE* itmt;
  uint16_t* pTpl;

  // allocate memory for new template
  dlgt = reinterpret_cast<DLGTEMPLATE*>(Om_alloc(tplSize));
  if(!dlgt) return -1;

  // main dialog template definition
  dlgt->style = DS_3DLOOK|DS_MODALFRAME|WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_SETFONT;
  if(!hwnd) dlgt->style |= DS_CENTER;
  dlgt->dwExtendedStyle = DS_SHELLFONT;
  dlgt->cdit = 0; //< item count
  dlgt->x = 0; dlgt->y = 0; dlgt->cx = 330; dlgt->cy = 150;
  pTpl = reinterpret_cast<uint16_t*>(dlgt + 1);
  *pTpl++ = 0; //< MENU
  *pTpl++ = 0; //< Dialog Class
  while((*reinterpret_cast<wchar_t*>(pTpl++) = *cpt++)); //< Caption string
  *pTpl++ = 8; //< FONT size
  while((*reinterpret_cast<wchar_t*>(pTpl++) = *fnt++)); //< Font name string

  long xalign = 235; //< first or alone button X position

  if(has_bt0) {
    // align to word boundary
    if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
    // Button Control for Cancel/No - Return 0
    itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
    itmt->id = OM_DLGBOX_BTN0;
    itmt->style = WS_CHILD|WS_VISIBLE;
    itmt->dwExtendedStyle = WS_EX_LEFT;
    itmt->x = xalign; itmt->y = 80; itmt->cx = 58; itmt->cy = 15;
    pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
    *pTpl++ = 0xFFFF; *pTpl++ = 0x0080; //< WC_BUTTON
    while((*reinterpret_cast<wchar_t*>(pTpl++) = *bt0++)); //< Btn text
    *pTpl++ = 0; //< No creation
    dlgt->cdit++; //< increment item count
    xalign -= 64; //< shift X position for second button
  }

  // align to word boundary
  if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
  // Button Control for OK/YES - Return 1
  itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
  itmt->id = OM_DLGBOX_BTN1;
  itmt->style = WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON;
  itmt->dwExtendedStyle = WS_EX_LEFT;
  itmt->x = xalign; itmt->y = 80; itmt->cx = 58; itmt->cy = 15;
  pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
  *pTpl++ = 0xFFFF; *pTpl++ = 0x0080; //< WC_BUTTON
  while((*reinterpret_cast<wchar_t*>(pTpl++) = *bt1++)); //< Btn text
  *pTpl++ = 0; //< No creation
  dlgt->cdit++; //< increment item count

  // align to word boundary
  if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
  // Static Control for white background
  itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
  itmt->id = OM_DLGBOX_SC_RECT;
  itmt->style = WS_CHILD|WS_VISIBLE|SS_WHITERECT;
  itmt->dwExtendedStyle = WS_EX_LEFT;
  itmt->x = 0; itmt->y = 0; itmt->cx = 400; itmt->cy = 50;
  pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
  *pTpl++ = 0xFFFF; *pTpl++ = 0x0082; //< WC_STATIC
  *pTpl++ = 0; //< No text
  *pTpl++ = 0; //< No creation
  dlgt->cdit++; //< increment item count

  // align to word boundary
  if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
  // Static Control for icon
  itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
  itmt->id = OM_DLGBOX_SC_ICON;
  itmt->style = WS_CHILD|WS_VISIBLE|SS_ICON;
  itmt->dwExtendedStyle = WS_EX_LEFT;
  itmt->x = 16; itmt->y = 12; itmt->cx = 25; itmt->cy = 24;
  pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
  *pTpl++ = 0xFFFF; *pTpl++ = 0x0082; //< WC_STATIC
  *pTpl++ = 0xFFFF; *pTpl++ = ico; //< Icon resource ID
  *pTpl++ = 0; //< No creation
  dlgt->cdit++; //< increment item count

  if(has_hdr) {
    // align to word boundary
    if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
    // Static Control for title/header
    itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
    itmt->id = OM_DLGBOX_SC_HEAD;
    itmt->style = WS_CHILD|WS_VISIBLE;
    itmt->dwExtendedStyle = WS_EX_LEFT;
    itmt->x = 45; itmt->y = 12; itmt->cx = 267; itmt->cy = 14;
    pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
    *pTpl++ = 0xFFFF; *pTpl++ = 0x0082; //< WC_STATIC
    while((*reinterpret_cast<wchar_t*>(pTpl++) = *hdr++));  //< Item text
    *pTpl++ = 0; //< No creation
    dlgt->cdit++; //< increment item count
  }

  if(has_msg) {
    // align to word boundary
    if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
    // Static Control main message
    itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
    itmt->id = OM_DLGBOX_SC_MESG;
    itmt->style = WS_CHILD|WS_VISIBLE;
    itmt->dwExtendedStyle = WS_EX_LEFT;
    itmt->x = 45; itmt->y = 30; itmt->cx = 220; itmt->cy = 0;
    pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
    *pTpl++ = 0xFFFF; *pTpl++ = 0x0082; //< WC_STATIC
    while((*reinterpret_cast<wchar_t*>(pTpl++) = *msg++));  //< Item text
    *pTpl++ = 0; //< No creation
    dlgt->cdit++; //< increment item count
  }

  if(has_lst) {
    // align to word boundary
    if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
    // Edit Control for item list
    itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
    itmt->id = OM_DLGBOX_SC_LIST;
    itmt->style = WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_READONLY|ES_AUTOHSCROLL;
    itmt->dwExtendedStyle = WS_EX_LEFT;
    itmt->x = 45; itmt->y = 50; itmt->cx = 255; itmt->cy = 0;
    pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
    *pTpl++ = 0xFFFF; *pTpl++ = 0x0081; //< WC_EDIT
    while((*reinterpret_cast<wchar_t*>(pTpl++) = *lst++)); //< Item text
    *pTpl++ = 0; //< No creation
    dlgt->cdit++; //< increment item count
  }

  INT_PTR result = DialogBoxIndirectParamW(hins, dlgt, hwnd, __Om_dlgBox_dlgProc, (LPARAM)hins);

  Om_free(dlgt);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_dlgBox(HINSTANCE hins, HWND hwnd, const wchar_t* cpt, uint16_t ico, const wchar_t* hdr, const wchar_t* msg, const wchar_t* lst, unsigned flags)
{
  return __Om_dlgBox(hins, hwnd, cpt, ico, hdr, msg, lst, flags);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgBox_ok(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

  __Om_dlgBox(hins, hwnd, cpt_buf, ico, hdr.c_str(), msg.c_str(), nullptr, 0x0); //< OM_DLGBOX_OK
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgBox_okl(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg, const OmWString& lst)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

  __Om_dlgBox(hins, hwnd, cpt_buf, ico, hdr.c_str(), msg.c_str(), lst.c_str(), 0x0); //< OM_DLGBOX_OK
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgBox_yn(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

  return (0 != __Om_dlgBox(hins, hwnd, cpt_buf, ico, hdr.c_str(), msg.c_str(), nullptr, 0x2)); //< OM_DLGBOX_YN
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgBox_ynl(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg, const OmWString& lst)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

  return (0 != __Om_dlgBox(hins, hwnd, cpt_buf, ico, hdr.c_str(), msg.c_str(), lst.c_str(), 0x2)); //< OM_DLGBOX_YN
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgBox_ca(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

  return (0 != __Om_dlgBox(hins, hwnd, cpt_buf, ico, hdr.c_str(), msg.c_str(), nullptr, 0x4)); //< OM_DLGBOX_CA
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgBox_cal(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg, const OmWString& lst)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

  return (0 != __Om_dlgBox(hins, hwnd, cpt_buf, ico, hdr.c_str(), msg.c_str(), lst.c_str(), 0x4)); //< OM_DLGBOX_CA
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgBox_err(const OmWString& cpt, const OmWString& hdr, const OmWString& msg)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  __Om_dlgBox(nullptr, nullptr, cpt_buf, 0x62, hdr.c_str(), msg.c_str(), nullptr, 0x0); //< OM_DLGBOX_OK
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgBox_wrn(const OmWString& cpt, const OmWString& hdr, const OmWString& msg)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  __Om_dlgBox(nullptr, nullptr, cpt_buf, 0x54, hdr.c_str(), msg.c_str(), nullptr, 0x0); //< OM_DLGBOX_OK
}


/// \brief Browse dialog callback.
///
/// Callback function for folder browse dialog window. It is used like a
/// WindowProc to select a default start folder when dialog window is
/// initialized.
///
INT CALLBACK __dialogBrowseDir_Proc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  if(uMsg == BFFM_INITIALIZED) { //< Brother dialog windows is initialized
    if(lpData != 0) SendMessageW(hWnd, BFFM_SETSELECTION, true, lpData);  //< set the selected folder
    if(lpData != 0) SendMessageW(hWnd, BFFM_SETEXPANDED, false, lpData);  //< set and expand the selected folder
  }

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgBrowseDir(OmWString& result, HWND hWnd, const wchar_t* title, const OmWString& start, bool captive, bool nonew)
{
  BROWSEINFOW bI = {};
  bI.hwndOwner = hWnd;
  bI.lpszTitle = title;
  bI.ulFlags = BIF_USENEWUI|BIF_RETURNONLYFSDIRS|BIF_VALIDATE;
  if(nonew) bI.ulFlags |= BIF_NONEWFOLDERBUTTON;

  PIDLIST_ABSOLUTE pIdl_start = nullptr;

  // this is the advanced way to use SHBrowseForFolderW, here we use a
  // callback function to handle the dialog window initialization, the "start"
  // path object will be passed as lParam to the callback with the
  // BFFM_INITIALIZED message.
  if(start.size()) {
    SHParseDisplayName(start.c_str(), nullptr, &pIdl_start, 0, nullptr); //< convert path string to LPITEMIDLIST
  }

  bI.pidlRoot = (captive) ? pIdl_start : 0;
  bI.lpfn = __dialogBrowseDir_Proc;
  bI.lParam = reinterpret_cast<LPARAM>(pIdl_start);

  bool suceess = false;

  LPITEMIDLIST pIdl;
  if((pIdl = SHBrowseForFolderW(&bI)) != nullptr) {

    wchar_t psz_path[OM_MAX_PATH];

    psz_path[0] = 0;
    if(SHGetPathFromIDListEx(pIdl, psz_path, OM_MAX_PATH, GPFIDL_DEFAULT)) {
      result = psz_path;
      suceess = true;
    }
  }

  CoTaskMemFree(pIdl);

  return suceess;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgOpenFile(OmWString& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const OmWString& start)
{
  wchar_t str_file[OM_MAX_PATH];

  OPENFILENAMEW ofn = {};
  ofn.lStructSize = sizeof(OPENFILENAMEW);

  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = filter;

  // Oyé oyé, dear me in the future trying to fix this...

  // lpstrInitialDir does not work as attended, since Windows 7 if it has the
  // same value as was passed the first time the application used an Open or
  // Save As dialog box, the path most recently selected by the user is used
  // as the initial directory.
  ofn.lpstrInitialDir = start.c_str();

  // As workaround for the lpstrInitialDir behavior, we could set lpstrFile
  // with an initial path with a wildcard as file name (eg. C:\folder\*.ext)
  // However, within the context of this function, this would require to add
  // an new argument specify the wildcard to set, or to parse the filter
  // string which would be such monstrous routine for a so little thing.
  ofn.lpstrFile = str_file;
  ofn.lpstrFile[0] = L'\0';

  ofn.nMaxFile = OM_MAX_PATH;

  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER|OFN_NONETWORKBUTTON|OFN_NOTESTFILECREATE;

  if(GetOpenFileNameW(&ofn)) {
    result = str_file;
    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgSaveFile(OmWString& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const OmWString& start)
{
  wchar_t str_file[OM_MAX_PATH];
  swprintf(str_file, OM_MAX_PATH, L"%ls", result.c_str());

  OPENFILENAMEW ofn = {};
  ofn.lStructSize = sizeof(OPENFILENAMEW);

  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = filter;

  ofn.lpstrFile = str_file;
  ofn.nMaxFile = OM_MAX_PATH;

  ofn.lpstrInitialDir = start.c_str();

  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER|OFN_NONETWORKBUTTON|OFN_NOTESTFILECREATE;

  if(GetSaveFileNameW(&ofn)) {
    result = str_file;
    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgCreateFolder(HWND hwnd, const OmWString& item, const OmWString& path)
{
  if(!PathFileExistsW(path.c_str())) {

    OmWString msg;

    msg = item + L" does not exists.\n\n  \"";
    msg += path + L"\"\n\nDo you want to create it ?";

    if(IDYES == MessageBoxW(hwnd, msg.c_str(), OM_APP_NAME, MB_YESNO|MB_ICONQUESTION)) {

      int result = SHCreateDirectoryExW(nullptr, path.c_str(), nullptr);
      if(result != 0) {

        msg = item + L" cannot be created.\n\n  \"";
        msg += path + L"\"\n\nError : " + Om_getErrorStr(result);

        MessageBoxW(hwnd, msg.c_str(), OM_APP_NAME, MB_OK|MB_ICONERROR);
        return false;
      }

    } else {
      return false;
    }
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgOverwriteFile(HWND hwnd, const OmWString& path)
{
  // check whether file already exists
  DWORD attr = GetFileAttributesW(path.c_str());

  if(attr != INVALID_FILE_ATTRIBUTES) {
    if(attr & FILE_ATTRIBUTE_DIRECTORY)
      return true; //< file does not exists, no overwriting possible
  } else {
    return true; //< file does not exists, no overwriting possible
  }

  return (IDYES == MessageBoxW(hwnd,
                              L"The file already exists. Do you want to overwrite it ?",
                              OM_APP_NAME, MB_YESNO|MB_ICONQUESTION));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgValidName(HWND hwnd, const OmWString& item, const OmWString& name)
{
  OmWString msg;

  if(!name.empty()) {

    if(!Om_isValidName(name)) {
      msg = L"Invalid " + item + L".\n\n  \"";
      msg += name + L"\"\n\nName cannot contain the following characters: / * ? \" < > | \\";
    }

  } else {
    msg = L"Invalid " + item + L".\n\n";
    msg += item + L" cannot be empty.";
  }

  if(!msg.empty()) {
    MessageBoxW(hwnd, msg.c_str(), OM_APP_NAME, MB_OK|MB_ICONWARNING);
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgValidPath(HWND hwnd, const OmWString& item, const OmWString& path)
{
  OmWString msg;

  if(!path.empty()) {

    if(!Om_isValidPath(path)) {
      msg = L"Invalid " + item + L".\n\n  \"";
      msg += path + L"\"\n\nPath cannot contain the following characters: / * ? \" < > |";
    }

  } else {
    msg = L"Invalid " + item + L".\n\n";
    msg += item + L" cannot be empty.";
  }

  if(!msg.empty()) {
    MessageBoxW(hwnd, msg.c_str(), OM_APP_NAME, MB_OK|MB_ICONWARNING);
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgValidDir(HWND hwnd, const OmWString& item,  const OmWString& path)
{
  // check whether file already exists
  DWORD attr = GetFileAttributesW(path.c_str());

  if(attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
    OmWString msg = L"Invalid" + item + L".\n\n\"";
    msg += L"\"\n\nFolder does not exists, " + item + L" must be an existing directory.";
    MessageBoxW(hwnd, msg.c_str(), OM_APP_NAME, MB_OK|MB_ICONWARNING);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgCloseUnsaved(HWND hwnd, const OmWString& cpt)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

  return (0 != __Om_dlgBox(hins, hwnd, cpt_buf, 804, L"Unsaved changes",
                           L"You made unsaved changes. Close without saving ?", nullptr, 0x2)); //< OM_DLGBOX_YN
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgResetUnsaved(HWND hwnd, const OmWString& cpt)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

  return (0 != __Om_dlgBox(hins, hwnd, cpt_buf, 804, L"Unsaved changes",
                           L"You made unsaved changes. Continue without saving ?", nullptr, 0x2)); //< OM_DLGBOX_YN
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgSaveSucces(HWND hwnd, const OmWString& cpt, const OmWString& hdr, const OmWString& item)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

  OmWString msg(item); msg.append(L" file was successfully saved.");

  __Om_dlgBox(hins, hwnd, cpt_buf, 803, hdr.c_str(), msg.c_str(), nullptr, 0x0); //< OM_DLGBOX_OK
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgSaveError(HWND hwnd, const OmWString& cpt, const OmWString& hdr, const OmWString& item, const OmWString& error)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

  OmWString msg(L"Saving "); msg.append(item); msg.append(L" file has failed:");

  __Om_dlgBox(hins, hwnd, cpt_buf, 801, hdr.c_str(), msg.c_str(), error.c_str(), 0x0); //< OM_DLGBOX_OK
}
