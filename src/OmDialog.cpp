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

#include "OmDialog.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmDialog::OmDialog(HINSTANCE hins) :
  _hins(hins),
  _hwnd(nullptr),
  _parent(nullptr),
  _child(),
  _accel(nullptr),
  _rect(),
  _data(nullptr),
  _init(true),
  _modal(false)
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
     if(this->_child[i]->id() == id) return this->_child[i];

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
  if(!this->id())
    return;

  if(this->_hwnd)
    return;

  this->_hwnd = CreateDialogParamW( this->_hins,
                                    MAKEINTRESOURCEW(this->id()),
                                    (this->_parent)?this->_parent->_hwnd:nullptr,
                                    (DLGPROC)this->_wndproc,
                                    (LPARAM)this);

  if(this->_hwnd != nullptr) {

    SetWindowLongPtr(this->_hwnd, GWLP_USERDATA, (LONG_PTR)this);

    if(this->_parent) {
      EnableWindow(this->_parent->_hwnd, false);
      this->_modal = true;
    }

    if(show)
      ShowWindow(this->_hwnd, SW_SHOW);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::modeless(bool show)
{
  if(!this->id())
    return;

  if(this->_hwnd)
    return;

  this->_hwnd = CreateDialogParamW( this->_hins,
                                    MAKEINTRESOURCEW(this->id()),
                                    (this->_parent)?this->_parent->_hwnd:nullptr,
                                    (DLGPROC)this->_wndproc,
                                    (LPARAM)this);

  if(this->_hwnd != nullptr) {

    SetWindowLongPtr(this->_hwnd, GWLP_USERDATA, (LONG_PTR)this);

    if(show)
      ShowWindow(this->_hwnd, SW_SHOW);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::refresh()
{
  if(this->_hwnd) {
    if(IsWindowVisible(this->_hwnd)) {

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
  for(size_t i = 0; i < this->_child.size(); ++i) {
    this->_child[i]->quit();
  }

  this->_onQuit();

  DestroyWindow(this->_hwnd);

  this->_hwnd = nullptr;

  if(this->_modal) {
    // in case the window is modal (typically a sub-dialog window)
    // the parent window must be enabled and activated back again
    EnableWindow(this->_parent->_hwnd, true);
    SetActiveWindow(this->_parent->_hwnd);
    this->_modal = false;
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
void OmDialog::setAccelerator(long id)
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
bool OmDialog::sendMessage(MSG* msg) const
{
  if(this->_hwnd) {
    for(size_t i = 0; i < this->_child.size(); ++i) {
      if(this->_child[i]->sendMessage(msg))
        return true;
    }

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
void OmDialog::setItemText(unsigned id, const wstring& text)
{
  SendMessageW(GetDlgItem(this->_hwnd, id), WM_SETTEXT, 0, reinterpret_cast<LPARAM>(text.c_str()));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmDialog::getItemText(unsigned id, wstring& text) const
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
void OmDialog::_setItemPos(unsigned id, long x, long y, long w, long h)
{
  long rect[4] = {x, y, w, h};
  MapDialogRect(this->_hwnd, (LPRECT)&rect);
  SetWindowPos(GetDlgItem(this->_hwnd, id), 0, rect[0], rect[1], rect[2], rect[3], SWP_NOZORDER|SWP_NOACTIVATE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmDialog::_createTooltip(unsigned id, const wstring& text)
{
  // create new Tooltip control
  HWND hTtip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr,
                            WS_POPUP|TTS_ALWAYSTIP,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            this->_hwnd, nullptr,
                            this->_hins, nullptr);

  // copy text to local buffer
  wchar_t wcbuf[OMM_MAX_PATH];
  swprintf(wcbuf, OMM_MAX_PATH, L"%ls", text.c_str());

  // associate the Tooltip with the target control.
  TTTOOLINFOW toolInfo = { };
  toolInfo.cbSize = sizeof(toolInfo);
  toolInfo.hwnd = this->_hwnd;
  toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
  toolInfo.uId = (UINT_PTR)GetDlgItem(this->_hwnd, id);
  //toolInfo.lpszText = wcbuf;
  toolInfo.lpszText = (LPWSTR)text.c_str();

  SendMessageW(hTtip, TTM_ADDTOOLW, 0, (LPARAM)&toolInfo);
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
bool OmDialog::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR CALLBACK OmDialog::_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OmDialog* dialog;

  if(uMsg == WM_INITDIALOG ) {
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lParam );
    dialog = reinterpret_cast<OmDialog*>(lParam);
  } else {
    dialog = reinterpret_cast<OmDialog*>((LPARAM)GetWindowLongPtr(hWnd, GWLP_USERDATA));
  }

  if(dialog) {

    LONG size[4] = {0, 0, 4, 8};

    switch(uMsg)
    {

    case WM_INITDIALOG:
      dialog->_init = true;
      return true; // case WM_INITDIALOG:

    case WM_SHOWWINDOW:
      if(wParam) { // SHOW
        // Get the initial window size and store it as min size
        GetWindowRect(hWnd, &dialog->_rect);
        dialog->_limit[0] = dialog->_rect.right - dialog->_rect.left;
        dialog->_limit[1] = dialog->_rect.bottom - dialog->_rect.top;

        // if this is the first show after WM_INITDIALOG
        if(dialog->_init) {
          dialog->_init = false;
          dialog->_onInit();
        }

        // call user function
        dialog->_onShow();

        // Initialize proper client rect and call resize function to force
        // controls alignments
        GetClientRect(hWnd, &dialog->_rect);
        // Calculate the dialog base unit
        MapDialogRect(dialog->_hwnd, (LPRECT)&size);
        dialog->_unit[0] = size[2];
        dialog->_unit[1] = size[3];
        // calculate dialog size in base unit
        dialog->_size[0] = MulDiv(dialog->_rect.right, 4, dialog->_unit[0]);
        dialog->_size[1] = MulDiv(dialog->_rect.bottom, 8, dialog->_unit[1]);

        // call user function
        dialog->_onResize();
      }
      return false; // case WM_SHOWWINDOW:

    case WM_WINDOWPOSCHANGED:
      return false; // case WM_WINDOWPOSCHANGED:

    case WM_MOVE:
      return false; // case WM_MOVE:

    case WM_SIZE:
      GetClientRect(hWnd, &dialog->_rect);
      // Calculate the dialog base unit
      MapDialogRect(dialog->_hwnd, (LPRECT)&size);
      dialog->_unit[0] = size[2];
      dialog->_unit[1] = size[3];
      // calculate dialog size in base unit
      dialog->_size[0] = MulDiv(dialog->_rect.right, 4, dialog->_unit[0]);
      dialog->_size[1] = MulDiv(dialog->_rect.bottom, 8, dialog->_unit[1]);

      // call user function
      dialog->_onResize();
      return false; // case WM_SIZE WM_MOVE WM_WINDOWPOSCHANGED

    case WM_GETMINMAXINFO:
      // Set minimum window size as initial window size
      ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = dialog->_limit[0];
      ((LPMINMAXINFO)lParam)->ptMinTrackSize.y = dialog->_limit[1];
      return false; // case WM_GETMINMAXINFO:

    case 736: // WM_DPICHANGED
      std::cout << "WM_DPICHANGED\n";
      return false; // case WM_DPICHANGED:

    case WM_CLOSE:
      dialog->_onClose();
      return false; // case WM_CLOSE:

    case WM_DESTROY:
      return false;

    }

    return dialog->_onMsg(uMsg, wParam, lParam);
  }

  return false;
}
