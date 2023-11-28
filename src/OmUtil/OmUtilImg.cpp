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
#include "OmBase.h"           //< string, vector, Om_alloc, OM_MAX_PATH, etc.
#include <algorithm>          //< std::sort
#include <cmath>              //< modf, floor, etc.

#ifdef DEBUG
#include <ctime>
#endif // DEBUG

#include "OmBaseWin.h"        //< WinAPI

#include "OmUtilImg.h"        //< OM_IMAGE_TYPE_*

#include "jpeg/jpeglib.h"
#include "png/png.h"
#include "gif/gif_lib.h"
//#include "3rdP/gif/quantize.c"

/// \brief Swap bytes
///
/// Swap bytes order in 32 bits value, to convert endianes.
///
/// \param[in]  num : input 32 value to swap bytes.
///
/// \return Swapped bytes version of the input value
///
inline static uint32_t __bytes_swap(uint32_t num)
{
  return  ((num >> 24) & 0xff) |
          ((num <<  8) & 0xff0000) |
          ((num >>  8) & 0xff00) |
          ((num << 24) & 0xff000000);
}

/// \brief Swap Blue and Red component
///
/// Swaps Blue and Rend component of 24 bits pixel data
///
/// \param[in]  data  : 24 Bits pixel data buffer to swap
/// \param[in]  size  : Size of data in bytes
///
inline static void __in_place_rb_swap_24(uint8_t* data, size_t size)
{
  for(size_t i = 0; i < size; i += 3)
    data[i] ^= data[i+2] ^= data[i] ^= data[i+2];
}

/// \brief Swap Blue and Red component
///
/// Swaps Blue and Rend component of 32 bits pixel data
///
/// \param[in]  data  : 32 Bits pixel data buffer to swap
/// \param[in]  size  : Size of data in bytes
///
inline static void __in_place_rb_swap_32(uint8_t* data, size_t size)
{
  for(size_t i = 0; i < size; i += 4)
    data[i] ^= data[i+2] ^= data[i] ^= data[i+2];
}

/// \brief Convert RGB to RGBA
///
/// Convert source 24 bits RGB buffer to 32 bits RGBA
///
/// \param[in]  rgb   : RGB data buffer.
/// \param[in]  n     : RGB Pixel count.
/// \param[in]  a     : Alpha value to set.
///
/// \return New RGBA buffer
///
inline static uint8_t* __unpack_rgb_to_rgba(const uint8_t* rgb, size_t n, uint8_t a)
{
  uint8_t* rgba = reinterpret_cast<uint8_t*>(Om_alloc(n * 4));
  if(!rgba) return nullptr;

  uint8_t* dp = rgba;

  for(size_t i = n; --i; dp += 4, rgb += 3) {
    (*reinterpret_cast<uint32_t*>(dp)) = (*reinterpret_cast<const uint32_t*>(rgb));
    dp[3] = a;
  }

  for(size_t i = 0; i < 3; ++i)
    dp[i] = rgb[i];

  dp[3] = a;

  return rgba;
}

/// \brief In-place convert RGB to RGBA
///
/// In-place convert source RGB data to RGBA data assuming input
/// buffer was allocated with enough space to hold RGBA data.
///
/// \param[in]  data  : RGB data buffer.
/// \param[in]  n     : RGB Pixel count.
/// \param[in]  a     : Alpha value to set.
///
/// \return New RGBA buffer
///
inline static void __inplace_rgb_to_rgba(uint8_t* data, size_t n, uint8_t a)
{
  uint8_t* dp = data + ((n * 4) - 4);
  uint8_t* sp = data + ((n * 3) - 3);

  for(size_t i = n; --i; sp -= 3, dp -= 4) {
    (*reinterpret_cast<uint32_t*>(dp)) = (*reinterpret_cast<uint32_t*>(sp));
    dp[3] = a;
  }
}

/// \brief Set pixel color
///
/// Set color of one RGBA pixel.
///
/// \param[in]  pixel : Pointer to RGBA pixel.
/// \param[in]  color : Color to set.
///
/// \return New RGBA buffer
///
inline static void __set_pixel_32(uint8_t* pixel, uint32_t color)
{
  (*reinterpret_cast<uint32_t*>(pixel)) = color;
}

/// \brief Set color for row of pixels
///
/// Set color of row of RGBA pixels.
///
/// \param[in]  row   : Pointer to pixels row.
/// \param[in]  width : Size of row in pixels.
/// \param[in]  color : Color to set.
///
/// \return New RGBA buffer
///
inline static void __set_row_32(uint8_t* row, size_t width, uint32_t color)
{
  for(size_t i = 0; i < width; ++i, row +=4)
    (*reinterpret_cast<uint32_t*>(row)) = color;
}


/// \brief Set pixel color
///
/// Set color of one RGBA pixel.
///
/// \param[in]  pixel : Pointer to RGBA pixel.
/// \param[in]  color : Color to set.
///
/// \return New RGBA buffer
///
inline static void __cpy_pixel_32(uint8_t* dst, const uint8_t* src)
{
  (*reinterpret_cast<uint32_t*>(dst)) = (*reinterpret_cast<const uint32_t*>(src));
}


/// \brief Clamp number Macro.
///
/// Clamp number between min and mix values.
///
/// \param[in]  l : Minimum value
/// \param[in]  n : value to clamp
/// \param[in]  u : Maximum value
///
#define CLAMP(l, n, u) (((n) <= (l)) ? (l) : ((n) >= (u)) ? (u) : (n))
#define MIN(n, u) (((n) >= (u)) ? (u) : (n))

/// \brief Cubic interpolation Macro.
///
/// Cubic interpolation equation.
///
/// \param[in]  a : Start handle
/// \param[in]  b : Start node
/// \param[in]  c : End node
/// \param[in]  d : End handle
/// \param[in]  t : Interpolation phase
///
#define CUBIC_INTERP(a, b, c, d, t) ((b) + 0.5f * (t) * ((c) - (a) + (t) * (2.0f * (a) - 5.0f * (b) + 4.0f * (c) - (d) + (t) * (3.0f * ((b) - (c)) + (d) - (a)))))

/// \brief Linear interpolation Macro.
///
/// Linear interpolation equation.
///
/// \param[in]  a : Start
/// \param[in]  b : End
/// \param[in]  t : Interpolation phase
///
#define LERP(a, b, t) ((a) + (t) * ((b) - (a)))

/// \brief Get bicubic interpolated pixel.
///
/// Compute the bicubic interpolated pixel sample at the specified
/// coordinates of the given source image.
///
/// \param[in]  sample  : Array to receive interpolated pixel components.
/// \param[in]  u       : Sample horizontal coordinate in image, from 0.0 to 1.0.
/// \param[in]  v       : Sample vertical coordinate in image, from 0.0 to 1.0.
/// \param[in]  src_pix : Input image RGB(A) data.
/// \param[in]  src_w   : Input image width.
/// \param[in]  src_h   : Input image height.
///
inline static void __get_sample_cub_32(uint8_t* sample, float u, float v, const uint8_t* src_pix, unsigned src_w, unsigned src_h, int max_w, int max_h, size_t row_bytes)
{
  float u_x, v_y;

  float t_x = std::modf((u * src_w) - 0.5f, &u_x);
  float t_y = std::modf((v * src_h) - 0.5f, &v_y);

  int32_t b_x = static_cast<int32_t>(u_x) - 1;
  int32_t b_y = static_cast<int32_t>(v_y) - 1;

  const uint8_t* sp[4];

  float r[4]; float g[4]; float b[4]; float a[4];

  for(int32_t j = 0; j < 4; ++j) {

    int32_t y = CLAMP(0, b_y + j, max_h) * row_bytes;

    for(int32_t i = 0; i < 4; ++i) {

      int32_t x = CLAMP(0, b_x + i, max_w) * 4;

      sp[i] = src_pix + (y + x);
    }

    r[j] = CUBIC_INTERP(sp[0][0], sp[1][0], sp[2][0], sp[3][0], t_x);
    g[j] = CUBIC_INTERP(sp[0][1], sp[1][1], sp[2][1], sp[3][1], t_x);
    b[j] = CUBIC_INTERP(sp[0][2], sp[1][2], sp[2][2], sp[3][2], t_x);
    a[j] = CUBIC_INTERP(sp[0][3], sp[1][3], sp[2][3], sp[3][3], t_x);
  }

  float m[4];

  m[0] = CUBIC_INTERP(r[0], r[1], r[2], r[3], t_y);
  m[1] = CUBIC_INTERP(g[0], g[1], g[2], g[3], t_y);
  m[2] = CUBIC_INTERP(b[0], b[1], b[2], b[3], t_y);
  m[3] = CUBIC_INTERP(a[0], a[1], a[2], a[3], t_y);

  sample[0] = static_cast<uint8_t>(CLAMP(0.0f, m[0], 255.0f));
  sample[1] = static_cast<uint8_t>(CLAMP(0.0f, m[1], 255.0f));
  sample[2] = static_cast<uint8_t>(CLAMP(0.0f, m[2], 255.0f));
  sample[3] = static_cast<uint8_t>(CLAMP(0.0f, m[3], 255.0f));
}

/// \brief Get bilinear interpolated pixel.
///
/// Compute the bilinear interpolated pixel sample at the specified
/// coordinates of the given source image.
///
/// \param[in]  sample  : Array to receive interpolated pixel components.
/// \param[in]  u       : Sample horizontal coordinate in image, from 0.0 to 1.0.
/// \param[in]  v       : Sample vertical coordinate in image, from 0.0 to 1.0.
/// \param[in]  src_pix : Input image RGB(A) data.
/// \param[in]  src_w   : Input image width.
/// \param[in]  src_h   : Input image height.
///
inline static void __get_sample_lin_32(uint8_t* sample, float u, float v, const uint8_t* src_pix, unsigned src_w, unsigned src_h, int max_w, int max_h, size_t row_bytes)
{
  float u_x, v_y;

  float t_x = std::modf((u * src_w) - 0.5f, &u_x);
  float t_y = std::modf((v * src_h) - 0.5f, &v_y);

  int b_x = static_cast<int>(u_x) - 1;
  int b_y = static_cast<int>(v_y) - 1;

  const uint8_t* sp[2];

  float r[2];
  float g[2];
  float b[2];
  float a[2];

  for(int32_t j = 0; j < 2; ++j) {

    int32_t y = CLAMP(0, b_y + j, max_h) * row_bytes;

    for(int32_t i = 0; i < 2; ++i) {

      int32_t x = CLAMP(0, b_x + i, max_w) * 4;

      sp[i] = src_pix + (y + x);
    }

    r[j] = LERP(sp[0][0], sp[1][0], t_x);
    g[j] = LERP(sp[0][1], sp[1][1], t_x);
    b[j] = LERP(sp[0][2], sp[1][2], t_x);
    a[j] = LERP(sp[0][3], sp[1][3], t_x);
  }

  sample[0] = static_cast<uint8_t>(LERP(r[0], r[1], t_y));
  sample[1] = static_cast<uint8_t>(LERP(g[0], g[1], t_y));
  sample[2] = static_cast<uint8_t>(LERP(b[0], b[1], t_y));
  sample[3] = static_cast<uint8_t>(LERP(a[0], a[1], t_y));
}


/// \brief Get box sample pixel.
///
/// Get the box pixel sample at the specified coordinates of the given
/// source image.
///
/// \param[in]  sample    : Array to receive interpolated pixel components.
/// \param[in]  b_w       : Box width in pixels.
/// \param[in]  b_h       : Box height in pixels.
/// \param[in]  u         : Sample horizontal coordinate in image, from 0.0 to 1.0.
/// \param[in]  v         : Sample vertical coordinate in image, from 0.0 to 1.0.
/// \param[in]  src_pix   : Input image RGB(A) data.
/// \param[in]  src_w     : Input image width.
/// \param[in]  src_h     : Input image height.
/// \param[in]  max_w     : Input image X index limit, should be src_w - 1.
/// \param[in]  max_h     : Input image Y index limit, should be src_h - 1.
/// \param[in]  row_bytes : size in bytes of an input image row
///
inline static void __get_sample_box_32(uint8_t* sample, int b_w, int b_h, float u, float v, const uint8_t* src_pix, unsigned src_w, unsigned src_h, int max_w, int max_h, size_t row_bytes)
{
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 0.0f;

  const uint8_t *sp;

  float n = 0;

  // box top-left corner position
  int b_x = (u * src_w) - (0.5f * b_w);
  int b_y = (v * src_h) - (0.5f * b_h);

  for(int32_t j = 0; j < b_h; ++j) {

    int32_t y = b_y + j;
    if(y < 0 || y > max_h) continue;

    y *= row_bytes;

    for(int32_t i = 0; i < b_w; ++i) {

      int32_t x = b_x + i;
      if(x < 0 || x > max_w) continue;

      sp = src_pix + (y + (x * 4));

      r += sp[0];
      g += sp[1];
      b += sp[2];
      a += sp[3];

      n += 1.0f;
    }
  }

  // assign average pixel
  sample[0] = r / n;
  sample[1] = g / n;
  sample[2] = b / n;
  sample[3] = a / n;
}

