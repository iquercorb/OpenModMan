/*
 * MD4C-RTF: RTF Renderer for MD4C parser
 *
 * Copyright (c) 2022 Eric M.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * ----------------------------------------------------------------------------
 *
 * This program is not standalone but extension of the MD4C Markdown parser.
 * To successfully compile this program you'll need:
 *
 *  - The MD4C headers files.
 *  - The MD4C source files to be compiled.
 *     OR
 *  - The MD4 compiled version as linked library.
 *
 * To get MD4C please visit MD4C github page:
 *    http://github.com/mity/md4c
 */
#ifndef MD4C_RTF_H
#define MD4C_RTF_H

#include "md4c.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef unsigned char MD_RTF_DATA;
typedef char MD_RTF_CHAR;

/* If set, debug output from md_parse() is sent to stderr. */
#define MD_RTF_FLAG_DEBUG                   0x0001
#define MD_RTF_FLAG_VERBATIM_ENTITIES       0x0002
#define MD_RTF_FLAG_SKIP_UTF8_BOM           0x0004

int md_rtf(const MD_CHAR* input, MD_SIZE input_size,
            void (*process_output)(const MD_RTF_DATA*, MD_SIZE, void*),
            void* userdata, unsigned parser_flags, unsigned renderer_flags,
            unsigned font_size, unsigned doc_width);

#ifdef __cplusplus
    }  /* extern "C" { */
#endif

#endif // MD4C_RTF_H
