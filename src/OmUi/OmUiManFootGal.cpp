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

#include "OmBaseApp.h"

#include "OmModMan.h"
#include "OmUiMan.h"

#include "OmUtilWin.h"

#include "OmUiPictView.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiManFootGal.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManFootGal::OmUiManFootGal(HINSTANCE hins) : OmDialog(hins),
  _UiMan(nullptr)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiManFootGal::~OmUiManFootGal()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiManFootGal::id() const
{
  return IDD_MGR_FOOT_GAL;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootGal::freeze(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootGal::freeze (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootGal::safemode(bool enable)
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootGal::safemode (" << (enable ? "enabled" : "disabled") << ")\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootGal::_onLvImgHit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootGal::_onLvImgHit\n";
  #endif

  OmUiPictView* pUiPictView = static_cast<OmUiPictView*>(this->_UiMan->childById(IDD_PICT_VIEW));
  if(pUiPictView->visible()) {
    SetActiveWindow(pUiPictView->hwnd());
  } else {
    pUiPictView->modeless(true);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootGal::_onInit()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootGal::_onInit\n";
  #endif

  // retrieve main dialog
  this->_UiMan = static_cast<OmUiMan*>(this->root());

  DWORD lvStyle;
  // Initialize Packages ListView control
  lvStyle = LVS_EX_DOUBLEBUFFER|LVS_EX_HIDELABELS|LVS_EX_ONECLICKACTIVATE;
  this->msgItem(IDC_LV_IMG, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lvStyle);
  // set explorer theme
  SetWindowTheme(this->getItem(IDC_LV_IMG),L"Explorer",nullptr);

  // set ListView icon cell size
  this->msgItem(IDC_LV_IMG, LVM_SETICONSPACING, 0, MAKELPARAM(135, 135));
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootGal::_onShow()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootGal::_onShow\n";
  #endif

  OmImage image;
  HBITMAP hBm;

  image.loadThumbnail(L"D:\\Atelier\\Code\\Cpp\\OpenModMan\\screenshot.png", OMM_MODPACK_THUMB_SIZE, OMM_SIZE_FIT);
  hBm = image.hbmp();

  HIMAGELIST hImgLs;

  // Get the previous Image List to be destroyed (Small and Normal uses the same)
  hImgLs = reinterpret_cast<HIMAGELIST>(this->msgItem(IDC_LV_IMG, LVM_GETIMAGELIST, LVSIL_NORMAL));
  if(hImgLs) ImageList_Destroy(hImgLs);

  // Create ImageList and fill it with bitmaps
  hImgLs = ImageList_Create(128, 128, ILC_COLOR32, 1, 0);
  for(unsigned i = 0; i < 1; ++i)
    ImageList_Add(hImgLs, hBm, nullptr);

  // Set ImageList to ListView
  this->msgItem(IDC_LV_IMG, LVM_SETIMAGELIST, LVSIL_SMALL, reinterpret_cast<LPARAM>(hImgLs));
  this->msgItem(IDC_LV_IMG, LVM_SETIMAGELIST, LVSIL_NORMAL, reinterpret_cast<LPARAM>(hImgLs));

  // empty list view
  this->msgItem(IDC_LV_IMG, LVM_DELETEALLITEMS);

  LVITEMW lvItem;
  lvItem.iSubItem = 0;
  for(unsigned i = 0; i < 6; ++i) {

    // the first column, package status, here we INSERT the new item
    lvItem.iItem = i;
    lvItem.mask = LVIF_IMAGE;
    lvItem.iImage = 0;
    lvItem.pszText = L"Toto";
    lvItem.iItem = this->msgItem(IDC_LV_IMG, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvItem));
  }

  // we enable the ListView
  this->enableItem(IDC_LV_IMG, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootGal::_onHide()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootGal::_onHide\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootGal::_onResize()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootGal::_onResize\n";
  #endif

  // Package List ListView
  this->_setItemPos(IDC_LV_IMG, 2, 2, this->cliUnitX()-4, this->cliUnitY()-4);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootGal::_onRefresh()
{
  #ifdef DEBUG
  std::cout << "DEBUG => OmUiManFootGal::_onRefresh\n";
  #endif
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiManFootGal::_onQuit()
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiManFootGal::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_NOTIFY) {

    if(LOWORD(wParam) == IDC_LV_IMG) {

      switch(reinterpret_cast<NMHDR*>(lParam)->code)
      {
      case NM_DBLCLK:
        //this->pkgTogg();
        break;

      case NM_RCLICK:
        //this->_onLvPkgRclk();
        break;

      case LVN_ITEMCHANGED:
        this->_onLvImgHit();
        break;
      }
    }

    return false;
  }

  if(uMsg == WM_COMMAND) {

  }

  return false;
}