/// \brief Copy and resample using bicubic interpolation.
///
/// Copy and resamples the specified rectangle of source image to destination
/// using bicubic interpolation.
///
/// This function should be preferred for upsampling operation, meaning when
/// the destination resolution is greater than the specified source rectangle.
///
/// \param[out] dst_pix   : Destination pixel buffer that receive result.
/// \param[in]  dst_w     : Destination width in pixel.
/// \param[in]  dst_h     : Destination height in pixel.
/// \param[in]  src_pix   : Source pixel buffer.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
/// \param[in]  src_c     : Source component count (bytes per pixel)
/// \param[in]  rec_x     : Rectangle top-left corner x coordinate in source.
/// \param[in]  rec_y     : Rectangle top-left corner y coordinate in source
/// \param[in]  rec_w     : Rectangle width
/// \param[in]  rec_h     : Rectangle height.
///
inline static void __copy_resample_cub(uint8_t* dst_pix, unsigned dst_w, unsigned dst_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, unsigned rec_x, unsigned rec_y, unsigned rec_w, unsigned rec_h)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // U and V shift corresponding to rectangle top-left corner
  float s_u = static_cast<float>(rec_x) / src_w;
  float s_v = static_cast<float>(rec_y) / src_h;

  // U and V factor corresponding to rectangle width and height
  float f_u = (1.0f / static_cast<float>(dst_w)) * (static_cast<float>(rec_w) / src_w);
  float f_v = (1.0f / static_cast<float>(dst_h)) * (static_cast<float>(rec_h) / src_h);

  // some constants
  size_t dst_row_bytes = (dst_w * 4); //< assuming RGBA data
  size_t src_row_bytes = (src_w * 4); //< assuming RGBA data
  int max_w = (src_w - 1);
  int max_h = (src_h - 1);

  // Loop for RGBA
  for(unsigned y = 0; y < dst_h; ++y) {
    uint8_t* dp = dst_pix + (dst_row_bytes * y);
    float v = s_v + (y * f_v);
    for(unsigned x = 0; x < dst_w; ++x, dp += 4) {
      float u = s_u + (x * f_u);
      __get_sample_cub_32(dp, u, v, src_pix, src_w, src_h, max_w, max_h, src_row_bytes);
    }
  }

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __copy_resample_cub : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG
}

/// \brief Copy and resample using bilinear interpolation.
///
/// Copy and resamples the specified rectangle of source image to destination
/// using bilinear interpolation.
///
/// \param[out] dst_pix   : Destination pixel buffer that receive result.
/// \param[in]  dst_w     : Destination width in pixel.
/// \param[in]  dst_h     : Destination height in pixel.
/// \param[in]  src_pix   : Source pixel buffer.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
/// \param[in]  src_c     : Source component count (bytes per pixel)
/// \param[in]  rec_x     : Rectangle top-left corner x coordinate in source.
/// \param[in]  rec_y     : Rectangle top-left corner y coordinate in source
/// \param[in]  rec_w     : Rectangle width
/// \param[in]  rec_h     : Rectangle height.
///
inline static void __copy_resample_lin(uint8_t* dst_pix, unsigned dst_w, unsigned dst_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, unsigned rec_x, unsigned rec_y, unsigned rec_w, unsigned rec_h)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // U and V shift corresponding to rectangle top-left corner
  float s_u = static_cast<float>(rec_x) / src_w;
  float s_v = static_cast<float>(rec_y) / src_h;

  // U and V factor corresponding to rectangle width and height
  float f_u = (1.0f / static_cast<float>(dst_w)) * (static_cast<float>(rec_w) / src_w);
  float f_v = (1.0f / static_cast<float>(dst_h)) * (static_cast<float>(rec_h) / src_h);

  // some constants
  size_t dst_row_bytes = (dst_w * 4); //< assuming RGBA data
  size_t src_row_bytes = (src_w * 4); //< assuming RGBA data
  int max_w = (src_w - 1);
  int max_h = (src_h - 1);

  // Loop for RGBA
  for(unsigned y = 0; y < dst_h; ++y) {
    uint8_t* dp = dst_pix + (dst_row_bytes * y);
    float v = s_v + (y * f_v);
    for(unsigned x = 0; x < dst_w; ++x, dp += 4) {
      float u = s_u + (x * f_u);
      __get_sample_lin_32(dp, u, v, src_pix, src_w, src_h, max_w, max_h, src_row_bytes);
    }
  }

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __copy_resample_lin : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG
}

/// \brief Copy and resample using box filter.
///
/// Copy and resamples the specified rectangle of source image to destination
/// using box filter.
///
/// This function should be preferred for downsampling operation, meaning when
/// the destination resolution is lesser than the specified source rectangle.
///
/// \param[out] dst_pix   : Destination pixel buffer that receive result.
/// \param[in]  dst_w     : Destination width in pixel.
/// \param[in]  dst_h     : Destination height in pixel.
/// \param[in]  src_pix   : Source pixel buffer.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
/// \param[in]  src_c     : Source component count (bytes per pixel)
/// \param[in]  rec_x     : Rectangle top-left corner x coordinate in source.
/// \param[in]  rec_y     : Rectangle top-left corner y coordinate in source
/// \param[in]  rec_w     : Rectangle width
/// \param[in]  rec_h     : Rectangle height.
///
inline static void __copy_resample_box(uint8_t* dst_pix, unsigned dst_w, unsigned dst_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, unsigned rec_x, unsigned rec_y, unsigned rec_w, unsigned rec_h)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // compute box size
  int b_w = ceil(static_cast<float>(rec_w) / dst_w);
  int b_h = ceil(static_cast<float>(rec_h) / dst_h);

  // U and V shift corresponding to rectangle top-left corner
  float s_u = static_cast<float>(rec_x) / src_w;
  float s_v = static_cast<float>(rec_y) / src_h;

  // U and V factor corresponding to rectangle width and height
  float f_u = (1.0f / static_cast<float>(dst_w)) * (static_cast<float>(rec_w) / src_w);
  float f_v = (1.0f / static_cast<float>(dst_h)) * (static_cast<float>(rec_h) / src_h);

  // some constants
  size_t dst_row_bytes = (dst_w * 4); //< assuming RGBA data
  size_t src_row_bytes = (src_w * 4); //< assuming RGBA data
  int max_w = (src_w - 1);
  int max_h = (src_h - 1);

  for(unsigned y = 0; y < dst_h; ++y) {
    uint8_t* dp = dst_pix + (dst_row_bytes * y);
    float v = s_v + (y * f_v);
    for(unsigned x = 0; x < dst_w; ++x, dp += 4) {
      float u = s_u + (x * f_u);
      __get_sample_box_32(dp, b_w, b_h, u, v, src_pix, src_w, src_h, max_w, max_h, src_row_bytes);
    }
  }


  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __copy_resample_box : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG
}

/// \brief Copy and resample.
///
/// Copy and resamples the specified rectangle of source image to destination
/// using best filtering method according source and destination resolutions.
///
/// \param[out] dst_pix   : Destination pixel buffer that receive result.
/// \param[in]  dst_w     : Destination width in pixel.
/// \param[in]  dst_h     : Destination height in pixel.
/// \param[in]  dst_f     : Destination color format.
/// \param[in]  src_pix   : Source pixel buffer.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
/// \param[in]  src_c     : Source component count (bytes per pixel)
/// \param[in]  rec_x     : Rectangle top-left corner x coordinate in source.
/// \param[in]  rec_y     : Rectangle top-left corner y coordinate in source
/// \param[in]  rec_w     : Rectangle width
/// \param[in]  rec_h     : Rectangle height.
///
inline static void __copy_resample(uint8_t* dst_pix, unsigned dst_w, unsigned dst_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, unsigned rec_x, unsigned rec_y, unsigned rec_w, unsigned rec_h)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // resize image to fit desired size
  if(rec_w != dst_w || rec_h != dst_h) {
    if(dst_w > rec_w || dst_h > rec_h) {
      // bicubic interpolation
      __copy_resample_cub(dst_pix, dst_w, dst_h, src_pix, src_w, src_h, rec_x, rec_y, rec_w, rec_h);
    } else {
      // average box filter
      __copy_resample_box(dst_pix, dst_w, dst_h, src_pix, src_w, src_h, rec_x, rec_y, rec_w, rec_h);
    }

  } else {

    size_t row_bytes = (rec_w * 4); //< assuming RGBA data
    size_t row_shift = (rec_x * 4); //< assuming RGBA data

    for(unsigned y = 0; y < dst_h; ++y) {
      const uint8_t* sp = src_pix + ((y + rec_y) * row_bytes) + row_shift;
      uint8_t* dp = dst_pix + (y * row_bytes);
      for(unsigned x = 0; x < dst_w; ++x, dp += 4, sp += 4) {
        dp[0] = sp[0];
        dp[1] = sp[1];
        dp[2] = sp[2];
        dp[3] = sp[3];
      }
    }

    #ifdef DEBUG
    t = clock() - t;
    std::cout << "DEBUG => __copy_resample : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
    #endif // DEBUG
  }
}

/// \brief Draw image in destination canvas
///
/// Draws the source image to fit into the given canvas keeping the source
/// aspect ratio, resampling source image using bicubic interpolation.
///
/// This function should be preferred for upsampling operation, meaning when
/// the destination resolution is greater than the specified source rectangle.
///
/// \param[out] cv_pix    : Canvas pixel buffer that receive result.
/// \param[in]  cv_w      : Canvas width in pixel.
/// \param[in]  cv_h      : Canvas height in pixel.
/// \param[in]  src_pix   : Source pixel buffer.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
/// \param[in]  src_c     : Source component count (bytes per pixel)
/// \param[in]  bck       : Background color
///
inline static void __draw_canvas_cub(uint8_t* cv_pix, unsigned cv_w, unsigned cv_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, uint32_t bck)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  unsigned dst_x, dst_y, dst_w, dst_h;

  if((static_cast<float>(src_w) / src_h) > (static_cast<float>(cv_w) / cv_h)) {

    dst_w = cv_w;
    dst_h = cv_w * (static_cast<float>(src_h) / src_w);
    dst_x = 0;
    dst_y = (0.5f * cv_h) - (0.5f * dst_h);

  } else {

    dst_w = cv_h * (static_cast<float>(src_w) / src_h);
    dst_h = cv_h;
    dst_x = (0.5f * cv_w) - (0.5f * dst_w);
    dst_y = 0;
  }

  // U and V factor corresponding to rectangle width and height
  float f_u = 1.0f / static_cast<float>(dst_w - 1);
  float f_v = 1.0f / static_cast<float>(dst_h - 1);

  // U and V shift corresponding to destination top-left corner in canvas
  float s_u = f_u * dst_x;
  float s_v = f_v * dst_y;

  // some constants
  size_t dst_row_bytes = (cv_w  * 4); //< assuming RGBA data
  size_t src_row_bytes = (src_w * 4); //< assuming RGBA data
  int max_w = (src_w - 1);
  int max_h = (src_h - 1);

  for(unsigned y = 0; y < cv_h; ++y) {
    uint8_t* dp = cv_pix + (dst_row_bytes * y);
    float v = (y * f_v) - s_v;
    if(v < 0.0 || v > 1.0) {
      __set_row_32(dp, cv_w, bck);
      continue;
    }
    for(unsigned x = 0; x < cv_w; ++x, dp += 4) {
      float u = (x * f_u) - s_u;
      if(u < 0.0 || u > 1.0) {
        __set_pixel_32(dp, bck);
      } else {
        __get_sample_cub_32(dp, u, v, src_pix, src_w, src_h, max_w, max_h, src_row_bytes);
      }
    }
  }

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __draw_canvas_cub : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG
}


