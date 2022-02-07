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

#include "Util/OmUtilImg.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
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
  _data(nullptr), _data_size(0), _data_type(0), _thumbnail(nullptr), _valid(false),
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

  // open file for reading
  HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

  if(hFile == INVALID_HANDLE_VALUE) {
    this->_ercode = OMM_IMAGE_ERR_OPEN;
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
    this->_ercode = OMM_IMAGE_ERR_READ;
    delete [] data;
    return false;
  }

  // check for image type
  int type = Om_imageType(data);
  if(type == 0) { //< unknown image format
    this->_ercode = OMM_IMAGE_ERR_TYPE;
    return false;
  }

  // decode image data
  unsigned w, h, c;
  uint8_t* rgb = Om_loadImage(&w, &h, &c, data, size, false);

  if(!rgb) { //< image decoding error
    this->_ercode = OMM_IMAGE_ERR_LOAD;
    return false;
  }

  // create thumbnail
  if(thumb) {
    uint8_t* thn = Om_thumbnailImage(thumb, rgb, w, h, c);
    if(thn) {
      this->_thumbnail = Om_hbitmapImage(thn, thumb, thumb, c);
      Om_free(thn); //< free allocated RGB data
    } else {
      this->_ercode = OMM_IMAGE_ERR_THMB;
      Om_free(rgb); //< free allocated RGB data
      return false;
    }
  }

  // free allocated image
  Om_free(rgb);

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

  // check for image type
  int type = Om_imageType(data);
  if(type == 0) { //< unknown image format
    this->_ercode = OMM_IMAGE_ERR_TYPE;
    return false;
  }

  // decode image data
  unsigned w, h, c;
  uint8_t* rgb = Om_loadImage(&w, &h, &c, data, size, false);

  if(type == -1) { //< image decoding error
    this->_ercode = OMM_IMAGE_ERR_LOAD;
    return false;
  }

  // create thumbnail
  if(thumb) {
    uint8_t* thn = Om_thumbnailImage(thumb, rgb, w, h, c);
    if(thn) {
      this->_thumbnail = Om_hbitmapImage(thn, thumb, thumb, c);
      Om_free(thn); //< free allocated RGB data
    } else {
      this->_ercode = OMM_IMAGE_ERR_THMB;
      Om_free(rgb); //< free allocated RGB data
      return false;
    }
  }

  // free allocated image
  Om_free(rgb);

  // copy data locally
  this->_data = new(std::nothrow) uint8_t[size];
  if(!this->_data) return false;

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
