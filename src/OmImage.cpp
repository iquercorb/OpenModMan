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
#include "thirdparty/jpeg/jpeglib.h"
#include "thirdparty/png/png.h"
#include "thirdparty/gif/gif_lib.h"
#include "thirdparty/gif/quantize.c"

#define OMM_IMAGE_VALID     1
#define OMM_IMAGE_PENDING   0
#define OMM_IMAGE_ERR_OPEN  -2
#define OMM_IMAGE_ERR_READ  -3
#define OMM_IMAGE_ERR_WRITE  -4
#define OMM_IMAGE_ERR_ALLOC  -5
#define OMM_IMAGE_ERR_DECODE  -6
#define OMM_IMAGE_ERR_ENCODE  -7
#define OMM_IMAGE_ERR_FORMAT  -8
#define OMM_IMAGE_ERR_NODATA  -9

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
  uint32_t  size;           ///< size of the whole .bmp file
  uint16_t  reserved1;      ///< must be 0
  uint16_t  reserved2;      ///< must be 0
  uint32_t  offbits;        ///< where bitmap data begins
}; // 12 bytes
#pragma pack()

/// BMP file specific signature / magic number
static unsigned char __sign_bmp[] = "BM";

/// JPG file specific signature / magic number
static unsigned char __sign_jpg[] = {0xFF, 0xD8, 0xFF};

