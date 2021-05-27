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
#include "gui/OmUiNewRep.h"
#include "gui/OmUiPropLocNet.h"
#include "gui/OmUiPropLoc.h"
#include "OmSocket.h"



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocNet::OmUiPropLocNet(HINSTANCE hins) : OmDialog(hins),
  _hBmBcNew(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_ADD), IMAGE_BITMAP, 0, 0, 0))),
  _hBmBcDel(static_cast<HBITMAP>(LoadImage(this->_hins, MAKEINTRESOURCE(IDB_BTN_REM), IMAGE_BITMAP, 0, 0, 0)))
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i)
    this->_chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropLocNet::~OmUiPropLocNet()
{
  //dtor
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropLocNet::id() const
{
  return IDD_PROP_LOC_NET;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocNet::setChParam(unsigned i, bool en)
{
  _chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocNet::_repoDel()
{
  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->location();

  if(pLoc == nullptr)
    return;

  int lb_sel = this->msgItem(IDC_LB_REPLS, LB_GETCURSEL);

  if(lb_sel >= 0) {

    // warns the user before committing the irreparable
    wstring qry = L"Are your sure you want to delete the Repository \"";
    qry += pLoc->repository(lb_sel)->base() + L" - " + pLoc->repository(lb_sel)->name();
    qry += L"\" ?";

    if(Om_dialogBoxQuerryWarn(this->_hwnd, L"Delete Repository", qry)) {
      pLoc->remRepository(lb_sel);
    }

    // refresh list and buttons
    this->_onRefresh();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocNet::_repoChk()
{
  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->location();

  if(pLoc == nullptr)
    return;

  int lb_sel = this->msgItem(IDC_LB_REPLS, LB_GETCURSEL);

  if(lb_sel >= 0) {

    OmRepository* pRep = pLoc->repository(lb_sel);

    OmSocket sock;

    this->setItemText(IDC_SC_DESC1, L"Pending...");

    wstring msg;

    string rep_xml;

    if(sock.httpGet(pRep->url(), rep_xml)) {

      OmConfig rep_def;

      if(rep_def.parse(Om_fromUtf8(rep_xml.c_str()), OMM_CFG_SIGN_REP)) {
        int n = rep_def.xml().childCount(L"package");
        msg = L"Available, providing "+to_wstring(n)+L" package(s)";
      } else {
        msg = L"Invalid XML definition";
      }
    } else {
      msg = L"HTTP request failed (" + sock.lastErrorStr() + L")";
    }

    // get local time
    int t_h, t_m, t_s;
    Om_getTime(&t_s, &t_m, &t_h);

    wchar_t hour[32];
    swprintf(hour, 32, L"[%02d:%02d:%02d] ", t_h, t_m, t_s);

    this->setItemText(IDC_SC_DESC1, hour + msg);
    this->enableItem(IDC_SC_DESC1, true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocNet::_onInit()
{
  // Set buttons inner icons
  this->msgItem(IDC_BC_ADD,   BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcNew));
  this->msgItem(IDC_BC_DEL,   BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcDel));
  //this->msgItem(IDC_BC_CHK,   BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(this->_hBmBcRef));

  // define controls tool-tips
  this->_createTooltip(IDC_LB_LOCLS,  L"Network repositories");

  this->_createTooltip(IDC_BC_DEL,    L"Remove repository");
  this->_createTooltip(IDC_BC_ADD,    L"Add new repository");
  this->_createTooltip(IDC_BC_EDI,    L"Test repository availability");

  this->enableItem(IDC_SC_DESC1, false);

  // Update values
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocNet::_onResize()
{
  // Locations list Label & ListBox
  this->_setItemPos(IDC_SC_LBL01, 5, 20, 64, 9);
  this->_setItemPos(IDC_LB_REPLS, 70, 20, this->width()-107, 30);

  // Remove Button
  this->_setItemPos(IDC_BC_DEL, 70, 55, 50, 14);
  // Add button
  this->_setItemPos(IDC_BC_ADD, this->width()-87, 55, 50, 14);

  // Test label
  this->_setItemPos(IDC_SC_LBL02, 71, 80, 40, 9);
  // Test button & entry
  this->_setItemPos(IDC_BC_CHK, 70, 90, 50, 14);
  this->_setItemPos(IDC_SC_DESC1, 122, 92, this->width()-135, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropLocNet::_onRefresh()
{
  OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->location();

  if(pLoc == nullptr)
    return;

  HWND hLb = this->getItem(IDC_LB_REPLS);

  SendMessageW(hLb, LB_RESETCONTENT, 0, 0);
  if(pLoc) {

    wstring str;
    OmRepository* pRep;

    for(unsigned i = 0; i < pLoc->repositoryCount(); ++i) {
      pRep = pLoc->repository(i);
      str = pRep->base() + L" - " + pRep->name();
      SendMessageW(hLb, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(str.c_str()));
    }
  }

  // Set controls default states and parameters
  this->enableItem(IDC_SC_DESC1, false);
  this->setItemText(IDC_SC_DESC1, L"<no test launched>");

  this->enableItem(IDC_BC_DEL,  false);
  this->enableItem(IDC_BC_CHK,  false);

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropLocNet::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    OmLocation* pLoc = static_cast<OmUiPropLoc*>(this->_parent)->location();

    if(pLoc == nullptr)
      return false;

    int lb_sel;

    switch(LOWORD(wParam))
    {

    case IDC_LB_REPLS: //< Location(s) list List-Box
      lb_sel = this->msgItem(IDC_LB_REPLS, LB_GETCURSEL);
      if(lb_sel >= 0) {
        this->enableItem(IDC_BC_DEL, true);
        this->enableItem(IDC_BC_CHK, true);
        this->enableItem(IDC_SC_DESC1, false);
        this->setItemText(IDC_SC_DESC1, L"<no test launched>");
      }
      break;

    case IDC_BC_ADD: //< New button for Location(s) list
      {
        OmUiNewRep* pUiNewRep = static_cast<OmUiNewRep*>(this->siblingById(IDD_NEW_REP));
        pUiNewRep->setLocation(pLoc);
        pUiNewRep->open(true);
      }
      break;

    case IDC_BC_CHK:
      this->_repoChk();
      break;

    case IDC_BC_DEL: //< Remove button for Location(s) list
      this->_repoDel();
      break;

    }
  }

  return false;
}
