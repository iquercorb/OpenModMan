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
#include <stdio.h>
#include <stdlib.h>     /* strtoul */
#include <string.h>

#include "md4c-rtf.h"
#include "entity.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199409L
    /* C89/90 or old compilers in general may not understand "inline". */
    #if defined __GNUC__
        #define inline __inline__
    #elif defined _MSC_VER
        #define inline __inline
    #else
        #define inline
    #endif
#endif

#ifdef _WIN32
    #define snprintf _snprintf
#endif

/* Implementation for long unsigned number conversion to string

Converts an long unsigned integer value to a null-terminated string
using the specified base and stores the result in the array given
by str parameter. */
static char*
ultostr(long unsigned value, char *str, int base, char trail)
{
  char buf[32];
  char *pb = buf;
  char *ps = str;
  int d;

  while(value || pb == buf)  {
    d = value % base;
    *pb++ = (d < 10) ? d+'0' : d+'a'-10;
    value /= base;
  }

  /* copy buffer to destination in reverse order */
  while(pb > buf)
    *ps++ = *--pb;

  /* add the optional trailing char */
  if(trail) *ps++ = trail;

  *ps++ = '\0';

  return str;
}

#ifdef _DEBUG
/* special print to standard ouput for debug purposes */
static
void printf_dbg(const char* str, size_t max)
{
  while(max--) {
    switch(*str) {
    case '\a': putc('\\', stdout); putc('a', stdout); str++; break;
    case '\b': putc('\\', stdout); putc('b', stdout); str++; break;
    case '\f': putc('\\', stdout); putc('f', stdout); str++; break;
    case '\n': putc('\\', stdout); putc('n', stdout); str++; break;
    case '\r': putc('\\', stdout); putc('r', stdout); str++; break;
    case '\t': putc('\\', stdout); putc('t', stdout); str++; break;
    case '\v': putc('\\', stdout); putc('v', stdout); str++; break;
    case '\0': putc('\\', stdout); putc('0', stdout); str++; break;
    default: putc(*str++, stdout); break;
    }
  }
}

/* print tabulation space to standard output, for debug purpose */
static
void printf_tabs(int size, int count)
{
  if(count > 0) {
    int n = size * count;
    while(n--) putchar(' ');
  }
}
#endif

/* Structure that hold list parameters for ordered or unordered list render */
typedef struct MD_RTF_list {
  unsigned            type;   /* ordered or unordered */
  unsigned            count;  /* item counter (incremeted) */
  unsigned            start;  /* item start number */
  unsigned            loose;  /* tight or loose list type */
  const MD_RTF_CHAR*  cw_tx;  /* bullet or delimiter character */
  const MD_RTF_CHAR*  cw_li;  /* indentation space to apply */
  const MD_RTF_CHAR*  cw_sb;  /* space-before to apply */
  const MD_RTF_CHAR*  cw_sa;  /* space-after to apply */
} MD_RTF_LIST;

#define MD_RTF_LIST_TYPE_UL   0x0
#define MD_RTF_LIST_TYPE_OL   0x1

static const MD_RTF_CHAR* g_cw_list_bullt[2] = {
  "\\u8226 ",
  "\\u9702 "};

static const MD_RTF_CHAR* g_cw_list_delim[2] = {
  ".",
  ")"};

typedef struct MD_RTF_tag {
  void        (*process_output)(const MD_RTF_DATA*, MD_SIZE, void*);
  void*       userdata;
  unsigned    flags;
  char        escape_map[256];
  /* RTF document page sizes (twip) */
  unsigned    page_width;
  unsigned    page_height;
  unsigned    page_margin;
  /* Normal font base size (half-point) */
  unsigned    font_base;
  /* list render process variables */
  MD_RTF_LIST list[8];
  int         list_dpth;
  unsigned    list_rset;
  unsigned    list_para;
  /* table render process variables */
  unsigned    tabl_cols;
  unsigned    tabl_head;
  /* do not end paragraph flag */
  unsigned    quot_blck;
  /* block code must render LF flag */
  unsigned    code_lf;
  /* RTF control words with prebuilt values */
  MD_RTF_CHAR cw_fs[2][8];
  MD_RTF_CHAR cw_hf[6][24];
  MD_RTF_CHAR cw_sa[2][16];
  MD_RTF_CHAR cw_sb[2][16];
  MD_RTF_CHAR cw_li[8][16];
  MD_RTF_CHAR cw_tr[2][72];
  MD_RTF_CHAR cw_fi[2][16];
  MD_RTF_CHAR cw_cx[2][16];
} MD_RTF;

#define NEED_RTF_ESC_FLAG   0x1
#define NEED_URL_ESC_FLAG   0x2
#define NEED_PRE_ESC_FLAG   0x4

/******************************************
 ***   RTF rendering helper functions   ***
 ******************************************/

#define ISDIGIT(ch)     ('0' <= (ch) && (ch) <= '9')
#define ISLOWER(ch)     ('a' <= (ch) && (ch) <= 'z')
#define ISUPPER(ch)     ('A' <= (ch) && (ch) <= 'Z')
#define ISALNUM(ch)     (ISLOWER(ch) || ISUPPER(ch) || ISDIGIT(ch))

#ifdef MD4C_USE_UTF16
/* Function to convert WCHAR to UTF8 string then forward result to the
specified render functions */
static void
render_wchar(MD_RTF* r, const wchar_t* text, MD_SIZE size,
              void (*fn_forward)(MD_RTF*, const MD_RTF_CHAR*, MD_SIZE))
{
  char utf8[6144];
  unsigned utf8_size;

  while(size > 2048) {
    utf8_size = WideCharToMultiByte(CP_UTF8, 0, text, 2048, utf8, 6144, NULL, NULL);
    fn_forward(r, utf8, utf8_size);
    text += 2048;
    size -= 2048;
  }

  utf8_size = WideCharToMultiByte(CP_UTF8, 0, text, size, utf8, 6144, NULL, NULL);
  fn_forward(r, utf8, utf8_size);
}
#endif


static inline void
render_verbatim(MD_RTF* r, const MD_RTF_CHAR* text, MD_SIZE size)
{
  r->process_output((MD_RTF_DATA*)text, size, r->userdata);
}

