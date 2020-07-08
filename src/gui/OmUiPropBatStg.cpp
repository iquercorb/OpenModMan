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
#include "OmBatch.h"
#include "OmDialogProp.h"
#include "OmUiPropBat.h"
#include "OmUiPropBatStg.h"



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropBatStg::OmUiPropBatStg(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i) {
    this->_chParam[i] = false;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropBatStg::~OmUiPropBatStg()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropBatStg::id() const
{
  return IDD_PROP_BAT_STG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::setChParam(unsigned i, bool en)
{
  this->_chParam[i] = en;
  reinterpret_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onShow()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Batch name");

  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onResize()
{
  // Batch Title Label & EditControl
  this->_setControlPos(IDC_SC_LBL01, 5, 60, 64, 9);
  this->_setControlPos(IDC_EC_INPT1, 70, 60, this->width()-90, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onRefresh()
{
  OmBatch* batch = reinterpret_cast<OmUiPropBat*>(this->_parent)->batch();

  if(batch == nullptr)
    return;

  SetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, batch->title().c_str());
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropBatStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    OmBatch* batch = reinterpret_cast<OmUiPropBat*>(this->_parent)->batch();

    if(batch == nullptr)
      return false;

    switch(LOWORD(wParam))
    {
    case IDC_EC_INPT1: //< Entry for Title
      // user modified parameter, notify it
      this->setChParam(BAT_PROP_STG_TITLE, true);
      break;
    }
  }

  return false;
}



