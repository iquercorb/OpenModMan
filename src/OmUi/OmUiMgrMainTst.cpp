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


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiMgrMainTst::OmUiMgrMainTst(HINSTANCE hins) : OmDialog(hins),
  _pUiMgr(nullptr)
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
  std::cout << "DEBUG => OmUiMgrMainTst::_onInit\n";
  #endif

  // retrieve main dialog
  this->_pUiMgr = static_cast<OmUiMgr*>(this->root());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainTst::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainTst::_onShow\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainTst::_onHide()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainTst::_onHide\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainTst::_onResize()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainTst::_onResize\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiMgrMainTst::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiMgrMainTst::_onRefresh\n";
  #endif
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
  if(uMsg == WM_COMMAND) {

  }

  return false;
}
