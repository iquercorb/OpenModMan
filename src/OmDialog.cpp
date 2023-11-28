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
#include "OmBaseWin.h"
#include <ShlObj.h>

#ifdef DEBUG
#include "OmUtilWin.h"
#endif

#define UWM_QUIT (WM_USER+1)

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmDialog.h"


/// \brief Initialized WinAPI Common Controls flag
///
/// Flag to tell whether WinAPI Common Controls must be initialized
///
static bool __cce_initialized = false;

/// \brief Initialize common controls
///
/// Function to initialize WinAPI Common Controls once
///
static inline void __cce_init()
{
  if(!__cce_initialized) {

    INITCOMMONCONTROLSEX icce;

    icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icce.dwICC = ICC_BAR_CLASSES|ICC_HOTKEY_CLASS|ICC_LINK_CLASS|
    ICC_LISTVIEW_CLASSES|ICC_NATIVEFNTCTL_CLASS|ICC_PROGRESS_CLASS|
    ICC_STANDARD_CLASSES|ICC_TAB_CLASSES;

    InitCommonControlsEx(&icce);

    LoadLibrary(TEXT("Msftedit.dll"));

    // we need to initialize only once per process
    __cce_initialized = true;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialog::OmDialog(HINSTANCE hins) :
  _hins(hins),
  _hwnd(nullptr),
  _parent(nullptr),
  _accel(nullptr),
  _menu(nullptr),
  _data(nullptr),
  _init(true),
  _modal(false),
  _recw{},
  _recc{},
  _ubase{},
  _limit{},
  _size{},
  _usize{},
  _active(false),
  _minimized(false),
  _safe(true),
  _wait_safe(false),
  _hdwp(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialog::~OmDialog()
{
  for(size_t i = 0; i < this->_child.size(); ++i)
    delete this->_child[i];
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmDialog::id() const
{
  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialog* OmDialog::childById(long id) const
{
  for(size_t i = 0; i < this->_child.size(); ++i)
    if(this->_child[i]->id() == id)
      return this->_child[i];

  return nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialog* OmDialog::siblingById(long id) const
{
  if(this->_parent)
    return this->_parent->childById(id);

  return nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialog* OmDialog::root()
{
  if(this->_parent) {
    return this->_parent->root();
  } else {
    return this;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::open(bool show)
{
  __cce_init(); //< initialize WinAPI Common Controls (done only once)

  if(!this->id())
    return;

  if(this->_hwnd)
    return;

  this->_hwnd = CreateDialogParamW( this->_hins,
                                    MAKEINTRESOURCEW(this->id()),
                                    (this->_parent)?this->_parent->_hwnd:nullptr,
                                    reinterpret_cast<DLGPROC>(this->_wndproc),
                                    reinterpret_cast<LPARAM>(this));

  if(this->_hwnd != nullptr) {

    SetWindowLongPtr(this->_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    if(this->_parent) {
      EnableWindow(this->_parent->_hwnd, false);
      this->_modal = true;
    }

    // Retrieve the associated dialog menu if exists
    this->_menu = GetMenu(this->_hwnd);

    if(show) ShowWindow(this->_hwnd, SW_SHOW);

    #ifdef DEBUG
    std::wcout << L"DEBUG => OmDialog(ID="<<this->id()<<L")::open (thread=: "<<GetCurrentThreadId()<<L")\n";
    #endif

  } else {
    #ifdef DEBUG
    int err = GetLastError();
    std::wcout << L"ERROR => OmDialog(ID="<<this->id()<<L")::open : "<<Om_getErrorStr(err)<<L"\n";
    #endif
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::modeless(bool show)
{
  __cce_init(); //< initialize WinAPI Common Controls (done only once)

  if(!this->id())
    return;

  if(this->_hwnd)
    return;

  this->_hwnd = CreateDialogParamW( this->_hins,
                                    MAKEINTRESOURCEW(this->id()),
                                    (this->_parent) ? this->_parent->_hwnd : nullptr,
                                    reinterpret_cast<DLGPROC>(this->_wndproc),
                                    reinterpret_cast<LPARAM>(this));

  if(this->_hwnd != nullptr) {

    SetWindowLongPtr(this->_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Retrieve the associated dialog menu if exists
    this->_menu = GetMenu(this->_hwnd);

    if(show) ShowWindow(this->_hwnd, SW_SHOW);

    #ifdef DEBUG
    std::wcout << L"DEBUG => OmDialog(ID="<<this->id()<<L")::modeless (thread=: "<<GetCurrentThreadId()<<L")\n";
    #endif

  } else {
    #ifdef DEBUG
    int err = GetLastError();
    std::wcout << L"ERROR => OmDialog(ID="<<this->id()<<L")::modeless : "<<Om_getErrorStr(err)<<L"\n";
    #endif
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::registered(const char* classname, bool show)
{
  if(!this->id())
    return;

  if(this->_hwnd)
    return;

  // register window class
  WNDCLASS	wc = {};
  wc.style = CS_HREDRAW|CS_VREDRAW;
  wc.cbWndExtra = DLGWINDOWEXTRA;
  wc.hInstance = this->_hins;
  wc.hCursor = LoadCursor(nullptr,IDC_ARROW);
  wc.hIcon = nullptr;
  wc.lpfnWndProc = DefDlgProc;
  wc.lpszClassName = classname;

  if(!RegisterClass(&wc))
    return;

  this->_hwnd = CreateDialogParamW( this->_hins,
                                    MAKEINTRESOURCEW(this->id()),
                                    (this->_parent) ? this->_parent->_hwnd : nullptr,
                                    reinterpret_cast<DLGPROC>(this->_wndproc),
                                    reinterpret_cast<LPARAM>(this));

  if(this->_hwnd != nullptr) {

    SetWindowLongPtr(this->_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Retrieve the associated dialog menu if exists
    this->_menu = GetMenu(this->_hwnd);

    if(show) ShowWindow(this->_hwnd, SW_SHOW);
  }

  #ifdef DEBUG
  std::wcout << L"DEBUG => OmDialog(ID="<<this->id()<<L")::registered (thread=: "<<GetCurrentThreadId()<<L")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::refresh()
{
  if(this->_hwnd) {
    //if(IsWindowVisible(this->_hwnd)) {
    if(!this->_init) {

      this->_onRefresh();

      for(size_t i = 0; i < this->_child.size(); ++i)
        this->_child[i]->refresh();
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::quit()
{
  for(size_t i = 0; i < this->_child.size(); ++i)
    this->_child[i]->quit();

  if(this->_hwnd) {

    // prevent onQuit to called twice in case dialog first
    // attempt to quit was delayed due to non-safe
    if(!this->_wait_safe)
      this->_onQuit();

    // check whether dialog is safe to quit
    if(this->_safe) {

      SendMessage(this->_hwnd, UWM_QUIT, 0, 0);

    } else {

      // we are waiting for dialog to be safe again to quit
      this->_wait_safe = true;

    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::addChild(OmDialog* dialog)
{
  if(dialog->_parent) {

    for(size_t i = 0; i < dialog->_parent->_child.size(); ++i) {

      if(dialog->_parent->_child[i] == dialog) {

        dialog->_parent->_child.erase(dialog->_parent->_child.begin() + i);
        break;
      }
    }
  }

  dialog->_parent = this;
  dialog->_hins = this->_hins;
  dialog->_data = this->_data;

  this->_child.push_back(dialog);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::setParent(OmDialog* dialog)
{
  if(this->_parent) {

    for(size_t i = 0; i < this->_parent->_child.size(); ++i) {

      if(this->_parent->_child[i] == this) {

        this->_parent->_child.erase(this->_parent->_child.begin() + i);
        break;
      }
    }
  }

  this->_parent = dialog;

  if(this->_parent) {
    this->_hins = this->_parent->_hins;
    this->_data = this->_parent->_data;
    this->_parent->_child.push_back(this);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::setAccel(long id)
{
  this->_accel = LoadAccelerators(this->_hins, MAKEINTRESOURCE(id));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::setData(void* data)
{
  this->_data = data;

  for(size_t i = 0; i < this->_child.size(); ++i)
    this->_child[i]->setData(this->_data);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::setStyle(long style, long exstyle)
{
  if(this->_hwnd) {
    SetWindowLongPtr(this->_hwnd, GWL_STYLE, style);
    SetWindowLongPtr(this->_hwnd, GWL_EXSTYLE, exstyle);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmDialog::sendMessage(MSG* msg) const
{
  if(this->_hwnd) {

    for(size_t i = 0; i < this->_child.size(); ++i)
      if(this->_child[i]->sendMessage(msg))
        return true;

    return IsDialogMessage(this->_hwnd, msg);
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::loopMessage() const
{
  MSG msg;
  while(GetMessage(&msg, nullptr, 0, 0) > 0) {

    if(!sendMessage(&msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    if(this->_accel)
      TranslateAccelerator(this->_hwnd, this->_accel, &msg);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmDialog::redrawItem(unsigned id, RECT* rect, uint32_t falgs) const
{
  HWND hCtrl = GetDlgItem(this->_hwnd, id);
  RedrawWindow(hCtrl, rect, nullptr, falgs);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::setItemText(unsigned id, const OmWString& text) const
{
  SendMessageW(GetDlgItem(this->_hwnd, id), WM_SETTEXT, 0, reinterpret_cast<LPARAM>(text.c_str()));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmDialog::getItemText(unsigned id, OmWString& text) const
{
  HWND hCtrl = GetDlgItem(this->_hwnd, id);
  int len = SendMessageW(hCtrl, WM_GETTEXTLENGTH , 0, 0);
  if(len > 0) {
    text.resize(len);
    int n = SendMessageW(hCtrl, WM_GETTEXT , len + 1, reinterpret_cast<LPARAM>(&text[0]));
    // Under certain conditions, the DefWindowProc function returns a value that is
    // larger than the actual length of the text. This occurs with certain mixtures
    // of ANSI and Unicode, and is due to the system allowing for the possible
    // existence of double-byte character set (DBCS) characters within the text.
    if(n < len) text.resize(n);
    return n;
  }
  text.clear();
  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmDialog::getLbText(unsigned id, unsigned i, OmWString& text) const
{
  HWND hCtrl = GetDlgItem(this->_hwnd, id);
  int len = SendMessageW(hCtrl, LB_GETTEXTLEN, i, 0);
  if(len > 0) {
    text.resize(len);
    int n = SendMessageW(hCtrl, LB_GETTEXT, i, reinterpret_cast<LPARAM>(&text[0]));
    // Under certain conditions, the DefWindowProc function returns a value that is
    // larger than the actual length of the text. This occurs with certain mixtures
    // of ANSI and Unicode, and is due to the system allowing for the possible
    // existence of double-byte character set (DBCS) characters within the text.
    if(n < len) text.resize(n);
    return n;
  }
  text.clear();
  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmDialog::findLvParam(unsigned id, uint64_t param) const
{
  // necessary to update icon in ListView
  LVFINDINFOW lvFind = {};
  lvFind.flags = LVFI_PARAM;
  lvFind.lParam = static_cast<LPARAM>(param); //< FIXME: What about 32-bit systems ?

  HWND hCtrl = GetDlgItem(this->_hwnd, id);
  return SendMessageW(hCtrl, LVM_FINDITEMW, -1, reinterpret_cast<LPARAM>(&lvFind));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmDialog::getLvSubRect(unsigned id, uint64_t item, uint32_t subitem, RECT* rect) const
{
  rect->top = subitem; rect->left = LVIR_BOUNDS;
  HWND hCtrl = GetDlgItem(this->_hwnd, id);
  return SendMessageW(hCtrl, LVM_GETSUBITEMRECT, item, reinterpret_cast<LPARAM>(rect));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP OmDialog::setStImage(unsigned id, HBITMAP image) const
{
  LPARAM lParam = reinterpret_cast<LPARAM>(image);
  LRESULT lResult = SendMessageW(GetDlgItem(this->_hwnd, id), STM_SETIMAGE, IMAGE_BITMAP, lParam);
  return reinterpret_cast<HBITMAP>(lResult);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HICON OmDialog::setStIcon(unsigned id, HICON icon) const
{
  WPARAM wParam = reinterpret_cast<LPARAM>(icon);
  LRESULT lResult = SendMessageW(GetDlgItem(this->_hwnd, id), STM_SETICON, wParam, 0);
  return reinterpret_cast<HICON>(lResult);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP OmDialog::setBmImage(unsigned id, HBITMAP image) const
{
  LPARAM lParam = reinterpret_cast<LPARAM>(image);
  LRESULT lResult = SendMessageW(GetDlgItem(this->_hwnd, id), BM_SETIMAGE, IMAGE_BITMAP, lParam);
  return reinterpret_cast<HBITMAP>(lResult);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HICON OmDialog::setBmIcon(unsigned id, HICON icon) const
{
  LPARAM lParam = reinterpret_cast<LPARAM>(icon);
  LRESULT lResult = SendMessageW(GetDlgItem(this->_hwnd, id), BM_SETIMAGE, IMAGE_ICON, lParam);
  return reinterpret_cast<HICON>(lResult);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_setItemPos(unsigned id, long x, long y, long w, long h, bool pixel)
{
  long rect[4] = {x, y, w, h};
  if(!pixel) MapDialogRect(this->_hwnd, reinterpret_cast<LPRECT>(&rect));
  SetWindowPos(GetDlgItem(this->_hwnd, id), nullptr, rect[0], rect[1], rect[2], rect[3], SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
  //DeferWindowPos(this->_hdwp, GetDlgItem(this->_hwnd, id), nullptr,rect[0], rect[1], rect[2], rect[3], SWP_NOZORDER|SWP_NOACTIVATE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_setChildPos(HWND hwnd, long x, long y, long w, long h, bool pixel)
{
  long rect[4] = {x, y, w, h};
  if(!pixel) MapDialogRect(this->_hwnd, reinterpret_cast<LPRECT>(&rect));
  SetWindowPos(hwnd, nullptr, rect[0], rect[1], rect[2], rect[3], SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
  //DeferWindowPos(this->_hdwp, hwnd, nullptr, rect[0], rect[1], rect[2], rect[3], SWP_NOZORDER|SWP_NOACTIVATE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_getItemPos(unsigned id, long* pos, bool pixel)
{
  long rect[4] = {};
  GetWindowRect(GetDlgItem(this->_hwnd, id), reinterpret_cast<LPRECT>(&rect));
  MapWindowPoints(HWND_DESKTOP, this->_hwnd, reinterpret_cast<LPPOINT>(&rect), 1);
  pos[0] = (pixel) ? rect[0] : MulDiv(rect[0], 4, this->_ubase[0]); // left, unit x
  pos[1] = (pixel) ? rect[1] : MulDiv(rect[1], 8, this->_ubase[1]); // top, unit y
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_getItemSize(unsigned id, long* size, bool pixel)
{
  long rect[4] = {};
  GetWindowRect(GetDlgItem(this->_hwnd, id), reinterpret_cast<LPRECT>(&rect));
  size[0] = (pixel) ? rect[2] : MulDiv(rect[2] - rect[0], 4, this->_ubase[0]); // right - left, unit x
  size[1] = (pixel) ? rect[3] : MulDiv(rect[3] - rect[1], 8, this->_ubase[1]); // bottom - top, unit y
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_createTooltip(unsigned id, const OmWString& text)
{
  // create new Tooltip control
  HWND hTtip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr,
                            WS_POPUP|TTS_ALWAYSTIP,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            this->_hwnd, nullptr,
                            this->_hins, nullptr);

  // associate the Tooltip with the target control.
  TTTOOLINFOW toolInfo = { };
  toolInfo.cbSize = sizeof(toolInfo);
  toolInfo.hwnd = this->_hwnd;
  toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
  toolInfo.uId = reinterpret_cast<UINT_PTR>(GetDlgItem(this->_hwnd, id));
  toolInfo.lpszText = const_cast<LPWSTR>(text.c_str());

  SendMessageW(hTtip, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&toolInfo));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_setSafe(bool safe)
{
  #ifdef DEBUG
  std::wcout << L"DEBUG => OmDialog(ID="<<this->id()<<L")::_setSafe (thread=: "<<GetCurrentThreadId()<<L")\n";
  #endif

  if(safe && !this->_safe) {

    // check for all children to be safe in order
    // to make this instance safe
    for(size_t i = 0; i < this->_child.size(); ++i)
      if(!this->_child[i]->_safe)
        return;
  }

  this->_safe = safe;

  // check whether dialog wait safe to quit
  if(this->_wait_safe && this->_safe)
    this->quit();

  // propagate to parent
  if(this->_parent)
    this->_parent->_setSafe(safe);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_onInit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_onShow()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_onHide()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_onResize()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_onClose()
{
  // default behavior is to quit
  this->quit();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmDialog::_onMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR CALLBACK OmDialog::_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  OmDialog* self;

  if(msg == WM_INITDIALOG) {
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lParam));
    self = reinterpret_cast<OmDialog*>(lParam);
  } else {
    self = reinterpret_cast<OmDialog*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  if(self) {

    LONG temp[4] = {0, 0, 4, 8};

    switch(msg)
    {

    case WM_ACTIVATE:
      self->_active = (LOWORD(wParam) != WA_INACTIVE);
      #ifdef DEBUG
        if(LOWORD(wParam) == WA_INACTIVE) {
          std::cout << "DEBUG => OmDialog(ID=" << (int)self->id() << ")::_wndproc : WA_INACTIVE\n";
        } else {
          std::cout << "DEBUG => OmDialog(ID=" << (int)self->id() << ")::_wndproc : WA_ACTIVE\n";
        }
      #endif
      break; // case WM_ACTIVATE:

    case WM_INITDIALOG:
      #ifdef DEBUG
      std::cout << "DEBUG => OmDialog(ID=" << (int)self->id() << ")::_wndproc : WM_INITDIALOG\n";
      #endif
      self->_init = true;
      return 1; // case WM_INITDIALOG:

    case WM_SHOWWINDOW:
      if(wParam) { // SHOW

        // Get the initial window size and store it as min size
        GetWindowRect(hwnd, reinterpret_cast<LPRECT>(&self->_recw));
        self->_size[0] = self->_recw[2] - self->_recw[0];
        self->_size[1] = self->_recw[3] - self->_recw[1];
        self->_limit[0] = self->_size[0];
        self->_limit[1] = self->_size[1];

        // Calculate the dialog base unit
        MapDialogRect(hwnd, reinterpret_cast<LPRECT>(&temp));
        self->_ubase[0] = temp[2];
        self->_ubase[1] = temp[3];

        // Update dialog client rect
        GetClientRect(hwnd, reinterpret_cast<LPRECT>(&self->_recc));
        // calculate dialog client size in base unit
        self->_usize[0] = MulDiv(self->_recc[2], 4, self->_ubase[0]);
        self->_usize[1] = MulDiv(self->_recc[3], 8, self->_ubase[1]);

        // if this is the first show after WM_INITDIALOG
        if(self->_init) {
          self->_init = false;
          self->_onInit();
        }

        // call user function
        self->_onShow();

        // call user function
        self->_onResize();

      } else {
        // call user function
        self->_onHide();
      }
      return 0; // case WM_SHOWWINDOW

    case WM_WINDOWPOSCHANGED:
      #ifdef DEBUG
      //std::cout << "DEBUG => OmDialog(ID=" << (int)self->id() << ")::_wndproc : WM_WINDOWPOSCHANGED\n";
      #endif
      return 0; // case WM_WINDOWPOSCHANGED

    case WM_WINDOWPOSCHANGING:
      #ifdef DEBUG
      //std::cout << "DEBUG => OmDialog(ID=" << (int)self->id() << ")::_wndproc : WM_WINDOWPOSCHANGING\n";
      #endif
      return 0; // case WM_WINDOWPOSCHANGING

    case WM_MOVE:
      return 0; // case WM_MOVE

    case WM_SIZE:
      // Update dialog window rect
      GetWindowRect(hwnd, reinterpret_cast<LPRECT>(&self->_recw));
      self->_size[0] = self->_recw[2] - self->_recw[0];
      self->_size[1] = self->_recw[3] - self->_recw[1];
      // Update dialog client rect
      GetClientRect(hwnd, reinterpret_cast<LPRECT>(&self->_recc));
      // calculate dialog size in base unit
      self->_usize[0] = MulDiv(self->_recc[2], 4, self->_ubase[0]);
      self->_usize[1] = MulDiv(self->_recc[3], 8, self->_ubase[1]);

      if(wParam == SIZE_MINIMIZED)
        self->_minimized = true;

      if(wParam == SIZE_RESTORED) {
        if(self->_minimized)
          self->_minimized = false;
      }

      // call user function
      self->_onResize();
      #ifdef DEBUG
      //std::cout << "DEBUG => OmDialog(ID=" << (int)self->id() << ")::_wndproc : WM_SIZE\n";
      #endif

      return 0; // case WM_SIZE

    case WM_GETMINMAXINFO:
      // Set minimum window size as initial window size
      reinterpret_cast<LPMINMAXINFO>(lParam)->ptMinTrackSize.x = self->_limit[0];
      reinterpret_cast<LPMINMAXINFO>(lParam)->ptMinTrackSize.y = self->_limit[1];
      return 0; // case WM_GETMINMAXINFO:

    case 736: // WM_DPICHANGED
    case WM_FONTCHANGE:
      #ifdef DEBUG
        std::cout << "DEBUG => OmDialog(ID=" << (int)self->id() << ")::_wndproc : WM_DPICHANGED or WM_FONTCHANGE\n";
      #endif
      // I think this will never happen...
      MapDialogRect(hwnd, reinterpret_cast<LPRECT>(&temp));
      self->_ubase[0] = temp[2];
      self->_ubase[1] = temp[3];

      // call user function
      self->_onResize();

      return 0; // case WM_DPICHANGED WM_FONTCHANGE

    case WM_CLOSE:
      self->_onClose();
      return 0; // case WM_CLOSE

    case UWM_QUIT: //< Custom QUIT message to destroy window from any thread
      if(DestroyWindow(self->_hwnd)) {

        self->_hwnd = nullptr;

        if(self->_modal) {
          // in case the window is modal (typically a sub-dialog window)
          // the parent window must be enabled and activated back again
          EnableWindow(self->_parent->_hwnd, true);
          SetActiveWindow(self->_parent->_hwnd);
          self->_modal = false;
        }

        if(!self->_parent)
          PostQuitMessage(0);
      }
      return 0;

    case WM_DESTROY:
      #ifdef DEBUG
        std::cout << "DEBUG => OmDialog(ID=" << (int)self->id() << ")::_wndproc : WM_DESTROY\n";
      #endif
      return 0;
    }

    #ifdef DEBUG
    //std::cout << "DEBUG => OmDialog(ID=" << (int)self->id() << ")::_wndproc : "<< msg <<"\n";
    #endif

    return self->_onMsg(msg, wParam, lParam);
  }

  return 0;
}
