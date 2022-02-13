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
#include "OmUiMgr.h"

#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiMgrMainTst.h"


DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE lpBuff, LONG cb, PLONG pcb)
{
  HANDLE hFile = (HANDLE)dwCookie;

  if(ReadFile(hFile, lpBuff, cb, (DWORD *)pcb, NULL)) {

    #ifdef DEBUG
    std::cout << "EditStreamCallback:: ReadFile OK\n";
    #endif

    return 0;
  }

  #ifdef DEBUG
  std::cout << "EditStreamCallback:: ReadFile failed\n";
  #endif

  return -1;
}

BOOL FillRichEditFromFile(HWND hwnd, LPCTSTR pszFile)
{
  BOOL fSuccess = FALSE;

  HANDLE hFile = CreateFile(pszFile, GENERIC_READ,
                            FILE_SHARE_READ, 0, OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if(hFile != INVALID_HANDLE_VALUE) {

    EDITSTREAM es = {};
    es.pfnCallback = EditStreamCallback;
    es.dwCookie    = (DWORD_PTR)hFile;

    if(SendMessage(hwnd, EM_STREAMIN, SF_RTF, (LPARAM)&es) && es.dwError == 0) {

      #ifdef DEBUG
      std::cout << "FillRichEditFromFile:: Success\n";
      #endif

      fSuccess = TRUE;
    }

    CloseHandle(hFile);

  } else {

    #ifdef DEBUG
    std::cout << "FillRichEditFromFile:: CreateFile failed\n";
    #endif
  }

  return fSuccess;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrMainTst::OmUiMgrMainTst(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr),
  _hdivHwnd(nullptr),
  _hdivHovr(false),
  _hdivCur{100,0},
  _hdivOld{0,0}
{


}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrMainTst::~OmUiMgrMainTst()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMgrMainTst::id() const
{
  return IDD_MGR_MAIN_TST;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainTst::_onInit()
{
  #ifdef DEBUG
  std::cout << "OmUiMgrMainTst::_onInit\n";
  #endif

  // retrieve main dialog
  this->_pUiMgr = static_cast<OmUiMgr*>(this->root());

  /*
  //SetWindowLongPtr(this->_hwnd, GWL_STYLE, WS_CHILD|WS_BORDER); //3d argument=style
  SetWindowLongPtr(this->_hwnd, GWL_EXSTYLE, WS_EX_STATICEDGE); //3d argument=style

  this->enableItem(IDC_SC_SEPAR, false);
  // set white background to fit tab background
  EnableThemeDialogTexture(this->getItem(IDC_SC_SEPAR), ETDT_ENABLETAB);

  // Defines fonts for package description, title, and log output
  HFONT hFt = Om_createFont(21, 400, L"Ms Shell Dlg");
  this->msgItem(IDC_SC_NAME, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  //WebBrowserCreate(0, WS_CHILD, this->_hwnd, reinterpret_cast<HMENU>(IDC_SC_HTML), this->_hins);
  //WebBrowserAttach(this->getItem(IDC_SC_HTML), nullptr);

  this->enableItem(IDC_SC_HTML, false);

  wstring html =
    L"<html><head>"
    "<style>"
    "  body {text-align:center; border:0; overflow:auto; font-family:helvetica;}"
    "  h1 {font-size: 1em;}"
    "</style>"
    "</head><body oncontextmenu=\"return false;\">"
    "<h1>This is a test</h1>"
    "</body></html>";

  //WebBrowserDocumentClose(this->getItem(IDC_SC_HTML));
  //WebBrowserDocumentWrite(this->getItem(IDC_SC_HTML), html.c_str());

  // splitter handle
  this->_hdivHwnd = this->getItem(IDC_SC_SEPAR);
  // initial splitter position
  this->_hdivCur[1] = this->cliUnitY() - this->_hdivCur[0];
  */


}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainTst::_onShow()
{
  #ifdef DEBUG
  std::cout << "OmUiMgrMainTst::_onShow\n";
  #endif

  FillRichEditFromFile(this->getItem(IDC_FT_DESC), "hello.rtf");
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainTst::_onHide()
{
  #ifdef DEBUG
  std::cout << "OmUiMgrMainTst::_onHide\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainTst::_onResize()
{
  // Horizontal separator
  this->_setItemPos(IDC_SC_LBL01, 2, 2, this->cliUnitX() - 4, 12);
  // RTF Viewer
  this->_setItemPos(IDC_FT_DESC, 2, 20, this->cliUnitX() - 4, this->cliUnitY() - 22 );
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainTst::_onRefresh()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainTst::_onQuit()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiMgrMainTst::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_LBUTTONDOWN) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMgrMainTst::_onMsg : WM_LBUTTONDOWN\n";
    #endif

    if(this->_hdivHovr) {

      this->_hdivOld[0] = this->_hdivCur[0];
      this->_hdivOld[1] = HIWORD(lParam);

      SetCapture(this->_hwnd);
    }

  }

  if(uMsg == WM_LBUTTONUP) {

    #ifdef DEBUG
    std::cout << "DEBUG => OmUiMgrMainTst::_onMsg : WM_LBUTTONUP\n";
    #endif

    if(GetCapture() == this->_hwnd) {
      ReleaseCapture();
    }
  }

  if(uMsg == WM_MOUSEMOVE) {

    long y = HIWORD(lParam);

    if(GetCapture() == this->_hwnd) {

      this->_hdivCur[0] = this->_hdivOld[0] + MulDiv(this->_hdivOld[1] - y, 8, this->ubaseY());

      if(this->_hdivCur[0] < 100) this->_hdivCur[0] = 100;
      if(this->_hdivCur[0] > this->cliUnitY()-100) this->_hdivCur[0] = this->cliUnitY() - 100;

      // splitter position
      this->_hdivCur[1] = this->cliUnitY() - this->_hdivCur[0];

      this->_onResize();

    } else {

      long pos[4];

      GetWindowRect(this->_hdivHwnd, reinterpret_cast<LPRECT>(&pos));
      MapWindowPoints(HWND_DESKTOP, this->_hwnd, reinterpret_cast<LPPOINT>(&pos), 1);

      this->_hdivHovr = (y > (pos[1] - 5) && y < (pos[1] + 5));
    }

    return true;
  }

  if(uMsg == WM_SETCURSOR) {

    if(this->_hdivHovr) {

      SetCursor(LoadCursor(0,IDC_SIZENS));

      return true;
    }
  }

  if(uMsg == WM_COMMAND) {

  }

  return false;
}