/// \brief Draw image in destination canvas
///
/// Draws the source image to fit into the given canvas keeping the source
/// aspect ratio, resampling source image using bilinear interpolation.
///
/// This function should be preferred for upsampling operation, meaning when
/// the destination resolution is greater than the specified source rectangle.
///
/// \param[out] cv_pix    : Canvas pixel buffer that receive result.
/// \param[in]  cv_w      : Canvas width in pixel.
/// \param[in]  cv_h      : Canvas height in pixel.
/// \param[in]  src_pix   : Source pixel buffer.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
/// \param[in]  src_c     : Source component count (bytes per pixel)
/// \param[in]  bck       : Background color
///
inline static void __draw_canvas_lin(uint8_t* cv_pix, unsigned cv_w, unsigned cv_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, uint32_t bck)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  unsigned dst_x, dst_y, dst_w, dst_h;

  if((static_cast<float>(src_w) / src_h) > (static_cast<float>(cv_w) / cv_h)) {

    dst_w = cv_w;
    dst_h = cv_w * (static_cast<float>(src_h) / src_w);
    dst_x = 0;
    dst_y = (0.5f * cv_h) - (0.5f * dst_h);

  } else {

    dst_w = cv_h * (static_cast<float>(src_w) / src_h);
    dst_h = cv_h;
    dst_x = (0.5f * cv_w) - (0.5f * dst_w);
    dst_y = 0;
  }

  // U and V factor corresponding to rectangle width and height
  float f_u = 1.0f / static_cast<float>(dst_w - 1);
  float f_v = 1.0f / static_cast<float>(dst_h - 1);

  // U and V shift corresponding to destination top-left corner in canvas
  float s_u = f_u * dst_x;
  float s_v = f_v * dst_y;

  // some constants
  size_t dst_row_bytes = (cv_w  * 4); //< assuming RGBA data
  size_t src_row_bytes = (src_w * 4); //< assuming RGBA data
  int max_w = (src_w - 1);
  int max_h = (src_h - 1);

  for(unsigned y = 0; y < cv_h; ++y) {
    uint8_t* dp = cv_pix + (dst_row_bytes * y);
    float v = (y * f_v) - s_v;
    if(v < 0.0 || v > 1.0) {
      __set_row_32(dp, cv_w, bck);
      continue;
    }
    for(unsigned x = 0; x < cv_w; ++x, dp += 4) {
      float u = (x * f_u) - s_u;
      if(u < 0.0 || u > 1.0) {
        __set_pixel_32(dp, bck);
      } else {
        __get_sample_lin_32(dp, u, v, src_pix, src_w, src_h, max_w, max_h, src_row_bytes);
      }
    }
  }

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __draw_canvas_lin : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG
}


/// \brief Draw image in destination canvas
///
/// Draws the source image to fit into the given canvas keeping the source
/// aspect ratio, resampling source image using box filter.
///
/// This function should be preferred for downsampling operation, meaning when
/// the destination resolution is lesser than the specified source rectangle.
///
/// \param[out] cv_pix    : Canvas pixel buffer that receive result.
/// \param[in]  cv_w      : Canvas width in pixel.
/// \param[in]  cv_h      : Canvas height in pixel.
/// \param[in]  src_pix   : Source pixel buffer.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
/// \param[in]  src_c     : Source component count (bytes per pixel)
/// \param[in]  bck       : Background color
///
inline static void __draw_canvas_box(uint8_t* cv_pix, unsigned cv_w, unsigned cv_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, uint32_t bck)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  unsigned dst_x, dst_y, dst_w, dst_h;

  if((static_cast<float>(src_w) / src_h) > (static_cast<float>(cv_w) / cv_h)) {

    dst_w = cv_w;
    dst_h = cv_w * (static_cast<float>(src_h) / src_w);
    dst_x = 0;
    dst_y = (0.5f * cv_h) - (0.5f * dst_h);

  } else {

    dst_w = cv_h * (static_cast<float>(src_w) / src_h);
    dst_h = cv_h;
    dst_x = (0.5f * cv_w) - (0.5f * dst_w);
    dst_y = 0;
  }

  // compute box size
  int b_w = ceil(static_cast<float>(src_w) / dst_w);
  int b_h = ceil(static_cast<float>(src_h) / dst_h);

  // U and V factor corresponding to rectangle width and height
  float f_u = 1.0f / static_cast<float>(dst_w - 1);
  float f_v = 1.0f / static_cast<float>(dst_h - 1);

  // U and V shift corresponding to destination top-left corner in canvas
  float s_u = f_u * dst_x;
  float s_v = f_v * dst_y;

  // some constants
  size_t dst_row_bytes = (cv_w  * 4); //< assuming RGBA data
  size_t src_row_bytes = (src_w * 4); //< assuming RGBA data
  int max_w = (src_w - 1);
  int max_h = (src_h - 1);

  for(unsigned y = 0; y < cv_h; ++y) {
    uint8_t* dp = cv_pix + (dst_row_bytes * y);
    float v = (y * f_v) - s_v;
    if(v < 0.0f || v > 1.0f) {
      __set_row_32(dp, cv_w, bck);
      continue;
    }
    for(unsigned x = 0; x < cv_w; ++x, dp += 4) {
      float u = (x * f_u) - s_u;
      if(u < 0.0f || u > 1.0f) {
        __set_pixel_32(dp, bck);
      } else {
        __get_sample_box_32(dp, b_w, b_h, u, v, src_pix, src_w, src_h, max_w, max_h, src_row_bytes);
      }
    }
  }

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __draw_canvas_box : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG
}

/// \brief Draw image in destination canvas.
///
/// Draws and resamples the specified rectangle of source image to destination
/// using best filtering method according source and destination resolutions.
///
/// \param[out] cv_pix    : Canvas pixel buffer that receive result.
/// \param[in]  cv_w      : Canvas width in pixel.
/// \param[in]  cv_h      : Canvas height in pixel.
/// \param[in]  dst_f     : Destination color format.
/// \param[in]  src_pix   : Source pixel buffer.
/// \param[in]  src_w     : Source width.
/// \param[in]  src_h     : source height.
/// \param[in]  bck       : Background color
///
inline static void __draw_canvas(uint8_t* cv_pix, unsigned cv_w, unsigned cv_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, uint32_t bck)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // resize image to fit desired size
  if(src_w != cv_w || src_h != cv_h) {

    bool mag;

    // determins whether we need to downsample or upsample source image
    if((static_cast<float>(src_w) / src_h) > (static_cast<float>(cv_w) / cv_h)) {
      mag = cv_w > src_w;
    } else {
      mag = cv_h > src_h;
    }

    if(mag) {
/*
      // bicubic interpolation
      __draw_canvas_cub(cv_pix, cv_w, cv_h, src_pix, src_w, src_h, __bytes_swap(bck));

      #ifdef DEBUG
      t = clock() - t;
      std::cout << "DEBUG => __draw_canvas_cub : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
      #endif // DEBUG
      */
      __draw_canvas_lin(cv_pix, cv_w, cv_h, src_pix, src_w, src_h, __bytes_swap(bck));

      #ifdef DEBUG
      t = clock() - t;
      std::cout << "DEBUG => __draw_canvas_lin : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
      #endif // DEBUG

    } else {

      // average box filter
/*
      __draw_canvas_box(cv_pix, cv_w, cv_h, src_pix, src_w, src_h, __bytes_swap(bck));

      #ifdef DEBUG
      t = clock() - t;
      std::cout << "DEBUG => __draw_canvas_box : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
      #endif // DEBUG
*/
      __draw_canvas_lin(cv_pix, cv_w, cv_h, src_pix, src_w, src_h, __bytes_swap(bck));

      #ifdef DEBUG
      t = clock() - t;
      std::cout << "DEBUG => __draw_canvas_lin : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
      #endif // DEBUG
    }

  } else {

    memcpy(cv_pix, src_pix, src_w * src_h * 4);

    #ifdef DEBUG
    t = clock() - t;
    std::cout << "DEBUG => __draw_canvas cpy : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
    #endif // DEBUG
  }
}

/* we make sure structures are packed to be properly aligned with
 read buffer */
#pragma pack(1)
/// \brief BMP info header
///
/// Structure for BMP file info header
struct OM_BITMAPINFOHEADER {
  uint32_t  size;           ///< size of the structure
  uint32_t  width;          ///< image width
  uint32_t  height;         ///< image height
  uint16_t  planes;         ///< bit planes
  int16_t   bpp;            ///< bit per pixel
  uint32_t  compression;    ///< compression
  uint32_t  sizeimage;      ///< size of the image
  int32_t   xppm;           ///< pixels per meter X
  int32_t   yppm;           ///< pixels per meter Y
  uint32_t  clrused;        ///< colors used
  uint32_t  clrimportant;   ///< important colors
}; // 40 bytes
/// \brief BMP base header
///
/// Structure for BMP file base header
struct OM_BITMAPHEADER {
  uint8_t   signature[2];   ///< BM magic word
  uint32_t  size;           ///< size of the whole .bmp file
  uint16_t  reserved1;      ///< must be 0
  uint16_t  reserved2;      ///< must be 0
  uint32_t  offbits;        ///< where bitmap data begins
}; // 14 bytes
#pragma pack()

