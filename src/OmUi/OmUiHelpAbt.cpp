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
  #include <ShlObj.h>

#include "OmBaseApp.h"

#include "OmModMan.h"

#include "OmUtilWin.h"
#include "OmUtilStr.h"
#include "OmUtilFs.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiHelpAbt.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiHelpAbt::OmUiHelpAbt(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiHelpAbt::~OmUiHelpAbt()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_RESUL, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
  hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_SC_VERS, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiHelpAbt::id() const
{
  return IDD_HELP_ABT;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpAbt::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(IDI_APP,2),Om_getResIcon(IDI_APP,1));

  HFONT hFt = Om_createFont(18, 700, L"Arial");
  this->msgItem(IDC_SC_VERS, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  OmWString vers = OM_APP_NAME L" - ";
  vers += std::to_wstring(OM_APP_MAJ);
  vers += L'.'; vers += std::to_wstring(OM_APP_MIN);
  #if (OM_APP_REV > 0)
  vers += L'.'; vers += std::to_wstring(OM_APP_REV);
  #endif
  vers += L" - " OM_APP_ARCH;
  this->setItemText(IDC_SC_VERS, vers);

  OmWString date_auth = OM_APP_DATE L" - " OM_APP_AUTHOR;
  this->setItemText(IDC_SC_DATE, date_auth);

  OmWString repo_url = L"<a href=\"" OM_APP_URL L"\">" OM_APP_URL L"</a>";
  this->setItemText(IDC_LM_LNK01, repo_url);

  hFt = Om_createFont(14, 300, L"Consolas");
  this->msgItem(IDC_EC_RESUL, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  OmCString credits;
  #ifdef DEBUG
  if(Om_loadPlainText(&credits, L"..\\..\\CREDITS")) {
  #else
  if(Om_loadPlainText(&credits, L"CREDITS")) {
  #endif // DEBUG
    SetDlgItemText(this->_hwnd, IDC_EC_RESUL, Om_toCRLF(credits).c_str());
  }

  // subclass window button
  SetWindowSubclass(GetDlgItem(this->_hwnd, IDC_BC_DONATE), OmUiHelpAbt::_donate_subclass_proc, 0, reinterpret_cast<DWORD_PTR>(this));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpAbt::_onResize()
{
  int32_t half_w = static_cast<int32_t>(this->cliWidth() * 0.5f);
  int32_t base_y = 0;

  // the white background
  this->_setItemPos(IDC_SC_WHITE, 0, 0, this->cliWidth(), this->cliHeight()-40, true);

  // App name - Version
  this->_setItemPos(IDC_SC_VERS, 10, base_y, this->cliWidth()-20, 16, true);

  // date - author
  this->_setItemPos(IDC_SC_DATE, 10, base_y+20, this->cliWidth()-20, 16, true);

  // github URL
  // Why Microsoft made such simple thing as "Centered" always so complicated ?
  HDC hDc = CreateCompatibleDC(nullptr);
  // set font to measure Link size
  HFONT hOldFnt = (HFONT)SelectObject(hDc, Om_createFont(12, 200, L"Ms Shell Dlg"));
  DeleteObject(hOldFnt);
  // get lint text width
  SIZE Size;
  GetTextExtentPoint32W(hDc, OM_APP_URL, wcslen(OM_APP_URL), &Size);
  this->_setItemPos(IDC_LM_LNK01, half_w - (Size.cx / 2), base_y+50, Size.cx, 16, true);
  // free DC
  ReleaseDC(nullptr, hDc);

  // donate button
  int32_t btn_w = 116;
  this->_setItemPos(IDC_BC_DONATE, half_w - (btn_w / 2), base_y+75, btn_w, 23, true);

  this->_setItemPos(IDC_EC_RESUL, 10, base_y+120, this->cliWidth()-20, this->cliHeight()-170, true);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 8, this->cliHeight()-40, this->cliWidth()-16, 1, true);
  // Close button
  this->_setItemPos(IDC_BC_CLOSE, this->cliWidth()-85, this->cliHeight()-30, 78, 23, true);

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
LRESULT WINAPI OmUiHelpAbt::_donate_subclass_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
  OM_UNUSED(uIdSubclass);  OM_UNUSED(dwRefData);

  static bool MouseHover = false;

  if(uMsg == WM_MOUSEMOVE) {
    if(!MouseHover) {
      MouseHover = true;
      TRACKMOUSEEVENT EventTrack = {};
      EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
      EventTrack.dwFlags = TME_LEAVE;
      EventTrack.hwndTrack = hWnd;
      TrackMouseEvent(&EventTrack);
      RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE);
    }
  }

  if(uMsg == WM_MOUSELEAVE) {
    MouseHover = false;
    RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE);
  }

  if(uMsg == WM_SETCURSOR) {
    // check whether cursor is hovering button
    if(MouseHover) {
      SetCursor(LoadCursor(0,IDC_HAND));
      return 1; //< bypass default process
    }
  }

  if(uMsg == WM_LBUTTONUP) {
    if(MouseHover) {
      SetCursor(LoadCursor(0,IDC_HAND));
      ShellExecuteW(0, 0, OM_DON_URL, 0, 0 , SW_SHOW );
      return 1;
    }
  }

  if(uMsg == WM_PAINT) {
    PAINTSTRUCT ps;
    HDC hDc = BeginPaint(hWnd, &ps);

    // draw white background
    HBRUSH hBrush = CreateSolidBrush(0x00FFFFFF);
    FillRect(hDc, &ps.rcPaint, hBrush);
    DeleteObject(hBrush);

    // select HBITMAP to be drawn
    HBITMAP hBm;
    if(MouseHover) {
      hBm = Om_getResImagePremult(IDB_DON_PP_HOV);
    } else {
      hBm = Om_getResImagePremult(IDB_DON_PP);
    }

    // get bitmap to get image size
    BITMAP bm;
    GetObject(hBm, sizeof(BITMAP), &bm);

    // create DC and select bitmap to be drawn
    HDC hDcMem = CreateCompatibleDC(hDc);
    SelectObject(hDcMem, hBm);

    BLENDFUNCTION BlendFunc = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    AlphaBlend(hDc, ps.rcPaint.left, ps.rcPaint.top, bm.bmWidth, bm.bmHeight, hDcMem,
                0, 0, bm.bmWidth, bm.bmHeight, BlendFunc);

    DeleteDC(hDcMem);

    EndPaint(hWnd, &ps);
  }

  return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiHelpAbt::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {

    NMHDR* pNmhdr = reinterpret_cast<NMHDR*>(lParam);

    switch(LOWORD(wParam))
    {
    case IDC_BC_DONATE:

      break;

    case IDC_LM_LNK01:
    case IDC_LM_LNK02:
      if(pNmhdr->code == NM_CLICK) {
        NMLINK* pNmlink = reinterpret_cast<NMLINK*>(lParam);
        ShellExecuteW(NULL, L"open", pNmlink->item.szUrl, nullptr, nullptr, SW_SHOW);
      }
      break;
    }
  }

  if(uMsg == WM_COMMAND) {
    switch(LOWORD(wParam))
    {
    case IDC_BC_CLOSE:
      this->quit();
      break;
    }
  }

  if(uMsg == WM_CTLCOLORSTATIC) {
    // set white background for static controls
    SetBkMode(reinterpret_cast<HDC>(wParam),TRANSPARENT);
    SetBkColor(reinterpret_cast<HDC>(wParam), 0x00FFFFFF);
    return (INT_PTR)GetStockObject(WHITE_BRUSH);
  }

  return false;
}