/* Keep this as a macro. Most compiler should then be smart enough to replace
 * the strlen() call with a compile-time constant if the string is a C literal. */
#define RENDER_VERBATIM(r, verbatim)                                    \
        render_verbatim((r), (verbatim), (MD_SIZE) (strlen(verbatim)))


static void
render_url_escaped(MD_RTF* r, const MD_RTF_CHAR* data, MD_SIZE size)
{
  static const MD_RTF_CHAR hex_chars[] = "0123456789ABCDEF";
  MD_OFFSET beg = 0;
  MD_OFFSET off = 0;

  /* Some characters need to be escaped in URL attributes. */
  #define NEED_URL_ESC(ch)    (r->escape_map[(unsigned char)(ch)] & NEED_URL_ESC_FLAG)

  while(1) {

    while(off < size  &&  !NEED_URL_ESC(data[off]))
      off++;

    if(off > beg)
      render_verbatim(r, data + beg, off - beg);

    if(off < size) {

      char hex[3];

      switch(data[off])
      {
      case '&':
        render_verbatim(r, "&amp;", 5);
        break;
      default:
        hex[0] = '%';
        hex[1] = hex_chars[((unsigned)data[off] >> 4) & 0xf];
        hex[2] = hex_chars[((unsigned)data[off] >> 0) & 0xf];
        render_verbatim(r, hex, 3);
        break;
      }

      off++;

    } else {
      break;
    }

    beg = off;
  }
}

static inline void
render_unicode(MD_RTF* r, unsigned u)
{
  MD_RTF_CHAR str_ucp[16];
  render_verbatim(r, "\\u", 2);
  RENDER_VERBATIM(r, ultostr(u, str_ucp, 10, ' ')); //< add space after number
}


static inline void
render_cp1252(MD_RTF* r, unsigned u)
{
  MD_RTF_CHAR str_acp[16];
  render_verbatim(r, "\\'", 2);
  RENDER_VERBATIM(r, ultostr(u, str_acp, 16, 0)); //< add space after number
}

static unsigned
render_non_ascii(MD_RTF* r, const unsigned char* c, unsigned size)
{
  unsigned u = 0;
  unsigned b = 0;

  #define IS_UTF8_2BYTES(a) (((a)[1] & 0xC0) == 0x80)
  #define IS_UTF8_3BYTES(a) (((a)[1] & 0xC0) == 0x80) && \
                            (((a)[2] & 0xC0) == 0x80)
  #define IS_UTF8_4BYTES(a) (((a)[1] & 0xC0) == 0x80) && \
                            (((a)[2] & 0xC0) == 0x80) && \
                            (((a)[3] & 0xC0) == 0x80) && \

  /* validate and decode UTF-8 */
  if((c[0] & 0xE0) == 0xC0) { /* 110X XXXX : 2 octets */
    if(size > 1) { /* need one more octet */
      if(IS_UTF8_2BYTES(c)) {
        u = (c[0] & 0x1F) <<  6 |
            (c[1] & 0x3F);
        b = 2; /* skip 2 byte */
      }
    }
  } else if((c[0] & 0xF0) == 0xE0) { /* 1110 XXXX : 3 octets */
    if(size > 2) { /* need 2 more octets */
      if(IS_UTF8_3BYTES(c)) {
        u = (c[0] & 0x0F) << 12 |
            (c[1] & 0x3F) <<  6 |
            (c[2] & 0x3F);
        b = 3; /* skip 3 byte */
      }
    }
  } else if((c[0] & 0xF8) == 0xF0) { /* 1111 0XXX : 4 octets */
    if(size > 3) { /* need 3 more octets */
      if(IS_UTF8_2BYTES(c)) {
        u = (c[0] & 0x07) << 18 |
            (c[1] & 0x3F) << 12 |
            (c[2] & 0x3F) <<  6 |
            (c[3] & 0x3F);
        b = 4; /* skip 4 byte */
      }
    }
  }

  /* check if we got a valid Unicode codepoint */
  if(b > 0) {

    render_unicode(r, u);

  } else {

    /* if we don't got a valid Unicode codepoint we assume an AINSI CP1252
    encoding. We translate it to RTF using old standard escaping for 8-bit
    non ASCII characters. */
    render_cp1252(r, *c);

    return 1; /* skip 1 byte */
  }

  return b;
}

static void
render_rtf_escaped(MD_RTF* r, const MD_RTF_CHAR* data, MD_SIZE size)
{
  MD_OFFSET beg = 0;
  MD_OFFSET off = 0;

  /* Some characters need to be escaped in normal RTF text. */
  #define NEED_RTF_ESC(ch)   (r->escape_map[(unsigned char)(ch)] & NEED_RTF_ESC_FLAG)

  while(1) {
    /* Optimization: Use some loop unrolling. */
    while(off + 3 < size  &&  !NEED_RTF_ESC(data[off+0])  &&  !NEED_RTF_ESC(data[off+1])
                          &&  !NEED_RTF_ESC(data[off+2])  &&  !NEED_RTF_ESC(data[off+3]))
      off += 4;

    while((off < size) && !NEED_RTF_ESC(data[off]))
      off++;

    if(off > beg)
      render_verbatim(r, data + beg, off - beg);

    if(off < size) {

      // escape Non-ASCII characters
      if((unsigned char)data[off] > 0x7F) {
        off += render_non_ascii(r, (unsigned char*)(data + off), size - off);
      } else {
        // escape RTF reserved characters
        switch(data[off]) {
          case '\\': render_verbatim(r, "\\\\", 2); break;
          case '{' : render_verbatim(r, "\\{", 2); break;
          case '}' : render_verbatim(r, "\\}", 2); break;
          case '\n': render_verbatim(r, "\\line1", 6); break;
        }
        off++;
      }

    } else {
      break;
    }

    beg = off;
  }
}

static void
render_text_code(MD_RTF* r, const MD_RTF_CHAR* data, MD_SIZE size)
{
  /* due to how MD4C parser works, a last LF '\n' is always emitted as last
  text block before leaving the code block which produce a visible empty line
  at the end of block. To avoid this, we need to handle line feeds manually
  inside the block code.

  To do so, we skip each input '\n' then rather manually put a line feed at
  start of the next text block. This way, the last received LF is never
  rendered. */

  /* if input data is LF we ignore it and simply set the flag make a line
  feed at start of the next text input */
  if(data[0] == '\n' && size == 1) {
    r->code_lf = 1;
    return;
  }

  /* previous input was LF, we now render the line feed */
  if(r->code_lf) {
    render_verbatim(r, "\\line1", 6);
    r->code_lf = 0;
  }

  /* render input text as normal text */
  render_rtf_escaped(r, data, size);
}