/// BMP file specific signature / magic number
static const unsigned char __sign_bmp[] = "BM";
/// JPG file specific signature / magic number
static const unsigned char __sign_jpg[] = {0xFF, 0xD8, 0xFF};
/// PNG file specific signature / magic number
static const unsigned char __sign_png[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
/// GIF file specific signature / magic number
static const unsigned char __sign_gif[] = "GIF89a";

/// \brief compare buffer to known images signatures
///
/// Check whether the given buffer matches any known file
/// signature or magic number.
///
/// \param[in]  buff  : Buffer to compare known signatures with
///
/// \return found image type or 0
///
inline static unsigned __image_sign_matches(const uint8_t* buff)
{
  // Test BMP signature
  if(0 == memcmp(buff, __sign_bmp, 2)) return OM_IMAGE_BMP;
  // Test JPG signature
  if(0 == memcmp(buff, __sign_jpg, 3)) return OM_IMAGE_JPG;
  // Test PNG signature
  if(0 == memcmp(buff, __sign_png, 8)) return OM_IMAGE_PNG;
  // Test GIF signature
  if(0 == memcmp(buff, __sign_gif, 6)) return OM_IMAGE_GIF;

  return 0;
}

/// \brief Quantized color node
///
/// Structure for linked list node describing a quantized color
///
struct __qz_rgb {
  uint8_t   rgb[3];       //< RGB color
  uint8_t   pos;          //< Position index in linked-list
  int32_t   ref_count;    //< Count pixel that reference this color
  __qz_rgb* next;         //< Next node in linked-list
};

/// \brief Quantized map
///
/// Structure for quantized color map
///
struct __qz_map {
  uint8_t   rgb_min[3];
  uint8_t   rgb_rng[3];
  uint32_t  idx_count;     //< Total number of pixels in all the entries
  uint32_t  size;          //< # of __qz_rgb in linked list below
  __qz_rgb* node_list;
};

/// \brief Quantized color sorting function
///
/// Quantized color node sorting function along Red axis
///
static bool __qz_sort_r_fn(const __qz_rgb* a, const __qz_rgb* b)
{
  unsigned h1 = a->rgb[0] * 256 * 256 + a->rgb[1] * 256 + a->rgb[2];
  unsigned h2 = b->rgb[0] * 256 * 256 + b->rgb[1] * 256 + b->rgb[2];
  return (h1 < h2);
}

/// \brief Quantized color sorting function
///
/// Quantized color node sorting function along Green axis
///
static bool __qz_sort_g_fn(const __qz_rgb* a, const __qz_rgb* b)
{
  unsigned h1 = a->rgb[1] * 256 * 256 + a->rgb[2] * 256 + a->rgb[0];
  unsigned h2 = b->rgb[1] * 256 * 256 + b->rgb[2] * 256 + b->rgb[0];
  return (h1 < h2);
}

/// \brief Quantized color sorting function
///
/// Quantized color node sorting function along Blue axis
///
static bool __qz_sort_b_fn(const __qz_rgb* a, const __qz_rgb* b)
{
  unsigned h1 = a->rgb[2] * 256 * 256 + a->rgb[0] * 256 + a->rgb[1];
  unsigned h2 = b->rgb[2] * 256 * 256 + b->rgb[0] * 256 + b->rgb[1];
  return (h1 < h2);
}

/// \brief Quantized color sort functions array
///
/// Array containing pointers to quantized color sort functions
///
static bool (*__qz_sort_fn[])(const __qz_rgb*, const __qz_rgb*) = {
  __qz_sort_r_fn,
  __qz_sort_g_fn,
  __qz_sort_b_fn
  };

/// \brief Quantization subdivision
///
/// Color quantization function to subdivide the RGB space recursively
/// using median cut in each axes alternatingly until ColorMapSize different
/// cubes exists.
/// The biggest cube in one dimension is subdivide unless it has only one entry.
///
/// \param[in]  cmap      : Pointer to color map to subdivide
/// \param[in]  in_size   : Initial size of the supplied color map
/// \param[in]  out_size  : New size of the subdivided color map
///
/// the following implementation is a rewriting of the SubdivColorMap
/// function from the quantize.c file of the GifLib library.
///
static inline void __image_quantize_subdiv(__qz_map* cmap, unsigned* out_size, unsigned in_size)
{
  __qz_rgb* node;
  std::vector<__qz_rgb*> sort_list;
  unsigned sort_axis, min_color, max_color, n, c, i, j, u = 0;
  int rng_max, r;

  while(in_size > *out_size) {
    // Find candidate for subdivision:
    rng_max = -1;
    for(i = 0; i < *out_size; ++i) {
      for(j = 0; j < 3; ++j) {
        if((static_cast<int>(cmap[i].rgb_rng[j]) > rng_max) && (cmap[i].size > 1)) {
          rng_max = cmap[i].rgb_rng[j];
          u = i;
          sort_axis = j;
        }
      }
    }

    if(rng_max == -1)
      return;

    // Split the entry Index into two along the axis SortRGBAxis:

    // Sort all elements in that entry along the given axis and split at
    // the median.
    sort_list.reserve(cmap[u].size);

    for(j = 0, node = cmap[u].node_list; j < cmap[u].size && node != nullptr; j++, node = node->next) {
      sort_list.push_back(node);
    }

    // Because qsort isn't stable, this can produce differing
    // results for the order of tuples depending on platform
    // details of how qsort() is implemented.
    //
    // We mitigate this problem by sorting on all three axes rather
    // than only the one specied by SortRGBAxis; that way the instability
    // can only become an issue if there are multiple color indices
    // referring to identical RGB tuples.  Older versions of this
    // sorted on only the one axis.
    sort(sort_list.begin(), sort_list.end(), __qz_sort_fn[sort_axis]);

    for(j = 0; j < cmap[u].size - 1; ++j)
      sort_list[j]->next = sort_list[j+1];

    sort_list[cmap[u].size - 1]->next = nullptr;
    cmap[u].node_list = node = sort_list[0];

    sort_list.clear();

    // Now simply add the Counts until we have half of the Count:
    r = cmap[u].idx_count / 2 - node->ref_count;
    n = 1;
    c = node->ref_count;
    while(node->next != nullptr && (r -= node->next->ref_count) >= 0 && node->next->next != nullptr) {
      node = node->next;
      n++;
      c += node->ref_count;
    }
    // Save the values of the last color of the first half, and first
    // of the second half so we can update the Bounding Boxes later.
    // Also as the colors are quantized and the BBoxes are full 0..255,
    // they need to be rescaled.
    max_color = node->rgb[sort_axis]; //< Max. of first half
    // coverity[var_deref_op]
    min_color = node->next->rgb[sort_axis]; //< of second
    max_color <<= 3;
    min_color <<= 3;

    // Partition right here:
    cmap[*out_size].node_list = node->next;
    node->next = nullptr;
    cmap[*out_size].idx_count = c;
    cmap[u].idx_count -= c;
    cmap[*out_size].size = cmap[u].size - n;
    cmap[u].size = n;
    for(j = 0; j < 3; ++j) {
      cmap[*out_size].rgb_min[j] = cmap[u].rgb_min[j];
      cmap[*out_size].rgb_rng[j] = cmap[u].rgb_rng[j];
    }
    cmap[*out_size].rgb_rng[sort_axis] = cmap[*out_size].rgb_min[sort_axis] + cmap[*out_size].rgb_rng[sort_axis] - min_color;
    cmap[*out_size].rgb_min[sort_axis] = min_color;

    cmap[u].rgb_rng[sort_axis] = max_color - cmap[u].rgb_min[sort_axis];

    (*out_size)++;
  }
}

/// \brief Color quantization
///
/// Function to Quantize high resolution image into lower one. Input image
/// consists of a 2D array for each of the RGB colors with size Width by Height.
/// There is no Color map for the input. Output is a quantized image with 2D
/// array of indexes into the output color map.
/// Note input image can be 24 bits at the most (8 for red/green/blue) and
/// the output has 256 colors at the most (256 entries in the color map.).
/// ColorMapSize specifies size of color map up to 256 and will be updated to
/// real size before returning.
/// Also non of the parameter are allocated by this routine.
///
/// the following implementation is a rewriting of the GifQuantizeBuffer
/// function from the quantize.c file of the GifLib library.
///
/// \param[out]   out_idx   : Output image pixels color indices (must be allocated).
/// \param[out]   out_map   : Output image color map (must be allocated).
/// \param[out]   map_size  : As input, the desired maximum size of color map, as output, the actual final size of color map.
/// \param[in]    in_rgb    : Input image RGB(A) pixel data.
/// \param[in]    in_w      : Input image width in pixels.
/// \param[in]    in_w      : Input image height in pixels.
/// \param[in]    in_c      : Input image color component count (bytes per pixel).
///
/// \return true if operation succeed, false otherwise.
///
static bool __image_quantize(uint8_t* out_idx, uint8_t* out_map, unsigned* map_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  __qz_map new_cmap[256];
  __qz_rgb *node_list;
  __qz_rgb *node;

  unsigned u, i, j;

  const uint8_t* sp;
  uint8_t* dp;

  size_t mtx_bytes = in_w * in_h;

  node_list = reinterpret_cast<__qz_rgb*>(Om_alloc(sizeof(__qz_rgb) * 32768));
  if(!node_list) return false;

  for(i = 0; i < 32768; ++i) {
    node_list[i].rgb[0] =  (i >> 10);
    node_list[i].rgb[1] =  (i >>  5) & 0x1F;
    node_list[i].rgb[2] =  (i      ) & 0x1F;
    node_list[i].ref_count = 0;
  }

  // Sample the colors and their distribution:
  for(i = 0, sp = in_rgb; i < mtx_bytes; ++i, sp += in_c) {
    u = ((sp[0] >> 3) << 10) + ((sp[1] >> 3) << 5) + (sp[2] >> 3);
    node_list[u].ref_count++;
  }

  /* Put all the colors in the first entry of the color map, and call the
   * recursive subdivision process.  */
  for(i = 0; i < 256; i++) {
    new_cmap[i].node_list = nullptr;
    new_cmap[i].idx_count = 0;
    new_cmap[i].size = 0;
    for(j = 0; j < 3; j++) {
      new_cmap[i].rgb_min[j] = 0;
      new_cmap[i].rgb_rng[j] = 255;
    }
  }

  /* Find the non empty entries in the color table and chain them: */
  for(i = 0; i < 32768; ++i) {
    if(node_list[i].ref_count > 0) break;
  }

  node = new_cmap[0].node_list = &node_list[i];
  unsigned n = 1;
  while(++i < 32768) {
    if(node_list[i].ref_count > 0) {
      node->next = &node_list[i];
      node = &node_list[i];
      n++;
    }
  }
  node->next = nullptr;

  new_cmap[0].size = n;               //< Different sampled colors
  new_cmap[0].idx_count = mtx_bytes;   //< Pixels

  unsigned new_size = 1;

  __image_quantize_subdiv(new_cmap, &new_size, (*map_size));

  if(new_size < (*map_size)) {
    // And clear rest of color map:
    memset(out_map + (new_size * 3), 0, ((*map_size) - new_size) * 3);
  }

  // Average the colors in each entry to be the color to be used in the
  // output color map, and plug it into the output color map itself.
  unsigned r, g, b;
  for(i = 0, dp = out_map; i < new_size; ++i, dp += 3) {
    if((j = new_cmap[i].size) > 0) {
      node = new_cmap[i].node_list;
      r = g = b = 0;
      while(node) {
        node->pos = i;
        r += node->rgb[0];
        g += node->rgb[1];
        b += node->rgb[2];
        node = node->next;
      }
      dp[0] = (r << 3) / j;
      dp[1] = (g << 3) / j;
      dp[2] = (b << 3) / j;
    }
  }

  // Finally scan the input buffer again and put the mapped index in the
  // output buffer.
  for(i = 0, sp = in_rgb; i < mtx_bytes; ++i, sp += in_c) {
    u = ((sp[0] >> 3) << 10) + ((sp[1] >> 3) <<  5) + (sp[2] >> 3);
    out_idx[i] = node_list[u].pos;
  }

  Om_free(node_list);

  (*map_size) = new_size;

  return true;
}

/// \brief Custom GIF reader
///
/// Custom read function for GIF library to read a file pointer.
///
/// \param[in]  gif     Decoder structure pointer.
/// \param[in]  dst     Destination buffer.
/// \param[in]  len     Length of data that should be read.
///
/// \return  The number of bytes that were read
///
static int __gif_read_file_fn(GifFileType* gif, uint8_t* dst, int len)
{
  return fread(dst, 1, len, reinterpret_cast<FILE*>(gif->UserData));
}

/// \brief Custom GIF write struct
///
/// Custom structure for custom GIF write routine.
///
struct __gif_read_st {
  const uint8_t*  src_data;
  size_t    src_seek;
};

/// \brief Custom GIF reader
///
/// Custom read function for GIF library to read memory buffer.
///
/// \param[in]  gif     Decoder structure pointer.
/// \param[in]  dst     Destination buffer.
/// \param[in]  len     Length of data that should be read.
///
/// \return  The number of bytes that were read
///
static int __gif_read_buff_fn(GifFileType* gif, uint8_t* dst, int len)
{
  __gif_read_st* read_st = reinterpret_cast<__gif_read_st*>(gif->UserData);
  memcpy(dst, read_st->src_data + read_st->src_seek, len);
  read_st->src_seek += len;
  return len;
}

/// \brief Custom GIF writer
///
/// Custom read function for GIF library to write a file pointer.
///
/// \param[in]  gif     Decoder structure pointer.
/// \param[in]  src     Source buffer.
/// \param[in]  len     Length of data that should be read.
///
/// \return  The number of length that were read
///
static int __gif_write_file_fn(GifFileType* gif, const uint8_t* src, int len)
{
  return fwrite(src, 1, len, reinterpret_cast<FILE*>(gif->UserData));
}

/// \brief Custom GIF write struct
///
/// Custom structure for custom GIF write routine.
///
struct __gif_write_st {
  uint8_t*  dst_data;
  size_t    dst_size;
  size_t    dst_seek;
};

/// \brief Custom GIF writer
///
/// Custom write function for GIF library to encode to memory.
///
/// \param[in]  gif     Decoder structure pointer.
/// \param[in]  src     Source buffer.
/// \param[in]  len     Length of data that should be read.
///
/// \return  The number of length that were read
///
static int __gif_write_buff_fn(GifFileType* gif, const uint8_t* src, int len)
{
  __gif_write_st* write_st = reinterpret_cast<__gif_write_st*>(gif->UserData);
  write_st->dst_size += len;
  if(write_st->dst_data) {
    write_st->dst_data = reinterpret_cast<uint8_t*>(Om_realloc(write_st->dst_data, write_st->dst_size));
  } else {
    write_st->dst_data = reinterpret_cast<uint8_t*>(Om_alloc(write_st->dst_size));
  }
  memcpy(write_st->dst_data + write_st->dst_seek, src, len);
  write_st->dst_seek += len;
  return len;
}

/// \brief Decode GIF.
///
/// Common function to decode GIF using the given GIF decoder structure.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  gif       : GIF decoder structure pointer.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGBA pixels or nullptr if failed.
///
static uint8_t* __gif_decode_common(unsigned* w, unsigned* h, GifFileType* gif, bool flip_y)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // Retrieve GIF decoder struct
  int error;

  // Load GIF content
  if(DGifSlurp(gif) == GIF_ERROR) {
    DGifCloseFile(gif, &error);
    return nullptr;
  }

  // Get image list, we care only about the first one
  SavedImage* images = gif->SavedImages;

  // Get image informations
  unsigned gif_w = images[0].ImageDesc.Width;
  unsigned gif_h = images[0].ImageDesc.Height;

  // Pointer to color table
  ColorMapObject* table;

  // check whether we got a local color table (may never happen)
  if(images[0].ImageDesc.ColorMap) {
    table = images[0].ImageDesc.ColorMap;
  } else {
    table = gif->SColorMap;
  }

  // define some useful sizes, we output as RGBA
  size_t row_bytes = gif_w * 4;
  size_t tot_bytes = gif_h * row_bytes;

  // allocate new buffer
  uint8_t* pixels = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!pixels) {
    DGifCloseFile(gif, &error);
    return nullptr;
  }

  // get GIF index list
  uint8_t* sp = static_cast<uint8_t*>(images[0].RasterBits);

  // destination pointer
  uint8_t* dp;

  // here we go to translate indexed color to RGB
  if(flip_y) {
    size_t max_h = gif_h - 1;
    for(unsigned y = 0; y < gif_h; ++y) {
      dp = pixels + (row_bytes * (max_h - y));
      for(unsigned x = 0; x < gif_w; ++x, ++sp, dp += 4) {
        dp[0] = table->Colors[*sp].Red;
        dp[1] = table->Colors[*sp].Green;
        dp[2] = table->Colors[*sp].Blue;
        dp[3] = 0xFF;
      }
    }
  } else {
    for(unsigned y = 0; y < gif_h; ++y) {
      dp = pixels + (y * row_bytes);
      for(unsigned x = 0; x < gif_w; ++x, ++sp, dp += 4) {
        dp[0] = table->Colors[*sp].Red;
        dp[1] = table->Colors[*sp].Green;
        dp[2] = table->Colors[*sp].Blue;
        dp[3] = 0xFF;
      }
    }
  }

  // free decoder
  DGifCloseFile(gif, &error);

  // assign output values
  (*w) = gif_w;
  (*h) = gif_h;

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __gif_decode_common : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return pixels;
}

