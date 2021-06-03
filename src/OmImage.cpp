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
#include "OmImage.h"

#define OMM_IMAGE_ERR_OPEN    -1
#define OMM_IMAGE_ERR_READ    -2
#define OMM_IMAGE_ERR_LOAD    -3
#define OMM_IMAGE_ERR_TYPE    -4
#define OMM_IMAGE_ERR_THMB    -5


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmImage::OmImage() :
  _data(nullptr),
  _data_size(0),
  _data_type(0),
  _thumbnail(nullptr),
  _valid(false),
  _ercode(0)
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
bool OmImage::open(const wstring& path, unsigned thumb)
{
  // clear all previous data
  this->clear();

  // open file
  FILE* fp;

  //if((fp = fopen(Om_toUtf8(path).c_str(), "rb")) == nullptr) {
  if((fp = _wfopen(path.c_str(), L"rb")) == nullptr) {
    this->_ercode = OMM_IMAGE_ERR_OPEN;
    return false;
  }

  // read the whole data at once, we will store it
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);

  // allocate buffer and read
  uint8_t* data;
  try {
    data = new uint8_t[size];
  } catch(const std::bad_alloc&) {
    return false;
  }

  if(fread(data, 1, size, fp) != size) {
    this->_ercode = OMM_IMAGE_ERR_READ;
    fclose(fp);
    delete [] data;
    return false;
  }

  fclose(fp);

  // decode image data
  uint8_t* rgb = nullptr;
  unsigned w, h, c;

  // check for known image file signatures
  int type = Om_loadImage(&rgb, &w, &h, &c, data, size, false);

  // check for image loading error
  if(type == 0) { //< unknown image format
    this->_ercode = OMM_IMAGE_ERR_TYPE;
    return false;
  }

  if(type == -1) { //< image decoding error
    this->_ercode = OMM_IMAGE_ERR_LOAD;
    return false;
  }

  // create thumbnail
  if(thumb) {
    uint8_t* thb_rgb = Om_thumbnailImage(thumb, rgb, w, h, c);
    if(thb_rgb) {
      this->_thumbnail = Om_hbitmapImage(thb_rgb, thumb, thumb, c);
      delete [] thb_rgb;
    } else {
      this->_ercode = OMM_IMAGE_ERR_THMB;
      delete [] rgb;
      return false;
    }
  }

  delete [] rgb;

  this->_data = data;
  this->_data_size = size;
  this->_data_type = type;
  this->_valid = true;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::open(uint8_t* data, size_t size, unsigned thumb)
{
  // clear all previous data
  this->clear();

  // decode image data
  uint8_t* rgb = nullptr;
  unsigned w, h, c;

  // check for known image file signatures
  int type = Om_loadImage(&rgb, &w, &h, &c, data, size, false);

  // check for image loading error
  if(type == 0) { //< unknown image format
    this->_ercode = OMM_IMAGE_ERR_TYPE;
    return false;
  }

  if(type == -1) { //< image decoding error
    this->_ercode = OMM_IMAGE_ERR_LOAD;
    return false;
  }

  // create thumbnail
  if(thumb) {
    uint8_t* thb_rgb = Om_thumbnailImage(thumb, rgb, w, h, c);
    if(thb_rgb) {
      this->_thumbnail = Om_hbitmapImage(thb_rgb, thumb, thumb, c);
      delete [] thb_rgb;
    } else {
      this->_ercode = OMM_IMAGE_ERR_THMB;
      delete [] rgb;
      return false;
    }
  }

  delete [] rgb;

  // copy data locally
  try {
    this->_data = new uint8_t[size];
  } catch(const std::bad_alloc&) {
    return false;
  }
  memcpy(this->_data, data, size);

  this->_data_size = size;
  this->_data_type = type;
  this->_valid = true;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmImage::clear()
{
  if(this->_data)
    delete [] this->_data;

  if(this->_thumbnail)
    DeleteObject(this->_thumbnail);

  this->_data = nullptr;
  this->_data_size = 0;
  this->_data_type = 0;
  this->_thumbnail = nullptr;

  this->_valid = false;

  this->_ercode = 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring OmImage::lastErrorStr() const
{
  wstring err;

  switch(this->_ercode)
  {
    case OMM_IMAGE_ERR_OPEN: err = L"File open error"; break;
    case OMM_IMAGE_ERR_READ: err = L"File read error"; break;
    case OMM_IMAGE_ERR_TYPE: err = L"Unsupported image format"; break;
    case OMM_IMAGE_ERR_LOAD: err = L"Error decoding image data"; break;
    case OMM_IMAGE_ERR_THMB: err = L"Thumbnail creation error"; break;
    default: err = L"No error"; break;
  }

  return err;
}
