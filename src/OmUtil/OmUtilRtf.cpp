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
#include "OmUtilRtf.h"        //< OM_MD2RTF_CTX

#include "OmBaseWin.h"        //< SendMessage etc.
  #include <RichEdit.h>       //< EM_STREAMIN, SF_RTF, etc.

#include "md4c-rtf/md4c-rtf.h"

/// \brief Converts an integer value to string
///
/// Special implementation to converts an integer value to string using the
/// specified base and stores the result in the array given by str parameter.
///
/// This implementation does not add the null character at end of string in
/// order to allow to use the function in string replacement context.
///
/// \param[in]  value : Value to be converted to a string.
/// \param[in]  str   : Array in memory where to store the resulting string.
/// \param[in]  base  : Numerical base used to represent the value as a string.
///
/// \return A pointer to the end of the resulting string.
///
inline static char* __ultostr_replace(unsigned long value, char *str, int base)
{
  char buf[32];
  char *p = buf;
  int d;

  while(value || p == buf)  {
    d = value % base;
    *p++ = (d < 10) ? d+'0' : d+'a'-10;
    value /= base;
  }

  /* copy buffer to destination in reverse order */
  while(p > buf)
    *str++ = *--p;

  return str;
}

/// \brief MD4C parser options
///
/// Global options flag set for MD4C parser
///
#define MD4C_OPTIONS  MD_FLAG_UNDERLINE|MD_FLAG_TABLES|MD_FLAG_PERMISSIVEAUTOLINKS|MD_FLAG_NOHTML

/// \brief Callback for Markdown to RTF parser
///
/// Callback for MD4C RTF parser/renderer used to receive and store
/// rendered RTF data
///
/// \param[in]  data  : Pointer to RTF data.
/// \param[in]  size  : Size of RTF data.
/// \param[in]  ptr   : Pointer to user data.
///
static void __md2rtf_render_cb(const uint8_t* data, unsigned size, void* ptr)
{
  OM_MD2RTF_CTX* ctx = reinterpret_cast<OM_MD2RTF_CTX*>(ptr);

  // check for need to alloc or realloc static buffer
  if(ctx->len + size > ctx->cap) {
    ctx->cap *= 2;
    ctx->buf = reinterpret_cast<uint8_t*>(Om_realloc(ctx->buf, ctx->cap));
  }

  // copy data to buffer
  memcpy(ctx->buf + ctx->len, data, size);

  // increment written length
  ctx->len += size;
}


/// \brief Callback for Rich Edit stream
///
/// Callback Rich Edit input stream, used to send RTF data to
/// Rich Edit control.
///
/// \param[in]  ptr       : Pointer to user data.
/// \param[in]  buff      : Destination buffer where to write RTF data.
/// \param[in]  size      : Destination buffer size.
/// \param[out] writ      : Count of bytes actually written to buffer.
///
static DWORD CALLBACK __edit_instream_cb(DWORD_PTR ptr, LPBYTE buff, LONG size, LONG* writ)
{
  OM_MD2RTF_CTX* ctx = reinterpret_cast<OM_MD2RTF_CTX*>(ptr);

  if(ctx->rem) {
    if(size <= ctx->rem) {
      memcpy(buff, ctx->buf + ctx->off, size);
      ctx->off += size;
      ctx->rem -= size;
      *writ = size;
    } else {
      *writ = ctx->rem;
      memcpy(buff, ctx->buf + ctx->off, ctx->rem);
      ctx->rem = 0;
    }
  } else {
    *writ = 0;
  }
  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_md2rtf_init(OM_MD2RTF_CTX* ctx)
{
  // initial allocation if required
  if(!ctx->buf) {
    ctx->buf = reinterpret_cast<uint8_t*>(Om_alloc(4096));
    ctx->cap = 4096;
  }

  ctx->len = 0;
  ctx->off = 0;
  ctx->rem = 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_md2rtf_free(OM_MD2RTF_CTX* ctx)
{
  if(ctx->buf) {
    Om_free(ctx->buf);
    ctx->buf = 0;
    ctx->cap = 0;
  }

  ctx->len = 0;
  ctx->off = 0;
  ctx->rem = 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_md2rtf_clear(OM_MD2RTF_CTX* ctx)
{
  if(ctx->buf) {
    ctx->buf[0] = '\0';
  }
  ctx->len = 0;
  ctx->off = 0;
  ctx->rem = 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_md2rtf_render(OM_MD2RTF_CTX* ctx, const OmWString& text, unsigned fs, unsigned w)
{
  // convert width pixels to millimeters
  w *= 0.2645833333f;

  // we fix a minimum width to ensure all \cellxN values will be greater or
  // equal to 10000, so we can safely perform string replacement for resizing
  //
  // notice that this value is fixed empirically according the minimum size
  // of a table cell that the MD2RTF renderer currently output, which is 90%
  // of page width.
  //
  // There is no other reason to fix a minimum width except to allow the fast
  // resize function to work properly.
  if(w < 197) w = 197;

  // initialize offsets
  ctx->len = 0;

  // perform Markdown parse to RTF render
  md_rtf(text.data(), text.size(), __md2rtf_render_cb, ctx,
         MD4C_OPTIONS, MD_RTF_FLAG_SKIP_UTF8_BOM, fs, w);

  // add null char to RTF data
  ctx->buf[ctx->len] = '\0';
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_md2rtf_autofit(OM_MD2RTF_CTX* ctx, HWND hwnd)
{
  long rect[4];
  float scale, width;
  unsigned cellx;
  char* p, * s;

  if(!ctx->len)
    return false;

  bool changed = false;

  // get control client width
  GetClientRect(hwnd, reinterpret_cast<LPRECT>(&rect));
  width = 15.0f * rect[2]; //< pixels to twips

  s = reinterpret_cast<char*>(ctx->buf);

  while((s = strstr(s, "\\trowd"))) {

    if((p = strstr(s, "\\trwWidth"))) {

      s = p + 9; //< seek to number

      // convert \trwWidth to scale 0.0 - 1.0
      scale = 0.0002f * strtoul(s, nullptr, 10);

      if((p = strstr(s, "\\cellx"))) {

        s = p + 6; //< seek to number

        // calculate cell width from given width and parsed scale
        cellx = unsigned(scale * width);

        // we keep minimum value to 10000 to properly replace all digits of
        // the previous \cellx value. We expect exactly 5 digits as the minimum
        // size of the document was previously fixed in Om_md2rtf_render() to
        // ensure this.
        //
        // Notice that a value with more than 5 digits approximately corresponds
        // to a width greater than 6600 pixels, near impossible to reach.
        if(cellx < 10000) cellx = 10000;

        // convert number to string, replacing previous \cellx value digits and
        // without adding null-character to the end.
        s = __ultostr_replace(cellx, s, 10);

        // at least one element changed
        changed = true;
      }
    }
  }

  return changed;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool Om_md2rtf_stream(OM_MD2RTF_CTX* ctx, HWND hwnd)
{
  // initialize offsets
  ctx->off = 0;
  ctx->rem = ctx->len;

  // send RTF data to Rich Edit
  EDITSTREAM es = {};
  es.pfnCallback = __edit_instream_cb;
  es.dwCookie = reinterpret_cast<DWORD_PTR>(ctx);

  if(SendMessage(hwnd, EM_STREAMIN, SF_RTF, reinterpret_cast<LPARAM>(&es))) {
    return (es.dwError == 0);
  }

  return false;
}
