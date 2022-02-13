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
#include "OmBase.h"           //< string, vector, Om_alloc, OMM_MAX_PATH, etc.
#include <algorithm>          //< std::sort
#include <cmath>              //< modf, floor, etc.

#include "OmBaseWin.h"        //< WinAPI

#include "OmUtilImg.h"   //< OMM_IMAGE_TYPE_*

#include "jpeg/jpeglib.h"
#include "png/png.h"
#include "gif/gif_lib.h"
//#include "3rdP/gif/quantize.c"

/* we make sure structures are packed to be properly aligned with
 read buffer */
#pragma pack(1)
/// \brief BMP info header
///
/// Structure for BMP file info header
struct OMM_BITMAPINFOHEADER {
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
struct OMM_BITMAPHEADER {
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
  if(0 == memcmp(buff, __sign_bmp, 2)) return OMM_IMAGE_TYPE_BMP;
  // Test JPG signature
  if(0 == memcmp(buff, __sign_jpg, 3)) return OMM_IMAGE_TYPE_JPG;
  // Test PNG signature
  if(0 == memcmp(buff, __sign_png, 8)) return OMM_IMAGE_TYPE_PNG;
  // Test GIF signature
  if(0 == memcmp(buff, __sign_gif, 6)) return OMM_IMAGE_TYPE_GIF;

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
  vector<__qz_rgb*> sort_list;
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
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  gif_dec : GIF decoder structure pointer.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB image data or nullptr if failed.
///
static uint8_t* __gif_decode_common(unsigned* out_w, unsigned* out_h, unsigned* out_c, void* gif_dec, bool flip_y)
{
  // Retrieve GIF decoder struct
  int error;
  GifFileType* gif = reinterpret_cast<GifFileType*>(gif_dec);

  // Load GIF content
  if(DGifSlurp(gif) == GIF_ERROR) {
    DGifCloseFile(gif, &error);
    return nullptr;
  }

  // Get image list, we care only about the first one
  SavedImage* images = gif->SavedImages;

  // Get image informations
  unsigned w = images[0].ImageDesc.Width;
  unsigned h = images[0].ImageDesc.Height;

  // Pointer to color table
  ColorMapObject* table;

  // check whether we got a local color table (may never happen)
  if(images[0].ImageDesc.ColorMap) {
    table = images[0].ImageDesc.ColorMap;
  } else {
    table = gif->SColorMap;
  }

  // define some useful sizes
  size_t row_bytes = w * 3;
  size_t tot_bytes = h * row_bytes;

  // allocate new buffer for RGB data
  uint8_t* rgb = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!rgb) {
    DGifCloseFile(gif, &error);
    return nullptr;
  }

  // get GIF index list
  uint8_t* sp = static_cast<uint8_t*>(images[0].RasterBits);

  // destination pointer
  uint8_t* dp;

  // here we go to translate indexed color to RGB
  for(unsigned y = 0; y < h; ++y) {

    dp = (flip_y) ? rgb + (row_bytes * ((h -1) - y)) : rgb + (y * row_bytes);

    for(unsigned x = 0; x < w; ++x) {
      dp[0] = table->Colors[*sp].Red;
      dp[1] = table->Colors[*sp].Green;
      dp[2] = table->Colors[*sp].Blue;
      dp += 3; sp++;
    }
  }

  // free decoder
  DGifCloseFile(gif, &error);

  // assign output values
  (*out_w) = w; (*out_h) = h; (*out_c) = 3;