/* Translate entity to its UTF-8 equivalent, or output the verbatim one
 * if such entity is unknown (or if the translation is disabled). */
static void
render_entity(MD_RTF* r, const MD_RTF_CHAR* text, MD_SIZE size)
{
  if(r->flags & MD_RTF_FLAG_VERBATIM_ENTITIES) {
    render_verbatim(r, text, size);
    return;
  }

  /* We assume Unicode output is what is desired. */
  if(size > 3 && text[1] == '#') {

    unsigned u = 0;

    if(text[2] == 'x' || text[2] == 'X') {
      u = strtoul(text + 3, NULL, 16);
    } else {
      u = strtoul(text + 2, NULL, 10);
    }

    render_unicode(r, u);

    return;

  } else {
    #ifdef MD4C_ENTITY_H
    /* Named entity (e.g. "&nbsp;"). */
    const struct entity* ent;

    ent = entity_lookup(text, size);
    if(ent != NULL) {

      /* we do not support > 4 bytes unicode for now */
      if(!ent->codepoints[1])
        render_unicode(r, ent->codepoints[0]);

      return;
    }
    #endif
  }

  render_rtf_escaped(r, text, size);
}


static void
render_font_norm(MD_RTF* r)
{
  /* change font style with the normal font (#0 : Calibri) */

  render_verbatim(r, "\\f0", 3);
  render_verbatim(r, r->cw_fs[0], 5);
}

static void
render_font_mono(MD_RTF* r)
{
  /* change paragraph style to monospace font (#1 :. Courrier New )
  with its dedicated size, little smaller than the normal */

  render_verbatim(r, "\\f1", 3);
  render_verbatim(r, r->cw_fs[1], 5);
}

static inline void
render_end_block(MD_RTF* r)
{
  /* this sequence is used to put standard paragraph space after a special
  block such as list, quote or code, where space-after value is usually
  changed or disabled. */

  /* reset to normal font font but without space after */
  render_verbatim(r, "\\pard\\f0", 8);
  render_verbatim(r, r->cw_fs[0], 5);

  /* end paragraph, notice that CRLF is here
  only for readability of source data */
  render_verbatim(r, "\\sb0\\sa0\\par\r\n", 14);
}

static inline void
render_list_start(MD_RTF* r)
{
  MD_RTF_CHAR str_num[16];

  unsigned d = r->list_dpth;

  #ifdef _DEBUG
  printf_tabs(2, d);
  printf("++ render_list_start (%u)\n", r->list[d].count);
  #endif

  /* if this is a reset after nested list we end the last item paragraph */
  if(r->list_rset)
    render_verbatim(r, "\\par", 4);

  render_verbatim(r, "\\pard", 5); /* reset paragraph */
  render_verbatim(r, r->cw_fs[0], 5);  /* normal font size */
  render_verbatim(r, "{\\pntext\\f0 ", 12);  /* normal font size */

  if(r->list[d].type == MD_RTF_LIST_TYPE_OL) {  /* OL */

    /* we need to the start number added to item count because
    this can be a list restart after the end of nested list */
    ultostr(r->list[d].count + r->list[d].start, str_num, 10, 0);

    RENDER_VERBATIM(r, str_num);
    render_verbatim(r, r->list[d].cw_tx, 1); /* delimiter character */
    render_verbatim(r, "\\tab}{\\*\\pn\\pnlvlbody\\pnf0\\pnstart", 34);
    RENDER_VERBATIM(r, str_num);
    render_verbatim(r, "\\pndec{\\pntxta", 14);
  } else {                                      /* UL */
    render_verbatim(r, r->list[d].cw_tx, 7); /* bullet character */
    render_verbatim(r, "\\tab}{\\*\\pn\\pnlvlblt\\pnf0{\\pntxtb", 33);
  }

  RENDER_VERBATIM(r, r->list[d].cw_tx); /* either delimiter or bullet char */
  render_verbatim(r, "}}", 2);
  RENDER_VERBATIM(r, r->list[d].cw_li); /* \liN */
  RENDER_VERBATIM(r, r->list[d].cw_sb); /* \sbN */
  RENDER_VERBATIM(r, r->list[d].cw_sa); /* \saN */

  if(r->list[d].type == MD_RTF_LIST_TYPE_OL) {  /* OL */
    RENDER_VERBATIM(r, r->cw_fi[1]); /* \fiN */
  } else {                                      /* UL */
    RENDER_VERBATIM(r, r->cw_fi[0]); /* \fiN */
  }

  r->list_rset = 0;
}

static inline void
render_list_item(MD_RTF* r)
{
  MD_RTF_CHAR str_num[16];

  unsigned d = r->list_dpth;

  #ifdef _DEBUG
  printf_tabs(2, d);
  printf("++ render_list_item (%u)\n", r->list[d].start);
  #endif

  /* reset paragraph */
  render_verbatim(r, "\\par{\\pntext\\f0 ", 16);

  if(r->list[d].type == MD_RTF_LIST_TYPE_OL) { /* OL */
    ultostr(r->list[d].count + r->list[d].start, str_num, 10, 0);
    RENDER_VERBATIM(r, str_num);
    render_verbatim(r, r->list[d].cw_tx, 1); /* delimiter char */
  } else {
    render_verbatim(r, r->list[d].cw_tx, 7); /* bullet char */
  }

  render_verbatim(r, "\\tab}", 5);
}

