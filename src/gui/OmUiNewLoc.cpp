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
#include "gui/OmUiNewLoc.h"
#include "OmManager.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewLoc::OmUiNewLoc(HINSTANCE hins) : OmDialog(hins),
  _context(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewLoc::~OmUiNewLoc()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiNewLoc::id() const
{
  return IDD_NEW_LOC;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewLoc::_apply()
{
  OmContext* context = this->_context;

  if(context == nullptr)
    return false;

  bool cust_lib, cust_bck;

  wstring loc_name, loc_dst, loc_lib, loc_bck;

  this->getItemText(IDC_EC_INPT1, loc_name);
  if(!Om_isValidName(loc_name)) {
    Om_dialogBoxWarn(this->_hwnd, L"Invalid Location title", OMM_STR_ERR_VALIDNAME);
    return false;
  }

  this->getItemText(IDC_EC_INPT2, loc_dst);
  if(!Om_isDir(loc_dst)) {
    Om_dialogBoxWarn(this->_hwnd, L"Invalid install destination folder", OMM_STR_ERR_ISDIR(loc_dst));
    return false;
  }

  cust_lib = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);
  if(cust_lib) {
    this->getItemText(IDC_EC_INPT3, loc_lib);
    if(!Om_isDir(loc_lib)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid custom library folder", OMM_STR_ERR_ISDIR(loc_lib));
      return false;
    }
  }

  cust_bck = this->msgItem(IDC_BC_CHK02, BM_GETCHECK);
  if(cust_bck) {
    this->getItemText(IDC_EC_INPT4, loc_bck);
    if(!Om_isDir(loc_bck)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid custom backup folder", OMM_STR_ERR_ISDIR(loc_bck));
      return false;
    }
  }

  this->quit();

  // create new Location in Context
  if(!context->makeLocation(loc_name, loc_dst, loc_lib, loc_bck)) {
    Om_dialogBoxErr(this->_hwnd,  L"Location creation failed",
                                  context->lastError());
  }

  // refresh all tree from the main dialog
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewLoc::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_EC_INPT1,  L"Indicative name");

  this->_createTooltip(IDC_EC_INPT2,  L"Package installation destination path");
  this->_createTooltip(IDC_BC_BROW2,  L"Select destination folder");

  this->_createTooltip(IDC_BC_CHK01,  L"Use custom Library folder");
  this->_createTooltip(IDC_EC_INPT3,  L"Custom Library folder path");
  this->_createTooltip(IDC_BC_BROW3,  L"Select custom Library folder");

  this->_createTooltip(IDC_BC_CHK02,  L"Use custom Backup folder");
  this->_createTooltip(IDC_EC_INPT4,  L"Custom Backup folder path");
  this->_createTooltip(IDC_BC_BROW4,  L"Select custom Backup folder");

  // set default start values
  this->setItemText(IDC_EC_INPT1, L"Main Location");
  this->setItemText(IDC_EC_INPT2, L"");
  this->setItemText(IDC_EC_INPT3, L"Main Location\\library");
  this->setItemText(IDC_EC_INPT4, L"Main Location\\backup");

  wstring item_str;

  // enable or disable "OK" button according values
  bool allow = true;

  this->getItemText(IDC_EC_INPT1, item_str);
  if(!item_str.empty()) {

    this->getItemText(IDC_EC_INPT2, item_str);
    if(!item_str.empty()) {

      if(this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INPT3, item_str);
        if(item_str.empty()) allow = false;
      }

      if(this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
        this->getItemText(IDC_EC_INPT4, item_str);
        if(item_str.empty()) allow = false;
      }

    } else {
      allow = false;
    }

  } else {
    allow = false;
  }

  this->enableItem(IDC_BC_OK, allow);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewLoc::_onResize()
{
  // Location title Label & EditControl
  this->_setItemPos(IDC_SC_LBL01, 10, 10, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INPT1, 10, 20, this->width()-25, 13);

  // Location Install Label & EditControl & Browse button
  this->_setItemPos(IDC_SC_LBL02, 10, 50, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INPT2, 10, 60, this->width()-45, 13);
  this->_setItemPos(IDC_BC_BROW2, this->width()-31, 60, 16, 13);

  // Custom Library Label & EditControl & Browse buttonben ess
  this->_setItemPos(IDC_BC_CHK01, 10, 110, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INPT3, 10, 120, this->width()-45, 13);
  this->_setItemPos(IDC_BC_BROW3, this->width()-31, 120, 16, 13);

  // Custom Library Label & EditControl & Browse button
  this->_setItemPos(IDC_BC_CHK02, 10, 140, this->width()-25, 9);
  this->_setItemPos(IDC_EC_INPT4, 10, 150, this->width()-45, 13);
  this->_setItemPos(IDC_BC_BROW4, this->width()-31, 150, 16, 13);

  // ---- separator
  this->_setItemPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Ok and Cancel buttons
  this->_setItemPos(IDC_BC_OK, this->width()-110, this->height()-19, 50, 14);
  this->_setItemPos(IDC_BC_CANCEL, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewLoc::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    bool bm_chk;

    wstring item_str, brow_str;

    switch(LOWORD(wParam))
    {

    case IDC_EC_INPT1: // Title
      this->getItemText(IDC_EC_INPT1, item_str);
      if(Om_isValidName(item_str)) {
        if(!this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
          this->setItemText(IDC_EC_INPT3, item_str + L"\\library");
        }
        if(!this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
          this->setItemText(IDC_EC_INPT4, item_str + L"\\backup");
        }
      } else {
        if(!this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
          this->setItemText(IDC_EC_INPT3, L"<invalid path>\\library");
        }
        if(!this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
          this->setItemText(IDC_EC_INPT4, L"<invalid path>\\backup");
        }
      }
      has_changed = true;
      break;

    case IDC_BC_BROW2: // browse destination
      this->getItemText(IDC_EC_INPT2, item_str);
      if(Om_dialogBrowseDir(brow_str, this->_hwnd, L"Select installation destination folder", item_str)) {
        this->setItemText(IDC_EC_INPT2, brow_str);
      }
      break;

    case IDC_BC_CHK01: // custom library check box
      bm_chk = this->msgItem(IDC_BC_CHK01, BM_GETCHECK);
      this->enableItem(IDC_BC_BROW3, bm_chk);
      this->enableItem(IDC_EC_INPT3, bm_chk);
      if(bm_chk) {
        item_str = L"";
      } else {
        this->getItemText(IDC_EC_INPT1, item_str);
        if(Om_isValidName(item_str)) {
          item_str += L"\\library";
        } else {
          item_str = L"<invalid path>\\library";
        }
      }
      this->setItemText(IDC_EC_INPT3, item_str);
    break;

    case IDC_BC_BROW3: // browse custom library
      this->getItemText(IDC_EC_INPT3, item_str);
      if(Om_dialogBrowseDir(brow_str, this->_hwnd, L"Select packages library custom folder", item_str)) {
        this->setItemText(IDC_EC_INPT3, brow_str);
      }
      break;

    case IDC_BC_CHK02: // custom backup check box
      bm_chk = this->msgItem(IDC_BC_CHK02, BM_GETCHECK);
      this->enableItem(IDC_BC_BROW4, bm_chk);
      this->enableItem(IDC_EC_INPT4, bm_chk);
      if(bm_chk) {
        item_str = L"";
      } else {
        this->getItemText(IDC_EC_INPT1, item_str);
        if(Om_isValidName(item_str)) {
          item_str += L"\\backup";
        } else {
          item_str = L"<invalid path>\\backup";
        }
      }
      this->setItemText(IDC_EC_INPT4, item_str);

    break;

    case IDC_BC_BROW4: // browse custom library
      this->getItemText(IDC_EC_INPT4, item_str);
      if(Om_dialogBrowseDir(brow_str, this->_hwnd, L"Select backup data custom folder", item_str)) {
        this->setItemText(IDC_EC_INPT4, brow_str);
      }
      break;

    case IDC_EC_INPT2: //< Location
    case IDC_EC_INPT3: //< Library
    case IDC_EC_INPT4: //< backup
      has_changed = true;
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

      this->getItemText(IDC_EC_INPT1, item_str);
      if(!item_str.empty()) {

        this->getItemText(IDC_EC_INPT2, item_str);
        if(!item_str.empty()) {

          if(this->msgItem(IDC_BC_CHK01, BM_GETCHECK)) {
            this->getItemText(IDC_EC_INPT3, item_str);
            if(item_str.empty()) allow = false;
          }

          if(this->msgItem(IDC_BC_CHK02, BM_GETCHECK)) {
            this->getItemText(IDC_EC_INPT4, item_str);
            if(item_str.empty()) allow = false;
          }

        } else {
          allow = false;
        }

      } else {
        allow = false;
      }

      this->enableItem(IDC_BC_OK, allow);
    }
  }

  return false;
}
