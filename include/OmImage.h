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
    /// \param[in]  path  : Image file path
    /// \param[in]  thumb : Size of the thumbnail to generate or 0 to ignore.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool open(const wstring& path, unsigned thumb = 0);

    /// \brief Load image.
    ///
    /// Load image from data in memory.
    ///
    /// \param[in]  data  : Image data to read.
    /// \param[in]  size  : Image data size in bytes.
    /// \param[in]  thumb : Size of the thumbnail to generate or 0 to ignore.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool open(uint8_t* data, size_t size, unsigned thumb = 0);

    /// \brief Check validity
    ///
    /// Check whether this instance is valid.
    ///
    /// \return True if this instance is valid, false otherwise
    ///
    bool valid() const {
      return _valid;
    }

    /// \brief Get image raw data.
    ///
    /// Returns stored image raw data.
    ///
    /// \return Pointer to raw data.
    ///
    const uint8_t* data() const {
      return _data;
    }

    /// \brief Get image raw data size.
    ///
    /// Returns stored image raw data size in bytes.
    ///
    /// \return Raw data size in bytes.
    ///
    size_t data_size() const {
      return _data_size;
    }

    /// \brief Get raw data image type.
    ///
    /// Returns stored image raw data image format.
    ///
    /// possibles values are the following:
    /// \c 0 : unknown or invalid type.
    /// \c OMM_IMAGE_TYPE_BMP (1) : BMP image.
    /// \c OMM_IMAGE_TYPE_JPG (2) : JPEG image.
    /// \c OMM_IMAGE_TYPE_PNG (3) : PNG image.
    /// \c OMM_IMAGE_TYPE_GIF (4) : GIF image.
    ///
    /// \return Raw data image type.
    ///
    unsigned data_type() const {
      return _data_type;
    }

    /// \brief Create thumbnail.
    ///
    /// Convert this image to its square thumbnail of the specified size.
    ///
    /// \param[in]  size   Thumbnail size.
    ///
    /// \return true if operation succeed, false otherwise.
    ///
    HBITMAP thumbnail() const {
      return _thumbnail;
    }

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

    uint8_t*            _data;        //< Image data

    size_t              _data_size;   //< Image data size

    unsigned            _data_type;   //< Image data type

    HBITMAP             _thumbnail;   //< Image thumbnail

    bool                _valid;       //< Valid image

    int                 _ercode;      //< Error code
};

#endif // OMIMAGE_H
