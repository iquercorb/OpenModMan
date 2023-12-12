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

#include "OmModMan.h"

#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiHelpLog.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiHelpLog::OmUiHelpLog(HINSTANCE hins) : OmDialog(hins)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiHelpLog::~OmUiHelpLog()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_EC_RESUL, WM_GETFONT));
  if(hFt) DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiHelpLog::id() const
{
  return IDD_HELP_LOG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpLog::_log_notify_cb(void* ptr, OmNotify notify, uint64_t param)
{
  OM_UNUSED(notify);

  OmUiHelpLog* self = reinterpret_cast<OmUiHelpLog*>(ptr);

  size_t len = self->msgItem(IDC_EC_RESUL, WM_GETTEXTLENGTH);
  self->msgItem(IDC_EC_RESUL, EM_SETSEL, len, len);
  self->msgItem(IDC_EC_RESUL, EM_REPLACESEL, 0, static_cast<LPARAM>(param));
  self->msgItem(IDC_EC_RESUL, WM_VSCROLL, SB_BOTTOM, 0);
  self->msgItem(IDC_EC_RESUL, 0, 0, RDW_ERASE|RDW_INVALIDATE);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpLog::_onInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(IDI_APP,2),Om_getResIcon(IDI_APP,1));

  // dialog is modeless so we set dialog title with app name
  this->setCaption(L"Debug log ");

  HFONT hFt = Om_createFont(14, 400, L"Consolas");
  this->msgItem(IDC_EC_RESUL, WM_SETFONT, reinterpret_cast<WPARAM>(hFt), true);

  OmModMan* ModMan = reinterpret_cast<OmModMan*>(this->_data);
  if(!ModMan) return;

  this->msgItem(IDC_EC_RESUL, EM_SETLIMITTEXT, 0, 0);
  this->msgItem(IDC_EC_RESUL, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(ModMan->currentLog().c_str()));

  ModMan->addLogNotify(OmUiHelpLog::_log_notify_cb, this);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpLog::_onResize()
{
  this->_setItemPos(IDC_EC_RESUL, 5, 5, this->cliUnitX()-10, this->cliUnitY()-40);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->cliUnitY()-25, this->cliUnitX()-10, 1);
  // Close button
  this->_setItemPos(IDC_BC_CLOSE, this->cliUnitX()-54, this->cliUnitY()-19, 50, 14);

  // redraw the window
  RedrawWindow(this->_hwnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiHelpLog::_onQuit()
{
  OmModMan* ModMan = reinterpret_cast<OmModMan*>(this->_data);
  if(!ModMan) return;

  ModMan->removeLogNotify(OmUiHelpLog::_log_notify_cb);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiHelpLog::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {
    switch(LOWORD(wParam))
    {
    case IDC_BC_CLOSE:
      this->quit();
      break;
    }
  }

  return false;
}

