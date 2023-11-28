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

#include "OmImage.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPictView.h"

OmUiPictView::OmUiPictView(HINSTANCE hins) : OmDialog(hins),
  _image(nullptr), _cvData(nullptr), _cvSize(0)
{

}

OmUiPictView::~OmUiPictView()
{
  HFONT hFt = reinterpret_cast<HFONT>(this->msgItem(IDC_SC_HEAD, WM_GETFONT));
  DeleteObject(hFt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPictView::id() const
{
  return IDD_PICT_VIEW;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPictView::_onInit()
{
  this->_image = new OmImage();
  this->_image->load(L"D:\\Atelier\\Code\\Cpp\\OpenModMan\\screenshot.png");

  this->_cvSize = 2048*2048*4;
  this->_cvData = reinterpret_cast<uint8_t*>(Om_alloc(this->_cvSize));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPictView::_onResize()
{
  InvalidateRect(this->_hwnd, nullptr, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPictView::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {


  }

  if(uMsg == WM_CTLCOLORDLG ) {
    return (INT_PTR)GetStockObject(BLACK_BRUSH);
  }

  if(uMsg == WM_PAINT) {

    HDC hDc;
    PAINTSTRUCT ps;
    BITMAPINFO bmi;
    unsigned x, y, w, h;

    hDc = BeginPaint(this->_hwnd, &ps);

    if(this->_image) {

      bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biXPelsPerMeter = 0;
      bmi.bmiHeader.biYPelsPerMeter = 0;
      bmi.bmiHeader.biClrUsed = 0;
      bmi.bmiHeader.biClrImportant = 0;
      bmi.bmiHeader.biPlanes = 1;
      bmi.bmiHeader.biBitCount = 32;
      bmi.bmiHeader.biCompression = BI_RGB;

      if(ps.rcPaint.right < this->_image->width() || ps.rcPaint.bottom < this->_image->height()) {

        if((static_cast<float>(this->_image->width()) / this->_image->height()) > (static_cast<float>(ps.rcPaint.right) / ps.rcPaint.bottom)) {

          w = ps.rcPaint.right;
          h = ps.rcPaint.right * (static_cast<float>(this->_image->height()) / this->_image->width());
          x = 0;
          y = 0.5f * (ps.rcPaint.bottom - h);

        } else {

          w = ps.rcPaint.bottom * (static_cast<float>(this->_image->width()) / this->_image->height());
          h = ps.rcPaint.bottom;
          x = 0.5f * (ps.rcPaint.right - w);
          y = 0;
        }

        Om_imgResample(this->_cvData, w, h, this->_image->data(), this->_image->width(), this->_image->height());

      } else {

        w = this->_image->width();
        h = this->_image->height();

        x = 0.5f * (ps.rcPaint.right - w);
        y = 0.5f * (ps.rcPaint.bottom - h);

        memcpy(this->_cvData, this->_image->data(), w * h * 4);
      }

      Om_imgRbSwap(this->_cvData, w, h, 4);

      bmi.bmiHeader.biWidth = w;
      bmi.bmiHeader.biHeight = -h;
      bmi.bmiHeader.biSizeImage = w * h * 4;

      SetDIBitsToDevice(hDc, x, y, w, h, 0, 0, 0, h, this->_cvData, &bmi, DIB_RGB_COLORS);
    }

    EndPaint(this->_hwnd, &ps);

    return 1;
  }

  return false;
}

