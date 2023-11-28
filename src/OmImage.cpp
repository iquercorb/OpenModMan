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
#include "OmUtilImg.h"
#include <ctime>
#include <iostream>

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmImage.h"


#define OM_IMAGE_ERR_OPEN    -1
#define OM_IMAGE_ERR_READ    -2
#define OM_IMAGE_ERR_LOAD    -3
#define OM_IMAGE_ERR_TYPE    -4
#define OM_IMAGE_ERR_THMB    -5

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmImage::OmImage() :
  _path(), _data(nullptr), _width(0), _height(0),
  _hbmp(nullptr), _valid(false), _ercode(0)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmImage::~OmImage()
{
  this->clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::load(const OmWString& path)
{
  // clear all previous data
  this->clear();

  // open file for reading
  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE) {
    this->_ercode = OM_IMAGE_ERR_OPEN;
    return false;
  }

  size_t size = GetFileSize(hFile, nullptr);

  // allocate buffer and read
  uint8_t* data = new(std::nothrow) uint8_t[size];
  if(!data) return false;

  // read full data at once
  DWORD rb;
  bool result = ReadFile(hFile, data, size, &rb, nullptr);

  // close file
  CloseHandle(hFile);

  if(!result) {
    this->_ercode = OM_IMAGE_ERR_READ;
    delete [] data;
    return false;
  }

  // check for image type
  int type = Om_imgGetType(data);
  if(type == 0) { //< unknown image format
    this->_ercode = OM_IMAGE_ERR_TYPE;
    return false;
  }

  unsigned w, h;

  // decode image data
  this->_data = Om_imgLoadData(&w, &h, data, size, false);

  if(!this->_data) {
    this->_ercode = OM_IMAGE_ERR_LOAD;
    return false;
  }

  this->_width = w;
  this->_height = h;

  this->_path = path;

  // create HBITMAP from data
  this->_hbmp = Om_imgEncodeHbmp(this->_data, w, h, 4);

  // image is loaded and valid
  this->_valid = true;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::load(uint8_t* data, size_t size)
{
  // clear all previous data
  this->clear();

  // check for image type
  int type = Om_imgGetType(data);
  if(type == 0) { //< unknown image format
    this->_ercode = OM_IMAGE_ERR_TYPE;
    return false;
  }

  unsigned w, h;

  this->_data = Om_imgLoadData(&w, &h, data, size, false);

  if(!this->_data) {
    this->_ercode = OM_IMAGE_ERR_LOAD;
    return false;
  }

  this->_width = w;
  this->_height = h;

  // create HBITMAP from data
  this->_hbmp = Om_imgEncodeHbmp(this->_data, w, h, 4);

  // image is loaded and valid
  this->_valid = true;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::loadThumbnail(const OmWString& path, unsigned span, OmSizeMode mode)
{
  // clear all previous data
  this->clear();

  // open file for reading
  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE) {
    this->_ercode = OM_IMAGE_ERR_OPEN;
    return false;
  }

  size_t size = GetFileSize(hFile, nullptr);

  // allocate buffer and read
  uint8_t* data = new(std::nothrow) uint8_t[size];
  if(!data) return false;

  // read full data at once
  DWORD rb;
  bool result = ReadFile(hFile, data, size, &rb, nullptr);

  // close file
  CloseHandle(hFile);

  if(!result) {
    this->_ercode = OM_IMAGE_ERR_READ;
    delete [] data;
    return false;
  }

  // check for image type
  int type = Om_imgGetType(data);
  if(type == 0) { //< unknown image format
    this->_ercode = OM_IMAGE_ERR_TYPE;
    return false;
  }

  unsigned w, h;
  uint8_t* rgb = Om_imgLoadData(&w, &h, data, size, false);

  if(!rgb) {
    this->_ercode = OM_IMAGE_ERR_LOAD;
    return false;
  }

  this->_data = Om_imgMakeThumb(span, mode, rgb, w, h);
  Om_free(rgb);

  if(!this->_data) {
    this->_ercode = OM_IMAGE_ERR_LOAD;
    return false;
  }

  this->_width = span;
  this->_height = span;

  this->_path = path;

  // create HBITMAP from data
  this->_hbmp = Om_imgEncodeHbmp(this->_data, span, span, 4);

  // image is loaded and valid
  this->_valid = true;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::loadThumbnail(uint8_t* data, size_t size, unsigned span, OmSizeMode mode)
{
  // clear all previous data
  this->clear();

  // check for image type
  int type = Om_imgGetType(data);
  if(type == 0) { //< unknown image format
    this->_ercode = OM_IMAGE_ERR_TYPE;
    return false;
  }

  unsigned w, h;
  uint8_t* rgb = Om_imgLoadData(&w, &h, data, size, false);

  if(!rgb) {
    this->_ercode = OM_IMAGE_ERR_LOAD;
    return false;
  }

  this->_data = Om_imgMakeThumb(span, mode, rgb, w, h);
  Om_free(rgb);

  if(!this->_data) {
    this->_ercode = OM_IMAGE_ERR_LOAD;
    return false;
  }

  this->_width = span;
  this->_height = span;

  // create HBITMAP from data
  this->_hbmp = Om_imgEncodeHbmp(this->_data, span, span, 4);

  // image is loaded and valid
  this->_valid = true;

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmImage::clear()
{
  if(this->_data) {
    Om_free(this->_data);
    this->_data = nullptr;
  }

  if(this->_hbmp) {
    DeleteObject(this->_hbmp);
    this->_hbmp = nullptr;
  }

  this->_path.clear();
  this->_width = 0;
  this->_height = 0;
  this->_valid = false;

  this->_ercode = 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString OmImage::lastErrorStr() const
{
  OmWString err;

  switch(this->_ercode)
  {
    case OM_IMAGE_ERR_OPEN: err = L"File open error"; break;
    case OM_IMAGE_ERR_READ: err = L"File read error"; break;
    case OM_IMAGE_ERR_TYPE: err = L"Unsupported image format"; break;
    case OM_IMAGE_ERR_LOAD: err = L"Error decoding image data"; break;
    case OM_IMAGE_ERR_THMB: err = L"Thumbnail creation error"; break;
    default: err = L"No error"; break;
  }

  return err;
}