/// \brief Encode GIF.
///
/// Common function to encode GIF using the given GIF encoder structure.
///
/// \param[in]  gif     : GIF encoder structure pointer.
/// \param[in]  in_rgb  : Input image RGB(A) data to encode.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count.
///
/// \return True if operation succeed, false otherwise
///
static bool __gif_encode_common(GifFileType* gif, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // Retrieve GIF encoder struct
  int error;

  // define useful sizes
  size_t mtx_bytes = in_w * in_h; //< image matrix size, one byte per pixel

  // allocate new buffer to receive color indices
  uint8_t* imtx = reinterpret_cast<uint8_t*>(Om_alloc(mtx_bytes));
  if(!imtx) {
    EGifCloseFile(gif, &error);
    return false;
  }

  // allocate new color map of 256 colors
  unsigned cmap_size = 256;
  uint8_t* cmap = reinterpret_cast<uint8_t*>(Om_alloc(cmap_size * 3)); //< cmap_size * RGB
  if(!cmap) {
    Om_free(imtx);
    EGifCloseFile(gif, &error);
    return false;
  }

  // quantize image
  if(!__image_quantize(imtx, cmap, &cmap_size, in_rgb, in_w, in_h, in_c)) {
    Om_free(imtx); Om_free(cmap);
    EGifCloseFile(gif, &error);
    return false;
  }

  // set GIF global parameters
  gif->SWidth = in_w;
  gif->SHeight = in_h;
  gif->SColorResolution = 8;
  gif->SBackGroundColor = 0;
  gif->SColorMap = GifMakeMapObject(256, reinterpret_cast<GifColorType*>(cmap)); //< global color table

  // set image parameters
  SavedImage image;
  image.ImageDesc.Left = 0;
  image.ImageDesc.Top = 0;
  image.ImageDesc.Width = in_w;
  image.ImageDesc.Height = in_h;
  image.ImageDesc.Interlace = false;
  image.ImageDesc.ColorMap = nullptr; //< no local color table
  image.RasterBits = imtx; //< our color indices
  image.ExtensionBlockCount = 0;
  image.ExtensionBlocks = nullptr;

  // add image to gif encoder
  GifMakeSavedImage(gif, &image);

  // encode GIF
  if(GIF_OK != EGifSpew(gif)) {
    Om_free(imtx);
    Om_free(cmap);
    EGifCloseFile(gif, &error);
    return false;
  }

  // free allocated data
  Om_free(imtx);
  Om_free(cmap);

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __gif_encode_common : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return true;
}

/// \brief Read GIF file.
///
/// Read GIF data from file pointer.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  gif_file  : File pointer to read GIF data from.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGBA pixels or nullptr if failed.
///
static uint8_t* __gif_read(unsigned* w, unsigned* h, FILE* gif_file, bool flip_y)
{
  int error;
  GifFileType* gif;

  // make sure we start at beginning
  fseek(gif_file, 0, SEEK_SET);

  // Define custom read function and load GIF header
  gif = DGifOpen(gif_file, __gif_read_file_fn, &error);
  if(gif == nullptr)
    return nullptr;

  // Decode GIF data
  return __gif_decode_common(w, h, gif, flip_y);
}

/// \brief Decode GIF data.
///
/// Decode GIF data from buffer in memory.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  gif_data  : Buffer to GIF data to decode.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGBA pixels or nullptr if failed.
///
static uint8_t* __gif_decode(unsigned* w, unsigned* h, const uint8_t* gif_data, bool flip_y)
{
  int error;
  GifFileType* gif;

  // custom read structure
  __gif_read_st read_st;
  read_st.src_data = gif_data;
  read_st.src_seek = 0;

  // Define custom read function and load GIF header
  gif = DGifOpen(&read_st, __gif_read_buff_fn, &error);
  if(gif == nullptr)
    return nullptr;

  // Decode GIF data
  return __gif_decode_common(w, h, gif, flip_y);
}

/// \brief Write GIF file.
///
/// Write GIF data to file pointer.
///
/// \param[out] out_file  : File pointer to write to.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return True if operation succeed, false otherwise
///
static bool __gif_write(FILE* out_file, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  int error;
  GifFileType* gif;

  // Define custom read function and load GIF header
  gif = EGifOpen(out_file, __gif_write_file_fn, &error);
  if(gif == nullptr)
    return false;

  // Encode RGB to GIF data
  return __gif_encode_common(gif, in_rgb, in_w, in_h, in_c);
}

/// \brief Encode GIF data.
///
/// Encode GIF data to buffer in memory.
///
/// \param[out] out_size  : Output GIF data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return Pointer to encoded GIF image data or nullptr if failed.
///
static uint8_t* __gif_encode(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  int error;
  GifFileType* gif;

  // custom write structure
  __gif_write_st write_st;
  write_st.dst_data = nullptr;
  write_st.dst_size = 0;
  write_st.dst_seek = 0;

  // Define custom read function and load GIF header
  gif = EGifOpen(&write_st, __gif_write_buff_fn, &error);
  if(gif == nullptr)
    return nullptr;

  // Encode RGB to GIF data
  if(!__gif_encode_common(gif, in_rgb, in_w, in_h, in_c))
    return nullptr;

  // assign output values
  (*out_size) = write_st.dst_size;

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __gif_encode : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return write_st.dst_data;
}

/// BMP Codec Context early delcaration
typedef struct _BMP_CONTEXT BMP_CONTEXT;

/// \brief BMP stream callback
///
/// Generic BMP input or output stream callback function for
/// encode and decode routines
///
/// \param[in]  ctx   : Pointer to BMP_CONTEXT structure.
/// \param[in]  buf   : Pointer to buffer to read or write to.
/// \param[in]  len   : Requested bytes to read or write.
///
/// \return Count of read or written bytes.
///
typedef size_t (*bmp_io_fn)(BMP_CONTEXT* ctx, void* buf, size_t len);

/// \brief BMP seek callback
///
/// Generic BMP seek callback function for encode and decode routines
///
/// \param[in]  ctx   : Pointer to BMP_CONTEXT structure.
/// \param[in]  off   : Offset to seek pointer to.
///
typedef void (*bmp_seek_fn)(BMP_CONTEXT* ctx, size_t off);

/// \brief BMP Codec Context
///
/// BMP Context structure for encode and decode routines
///
struct _BMP_CONTEXT
{
  FILE* f_ptr;

  const void* i_ptr;

  void* o_ptr;

  size_t io_off;

  void** out_dest;

  size_t* out_size;

  bmp_io_fn io_fn;

  bmp_seek_fn seek_fn;
};

/// \brief File seek callback for BMP decode
///
/// Callback function definition for seek file pointer
///
static void __bmp_seek_file(BMP_CONTEXT* ctx, size_t off)
{
  fseek(ctx->f_ptr, off, SEEK_SET);
}

/// \brief File seek callback for BMP decode
///
/// Callback function definition for seek file pointer
///
static void __bmp_seek_mem(BMP_CONTEXT* ctx, size_t off)
{
  ctx->io_off = off;
}

/// \brief File read callback for BMP decode
///
/// Callback function definition for read from file pointer
///
static size_t __bmp_read_file(BMP_CONTEXT* ctx, void* buf, size_t len)
{
  size_t rb = fread(buf, 1, len, ctx->f_ptr);
  return rb;
}

/// \brief Memory read callback for BMP decode
///
/// Callback function definition for read from memory
///
static size_t __bmp_read_mem(BMP_CONTEXT* ctx, void* buf, size_t len)
{
  const uint8_t* sp = reinterpret_cast<const uint8_t*>(ctx->i_ptr);
  memcpy(buf, sp + ctx->io_off, len);
  ctx->io_off += len;
  return len;
}

/// \brief File read callback for BMP decode
///
/// Callback function definition for read from file pointer
///
static size_t __bmp_write_file(BMP_CONTEXT* ctx, void* buf, size_t len)
{
  size_t wb = fwrite(buf, 1, len, ctx->f_ptr);
  return wb;
}

/// \brief Memory read callback for BMP decode
///
/// Callback function definition for read from memory
///
static size_t __bmp_write_mem(BMP_CONTEXT* ctx, void* buf, size_t len)
{
  uint8_t* dp = reinterpret_cast<uint8_t*>(ctx->o_ptr);
  memcpy(dp + ctx->io_off, buf, len);
  ctx->io_off += len;
  return len;
}

/// \brief Init BMP context.
///
/// Initialize BMP context for decode from file.
///
/// \param[out] ctx     : Pointer to BMP Codec Context structure.
/// \param[out] file    : File pointer to read.
///
void __bmp_init_read_file(BMP_CONTEXT* ctx, FILE* file)
{
  ctx->f_ptr = file;
  // reset pointer offset
  ctx->io_off = 0;
  // set callback functions
  ctx->io_fn = __bmp_read_file;
  ctx->seek_fn = __bmp_seek_file;

  // avoid freeing bad pointer
  ctx->o_ptr = nullptr;
}

/// \brief Init BMP context.
///
/// Initialize BMP context for decode from file.
///
/// \param[out] ctx     : Pointer to BMP Codec Context structure.
/// \param[out] file    : File pointer to read.
///
void __bmp_init_read_mem(BMP_CONTEXT* ctx, const void* ptr)
{
  ctx->i_ptr = ptr;
  // reset pointer offset
  ctx->io_off = 0;
  // set callback functions
  ctx->io_fn = __bmp_read_mem;
  ctx->seek_fn = __bmp_seek_mem;

  // avoid freeing bad pointer
  ctx->o_ptr = nullptr;
}


/// \brief Init BMP context.
///
/// Initialize BMP context for encode to file.
///
/// \param[out] ctx     : Pointer to BMP Codec Context structure.
/// \param[out] file    : File pointer to read.
///
void __bmp_init_write_file(BMP_CONTEXT* ctx, FILE* file)
{
  ctx->f_ptr = file;
  // set callback functions
  ctx->io_fn = __bmp_write_file;
  ctx->seek_fn = __bmp_seek_file;

  // avoid freeing bad pointer
  ctx->o_ptr = nullptr;
}

/// \brief Init BMP context.
///
/// Initialize BMP context for encode to memory.
///
/// \param[out] ctx     : Pointer to BMP Codec Context structure.
/// \param[out] pdest   : Pointer to pointer that receive allocated data.
/// \param[out] psize   : Pointer to size_t that receive allocated data size.
///
void __bmp_init_write_mem(BMP_CONTEXT* ctx, void** pdest, size_t* psize)
{
  // this is not a write to file
  ctx->f_ptr = nullptr;

  // store user pointers
  ctx->out_dest = pdest;
  ctx->out_size = psize;
  *ctx->out_dest = nullptr;
  *ctx->out_size = 0;

  // null buffer to be allocated
  ctx->o_ptr = nullptr;

  // reset pointer offset
  ctx->io_off = 0;

  // set callback functions
  ctx->io_fn = __bmp_write_mem;
  ctx->seek_fn = __bmp_seek_mem;
}

/// \brief Init BMP context.
///
/// Allocate required memory for encode to memory
///
/// \param[out] ctx     : Pointer to BMP Codec Context structure.
/// \param[out] size    : File pointer to read.
///
/// \return true if operation succeed, false otherwise
///
inline bool __bmp_alloc_write_mem(BMP_CONTEXT* ctx, size_t size)
{
  // we do not allocate memory for file
  if(ctx->f_ptr != nullptr)
    return true;

  // this should never happen
  if(ctx->o_ptr)
    Om_free(ctx->o_ptr);

  // allocate buffer
  ctx->o_ptr = Om_alloc(size);
  if(!ctx->o_ptr) return false;

  // set values to user variables
  *ctx->out_dest = ctx->o_ptr;
  *ctx->out_size = size;

  return true;
}