  return rgb;
}

/// \brief Encode GIF.
///
/// Common function to encode GIF using the given GIF encoder structure.
///
/// \param[in]  gif_enc : GIF encoder structure pointer.
/// \param[in]  in_rgb  : Input image RGB(A) data to encode.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count.
///
/// \return True if operation succeed, false otherwise
///
static bool __gif_encode_common(void* gif_enc, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // Retrieve GIF encoder struct
  int error;
  GifFileType* gif = reinterpret_cast<GifFileType*>(gif_enc);

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
  gif->SColorMap = GifMakeMapObject(cmap_size, reinterpret_cast<GifColorType*>(cmap)); //< global color table

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

  return true;
}

/// \brief Read GIF file.
///
/// Read GIF data from file pointer.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_file : Input file pointer to read data from.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB image data or nullptr if failed.
///
static uint8_t* __gif_read(unsigned* out_w, unsigned* out_h, unsigned* out_c, FILE* in_file, bool flip_y)
{
  int error;
  GifFileType* gif;

  // make sure we start at beginning
  fseek(in_file, 0, SEEK_SET);

  // Define custom read function and load GIF header
  gif = DGifOpen(in_file, __gif_read_file_fn, &error);
  if(gif == nullptr)
    return nullptr;

  // Decode GIF data
  return __gif_decode_common(out_w, out_h, out_c, gif, flip_y);
}

/// \brief Decode GIF data.
///
/// Decode GIF data from buffer in memory.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_data : Input GIF data to decode.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB image data or nullptr if failed.
///
static uint8_t* __gif_decode(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, bool flip_y)
{
  int error;
  GifFileType* gif;

  // custom read structure
  __gif_read_st read_st;
  read_st.src_data = in_data;
  read_st.src_seek = 0;

  // Define custom read function and load GIF header
  gif = DGifOpen(&read_st, __gif_read_buff_fn, &error);
  if(gif == nullptr)
    return nullptr;

  // Decode GIF data
  return __gif_decode_common(out_w, out_h, out_c, gif, flip_y);
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

  return write_st.dst_data;
}

/// \brief Read BMP file.
///
/// Read BMP data from file pointer.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_file : Input file pointer to read data from.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB(A) image data or nullptr if failed.
///
static uint8_t* __bmp_read(unsigned* out_w, unsigned* out_h, unsigned* out_c, FILE* in_file, bool flip_y)
{
  // make sure we start at begining
  fseek(in_file, 0, SEEK_SET);

  // BMP headers structures
  OMM_BITMAPHEADER bmp_head;
  OMM_BITMAPINFOHEADER bmp_info;
  // read base header
  if(fread(&bmp_head, 1, 14, in_file) < 14)
    return nullptr;
  // check BM signature
  if(0 != memcmp(bmp_head.signature, "BM", 2))
    return nullptr;
  // read info header
  if(fread(&bmp_info, 1, 40, in_file) < 40)
    return nullptr;

  // we support only 24 or 32 bpp
  if(bmp_info.bpp < 24)
    return nullptr;
  // get BMP image parameters
  unsigned w = bmp_info.width;
  unsigned h = bmp_info.height;
  unsigned c = bmp_info.bpp / 8; // channel count

  // define some useful sizes
  size_t row_bytes = w * c;
  size_t tot_bytes = h * row_bytes;

  // allocate new buffer to receive rgb data
  uint8_t* rgb = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!rgb) return nullptr;

  // seek to bitmap data location and read
  fseek(in_file, bmp_head.offbits, SEEK_SET);

  // BMP data is natively stored upside down
  if(flip_y) {
    // read all data at once from
    if(fread(rgb, 1, tot_bytes, in_file) != tot_bytes) {
      Om_free(rgb); return nullptr;
    }
  } else {
    // read rows in reverse order
    unsigned hmax = (h - 1);
    for(unsigned y = 0; y < h; ++y) {
      if(fread(rgb + (row_bytes * (hmax - y)), 1, row_bytes, in_file) != row_bytes) {
        Om_free(rgb); return nullptr;
      }
    }
  }

  // finally swap components order BGR to RGB
  for(unsigned i = 0; i < tot_bytes; i += c)
    rgb[i  ] ^= rgb[i+2] ^= rgb[i  ] ^= rgb[i+2]; //< BGR => RGB

  // assign output values
  (*out_w) = w; (*out_h) = h; (*out_c) = c;

  return rgb;
}

