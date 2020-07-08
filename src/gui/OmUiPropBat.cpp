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
#include "OmUiPropBat.h"
#include "OmUiPropBatStg.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropBat::OmUiPropBat(HINSTANCE hins) : OmDialogProp(hins),
  _batch(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"Settings", new OmUiPropBatStg(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropBat::~OmUiPropBat()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropBat::id() const
{
  return IDD_PROP_BAT;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropBat::checkChanges()
{
  OmBatch* batch = reinterpret_cast<OmBatch*>(this->_batch);
  OmUiPropBatStg* uiPropBatStg  = reinterpret_cast<OmUiPropBatStg*>(this->childById(IDD_PROP_BAT_STG));

  bool changed = false;

  wchar_t wcbuf[MAX_PATH];

  if(uiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) {  //< parameter for Context title
    GetDlgItemTextW(uiPropBatStg->hwnd(), IDC_EC_INPT1, wcbuf, MAX_PATH);
    if(batch->title() != wcbuf) {
      changed = true;
    } else {
      uiPropBatStg->setChParam(BAT_PROP_STG_TITLE, false);
    }
  }

  // enable Apply button
  if(IsWindowEnabled(GetDlgItem(this->_hwnd, IDC_BC_APPLY)) != changed) {
    EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), changed);
  }

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropBat::applyChanges()
{
  OmBatch* batch = reinterpret_cast<OmBatch*>(this->_batch);
  OmUiPropBatStg* uiPropBatStg  = reinterpret_cast<OmUiPropBatStg*>(this->childById(IDD_PROP_BAT_STG));

  wchar_t title[MAX_PATH];

  // Step 1, verify everything
  if(uiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) { //< parameter for Context title
    GetDlgItemTextW(uiPropBatStg->hwnd(), IDC_EC_INPT1, title, MAX_PATH);
    if(!wcslen(title)) {
      Om_dialogBoxErr(this->_hwnd, L"Invalid Batch title",
                                   L"Please enter a valid title.");
      return false;
    }
    // Check whether name already exists
    OmContext* context = batch->context();
    for(unsigned i = 0; i < context->batchCount(); ++i) {
      if(context->batch(i)->title() == title) {
        Om_dialogBoxErr(this->_hwnd, L"Not unique Batch title",
                                     L"A Batch with the same title already "
                                     L"exists. Please choose another title.");
        return false;
      }
    }
  }

  // Step 2, save changes
  if(uiPropBatStg->hasChParam(BAT_PROP_STG_TITLE)) { //< parameter for Context title
    if(!batch->rename(title)) { //< rename Batch filename
      Om_dialogBoxErr(this->_hwnd, L"Batch rename failed", batch->lastError());
    }
    batch->setTitle(title); //< change Batch title
    // Reset parameter as unmodified
    uiPropBatStg->setChParam(BAT_PROP_STG_TITLE, false);
  }

  // disable Apply button
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_APPLY), false);

  return true;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBat::_onShow()
{
  // Initialize TabControl with pages dialogs
  this->_pagesOnShow(IDC_TC_TABS1);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBat::_onResize()
{
  // TabControl
  this->_setControlPos(IDC_TC_TABS1, 4, 5, this->width()-8, this->height()-28);
  // Resize page dialogs according IDC_TC_TABS1
  this->_pagesOnResize(IDC_TC_TABS1);

  // OK Button
  this->_setControlPos(IDC_BC_OK, this->width()-161, this->height()-19, 50, 14);
  // Cancel Button
  this->_setControlPos(IDC_BC_CANCEL, this->width()-108, this->height()-19, 50, 14);
  // Apply Button
  this->_setControlPos(IDC_BC_APPLY, this->width()-54, this->height()-19, 50, 14);

  // force buttons to redraw to prevent artifacts
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_OK), nullptr, true);
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_CANCEL), nullptr, true);
  InvalidateRect(GetDlgItem(this->_hwnd, IDC_BC_APPLY), nullptr, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBat::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropBat::_onQuit()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropBat::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {
    // handle TabControl page selection change
    this->_pagesOnNotify(IDC_TC_TABS1, wParam, lParam);
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_APPLY:
      if(this->applyChanges()) {
        // refresh all tree from the main dialog
        this->root()->refresh();
      }
      break;

    case IDC_BC_OK:
      if(this->checkChanges()) {
        if(this->applyChanges()) {
          // quit the dialog
          this->quit();
          // refresh all tree from the main dialog
          this->root()->refresh();
        }
      } else {
        // quit the dialog
        this->quit();
      }
      break; // case BTN_OK:

    case IDC_BC_CANCEL:
      this->quit();
      break; // case BTN_CANCEL:
    }
  }

  return false;
}
