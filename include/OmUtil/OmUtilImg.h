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
#ifndef OMUTILIMG_H
#define OMUTILIMG_H

#include "OmBase.h"
#include "OmBaseWin.h"

enum OmImgType : unsigned {
  OM_IMAGE_BMP = 1,
  OM_IMAGE_JPG,
  OM_IMAGE_PNG,
  OM_IMAGE_GIF
};

enum OmSizeMode : unsigned {
  OM_SIZE_FIT = 0,
  OM_SIZE_FILL
};

/// \brief Get image type.
///
/// Search within the given data buffer for known image signature. Possible
/// values are the following:
///
/// \c 0 : unknown or invalid image format.
/// \c 1 or \c OM_IMAGE_TYPE_BMP : BMP image.
/// \c 2 or \c OM_IMAGE_TYPE_JPG : JPEG image.
/// \c 3 or \c OM_IMAGE_TYPE_PNG : PNG image.
/// \c 4 or \c OM_IMAGE_TYPE_GIF : GIF image.
///
/// \param[in]  data  : Image data to read, must be at least 8 bytes length.
///
/// \return Image type or 0 if unknown type.
///
int Om_imgGetType(uint8_t* data);

/// \brief Get image type.
///
/// Search within the given data buffer for known image signature. Possible
/// values are the following:
///
/// \c 0 : unknown or invalid image format.
/// \c 1 or \c OM_IMAGE_TYPE_BMP : BMP image.
/// \c 2 or \c OM_IMAGE_TYPE_JPG : JPEG image.
/// \c 3 or \c OM_IMAGE_TYPE_PNG : PNG image.
/// \c 4 or \c OM_IMAGE_TYPE_GIF : GIF image.
///
/// \param[in]  file  : File pointer to read data.
///
/// \return Image type, 0 if unknown type or -1 if read error occurred.
///
int Om_imgGetType(FILE* file);

/// \brief Load image.
///
/// Load image data from image file. Supported format are Bmp, Jpeg, Png and Gif.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[in]  in_path : Input image file path to read data.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to RGB(A) image data or nullptr if failed.
///
uint8_t* Om_imgLoadFile(unsigned* out_w, unsigned* out_h, const OmWString& in_path, bool flip_y = false);

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
uint8_t* Om_imgLoadData(unsigned* out_w, unsigned* out_h, const uint8_t* in_data, size_t in_size, bool flip_y = false);

/// \brief Load HBITMAP data.
///
/// Load HBITMAP data.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_hbmp : Input image data to decode.
///
/// \return Pointer to RGBA image data or nullptr if failed.
///
uint8_t* Om_imgLoadHBmp(unsigned* dst_w, unsigned* dst_h, HBITMAP in_hbmp);

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
bool Om_imgSaveBmp(const OmWString& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

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
bool Om_imgSaveJpg(const OmWString& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level = 8);

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
bool Om_imgSavePng(const OmWString& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level = 4);

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
bool Om_imgSaveGif(const OmWString& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

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
uint8_t* Om_imgEncodeBmp(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

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
uint8_t* Om_imgEncodeJpg(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level = 8);

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
uint8_t* Om_imgEncodePng(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level = 4);

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
uint8_t* Om_imgEncodeGif(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c);

/// \brief Encode DDB data.
///
/// Encode Device Dependant Bitmap (HBITMAP) version of the given image data.
///
/// \param[in]  src_pix    : Source image RGB or RGBA pixel data.
/// \param[in]  src_w      : Source image width.
/// \param[in]  src_h      : Source image height.
/// \param[in]  src_c      : Source image color component count, either 3 or 4.
///
/// \return New HBITMAP or null if error.
///
HBITMAP Om_imgEncodeHbmp(const uint8_t* src_pix, unsigned src_w, unsigned src_h, unsigned src_c);

/// \brief Copy and resample.
///
/// Copy and resamples the specified rectangle of source image to destination
/// using best filtering method according source and destination resolutions.
///
/// Destination buffer must be allocated
///
/// \param[out] dst_pix   : Destination pixel buffer that receive result.
/// \param[in]  dst_w     : Destination width in pixel.
/// \param[in]  dst_h     : Destination height in pixel.
/// \param[in]  src_pix   : Source image RGBA pixel data.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
/// \param[in]  rec_x     : Rectangle top-left corner x coordinate in source.
/// \param[in]  rec_y     : Rectangle top-left corner y coordinate in source
/// \param[in]  rec_w     : Rectangle width
/// \param[in]  rec_h     : Rectangle height.
///
void Om_imgCopyResample(uint8_t* dst_buf, unsigned dst_w, unsigned dst_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, unsigned rec_x, unsigned rec_y, unsigned rec_w, unsigned rec_h);

/// \brief Resample image
///
/// Resamples source image to the specified destination buffer using
/// bilinear interpolation filter.
///
/// \param[out] dst_pix   : Destination pixel buffer that receive result.
/// \param[in]  dst_w     : Destination width in pixel.
/// \param[in]  dst_h     : Destination height in pixel.
/// \param[in]  src_pix   : Source image RGBA pixel data.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
///
void Om_imgResample(uint8_t* dst_buf, unsigned dst_w, unsigned dst_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h);

/// \brief Draw image in destination canvas.
///
/// Draws and resamples the specified rectangle of source image to destination
/// using best filtering method according source and destination resolutions.
///
/// \param[out] cv_pix    : Canvas pixel buffer that receive result.
/// \param[in]  cv_w      : Canvas width in pixel.
/// \param[in]  cv_h      : Canvas height in pixel.
/// \param[in]  dst_f     : Destination color format.
/// \param[in]  src_pix   : Source image RGBA pixel data.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
/// \param[in]  bck       : Background color
///
void Om_imgDrawCanvas(uint8_t* cv_pix, unsigned cv_w, unsigned cv_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, uint32_t bck);


/// \brief Create image thumbnail.
///
/// Create thumbnail version of the given image data.
///
/// \param[in]  span    : Thumbnail target span.
/// \param[in]  mode    : Thumbnail resize mode.
/// \param[in]  src_pix : Source image RGBA pixel data.
/// \param[in]  src_w   : Source image width.
/// \param[in]  src_h   : Source image height.
///
/// \return New pointer to resized image data or null if error.
///
uint8_t* Om_imgMakeThumb(unsigned span, OmSizeMode mode, const uint8_t* src_pix, unsigned src_w, unsigned src_h);

/// \brief Swap Red and Blue components.
///
/// Swap Rend and Blue components of the given data buffer.
///
/// \param[in]  src_pix : Source image RGB or RGBA pixel data.
/// \param[in]  src_w   : Source image width.
/// \param[in]  src_h   : Source image height.
/// \param[in]  src_c   : Source image color component count, either 3 or 4.
///
/// \return New pointer to resized image data or null if error.
///
void Om_imgRbSwap(uint8_t* src_pix, unsigned src_w, unsigned src_h, unsigned src_c);

#endif // OMUTILIMG_H
