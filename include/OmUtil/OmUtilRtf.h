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
#ifndef OMUTILRTF_H
#define OMUTILRTF_H

#include "OmBaseWin.h"

struct OM_MD2RTF_CTX {
  uint8_t*  buf;
  size_t    cap;
  size_t    len;
  long      off;
  long      rem;
};

/// \brief Initialize MD2RTF Context
///
/// Initialize MD2RTF Context for further MD parsing and Edit stream operations.
///
/// \param[in]  ctx   : Pointer to OM_MD2RTF_CTX structure to initialize.
///
void Om_md2rtf_init(OM_MD2RTF_CTX* ctx);

/// \brief Free MD2RTF Context
///
/// Cleanup and free MD2RTF Context allocated data.
///
/// \param[in]  ctx   : Pointer to OM_MD2RTF_CTX structure to free.
///
void Om_md2rtf_free(OM_MD2RTF_CTX* ctx);

/// \brief Clear MD2RTF Context
///
/// Reset and clear working data without destroying buffers.
///
/// \param[in]  ctx   : Pointer to OM_MD2RTF_CTX structure to free.
///
void Om_md2rtf_clear(OM_MD2RTF_CTX* ctx);

/// \brief Render to RTF document
///
/// Parse the given text as Markdown and render as RTF document using
/// specified parameters.
///
/// \param[in]  ctx   : Pointer to MD2RTF Context.
/// \param[in]  fs    : RTF document base font size in points.
/// \param[in]  w     : RTF document page width in pixels.
///
void Om_md2rtf_render(OM_MD2RTF_CTX* ctx, const OmWString& text, unsigned fs, unsigned w);

/// \brief Autofit RTF tables
///
/// Adjusts RTF document tables width to fit control width. This
/// function is used to emulate table width autofit which does not
/// work in Rich Edit 4.1.
///
/// \param[in]  ctx   : Pointer to MD2RTF Context.
/// \param[in]  hwnd  : Handle to Rich Edit control to fit width to.
///
/// \return True if a least one table was resized, false otherwise.
///
bool Om_md2rtf_autofit(OM_MD2RTF_CTX* ctx, HWND hwnd);

/// \brief Stream-In RTF data
///
/// Stream-In RTF data to the specified Rich Edit Control.
///
/// \param[in]  ctx   : Pointer to MD2RTF Context.
/// \param[in]  hwnd  : Handle to Rich Edit control to stream to.
///
/// \return True if operation succeed, false otherwise.
///
bool Om_md2rtf_stream(OM_MD2RTF_CTX* ctx, HWND hwnd);

#endif // OMUTILRTF_H