/// \brief Free BMP context allocated data.
///
/// Free any allocated buffer and reset data.
///
/// \param[out] ctx     : Pointer to BMP Codec Context structure.
///
///
inline void __bmp_free(BMP_CONTEXT* ctx)
{
  // check for allocated write buffer
  if(ctx->o_ptr) {
    Om_free(ctx->o_ptr);
    ctx->o_ptr = nullptr;
    *ctx->out_dest = ctx->o_ptr;
    *ctx->out_size = 0;
  }
}

/// \brief Decode BMP.
///
/// Common function to decode BMP using the given BMP codec context structure.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  bmp_ctx   : Pointer to BMP Codec Context.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGBA image pixels or nullptr if failed.
///
static uint8_t* __bmp_decode_common(unsigned* w, unsigned* h, BMP_CONTEXT* bmp_ctx, bool flip_y)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // read function
  bmp_io_fn read_cb = bmp_ctx->io_fn;
  bmp_seek_fn seek_cb = bmp_ctx->seek_fn;

  if(!read_cb) return nullptr;
  if(!seek_cb) return nullptr;

  // BMP headers structures
  OM_BITMAPHEADER bmp_head;
  OM_BITMAPINFOHEADER bmp_info;
  // get base header
  if(read_cb(bmp_ctx, &bmp_head, 14) != 14)
    return nullptr;
  // check BM signature
  if(0 != memcmp(bmp_head.signature, "BM", 2))
    return nullptr;
  // get info header
  if(read_cb(bmp_ctx, &bmp_info, 40) != 40)
    return nullptr;

  // we support only 24 or 32 bpp
  if(bmp_info.bpp < 24)
    return nullptr;

  // get BMP image parameters
  unsigned bmp_w = bmp_info.width;
  unsigned bmp_h = bmp_info.height;
  unsigned bmp_c = bmp_info.bpp / 8; // channel count

  // define some useful sizes
  size_t row_bytes = bmp_w * bmp_c;
  size_t tot_bytes = bmp_h * row_bytes;

  // allocate new buffer, large enough to receive **RGBA** data
  uint8_t* pixels = reinterpret_cast<uint8_t*>(Om_alloc(bmp_w * bmp_h * 4));
  if(!pixels) return nullptr;

  // seek to bitmap data location
  seek_cb(bmp_ctx, bmp_head.offbits);

  // BMP data is natively stored upside down
  if(flip_y) {
    // read all data at once from
    if(read_cb(bmp_ctx, pixels, tot_bytes) < tot_bytes) {
      Om_free(pixels); return nullptr;
    }
  } else {
    // read rows in reverse order
    unsigned max_h = (bmp_h - 1);
    for(unsigned y = 0; y < bmp_h; ++y) {
      if(read_cb(bmp_ctx, pixels + (row_bytes * (max_h - y)), row_bytes) < row_bytes) {
        Om_free(pixels); return nullptr;
      }
    }
  }

  // in-place conversion BGR to BGRA
  if(bmp_c == 3)
    __inplace_rgb_to_rgba(pixels, bmp_w * bmp_h, 0xFF);

  // finally swap components order BGRA to RGBA
  __in_place_rb_swap_32(pixels, tot_bytes);

  // assign output values
  (*w) = bmp_w;
  (*h) = bmp_h;

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __bmp_decode_common : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return pixels;
}

/// \brief Encode BMP data.
///
/// Encode BMP data to buffer in memory.
///
/// \param[in]  bmp_ctx   : Pointer to BMP Codec Context.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return Pointer to encoded BMP image data or nullptr if failed.
///
static bool __bmp_encode_common(BMP_CONTEXT* bmp_ctx, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // write function
  bmp_io_fn write_cb = bmp_ctx->io_fn;
  //bmp_seek_fn seek_cb = bmp_ctx->seek_fn;

  // compute data sizes
  size_t hdr_bytes = sizeof(OM_BITMAPHEADER) + sizeof(OM_BITMAPINFOHEADER);
  size_t row_bytes = in_w * in_c;                   //< row size in bytes
  size_t r4b_bytes = row_bytes + (row_bytes % 4);   //< row size rounded up to a multiple of 4 bytes
  size_t tot_bytes = r4b_bytes * in_h;
  size_t bmp_bytes = tot_bytes + hdr_bytes;

  // BMP headers structure
  OM_BITMAPHEADER bmp_head = {};
  bmp_head.signature[0] = 0x42; bmp_head.signature[1] = 0x4D; // BM signature
  bmp_head.offbits = hdr_bytes; // file header + info header = 54 bytes
  bmp_head.size = bmp_bytes;

  OM_BITMAPINFOHEADER bmp_info = {};
  bmp_info.size = sizeof(OM_BITMAPINFOHEADER);
  bmp_info.width = in_w;
  bmp_info.height = in_h;
  bmp_info.planes = 1;
  bmp_info.bpp = in_c * 8;
  bmp_info.compression = 0;
  bmp_info.sizeimage = tot_bytes;
  bmp_info.xppm = bmp_info.yppm = 0x0ec4;

  // checks whether we need to allocate buffer
  if(!__bmp_alloc_write_mem(bmp_ctx, bmp_bytes)) {
    return false;
  }

  // write file header
  if(write_cb(bmp_ctx, &bmp_head, sizeof(OM_BITMAPHEADER)) != sizeof(OM_BITMAPHEADER)) {
    __bmp_free(bmp_ctx); return false;
  }

  // write info header
  if(write_cb(bmp_ctx, &bmp_info, sizeof(OM_BITMAPINFOHEADER)) != sizeof(OM_BITMAPINFOHEADER)) {
    __bmp_free(bmp_ctx); return false;
  }

  // allocate buffer for data translation
  uint8_t* row = reinterpret_cast<uint8_t*>(Om_alloc(r4b_bytes));
  if(!row) {
    __bmp_free(bmp_ctx); return false;
  }

  // fill padding with zeros
  if((r4b_bytes - row_bytes) > 0)
    memset(row + row_bytes, 0, r4b_bytes - row_bytes);

  // useful values for translation
  const uint8_t* sp;
  uint8_t* dp;
  unsigned hmax = (in_h - 1);

  if(in_c == 4) {
    for(unsigned y = 0; y < in_h; ++y) {
      dp = row;
      sp = in_rgb + (row_bytes * (hmax - y)); // reverse row up to bottom
      for(unsigned x = 0; x < in_w; ++x, sp += 4, dp += 4) {
        // convert RGBA to BGRA
        dp[0] = sp[2]; dp[1] = sp[1]; dp[2] = sp[0]; dp[3] = sp[3];
      }
      // write row
      if(write_cb(bmp_ctx, row, r4b_bytes) != r4b_bytes) {
        __bmp_free(bmp_ctx); return false;
      }
    }
  } else {
    for(unsigned y = 0; y < in_h; ++y) {
      dp = row;
      sp = in_rgb + (row_bytes * (hmax - y)); // reverse row up to bottom
      for(unsigned x = 0; x < in_w; ++x, sp += 3, dp += 3) {
        // convert RGB to BGR
        dp[0] = sp[2]; dp[1] = sp[1]; dp[2] = sp[0];
      }
      // write row
      if(write_cb(bmp_ctx, row, r4b_bytes) != r4b_bytes) {
        __bmp_free(bmp_ctx); return false;
      }
    }
  }

  Om_free(row);

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __bmp_encode_common : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return true;
}


/// \brief Read BMP file.
///
/// Read BMP data from file pointer.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  bmp_file  : File pointer to read BMP data from.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGBA image piwels or nullptr if failed.
///
static uint8_t* __bmp_read(unsigned* w, unsigned* h, FILE* bmp_file, bool flip_y)
{
  // BMP Codec Context
  BMP_CONTEXT bmp;

  // make sure we start at begining
  fseek(bmp_file, 0, SEEK_SET);

  // Initialize BMP context for file read
  __bmp_init_read_file(&bmp, bmp_file);

  // decode BMP data
  return __bmp_decode_common(w, h, &bmp, flip_y);
}

/// \brief Decode BMP data.
///
/// Decode BMP data from buffer in memory.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  bmp_data  : Buffer to BMP data to decode.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGBA piwels data or nullptr if failed.
///
static uint8_t* __bmp_decode(unsigned* w, unsigned* h,  const uint8_t* bmp_data, bool flip_y)
{
  // BMP Codec Context
  BMP_CONTEXT bmp;

  // Initialize BMP context for read from memory
  __bmp_init_read_mem(&bmp, bmp_data);

  // decode BMP data
  return __bmp_decode_common(w, h, &bmp, flip_y);
}


/// \brief Write BMP file.
///
/// Write BMP data to file pointer.
///
/// \param[out] out_file  : File pointer to write to.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
///
/// \return True if operation succeed, false otherwise
///
static bool __bmp_write(FILE* out_file, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // BMP Codec Context
  BMP_CONTEXT bmp;

  // make sure we start at begining
  fseek(out_file, 0, SEEK_SET);

  // Initialize BMP context for file read
  __bmp_init_write_file(&bmp, out_file);

  // decode BMP data
  return __bmp_encode_common(&bmp, in_rgb, in_w, in_h, in_c);
}

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
static uint8_t* __bmp_encode(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // BMP Codec Context
  BMP_CONTEXT bmp;

  // pointer to be allocated
  uint8_t* bmp_data;
  size_t bmp_size;

  // Initialize BMP context for read from memory
  __bmp_init_write_mem(&bmp, reinterpret_cast<void**>(&bmp_data), &bmp_size);

  // decode BMP data
  if(!__bmp_encode_common(&bmp, in_rgb, in_w, in_h, in_c))
    return nullptr;

  (*out_size) = bmp_size;

  return bmp_data;
}

/// \brief Decode JPEG.
///
/// Common function to decode JPEG using the given GIF decoder structure.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  jpg_dec   : JPEG decoder structure pointer.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGBA image pixels or nullptr if failed.
///
static uint8_t* __jpg_decode_common(unsigned* w, unsigned* h, void* jpg_dec, bool flip_y)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  jpeg_decompress_struct* jpg = reinterpret_cast<jpeg_decompress_struct*>(jpg_dec);

  // read jpeg header
  if(jpeg_read_header(jpg, true) != 1)
    return nullptr;

  // initialize decompression
  jpeg_start_decompress(jpg);

  // get image parameters
	unsigned jpg_w = jpg->output_width;
	unsigned jpg_h = jpg->output_height;
	unsigned jpg_c = jpg->output_components;

	// define sizes
  size_t row_bytes = jpg_w * jpg_c;

  // allocate buffer, large enough to store **RGBA** data
  uint8_t* pixels = reinterpret_cast<uint8_t*>(Om_alloc(jpg_w * jpg_h * 4));
  if(!pixels) return nullptr;

  // row pointer for jpeg decoder
  uint8_t* dp;

  if(flip_y) {
    unsigned max_h = jpg_h - 1;
    while(jpg->output_scanline < jpg->output_height) {
      dp = pixels + ((max_h - jpg->output_scanline) * row_bytes);
      jpeg_read_scanlines(jpg, &dp, 1); //< read one row (scanline)
    }
  } else {
    while(jpg->output_scanline < jpg->output_height) {
      dp = pixels + (jpg->output_scanline * row_bytes);
      jpeg_read_scanlines(jpg, &dp, 1); //< read one row (scanline)
    }
  }

	// finalize reading
	jpeg_finish_decompress(jpg);

  // cleanup decoder
	jpeg_destroy_decompress(jpg);

  // in-place conversion RGB to RGBA
  if(jpg_c == 3)
    __inplace_rgb_to_rgba(pixels, jpg_w * jpg_h, 0xFF);

	// assign output values
	(*w) = jpg_w;
	(*h) = jpg_h;

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __jpg_decode_common : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

	return pixels;
}