/// PNG file specific signature / magic number
static unsigned char __sign_png[] = {
  0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

/// GIF file specific signature / magic number
static unsigned char __sign_gif[] = "GIF89a";


/// \brief compare buffer to known images signatures
///
/// Check whether the given buffer matches any known file
/// signature or magic number.
///
/// \param[in]  buff  : Buffer to compare known signatures with
///
/// \return     \c number greater than 0 if buffer matches signature, \c 0 otherwise.
///
inline static OmImageType __sign_matches(const unsigned char* buff)
{
  bool match;

  // Test PNG signature
  match = true;

  for(unsigned i = 0; i < 8; i++) {
    if(buff[i] != __sign_png[i]) {
      match = false; break;
    }
  }

  if(match) return OMM_IMAGE_PNG;

  // Test JPG signature
  match = true;

  for(unsigned i = 0; i < 3; i++) {
    if(buff[i] != __sign_jpg[i]) {
      match = false; break;
    }
  }

  if(match) return OMM_IMAGE_JPG;

  // Test GIF signature
  match = true;

  for(unsigned i = 0; i < 6; i++) {
    if(buff[i] != __sign_gif[i]) {
      match = false; break;
    }
  }

  if(match) return OMM_IMAGE_GIF;

  // Test BMP signature
  match = true;

  for(unsigned i = 0; i < 2; i++) {
    if(buff[i] != __sign_bmp[i]) {
      match = false; break;
    }
  }

  if(match) return OMM_IMAGE_BMP;

  return OMM_IMAGE_UNKNOW;
}

/// \brief Custom GIF reader
///
/// Custom read function for GIF library to read a file pointer.
///
/// \param[in]  gifftp  Decoder structure pointer.
/// \param[in]  buff    Destination buffer.
/// \param[in]  length  Length of data that should be read.
///
/// \return  The number of bytes that were read
///
static int __gif_read_fp_fn(GifFileType* gifftp, uint8_t* buff, int length)
{
  return fread(buff, 1, length, reinterpret_cast<FILE*>(gifftp->UserData));
}

/// \brief Load GIF from file.
///
/// Load GIF data from the specified file pointer.
///
/// \param[out] dest    : Pointer to destination data pointer to be allocated.
/// \param[out] width   : Decoded image width
/// \param[out] height  : Decoded image height
/// \param[out] depth   : Decoded image depth (bits per pixel).
/// \param[in]  flip_y  : Set image for bottom-left origin usage (upside down)
/// \param[in]  fp      : File pointer to read data.
///
/// \return Error code or 1 if operation succeed.
///
static int __gif_decode_from_fp(  uint8_t** dest,
                                  unsigned* width,
                                  unsigned* height,
                                  unsigned* depth,
                                  bool flip_y,
                                  FILE* fp)
{
  GifFileType* gifftp;

  int error;

  // make sure we start at beginning
  fseek(fp, 0, SEEK_SET);

  // Define custom read function and load GIF header
  gifftp = DGifOpen(fp, __gif_read_fp_fn, &error);
  if(gifftp == nullptr) {
    return OMM_IMAGE_ERR_DECODE;
  }
  // Load GIF content
  if(DGifSlurp(gifftp) == GIF_ERROR) {
      DGifCloseFile(gifftp, &error);
      return false;
  }

  // Get image list, we care only about the first one
  SavedImage* images = gifftp->SavedImages;

  // Get image informations
  unsigned w = images[0].ImageDesc.Width;
  unsigned h = images[0].ImageDesc.Height;

  // Pointer to color table
  ColorMapObject* table;

  // check whether we got a local color table (may never happen)
  if(images[0].ImageDesc.ColorMap) {
    table = images[0].ImageDesc.ColorMap;
  } else {
    table = gifftp->SColorMap;
  }

  // allocate new buffer for RGB data
  size_t rowbytes = w * 3;
  size_t totbytes = h * rowbytes;
  uint8_t* buff;
  try {
    buff = new uint8_t[totbytes];
  } catch(const std::bad_alloc&) {
    DGifCloseFile(gifftp, &error);
    return OMM_IMAGE_ERR_ALLOC;
  }

  // get GIF index list
  uint8_t* sp = static_cast<uint8_t*>(images[0].RasterBits);

  // destination pointer
  uint8_t* dp;

  // here we go to translate indexed color to RGB
  for(unsigned y = 0; y < h; ++y) {

    if(flip_y) {
      dp = &buff[rowbytes * ((h -1) - y)];
    } else {
      dp = &buff[y * rowbytes];
    }

    for(unsigned x = 0; x < w; ++x) {
      dp[0] = table->Colors[*sp].Red;
      dp[1] = table->Colors[*sp].Green;
      dp[2] = table->Colors[*sp].Blue;
      dp += 3; sp++;
    }
  }

  // free decoder
  DGifCloseFile(gifftp, &error);

  // assign values
  (*dest) = buff;
  (*width) = w;
  (*height) = h;
  (*depth) = 24;

  return 1;
}

/// \brief Custom GIF reader
///
/// Custom read function for GIF library to read memory buffer.
///
/// \param[in]  gifftp  Decoder structure pointer.
/// \param[in]  dest    Destination buffer.
/// \param[in]  length  Length of data that should be read.
///
/// \return  The number of bytes that were read
///
static int __gif_read_mem_fn(GifFileType* gifftp, uint8_t* dest, int length)
{
  memcpy(dest, gifftp->UserData, length);
  return length;
}

/// \brief Load GIF from memory.
///
/// Load GIF from the specified buffer in memory.
///
/// \param[out] dest    : Pointer to destination data pointer to be allocated.
/// \param[out] width   : Decoded image width
/// \param[out] height  : Decoded image height
/// \param[out] depth   : Decoded image depth (bits per pixel).
/// \param[in]  flip_y  : Set image for bottom-left origin usage (upside down)
/// \param[in]  data    : Source data to decode.
/// \param[in]  size    : Source data size in bytes.
///
/// \return Error code or 1 if operation succeed.
///
static int __gif_decode_from_mem( uint8_t** dest,
                                  unsigned* width,
                                  unsigned* height,
                                  unsigned* depth,
                                  bool flip_y,
                                  uint8_t* data,
                                  size_t size)
{
  GifFileType* gifftp;

  int error;

  // Define custom read function and load GIF header
  gifftp = DGifOpen(data, __gif_read_mem_fn, &error);
  if(gifftp == nullptr) {
    return OMM_IMAGE_ERR_DECODE;
  }
  // Load GIF content
  if(DGifSlurp(gifftp) == GIF_ERROR) {
      DGifCloseFile(gifftp, &error);
      return false;
  }

  // Get image list, we care only about the first one
  SavedImage* images = gifftp->SavedImages;

  // Get image informations
  unsigned w = images[0].ImageDesc.Width;
  unsigned h = images[0].ImageDesc.Height;

  // Pointer to color table
  ColorMapObject* table;

  // check whether we got a local color table (may never happen)
  if(images[0].ImageDesc.ColorMap) {
    table = images[0].ImageDesc.ColorMap;
  } else {
    table = gifftp->SColorMap;
  }

  // allocate new buffer for RGB data
  size_t rowbytes = w * 3;
  size_t totbytes = h * rowbytes;
  uint8_t* buff;
  try {
    buff = new uint8_t[totbytes];
  } catch(const std::bad_alloc&) {
    DGifCloseFile(gifftp, &error);
    return OMM_IMAGE_ERR_ALLOC;
  }

  // get GIF index list
  uint8_t* sp = static_cast<uint8_t*>(images[0].RasterBits);

  // destination pointer
  uint8_t* dp;

  // here we go to translate indexed color to RGB
  for(unsigned y = 0; y < h; ++y) {

    if(flip_y) {
      dp = &buff[rowbytes * ((h -1) - y)];
    } else {
      dp = &buff[y * rowbytes];
    }

    for(unsigned x = 0; x < w; ++x) {

      dp[0] = table->Colors[*sp].Red;
      dp[1] = table->Colors[*sp].Green;
      dp[2] = table->Colors[*sp].Blue;
      dp += 3; sp++;
    }
  }

  // free decoder
  DGifCloseFile(gifftp, &error);

  // assign values
  (*dest) = buff;
  (*width) = w;
  (*height) = h;
  (*depth) = 24;

  return 1;
}

/// \brief Custom GIF writer
///
/// Custom read function for GIF library to write a file pointer.
///
/// \param[in]  gifftp  Decoder structure pointer.
/// \param[in]  buff    Source buffer.
/// \param[in]  length  Length of data that should be read.
///
/// \return  The number of length that were read
///
static int __gif_write_fp_fn(GifFileType* gifftp, const uint8_t* buff, int length)
{
  return fwrite(buff, 1, length, reinterpret_cast<FILE*>(gifftp->UserData));
}

/// \brief Write GIF to file.
///
/// Write GIF data to the specified file pointer.
///
/// \param[in]  fp      : File pointer to write to
/// \param[in]  data    : Source data to decode.
/// \param[in]  width   : Source Image width in pixel.
/// \param[in]  height  : Source Image depth in pixel.
/// \param[in]  depth   : Source Image bits per pixel, either 24 or 32 are supported.
///
/// \return Error code or 1 if operation succeed.
///
static int __gif_encode_to_fp(  FILE* fp,
                                uint8_t* data,
                                unsigned width,
                                unsigned height,
                                unsigned depth)
{
  // create red, green and blue array for quantizing
  size_t cmpbytes = width * height;
  uint8_t* r_array;
  try {
    r_array = new uint8_t[cmpbytes];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }
  uint8_t* g_array;
  try {
    g_array = new uint8_t[cmpbytes];
  } catch(const std::bad_alloc&) {
    delete [] r_array;
    return OMM_IMAGE_ERR_ALLOC;
  }
  uint8_t* b_array;
  try {
    b_array = new uint8_t[cmpbytes];
  } catch(const std::bad_alloc&) {
    delete [] r_array;
    delete [] g_array;
    return OMM_IMAGE_ERR_ALLOC;
  }

  unsigned c = (depth / 8);

  uint8_t* sp;

  unsigned i = 0;
  for(unsigned y = 0; y < height; ++y) {
    sp = &data[y * height * c];
    for(unsigned x = 0; x < width; ++x) {
      r_array[i] = sp[0];
      g_array[i] = sp[1];
      b_array[i] = sp[2];
      sp += c; ++i;
    }
  }

  // allocate new buffer to receive color indices
  uint8_t* indices;
  try {
    indices = new uint8_t[cmpbytes];
  } catch(const std::bad_alloc&) {
    delete [] r_array;
    delete [] g_array;
    delete [] b_array;
    return OMM_IMAGE_ERR_ALLOC;
  }

  // allocate new color map of 256 colors
  GifColorType* table;
  try {
    table = new GifColorType[256];
  } catch(const std::bad_alloc&) {
    delete [] r_array;
    delete [] g_array;
    delete [] b_array;
    delete [] indices;
    return OMM_IMAGE_ERR_ALLOC;
  }

  // quantize image
  int table_size = 256;
  if(GIF_OK != GifQuantizeBuffer(width, height, &table_size, r_array, g_array, b_array, indices, table)) {
    delete [] r_array;
    delete [] g_array;
    delete [] b_array;
    delete [] indices;
    delete [] table;
    return OMM_IMAGE_ERR_ENCODE;
  }

  // we do not need color array anymore
  delete [] r_array;
  delete [] g_array;
  delete [] b_array;

  // now create and configure the GIF encoder
  GifFileType* gifftp;

  int error = 0;

  // Define custom read function and load GIF header
  gifftp = EGifOpen(fp, __gif_write_fp_fn, &error);
  if(gifftp == nullptr) {
    return OMM_IMAGE_ERR_ENCODE;
  }

  // set GIF global parameters
  gifftp->SWidth = width;
  gifftp->SHeight = height;
  gifftp->SColorResolution = 8;
  gifftp->SBackGroundColor = 0;
  gifftp->SColorMap = GifMakeMapObject(table_size, table); //< global color table

  // set image parameters
  SavedImage image;
  image.ImageDesc.Left = 0;
  image.ImageDesc.Top = 0;
  image.ImageDesc.Width = width;
  image.ImageDesc.Height = height;
  image.ImageDesc.Interlace = false;
  image.ImageDesc.ColorMap = nullptr; //< no local color table
  image.RasterBits = indices; //< our color indices
  image.ExtensionBlockCount = 0;
  image.ExtensionBlocks = nullptr;

  // add image to gif encoder
  GifMakeSavedImage(gifftp, &image);

  // encode GIF
  if(GIF_OK != EGifSpew(gifftp)) {
    delete [] indices;
    delete [] table;
    EGifCloseFile(gifftp, &error);
    return OMM_IMAGE_ERR_ENCODE;
  }

  // we do not need this anymore
  delete [] indices;
  delete [] table;

  return 1;
}

/// \brief Custom GIF write struct
///
/// Custom structure for custom GIF write routine.
///
struct __gif_write_st {
  uint8_t*  buf_data;
  size_t    buf_size;
  size_t    buf_seek;
};

/// \brief Custom GIF writer
///
/// Custom write function for GIF library to encode to memory.
///
/// \param[in]  gifftp  Decoder structure pointer.
/// \param[in]  data    Source buffer.
/// \param[in]  length  Length of data that should be read.
///
/// \return  The number of length that were read
///
static int __gif_write_mem_fn(GifFileType* gifftp, const uint8_t* data, int length)
{
  __gif_write_st* write_st = reinterpret_cast<__gif_write_st*>(gifftp->UserData);

  write_st->buf_size += length;

  // new buffer
  if(write_st->buf_data) {
    write_st->buf_data = reinterpret_cast<uint8_t*>(realloc(write_st->buf_data, write_st->buf_size));
  } else {
    write_st->buf_data = reinterpret_cast<uint8_t*>(malloc(write_st->buf_size));
  }

  memcpy(write_st->buf_data + write_st->buf_seek, data, length);

  write_st->buf_seek += length;

  return length;
}

/// \brief Write BMP to memory.
///
/// Write BMP data to buffer in memory.
///
/// \param[in]  dest    : Pointer to destination data pointer to be allocated.
/// \param[in]  size    : Resulting encoded data size.
/// \param[in]  data    : Source data to decode.
/// \param[in]  width   : Source Image width in pixel.
/// \param[in]  height  : Source Image depth in pixel.
/// \param[in]  depth   : Source Image bits per pixel, either 24 or 32 are supported.
///
/// \return Error code or 1 if operation succeed.
///
static int __gif_encode_to_mem( uint8_t** dest,
                                size_t* size,
                                const uint8_t* data,
                                unsigned width,
                                unsigned height,
                                unsigned depth)
{
  // create red, green and blue array for quantizing
  size_t pixbytes = width * height;
  uint8_t* r_array;
  try {
    r_array = new uint8_t[pixbytes];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }
  uint8_t* g_array;
  try {
    g_array = new uint8_t[pixbytes];
  } catch(const std::bad_alloc&) {
    delete [] r_array;
    return OMM_IMAGE_ERR_ALLOC;
  }
  uint8_t* b_array;
  try {
    b_array = new uint8_t[pixbytes];
  } catch(const std::bad_alloc&) {
    delete [] r_array;
    delete [] g_array;
    return OMM_IMAGE_ERR_ALLOC;
  }

  unsigned c = (depth / 8);

  const uint8_t* sp;

  unsigned i = 0;
  for(unsigned y = 0; y < height; ++y) {
    sp = &data[y * height * c];
    for(unsigned x = 0; x < width; ++x) {
      r_array[i] = sp[0];
      g_array[i] = sp[1];
      b_array[i] = sp[2];
      sp += c; ++i;
    }
  }

  // allocate new buffer to receive color indices
  uint8_t* indices;
  try {
    indices = new uint8_t[pixbytes];
  } catch(const std::bad_alloc&) {
    delete [] r_array;
    delete [] g_array;
    delete [] b_array;
    return OMM_IMAGE_ERR_ALLOC;
  }

  // allocate new color map of 256 colors
  GifColorType* table;
  try {
    table = new GifColorType[256];
  } catch(const std::bad_alloc&) {
    delete [] r_array;
    delete [] g_array;
    delete [] b_array;
    delete [] indices;
    return OMM_IMAGE_ERR_ALLOC;
  }

  // quantize image
  int table_size = 256;
  if(GIF_OK != GifQuantizeBuffer(width, height, &table_size, r_array, g_array, b_array, indices, table)) {
    delete [] r_array;
    delete [] g_array;
    delete [] b_array;
    delete [] indices;
    delete [] table;
    return OMM_IMAGE_ERR_ENCODE;
  }

  // we do not need color array anymore
  delete [] r_array;
  delete [] g_array;
  delete [] b_array;

  // now create and configure the GIF encoder
  GifFileType* gifftp;

  int error = 0;

  // custom write structure
  __gif_write_st write_st;
  write_st.buf_data = nullptr;
  write_st.buf_size = 0;
  write_st.buf_seek = 0;

  // Define custom read function and load GIF header
  gifftp = EGifOpen(&write_st, __gif_write_mem_fn, &error);
  if(gifftp == nullptr) {
    return OMM_IMAGE_ERR_ENCODE;
  }

  // set GIF global parameters
  gifftp->SWidth = width;
  gifftp->SHeight = height;
  gifftp->SColorResolution = 8;
  gifftp->SBackGroundColor = 0;
  gifftp->SColorMap = GifMakeMapObject(table_size, table); //< global color table

  // set image parameters
  SavedImage image;
  image.ImageDesc.Left = 0;
  image.ImageDesc.Top = 0;
  image.ImageDesc.Width = width;
  image.ImageDesc.Height = height;
  image.ImageDesc.Interlace = false;
  image.ImageDesc.ColorMap = nullptr; //< no local color table
  image.RasterBits = indices; //< our color indices
  image.ExtensionBlockCount = 0;
  image.ExtensionBlocks = nullptr;

  // add image to gif encoder
  GifMakeSavedImage(gifftp, &image);

  // encode GIF
  if(GIF_OK != EGifSpew(gifftp)) {
    delete [] indices;
    delete [] table;
    EGifCloseFile(gifftp, &error);
    return OMM_IMAGE_ERR_ENCODE;
  }

  // we do not need this anymore
  delete [] indices;
  delete [] table;

  (*dest) = write_st.buf_data;
  (*size) = write_st.buf_size;

  return 1;
}

/// \brief Load BMP from file.
///
/// Load BMP data from the specified file pointer.
///
/// \param[out] dest    : Pointer to destination data pointer to be allocated.
/// \param[out] width   : Decoded image width
/// \param[out] height  : Decoded image height
/// \param[out] depth   : Decoded image depth (bits per pixel).
/// \param[in]  flip_y  : Set image for bottom-left origin usage (upside down)
/// \param[in]  fp      : File pointer to read data.
///
/// \return Error code or 1 if operation succeed.
///
static int __bmp_decode_from_fp(  uint8_t** dest,
                                  unsigned* width,
                                  unsigned* height,
                                  unsigned* depth,
                                  bool flip_y,
                                  FILE* fp)
{
  OMM_BITMAPHEADER bmp_head;
  OMM_BITMAPINFOHEADER bmp_info;

  // start at beginning of the file + signature
  fseek(fp, 2, SEEK_SET);

  // read base header
  if(fread(&bmp_head, 1, 12, fp) < 12) {
    return OMM_IMAGE_ERR_READ;
  }
  // read info header
  if(fread(&bmp_info, 1, 40, fp) < 40) {
    return OMM_IMAGE_ERR_READ;
  }

  // we support only 24 or 32 bpp
  if(bmp_info.bpp < 24) {
    return OMM_IMAGE_ERR_DECODE;
  }

  unsigned w = bmp_info.width;
  unsigned h = bmp_info.height;
  unsigned d = bmp_info.bpp;
  unsigned c = d / 8; // channel count

  size_t rowbytes = w * c;
  size_t totbytes = h * rowbytes;
  uint8_t* buff;
  try {
    buff = new uint8_t[totbytes];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }

  // seek to bitmap data location and read
  fseek(fp, bmp_head.offbits, SEEK_SET);

  // BMP store data upside down
  if(flip_y) {
    if(fread(buff, 1, totbytes, fp) != totbytes) {
      delete[] buff;
      return OMM_IMAGE_ERR_READ;
    }
  } else {
    uint8_t* row;
    unsigned i = h;
    do {
      i--;
      row = &buff[i * rowbytes];
      if(fread(row, 1, rowbytes, fp) != rowbytes) {
        delete[] buff;
        return OMM_IMAGE_ERR_READ;
      }
    } while(i > 0);
  }

  switch(c) {
    case 3: {
      for(unsigned i = 0; i < totbytes - 2; i += c) {
        buff[i  ] ^= buff[i+2] ^= buff[i  ] ^= buff[i+2]; //< BGR => RGB
      }
      break;
    }
    case 4: {
      for(unsigned i = 0; i < totbytes - 3; i += c) {
        buff[i  ] ^= buff[i+2] ^= buff[i  ] ^= buff[i+2]; //< BGRA => RGBA
      }
      break;
    }
  }

  // assign values
  (*dest) = buff;
  (*width) = w;
  (*height) = h;
  (*depth) = d;

  return 1;
}

/// \brief Load BMP from memory.
///
/// Load BMP from the specified buffer in memory.
///
/// \param[out] dest    : Pointer to destination data pointer to be allocated.
/// \param[out] width   : Decoded image width
/// \param[out] height  : Decoded image height
/// \param[out] depth   : Decoded image depth (bits per pixel).
/// \param[in]  flip_y  : Set image for bottom-left origin usage (upside down)
/// \param[in]  data    : Source data to decode.
/// \param[in]  size    : Source data size in bytes.
///
/// \return Error code or 1 if operation succeed.
///
static int __bmp_decode_from_mem( uint8_t** dest,
                                  unsigned* width,
                                  unsigned* height,
                                  unsigned* depth,
                                  bool flip_y,
                                  const uint8_t* data,
                                  size_t size)
{
  const OMM_BITMAPHEADER* bmp_head;
  const OMM_BITMAPINFOHEADER* bmp_info;

  // get pointers to headers
  bmp_head = reinterpret_cast<const OMM_BITMAPHEADER*>(&data[2]);
  bmp_info = reinterpret_cast<const OMM_BITMAPINFOHEADER*>(&data[14]);

  // we support only 24 or 32 bpp
  if(bmp_info->bpp < 24) {
    return OMM_IMAGE_ERR_DECODE;
  }

  unsigned w = bmp_info->width;
  unsigned h = bmp_info->height;
  unsigned d = bmp_info->bpp;
  unsigned c = d / 8; // channel count

  size_t rowbytes = w * c;
  size_t totbytes = h * rowbytes;
  uint8_t* buff;
  try {
    buff = new uint8_t[totbytes];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }

  // seek to bitmap data location and read
  const uint8_t* bmp_data = &data[bmp_head->offbits];


  // BMP store data upside down
  if(flip_y) {
    memcpy(buff, bmp_data, totbytes);
  } else {
    uint8_t* row;
    unsigned i = h;
    do {
      i--;
      row = &buff[i * rowbytes];
      memcpy(row, bmp_data, rowbytes);
      bmp_data += rowbytes;
    } while(i > 0);
  }

  switch(c) {
    case 3: {
      for(unsigned i = 0; i < totbytes - 2; i += c) {
        buff[i  ] ^= buff[i+2] ^= buff[i  ] ^= buff[i+2]; //< BGR => RGB
      }
      break;
    }
    case 4: {
      for(unsigned i = 0; i < totbytes - 3; i += c) {
        buff[i  ] ^= buff[i+2] ^= buff[i  ] ^= buff[i+2]; //< BGRA => RGBA
      }
      break;
    }
  }

  // assign values
  (*dest) = buff;
  (*width) = w;
  (*height) = h;
  (*depth) = d;

  return 1;
}

/// \brief Write BMP to file.
///
/// Write BMP data to the specified file pointer.
///
/// \param[in]  fp      : File pointer to write to
/// \param[in]  data    : Source data to decode.
/// \param[in]  width   : Source Image width in pixel.
/// \param[in]  height  : Source Image depth in pixel.
/// \param[in]  depth   : Source Image bits per pixel, either 24 or 32 are supported.
///
/// \return Error code or 1 if operation succeed.
///
static int __bmp_encode_to_fp(  FILE* fp,
                                uint8_t* data,
                                unsigned width,
                                unsigned height,
                                unsigned depth)
{
  // BMP signature ASCII "BM"
  uint16_t type = 0x4d42; //< BMP signature

  if(fwrite(&type, 1, 2, fp) != 2) {
    return OMM_IMAGE_ERR_WRITE;
  }

  unsigned c = (depth / 8);
  unsigned pix_size = width * height * c;

  OMM_BITMAPHEADER bmp_head;
  bmp_head.size = pix_size + 54; // magic + base header + info header
  bmp_head.reserved1 = 0;
  bmp_head.reserved2 = 0;
  bmp_head.offbits = 54; // magic + base header + info header

  OMM_BITMAPINFOHEADER bmp_info;
  bmp_info.size = 40;
  bmp_info.width = width;
  bmp_info.height = height;
  bmp_info.planes = 1;
  bmp_info.bpp = depth;
  bmp_info.compression = 0;
  bmp_info.sizeimage = pix_size;
  bmp_info.xppm = 0x0ec4;
  bmp_info.yppm = 0x0ec4;
  bmp_info.clrused = 0;
  bmp_info.clrimportant = 0;

  if(fwrite(&bmp_head, 1, 12, fp) != 12) {
    return OMM_IMAGE_ERR_WRITE;
  }
  if(fwrite(&bmp_info, 1, 40, fp) != 40) {
    return OMM_IMAGE_ERR_WRITE;
  }

  // allocate row for data translation
  size_t rowbytes = width * c;
  uint8_t* row;
  try {
    row = new uint8_t[rowbytes];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }

  uint8_t* sp;
  uint8_t* dp;

  for(unsigned y = 0; y < height; ++y) {

    sp = &data[rowbytes * ((height - 1) - y)]; // reverse row up to bottom
    dp = row;

    switch(c)
    {
    case 3:
      for(unsigned x = 0; x < width; ++x) {
        // convert RGBA to BGRA
        dp[0] = sp[2];
        dp[1] = sp[1];
        dp[2] = sp[0];
        sp += 3;
        dp += 3;
      }
      break;
    case 4:
      for(unsigned x = 0; x < width; ++x) {
        // convert RGBA to BGRA
        dp[0] = sp[2];
        dp[1] = sp[1];
        dp[2] = sp[0];
        dp[3] = sp[3];
        sp += 4;
        dp += 4;
      }
      break;
    }

    // write row to file
    if(fwrite(row, 1, rowbytes, fp) != rowbytes) {
      delete [] row;
      return OMM_IMAGE_ERR_WRITE;
    }
  }

  delete [] row;

  return 1;
}

/// \brief Write BMP to memory.
///
/// Write BMP data to buffer in memory.
///
/// \param[in]  dest    : Pointer to destination data pointer to be allocated.
/// \param[in]  size    : Resulting encoded data size.
/// \param[in]  data    : Source data to decode.
/// \param[in]  width   : Source Image width in pixel.
/// \param[in]  height  : Source Image depth in pixel.
/// \param[in]  depth   : Source Image bits per pixel, either 24 or 32 are supported.
///
/// \return Error code or 1 if operation succeed.
///
static int __bmp_encode_to_mem( uint8_t** dest,
                                size_t* size,
                                const uint8_t* data,
                                unsigned width,
                                unsigned height,
                                unsigned depth)
{
  unsigned c = (depth / 8);
  unsigned pix_size = width * height * c;

  size_t tot_size = pix_size + 54; // magic + base header + info header

  // Allocate new buffer
  uint8_t* buff;
  try {
    buff = new uint8_t[tot_size];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }

  // set the BMP signature as ASCII "BM"
  buff[0] = 'B'; buff[1] = 'M';
  size_t offset = 2; // offset

  OMM_BITMAPHEADER bmp_head;
  bmp_head.size = tot_size;
  bmp_head.reserved1 = 0;
  bmp_head.reserved2 = 0;
  bmp_head.offbits = 54; // magic + base header + info header

  OMM_BITMAPINFOHEADER bmp_info;
  bmp_info.size = 40;
  bmp_info.width = width;
  bmp_info.height = height;
  bmp_info.planes = 1;
  bmp_info.bpp = depth;
  bmp_info.compression = 0;
  bmp_info.sizeimage = pix_size;
  bmp_info.xppm = 0x0ec4;
  bmp_info.yppm = 0x0ec4;
  bmp_info.clrused = 0;
  bmp_info.clrimportant = 0;

  memcpy(buff + offset, &bmp_head, 12);
  offset += 12;

  memcpy(buff + offset, &bmp_info, 40);
  offset += 40;

  // allocate row for data translation
  size_t rowbytes = width * c;
  uint8_t* row;
  try {
    row = new uint8_t[rowbytes];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }

  const uint8_t* sp;
  uint8_t* dp;

  for(unsigned y = 0; y < height; ++y) {

    sp = &data[rowbytes * ((height - 1) - y)]; // reverse row up to bottom
    dp = row;

    switch(c)
    {
    case 3:
      for(unsigned x = 0; x < width; ++x) {
        // convert RGBA to BGRA
        dp[0] = sp[2];
        dp[1] = sp[1];
        dp[2] = sp[0];
        sp += 3;
        dp += 3;
      }
      break;
    case 4:
      for(unsigned x = 0; x < width; ++x) {
        // convert RGBA to BGRA
        dp[0] = sp[2];
        dp[1] = sp[1];
        dp[2] = sp[0];
        dp[3] = sp[3];
        sp += 4;
        dp += 4;
      }
      break;
    }

    // copy row to buffer
    memcpy(buff + offset, row, rowbytes);
    offset += rowbytes;
  }

  delete [] row;

  (*size) = tot_size;
  (*dest) = buff;

  return 1;
}

/// \brief Load JPEG from file.
///
/// Load JPEG data from the specified file pointer.
///
/// \param[out] dest    : Pointer to destination data pointer to be allocated.
/// \param[out] width   : Decoded image width
/// \param[out] height  : Decoded image height
/// \param[out] depth   : Decoded image depth (bits per pixel).
/// \param[in]  flip_y  : Set image for bottom-left origin usage (upside down)
/// \param[in]  fp      : File pointer to read data.
///
/// \return Error code or 1 if operation succeed.
///
static int __jpg_decode_from_fp(  uint8_t** dest,
                                  unsigned* width,
                                  unsigned* height,
                                  unsigned* depth,
                                  bool flip_y,
                                  FILE* fp)
{
  // create base object for jpeg decoder
  jpeg_decompress_struct cinfo;
  jpeg_error_mgr jerr;

  // create jpeg decoder
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  // make sure we start at beginning and setup jpeg IO
  fseek(fp, 0, SEEK_SET);
  jpeg_stdio_src(&cinfo, fp);

  // read jpeg header
  if(jpeg_read_header(&cinfo, TRUE) != 1) {
    return OMM_IMAGE_ERR_DECODE;
  }
  jpeg_start_decompress(&cinfo);

	unsigned w = cinfo.output_width;
	unsigned h = cinfo.output_height;
	unsigned c = cinfo.output_components;

	// bytes per row (scanline)
  int rowbytes = w * c;

  // allocate buffer
  size_t totbytes = h * rowbytes;
  uint8_t* buff;
  try {
    buff = new uint8_t[totbytes];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }

  // row pointer
  uint8_t* rowptr[1];

  if(flip_y) {
    while (cinfo.output_scanline < cinfo.output_height) {
      // set pointer to buffer according scanline (row)
      rowptr[0] = buff + (((h - cinfo.output_scanline) * rowbytes) - rowbytes);
      jpeg_read_scanlines(&cinfo, rowptr, 1); //< read one row (scanline)
    }
  } else {
    while (cinfo.output_scanline < cinfo.output_height) {
      // set pointer to buffer according scanline (row)
      rowptr[0] = buff + (cinfo.output_scanline * rowbytes);
      jpeg_read_scanlines(&cinfo, rowptr, 1); //< read one row (scanline)
    }
  }

	// cleanup
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

  // assign values
  (*dest) = buff;
  (*width) = w;
  (*height) = h;
  (*depth) = c * 8;

  return 1;
}

/// \brief Load JPEG from memory.
///
/// Load JPEG from the specified buffer in memory.
///
/// \param[out] dest    : Pointer to destination data pointer to be allocated.
/// \param[out] width   : Decoded image width
/// \param[out] height  : Decoded image height
/// \param[out] depth   : Decoded image depth (bits per pixel).
/// \param[in]  flip_y  : Set image for bottom-left origin usage (upside down)
/// \param[in]  data    : Source data to decode.
/// \param[in]  size    : Source data size in bytes.
///
/// \return Error code or 1 if operation succeed.
///
static int __jpg_decode_from_mem( uint8_t** dest,
                                  unsigned* width,
                                  unsigned* height,
                                  unsigned* depth,
                                  bool flip_y,
                                  const uint8_t* data,
                                  size_t size)
{
  // create base object for jpeg decoder
  jpeg_decompress_struct cinfo;
  jpeg_error_mgr jerr;

  // create jpeg decoder
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  // set read data pointer
  jpeg_mem_src(&cinfo, data, size);

  // read jpeg header
  if(jpeg_read_header(&cinfo, TRUE) != 1) {
    return OMM_IMAGE_ERR_DECODE;
  }
  jpeg_start_decompress(&cinfo);

	unsigned w = cinfo.output_width;
	unsigned h = cinfo.output_height;
	unsigned c = cinfo.output_components;

	// bytes per row (scanline)
  int rowbytes = w * c;

  // allocate buffer
  size_t totbytes = h * rowbytes;
  uint8_t* buff;
  try {
    buff = new uint8_t[totbytes];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }

  // row pointer
  uint8_t* rowptr[1];

  if(flip_y) {
    while (cinfo.output_scanline < cinfo.output_height) {
      // set pointer to buffer according scanline (row)
      rowptr[0] = buff + (((h - cinfo.output_scanline) * rowbytes) - rowbytes);
      jpeg_read_scanlines(&cinfo, rowptr, 1); //< read one row (scanline)
    }
  } else {
    while (cinfo.output_scanline < cinfo.output_height) {
      // set pointer to buffer according scanline (row)
      rowptr[0] = buff + (cinfo.output_scanline * rowbytes);
      jpeg_read_scanlines(&cinfo, rowptr, 1); //< read one row (scanline)
    }
  }

	// cleanup
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

  // assign values
  (*dest) = buff;
  (*width) = w;
  (*height) = h;
  (*depth) = c * 8;

  return 1;
}

/// \brief Write JPEG to file.
///
/// Encode and write JPEG data to the specified file pointer.
///
/// \param[in]  fp      : File pointer to write to.
/// \param[in]  level   : JPEG quality level 0 to 10.
/// \param[in]  data    : Source data to decode.
/// \param[in]  width   : Source Image width in pixel.
/// \param[in]  height  : Source Image depth in pixel.
/// \param[in]  depth   : Source Image bits per pixel, either 24 or 32 are supported.
///
/// \return True if succeed, false if write error occurred.
///
static int __jpg_encode_to_fp(  FILE* fp,
                                int level,
                                uint8_t* data,
                                unsigned width,
                                unsigned height,
                                unsigned depth)
{

  // create base object for jpeg encoder
  jpeg_compress_struct cinfo;
  jpeg_error_mgr jerr;

  // create jpeg encoder
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  // make sure we start at beginning and setup jpeg IO
  fseek(fp, 0, SEEK_SET);
  jpeg_stdio_dest(&cinfo, fp);


  // define image parameters
  cinfo.image_width = width;        //< Image width in pixels
  cinfo.image_height = height;      //< Image height in pixels
  cinfo.input_components = 3;       //< per pixel color components
  cinfo.in_color_space = JCS_RGB; 	//< pixel format

  // set parameters to encoder
  jpeg_set_defaults(&cinfo);

  // set compression quality
  jpeg_set_quality(&cinfo, level * 10, true); // quality is 0-100 scaled

  // initialize encoder
  jpeg_start_compress(&cinfo, true);

  unsigned c = depth / 8;

  // hold row size in bytes
  unsigned rowbytes = width * c;

  uint8_t* row[1];

  if(c == 4) {

    uint8_t* sp;
    uint8_t* dp;

    try {
      row[0] = new uint8_t[width * 3];
    } catch(const std::bad_alloc&) {
      return OMM_IMAGE_ERR_ALLOC;
    }

    while(cinfo.next_scanline < cinfo.image_height) {
      // set source and destination pointers
      sp = &data[cinfo.next_scanline * rowbytes];
      dp = row[0];
      // convert RGBA to RGB
      for(unsigned i = 0; i < width; ++i) {
        dp[0] = sp[0]; dp[1] = sp[4]; dp[2] = sp[3];
        sp += 4; dp += 3;
      }
      // send to encoder
      jpeg_write_scanlines(&cinfo, row, 1);
    }

    delete [] row[0];

  } else {

    while(cinfo.next_scanline < cinfo.image_height) {
      // get pointer to row
      row[0] = &data[cinfo.next_scanline * rowbytes];
      // send to encoder
      jpeg_write_scanlines(&cinfo, row, 1);
    }

  }

  // finalize compression
  jpeg_finish_compress(&cinfo);

  // destroy encoder
  jpeg_destroy_compress(&cinfo);

  return 1;
}

/// \brief Write JPEG to memory.
///
/// Encode and write JPEG data to buffer in memory.
///
/// \param[in]  dest    : Pointer to destination data pointer to be allocated.
/// \param[in]  size    : Resulting encoded data size.
/// \param[in]  level   : JPEG quality level 0 to 10.
/// \param[in]  data    : Source data to decode.
/// \param[in]  width   : Source Image width in pixel.
/// \param[in]  height  : Source Image depth in pixel.
/// \param[in]  depth   : Source Image bits per pixel, either 24 or 32 are supported.
///
/// \return True if succeed, false if write error occurred.
///
static int __jpg_encode_to_mem( uint8_t** dest,
                                size_t* size,
                                int level,
                                uint8_t* data,
                                unsigned width,
                                unsigned height,
                                unsigned depth)
{

  // create base object for jpeg encoder
  jpeg_compress_struct cinfo;
  jpeg_error_mgr jerr;

  // create jpeg encoder
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  // set pointer params
  unsigned long jpg_size;
  jpeg_mem_dest(&cinfo, dest, &jpg_size);

  // define image parameters
  cinfo.image_width = width;        //< Image width in pixels
  cinfo.image_height = height;      //< Image height in pixels
  cinfo.input_components = 3;       //< per pixel color components
  cinfo.in_color_space = JCS_RGB; 	//< pixel format

  // set parameters to encoder
  jpeg_set_defaults(&cinfo);

  // set compression quality
  jpeg_set_quality(&cinfo, level * 10, true); // quality is 0-100 scaled

  // initialize encoder
  jpeg_start_compress(&cinfo, true);

  unsigned c = depth / 8;

  // hold row size in bytes
  unsigned rowbytes = width * c;

  uint8_t* row[1];

  if(c == 4) {

    uint8_t* sp;
    uint8_t* dp;

    try {
      row[0] = new uint8_t[width * 3];
    } catch(const std::bad_alloc&) {
      return OMM_IMAGE_ERR_ALLOC;
    }

    while(cinfo.next_scanline < cinfo.image_height) {
      // set source and destination pointers
      sp = &data[cinfo.next_scanline * rowbytes];
      dp = row[0];
      // convert RGBA to RGB
      for(unsigned i = 0; i < width; ++i) {
        dp[0] = sp[0]; dp[1] = sp[4]; dp[2] = sp[3];
        sp += 4; dp += 3;
      }
      // send to encoder
      jpeg_write_scanlines(&cinfo, row, 1);
    }

    delete [] row[0];

  } else {

    while(cinfo.next_scanline < cinfo.image_height) {
      // get pointer to row
      row[0] = &data[cinfo.next_scanline * rowbytes];
      // send to encoder
      jpeg_write_scanlines(&cinfo, row, 1);
    }

  }

  // finalize compression
  jpeg_finish_compress(&cinfo);

  // destroy encoder
  jpeg_destroy_compress(&cinfo);

  (*size) = jpg_size;

  return 1;
}

/// \brief Load PNG from file.
///
/// Load PNG data from the specified file pointer.
///
/// \param[out] dest    : Pointer to destination data pointer to be allocated.
/// \param[out] width   : Decoded image width
/// \param[out] height  : Decoded image height
/// \param[out] depth   : Decoded image depth (bits per pixel).
/// \param[in]  flip_y  : Set image for bottom-left origin usage (upside down)
/// \param[in]  fp      : File pointer to read data.
///
/// \return Error code or 1 if operation succeed.
///
static int __png_decode_from_fp(  uint8_t** dest,
                                  unsigned* width,
                                  unsigned* height,
                                  unsigned* depth,
                                  bool flip_y,
                                  FILE* fp)
{
  // create base object for png decoder
  png_structp   png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  png_infop     png_info = png_create_info_struct(png);

  // make sure we start at beginning and setup png IO
  fseek(fp, 0, SEEK_SET);
  png_init_io(png, fp);

  // get image properties
  png_read_info(png, png_info);

  unsigned w = png_get_image_width(png, png_info);
  unsigned h = png_get_image_height(png, png_info);
  unsigned c = png_get_channels(png, png_info);
  unsigned d = png_get_bit_depth(png, png_info) * c;

  // bytes count per row
  size_t rowbytes = png_get_rowbytes(png, png_info);

  size_t totbytes = h * rowbytes;
  uint8_t* buff;
  try {
    buff = new uint8_t[totbytes];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }

  // we need an array of pointers, with one pointer per row
  uint8_t** rowptr;
  try {
    rowptr = new uint8_t*[h];
  } catch(const std::bad_alloc&) {
    delete [] buff;
    return OMM_IMAGE_ERR_ALLOC;
  }

  // setup each pointer to a destination row in destination buffer
  if(flip_y) {
    for(unsigned i = 0; i < h; i++)
      rowptr[h - 1 - i] = buff + (i * rowbytes);
  } else {
    for(unsigned i = 0; i < h; i++)
      rowptr[i] = buff + (i * rowbytes);
  }

  // get bitmap data
  png_read_image(png, rowptr);

  // cleanup
  png_destroy_read_struct(&png, &png_info, nullptr);
  delete[] rowptr;

  // assign values
  (*dest) = buff;
  (*width) = w;
  (*height) = h;
  (*depth) = d;

  return 1;
}

/// \brief Custom PNG read struct
///
/// Custom structure for custom PNG read function
///
struct __png_read_st {
  const uint8_t*  buf_data;
  size_t          buf_size;
  size_t          buf_seek;
};

/// \brief Custom PNG read
///
/// Custom read function for PNG library to read from memory.
///
/// \param[in]  png     Decoder structure pointer.
/// \param[in]  dest    Source buffer.
/// \param[in]  length  Length of data that should be read.
///
void __png_read_fn(png_structp png, png_bytep dest, png_size_t length)
{
  __png_read_st *read_st = static_cast<__png_read_st*>(png_get_io_ptr(png));

  if(length > (read_st->buf_size - read_st->buf_seek))
      png_error(png, "read error in __png_read_fn");

  memcpy(dest, read_st->buf_data + read_st->buf_seek, length);

  read_st->buf_seek += length;
}

/// \brief Load PNG from memory.
///
/// Load PNG from the specified buffer in memory.
///
/// \param[out] dest    : Pointer to destination data pointer to be allocated.
/// \param[out] width   : Decoded image width
/// \param[out] height  : Decoded image height
/// \param[out] depth   : Decoded image depth (bits per pixel).
/// \param[in]  flip_y  : Set image for bottom-left origin usage (upside down)
/// \param[in]  data    : Source data to decode.
/// \param[in]  size    : Source data size in bytes.
///
/// \return Error code or 1 if operation succeed.
///
static int __png_decode_from_mem( uint8_t** dest,
                                  unsigned* width,
                                  unsigned* height,
                                  unsigned* depth,
                                  bool flip_y,
                                  const uint8_t* data,
                                  size_t size)
{
  // create base object for png decoder
  png_structp   png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  png_infop     png_info = png_create_info_struct(png);

  // read structure
  __png_read_st read_st;
  read_st.buf_data = data;
  read_st.buf_size = size;
  read_st.buf_seek = 0;

  // custom read process
  png_set_read_fn(png, &read_st, __png_read_fn);

  // get image properties
  png_read_info(png, png_info);

  unsigned w = png_get_image_width(png, png_info);
  unsigned h = png_get_image_height(png, png_info);
  unsigned c = png_get_channels(png, png_info);
  unsigned d = png_get_bit_depth(png, png_info) * c;

  // bytes count per row
  size_t rowbytes = png_get_rowbytes(png, png_info);

  size_t totbytes = h * rowbytes;
  uint8_t* buff;
  try {
    buff = new uint8_t[totbytes];
  } catch(const std::bad_alloc&) {
    return OMM_IMAGE_ERR_ALLOC;
  }

  // we need an array of pointers, with one pointer per row
  uint8_t** rowptr;
  try {
    rowptr = new uint8_t*[h];
  } catch(const std::bad_alloc&) {
    delete [] buff;
    return OMM_IMAGE_ERR_ALLOC;
  }

  // setup each pointer to a destination row in destination buffer
  if(flip_y) {
    for(unsigned i = 0; i < h; i++)
      rowptr[h - 1 - i] = buff + (i * rowbytes);
  } else {
    for(unsigned i = 0; i < h; i++)
      rowptr[i] = buff + (i * rowbytes);
  }

  // get bitmap data
  png_read_image(png, rowptr);

  // cleanup
  png_destroy_read_struct(&png, &png_info, nullptr);
  delete[] rowptr;

  // assign values
  (*dest) = buff;
  (*width) = w;
  (*height) = h;
  (*depth) = d;

  return 1;
}

/// \brief Write PNG to file.
///
/// Encode and write PNG data to the specified file pointer.
///
/// \param[in]  fp      : File pointer to write to
/// \param[in]  level   : PNG compression level 10 to 0.
/// \param[in]  data    : Source data to decode.
/// \param[in]  width   : Source Image width in pixel.
/// \param[in]  height  : Source Image depth in pixel.
/// \param[in]  depth   : Source Image bits per pixel, either 24 or 32 are supported.
///
/// \return Error code or 1 if operation succeed.
///
static int __png_encode_to_fp(  FILE* fp,
                                int level,
                                const uint8_t* data,
                                unsigned width,
                                unsigned height,
                                unsigned depth)
{
  // create base object for png decoder
  png_structp   png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  png_infop     png_info = png_create_info_struct(png);

  // make sure we start at beginning and setup png IO
  fseek(fp, 0, SEEK_SET);
  png_init_io(png, fp);

  unsigned c = depth / 8;

  // set PNG parameters
  png_set_IHDR(png, png_info, width, height, 8,
               (c == 4)?PNG_COLOR_TYPE_RGB_ALPHA:PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  // set compression level
  if(level > 9) level = 9; // clamp to 9
  png_set_compression_level(png, level);

  // write info to PNG
  png_write_info(png, png_info);

  const uint8_t* row;

  // write data
  for(unsigned i = 0; i < height; ++i) {
    row = &data[(width * c) * i];
    png_write_row(png, row);
  }

  // clear PGN encoder
  png_free_data(png, png_info, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png, &png_info);

  return 1;
}

/// \brief Custom PNG write struct
///
/// Custom structure for custom PNG write function
///
struct __png_write_st {
  uint8_t*  buf_data;
  size_t    buf_size;
  size_t    buf_seek;
};

/// \brief Custom PNG writer
///
/// Custom write function for PNG library to encode to memory.
///
/// \param[in]  png     Decoder structure pointer.
/// \param[in]  data    Destination buffer.
/// \param[in]  length  Length of data that should be written.
///
void __png_write_fn(png_structp png, png_bytep data, png_size_t length)
{
  __png_write_st *write_st = static_cast<__png_write_st*>(png_get_io_ptr(png));

  write_st->buf_size += length;

  // new buffer
  if(write_st->buf_data) {
    write_st->buf_data = reinterpret_cast<uint8_t*>(realloc(write_st->buf_data, write_st->buf_size));
  } else {
    write_st->buf_data = reinterpret_cast<uint8_t*>(malloc(write_st->buf_size));
  }

  if(!write_st->buf_data)
    png_error(png, "alloc error in __png_write_fn");

  memcpy(write_st->buf_data + write_st->buf_seek, data, length);

  write_st->buf_seek += length;
}

/// \brief Custom PNG flush function
///
/// Custom callback function for PNG encoder flush.
///
void __png_flush_fn(png_structp png)
{
}

/// \brief Write PNG to memory.
///
/// Encode and write PNG data to buffer in memory.
///
/// \param[in]  dest    : Pointer to destination data pointer to be allocated.
/// \param[in]  size    : Resulting encoded data size.
/// \param[in]  data    : Source data to decode.
/// \param[in]  width   : Source Image width in pixel.
/// \param[in]  height  : Source Image depth in pixel.
/// \param[in]  depth   : Source Image bits per pixel, either 24 or 32 are supported.
///
/// \return Error code or 1 if operation succeed.
///
static int __png_encode_to_mem( uint8_t** dest,
                                size_t* size,
                                int level,
                                const uint8_t* data,
                                unsigned width,
                                unsigned height,
                                unsigned depth)
{
  // create base object for png decoder
  png_structp   png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  png_infop     png_info = png_create_info_struct(png);

  // write structure
  __png_write_st write_st;
  write_st.buf_data = nullptr;
  write_st.buf_size = 0;
  write_st.buf_seek = 0;

  // custom write process
  png_set_write_fn(png, &write_st, __png_write_fn, __png_flush_fn);

  unsigned c = depth / 8;

  // set PNG parameters
  png_set_IHDR(png, png_info, width, height, 8,
               (c == 4)?PNG_COLOR_TYPE_RGB_ALPHA:PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  // set compression level
  if(level > 9) level = 9; // clamp to 9
  png_set_compression_level(png, level);

  // write info to PNG
  png_write_info(png, png_info);

  const uint8_t* row;

  // write data
  for(unsigned i = 0; i < height; ++i) {
    row = &data[(width * c) * i];
    png_write_row(png, row);
  }

  // clear PGN encoder
  png_free_data(png, png_info, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png, &png_info);

  (*size) = write_st.buf_size;
  (*dest) = write_st.buf_data;

  return 1;
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
/// \param[in]  dst     : Array to receive interpolated pixel values
/// \param[in]  data    : Image pixel data
/// \param[in]  w       : Image width
/// \param[in]  h       : Image height
/// \param[in]  c       : Image channel count
/// \param[in]  u       : Horizontal coordinate to get interpolated pixel
/// \param[in]  v       : Vertical coordinate to get interpolated pixel
///
static inline void __sample_bicubic(uint8_t* dst, const uint8_t* data, unsigned w, unsigned h, unsigned c, float u, float v)
{
  float x, y;

  float xf = modf((u * w) - 0.5f, &x);
  float yf = modf((v * h) - 0.5f, &y);

  int ix = static_cast<int>(x) - 1;
  int iy = static_cast<int>(y) - 1;

  const uint8_t* p[4];

  float r[4];
  float g[4];
  float b[4];
  float a[4];

  int bx, by;
  int hmax = h - 1;
  int wmax = w - 1;

  for(int j = 0; j < 4; ++j) {

    by = std::max(0, std::min(iy + j, hmax));

    for(int i = 0; i < 4; ++i) {

      bx = std::max(0, std::min(ix + i, wmax));

      p[i] = &data[(by * (w * c)) + (bx * c)];
    }

    r[j] = __interp_cubic(p[0][0], p[1][0], p[2][0], p[3][0], xf);
    g[j] = __interp_cubic(p[0][1], p[1][1], p[2][1], p[3][1], xf);
    b[j] = __interp_cubic(p[0][2], p[1][2], p[2][2], p[3][2], xf);
    if(c == 4) a[j] = __interp_cubic(p[0][3], p[1][3], p[2][3], p[3][3], xf);
  }

  float m[4];

  m[0] = __interp_cubic(r[0], r[1], r[2], r[3], yf);
  m[1] = __interp_cubic(g[0], g[1], g[2], g[3], yf);
  m[2] = __interp_cubic(b[0], b[1], b[2], b[3], yf);
  if(c == 4) m[3] = __interp_cubic(a[0], a[1], a[2], a[3], yf);

  for(unsigned i = 0; i < c; ++i) {
    dst[i] = static_cast<uint8_t>(std::max(0.0f, std::min(m[i], 255.0f)));
  }
}


/// \brief Get box interpolated pixel.
///
/// Compute the box interpolated pixel at the specified coordinates.
///
/// \param[in]  dst     : Array to receive interpolated pixel values
/// \param[in]  data    : Image pixel data
/// \param[in]  w       : Image width
/// \param[in]  h       : Image height
/// \param[in]  c       : Image channel count
/// \param[in]  u       : Horizontal coordinate to get interpolated pixel
/// \param[in]  v       : Vertical coordinate to get interpolated pixel
/// \param[in]  bw      : Box width in pixel
/// \param[in]  bh      : Box height in pixel
///
static inline void __sample_box(uint8_t* dst, const uint8_t* data, unsigned w, unsigned h, unsigned c, float u, float v, unsigned bw, unsigned bh)
{
  unsigned by, bx;

  const uint8_t *sp;

  unsigned r = 0;
  unsigned g = 0;
  unsigned b = 0;
  unsigned a = 0;

  int hmax = (h - 1);
  int wmax = (w - 1);

  int x = u * hmax;
  int y = v * wmax;

  for(int j = 0; j < bh; ++j) {

    by = std::max(0, std::min(y + j, hmax));

    for(int i = 0; i < bw; ++i) {

      bx = std::max(0, std::min(x + i, wmax));

      sp = &data[(bx + by * w) * c];

      r += sp[0];
      g += sp[1];
      b += sp[2];
      if(c == 4) a += sp[3];
    }
  }

  unsigned f = bh * bw;

  dst[0] = r / f;
  dst[1] = g / f;
  dst[2] = b / f;
  if(c == 4) dst[3] = a / f;
}


/// \brief Downsample image
///
/// Reduce image size using box filtering.
///
/// \param[in]  dst  : Destination pointer
/// \param[in]  dw   : Destination width
/// \param[in]  dh   : Destination height
/// \param[in]  src  : Source pointer
/// \param[in]  sw   : Source width
/// \param[in]  sh   : source height
/// \param[in]  c    : Channel count (bytes per pixel)
///
static void __image_downsample(uint8_t* dst, unsigned dw, unsigned dh, const uint8_t* src, unsigned sw, unsigned sh, unsigned c)
{
  unsigned bw = floor(static_cast<float>(sw) / dw);
  unsigned bh = floor(static_cast<float>(sh) / dh);

  float mx = 1.0f / (static_cast<float>(dw) - 1);
  float my = 1.0f / (static_cast<float>(dh) - 1);
  float u, v;

  uint8_t px[4];

  uint8_t* dp;

  for(unsigned y = 0; y < dh; ++y) {

    dp = &dst[dw * c * y];

    v = y * my;

    for(unsigned x = 0; x < dw; ++x) {

      u = x * mx;

      __sample_box(px, src, sw, sh, c, u, v, bw, bh);

      dp[0] = px[0];
      dp[1] = px[1];
      dp[2] = px[2];
      if(c == 4) dp[3] = px[3];

      dp += c;
    }
  }
}


/// \brief Upsample image
///
/// Reduce image size using box filtering.
///
/// \param[in]  dst  : Destination pointer
/// \param[in]  dw   : Destination width
/// \param[in]  dh   : Destination height
/// \param[in]  src  : Source pointer
/// \param[in]  sw   : Source width
/// \param[in]  sh   : source height
/// \param[in]  c    : Channel count (bytes per pixel)
///
static void __image_upsample(uint8_t* dst, unsigned dw, unsigned dh, const uint8_t* src, unsigned sw, unsigned sh, unsigned c)
{
  float mx = 1.0f / (static_cast<float>(dw) - 1);
  float my = 1.0f / (static_cast<float>(dh) - 1);

  uint8_t* dp;

  uint8_t px[4];

  float u, v;

  for(unsigned y = 0; y < dh; ++y) {

    dp = &dst[dw * c * y];

    v = y * my;

    for(unsigned x = 0; x < dw; ++x) {

      u = x * mx;

      __sample_bicubic(px, src, sw, sh, c, u, v);

      dp[0] = px[0];
      dp[1] = px[1];
      dp[2] = px[2];
      if(c == 4) dp[3] = px[3];

      dp += c;
    }
  }

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmImage::OmImage() :
  _status(OMM_IMAGE_ERR_NODATA),
  _data(nullptr),
  _width(0),
  _height(0),
  _depth(0)
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
bool OmImage::load(const wstring& path)
{
  this->clear();

  this->_status = 0;

  // open file
  FILE* fp;
  //if((fp = fopen(Om_toUtf8(path).c_str(), "rb")) == nullptr) {
  if((fp = _wfopen(path.c_str(), L"rb")) == nullptr) {
    this->_status = OMM_IMAGE_ERR_OPEN;
    return false;
  }

  // read first 8 bytes of the head of file
  unsigned char head[8];
  fseek(fp, 0, SEEK_SET);
  if(fread(head, 1, 8, fp) < 8) {
    this->_status = OMM_IMAGE_ERR_READ;
    return false;
  }

  // check for known image file signatures
  switch(__sign_matches(head))
  {
  case OMM_IMAGE_BMP:
    this->_status = __bmp_decode_from_fp(&this->_data, &this->_width, &this->_height, &this->_depth, false, fp);
    break;
  case OMM_IMAGE_JPG:
    this->_status = __jpg_decode_from_fp(&this->_data, &this->_width, &this->_height, &this->_depth, false, fp);
    break;
  case OMM_IMAGE_PNG:
    this->_status = __png_decode_from_fp(&this->_data, &this->_width, &this->_height, &this->_depth, false, fp);
    break;
  case OMM_IMAGE_GIF:
    this->_status = __gif_decode_from_fp(&this->_data, &this->_width, &this->_height, &this->_depth, false, fp);
    break;
  default:
    this->_status = OMM_IMAGE_ERR_FORMAT;
    return false;
  }

  fclose(fp);

  return (this->_status == 1);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::load(uint8_t* data, size_t size)
{
  this->clear();

  this->_status = 0;

  // must have at least 8 bytes to read...
  if(size < 8) {
    this->_status = OMM_IMAGE_ERR_FORMAT;
    return false;
  }

  // check for known image file signatures
  switch(__sign_matches(data))
  {
  case OMM_IMAGE_BMP:
    this->_status = __bmp_decode_from_mem(&this->_data, &this->_width, &this->_height, &this->_depth, false, data, size);
    break;
  case OMM_IMAGE_JPG:
    this->_status = __jpg_decode_from_mem(&this->_data, &this->_width, &this->_height, &this->_depth, false, data, size);
    break;
  case OMM_IMAGE_PNG:
    this->_status = __png_decode_from_mem(&this->_data, &this->_width, &this->_height, &this->_depth, false, data, size);
    break;
  case OMM_IMAGE_GIF:
    this->_status = __gif_decode_from_mem(&this->_data, &this->_width, &this->_height, &this->_depth, false, data, size);
    break;
  default:
    this->_status = OMM_IMAGE_ERR_FORMAT;
    return false;
  }

  return (this->_status == 1);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::save(const wstring& path, OmImageType type, int level)
{
  if(this->_data == nullptr) {
    this->_status = OMM_IMAGE_ERR_NODATA;
    return false;
  }

  // Open for writing
  FILE* fp;
  //fp = fopen(Om_toUtf8(path).c_str(), "wb");
  fp = _wfopen(path.c_str(), L"wb");
  if(!fp) {
    this->_status = OMM_IMAGE_ERR_OPEN;
    return false;
  }



  switch(type)
  {
  case OMM_IMAGE_BMP:
    this->_status = __bmp_encode_to_fp(fp, this->_data, this->_width, this->_height, this->_depth);
    break;
  case OMM_IMAGE_JPG:
    this->_status = __jpg_encode_to_fp(fp, level, this->_data, this->_width, this->_height, this->_depth);
    break;
  case OMM_IMAGE_PNG:
    this->_status = __png_encode_to_fp(fp, level, this->_data, this->_width, this->_height, this->_depth);
    break;
  case OMM_IMAGE_GIF:
    this->_status = __gif_encode_to_fp(fp, this->_data, this->_width, this->_height, this->_depth);
    break;
  default:
    this->_status = OMM_IMAGE_ERR_FORMAT;
    break;
  }

  fclose(fp);

  return (this->_status == 1);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::save(uint8_t** dest, size_t* size, OmImageType type, int level)
{
  if(this->_data == nullptr) {
    this->_status = OMM_IMAGE_ERR_NODATA;
    return false;
  }

  // initialize output values
  (*dest) = nullptr;
  (*size) = 0;

  switch(type)
  {
  case OMM_IMAGE_BMP:
    this->_status = __bmp_encode_to_mem(dest, size, this->_data, this->_width, this->_height, this->_depth);
    break;
  case OMM_IMAGE_JPG:
    this->_status = __jpg_encode_to_mem(dest, size, level, this->_data, this->_width, this->_height, this->_depth);
    break;
  case OMM_IMAGE_PNG:
    this->_status = __png_encode_to_mem(dest, size, level, this->_data, this->_width, this->_height, this->_depth);
    break;
  case OMM_IMAGE_GIF:
    this->_status = __gif_encode_to_mem(dest, size, this->_data, this->_width, this->_height, this->_depth);
    break;
  default:
    this->_status = OMM_IMAGE_ERR_FORMAT;
    return false;
    break;
  }

  return (this->_status == 1);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmImage::clear()
{
  if(this->_data) {
    delete [] this->_data;
    this->_data = nullptr;
    this->_width = 0;
    this->_height = 0;
    this->_depth = 0;
  }

  this->_status = OMM_IMAGE_ERR_NODATA;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::resize(unsigned dw, unsigned dh)
{
  if(this->_data == nullptr) {
    this->_status = OMM_IMAGE_ERR_NODATA;
    return false;
  }

  // gain of time
  if(dw == this->_width && dh == this->_height) {
    return true;
  }

  // compute channel count (bytes per pixel)
  unsigned c = this->_depth / 8;

  // we need an array of pointers, with one pointer per row
  uint8_t* buff;
  try {
    buff = new uint8_t[dw * c * dh];
  } catch(const std::bad_alloc&) {
    this->_status = OMM_IMAGE_ERR_ALLOC;
    return false;
  }

  if(dw > this->_width || dh > this->_height) {
    __image_upsample(buff, dw, dh, this->_data, this->_width, this->_height, c);
  } else {
    __image_downsample(buff, dw, dh, this->_data, this->_width, this->_height, c);
  }

  this->_width = dw;
  this->_height = dh;
  delete [] this->_data;
  this->_data = buff;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::crop(unsigned dx, unsigned dy, unsigned dw, unsigned dh)
{
  if(this->_data == nullptr) {
    this->_status = OMM_IMAGE_ERR_NODATA;
    return false;
  }

  if(dx + dw > this->_width || dy + dh > this->_height) {
    this->_status = OMM_IMAGE_ERR_ALLOC;
    return false;
  }

  unsigned c = this->_depth / 8;

  // we need an array of pointers, with one pointer per row
  uint8_t* buff;
  try {
    buff = new uint8_t[dw * c * dh];
  } catch(const std::bad_alloc&) {
    this->_status = OMM_IMAGE_ERR_ALLOC;
    return false;
  }

  uint8_t* sp;
  uint8_t* dp;

  for(unsigned y = 0; y < dh; ++y) {

    dp = &buff[(dw * c) * y];
    sp = &this->_data[this->_width * c * (dy + y) + (dx * c)];

    for(unsigned x = 0; x < dw; ++x) {

      dp[0] = sp[0];
      dp[1] = sp[1];
      dp[2] = sp[2];
      if(c == 4) dp[3] = sp[3];

      dp += c;
      sp += c;
    }
  }

  this->_width = dw;
  this->_height = dh;
  delete [] this->_data;
  this->_data = buff;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmImage::thumbnail(unsigned size)
{
  if(this->_data == nullptr) {
    this->_status = OMM_IMAGE_ERR_NODATA;
    return false;
  }

  if(this->_width != this->_height) {
    // crop image to square
    if(this->_width > this->_height) {
      if(!this->crop((this->_width * 0.5f) - (this->_height * 0.5f), 0, this->_height, this->_height))
        return false;
    } else {
      if(!this->crop(0, (this->_height * 0.5f) - (this->_width * 0.5f), this->_width, this->_width))
        return false;
    }
  }

  if(!this->resize(size, size))
    return false;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
HBITMAP OmImage::asBITMAP() const
{
  if(this->_data == nullptr) {
    return nullptr;
  }

  unsigned c = this->_depth / 8;
  unsigned sb = this->_width * c; // source data line size in bytes
  unsigned db = this->_width * 4; // destination data line size in bytes

  uint8_t* buff;
  try {
    buff = new uint8_t[db * this->_height];
  } catch(const std::bad_alloc&) {
    //this->_status = OMM_IMAGE_ERR_ALLOC;
    return nullptr;
  }

  uint8_t *dp, *sp;

  // Convert RGB(A) to BGRA
  switch(c)
  {
  case 3:
    for(unsigned y = 0; y < this->_height; ++y) {
      dp = &buff[db * y];
      sp = &this->_data[sb * y];
      for(unsigned x = 0; x < this->_width; ++x) {
        dp[0] = sp[2]; // B
        dp[1] = sp[1]; // G
        dp[2] = sp[0]; // R
        dp[3] = 0xff;  // A
        sp += c;
        dp += 4;
      }
    }
    break;
  case 4:
    for(unsigned y = 0; y < this->_height; ++y) {
      dp = &buff[db * y];
      sp = &this->_data[sb * y];
      for(unsigned x = 0; x < this->_width; ++x) {
        dp[0] = sp[2]; // B
        dp[1] = sp[1]; // G
        dp[2] = sp[0]; // R
        dp[3] = sp[3]; // A
        sp += c;
        dp += 4;
      }
    }
    break;
  }

  HBITMAP result = CreateBitmap(this->_width, this->_height, 1, 32, buff);

  delete [] buff;

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring OmImage::lastErrorStr() const
{
  wstring err;

  switch(this->_status)
  {
    case OMM_IMAGE_ERR_OPEN: err = L"File open error"; break;
    case OMM_IMAGE_ERR_READ: err = L"File read error"; break;
    case OMM_IMAGE_ERR_WRITE: err = L"File write error"; break;
    case OMM_IMAGE_ERR_ALLOC: err = L"Memory allocation error"; break;
    case OMM_IMAGE_ERR_DECODE: err = L"Image data decoding error"; break;
    case OMM_IMAGE_ERR_ENCODE: err = L"Image data encoding error"; break;
    case OMM_IMAGE_ERR_FORMAT: err = L"Unknow image format"; break;
    case OMM_IMAGE_ERR_NODATA: err = L"No image data loaded"; break;
    default: err = L"No error"; break;
  }

  return err;
}