/// \brief Decode BMP data.
///
/// Decode BMP data from buffer in memory.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_data : Input BMP data to decode.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB(A) image data or nullptr if failed.
///
static uint8_t* __bmp_decode(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, bool flip_y)
{
  // pointer to input data
  const uint8_t* in_ptr = in_data;

  // BMP headers structures
  OMM_BITMAPHEADER bmp_head;
  OMM_BITMAPINFOHEADER bmp_info;
  // get base header
  memcpy(&bmp_head, in_ptr, 14); in_ptr += 14;
  // check BM signature
  if(0 != memcmp(bmp_head.signature, "BM", 2))
    return nullptr;
  // get info header
  memcpy(&bmp_info, in_ptr, 40); in_ptr += 40;

  // we support only 24 or 32 bpp
  if(bmp_info.bpp < 24)
    return nullptr;

  // get BMP image parameters
  unsigned w = bmp_info.width;
  unsigned h = bmp_info.height;
  unsigned c = bmp_info.bpp / 8; // channel count

  // define some useful sizes
  size_t row_bytes = w * c;
  size_t tot_bytes = h * row_bytes;

  // allocate new buffer to receive rgb data
  uint8_t* rgb = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!rgb) return nullptr;

  // seek to bitmap data location
  in_ptr = in_data + bmp_head.offbits;

  // BMP data is natively stored upside down
  if(flip_y) {
    // read all data at once from
    memcpy(rgb, in_ptr, tot_bytes); in_ptr += tot_bytes;
  } else {
    // read rows in reverse order
    unsigned hmax = (h - 1);
    for(unsigned y = 0; y < h; ++y) {
      memcpy(rgb + (row_bytes * (hmax - y)), in_ptr, row_bytes);
      in_ptr += row_bytes;
    }
  }

  // finally swap components order BGR to RGB
  for(unsigned i = 0; i < tot_bytes; i += c)
    rgb[i  ] ^= rgb[i+2] ^= rgb[i  ] ^= rgb[i+2]; //< BGR => RGB

  // assign output values
  (*out_w) = w; (*out_h) = h; (*out_c) = c;

  return rgb;
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
  // compute data sizes
  size_t hdr_bytes = sizeof(OMM_BITMAPHEADER) + sizeof(OMM_BITMAPINFOHEADER);
  size_t row_bytes = in_w * in_c;                   //< row size in bytes
  size_t r4b_bytes = row_bytes + (row_bytes % 4);   //< row size rounded up to a multiple of 4 bytes
  size_t tot_bytes = r4b_bytes * in_h;
  size_t bmp_bytes = tot_bytes + hdr_bytes;

  // BMP headers structure
  OMM_BITMAPHEADER bmp_head = {};
  bmp_head.signature[0] = 0x42;
  bmp_head.signature[1] = 0x4D; // BM signature
  bmp_head.offbits = 54; // file header + info header = 54 bytes
  bmp_head.size = bmp_bytes;

  OMM_BITMAPINFOHEADER bmp_info = {};
  bmp_info.size = sizeof(OMM_BITMAPINFOHEADER);
  bmp_info.width = in_w;
  bmp_info.height = in_h;
  bmp_info.planes = 1;
  bmp_info.bpp = in_c * 8;
  bmp_info.compression = 0;
  bmp_info.sizeimage = tot_bytes;
  bmp_info.xppm = bmp_info.yppm = 0x0ec4;

  // make sure we start at begining
  fseek(out_file, 0, SEEK_SET);

  // write file header
  if(fwrite(&bmp_head, 1, sizeof(OMM_BITMAPHEADER), out_file) != sizeof(OMM_BITMAPHEADER))
    return false;
  // write info header
  if(fwrite(&bmp_info, 1, sizeof(OMM_BITMAPINFOHEADER), out_file) != sizeof(OMM_BITMAPINFOHEADER))
    return false;

  // allocate buffer for data translation
  uint8_t* row = reinterpret_cast<uint8_t*>(Om_alloc(r4b_bytes));
  if(!row) return false;

  // useful values for translation
  const uint8_t* sp;
  uint8_t* dp;
  unsigned hmax = (in_h - 1);

  for(unsigned y = 0; y < in_h; ++y) {
    sp = in_rgb + (row_bytes * (hmax - y)); // reverse row up to bottom
    dp = row;
    for(unsigned x = 0; x < in_w; ++x) {
      // convert RGBA to BGRA
      dp[0] = sp[2];
      dp[1] = sp[1];
      dp[2] = sp[0];
      if(in_c == 4) dp[3] = sp[3];
      sp += in_c;
      dp += in_c;
    }
    // write row to file
    if(fwrite(row, 1, r4b_bytes, out_file) != r4b_bytes) {
      Om_free(row); return false;
    }
  }

  Om_free(row);

  return true;
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
  // compute data sizes
  size_t hdr_bytes = sizeof(OMM_BITMAPHEADER) + sizeof(OMM_BITMAPINFOHEADER);
  size_t row_bytes = in_w * in_c;                   //< row size in bytes
  size_t r4b_bytes = row_bytes + (row_bytes % 4);   //< row size rounded up to a multiple of 4 bytes
  size_t tot_bytes = r4b_bytes * in_h;
  size_t bmp_bytes = tot_bytes + hdr_bytes;

  // BMP headers structure
  OMM_BITMAPHEADER bmp_head = {};
  bmp_head.signature[0] = 0x42; bmp_head.signature[1] = 0x4D; // BM signature
  bmp_head.offbits = hdr_bytes; // file header + info header = 54 bytes
  bmp_head.size = bmp_bytes;

  OMM_BITMAPINFOHEADER bmp_info = {};
  bmp_info.size = sizeof(OMM_BITMAPINFOHEADER);
  bmp_info.width = in_w;
  bmp_info.height = in_h;
  bmp_info.planes = 1;
  bmp_info.bpp = in_c * 8;
  bmp_info.compression = 0;
  bmp_info.sizeimage = tot_bytes;
  bmp_info.xppm = bmp_info.yppm = 0x0ec4;

  // allocate buffer for BMP data
  uint8_t* bmp = reinterpret_cast<uint8_t*>(Om_alloc(bmp_bytes));
  if(!bmp) return nullptr;

  // keep pointer to buffer
  uint8_t* bmp_ptr = bmp;

  // write file header
  memcpy(bmp_ptr, &bmp_head, sizeof(OMM_BITMAPHEADER));
  bmp_ptr += sizeof(OMM_BITMAPHEADER);
  // write info header
  memcpy(bmp_ptr, &bmp_info, sizeof(OMM_BITMAPINFOHEADER));
  bmp_ptr += sizeof(OMM_BITMAPINFOHEADER);

  // allocate buffer for data translation
  uint8_t* row = reinterpret_cast<uint8_t*>(Om_alloc(r4b_bytes));
  if(!row) {
    Om_free(bmp);
    return nullptr;
  }

  // useful values for translation
  const uint8_t* sp;
  uint8_t* dp;
  unsigned hmax = (in_h - 1);

  for(unsigned y = 0; y < in_h; ++y) {
    sp = in_rgb + (row_bytes * (hmax - y)); // reverse row up to bottom
    dp = row;
    for(unsigned x = 0; x < in_w; ++x) {
      // convert RGBA to BGRA
      dp[0] = sp[2];
      dp[1] = sp[1];
      dp[2] = sp[0];
      if(in_c == 4) dp[3] = sp[3];
      sp += in_c;
      dp += in_c;
    }

    // write row to buffer
    memcpy(bmp_ptr, row, r4b_bytes);
    bmp_ptr += r4b_bytes;
  }

  Om_free(row);

  (*out_size) = bmp_bytes;

  return bmp;
}