/// \brief Encode JEPG.
///
/// Common function to encode JEPG using the given JEPG encoder structure.
///
/// \param[in]  jpg_enc : JEPG encoder structure pointer.
/// \param[in]  in_rgb  : Input image RGB(A) data to encode.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count.
/// \param[in]  level   : JPEG compression quality level 0 to 100.
///
/// \return True if operation succeed, false otherwise
///
static bool __jpg_encode_common(void* jpg_enc, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  jpeg_compress_struct* jpg = reinterpret_cast<jpeg_compress_struct*>(jpg_enc);

  // clamp quality value
  if(level < 0) level = 0;
  if(level > 100) level = 100;

  // define image parameters
  jpg->image_width = in_w;        //< Image width in pixels
  jpg->image_height = in_h;       //< Image height in pixels
  jpg->input_components = 3;      //< per pixel color components
  jpg->in_color_space = JCS_RGB;  //< pixel format

  // set parameters to encoder
  jpeg_set_defaults(jpg);
  // set compression quality
  jpeg_set_quality(jpg, level, true); // quality is 0-100 scaled
  // initialize encoder
  jpeg_start_compress(jpg, true);

  // hold row size in bytes
  unsigned row_bytes = in_w * in_c;

  if(in_c == 4) {

    // JPEG encoder does not handle RGBA source we must convert data
    const uint8_t* sp;
    uint8_t* dp;

    // create new buffer for one RGB row
    uint8_t* row = reinterpret_cast<uint8_t*>(Om_alloc(in_w * 3));
    if(!row) return false;

    // give RGB data to JPEG encoder
    while(jpg->next_scanline < jpg->image_height) {
      // set source and destination pointers
      sp = in_rgb + (jpg->next_scanline * row_bytes);
      dp = row;
      // convert RGBA to RGB
      for(unsigned i = 0; i < in_w; ++i) {
        dp[0] = sp[0];
        dp[1] = sp[1];
        dp[2] = sp[2];
        sp += 4;
        dp += 3;
      }
      // send to encoder
      jpeg_write_scanlines(jpg, &row, 1);
    }

    Om_free(row);

  } else {

    // pointer to source RGB row
    uint8_t* row_p;

    // give RGB data to JPEG encoder
    while(jpg->next_scanline < jpg->image_height) {
      // get pointer to rows
      row_p = const_cast<uint8_t*>(in_rgb + (jpg->next_scanline * row_bytes));
      // send to encoder
      jpeg_write_scanlines(jpg, &row_p, 1);
    }
  }

  // finalize compression
  jpeg_finish_compress(jpg);
  // destroy encoder
  jpeg_destroy_compress(jpg);

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __jpg_encode_common : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return true;
}

/// \brief Read JPEG file.
///
/// Read JPEG data from file pointer.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  jpg_file  : File pointer to read JPEG data from.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGBA pixels or nullptr if failed.
///
static uint8_t* __jpg_read(unsigned* w, unsigned* h, FILE* jpg_file, bool flip_y)
{
  // create base object for jpeg decoder
  jpeg_decompress_struct jpg;
  jpeg_error_mgr jerr;

  // create jpeg decoder
  jpg.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&jpg);

  // make sure we start at beginning and setup jpeg IO
  fseek(jpg_file, 0, SEEK_SET);
  jpeg_stdio_src(&jpg, jpg_file);

  return __jpg_decode_common(w, h, &jpg, flip_y);
}

/// \brief Decode JPEG data.
///
/// Decode JPEG data from buffer in memory.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  jpg_data  : Buffer to JPEG data to decode.
/// \param[in]  jpg_size  : Size of JPEG data to decode.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGBA pixels or nullptr if failed.
///
static uint8_t* __jpg_decode(unsigned* w, unsigned* h, const uint8_t* jpg_data, size_t jpg_size, bool flip_y)
{
  // create base object for jpeg decoder
  jpeg_decompress_struct jpg;
  jpeg_error_mgr jerr;

  // create jpeg decoder
  jpg.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&jpg);

  // set read data pointer
  jpeg_mem_src(&jpg, jpg_data, jpg_size);

  return __jpg_decode_common(w, h, &jpg, flip_y);
}

/// \brief Write JPEG file.
///
/// Write JPEG data to file pointer.
///
/// \param[out] out_file  : File pointer to write to.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
/// \param[in]  level     : JPEG compression quality level 0 to 10.
///
/// \return True if operation succeed, false otherwise
///
static bool __jpg_write(FILE* out_file, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // create base object for jpeg encoder
  jpeg_compress_struct jpg;
  jpeg_error_mgr jerr;

  // create jpeg encoder
  jpg.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&jpg);

  // make sure we start at beginning and setup jpeg IO
  fseek(out_file, 0, SEEK_SET);
  jpeg_stdio_dest(&jpg, out_file);

  return __jpg_encode_common(&jpg, in_rgb, in_w, in_h, in_c, level * 10);
}

/// \brief Encode JPEG data.
///
/// Encode JPEG data to buffer in memory.
///
/// \param[out] out_size  : Output JPEG data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
/// \param[in]  level     : JPEG compression quality level 0 to 10.
///
/// \return Pointer to decoded RGB(A) image data or nullptr if failed.
///
static uint8_t* __jpg_encode(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{

  // create base object for jpeg encoder
  jpeg_compress_struct jpg;
  jpeg_error_mgr jerr;

  // create jpeg encoder
  jpg.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&jpg);

  // pointer to be allocated
  uint8_t* jpg_data = nullptr;

  // set pointer params
  unsigned long jpg_size = 0;
  jpeg_mem_dest(&jpg, &jpg_data, &jpg_size);

  if(!__jpg_encode_common(&jpg, in_rgb, in_w, in_h, in_c, level * 10))
    return nullptr;

  (*out_size) = jpg_size;

  return jpg_data;
}

/// \brief Decode PNG.
///
/// Common function to decode PNG using the given PNG decoder structure.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  png       : PNG decoder structure pointer.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGBA pixels or nullptr if failed.
///
static uint8_t* __png_decode_common(unsigned* w, unsigned* h, png_structp png, bool flip_y)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // get decoder
  //png_structp png = reinterpret_cast<png_structp>(png_dec);

  // create PNG info structure
  png_infop png_info = png_create_info_struct(png);

  // get image properties
  png_read_info(png, png_info);
  unsigned png_w = png_get_image_width(png, png_info);
  unsigned png_h = png_get_image_height(png, png_info);
  unsigned png_c = png_get_channels(png, png_info);

  // we support only RGB or RGBA
  if(png_c < 3)
    return nullptr;

  // retrieve and define useful sizes
  size_t row_bytes = png_w * png_c;

  // allocate buffer, large enough to store **RGBA** data
  uint8_t* pixels = reinterpret_cast<uint8_t*>(Om_alloc(png_w * png_h * 4));
  if(!pixels) return nullptr;

  // define pointers to each row in output RGB(A) data
  if(flip_y) {
    unsigned hmax = png_h - 1;
    for(unsigned y = 0; y < png_h; ++y)
      png_read_row(png, pixels + ((hmax - y) * row_bytes), nullptr);
  } else {
    for(unsigned y = 0; y < png_h; ++y)
      png_read_row(png, pixels + (y * row_bytes), nullptr);
  }

  // cleanup
  png_destroy_read_struct(&png, &png_info, nullptr);

  // in-place conversion RGB to RGBA
  if(png_c == 3)
    __inplace_rgb_to_rgba(pixels, png_w * png_h, 0xFF);

  // assign output values
  (*w) = png_w;
  (*h) = png_h;

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __png_decode_common : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return pixels;
}

