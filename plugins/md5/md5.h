/*
 * MD5 Message-Digest Algorithm implementation.
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
 */

/*
 * -----------------------------------------------------------------------------
 *
 * This MD5 implementation is an adapted and slightly modernized version of
 * the 1991 implementation by RSA Data Security as provided in the RFC 1321
 * https://www.rfc-editor.org/rfc/rfc1321
 *
 * -----------------------------------------------------------------------------
 */

#ifndef MD5_H
#define MD5_H

#ifdef __cplusplus
    extern "C" {
#endif

/* MD5 context. */
typedef struct {
  unsigned state[4];          /* state (ABCD) */
  unsigned count[2];          /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];   /* input buffer */
} MD5_CTX;

void MD5_Init(MD5_CTX*);
void MD5_Update(MD5_CTX*, unsigned char*, unsigned);
void MD5_Final(unsigned char[16], MD5_CTX*);

#ifdef __cplusplus
    }  /* extern "C" { */
#endif

#endif /* MD5_H */
