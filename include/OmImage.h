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
#ifndef OMIMAGE_H
#define OMIMAGE_H

#include "OmGlobal.h"

enum OmImageType {
  OMM_IMAGE_UNKNOW  = 0,
  OMM_IMAGE_BMP     = 1,
  OMM_IMAGE_JPG     = 2,
  OMM_IMAGE_PNG     = 3,
  OMM_IMAGE_GIF     = 4
};

/// \brief Image file interface.
///
/// Object to provide interface for an image file.
///
class OmImage
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor
    ///
    /// Default object constructor
    ///
    OmImage();

    /// \brief Destructor
    ///
    /// Default object destructor
    ///
    ~OmImage();

    /// \brief Load image.
    ///
    /// Load image from file.
    ///
    /// \param[in]  path  Image file path
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool load(const wstring& path);

    /// \brief Load image.
    ///
    /// Load image from data in memory.
    ///
    /// \param[in]  data  Image data to read.
    /// \param[in]  size  Image data size in bytes.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool load(uint8_t* data, size_t size);

    /// \brief Save image.
    ///
    /// Save image to specified file.
    ///
    /// \param[in]  path    File path to save image as.
    /// \param[in]  type    Image format to save.
    /// \param[in]  level   Optional 0-10 JPEG quality or PNG compression.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool save(const wstring& path, OmImageType type, int level = 8);

    /// \brief Save image in memory.
    ///
    /// Save image to memory.
    ///
    /// \param[in]  dest    Pointer to pointer to be allocated.
    /// \param[in]  size    Image data allocated size.
    /// \param[in]  type    Image format to encode.
    /// \param[in]  level   Optional 0 to 10 quality level for JPEG.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool save(uint8_t** dest, size_t* size, OmImageType type, int level = 8);

    /// \brief Get image data.
    ///
    /// Returns image raw pixels data.
    ///
    /// \return Pointer to raw pixels data.
    ///
    uint8_t* data() {
      return _data;
    }

    /// \brief Get image width.
    ///
    /// Returns image width in pixels
    ///
    /// \return Image width in pixels
    ///
    unsigned width() const {
      return _width;
    }

    /// \brief Get image height.
    ///
    /// Returns image height in pixels
    ///
    /// \return Image height in pixels
    ///
    unsigned height() const {
      return _height;
    }

    /// \brief Get image depth.
    ///
    /// Returns image bits per pixels
    ///
    /// \return Image bits per pixels
    ///
    unsigned depth() const {
      return _depth;
    }

    /// \brief Get image as BITMAP.
    ///
    /// Returns WinAPI HBITMAP version of pixels data.
    ///
    /// \return HBITMAP object.
    ///
    HBITMAP asBITMAP() const;

    /// \brief Resize bitmap.
    ///
    /// Resize image to fit the specified new width and height.
    ///
    /// \param[in]  width   Image new width.
    /// \param[in]  height  Image new height.
    ///
    /// \return true if operation succeed, false otherwise.
    ///
    bool resize(unsigned width, unsigned height);

    /// \brief Crop bitmap.
    ///
    /// Crop image to the specified rectangle
    ///
    /// \param[in]  x       Crop rectangle upper-left corner x coordinate.
    /// \param[in]  y       Crop rectangle upper-left corner y coordinate.
    /// \param[in]  width   Crop rectangle width.
    /// \param[in]  height  Crop rectangle height.
    ///
    /// \return true if operation succeed, false otherwise.
    ///
    bool crop(unsigned x, unsigned y, unsigned width, unsigned height);

    /// \brief Create thumbnail.
    ///
    /// Convert this image to its square thumbnail of the specified size.
    ///
    /// \param[in]  size   Thumbnail size.
    ///
    /// \return true if operation succeed, false otherwise.
    ///
    bool thumbnail(unsigned size);

    /// \brief Clear instance.
    ///
    /// Delete all image data and reset instance.
    ///
    void clear();

    /// \brief Get last error string.
    ///
    /// Returns the string corresponding to the last XML parsing error code.
    ///
    /// \return XML parsing error as string.
    ///
    wstring lastErrorStr() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    int                 _status;      //< Image status

    uint8_t*            _data;        //< image pixel data

    unsigned            _width;       //< image width in pixels

    unsigned            _height;      //< image height in pixels

    unsigned            _depth;       //< image pixel depth
};

#endif // OMIMAGE_H