/// \brief Encode PNG.
///
/// Common function to encode PNG using the given PNG encoder structure.
///
/// \param[in]  png_enc : PNG encoder structure pointer.
/// \param[in]  in_rgb  : Input image RGB(A) data to encode.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count.
/// \param[in]  level   : PNG compression level 0 to 9.
///
/// \return True if operation succeed, false otherwise
///
static bool __png_encode_common(png_structp png, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // get encoder
  //png_structp png = reinterpret_cast<png_structp>(png_enc);

  // create PNG info structure
  png_infop png_info = png_create_info_struct(png);

  // set PNG parameters
  png_set_IHDR(png, png_info, in_w, in_h, 8,
               (in_c == 4) ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  // clamp compression level
  if(level < 0) level = 0;
  if(level > 9) level = 9;

  // set compression level
  png_set_compression_level(png, level);

  // write info to PNG
  png_write_info(png, png_info);

  // define useful sizes
  size_t row_bytes = in_w * in_c;

  // write each row
  for(unsigned y = 0; y < in_h; ++y) {
    png_write_row(png, const_cast<uint8_t*>(in_rgb + (y * row_bytes)));
  }

  // finalize write process
  png_write_end(png, nullptr);

  // clear PGN encoder
  png_destroy_write_struct(&png, &png_info);
  png_free_data(png, png_info, PNG_FREE_ALL, -1);

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __png_encode_common : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return true;
}

/// \brief Read PNG file.
///
/// Read PNG data from file pointer.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  png_file  : File pointer to read PNG data from.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGBA pixels or nullptr if failed.
///
static uint8_t* __png_read(unsigned* w, unsigned* h, FILE* png_file, bool flip_y)
{
  // create PNG decoder structure
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // make sure we start at beginning and setup png IO
  fseek(png_file, 0, SEEK_SET);
  png_init_io(png, png_file);

  // decode PNG data
  return __png_decode_common(w, h, png, flip_y);
}

/// \brief Custom PNG read struct
///
/// Custom structure for custom PNG read function
///
struct __png_read_st {
  const uint8_t*  src_data;
  size_t          src_seek;
};

/// \brief Custom PNG read
///
/// Custom read function for PNG library to read from memory.
///
/// \param[in]  png     Decoder structure pointer.
/// \param[in]  dst     Destination buffer.
/// \param[in]  len     Length of data that should be read.
///
void __png_read_buff_fn(png_structp png, uint8_t* dst, size_t len)
{
  __png_read_st *read_st = static_cast<__png_read_st*>(png_get_io_ptr(png));
  memcpy(dst, read_st->src_data + read_st->src_seek, len);
  read_st->src_seek += len;
}

/// \brief Decode PNG.
///
/// Decode PNG data from buffer in memory.
///
/// \param[out] w         : Pointer that receive decoded image width.
/// \param[out] h         : Pointer that receive decoded image height.
/// \param[in]  png_data  : Buffer to read PNG data from.
/// \param[in]  flip_y    : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGB(A) data or nullptr if failed.
///
static uint8_t* __png_decode(unsigned* w, unsigned* h, const uint8_t* png_data, bool flip_y)
{

  // create PNG decoder structure
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // custom read structure
  __png_read_st read_st;
  read_st.src_data = png_data;
  read_st.src_seek = 0;

  // set custom read process
  png_set_read_fn(png, &read_st, __png_read_buff_fn);

  // decode PNG data
  return __png_decode_common(w, h, png, flip_y);
}

/// \brief Write PNG file.
///
/// Write PNG data to file pointer.
///
/// \param[out] out_file  : File pointer to write to.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count, either 3 or 4.
/// \param[in]  level     : PNG compression level 0 to 9.
///
/// \return True if operation succeed, false otherwise
///
static bool __png_write(FILE* out_file, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // create PNG encoder structure
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // make sure we start at beginning and setup png IO
  fseek(out_file, 0, SEEK_SET);
  png_init_io(png, out_file);

  return __png_encode_common(png, in_rgb, in_w, in_h, in_c, level);
}

/// \brief Custom PNG write struct
///
/// Custom structure for custom PNG write function
///
struct __png_write_st {
  uint8_t*  dst_data;
  size_t    dst_size;
  size_t    dst_seek;
};

/// \brief Custom PNG writer
///
/// Custom write function for PNG library to encode to memory.
///
/// \param[in]  png     Decoder structure pointer.
/// \param[in]  src     Source data buffer.
/// \param[in]  len     Length of data that should be written.
///
void __png_write_buff_fn(png_structp png, uint8_t* src, size_t len)
{
  __png_write_st *write_st = static_cast<__png_write_st*>(png_get_io_ptr(png));
  write_st->dst_size += len;
  if(write_st->dst_data) {
    write_st->dst_data = reinterpret_cast<uint8_t*>(Om_realloc(write_st->dst_data, write_st->dst_size));
  } else {
    write_st->dst_data = reinterpret_cast<uint8_t*>(Om_alloc(write_st->dst_size));
  }
  if(!write_st->dst_data) png_error(png, "alloc error in __png_write_fn");
  memcpy(write_st->dst_data + write_st->dst_seek, src, len);
  write_st->dst_seek += len;
}

/// \brief Custom PNG flush function
///
/// Custom callback function for PNG encoder flush.
///
void __png_flush_fn(png_structp png)
{
}

/// \brief Encode PNG data.
///
/// Common function to encode PNG using the given PNG encoder structure.
///
/// \param[in]  out_size  : Pointer to receive encoded data size in bytes.
/// \param[in]  in_rgb    : Input image RGB(A) data to encode.
/// \param[in]  in_w      : Input image width.
/// \param[in]  in_h      : Input image height.
/// \param[in]  in_c      : Input image color component count.
/// \param[in]  level     : PNG compression level 0 to 9.
///
/// \return Pointer to encoded PNG data
///
static uint8_t* __png_encode(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // create PNG encoder structure
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // custom write structure
  __png_write_st write_st;
  write_st.dst_data = nullptr;
  write_st.dst_size = 0;
  write_st.dst_seek = 0;

  // custom write process
  png_set_write_fn(png, &write_st, __png_write_buff_fn, __png_flush_fn);

  if(!__png_encode_common(png, in_rgb, in_w, in_h, in_c, level))
    return nullptr;

  (*out_size) = write_st.dst_size;

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => __png_encode : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return write_st.dst_data;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_imgGetType(uint8_t* data)
{
  // identify image format
  return __image_sign_matches(data);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_imgGetType(FILE* file)
{
  // read first 8 bytes of the file
  uint8_t buff[8];
  fseek(file, 0, SEEK_SET);
  if(fread(buff, 1, 8, file) < 8) return -1;

  // identify image format
  return __image_sign_matches(buff);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_imgLoadFile(unsigned* w, unsigned* h, const OmWString& path, bool flip_y)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // initialize output values
  (*w) = 0;
  (*h) = 0;
  uint8_t* pix = nullptr;

  // Open file for reading
  FILE* fp;
  if((fp = _wfopen(path.c_str(), L"rb")) == nullptr)
    return nullptr;

  // read first 8 bytes of the file
  uint8_t buff[8];
  fseek(fp, 0, SEEK_SET);
  if(fread(buff, 1, 8, fp) < 8)
    return nullptr;

  // identify image format
  int type = __image_sign_matches(buff);

  // load according image format
  if(type != 0) {
    switch(type)
    {
    case OM_IMAGE_BMP:
      pix = __bmp_read(w, h, fp, flip_y);
      break;
    case OM_IMAGE_JPG:
      pix = __jpg_read(w, h, fp, flip_y);
      break;
    case OM_IMAGE_PNG:
      pix = __png_read(w, h, fp, flip_y);
      break;
    case OM_IMAGE_GIF:
      pix = __gif_read(w, h, fp, flip_y);
      break;
    }
  }

  fclose(fp);

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => Om_imgLoadFile : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return pix;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_imgLoadData(unsigned* w, unsigned* h, const uint8_t* data, size_t size, bool flip_y)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // initialize output values
  (*w) = 0;
  (*h) = 0;
  uint8_t* pix = nullptr;

  // prevent idiot attempts
  if(!data || !size)
    return nullptr;

  // identify image format
  int type = __image_sign_matches(data);

  // load according image format
  if(type != 0) {
    switch(type)
    {
    case OM_IMAGE_BMP:
      pix = __bmp_decode(w, h, data, flip_y);
      break;
    case OM_IMAGE_JPG:
      pix = __jpg_decode(w, h, data, size, flip_y);
      break;
    case OM_IMAGE_PNG:
      pix = __png_decode(w, h, data, flip_y);
      break;
    case OM_IMAGE_GIF:
      pix = __gif_decode(w, h, data, flip_y);
      break;
    }
  }

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => Om_imgLoadData : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return pix;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_imgLoadHBmp(unsigned *w, unsigned *h, HBITMAP hBmp)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  //HDC hDc = CreateCompatibleDC(nullptr);
  HDC hDc = GetDC(nullptr);

  BITMAPINFO bmInfo = {};
  bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);

  // Get the BITMAPINFO  structure from the bitmap
  if(0 == GetDIBits(hDc, hBmp, 0, 0, nullptr, &bmInfo, DIB_RGB_COLORS)) {
    return nullptr;
  }

  unsigned bmp_w = bmInfo.bmiHeader.biWidth;
  unsigned bmp_h = bmInfo.bmiHeader.biHeight;
  unsigned bmp_c = bmInfo.bmiHeader.biBitCount / 8;

  size_t row_bytes = bmp_w * bmp_c;
  size_t tot_bytes = row_bytes * bmp_h;

  // allocate new buffer, large enough to hold RGBA data
  uint8_t* pixels = reinterpret_cast<uint8_t*>(Om_alloc(bmp_w * bmp_h * 4));
  if(!pixels) return nullptr;

  // we need to specify compression value or GetDIBits return garbage
  bmInfo.bmiHeader.biCompression = BI_RGB;
  // set negative height to get scanlines in proper order
  bmInfo.bmiHeader.biHeight = -bmp_h;

  // get bitmap data
  if(0 == GetDIBits(hDc, hBmp, 0, bmp_h, pixels, &bmInfo, DIB_RGB_COLORS)) {
    return nullptr;
  }

  // in-place conversion RGB to RGBA
  if(bmp_c == 3)
    __inplace_rgb_to_rgba(pixels, bmp_w * bmp_h, 0xFF);


  // swap BGRA to RGBA
  __in_place_rb_swap_32(pixels, tot_bytes);

  ReleaseDC(nullptr, hDc);

  *w = bmp_w;
  *h = bmp_h;

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => Om_imgLoadHBmp : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return pixels;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_imgSaveBmp(const OmWString& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return false;

  // open file
  FILE* out_file;

  //if((fp = fopen(Om_toUTF8(out_path).c_str(), "rb")) == nullptr) {
  if((out_file = _wfopen(out_path.c_str(), L"wb")) == nullptr)
    return false;

  bool result = __bmp_write(out_file, in_rgb, in_w, in_h, in_c);

  fclose(out_file);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_imgSaveJpg(const OmWString& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return false;

  // open file
  FILE* out_file;

  //if((fp = fopen(Om_toUTF8(out_path).c_str(), "rb")) == nullptr) {
  if((out_file = _wfopen(out_path.c_str(), L"wb")) == nullptr)
    return false;

  bool result = __jpg_write(out_file, in_rgb, in_w, in_h, in_c, level);

  fclose(out_file);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_imgSavePng(const OmWString& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return false;

  // open file
  FILE* out_file;

  //if((fp = fopen(Om_toUTF8(out_path).c_str(), "rb")) == nullptr) {
  if((out_file = _wfopen(out_path.c_str(), L"wb")) == nullptr)
    return false;

  bool result = __png_write(out_file, in_rgb, in_w, in_h, in_c, level);

  fclose(out_file);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_imgSaveGif(const OmWString& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return false;

  // open file
  FILE* out_file;

  //if((fp = fopen(Om_toUTF8(out_path).c_str(), "rb")) == nullptr) {
  if((out_file = _wfopen(out_path.c_str(), L"wb")) == nullptr)
    return false;

  bool result = __gif_write(out_file, in_rgb, in_w, in_h, in_c);

  fclose(out_file);

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_imgEncodeBmp(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __bmp_encode(out_size, in_rgb, in_w, in_h, in_c);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_imgEncodeJpg(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __jpg_encode(out_size, in_rgb, in_w, in_h, in_c, level);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_imgEncodePng(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __png_encode(out_size, in_rgb, in_w, in_h, in_c, level);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_imgEncodeGif(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{

  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __gif_encode(out_size, in_rgb, in_w, in_h, in_c);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_imgEncodeHbmp(const uint8_t* src_pix, unsigned src_w, unsigned src_h, unsigned src_c)
{
  #ifdef DEBUG
  clock_t t = clock();
  #endif // DEBUG

  // prevent idiot attempts
  if(!src_pix || !src_w || !src_h || !src_c)
    return nullptr;

  // BITMAP with 24 bits pixels data seem to be always interpreted with
  // transparent alpha once supplied to STATIC control, so we always
  // convert to 32 bits pixels data.

  // buffer size
  size_t tot_bytes = src_w * src_h * 4;

  // allocate buffer for 32 bits BMP data
  uint8_t* temp = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!temp) return nullptr;

  if(src_c == 4) {
    //copy RGBA data to temp buffer
    memcpy(temp, src_pix, tot_bytes);
  } else {
    // Convert RGB to RGBA
    temp = __unpack_rgb_to_rgba(src_pix, src_w * src_h, 0xff);
  }

  // swap RGBA to BGRA
  __in_place_rb_swap_32(temp, tot_bytes);

  HBITMAP hBmp = CreateBitmap(src_w, src_h, 1, 32, temp);

  Om_free(temp);

  #ifdef DEBUG
  t = clock() - t;
  std::cout << "DEBUG => Om_imgEncodeHbmp : " << 1000.0 * ((double)t / CLOCKS_PER_SEC) << " ms\n";
  #endif // DEBUG

  return hBmp;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_imgCopyResample(uint8_t* dst_buf, unsigned dst_w, unsigned dst_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, unsigned rec_x, unsigned rec_y, unsigned rec_w, unsigned rec_h)
{
  // copy rectangle in source image to destination
  __copy_resample_lin(dst_buf, dst_w, dst_h, src_pix, src_w, src_h, rec_x, rec_y, rec_w, rec_h);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_imgResample(uint8_t* dst_buf, unsigned dst_w, unsigned dst_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h)
{
  // copy rectangle in source image to destination
  __copy_resample_lin(dst_buf, dst_w, dst_h, src_pix, src_w, src_h, 0, 0, src_w, src_h);
  //__copy_resample_cub(dst_buf, dst_w, dst_h, src_pix, src_w, src_h, 0, 0, src_w, src_h);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_imgDrawCanvas(uint8_t* cv_pix, unsigned cv_w, unsigned cv_h, const uint8_t* src_pix, unsigned src_w, unsigned src_h, uint32_t bck)
{
  // resize image to fit desired size
  if(src_w == cv_w && src_h == cv_h) {
    memcpy(cv_pix, src_pix, src_w * src_h * 4);
  } else {
    __draw_canvas_lin(cv_pix, cv_w, cv_h, src_pix, src_w, src_h, __bytes_swap(bck));
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_imgMakeThumb(unsigned span, OmSizeMode mode, const uint8_t* src_pix, unsigned src_w, unsigned src_h)
{
  // prevent idiot attempts
  if(!span || !src_pix || !src_w || !src_h)
    return nullptr;

  // create output buffer
  uint8_t* thumb = reinterpret_cast<uint8_t*>(Om_alloc(span * span * 4));
  if(!thumb) return nullptr;

  if((src_w == src_h) || (mode == OM_SIZE_FIT)) {
    if(src_w == span && src_h == span) {
      memcpy(thumb, src_pix, src_w * src_h * 4);
    } else {

      // resize image to fit desired size
      bool mag;

      // determins whether we need to downsample or upsample source image
      if((static_cast<float>(src_w) / src_h) > 1.0f) {
        mag = span > src_w;
      } else {
        mag = span > src_h;
      }

      if(mag) {
        // bicubic interpolation
        __draw_canvas_cub(thumb, span, span, src_pix, src_w, src_h, 0x0);
      } else {
        // average box filter
        __draw_canvas_box(thumb, span, span, src_pix, src_w, src_h, 0x0);
      }
    }

  } else {
    // calculate rectangle in source image to get the proper image portion to
    // resize according the specified mode
    unsigned rec_x, rec_y, rec_w, rec_h;
    // we get a "zoom" rectangle in the source so the resulting thumbnail
    // will fill the entire square
    if(src_w > src_h) {
      rec_x = (src_w * 0.5f) - (src_h * 0.5f);
      rec_y = 0;
      rec_w = rec_h = src_h;
    } else {
      rec_x = 0;
      rec_y = (src_h * 0.5f) - (src_w * 0.5f);
      rec_w = rec_h = src_w;
    }

    // resize image to fit desired size
    if(rec_w == span && rec_h == span) {

      size_t row_bytes = (rec_w * 4); //< assuming RGBA data
      size_t row_shift = (rec_x * 4); //< assuming RGBA data

      const uint8_t* sp;
      uint8_t* dp;

      for(unsigned y = 0; y < span; ++y) {
        sp = src_pix + ((y + rec_y) * row_bytes) + row_shift;
        dp = thumb + (y * row_bytes);
        for(unsigned x = 0; x < span; ++x, dp += 4, sp += 4) {
          __cpy_pixel_32(dp, sp);
        }
      }
    } else {

      if(span > rec_w || span > rec_h) {
        // bicubic interpolation
        __copy_resample_cub(thumb, span, span, src_pix, src_w, src_h, rec_x, rec_y, rec_w, rec_h);
      } else {
        // average box filter
        __copy_resample_box(thumb, span, span, src_pix, src_w, src_h, rec_x, rec_y, rec_w, rec_h);
      }

    }
  }


  return thumb;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_imgRbSwap(uint8_t* src_pix, unsigned src_w, unsigned src_h, unsigned src_c)
{
  if(src_c > 3) {
    __in_place_rb_swap_32(src_pix, src_w * src_h * 4);
  } else {
    __in_place_rb_swap_24(src_pix, src_w * src_h * 3);
  }
}