/// \brief Decode JPEG.
///
/// Common function to decode JPEG using the given GIF decoder structure.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  jpg_dec : JPEG decoder structure pointer.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB image data or nullptr if failed.
///
static uint8_t* __jpg_decode_common(unsigned* out_w, unsigned* out_h, unsigned* out_c, void* jpg_dec, bool flip_y)
{
  jpeg_decompress_struct* jpg = reinterpret_cast<jpeg_decompress_struct*>(jpg_dec);

  // read jpeg header
  if(jpeg_read_header(jpg, true) != 1)
    return nullptr;

  // initialize decompression
  jpeg_start_decompress(jpg);

  // get image parameters
	unsigned w = jpg->output_width;
	unsigned h = jpg->output_height;
	unsigned c = jpg->output_components;

	// define sizes
  size_t row_bytes = w * c;
  size_t tot_bytes = row_bytes * h;

  // allocate buffer to receive RGB data
  uint8_t* rgb = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!rgb) return nullptr;

  // row list pointer for jpeg decoder
  uint8_t* row_p[1];

  if(flip_y) {
    unsigned hmax = h - 1;
    while(jpg->output_scanline < jpg->output_height) {
      row_p[0] = rgb + ((hmax - jpg->output_scanline) * row_bytes);
      jpeg_read_scanlines(jpg, row_p, 1); //< read one row (scanline)
    }
  } else {
    while(jpg->output_scanline < jpg->output_height) {
      row_p[0] = rgb + (jpg->output_scanline * row_bytes);
      jpeg_read_scanlines(jpg, row_p, 1); //< read one row (scanline)
    }
  }

	// finalize reading
	jpeg_finish_decompress(jpg);

  // cleanup decoder
	jpeg_destroy_decompress(jpg);

	// assign output values
	(*out_w) = w; (*out_h) = h; (*out_c) = c;

	return rgb;
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

  return true;
}

/// \brief Read JPEG file.
///
/// Read JPEG data from file pointer.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_file : Input file pointer to read data from.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGB data or nullptr if failed.
///
static uint8_t* __jpg_read(unsigned* out_w, unsigned* out_h, unsigned* out_c, FILE* in_file, bool flip_y)
{
  // create base object for jpeg decoder
  jpeg_decompress_struct jpg;
  jpeg_error_mgr jerr;

  // create jpeg decoder
  jpg.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&jpg);

  // make sure we start at beginning and setup jpeg IO
  fseek(in_file, 0, SEEK_SET);
  jpeg_stdio_src(&jpg, in_file);

  return __jpg_decode_common(out_w, out_h, out_c, &jpg, flip_y);
}