static void
render_enter_block_doc(MD_RTF* r)
{
  MD_RTF_CHAR str_page[512];

  RENDER_VERBATIM(r,  "{\\rtf1\\ansi\\ansicpg1252\\deff0"
                        /* font table */
                        "{\\fonttbl"
                          "{\\f0\\fswiss Calibri;}"
                          "{\\f1\\fmodern Consolas;}"
                          "{\\f2\\fnil Symbol;}"
                        "}"
                        /* color table */
                        "{\\colortbl;"
                          "\\red0\\green0\\blue0;"        /* black */
                          "\\red255\\green255\\blue255;"  /* white */
                          "\\red180\\green180\\blue180;"  /* gray */
                          "\\red0\\green102\\blue204;"    /* blue */
                          "\\red240\\green240\\blue240;"  /* silver */
                          "\\red90\\green90\\blue90;"     /* dark gray */
                        "}"
                        /* additional informations */
                        "{\\*\\generator MD4C-RTF}\\viewkind5");


                        /* document parameters */
  sprintf(str_page,     "\\paperw%u\\paperh%u"
                        "\\margl%u\\margr%u\\margt%u\\margb%u",
                        r->page_width, r->page_height,
                        r->page_margin, r->page_margin, r->page_margin, r->page_margin);

  RENDER_VERBATIM(r, str_page);

                        /* document initialization */
  RENDER_VERBATIM(r, "\\uc0\r\n\\pard");

  /* default space after and before */
  render_verbatim(r, "\\sb0\\sa0 ", 9);
}

static void
render_leave_block_doc(MD_RTF* r)
{
  render_verbatim(r, "}\0", 2);
}

static void
render_enter_block_hr(MD_RTF* r)
{
  /* RTF does not allow (in simple way) to display an horizontal rule, on the
  other hand, Rich Edit 4.1 (WordPad and WinAPI Rich Edit control) does not
  properly handle paragraph borders and displays table borders in its own
  specific way.

  So, to display an horizontal rule equally on all viewer, we create a table
  with the minimum size (font size defined to 0) with only the bottom border
  visible. Other border are also visible but defined with the same color as
  background, this way Rich Edit 4.1 don't display them in light gray. */

  render_verbatim(r, "\\pard\\f0\\fs0\\trowd\\trrh-1\\trftsWidth2\\trwWidth5000\\trautofit1" /* 61 bytes */
                      "\\clbrdrt\\brdrs\\brdrw1\\brdrcf2" /* 29 bytes */
                      "\\clbrdrb\\brdrs\\brdrw1\\brdrcf3"
                      "\\clbrdrl\\brdrs\\brdrw1\\brdrcf2"
                      "\\clbrdrr\\brdrs\\brdrw1\\brdrcf2", 177);

  RENDER_VERBATIM(r, r->cw_cx[1]); // \cellxN
  render_verbatim(r, "\\cell\\row", 9);

  /* create proper space after paragraph */
  render_end_block(r);
}

static void
render_enter_block_h(MD_RTF* r, const MD_BLOCK_H_DETAIL* h)
{
  RENDER_VERBATIM(r, r->cw_hf[h->level - 1]);
}

static void
render_leave_block_h(MD_RTF* r, const MD_BLOCK_H_DETAIL* h)
{
  if(h->level > 3) {
    render_verbatim(r, "\\b0\\i0 \\par\r\n", 13);
  } else {
    render_verbatim(r, "\\b0 \\par\r\n", 10);
  }
}

static void
render_enter_block_quote(MD_RTF* r)
{
  /* reset paragraph to normal font style */
  render_verbatim(r, "\\pard\\f0", 8);
  render_verbatim(r, r->cw_fs[0], 5);

  /* start table row with proper parameters */
  render_verbatim(r, "\\cf6\\i\\trowd", 12);
  RENDER_VERBATIM(r, r->cw_tr[0]);

  /* quote is enclosed in a table with only the left border visible */
  render_verbatim(r,  "\\clbrdrt\\brdrs\\brdrw1\\brdrcf2"   /* invisible border */
                      "\\clbrdrb\\brdrs\\brdrw1\\brdrcf2"   /* invisible border */
                      "\\clbrdrl\\brdrs\\brdrw70\\brdrcf3"
                      "\\clbrdrr\\brdrs\\brdrw1\\brdrcf2", 117); /* invisible border */

  /* cell width fixed to 90% of page width */
  RENDER_VERBATIM(r, r->cw_cx[0]); /* \cellxN */

  /* prevent space-after and line feed at end of paragraph */
  r->quot_blck = 1;
}

static void
render_leave_block_quote(MD_RTF* r)
{
  render_verbatim(r, "\\intbl\\cell\\row\\i0\\cf1", 21);

  /* create proper space after paragraph */
  render_end_block(r);

  /* we can now treat paragraphs normally */
  r->quot_blck = 0;
}

static void
render_enter_block_code(MD_RTF* r)
{
  /* reset paragraph to monospace font style */
  render_verbatim(r, "\\pard\\f1", 8);
  render_verbatim(r, r->cw_fs[1], 5);
  /* add space before and space after to simulate padding*/
  RENDER_VERBATIM(r, r->cw_sa[1]);
  RENDER_VERBATIM(r, r->cw_sb[1]);

  /* start table row with proper parameters */
  render_verbatim(r, "\\trowd", 6);
  RENDER_VERBATIM(r, r->cw_tr[0]);

  /* code is enclosed in gray block */

  render_verbatim(r,  "\\clbrdrt\\brdrs\\brdrw1\\brdrcf5"  /* invisible border */
                      "\\clbrdrb\\brdrs\\brdrw1\\brdrcf5"  /* invisible border */
                      "\\clbrdrl\\brdrs\\brdrw70\\brdrcf3"
                      "\\clbrdrr\\brdrs\\brdrw1\\brdrcf2" /* invisible border */
                      "\\clcbpat5", /*117*/ 126);


  /* cell width fixed to 90% of page width */
  RENDER_VERBATIM(r, r->cw_cx[0]); /* \cellxN */
}

static void
render_leave_block_code(MD_RTF* r)
{
  render_verbatim(r, "\\cell\\row", 9);

  /* create proper space after paragraph */
  render_end_block(r);
}

