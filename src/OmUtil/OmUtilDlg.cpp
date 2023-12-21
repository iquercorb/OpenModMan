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
#include <commctrl.h>
#include <ShlObj.h>           //< BROWSEINFOW, etc.
#include <ShlWApi.h>          //< PathFileExistW, etc.

#include "OmBaseUi.h"
#include "OmBaseApp.h"        //< OM_APP_NAME, etc.

#include "OmUtilWin.h"   //< Om_getErrorStr
#include "OmUtilStr.h"   //< Om_hasLegalSysChar, etc.

/// \brief IDs for message box.
///
/// Custom IDs for message box dialog controls.
///
#define OM_DLGBOX_BTN0      200
#define OM_DLGBOX_BTN1      201
#define OM_DLGBOX_BTNX      202
#define OM_DLGBOX_SC_RECT   300
#define OM_DLGBOX_SC_ICON   301
#define OM_DLGBOX_SC_HEAD   400
#define OM_DLGBOX_SC_MESG   401
#define OM_DLGBOX_SC_LIST   402
#define OM_DLGBOX_PB_BAR0   403
#define OM_DLGBOX_PB_BAR1   404

/// \brief Fonts for custom message box.
///
/// Custom fonts for message box dialog controls.
///
static HFONT __Om_dlgBox_FontB = nullptr;
static HFONT __Om_dlgBox_FontS = nullptr;
static HICON __Om_dlgBox_TIcon = nullptr;

