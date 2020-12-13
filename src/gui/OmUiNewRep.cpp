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
#include "gui/OmUiNewRep.h"
#include "OmManager.h"
#include "OmSocket.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewRep::OmUiNewRep(HINSTANCE hins) : OmDialog(hins),
  _context(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewRep::~OmUiNewRep()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiNewRep::id() const
{
  return IDD_NEW_REP;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewRep::_test()
{
  OmSocket sock;

  wstring url;
  wstring name;

  this->getItemText(IDC_EC_INPT1, url);
  this->getItemText(IDC_EC_INPT2, name);

  string data;
  sock.httpGet(Om_toUtf8(url), data);

  std::cout << data << "\n";

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewRep::_apply()
{
  OmContext* pCtx = this->_context;

  if(pCtx == nullptr)
    return false;

  this->quit();

  // create new Location in Context
  /*
  if(!pCtx->addRepository(loc_name, loc_dst, loc_lib, loc_bck)) {
    Om_dialogBoxErr(this->_hwnd, L"Location creation failed", pCtx->lastError());
  }
  */

  // refresh all tree from the main dialog
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewRep::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Base URL");
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Section name");

  // set default start values
  this->setItemText(IDC_EC_INPT1, L"https://www.dest1.org");
  this->setItemText(IDC_EC_INPT2, L"default");

  this->enableItem(IDC_BC_OK, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewRep::_onResize()
{
  // Repository URL Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 10, 20, 80, 9);
  this->_setItemPos(IDC_EC_INPT1, 10, 30, this->width()-125, 13);

  // Repository Name Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, this->width()-110, 20, 80, 9);
  this->_setItemPos(IDC_EC_INPT2, this->width()-110, 30, 80, 13);

  // Repository Test button
  this->_setItemPos(IDC_BC_REF, 10, 60, 50, 13);

  // Repository Test status/result
  this->_setItemPos(IDC_SC_DESC1, 10, 90, 100, 18);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Ok and Cancel buttons
  this->_setItemPos(IDC_BC_OK, this->width()-110, this->height()-19, 50, 14);
  this->_setItemPos(IDC_BC_CANCEL, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewRep::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    wstring item_str, brow_str;

    switch(LOWORD(wParam))
    {

    case IDC_EC_INPT1: // Title
      this->getItemText(IDC_EC_INPT1, item_str);
      this->enableItem(IDC_BC_REF, (item_str.size() > 7));
      has_changed = true;
      break;

    case IDC_BC_REF:
      this->_test();
      break;

    case IDC_BC_OK:
      this->_apply();
      break;

    case IDC_BC_CANCEL:
      this->quit();
      break;
    }

    // enable or disable "OK" button according values
    if(has_changed) {

      bool allow = true;

      this->enableItem(IDC_BC_OK, allow);
    }
  }

  return false;
}