static void
render_enter_block_ul(MD_RTF* r, const MD_BLOCK_UL_DETAIL* ul)
{
  /* increment depth */
  unsigned d = ++r->list_dpth;

  #ifdef _DEBUG
  printf_tabs(2, d);
  printf("=> render_enter_block_ul\n");
  #endif

  /* nested list, we close the previous paragraph */
  if(d > 0) render_verbatim(r, "\\par", 4);

  /* do we need more than 8 levels of nested list ? */
  if(d > 7) return;

  /* initialize list parameters */
  r->list[d].type = MD_RTF_LIST_TYPE_UL;
  /* current item count */
  r->list[d].count = 0;
  /* tight or lose list */
  r->list[d].loose = !ul->is_tight;
  /* bullet character */
  r->list[d].cw_tx = g_cw_list_bullt[d % 2];
  /* space-after \sbN to use */
  r->list[d].cw_sb = ul->is_tight ? r->cw_sb[0] : r->cw_sb[1];
  /* space-after \saN to use */
  r->list[d].cw_sa = ul->is_tight ? r->cw_sa[0] : r->cw_sa[1];
  /* left-indent \liN to use */
  r->list[d].cw_li = r->cw_li[d];

  /* start new list paragraph */
  render_list_start(r);
}

static void
render_leave_block_ul(MD_RTF* r)
{
  #ifdef _DEBUG
  for(int i = 0; i < (r->list_dpth*2); ++i) putchar(' ');
  printf("<= render_leave_block_ul\n");
  #endif

  if(r->list_dpth) {
    /* nested list ended, we must setup block again */
    r->list_rset = 1;
  } else {
    /* properly end the last item paragraph*/
    render_verbatim(r, "\\par", 4);
    /* all ended, create proper space after paragraph */
    render_end_block(r);
    /* properly reset parameters */
    r->list_para = 0;
    r->list_rset = 0;
  }

  /* decrement depth */
  r->list_dpth--;
}

static void
render_enter_block_ol(MD_RTF* r, const MD_BLOCK_OL_DETAIL* ol)
{
  /* increment depth */
  unsigned d = ++r->list_dpth;

  #ifdef _DEBUG
  printf_tabs(2, d);
  printf("=> render_enter_block_ol\n");
  #endif

  /* nested list, we close the previous paragraph */
  if(d > 0) render_verbatim(r, "\\par", 4);

  /* do we need more than 8 levels of nested list ? */
  if(d > 7) return;

  /* initialize list parameters */
  r->list[d].type = MD_RTF_LIST_TYPE_OL;
  /* current item count */
  r->list[d].count = 0;
  /* item start number */
  r->list[d].start = ol->start;
  /* tight or lose list */
  r->list[d].loose = !ol->is_tight;
  /* delimiter character */
  r->list[d].cw_tx = (ol->mark_delimiter == ')')  ? g_cw_list_delim[1]
                                                  : g_cw_list_delim[0];
  /* space-after \sbN to use */
  r->list[d].cw_sb = ol->is_tight ? r->cw_sb[0] : r->cw_sb[1];
  /* space-after \saN to use */
  r->list[d].cw_sa = ol->is_tight ? r->cw_sa[0] : r->cw_sa[1];
  /* left-indent \liN to use */
  r->list[d].cw_li = r->cw_li[d];

  /* start new list paragraph */
  render_list_start(r);
}

static void
render_leave_block_ol(MD_RTF* r)
{
  #ifdef _DEBUG
  printf_tabs(2, r->list_dpth);
  printf("<= render_leave_block_ol\n");
  #endif

  if(r->list_dpth) {
    /* nested list ended, we must setup block again */
    r->list_rset = 1;
  } else {
    /* properly end the last item paragraph*/
    render_verbatim(r, "\\par", 4);
    /* all ended, create proper space after paragraph */
    render_end_block(r);
    /* properly reset parameters */
    r->list_para = 0;
    r->list_rset = 0;
  }

  /* decrement depth */
  r->list_dpth--;
}

static void
render_enter_block_li(MD_RTF* r, const MD_BLOCK_LI_DETAIL* li)
{
  /* get depth */
  unsigned d = r->list_dpth;

  #ifdef _DEBUG
  printf_tabs(2, d);
  printf("=> render_enter_block_li\n");
  #endif


  /* reset paragraph counter */
  r->list_para = 0;

  /* if we just leave a nested list, we may need to start a new paragraph
  with proper parameters */
  if(r->list_rset) {
    render_list_start(r);
    return;
  }

  /* if this is the fist item of list, proper data is already
  written during pn block start */
  if(r->list[d].count > 0)
    render_list_item(r);

  /* increment item count */
  r->list[d].count++;
}

static void
render_leave_block_li(MD_RTF* r)
{
  #ifdef _DEBUG
  printf_tabs(2, r->list_dpth);
  printf("<= render_leave_block_li\n");
  #endif

  /* Unlike HTML, RTF do not work with opened blocks which must be closed but
  by ending a paragraph which automatically begins a new one, which produce
  a line feed.

  considering this behavior, ending a paragraph each time a block is closed
  would produce inconsistent line feeds. This is especially problematic with
  nested lists where we can have cascading LI blocks closures.

  To avoid this problem, we only end paragraph when entering a new item */
}

static void
render_enter_block_table(MD_RTF* r, const MD_BLOCK_TABLE_DETAIL* tb)
{
  /* we hold column count to calculate cell width */
  r->tabl_cols = tb->col_count;

  /* start new table with smaller font and horizontal align to center */
  render_verbatim(r, "\\pard\\f0", 8);
  render_verbatim(r, r->cw_fs[1], 5);
}

static inline void
render_leave_block_table(MD_RTF* r)
{
  r->tabl_cols = 0;

  /* all ended, create proper space after paragraph */
  render_end_block(r);
}

static inline void
render_enter_block_thead(MD_RTF* r)
{
  /* simply set a flag to know we are in table head */
  r->tabl_head = 1;
}

static inline void
render_leave_block_thead(MD_RTF* r)
{
  /* we are no longer in table head */
  r->tabl_head = 0;
}

