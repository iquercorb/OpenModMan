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

//#include "OmBaseApp.h"

#include "OmModChan.h"

//#include "OmUiMan.h"
#include "OmUiPropChn.h"

//#include "OmUtilDlg.h"
//#include "OmUtilStr.h"
//#include "OmUtilSys.h"
//#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropChnDnl.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnDnl::OmUiPropChnDnl(HINSTANCE hins) : OmDialogPropTab(hins)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnDnl::~OmUiPropChnDnl()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropChnDnl::id() const
{
  return IDD_PROP_CHN_DNL;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnDnl::_limit_rate_toggle()
{
  bool enabled = this->msgItem(IDC_BC_CKBX4, BM_GETCHECK);

  this->enableItem(IDC_EC_NUM01, enabled);
  this->enableItem(IDC_UD_SPIN1, enabled);
  this->redrawItem(IDC_UD_SPIN1, nullptr, RDW_INVALIDATE);

  // notify parameters changes
  this->paramCheck(CHN_PROP_DNL_LIMITS);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnDnl::_limit_thread_toggle()
{
  bool enabled = this->msgItem(IDC_BC_CKBX5, BM_GETCHECK);

  this->enableItem(IDC_EC_NUM02, enabled);
  this->enableItem(IDC_UD_SPIN2, enabled);
  this->redrawItem(IDC_UD_SPIN2, nullptr, RDW_INVALIDATE);

  // notify parameters changes
  this->paramCheck(CHN_PROP_DNL_LIMITS);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnDnl::_onTbInit()
{
  DWORD ud_style = WS_CHILDWINDOW|WS_VISIBLE|UDS_SETBUDDYINT|UDS_HOTTRACK|UDS_NOTHOUSANDS;

  CreateWindowEx(WS_EX_LEFT|WS_EX_LTRREADING, UPDOWN_CLASS, nullptr, ud_style, 0, 0, 0, 0,
                 this->_hwnd, reinterpret_cast<HMENU>(IDC_UD_SPIN1), this->_hins, nullptr);

  this->msgItem(IDC_UD_SPIN1, UDM_SETBUDDY, reinterpret_cast<WPARAM>(this->getItem(IDC_EC_NUM01)));
  this->msgItem(IDC_UD_SPIN1, UDM_SETRANGE32, 10, 999999);

  CreateWindowEx(WS_EX_LEFT|WS_EX_LTRREADING, UPDOWN_CLASS, nullptr, ud_style, 0, 0, 0, 0,
                 this->_hwnd, reinterpret_cast<HMENU>(IDC_UD_SPIN2), this->_hins, nullptr);

  this->msgItem(IDC_UD_SPIN2, UDM_SETBUDDY, reinterpret_cast<WPARAM>(this->getItem(IDC_EC_NUM02)));
  this->msgItem(IDC_UD_SPIN2, UDM_SETRANGE32, 1, 64);

  this->_createTooltip(IDC_BC_CKBX1,  L"Warn if Mods download requires additional dependencies to be downloaded");
  this->_createTooltip(IDC_BC_CKBX2,  L"Warn if Mods to download have missing dependencies");
  this->_createTooltip(IDC_BC_CKBX3,  L"Warn if supersede Mods will delete previous versions required by other");

  this->_createTooltip(IDC_BC_RAD01,  L"On Mod supersede, the previous Mod is moved to recycle bin");
  this->_createTooltip(IDC_BC_RAD02,  L"On Mod supersede, the previous Mod is renamed with .old extension");

  this->_createTooltip(IDC_BC_CKBX4,  L"Limit download rate for individual download thread");
  this->_createTooltip(IDC_EC_NUM01,  L"Maximum download rate in Kilobytes per seconds");
  this->_createTooltip(IDC_BC_CKBX5,  L"Limit count of concurrent download thread");
  this->_createTooltip(IDC_EC_NUM02,  L"Maximum count of concurrent download");

  // Update values
  this->_onTbRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnDnl::_onTbResize()
{
  int32_t y_base = 40;

  // Warnings label
  this->_setItemPos(IDC_SC_LBL02, 50, y_base, 300, 16, true);
  // Warnings CheckBoxes
  this->_setItemPos(IDC_BC_CKBX1, 75, y_base+20, 300, 16, true);
  this->_setItemPos(IDC_BC_CKBX2, 75, y_base+40, 300, 16, true);
  this->_setItemPos(IDC_BC_CKBX3, 75, y_base+60, 300, 16, true);

  // Package upgrade label
  this->_setItemPos(IDC_SC_LBL03, 50, y_base+100, 300, 16, true);
  // Move to trash RadioButton
  this->_setItemPos(IDC_BC_RAD01, 75, y_base+120, 300, 16, true);
  // Rename RadioButton
  this->_setItemPos(IDC_BC_RAD02, 75, y_base+140, 300, 16, true);

  // Download limits Label
  this->_setItemPos(IDC_SC_LBL04, 50, y_base+170, 300, 16, true);
  // Max rate CheckBox, Entry, KB/s label
  this->_setItemPos(IDC_BC_CKBX4, 75, y_base+190, 135, 16, true);
  this->_setItemPos(IDC_EC_NUM01, 220, y_base+188, 60, 19, true);
  this->_setItemPos(IDC_UD_SPIN1, 280, y_base+187, 15, 21, true);
  this->_setItemPos(IDC_SC_LBL05, 300, y_base+190, 40, 16, true);

  // Max thread CheckBox & entry
  this->_setItemPos(IDC_BC_CKBX5, 75, y_base+210, 135, 16, true);
  this->_setItemPos(IDC_EC_NUM02, 220, y_base+208, 60, 19, true);
  this->_setItemPos(IDC_UD_SPIN2, 280, y_base+207, 15, 21, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnDnl::_onTbRefresh()
{
  OmModChan* ModChan = static_cast<OmUiPropChn*>(this->_parent)->ModChan();
  if(!ModChan) return;

  // set warning messages
  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, ModChan->warnExtraDnld());
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, ModChan->warnMissDnld());
  this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, ModChan->warnUpgdBrkDeps());

  // set Upgrade Rename
  this->msgItem(IDC_BC_RAD01, BM_SETCHECK, !ModChan->upgdRename());
  this->msgItem(IDC_BC_RAD02, BM_SETCHECK, ModChan->upgdRename());

  // set download rate limit
  bool limit_rate = (ModChan->downMaxRate() > 0);
  this->msgItem(IDC_BC_CKBX4, BM_SETCHECK, limit_rate);
  this->enableItem(IDC_EC_NUM01, limit_rate);
  this->enableItem(IDC_UD_SPIN1, limit_rate);
  this->msgItem(IDC_UD_SPIN1, UDM_SETPOS32, 0, limit_rate ? ModChan->downMaxRate()/1000 : 300);
  this->redrawItem(IDC_UD_SPIN1, nullptr, RDW_INVALIDATE);

  // set download thread limit
  bool limit_thread = (ModChan->downMaxThread() > 0);
  this->msgItem(IDC_BC_CKBX5, BM_SETCHECK, limit_thread);
  this->enableItem(IDC_EC_NUM02, limit_thread);
  this->enableItem(IDC_UD_SPIN2, limit_thread);
  this->msgItem(IDC_UD_SPIN2, UDM_SETPOS32, 0, limit_thread ? ModChan->downMaxThread() : 5);
  this->redrawItem(IDC_UD_SPIN2, nullptr, RDW_INVALIDATE);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChnDnl::_onTbMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  OM_UNUSED(lParam);

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_CKBX1: //< CheckBox: warn extra downloads
    case IDC_BC_CKBX2: //< CheckBox: warn missing dependency
    case IDC_BC_CKBX3: //< CheckBox: warn upgrade breaks depends
      if(HIWORD(wParam) == BN_CLICKED)
        // notify parameters changes
        this->paramCheck(CHN_PROP_DNL_WARNINGS);
      break;

    case IDC_BC_RAD01: //< Radio: on upgrade move to recycle bin
    case IDC_BC_RAD02: //< Radio: on upgrade rename to .old
      if(HIWORD(wParam) == BN_CLICKED)
        // notify parameters changes
        this->paramCheck(CHN_PROP_DNL_ONUPGRADE);
      break;

    case IDC_BC_CKBX4: //< CheckBox: Limit download rate
      if(HIWORD(wParam) == BN_CLICKED)
        this->_limit_rate_toggle();
      break;

    case IDC_BC_CKBX5: //< CheckBox: Limit download thread
      if(HIWORD(wParam) == BN_CLICKED)
        this->_limit_thread_toggle();
      break;

    case IDC_EC_NUM01: //< Entry: download rate KB/s
    case IDC_EC_NUM02: //< Entry: download thread
      if(HIWORD(wParam) == EN_CHANGE)
        // notify parameters changes
        this->paramCheck(CHN_PROP_DNL_LIMITS);
      break;
    }
  }

  return false;
}
