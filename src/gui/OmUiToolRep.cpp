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
#include "gui/OmUiToolRep.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolRep::OmUiToolRep(HINSTANCE hins) : OmDialog(hins),
  _hFtMonos(Om_createFont(14, 400, L"Consolas")),
  _hBmBlank(static_cast<HBITMAP>(LoadImage(hins,MAKEINTRESOURCE(IDB_PKG_BLANK),IMAGE_BITMAP,0,0,0))),
  _image()
{
  //ctor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiToolRep::~OmUiToolRep()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiToolRep::id() const
{
  return IDD_TOOL_REP;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onInit()
{
  // Set font for description
  this->msgItem(IDC_EC_PKTXT, WM_SETFONT, reinterpret_cast<WPARAM>(this->_hFtMonos), true);
  // Set default package picture
  this->msgItem(IDC_SB_PKIMG, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBlank));

  // Enable Create new repository
  this->msgItem(IDC_BC_RAD02, BM_SETCHECK, 1);

  // Disable the Remove package button
  this->enableItem(IDC_BC_REM, false);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiToolRep::_onResize()
{
  // Edit existing RadioButton
  this->_setItemPos(IDC_BC_RAD01, 10, 10, 150, 9);
  // From Folder EditControl & Brows Button
  this->_setItemPos(IDC_EC_INPT1, 10, 20, this->width()-38, 13);
  this->_setItemPos(IDC_BC_BROW1, this->width()-26, 20, 16, 13);

  // Create new RadioButton
  this->_setItemPos(IDC_BC_RAD02, 10, 40, 150, 9);

  // Packages list label
  this->_setItemPos(IDC_SC_LBL01, 10, 63, 60, 13);
  // Packages list Add.. & Remove Buttons
  this->_setItemPos(IDC_BC_ADD, this->width()-92, 58, 40, 14);
  this->_setItemPos(IDC_BC_REM, this->width()-50, 58, 40, 14);
  // Packages list ListBox
  this->_setItemPos(IDC_LB_PKGLS, 10, 75, this->width()-20, 65);

  // Package detail GroupBox
  this->_setItemPos(IDC_GB_GRP01, 10, 140, this->width()-20, this->height()-190);
  // Package file Label & EditText
  this->_setItemPos(IDC_SC_LBL02, 15, 150, 75, 9);
  this->_setItemPos(IDC_EC_ENT02, 85, 150, this->width()-100, 12);
  // Download path Label & EditTexit
  this->_setItemPos(IDC_SC_LBL03, 15, 165, 75, 9);
  this->_setItemPos(IDC_EC_ENT03, 85, 165, this->width()-100, 12);
  // Dependencies Label, Check Button & EditTexit
  this->_setItemPos(IDC_SC_LBL04, 15, 180, 75, 9);
  this->_setItemPos(IDC_EC_ENT04, 85, 181, this->width()-145, 30);
  this->_setItemPos(IDC_BC_CHK, this->width()-55, 180, 40, 14);
  // Snapshot Label & Static Bitmap
  this->_setItemPos(IDC_SC_LBL05, 15, 215, 75, 9);
  this->_setItemPos(IDC_SB_PKIMG, 85, 216, 85, 78);
  // Snapshot Change... & Delete Buttons
  this->_setItemPos(IDC_BC_EDI, this->width()-55, 217, 40, 14);
  this->_setItemPos(IDC_BC_DEL, this->width()-55, 232, 40, 14);
  // Description Label, Load.. & Save Button
  this->_setItemPos(IDC_SC_LBL06, 15, 305, 75, 9);
  this->_setItemPos(IDC_BC_LOAD, this->width()-97, 302, 40, 14);
  this->_setItemPos(IDC_BC_SAVE, this->width()-55, 302, 40, 14);
  // Description EditText
  this->_setItemPos(IDC_EC_PKTXT, 15, 320, this->width()-30, this->height()-375);

  // Save As Button
  this->_setItemPos(IDC_BC_BROW2, 10, this->height()-45, 50, 14);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Close button
  this->_setItemPos(IDC_BC_CLOSE, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiToolRep::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    wstring item_str, brow_str;

    switch(LOWORD(wParam))
    {

    case IDC_BC_CLOSE:
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