static void
render_enter_block_tr(MD_RTF* r)
{
  MD_RTF_CHAR str_num[16];

  /* create new raw with proper parameters */
  render_verbatim(r, "\\trowd", 6);
  RENDER_VERBATIM(r, r->cw_tr[1]);

  /* 9000 seem to be the average width of an RTF document */
  float tw = 0.9f * r->page_width;
  unsigned cw = tw / r->tabl_cols;

  /* we must first declare cells with their respecting properties */
  for(unsigned i = 0; i < r->tabl_cols; ++i) {

    render_verbatim(r,  "\\clvertalc" /* vertical-align center */
                        "\\clbrdrt\\brdrs\\brdrw20\\brdrcf3" //< 30 bytes
                        "\\clbrdrb\\brdrs\\brdrw20\\brdrcf3"
                        "\\clbrdrl\\brdrs\\brdrw20\\brdrcf3"
                        "\\clbrdrr\\brdrs\\brdrw20\\brdrcf3", 130);

    /* if we render a table head, we add a background to cells */
    if(r->tabl_head) {
      render_verbatim(r, "\\clcbpat5\\cellx", 15);
      ultostr(cw * (i + 1), str_num, 10, 0);
    } else {
      render_verbatim(r, "\\cellx", 6);
      ultostr(cw * (i + 1), str_num, 10, 0);
    }

    RENDER_VERBATIM(r, str_num);
  }
}

static inline void
render_leave_block_tr(MD_RTF* r)
{
  /* close the row */
  render_verbatim(r, "\\row\r\n", 6);
}

static void
render_enter_block_td(MD_RTF* r, const MD_BLOCK_TD_DETAIL* td)
{
  switch(td->align) {
    case MD_ALIGN_CENTER: render_verbatim(r, "\\qc ", 4); break;
    case MD_ALIGN_RIGHT: render_verbatim(r, "\\qr ", 4); break;
    default: render_verbatim(r, "\\ql ", 4); break;
  }
}

static inline void
render_leave_block_td(MD_RTF* r)
{
  render_verbatim(r, "\\intbl\\cell ", 12);
}

static void
render_enter_block_th(MD_RTF* r, const MD_BLOCK_TD_DETAIL* td)
{
  /* same as td but with bold text */
  switch(td->align) {
    case MD_ALIGN_CENTER: render_verbatim(r, "\\qc\\b ", 6); break;
    case MD_ALIGN_RIGHT: render_verbatim(r, "\\qr\\b ", 6); break;
    default: render_verbatim(r, "\\ql\\b ", 6); break;
  }
}

static inline void
render_leave_block_th(MD_RTF* r)
{
  render_verbatim(r, "\\b0\\intbl\\cell ", 15);
}

static inline void
render_enter_block_p(MD_RTF* r)
{
  #ifdef _DEBUG
  printf_tabs(2, r->list_dpth);
  printf("=> render_enter_block_p\n");
  #endif

  /* special case if we are inside a list item */
  if(r->list_dpth >= 0) {

    /* if we have more than one paragraph inside list item we add line feed
    at paragraph start */
    if(r->list_para) {
      render_verbatim(r, "\\line1\r\n", 8);
    } else {
      /* enabled line feed for the next paragraph */
      r->list_para = 1;
    }

    /* we do not need further parameters */
    return;
  }

  /* use normal font */
  render_font_norm(r);

  /* default space after and before */
  render_verbatim(r, "\\sb0\\sa0 ", 9);
}

static inline void
render_leave_block_p(MD_RTF* r)
{
  #ifdef _DEBUG
  printf_tabs(2, r->list_dpth);
  printf("<= render_leave_block_p\n");
  #endif

  /* special case if within block quote or item list we ignore paragraph end */
  if(r->quot_blck || (r->list_dpth >= 0))
    return;

  /* standard end of paragraph */
  render_verbatim(r, "\\line1\\par\r\n", 12);
}

static void
render_enter_span_url(MD_RTF* r, const MD_SPAN_A_DETAIL* a)
{
  render_verbatim(r, "\\cf4\\ul {\\field{\\*\\fldinst HYPERLINK \"", 38);
  #ifdef MD4C_USE_UTF16
  render_wchar(r, a->href.text, a->href.size, render_url_escaped);
  #else
  render_url_escaped(r, a->href.text, a->href.size);
  #endif
  render_verbatim(r, "\"}{\\fldrslt ", 12);
}

static inline void
render_leave_span_url(MD_RTF* r)
{
  render_verbatim(r, "}}\\ul0 \\cf0 ", 12);
}

static inline void
render_enter_span_code(MD_RTF* r)
{
  render_font_mono(r);
}

static inline void
render_leave_span_code(MD_RTF* r)
{
  render_font_norm(r);

  /* parser eat space after code span, we add it */
  render_verbatim(r, " ", 1);
}





/***************************************
 ***   RTF renderer implementation   ***
 ***************************************/

static int
enter_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata)
{
  MD_RTF* r = (MD_RTF*) userdata;

  switch(type) {
      case MD_BLOCK_DOC:      render_enter_block_doc(r); break;
      case MD_BLOCK_QUOTE:    render_enter_block_quote(r); break;
      case MD_BLOCK_UL:       render_enter_block_ul(r, (const MD_BLOCK_UL_DETAIL*)detail); break;
      case MD_BLOCK_OL:       render_enter_block_ol(r, (const MD_BLOCK_OL_DETAIL*)detail); break;
      case MD_BLOCK_LI:       render_enter_block_li(r, (const MD_BLOCK_LI_DETAIL*)detail); break;
      case MD_BLOCK_HR:       render_enter_block_hr(r); break;
      case MD_BLOCK_H:        render_enter_block_h(r, (const MD_BLOCK_H_DETAIL*)detail); break;
      case MD_BLOCK_CODE:     render_enter_block_code(r); break;
      case MD_BLOCK_P:        render_enter_block_p(r); break;
      case MD_BLOCK_HTML:     /* noop */ break;
      case MD_BLOCK_TABLE:    render_enter_block_table(r, (const MD_BLOCK_TABLE_DETAIL*)detail); break;
      case MD_BLOCK_THEAD:    render_enter_block_thead(r); break;
      case MD_BLOCK_TBODY:    /* noop */ break;
      case MD_BLOCK_TR:       render_enter_block_tr(r); break;
      case MD_BLOCK_TH:       render_enter_block_th(r,(const MD_BLOCK_TD_DETAIL*)detail); break;
      case MD_BLOCK_TD:       render_enter_block_td(r,(const MD_BLOCK_TD_DETAIL*)detail); break;
  }

  return 0;
}