/// \brief Decode JPEG data.
///
/// Decode JPEG data from buffer in memory.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_data : Input JPEG data to decode.
/// \param[in]  in_size : Input JPEG data size in bytes.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGB data or nullptr if failed.
///
static uint8_t* __jpg_decode(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, size_t in_size, bool flip_y)
{
  // create base object for jpeg decoder
  jpeg_decompress_struct jpg;
  jpeg_error_mgr jerr;

  // create jpeg decoder
  jpg.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&jpg);

  // set read data pointer
  jpeg_mem_src(&jpg, in_data, in_size);

  return __jpg_decode_common(out_w, out_h, out_c, &jpg, flip_y);
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
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  png_dec : PNG decoder structure pointer.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded RGB(A) image data or nullptr if failed.
///
static uint8_t* __png_decode_common(unsigned* out_w, unsigned* out_h, unsigned* out_c, void* png_dec, bool flip_y)
{
  // get decoder
  png_structp png = reinterpret_cast<png_structp>(png_dec);

  // create PNG info structure
  png_infop png_info = png_create_info_struct(png);

  // get image properties
  png_read_info(png, png_info);
  unsigned w = png_get_image_width(png, png_info);
  unsigned h = png_get_image_height(png, png_info);
  unsigned c = png_get_channels(png, png_info);

  // retrieve and define useful sizes
  size_t row_bytes = png_get_rowbytes(png, png_info);
  size_t tot_bytes = h * row_bytes;

  // allocate pointer to receive RGB(A) data
  uint8_t* rgb = reinterpret_cast<uint8_t*>(Om_alloc(tot_bytes));
  if(!rgb) return nullptr;

  // allocate list of pointers for output RGB(A) rows
  uint8_t** rows_p = reinterpret_cast<uint8_t**>(Om_alloc(sizeof(void*)*h));
  if(!rows_p) {
    Om_free(rgb);
    return nullptr;
  }

  // define pointers to each row in output RGB(A) data
  if(flip_y) {
    unsigned hmax = h - 1;
    for(unsigned y = 0; y < h; ++y)
      rows_p[y] = rgb + ((hmax - y) * row_bytes);
  } else {
    for(unsigned y = 0; y < h; ++y)
      rows_p[y] = rgb + (y * row_bytes);
  }

  // read all rows at once
  png_read_image(png, rows_p);

  // cleanup
  png_destroy_read_struct(&png, &png_info, nullptr);

  // delete list of pointers
  Om_free(rows_p);

  // assign output values
  (*out_w) = w; (*out_h) = h; (*out_c) = c;

  return rgb;
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
static bool __png_encode_common(void* png_enc, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // get encoder
  png_structp png = reinterpret_cast<png_structp>(png_enc);

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

  // allocate list of pointers for input RGB(A) rows
  const uint8_t** rows_p = reinterpret_cast<const uint8_t**>(Om_alloc(sizeof(void*)*in_h));
  if(!rows_p) {
    png_destroy_write_struct(&png, &png_info);
    png_free_data(png, png_info, PNG_FREE_ALL, -1);
    return false;
  }

  // define pointers to each row in input RGB(A) data
  for(unsigned y = 0; y < in_h; ++y)
    rows_p[y] = in_rgb + (y * row_bytes);

  // supply all data at once to encoder
  png_write_image(png, const_cast<uint8_t**>(rows_p));

  // finalize write process
  png_write_end(png, nullptr);

  // clear PGN encoder
  png_destroy_write_struct(&png, &png_info);
  png_free_data(png, png_info, PNG_FREE_ALL, -1);

  // delete list of pointers
  Om_free(rows_p);

  return true;
}

/// \brief Read PNG file.
///
/// Read PNG data from file pointer.
///
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_file : Input file pointer to read data from.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGB(A) data or nullptr if failed.
///
static uint8_t* __png_read(unsigned* out_w, unsigned* out_h, unsigned* out_c, FILE* in_file, bool flip_y)
{
  // create PNG decoder structure
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // make sure we start at beginning and setup png IO
  fseek(in_file, 0, SEEK_SET);
  png_init_io(png, in_file);

  // decode PNG data
  return __png_decode_common(out_w, out_h, out_c, png, flip_y);
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
/// \param[out] out_w   : Output image width
/// \param[out] out_h   : Output image height
/// \param[out] out_c   : Output image color component count.
/// \param[in]  in_data : Input PNG data to decode.
/// \param[in]  flip_y  : Load image for bottom-left origin usage (upside down)
///
/// \return Pointer to decoded image RGB(A) data or nullptr if failed.
///
static uint8_t* __png_decode(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, bool flip_y)
{
  // create PNG decoder structure
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // custom read structure
  __png_read_st read_st;
  read_st.src_data = in_data;
  read_st.src_seek = 0;

  // set custom read process
  png_set_read_fn(png, &read_st, __png_read_buff_fn);

  // decode PNG data
  return __png_decode_common(out_w, out_h, out_c, png, flip_y);
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

  return write_st.dst_data;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_imageType(uint8_t* data)
{
  // identify image format
  return __image_sign_matches(data);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int Om_imageType(FILE* file)
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
uint8_t* Om_loadImage(unsigned* out_w, unsigned* out_h, unsigned* out_c, const wstring& in_path, bool flip_y)
{
  // initialize output values
  (*out_w) = 0;
  (*out_h) = 0;
  (*out_c) = 0;

  // Open file for writing
  FILE* in_file;
  if((in_file = _wfopen(in_path.c_str(), L"rb")) == nullptr)
    return nullptr;

  // read first 8 bytes of the file
  uint8_t buff[8];
  fseek(in_file, 0, SEEK_SET);
  if(fread(buff, 1, 8, in_file) < 8)
    return nullptr;

  // identify image format
  int type = __image_sign_matches(buff);

  // load according image format
  if(type != 0) {
    switch(type)
    {
    case OMM_IMAGE_TYPE_BMP:
      return __bmp_read(out_w, out_h, out_c, in_file, flip_y);
    case OMM_IMAGE_TYPE_JPG:
      return __jpg_read(out_w, out_h, out_c, in_file, flip_y);
    case OMM_IMAGE_TYPE_PNG:
      return __png_read(out_w, out_h, out_c, in_file, flip_y);
    case OMM_IMAGE_TYPE_GIF:
      return __gif_read(out_w, out_h, out_c, in_file, flip_y);
    }
  }

  fclose(in_file);

  return nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_loadImage(unsigned* out_w, unsigned* out_h, unsigned* out_c, const uint8_t* in_data, size_t in_size, bool flip_y)
{
  // initialize output values
  (*out_w) = 0;
  (*out_h) = 0;
  (*out_c) = 0;

  // prevent idiot attempts
  if(!in_data || !in_size)
    return nullptr;

  // identify image format
  int type = __image_sign_matches(in_data);

  // load according image format
  if(type != 0) {
    switch(type)
    {
    case OMM_IMAGE_TYPE_BMP:
      return __bmp_decode(out_w, out_h, out_c, in_data, flip_y);
    case OMM_IMAGE_TYPE_JPG:
      return __jpg_decode(out_w, out_h, out_c, in_data, in_size, flip_y);
    case OMM_IMAGE_TYPE_PNG:
      return __png_decode(out_w, out_h, out_c, in_data, flip_y);
    case OMM_IMAGE_TYPE_GIF:
      return __gif_decode(out_w, out_h, out_c, in_data, flip_y);
    }
  }

  return nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_saveBmp(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
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
bool Om_saveJpg(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
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
bool Om_savePng(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
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
bool Om_saveGif(const wstring& out_path, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
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
uint8_t* Om_encodeBmp(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __bmp_encode(out_size, in_rgb, in_w, in_h, in_c);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_encodeJpg(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __jpg_encode(out_size, in_rgb, in_w, in_h, in_c, level);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_encodePng(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c, int level)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __png_encode(out_size, in_rgb, in_w, in_h, in_c, level);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_encodeGif(size_t* out_size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  return __gif_encode(out_size, in_rgb, in_w, in_h, in_c);
}


/// \brief Compute cubic interpolation.
///
/// Compute cubic the interpolation of the specified values.
///
/// \param[in]  a : Start handle
/// \param[in]  b : Start node
/// \param[in]  c : End node
/// \param[in]  d : End handle
/// \param[in]  t : Interpolation phase
///
/// \return cubic interpolated value
///
static inline float __interp_cubic(float a, float b, float c, float d, float t)
{
  return b + 0.5f * t * (c - a + t * (2.0f * a - 5.0f * b + 4.0f * c - d + t * (3.0f * (b - c) + d - a)));
}

/// \brief Get BiCubic interpolated pixel.
///
/// Compute the BiCubic interpolated pixel at the specified coordinates.
///
/// \param[in]  out_pix : Array to receive interpolated pixel components.
/// \param[in]  u       : Sample horizontal coordinate in image, from 0.0 to 1.0.
/// \param[in]  v       : Sample vertical coordinate in image, from 0.0 to 1.0.
/// \param[in]  in_rgb  : Input image RGB(A) data.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count (bytes per pixel).
///
static inline void __image_sample_bicubic(uint8_t* out_pix, float u, float v, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  float x, y;

  float xf = modf((u * in_w) - 0.5f, &x);
  float yf = modf((v * in_h) - 0.5f, &y);

  int ix = static_cast<int>(x) - 1;
  int iy = static_cast<int>(y) - 1;

  const uint8_t* sp[4];

  float r[4]; float g[4]; float b[4]; float a[4];

  int x_b, y_b;
  int hmax = in_h - 1;
  int wmax = in_w - 1;

  for(int j = 0; j < 4; ++j) {
    y_b = (std::max(0, std::min(iy + j, hmax)) * in_w) * in_c;
    for(int i = 0; i < 4; ++i) {
      x_b = std::max(0, std::min(ix + i, wmax)) * in_c;
      sp[i] = in_rgb + (y_b + x_b);
    }
    r[j] = __interp_cubic(sp[0][0], sp[1][0], sp[2][0], sp[3][0], xf);
    g[j] = __interp_cubic(sp[0][1], sp[1][1], sp[2][1], sp[3][1], xf);
    b[j] = __interp_cubic(sp[0][2], sp[1][2], sp[2][2], sp[3][2], xf);
    if(in_c == 4) a[j] = __interp_cubic(sp[0][3], sp[1][3], sp[2][3], sp[3][3], xf);
  }

  float m[4];
  m[0] = __interp_cubic(r[0], r[1], r[2], r[3], yf);
  m[1] = __interp_cubic(g[0], g[1], g[2], g[3], yf);
  m[2] = __interp_cubic(b[0], b[1], b[2], b[3], yf);
  if(in_c == 4) m[3] = __interp_cubic(a[0], a[1], a[2], a[3], yf);

  for(unsigned i = 0; i < in_c; ++i) {
    out_pix[i] = static_cast<uint8_t>(std::max(0.0f, std::min(m[i], 255.0f)));
  }
}

/// \brief Get box interpolated pixel.
///
/// Compute the box interpolated pixel at the specified coordinates.
///
/// \param[in]  out_pix : Array to receive interpolated pixel components.
/// \param[in]  box_w   : Box width in pixels.
/// \param[in]  box_h   : Box height in pixels.
/// \param[in]  u       : Sample horizontal coordinate in image, from 0.0 to 1.0.
/// \param[in]  v       : Sample vertical coordinate in image, from 0.0 to 1.0.
/// \param[in]  in_rgb  : Input image RGB(A) data.
/// \param[in]  in_w    : Input image width.
/// \param[in]  in_h    : Input image height.
/// \param[in]  in_c    : Input image color component count (bytes per pixel).
///
static inline void __image_sample_box(uint8_t* out_pix, int box_w, int box_h, float u, float v, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  float r = 0.0f; float g = 0.0f; float b = 0.0f; float a = 0.0f;

  const uint8_t *sp;

  int x_b, y_b;
  int wmax = (in_w - 1);
  int hmax = (in_h - 1);

  int x = u * wmax;
  int y = v * hmax;

  for(int j = 0; j < box_h; ++j) {
    y_b = (std::max(0, std::min(y + j, hmax)) * in_w) * in_c;
    for(int i = 0; i < box_w; ++i) {
      x_b = std::max(0, std::min(x + i, wmax)) * in_c;
      sp = in_rgb + (y_b + x_b);
      r += sp[0]; g += sp[1]; b += sp[2];
      if(in_c == 4) a += sp[3];
    }
  }

  float f = box_h * box_w;

  out_pix[0] = r / f; out_pix[1] = g / f; out_pix[2] = b / f;
  if(in_c == 4) out_pix[3] = a / f;
}

/// \brief Box filter downsample image.
///
/// Reduce image resolution using box filtering.
///
/// \param[out] out_rgb : Output destination pointer.
/// \param[in]  w       : target width.
/// \param[in]  w       : target height.
/// \param[in]  in_rgb  : Source image RGB(A) data.
/// \param[in]  in_w    : Source image width.
/// \param[in]  in_h    : source image height.
/// \param[in]  in_c    : Source image component count (bytes per pixel)
///
static void __image_dsample(uint8_t* out_rgb, unsigned w, unsigned h, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  unsigned box_w = floor(static_cast<float>(in_w) / w);
  unsigned box_h = floor(static_cast<float>(in_h) / h);

  float inv_w = 1.0f / (static_cast<float>(w) - 1);
  float inv_h = 1.0f / (static_cast<float>(h) - 1);

  uint8_t smpl[4];

  uint8_t* dp;
  float u, v;

  for(unsigned y = 0; y < h; ++y) {
    dp = out_rgb + ((w * in_c) * y);
    v = y * inv_h;
    for(unsigned x = 0; x < w; ++x) {
      u = x * inv_w;
      __image_sample_box(smpl, box_w, box_h, u, v, in_rgb, in_w, in_h, in_c);
      dp[0] = smpl[0]; dp[1] = smpl[1]; dp[2] = smpl[2];
      if(in_c == 4) dp[3] = smpl[3];
      dp += in_c;
    }
  }
}

/// \brief Bicubic filter upsample image
///
/// Increase image resolution using bicubic filtering.
///
/// \param[out] out_rgb : Output destination pointer.
/// \param[in]  w       : target width.
/// \param[in]  w       : target height.
/// \param[in]  in_rgb  : Source image RGB(A) data.
/// \param[in]  in_w    : Source image width.
/// \param[in]  in_h    : source image height.
/// \param[in]  in_c    : Source image component count (bytes per pixel)
///
static void __image_usample(uint8_t* out_rgb, unsigned w, unsigned h, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  float inv_w = 1.0f / (static_cast<float>(w) - 1);
  float inv_h = 1.0f / (static_cast<float>(h) - 1);

  uint8_t smpl[4];

  uint8_t* dp;
  float u, v;

  for(unsigned y = 0; y < h; ++y) {
    dp = out_rgb + ((w * in_c) * y);
    v = y * inv_h;
    for(unsigned x = 0; x < w; ++x) {
      u = x * inv_w;
      __image_sample_bicubic(smpl, u, v, in_rgb, in_w, in_h, in_c);
      dp[0] = smpl[0]; dp[1] = smpl[1]; dp[2] = smpl[2];
      if(in_c == 4) dp[3] = smpl[3];
      dp += in_c;
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_resizeImage(unsigned w, unsigned h, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!w  || !h || !in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  // define useful sizes
  size_t out_row_bytes = w * in_c;

  uint8_t* out_rgb = reinterpret_cast<uint8_t*>(Om_alloc(out_row_bytes * h));
  if(!out_rgb) return nullptr;

  // resize image to fit desired square
  if(in_w != w || in_h != h) {

    if(w > in_w || h > in_h) {
      __image_usample(out_rgb, w, h, in_rgb, in_w, in_h, in_c);
    } else {
      __image_dsample(out_rgb, w, h, in_rgb, in_w, in_h, in_c);
    }

  } else {

    memcpy(out_rgb, in_rgb, (in_w * in_c) * in_h);
  }

  return out_rgb;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_cropImage(unsigned x, unsigned y, unsigned w, unsigned h, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!w  || !h || !in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  if((x + w) > in_w || (y + h) > in_h)
    return nullptr;

  // define useful sizes
  size_t in_row_bytes = in_w * in_c;
  size_t out_row_bytes = w * in_c;

  // allocate new buffer for cropped data
  uint8_t* out_rgb = reinterpret_cast<uint8_t*>(Om_alloc(out_row_bytes * h));
  if(!out_rgb) return nullptr;

  // copy required RGB data
  const uint8_t* sp;
  uint8_t* dp;

  unsigned x_shift = (x * in_c);

  for(unsigned j = 0; j < h; ++j) {

    dp = out_rgb + (out_row_bytes * j);
    sp = in_rgb + ((in_row_bytes * (j + y)) + x_shift);

    for(unsigned i = 0; i < w; ++i) {

      dp[0] = sp[0]; dp[1] = sp[1]; dp[2] = sp[2];
      if(in_c == 4) dp[3] = sp[3];

      dp += in_c; sp += in_c;
    }
  }

  return out_rgb;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint8_t* Om_thumbnailImage(unsigned size, const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!size || !in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  // create locale copy of original data
  uint8_t* out_rgb = reinterpret_cast<uint8_t*>(Om_alloc((in_w * in_c) * in_h));
  if(!out_rgb) return nullptr;

  memcpy(out_rgb, in_rgb, (in_w * in_c) * in_h);

  // resize image to fit desired size
  if(in_w != size || in_h != size) {

    unsigned out_w, out_h;
    // determine target size according image aspect ratio
    float a = static_cast<float>(in_w) / in_h;
    if(a > 1.0f) {
      out_w = static_cast<float>(size) * a;
      out_h = size;
    } else {
      out_w = size;
      out_h = static_cast<float>(size) / a;
    }

    uint8_t* tmp_rgb = Om_resizeImage(out_w, out_h, out_rgb, in_w, in_h, in_c);
    if(tmp_rgb == nullptr) {
      return nullptr;
    }

    // swap buffers
    Om_free(out_rgb);
    out_rgb = tmp_rgb;

    // update input width and height
    in_h = out_h;
    in_w = out_w;
  }

  // crop image to square
  if(in_w != in_h) {

    unsigned x, y;

    if(in_w > in_h) {
      x = (in_w * 0.5f) - (in_h * 0.5f);
      y = 0;
    } else {
      x = 0;
      y = (in_h * 0.5f) - (in_w * 0.5f);
    }

    uint8_t* tmp_rgb = Om_cropImage(x, y, size, size, out_rgb, in_w, in_h, in_c);
    if(tmp_rgb == nullptr) {
      return nullptr;
    }

    // swap buffers
    Om_free(out_rgb);
    out_rgb = tmp_rgb;
  }

  return out_rgb;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP Om_hbitmapImage(const uint8_t* in_rgb, unsigned in_w, unsigned in_h, unsigned in_c)
{
  // prevent idiot attempts
  if(!in_rgb || !in_w || !in_h || !in_c)
    return nullptr;

  // BITMAP with 24 bits pixels data seem to be always interpreted with
  // transparent alpha once supplied to STATIC control, so we always
  // convert to 32 bits pixels data.

  // allocate buffer for 32 bits BMP data
  uint8_t* bmp = reinterpret_cast<uint8_t*>(Om_alloc((in_w * 4) * in_h));
  if(!bmp) return nullptr;

  if(in_c == 4) {

    // compute data sizes
    size_t tot_bytes = (in_w * in_c) * in_h;

    //copy RGBA data to temp buffer
    memcpy(bmp, in_rgb, tot_bytes);

    // swap pixels components from RGBA to BGRA
    for(unsigned i = 0; i < tot_bytes; i += 4)
      bmp[i  ] ^= bmp[i+2] ^= bmp[i  ] ^= bmp[i+2]; //< RGB => BGR

  } else {

    // compute data sizes
    size_t row3_bytes = in_w * 3;
    size_t row4_bytes = in_w * 4;

    // pointers for translation
    const uint8_t* sp;
    uint8_t* dp;

    for(unsigned y = 0; y < in_h; ++y) {

      sp = in_rgb + (row3_bytes * y);
      dp = bmp + (row4_bytes * y);

      for(unsigned x = 0; x < in_w; ++x) {

        // convert RGBA to BGRA
        dp[0] = sp[2];
        dp[1] = sp[1];
        dp[2] = sp[0];
        dp[3] = 0xFF;
        sp += 3;
        dp += 4;
      }
    }
  }

  HBITMAP hBmp = CreateBitmap(in_w, in_h, 1, 32, bmp);

  Om_free(bmp);

  return hBmp;
}
