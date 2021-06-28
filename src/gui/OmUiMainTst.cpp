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
#include "gui/res/resource.h"
#include "OmManager.h"
#include "gui/OmUiMainTst.h"
#include "gui/OmUiMain.h"

#include <Richedit.h>

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainTst::OmUiMainTst(HINSTANCE hins) : OmDialog(hins)
{
  // This is needed for RichEdit control to work properly
  LoadLibrary("Msftedit.dll");
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMainTst::~OmUiMainTst()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiMainTst::id() const
{
  return IDD_MAIN_TST;
}

static DWORD WINAPI __es_stream_in(DWORD_PTR ptr, LPBYTE buf, LONG  wbytes, LONG * rbytes)
{
  DWORD rb;
  ReadFile(reinterpret_cast<HANDLE>(ptr), buf, wbytes, &rb, nullptr);
  *rbytes = rb;

  if(rb == 0) {
    CloseHandle(reinterpret_cast<HANDLE>(ptr));
  }

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainTst::_onInit()
{
  HANDLE hFile = CreateFileW( L"rtf.rtf", GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE)
    return;

  EDITSTREAM es = {};
  es.dwCookie = reinterpret_cast<DWORD_PTR>(hFile);
  es.pfnCallback = &__es_stream_in;

  this->msgItem(IDC_EC_RTF, EM_STREAMIN, SF_RTF, reinterpret_cast<LPARAM>(&es));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainTst::_onShow()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainTst::_onHide()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainTst::_onResize()
{
  this->_setItemPos(IDC_EC_RTF, 5, 20, this->width()-10, this->height()-30);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainTst::_onRefresh()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMainTst::_onQuit()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiMainTst::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

  }

  return false;
}
