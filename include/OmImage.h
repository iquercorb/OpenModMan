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

#include "OmBase.h"
#include "OmBaseWin.h"

#include "OmUtilImg.h"

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
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool load(const OmWString& path);

    /// \brief Load image.
    ///
    /// Load image from data in memory.
    ///
    /// \param[in]  data  : Image data to read.
    /// \param[in]  size  : Image data size in bytes.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool load(uint8_t* data, size_t size);

    /// \brief Load image to thumbnail.
    ///
    /// Load image from file then create and store its
    /// thumbnail version.
    ///
    /// \param[in]  path  : Image file path
    /// \param[in]  span  : Span of thumbnail canvas to create.
    /// \param[in]  mode  : Thumbnail resize mode.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool loadThumbnail(const OmWString& path, unsigned span, OmSizeMode mode);

    /// \brief Load image to thumbnail.
    ///
    /// Load image from data in memory then create and store its
    /// thumbnail version.
    ///
    /// \param[in]  data  : Image data to read.
    /// \param[in]  size  : Image data size in bytes.
    /// \param[in]  span  : Span of thumbnail canvas to create.
    /// \param[in]  mode  : Thumbnail resize mode.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool loadThumbnail(uint8_t* data, size_t size, unsigned span, OmSizeMode mode);

    /// \brief Check validity
    ///
    /// Check whether this instance is valid.
    ///
    /// \return True if this instance is valid, false otherwise
    ///
    bool valid() const {
      return _valid;
    }

    /// \brief Source image path.
    ///
    /// Get source image file path. If image was loaded from data in
    /// memory this value is empty.
    ///
    /// \return Pointer to pixel data.
    ///
    const OmWString& path() const {
      return _path;
    }

    /// \brief Image pixel data.
    ///
    /// Get pointer to stored image pixel data.
    ///
    /// \return Pointer to pixel data.
    ///
    const uint8_t* data() const {
      return _data;
    }

    /// \brief Image width
    ///
    /// Get stored image width in pixel.
    ///
    /// \return Image width in pixels
    ///
    unsigned width() const {
      return _width;
    }

    /// \brief Image height
    ///
    /// Get stored image height in pixel.
    ///
    /// \return Image height in pixels
    ///
    unsigned height() const {
      return _height;
    }

    /// \brief Get thumbnail.
    ///
    /// Get the previously generated thumbnail bitmap.
    ///
    /// \return Last generated thumbnail bitmap or null if none was generated.
    ///
    HBITMAP hbmp() const {
      return _hbmp;
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
    OmWString lastErrorStr() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmWString           _path;        //< Source image path if exists

    uint8_t*            _data;        //< Image pixel data

    unsigned            _width;       //< Image width

    unsigned            _height;      //< Image height

    HBITMAP             _hbmp;        //< Corresponding HBITMAP

    bool                _valid;       //< Valid image

    int                 _ercode;      //< Error code
};

#endif // OMIMAGE_H