static int
leave_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata)
{
  MD_RTF* r = (MD_RTF*) userdata;

  switch(type) {
      case MD_BLOCK_DOC:      render_leave_block_doc(r); break;
      case MD_BLOCK_QUOTE:    render_leave_block_quote(r); break;
      case MD_BLOCK_UL:       render_leave_block_ul(r); break;
      case MD_BLOCK_OL:       render_leave_block_ol(r); break;
      case MD_BLOCK_LI:       render_leave_block_li(r); break;
      case MD_BLOCK_HR:       /*noop*/ break;
      case MD_BLOCK_H:        render_leave_block_h(r, (const MD_BLOCK_H_DETAIL*)detail); break;
      case MD_BLOCK_CODE:     render_leave_block_code(r); break;
      case MD_BLOCK_P:        render_leave_block_p(r); break;
      case MD_BLOCK_HTML:     /* noop */ break;
      case MD_BLOCK_TABLE:    render_leave_block_table(r); break;
      case MD_BLOCK_THEAD:    render_leave_block_thead(r); break;
      case MD_BLOCK_TBODY:    /* noop */ break;
      case MD_BLOCK_TR:       render_leave_block_tr(r); break;
      case MD_BLOCK_TH:       render_leave_block_th(r); break;
      case MD_BLOCK_TD:       render_leave_block_td(r); break;
  }

  return 0;
}

static int
enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata)
{
  MD_RTF* r = (MD_RTF*) userdata;

  switch(type) {
      case MD_SPAN_EM:                render_verbatim(r, "\\i ", 3); break;
      case MD_SPAN_STRONG:            render_verbatim(r, "\\b ", 3); break;
      case MD_SPAN_U:                 render_verbatim(r, "\\ul ", 4); break;
      case MD_SPAN_DEL:               render_verbatim(r, "\\strike ", 8); break;
      case MD_SPAN_A:                 render_enter_span_url(r, (MD_SPAN_A_DETAIL*) detail); break;
      case MD_SPAN_CODE:              render_enter_span_code(r); break;
      //case MD_SPAN_IMG:               render_open_img_span(r, (MD_SPAN_IMG_DETAIL*) detail); break;
      //case MD_SPAN_LATEXMATH:         RENDER_VERBATIM(r, "<x-equation>"); break;
      //case MD_SPAN_LATEXMATH_DISPLAY: RENDER_VERBATIM(r, "<x-equation type=\"display\">"); break;
      //case MD_SPAN_WIKILINK:          render_open_wikilink_span(r, (MD_SPAN_WIKILINK_DETAIL*) detail); break;
  }

  return 0;
}

static int
leave_span_callback(MD_SPANTYPE type, void* detail, void* userdata)
{
  MD_RTF* r = (MD_RTF*) userdata;

  switch(type) {
      case MD_SPAN_EM:                render_verbatim(r, "\\i0 ", 4); break;
      case MD_SPAN_STRONG:            render_verbatim(r, "\\b0 ", 4); break;
      case MD_SPAN_U:                 render_verbatim(r, "\\ul0 ", 5); break;
      case MD_SPAN_DEL:               render_verbatim(r, "\\strike0 ", 9); break;
      case MD_SPAN_A:                 render_leave_span_url(r); break;
      case MD_SPAN_CODE:              render_leave_span_code(r); break;
      //case MD_SPAN_IMG:               /*noop, handled above*/ break;
      //case MD_SPAN_LATEXMATH:         /*fall through*/
      //case MD_SPAN_LATEXMATH_DISPLAY: RENDER_VERBATIM(r, "</x-equation>"); break;
      //case MD_SPAN_WIKILINK:          RENDER_VERBATIM(r, "</x-wikilink>"); break;
  }

  return 0;
}

#ifdef MD4C_USE_UTF16
static int
text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata)
{
  MD_RTF* r = (MD_RTF*) userdata;

  #ifdef _DEBUG
  printf_tabs(2, r->list_dpth);
  printf("++ text_callback (");
  switch(type) {
    case MD_TEXT_NULLCHAR: printf("MD_TEXT_NULLCHAR)\n"); break;
    case MD_TEXT_BR:  printf("MD_TEXT_BR)\n"); break;
    case MD_TEXT_SOFTBR:  printf("MD_TEXT_SOFTBR)\n"); break;
    case MD_TEXT_CODE: printf("MD_TEXT_CODE)\n"); break;
    case MD_TEXT_HTML:  printf("MD_TEXT_HTML)\n"); break;
    case MD_TEXT_ENTITY: printf("MD_TEXT_ENTITY)\n"); break;
    default: printf("NORMAL)\n"); break;
  }
  #endif

  switch(type) {
      case MD_TEXT_NULLCHAR:  render_verbatim(r, "\0", 1); break;
      case MD_TEXT_BR:        render_verbatim(r, "\\line1", 6); break;
      case MD_TEXT_SOFTBR:    render_verbatim(r, " ", 1); break;
      case MD_TEXT_CODE:      render_wchar(r, text, size, render_text_code); break;
      case MD_TEXT_HTML:      render_wchar(r, text, size, render_rtf_escaped); break;
      case MD_TEXT_ENTITY:    render_wchar(r, text, size, render_entity); break;
      default:                render_wchar(r, text, size, render_rtf_escaped); break;
  }
  return 0;
}
#else
static int
text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata)
{
  MD_RTF* r = (MD_RTF*) userdata;

  #ifdef _DEBUG
  printf_tabs(2, r->list_dpth);
  printf("++ text_callback (");
  switch(type) {
    case MD_TEXT_NULLCHAR: printf("MD_TEXT_NULLCHAR)\n"); break;
    case MD_TEXT_BR:  printf("MD_TEXT_BR)\n"); break;
    case MD_TEXT_SOFTBR:  printf("MD_TEXT_SOFTBR)\n"); break;
    case MD_TEXT_CODE: printf("MD_TEXT_CODE)\n"); break;
    case MD_TEXT_HTML:  printf("MD_TEXT_HTML)\n"); break;
    case MD_TEXT_ENTITY: printf("MD_TEXT_ENTITY)\n"); break;
    default: printf("NORMAL)\n"); break;
  }
  #endif

  switch(type) {
      case MD_TEXT_NULLCHAR:  render_verbatim(r, "\0", 1); break;
      case MD_TEXT_BR:        render_verbatim(r, "\\line1", 6); break;
      case MD_TEXT_SOFTBR:    render_verbatim(r, " ", 1); break;
      case MD_TEXT_CODE:      render_text_code(r, text, size); break;
      case MD_TEXT_HTML:      render_rtf_escaped(r, text, size); break;
      case MD_TEXT_ENTITY:    render_entity(r, text, size); break;
      default:                render_rtf_escaped(r, text, size); break;
  }
  return 0;
}
#endif

