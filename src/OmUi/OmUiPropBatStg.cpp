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
  #include <algorithm>    // std::find

#include "OmBaseUi.h"

#include "OmContext.h"
#include "OmBatch.h"
#include "OmDialogProp.h"

#include "OmUiPropBat.h"

#include "OmUtilStr.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
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
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onCkBoxIonly()
{
  this->setChParam(BAT_PROP_STG_IONLY, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INP01,  L"Installation Batch name, to identify it");
  this->_createTooltip(IDC_BC_CKBX1,  L"Installation Batch will installs selected packages without uninstalling others");

  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onResize()
{
  // Batch Title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 50, 25, 240, 9);
  this->_setItemPos(IDC_EC_INP01, 50, 35, this->cliUnitX()-100, 13);

  // Install Only checkbox
  this->_setItemPos(IDC_BC_CKBX1, 50, 65, this->cliUnitX()-100, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBatStg::_onRefresh()
{
  OmBatch* pBat = static_cast<OmUiPropBat*>(this->_parent)->batCur();
  if(!pBat) return;

  OmContext* pCtx = pBat->pCtx();
  if(!pCtx) return;

  // batch title
  this->setItemText(IDC_EC_INP01, pBat->title());

  // batch install-only option
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, pBat->installOnly());

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropBatStg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_EC_INP01: //< Title EditText
      if(HIWORD(wParam) == EN_CHANGE)
        // user modified parameter, notify it
        this->setChParam(BAT_PROP_STG_TITLE, true);
      break;

    case IDC_BC_CKBX1:
      this->_onCkBoxIonly();
      break;
    }
  }

  return false;
}