inline static void __Om_dlgBox_peekMessages(HWND hwnd)
{
  MSG msg;

  // force all dialog messages to be treated before return
  while(PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
    if(!IsDialogMessage(hwnd, &msg))
      DispatchMessage(&msg);
}

/// \brief Dialog Procedure function for message box.
///
/// Custom Dialog Procedure function for custom message box.
///
static INT_PTR CALLBACK __Om_dlgBox_dlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static int* result_ptr = nullptr;

  if(uMsg == WM_INITDIALOG) {

    HWND hItem;
    wchar_t* str_buf;
    int str_len;
    long rect[4] = {0,0,0,0};
    long X, Y, xalign, yalign = 25;
    HWND hParent = GetParent(hWnd);
    UINT uSwp;

    result_ptr = reinterpret_cast<int*>(lParam);

    if(__Om_dlgBox_TIcon == nullptr)
      __Om_dlgBox_TIcon = Om_getResIcon(IDI_APP, 1);

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

    // add little margin before progress bar
    yalign += 10;

    // default progress bar height
    rect[3] = 18;

    // Progress Bar control - Secondary progress bar
    hItem = GetDlgItem(hWnd, OM_DLGBOX_PB_BAR1);
    if(hItem) {
      rect[3] = 10; //< smaller progress bars
      // move and resize control
      SetWindowPos(hItem, nullptr, 30, yalign, 430, rect[3], SWP_NOZORDER|SWP_NOACTIVATE);
      yalign += rect[3] + 2;
    }

    // Progress Bar control - Main or Alone progress bar
    hItem = GetDlgItem(hWnd, OM_DLGBOX_PB_BAR0);
    if(hItem) {
      // move and resize control
      SetWindowPos(hItem, nullptr, 30, yalign, 430, rect[3], SWP_NOZORDER|SWP_NOACTIVATE);
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
    xalign = 394; // alignment for 2 buttons

    // if present, move the No/Cancel button to the left of dialog
    hItem = GetDlgItem(hWnd, OM_DLGBOX_BTNX);
    if(hItem) {
      SetWindowPos(hItem, nullptr, xalign, yalign, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
      xalign -= 95; // shift position for the next button
    }

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
      X = Y = 0;
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
      if(result_ptr) *result_ptr = 1;
      // exit dialog and return 1
      EndDialog(hWnd, 1);
      break;

    case OM_DLGBOX_BTN0:
      if(result_ptr) *result_ptr = 0;
      // exit dialog and return 0
      EndDialog(hWnd, 0);
      break;

    case OM_DLGBOX_BTNX:
      if(result_ptr) *result_ptr = -1;
      // exit dialog and return -1
      EndDialog(hWnd, -1);
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
  const wchar_t* btX = nullptr;
  if(flags & 0x1) { //< OM_DLGBOX_OC: OK - Cancel
    bt0 = __Om_dlgBox_str_CA;
  } else if(flags & 0x2) { //< OM_DLGBOX_YN: Yes - No
    bt1 = __Om_dlgBox_str_YE; bt0 = __Om_dlgBox_str_NO;
  } else if(flags & 0x4) { //< OM_DLGBOX_CA: Continue - Abort
    bt1 = __Om_dlgBox_str_CO; bt0 = __Om_dlgBox_str_AB;
  } else if(flags & 0x8) {
    bt1 = __Om_dlgBox_str_YE; bt0 = __Om_dlgBox_str_NO; btX = __Om_dlgBox_str_CA;
  }

  bool has_bt0 = bt0 != nullptr;
  bool has_btX = btX != nullptr;
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
  if(has_btX) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(btX);
  if(has_hdr) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(hdr);
  if(has_msg) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(msg);
  if(flags & 0x10) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(PROGRESS_CLASSW);
  else if(has_lst) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(lst);

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

  if(has_btX) {
    // align to word boundary
    if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
    // Button Control for Cancel/No - Return 0
    itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
    itmt->id = OM_DLGBOX_BTNX;
    itmt->style = WS_CHILD|WS_VISIBLE;
    itmt->dwExtendedStyle = WS_EX_LEFT;
    itmt->x = xalign; itmt->y = 80; itmt->cx = 58; itmt->cy = 15;
    pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
    *pTpl++ = 0xFFFF; *pTpl++ = 0x0080; //< WC_BUTTON
    while((*reinterpret_cast<wchar_t*>(pTpl++) = *btX++)); //< Btn text
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

  if(flags & 0x10) {

    // align to word boundary
    if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
    // Edit Control for item list
    itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
    itmt->id = OM_DLGBOX_PB_BAR0;
    itmt->style = WS_CHILD|WS_VISIBLE;
    itmt->dwExtendedStyle = WS_EX_LEFT;
    itmt->x = 16; itmt->y = 50; itmt->cx = 255; itmt->cy = 22;
    pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
    const wchar_t* clsname = PROGRESS_CLASSW;
    while((*reinterpret_cast<wchar_t*>(pTpl++) = *clsname++)); //< class name
    *pTpl++ = 0; //< No Item text
    *pTpl++ = 0; //< No creation
    dlgt->cdit++; //< increment item count

  } else if(has_lst) {

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


  INT_PTR result = DialogBoxIndirectParamW(hins, dlgt, hwnd, __Om_dlgBox_dlgProc, 0);

  Om_free(dlgt);

  return result;
}

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
static HWND __Om_dlgProgress(HINSTANCE hins, HWND hparent, const wchar_t* cpt, uint16_t ico, const wchar_t* hdr, int* result, unsigned flags)
{
  // configure buttons according params
  const wchar_t* fnt = __Om_dlgBox_str_FNT;
  const wchar_t* bt1 = __Om_dlgBox_str_OK;
  bt1 = __Om_dlgBox_str_AB;

  bool has_hdr = hdr ? wcslen(hdr) : false;
  bool has_2pb = (flags & 0x80) ? true : false;

  // compute template size
  size_t tplSize = sizeof(DLGTEMPLATE) + 10;
  tplSize += (3 * sizeof(DLGITEMTEMPLATE)) + 32;
  tplSize += sizeof(wchar_t) * wcslen(cpt);
  tplSize += sizeof(wchar_t) * wcslen(fnt);
  tplSize += sizeof(wchar_t) * wcslen(bt1);
  if(has_hdr) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(hdr);
  tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * 2;
  tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(PROGRESS_CLASSW);
  if(has_2pb) tplSize += sizeof(DLGITEMTEMPLATE) + 10 + sizeof(wchar_t) * wcslen(PROGRESS_CLASSW);

  DLGTEMPLATE* dlgt;
  DLGITEMTEMPLATE* itmt;
  uint16_t* pTpl;

  // allocate memory for new template
  dlgt = reinterpret_cast<DLGTEMPLATE*>(Om_alloc(tplSize));
  if(!dlgt) return nullptr;

  // main dialog template definition
  dlgt->style = DS_3DLOOK|DS_MODALFRAME|WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_SETFONT;
  if(!hparent) dlgt->style |= DS_CENTER;
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

  // align to word boundary
  if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
  // Static Control main message
  itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
  itmt->id = OM_DLGBOX_SC_MESG;
  itmt->style = WS_CHILD|WS_VISIBLE|SS_PATHELLIPSIS/*SS_ENDELLIPSIS*/;
  itmt->dwExtendedStyle = WS_EX_LEFT;
  itmt->x = 45; itmt->y = 30; itmt->cx = 220; itmt->cy = 0;
  pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
  *pTpl++ = 0xFFFF; *pTpl++ = 0x0082; //< WC_STATIC
  *pTpl++ = L' '; *pTpl++ = 0; //< Item text, a white space to initialize control height
  *pTpl++ = 0; //< No creation
  dlgt->cdit++; //< increment item count

  // align to word boundary
  if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
  // Edit Control for item list
  itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
  itmt->id = OM_DLGBOX_PB_BAR0;
  itmt->style = WS_CHILD|WS_VISIBLE;
  itmt->dwExtendedStyle = WS_EX_LEFT;
  itmt->x = 16; itmt->y = 50; itmt->cx = 255; itmt->cy = 22;
  pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
  const wchar_t* clsname = PROGRESS_CLASSW;
  while((*reinterpret_cast<wchar_t*>(pTpl++) = *clsname++)); //< class name
  *pTpl++ = 0; //< No Item text
  *pTpl++ = 0; //< No creation
  dlgt->cdit++; //< increment item count

  if(has_2pb) {
    // align to word boundary
    if(reinterpret_cast<uint64_t>(pTpl) & 0x2) pTpl++;
    // Edit Control for item list
    itmt = reinterpret_cast<DLGITEMTEMPLATE*>(pTpl);
    itmt->id = OM_DLGBOX_PB_BAR1;
    itmt->style = WS_CHILD|WS_VISIBLE;
    itmt->dwExtendedStyle = WS_EX_LEFT;
    itmt->x = 16; itmt->y = 50; itmt->cx = 255; itmt->cy = 22;
    pTpl = reinterpret_cast<uint16_t*>(itmt + 1);
    const wchar_t* clsname = PROGRESS_CLASSW;
    while((*reinterpret_cast<wchar_t*>(pTpl++) = *clsname++)); //< class name
    *pTpl++ = 0; //< No Item text
    *pTpl++ = 0; //< No creation
    dlgt->cdit++; //< increment item count
  }


  HWND hwnd = CreateDialogIndirectParamW(hins, dlgt, hparent, __Om_dlgBox_dlgProc, reinterpret_cast<LPARAM>(result));
  Om_free(dlgt);

  // mimic modal dialog window
  if(hparent)
    EnableWindow(hparent, false);

  ShowWindow(hwnd, SW_SHOW);

  // force all dialog messages to be treated before return
  __Om_dlgBox_peekMessages(hwnd);

  return hwnd;
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
HWND Om_dlgProgress(HWND hwnd, const wchar_t* cpt, uint16_t ico, const wchar_t* hdr, int* result, unsigned flags)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt, OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
  if(!hins) hins = GetModuleHandle(nullptr);

  return __Om_dlgProgress(hins, hwnd, cpt_buf, ico, hdr, result, flags);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgProgressUpdate(HWND hwnd, int tot, int cur, const wchar_t* text, uint8_t bar)
{
  int32_t bar_id = (bar == 0) ? OM_DLGBOX_PB_BAR0 : OM_DLGBOX_PB_BAR1;

  if(tot >= 0)
    SendMessageW(GetDlgItem(hwnd, bar_id), PBM_SETRANGE, 0, MAKELPARAM(0, tot));

  if(cur >= 0) {
    SendMessageW(GetDlgItem(hwnd, bar_id), PBM_SETPOS, cur+1, 0);
    SendMessageW(GetDlgItem(hwnd, bar_id), PBM_SETPOS, cur, 0);
  }

  if(text)
    SendMessageW(GetDlgItem(hwnd, OM_DLGBOX_SC_MESG), WM_SETTEXT, 0, reinterpret_cast<LPARAM>(text));

  // force all dialog messages to be treated before return
  __Om_dlgBox_peekMessages(hwnd);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgProgressClose(HWND hwnd)
{
  // force all dialog messages to be treated
  __Om_dlgBox_peekMessages(hwnd);

  // get parent Windows to stop 'modal'
  HWND hparent = GetParent(hwnd);

  if(DestroyWindow(hwnd)) {

    // stop 'modal'
    EnableWindow(hparent, true);

    // this does not work and fail with error 0x578 (ERROR_INVALID_WINDOW_HANDLE) : Why ?!
    // SetActiveWindow(hparent)

    // this "work" but is ugly and does not "focus" windows
    //SetWindowPos(hparent, HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
    //SetWindowPos(hparent, HWND_NOTOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);

    // this work, it seem, and "focus" window
    SetForegroundWindow(hparent);

  } else {

    #ifdef DEBUG
    std::cout << "DEBUG => Om_dlgProgressClose : DestroyWindow failed\n";
    #endif // DEBUG
  }

  // force all dialog messages to be treated
  __Om_dlgBox_peekMessages(hwnd);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgBox_ok(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
  if(!hins) hins = GetModuleHandle(nullptr);

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
  if(!hins) hins = GetModuleHandle(nullptr);

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
  if(!hins) hins = GetModuleHandle(nullptr);

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
  if(!hins) hins = GetModuleHandle(nullptr);

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
  if(!hins) hins = GetModuleHandle(nullptr);

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
  if(!hins) hins = GetModuleHandle(nullptr);

  return (0 != __Om_dlgBox(hins, hwnd, cpt_buf, ico, hdr.c_str(), msg.c_str(), lst.c_str(), 0x4)); //< OM_DLGBOX_CA
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t Om_dlgBox_ync(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
  if(!hins) hins = GetModuleHandle(nullptr);

  return __Om_dlgBox(hins, hwnd, cpt_buf, ico, hdr.c_str(), msg.c_str(), nullptr, 0x8); //< OM_DLGBOX_YNC
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int32_t Om_dlgBox_yncl(HWND hwnd, const OmWString& cpt, uint16_t ico, const OmWString& hdr, const OmWString& msg, const OmWString& lst)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
  if(!hins) hins = GetModuleHandle(nullptr);

  return __Om_dlgBox(hins, hwnd, cpt_buf, ico, hdr.c_str(), msg.c_str(), lst.c_str(), 0x8); //< OM_DLGBOX_YNC
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgBox_err(const OmWString& hdr, const OmWString& msg, const OmWString& item)
{
  HINSTANCE hins = GetModuleHandle(nullptr);

  __Om_dlgBox(hins, nullptr, OM_APP_NAME, IDI_DLG_ERR, hdr.c_str(), msg.c_str(), item.c_str(), 0x0); //< OM_DLGBOX_OK
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgBox_wrn(const OmWString& hdr, const OmWString& msg, const OmWString& item)
{
  HINSTANCE hins = GetModuleHandle(nullptr);

  __Om_dlgBox(hins, nullptr, OM_APP_NAME, IDI_DLG_WRN, hdr.c_str(), msg.c_str(), item.c_str(), 0x0); //< OM_DLGBOX_OK
}


/// \brief Browse dialog callback.
///
/// Callback function for folder browse dialog window. It is used like a
/// WindowProc to select a default start folder when dialog window is
/// initialized.
///
INT CALLBACK __dialogBrowseDir_Proc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  OM_UNUSED(lParam);

  if(uMsg == BFFM_INITIALIZED) { //< Brother dialog windows is initialized
    if(lpData != 0) SendMessageW(hWnd, BFFM_SETSELECTION, true, lpData);  //< set the selected folder
    if(lpData != 0) SendMessageW(hWnd, BFFM_SETEXPANDED, false, lpData);  //< set and expand the selected folder
  }

  return 0;
}

/// \brief Microsoft COM library initialization flag
///
/// Flag used for Microsoft COM library initialization function, to
/// prevent multiple initialization attemp
///
static bool __co_initialized = false;

/// \brief Microsoft COM library initialization
///
/// Initialization function for the Microsoft COM library for main application thread
///
inline static void __co_initialize()
{
  if(!__co_initialized) {
    if(S_OK == CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE)) {
      __co_initialized = true;
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgOpenDir(OmWString& result, HWND hWnd, const wchar_t* title, const OmWString& start, bool force)
{
  // co co initialize co !
  __co_initialize();

  bool has_cancel = false;

  IFileOpenDialog *FileOpenDialog;
  if(S_OK == CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&FileOpenDialog))) {

    FileOpenDialog->SetOptions(FOS_PICKFOLDERS);
    FileOpenDialog->SetTitle(title);

    // set start location
    if(!start.empty()) {
      IShellItem* ItemStart;
      if(S_OK == SHCreateItemFromParsingName(start.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&ItemStart))) {
        if(!force) {
          FileOpenDialog->SetFolder(ItemStart);
        } else {
          FileOpenDialog->SetDefaultFolder(ItemStart);
        }
        ItemStart->Release();
      }
    }

    if(S_OK == FileOpenDialog->Show(hWnd)) {

      IShellItemArray* ShellItemArray = nullptr;
      if(S_OK == FileOpenDialog->GetResults(&ShellItemArray)) {

        IShellItem* ShellItem = nullptr;
        if(S_OK == ShellItemArray->GetItemAt(0, &ShellItem)) {

          wchar_t* szName;
          if(S_OK == ShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &szName)) {
            result = szName;
            CoTaskMemFree(szName);
          }

          ShellItem->Release();
        }

        ShellItemArray->Release();
      }

    } else {
      has_cancel = true;
    }

    FileOpenDialog->Release();
  }

  return !has_cancel;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgOpenDirMultiple(OmWStringArray& result, HWND hWnd, const wchar_t* title, const OmWString& start, bool force)
{
  // co co initialize co !
  __co_initialize();

  bool has_cancel = false;

  IFileOpenDialog *FileOpenDialog;
  if(S_OK == CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&FileOpenDialog))) {

    FileOpenDialog->SetOptions(FOS_PICKFOLDERS|FOS_ALLOWMULTISELECT);
    FileOpenDialog->SetTitle(title);

    // set start location
    if(!start.empty()) {
      IShellItem* ItemStart;
      if(S_OK == SHCreateItemFromParsingName(start.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&ItemStart))) {
        if(!force) {
          FileOpenDialog->SetFolder(ItemStart);
        } else {
          FileOpenDialog->SetDefaultFolder(ItemStart);
        }
        ItemStart->Release();
      }
    }

    if(S_OK == FileOpenDialog->Show(hWnd)) {

      IShellItemArray* ShellItemArray = nullptr;
      if(S_OK == FileOpenDialog->GetResults(&ShellItemArray)) {

        DWORD ItemCount = 0;
        ShellItemArray->GetCount(&ItemCount);

        for(DWORD i = 0; i < ItemCount; ++i) {

          IShellItem* ShellItem = nullptr;
          if(S_OK == ShellItemArray->GetItemAt(i, &ShellItem)) {

            wchar_t* szName;
            if(S_OK == ShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &szName)) {
              result.push_back(szName);
              CoTaskMemFree(szName);
            }

            ShellItem->Release();
          }
        }

        ShellItemArray->Release();
      }

    } else {
      has_cancel = true;
    }

    FileOpenDialog->Release();
  }

  return !has_cancel;
}

/// \brief COMDLG_FILTERSPEC static array size
///
/// Size of the COMDLG_FILTERSPEC static array array.
///
#define OM_MAX_COMDLG_FILTERSPEC  64

/// \brief Static array of COMDLG_FILTERSPEC
///
/// Static array of COMDLG_FILTERSPEC structure used for OFN filter conversion
///
static COMDLG_FILTERSPEC __comdlg_filterspec_buf[OM_MAX_COMDLG_FILTERSPEC];

/// \brief Parse OFN filter as COMDLG_FILTERSPEC
///
/// Parse old OFN filter null-delimited string array into an array of COMDLG_FILTERSPEC structures.
///
/// \param[in]  ofn_filter  : Old OFN filter null-delimited string array
/// \param[out] spec_count  : Point to integer that receive size of the returned COMDLG_FILTERSPEC array
///
/// \return Pointer to COMDLG_FILTERSPEC array or nullptr if failed
///
inline static COMDLG_FILTERSPEC* __ofn_filter_to_comdlg_spec(const wchar_t* ofn_filter, uint32_t* spec_count)
{
  uint32_t n;
  const wchar_t* p;

  // 1. get count of individual null-terminated string in the OFN filter
  n = 0; p = ofn_filter;
  while(*p != 0) {
    n++;  p = &p[wcslen(p)+1];
  }

  // 2. check whether string count is even and compatible with
  // static buffer size
  if(n % 2 != 0) {
    *spec_count = 0;
    return nullptr;
  }

  n /= 2;

  if(n > OM_MAX_COMDLG_FILTERSPEC) {
    *spec_count = 0;
    return nullptr;
  }

  // 3. fill the comdlg spec buffer
  *spec_count = n;

  n = 0; p = ofn_filter;
  while(*p != 0) {

    __comdlg_filterspec_buf[n].pszName = p;
    p = &p[wcslen(p)+1];

    __comdlg_filterspec_buf[n].pszSpec = p;
    p = &p[wcslen(p)+1];

    n++;
  }

  return __comdlg_filterspec_buf;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgOpenFile(OmWString& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const OmWString& start, bool force)
{
  // co co initialize co !
  __co_initialize();

  bool has_cancel = false;

  IFileOpenDialog *FileOpenDialog;
  if(S_OK == CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&FileOpenDialog))) {

    FileOpenDialog->SetTitle(title);

    // set file filter
    if(filter) {

      // as the new API use structure for file filter we need to parse our old OFN filter
      // string into array of COMDLG_FILTERSPEC struct
      uint32_t spec_count;
      COMDLG_FILTERSPEC* spec_array = __ofn_filter_to_comdlg_spec(filter, &spec_count);

      if(spec_array)
        FileOpenDialog->SetFileTypes(spec_count, spec_array);
    }

    // set start location
    if(!start.empty()) {
      IShellItem* ItemStart;
      if(S_OK == SHCreateItemFromParsingName(start.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&ItemStart))) {
        if(!force) {
          FileOpenDialog->SetFolder(ItemStart);
        } else {
          FileOpenDialog->SetDefaultFolder(ItemStart);
        }
        ItemStart->Release();
      }
    }

    if(S_OK == FileOpenDialog->Show(hWnd)) {

      IShellItemArray* ShellItemArray = nullptr;
      if(S_OK == FileOpenDialog->GetResults(&ShellItemArray)) {

        IShellItem* ShellItem = nullptr;
        if(S_OK == ShellItemArray->GetItemAt(0, &ShellItem)) {

          wchar_t* szName;
          if(S_OK == ShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &szName)) {
            result = szName;
            CoTaskMemFree(szName);
          }

          ShellItem->Release();
        }

        ShellItemArray->Release();
      }

    } else {
      has_cancel = true;
    }

    FileOpenDialog->Release();
  }

  return !has_cancel;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgOpenFileMultiple(OmWStringArray& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const OmWString& start, bool force = false)
{
  // co co initialize co !
  __co_initialize();

  bool has_cancel = false;

  IFileOpenDialog *FileOpenDialog;
  if(S_OK == CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&FileOpenDialog))) {

    FileOpenDialog->SetOptions(FOS_ALLOWMULTISELECT);
    FileOpenDialog->SetTitle(title);

    // set file filter
    if(filter) {

      // as the new API use structure for file filter we need to parse our old OFN filter
      // string into array of COMDLG_FILTERSPEC struct
      uint32_t spec_count;
      COMDLG_FILTERSPEC* spec_array = __ofn_filter_to_comdlg_spec(filter, &spec_count);

      if(spec_array)
        FileOpenDialog->SetFileTypes(spec_count, spec_array);
    }

    // set start location
    if(!start.empty()) {
      IShellItem* ItemStart;
      if(S_OK == SHCreateItemFromParsingName(start.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&ItemStart))) {
        if(!force) {
          FileOpenDialog->SetFolder(ItemStart);
        } else {
          FileOpenDialog->SetDefaultFolder(ItemStart);
        }
        ItemStart->Release();
      }
    }

    if(S_OK == FileOpenDialog->Show(hWnd)) {

      IShellItemArray* ShellItemArray = nullptr;
      if(S_OK == FileOpenDialog->GetResults(&ShellItemArray)) {

        DWORD ItemCount = 0;
        ShellItemArray->GetCount(&ItemCount);

        for(DWORD i = 0; i < ItemCount; ++i) {

          IShellItem* ShellItem = nullptr;
          if(S_OK == ShellItemArray->GetItemAt(i, &ShellItem)) {

            wchar_t* szName;
            if(S_OK == ShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &szName)) {
              result.push_back(szName);
              CoTaskMemFree(szName);
            }

            ShellItem->Release();
          }
        }

        ShellItemArray->Release();
      }

    } else {
      has_cancel = true;
    }

    FileOpenDialog->Release();
  }

  return !has_cancel;
}
/*
///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgOpenFile(OmWString& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const OmWString& start)
{
  wchar_t result_buf[OM_MAX_PATH];

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
  ofn.lpstrFile = result_buf;
  ofn.lpstrFile[0] = L'\0';

  ofn.nMaxFile = OM_MAX_PATH;

  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER|OFN_NONETWORKBUTTON|OFN_NOTESTFILECREATE;

  if(!GetOpenFileNameW(&ofn))
    return false;

  result = result_buf;

  return true;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgOpenFileMultiple(OmWStringArray& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const OmWString& start)
{
  wchar_t result_buf[OM_MAX_PATH*50];

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
  ofn.lpstrFile = result_buf;
  ofn.lpstrFile[0] = L'\0';

  ofn.nMaxFile = OM_MAX_PATH*50;

  ofn.lpstrTitle = title;
  ofn.Flags = OFN_EXPLORER|OFN_NONETWORKBUTTON|OFN_NOTESTFILECREATE|OFN_ALLOWMULTISELECT;

  if(!GetOpenFileNameW(&ofn))
    return false;

  // parse result, the first string is path to folder, the following ones are
  // selected file names, everything is null-separated
  OmWString path, directory = result_buf;
  wchar_t* filename = &result_buf[ofn.nFileOffset];
  while(*filename != 0) {

    Om_concatPaths(path, directory, filename);
    result.push_back(path);

    // jump to next string
    filename = &filename[wcslen(filename)+1];
  }

  return true;
}

*/

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgSaveFile(OmWString& result, HWND hWnd, const wchar_t* title, const wchar_t* filter, const wchar_t* ext, const wchar_t* name, const OmWString& start, bool force)
{
  // co co initialize co !
  __co_initialize();

  bool has_cancel = false;

  IFileSaveDialog* FileSaveDialog;
  if(S_OK == CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&FileSaveDialog))) {

    FileSaveDialog->SetTitle(title);

    // set file filter
    if(filter) {

      // as the new API use structure for file filter we need to parse our old OFN filter
      // string into array of COMDLG_FILTERSPEC struct
      uint32_t spec_count;
      COMDLG_FILTERSPEC* spec_array = __ofn_filter_to_comdlg_spec(filter, &spec_count);

      if(spec_array)
        FileSaveDialog->SetFileTypes(spec_count, spec_array);
    }

    // set default extension
    if(ext)
     FileSaveDialog->SetDefaultExtension(ext);

    // set default file name
    if(name)
      FileSaveDialog->SetFileName(name);

    // set start location
    if(!start.empty()) {
      IShellItem* ItemStart;
      if(S_OK == SHCreateItemFromParsingName(start.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&ItemStart))) {
        if(!force) {
          FileSaveDialog->SetFolder(ItemStart);
        } else {
          FileSaveDialog->SetDefaultFolder(ItemStart);
        }
        ItemStart->Release();
      }
    }

    if(S_OK == FileSaveDialog->Show(hWnd)) {

      IShellItem* ShellItem = nullptr;
      if(S_OK == FileSaveDialog->GetResult(&ShellItem)) {

        wchar_t* szName;
        if(S_OK == ShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &szName)) {
          result = szName;
          CoTaskMemFree(szName);
        }

        ShellItem->Release();
      }

    } else {
      has_cancel = true;
    }

    FileSaveDialog->Release();
  }

  return !has_cancel;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgCreateFolder(HWND hwnd, const OmWString& item, const OmWString& path)
{
  OM_UNUSED(item);

  if(!PathFileExistsW(path.c_str())) {

    wchar_t cpt_buf[OM_MAX_ITEM];

    swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - Create directory", OM_APP_NAME);

    HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
    if(!hins) hins = GetModuleHandle(nullptr);

    if(0 != __Om_dlgBox(hins, hwnd, cpt_buf, 804/*IDI_DLG_QRY*/, L"Create directory",
                           L"The directory does not exists, do you want to create it ?", path.c_str(), 0x2/*OM_DLGBOX_YN*/)) {

      int result = SHCreateDirectoryExW(nullptr, path.c_str(), nullptr);
      if(result != 0) {
        __Om_dlgBox(hins, hwnd, cpt_buf, 801/*IDI_DLG_ERR*/, L"Create directory error",
                    L"Unable to create directory:", Om_getErrorStr(result).c_str(), 0x0/*OM_DLGBOX_OK*/);
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

  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - Overwrite file", OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
  if(!hins) hins = GetModuleHandle(nullptr);

  return (0 != __Om_dlgBox(hins, hwnd, cpt_buf, 804/*IDI_DLG_QRY*/, L"Overwrite file",
                           L"The file already exists. Do you want to overwrite it ?", path.c_str(), 0x2/*OM_DLGBOX_YN*/));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgValidName(HWND hwnd, const OmWString& item, const OmWString& name)
{
  OmWString msg;

  if(!name.empty()) {
    if(!Om_hasLegalSysChar(name)) {
      msg = item; msg += L" cannot contain the following characters: / * ? \" < > | \\";
    }
  } else {
    msg = item; msg += L" cannot be empty";
  }

  if(!msg.empty()) {

    wchar_t cpt_buf[OM_MAX_ITEM];

    swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - Invalid name", OM_APP_NAME);

    HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
    if(!hins) hins = GetModuleHandle(nullptr);

    OmWString hdr = L"Invalid "; hdr += item;

    __Om_dlgBox(hins, hwnd, cpt_buf, 802/*IDI_DLG_WRN*/, hdr.c_str(), msg.c_str(), name.c_str(), 0x0/*OM_DLGBOX_OK*/);

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
    if(!Om_hasLegalPathChar(path)) {
      msg = item; msg += L" path cannot contain the following characters: / * ? \" < > |";
    }
  } else {
    msg = item; msg += L" cannot be empty";
  }

  if(!msg.empty()) {

    wchar_t cpt_buf[OM_MAX_ITEM];

    swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - Invalid path", OM_APP_NAME);

    HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
    if(!hins) hins = GetModuleHandle(nullptr);

    OmWString hdr = L"Invalid "; hdr += item;

    __Om_dlgBox(hins, hwnd, cpt_buf, 802/*IDI_DLG_WRN*/, hdr.c_str(), msg.c_str(), path.c_str(), 0x0/*OM_DLGBOX_OK*/);

    return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgValidUrl(HWND hwnd, const OmWString& item, const OmWString& url)
{
  OmWString msg;

  if(!url.empty()) {
    if(!Om_isUrl(url)) {
      msg = item; msg += L" must be a valid URL";
    }
  } else {
    msg = item; msg += L" cannot be empty";
  }

  if(!msg.empty()) {

    wchar_t cpt_buf[OM_MAX_ITEM];

    swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - Invalid URL", OM_APP_NAME);

    HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
    if(!hins) hins = GetModuleHandle(nullptr);

    OmWString hdr = L"Invalid "; hdr += item;

    __Om_dlgBox(hins, hwnd, cpt_buf, 802/*IDI_DLG_WRN*/, hdr.c_str(), msg.c_str(), url.c_str(), 0x0/*OM_DLGBOX_OK*/);

    return false;
  }

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgValidUrlPath(HWND hwnd, const OmWString& item, const OmWString& url)
{
  OmWString msg;

  if(!url.empty()) {
    if(!Om_hasLegalUrlChar(url)) {
      msg = item; msg += L" cannot contain the following characters: # \" < > | \\ { } ^ [ ] ` + : @ $";
    }
  } else {
    msg = item; msg += L" cannot be empty";
  }

  if(!msg.empty()) {

    wchar_t cpt_buf[OM_MAX_ITEM];

    swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - Illegal URL character", OM_APP_NAME);

    HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
    if(!hins) hins = GetModuleHandle(nullptr);

    OmWString hdr = L"Invalid "; hdr += item;

    __Om_dlgBox(hins, hwnd, cpt_buf, 802/*IDI_DLG_WRN*/, hdr.c_str(), msg.c_str(), url.c_str(), 0x0/*OM_DLGBOX_OK*/);

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

    OmWString hdr = L"Invalid "; hdr += item;
    OmWString msg = item; msg += L" must be an existing directory.";

    wchar_t cpt_buf[OM_MAX_ITEM];

    swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - Invalid directory", OM_APP_NAME);

    HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
    if(!hins) hins = GetModuleHandle(nullptr);

    __Om_dlgBox(hins, hwnd, cpt_buf, 802/*IDI_DLG_WRN*/, hdr.c_str(), msg.c_str(), path.c_str(), 0x0/*OM_DLGBOX_OK*/);

    return false;
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
  if(!hins) hins = GetModuleHandle(nullptr);

  return (0 != __Om_dlgBox(hins, hwnd, cpt_buf, 804/*IDI_DLG_QRY*/, L"Unsaved changes",
                           L"You made unsaved changes. Close without saving ?", nullptr, 0x2/*OM_DLGBOX_YN*/));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_dlgResetUnsaved(HWND hwnd, const OmWString& cpt)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
  if(!hins) hins = GetModuleHandle(nullptr);

  return (0 != __Om_dlgBox(hins, hwnd, cpt_buf, 804/*IDI_DLG_QRY*/, L"Unsaved changes",
                           L"You made unsaved changes. Continue without saving ?", nullptr, 0x2/*OM_DLGBOX_YN*/));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgSaveSucces(HWND hwnd, const OmWString& cpt, const OmWString& hdr, const OmWString& item)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
  if(!hins) hins = GetModuleHandle(nullptr);

  OmWString msg(item); msg.append(L" file was successfully saved.");

  __Om_dlgBox(hins, hwnd, cpt_buf, 803/*IDI_DLG_NFO*/, hdr.c_str(), msg.c_str(), nullptr, 0x0/*OM_DLGBOX_OK*/);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_dlgSaveError(HWND hwnd, const OmWString& cpt, const OmWString& hdr, const OmWString& item, const OmWString& error)
{
  wchar_t cpt_buf[OM_MAX_ITEM];

  swprintf(cpt_buf, OM_MAX_ITEM, L"%ls - %ls", cpt.c_str(), OM_APP_NAME);

  HINSTANCE hins = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
  if(!hins) hins = GetModuleHandle(nullptr);

  OmWString msg(L"Saving "); msg.append(item); msg.append(L" file has failed:");

  __Om_dlgBox(hins, hwnd, cpt_buf, 801/*IDI_DLG_ERR*/, hdr.c_str(), msg.c_str(), error.c_str(), 0x0/*OM_DLGBOX_OK*/);
}