static void
debug_log_callback(const char* msg, void* userdata)
{
  MD_RTF* r = (MD_RTF*) userdata;
  if(r->flags & MD_RTF_FLAG_DEBUG)
      fprintf(stderr, "MD4C: %s\n", msg);
}


int md_rtf(const MD_CHAR* input, MD_SIZE input_size,
            void (*process_output)(const MD_RTF_DATA*, MD_SIZE, void*),
            void* userdata, unsigned parser_flags, unsigned renderer_flags,
            unsigned font_size, unsigned doc_width)
{
  MD_RTF render;
  render.process_output = process_output;
  render.userdata = userdata;
  render.flags = renderer_flags;
  render.font_base = 2 * font_size; /* point to half-point */
  render.page_width = 56.689f * doc_width; /* pixels to twips */
  render.page_height = 1.41428f * render.page_width; /* ISO 216 ratio */
  render.page_margin = 400; /* left and right margin */
  render.list_dpth = -1;
 // render.list_stop = 0;
  render.list_para = 0;
  render.list_rset = 0;
  render.code_lf = 0;
  render.quot_blck = 0;

  MD_PARSER parser = {
      0,
      parser_flags,
      enter_block_callback,
      leave_block_callback,
      enter_span_callback,
      leave_span_callback,
      text_callback,
      debug_log_callback,
      NULL
  };

  /* Build map of characters which need escaping. */
  for(unsigned i = 0; i < 256; i++) {

      unsigned char ch = (unsigned char)i;

      render.escape_map[i] = 0;

      if(strchr("\\{}\n", ch) != NULL || ch > 0x7F)
          render.escape_map[i] |= NEED_RTF_ESC_FLAG|NEED_PRE_ESC_FLAG;

      if(!ISALNUM(ch)  &&  strchr("~-_.+!*(),%#@?=;:/,+$", ch) == NULL)
          render.escape_map[i] |= NEED_URL_ESC_FLAG;
  }

  /* Consider skipping UTF-8 byte order mark (BOM). */
  if(renderer_flags & MD_RTF_FLAG_SKIP_UTF8_BOM && sizeof(MD_CHAR) == 1) {

    static const MD_CHAR bom[3] = { 0xef, 0xbb, 0xbf };

    if(input_size >= sizeof(bom)  &&  memcmp(input, bom, sizeof(bom)) == 0) {
      input += sizeof(bom);
      input_size -= sizeof(bom);
    }
  }

  /* build preformated strings of control words with space and size
  values according given font size and document width */

  /* we clamp size in order to prevent buffer overflow due to large numbers
  printed in template strings */
  if(render.font_base > 98)
    render.font_base = 98;

  /* general font sizes */
  sprintf(render.cw_fs[0], "\\fs%u ", render.font_base );
  sprintf(render.cw_fs[1], "\\fs%u ", (unsigned)(0.9f*render.font_base) );

  /* titles styles per level with font size and space-after values */
  sprintf(render.cw_hf[0], "\\fs%u\\sa%u\\b ", (unsigned)(2.2f*render.font_base), 8*render.font_base);
  sprintf(render.cw_hf[1], "\\fs%u\\sa%u\\b ", (unsigned)(1.7f*render.font_base), 8*render.font_base);
  sprintf(render.cw_hf[2], "\\fs%u\\sa%u\\b ", (unsigned)(1.4f*render.font_base), 8*render.font_base);
  sprintf(render.cw_hf[3], "\\fs%u\\sa%u\\b\\i ", (unsigned)(1.2f*render.font_base), 6*render.font_base);
  sprintf(render.cw_hf[4], "\\fs%u\\sa%u\\b\\i ", (unsigned)(1.1f*render.font_base), 6*render.font_base);
  sprintf(render.cw_hf[5], "\\fs%u\\sa%u\\b\\i ", (unsigned)(render.font_base), 6*render.font_base);

  /* space-before values */
  sprintf(render.cw_sb[0], "\\sb%u ", 0*render.font_base);
  sprintf(render.cw_sb[1], "\\sb%u ", 2*render.font_base);

  /* space-after values */
  sprintf(render.cw_sa[0], "\\sa%u ", 2*render.font_base);
  sprintf(render.cw_sa[1], "\\sa%u ", 2*render.font_base);

  /* left-ident values , up to 8 level */
  sprintf(render.cw_li[0], "\\li%u ",  20*render.font_base);
  sprintf(render.cw_li[1], "\\li%u ",  40*render.font_base);
  sprintf(render.cw_li[2], "\\li%u ",  60*render.font_base);
  sprintf(render.cw_li[3], "\\li%u ",  80*render.font_base);
  sprintf(render.cw_li[4], "\\li%u ", 100*render.font_base);
  sprintf(render.cw_li[5], "\\li%u ", 120*render.font_base);
  sprintf(render.cw_li[6], "\\li%u ", 140*render.font_base);
  sprintf(render.cw_li[7], "\\li%u ", 160*render.font_base);

  /* tables basic parameter and left margin */
  unsigned g = 8*render.font_base;
  if(g > 255) g = 255; //< \\tgrah value must be 0 to 255
  sprintf(render.cw_tr[0], "\\trgaph%u\\trftsWidth2\\trwWidth4500\\trautofit1 ", g);
  sprintf(render.cw_tr[1], "\\trgaph%u\\trrh%u\\trftsWidth2\\trwWidth4500\\trautofit1 ", 3*render.font_base, 16*render.font_base);

  /* frist-line indent values, used for bulleted and numbered lists */
  sprintf(render.cw_fi[0], "\\fi%i ", -10*render.font_base);
  sprintf(render.cw_fi[1], "\\fi%i ", -12*render.font_base);

  /* table cell width adjusted to given page width */
  sprintf(render.cw_cx[0], "\\cellx%u ", (unsigned)(0.9f * render.page_width));
  sprintf(render.cw_cx[1], "\\cellx%u ", render.page_width);

  int result = md_parse(input, input_size, &parser, (void*)&render);

  return result;
}
