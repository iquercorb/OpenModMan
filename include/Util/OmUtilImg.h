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
#ifndef OMUTILIMG_H_INCLUDED
#define OMUTILIMG_H_INCLUDED

#include "OmBase.h"
#include "OmBaseWin.h"

#define OMM_IMAGE_TYPE_BMP        1
#define OMM_IMAGE_TYPE_JPG        2
#define OMM_IMAGE_TYPE_PNG        3
#define OMM_IMAGE_TYPE_GIF        4

/// \brief Get image type.
///
/// Search within the given data buffer for known image signature. Possible
/// values are the following:
///
/// \c 0 : unknown or invalid image format.
/// \c 1 or \c OMM_IMAGE_TYPE_BMP : BMP image.
/// \c 2 or \c OMM_IMAGE_TYPE_JPG : JPEG image.
/// \c 3 or \c OMM_IMAGE_TYPE_PNG : PNG image.
/// \c 4 or \c OMM_IMAGE_TYPE_GIF : GIF image.
///
/// \param[in]  data  : Image data to read, must be at least 8 bytes length.
///
/// \return Image type or 0 if unknown type.
///
int Om_imageType(uint8_t* data);

/// \brief Get image type.
///
/// Search within the given data buffer for known image signature. Possible
/// values are the following:
///
/// \c 0 : unknown or invalid image format.
/// \c 1 or \c OMM_IMAGE_TYPE_BMP : BMP image.
/// \c 2 or \c OMM_IMAGE_TYPE_JPG : JPEG image.
/// \c 3 or \c OMM_IMAGE_TYPE_PNG : PNG image.
/// \c 4 or \c OMM_IMAGE_TYPE_GIF : GIF image.
///
/// \param[in]  file  : File pointer to read data.
///
/// \return Image type, 0 if unknown type or -1 if read error occurred.
///
int Om_imageType(FILE* file);

/// \brief Load image.
///
/// Load image data from image file. Supported format are Bmp, Jpeg, Png and Gif.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_path : Input image file path to read data.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to RGB(A) image data or nullptr if failed.
///
uint8_t* Om_loadImage(unsigned* out_w, unsigned* out_h, unsigned* out_c, const wstring& in_path, bool flip_y = false);

/// \brief Load image.
///
/// Load image data from buffer in memory. Supported format are Bmp, Jpeg, Png and Gif.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_data : Input image data to decode.
/// \param[in]  in_size : Input image data size in bytes.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to RGB(A) image data or nullptr if failed.
///
uint8_t* Om_loadImage(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, size_t in_size, bool flip_y = false);

/// \brief Save image as BMP.
///
/// Save given image data as BMP file.
///
/// \param[out] out_path  : File path to save.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return True if operation succeed, false otherwise
///
bool Om_saveBmp(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

/// \brief Save image as JPEG.
///
/// Save given image data as JPEG file.
///
/// \param[out] out_path  : File path to save.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
/// \param[in]  level     : JPEG compression quality 0 to 100.
///
/// \return True if operation succeed, false otherwise
///
bool Om_saveJpg(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level = 8);

/// \brief Save image as PNG.
///
/// Save given image data as PNG file.
///
/// \param[out] out_path  : File path to save.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
/// \param[in]  level     : PNG compression level 0 to 9.
///
/// \return True if operation succeed, false otherwise
///
bool Om_savePng(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level = 9);

/// \brief Save image as GIF.
///
/// Save given image data as GIF file.
///
/// \param[out] out_path  : File path to save.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return True if operation succeed, false otherwise
///
bool Om_saveGif(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

/// \brief Encode BMP data.
///
/// Encode BMP data to buffer in memory.
///
/// \param[out] out_size  : Output BMP data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return Pointer to encoded BMP image data or nullptr if failed.
///
uint8_t* Om_encodeBmp(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

/// \brief Encode JPEG data.
///
/// Encode JPEG data to buffer in memory.
///
/// \param[out] out_data  : Output JPEG data, pointer to pointer to be allocated.
/// \param[out] out_size  : Output JPEG data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
/// \param[in]  level     : JPEG compression quality 0 to 100.
///
/// \return Pointer to encoded JPG image data or nullptr if failed.
///
uint8_t* Om_encodeJpg(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level = 8);

/// \brief Encode PNG data.
///
/// Encode PNG data to buffer in memory.
///
/// \param[out] out_data  : Output PNG data, pointer to pointer to be allocated.
/// \param[out] out_size  : Output PNG data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
/// \param[in]  level     : PNG compression level 0 to 9.
///
/// \return Pointer to encoded PNG image data or nullptr if failed.
///
uint8_t* Om_encodePng(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level = 9);

/// \brief Encode GIF data.
///
/// Encode GIF data to buffer in memory.
///
/// \param[out] out_data  : Output GIF data, pointer to pointer to be allocated.
/// \param[out] out_size  : Output GIF data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return Pointer to encoded GIF image data or nullptr if failed.
///
uint8_t* Om_encodeGif(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

/// \brief Resize image.
///
/// Resize given image data to the specified width and heigth.
///
/// \param[out] w         : Desired image width.
/// \param[out] h         : Desired image height.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return New pointer to resized image data or null if error.
///
uint8_t* Om_resizeImage(unsigned w, unsigned h, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

/// \brief Crop image.
///
/// Crop given image data according the specified rectangle coordinates.
///
/// \param[out] x         : Crop rectangle top-left corner horizontal position in image.
/// \param[out] y         : Crop rectangle top-left corner vertical position in image.
/// \param[out] w         : Crop rectangle width.
/// \param[out] h         : Crop rectangle height.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return New pointer to resized image data or null if error.
///
uint8_t* Om_cropImage(unsigned x, unsigned y, unsigned w, unsigned h, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

/// \brief Create image thumbnail.
///
/// Create thumbnail version of the given image data.
///
/// \param[out] size      : Thumbnail size.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return New pointer to resized image data or null if error.
///
uint8_t* Om_thumbnailImage(unsigned size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

/// \brief Create HBITMAP image.
///
/// Create HBITMAP version of the given image data.
///
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return New HBITMAP or null if error.
///
HBITMAP Om_hbitmapImage(const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

#endif // OMUTILIMG_H_INCLUDED
