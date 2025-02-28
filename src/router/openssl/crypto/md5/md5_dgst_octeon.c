/* ====================================================================
 * Copyright (c) 1998-2008 The OpenSSL Project.  All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */


/**
 * @file md5_dgst.c
 * MD5 Hash API
*/
#define OCTEON_HASH
#include <stdio.h>
#include <stdio.h>
#include "md5_local.h"
#include <openssl/opensslv.h>

#ifdef OCTEON_OPENSSL
#include "cvmx-address.h"
#include "cvmx-asm.h"
#include "cvmx-key.h"
#include "cvmx-swap.h"
#endif


#ifdef OCTEON_OPENSSL
static inline uint64_t
swap64 (uint64_t v)
{
  return ((v >> 56) |
    (((v >> 48) & 0xfful) << 8) |
    (((v >> 40) & 0xfful) << 16) |
    (((v >> 32) & 0xfful) << 24) |
    (((v >> 24) & 0xfful) << 32) |
    (((v >> 16) & 0xfful) << 40) |
    (((v >> 8) & 0xfful) << 48) | (((v >> 0) & 0xfful) << 56));
}
#endif
/* Implemented from RFC1321 The MD5 Message-Digest Algorithm
 */

#define INIT_DATA_A (unsigned long)0x67452301L
#define INIT_DATA_B (unsigned long)0xefcdab89L
#define INIT_DATA_C (unsigned long)0x98badcfeL
#define INIT_DATA_D (unsigned long)0x10325476L

#ifdef OCTEON_OPENSSL
int
MD5_Update (MD5_CTX * c, const void *data_, size_t len)
{
  unsigned long remaining = 0, totlen = 0, copied = 0;
  const uint64_t *ptr = (const uint64_t *) data_;

  if (!len)
    return -1;

  totlen = len;
  if (c->num) {
    /* Adjust the previous data */
    /* This is not the first update call */
    totlen += remaining = c->num % 64;
    if (remaining) {
      memcpy ((void *) &(c->data) + remaining, data_, 64 - remaining);
      copied = 1;
    }
  }
  if (totlen >= 64) {
    /* Initialise the running IV */
    CVMX_MT_HSH_IV (c->E, 0);
    CVMX_MT_HSH_IV (c->F, 1);

    while (totlen >= 64) {
      if (copied) {
        copied = 0;
      }
      if (remaining) {
        ptr = (uint64_t *) & c->data;
      }
      CVMX_MT_HSH_DAT (*ptr++, 0);
      CVMX_MT_HSH_DAT (*ptr++, 1);
      CVMX_MT_HSH_DAT (*ptr++, 2);
      CVMX_MT_HSH_DAT (*ptr++, 3);
      CVMX_MT_HSH_DAT (*ptr++, 4);
      CVMX_MT_HSH_DAT (*ptr++, 5);
      CVMX_MT_HSH_DAT (*ptr++, 6);
      CVMX_MT_HSH_STARTMD5 (*ptr++);
      totlen -= 64;
      if (remaining) {
        ptr = data_ + (64 - remaining);
        remaining = 0;
      }
    }
    CVMX_MF_HSH_IV (c->E, 0);
    CVMX_MF_HSH_IV (c->F, 1);
  }                             /* if (len > 64) */
  c->num += (len);
  /* Copy the remaining stuffs in to a buffer and do the hash in the next 
   * update or final.
   */
  if (!copied)
    memcpy (&(c->data), ptr, totlen);
  return 1;
}

int
MD5_Final (unsigned char *md, MD5_CTX * c)
{
  unsigned long len;
  uint8_t chunk[64];
  uint64_t bits;
  const uint64_t *ptr;

  len = c->num % 64;
  bits = cvmx_cpu_to_le64(c->num * 8);
  /* The rest of the data will need to be copied into a chunk */
  if (len > 0)
    memcpy (chunk, c->data, len);
  chunk[len] = 0x80;
  memset (chunk + len + 1, 0, 64 - len - 1);
  /* Initialise the running IV */
  CVMX_MT_HSH_IV (c->E, 0);
  CVMX_MT_HSH_IV (c->F, 1);

  ptr = (const uint64_t *) chunk;
  CVMX_MT_HSH_DAT (*ptr++, 0);
  CVMX_MT_HSH_DAT (*ptr++, 1);
  CVMX_MT_HSH_DAT (*ptr++, 2);
  CVMX_MT_HSH_DAT (*ptr++, 3);
  CVMX_MT_HSH_DAT (*ptr++, 4);
  CVMX_MT_HSH_DAT (*ptr++, 5);
  CVMX_MT_HSH_DAT (*ptr++, 6);

  /* Check to see if there is room for the bit count */
  if (len < 56)
    CVMX_MT_HSH_STARTMD5 (bits);
  else {
    CVMX_MT_HSH_STARTMD5 (*ptr);
    /* Another block was needed */
    CVMX_MT_HSH_DATZ (0);
    CVMX_MT_HSH_DATZ (1);
    CVMX_MT_HSH_DATZ (2);
    CVMX_MT_HSH_DATZ (3);
    CVMX_MT_HSH_DATZ (4);
    CVMX_MT_HSH_DATZ (5);
    CVMX_MT_HSH_DATZ (6);

    /* 
     * Workaround for issue described in section "2.2.3.2 MD5 Hash Calculation
     * Restart" of the CN63XX Pass 1 Known Issues document.
     */
    CVMX_MF_HSH_IV (c->E, 0);
    CVMX_MF_HSH_IV (c->F, 1);

    CVMX_MT_HSH_STARTMD5 (bits);
  }
  CVMX_MF_HSH_IV (c->E, 0);
  CVMX_MF_HSH_IV (c->F, 1);
  c->E = cvmx_be64_to_cpu(c->E);
  c->F = cvmx_be64_to_cpu(c->F);
  memcpy (md, (void *) &c->E, 8);
  memcpy (md + 8, (void *) &c->F, 8);
  c->num = 0;
  return 1;
}
#endif

int
MD5_Init (MD5_CTX * c)
{
#ifndef OCTEON_OPENSSL
  c->A = INIT_DATA_A;
  c->B = INIT_DATA_B;
  c->C = INIT_DATA_C;
  c->D = INIT_DATA_D;
  c->Nl = 0;
  c->Nh = 0;
#else
  /* Set the IV to the MD5 magic start value */
  c->E = cvmx_cpu_to_be64(0x0123456789abcdefull);
  c->F = cvmx_cpu_to_be64(0xfedcba9876543210ull);
#endif
  c->num = 0;
  return 1;
}

#ifndef md5_block_host_order
void
md5_block_host_order (MD5_CTX * c, const void *data, size_t num)
{
  const MD5_LONG *X = data;
  register unsigned MD32_REG_T A, B, C, D;

  A = c->A;
  B = c->B;
  C = c->C;
  D = c->D;

  for (; num--; X += HASH_LBLOCK) {
    /* Round 0 */
    R0 (A, B, C, D, X[0], 7, 0xd76aa478L);
    R0 (D, A, B, C, X[1], 12, 0xe8c7b756L);
    R0 (C, D, A, B, X[2], 17, 0x242070dbL);
    R0 (B, C, D, A, X[3], 22, 0xc1bdceeeL);
    R0 (A, B, C, D, X[4], 7, 0xf57c0fafL);
    R0 (D, A, B, C, X[5], 12, 0x4787c62aL);
    R0 (C, D, A, B, X[6], 17, 0xa8304613L);
    R0 (B, C, D, A, X[7], 22, 0xfd469501L);
    R0 (A, B, C, D, X[8], 7, 0x698098d8L);
    R0 (D, A, B, C, X[9], 12, 0x8b44f7afL);
    R0 (C, D, A, B, X[10], 17, 0xffff5bb1L);
    R0 (B, C, D, A, X[11], 22, 0x895cd7beL);
    R0 (A, B, C, D, X[12], 7, 0x6b901122L);
    R0 (D, A, B, C, X[13], 12, 0xfd987193L);
    R0 (C, D, A, B, X[14], 17, 0xa679438eL);
    R0 (B, C, D, A, X[15], 22, 0x49b40821L);
    /* Round 1 */
    R1 (A, B, C, D, X[1], 5, 0xf61e2562L);
    R1 (D, A, B, C, X[6], 9, 0xc040b340L);
    R1 (C, D, A, B, X[11], 14, 0x265e5a51L);
    R1 (B, C, D, A, X[0], 20, 0xe9b6c7aaL);
    R1 (A, B, C, D, X[5], 5, 0xd62f105dL);
    R1 (D, A, B, C, X[10], 9, 0x02441453L);
    R1 (C, D, A, B, X[15], 14, 0xd8a1e681L);
    R1 (B, C, D, A, X[4], 20, 0xe7d3fbc8L);
    R1 (A, B, C, D, X[9], 5, 0x21e1cde6L);
    R1 (D, A, B, C, X[14], 9, 0xc33707d6L);
    R1 (C, D, A, B, X[3], 14, 0xf4d50d87L);
    R1 (B, C, D, A, X[8], 20, 0x455a14edL);
    R1 (A, B, C, D, X[13], 5, 0xa9e3e905L);
    R1 (D, A, B, C, X[2], 9, 0xfcefa3f8L);
    R1 (C, D, A, B, X[7], 14, 0x676f02d9L);
    R1 (B, C, D, A, X[12], 20, 0x8d2a4c8aL);
    /* Round 2 */
    R2 (A, B, C, D, X[5], 4, 0xfffa3942L);
    R2 (D, A, B, C, X[8], 11, 0x8771f681L);
    R2 (C, D, A, B, X[11], 16, 0x6d9d6122L);
    R2 (B, C, D, A, X[14], 23, 0xfde5380cL);
    R2 (A, B, C, D, X[1], 4, 0xa4beea44L);
    R2 (D, A, B, C, X[4], 11, 0x4bdecfa9L);
    R2 (C, D, A, B, X[7], 16, 0xf6bb4b60L);
    R2 (B, C, D, A, X[10], 23, 0xbebfbc70L);
    R2 (A, B, C, D, X[13], 4, 0x289b7ec6L);
    R2 (D, A, B, C, X[0], 11, 0xeaa127faL);
    R2 (C, D, A, B, X[3], 16, 0xd4ef3085L);
    R2 (B, C, D, A, X[6], 23, 0x04881d05L);
    R2 (A, B, C, D, X[9], 4, 0xd9d4d039L);
    R2 (D, A, B, C, X[12], 11, 0xe6db99e5L);
    R2 (C, D, A, B, X[15], 16, 0x1fa27cf8L);
    R2 (B, C, D, A, X[2], 23, 0xc4ac5665L);
    /* Round 3 */
    R3 (A, B, C, D, X[0], 6, 0xf4292244L);
    R3 (D, A, B, C, X[7], 10, 0x432aff97L);
    R3 (C, D, A, B, X[14], 15, 0xab9423a7L);
    R3 (B, C, D, A, X[5], 21, 0xfc93a039L);
    R3 (A, B, C, D, X[12], 6, 0x655b59c3L);
    R3 (D, A, B, C, X[3], 10, 0x8f0ccc92L);
    R3 (C, D, A, B, X[10], 15, 0xffeff47dL);
    R3 (B, C, D, A, X[1], 21, 0x85845dd1L);
    R3 (A, B, C, D, X[8], 6, 0x6fa87e4fL);
    R3 (D, A, B, C, X[15], 10, 0xfe2ce6e0L);
    R3 (C, D, A, B, X[6], 15, 0xa3014314L);
    R3 (B, C, D, A, X[13], 21, 0x4e0811a1L);
    R3 (A, B, C, D, X[4], 6, 0xf7537e82L);
    R3 (D, A, B, C, X[11], 10, 0xbd3af235L);
    R3 (C, D, A, B, X[2], 15, 0x2ad7d2bbL);
    R3 (B, C, D, A, X[9], 21, 0xeb86d391L);

    A = c->A += A;
    B = c->B += B;
    C = c->C += C;
    D = c->D += D;
  }
}
#endif

#ifndef md5_block_data_order
#ifdef X
#undef X
#endif
void
md5_block_data_order (MD5_CTX * c, const void *data_, size_t num)
{
  const unsigned char *data = data_;
  register unsigned MD32_REG_T A, B, C, D, l;
#ifndef MD32_XARRAY
  /* See comment in crypto/sha/sha_locl.h for details. */
  unsigned MD32_REG_T XX0, XX1, XX2, XX3, XX4, XX5, XX6, XX7,
    XX8, XX9, XX10, XX11, XX12, XX13, XX14, XX15;
# define X(i)    XX##i
#else
  MD5_LONG XX[MD5_LBLOCK];
# define X(i)    XX[i]
#endif

  A = c->A;
  B = c->B;
  C = c->C;
  D = c->D;

  for (; num--;) {
    HOST_c2l (data, l);
    X (0) = l;
    HOST_c2l (data, l);
    X (1) = l;
    /* Round 0 */
    R0 (A, B, C, D, X (0), 7, 0xd76aa478L);
    HOST_c2l (data, l);
    X (2) = l;
    R0 (D, A, B, C, X (1), 12, 0xe8c7b756L);
    HOST_c2l (data, l);
    X (3) = l;
    R0 (C, D, A, B, X (2), 17, 0x242070dbL);
    HOST_c2l (data, l);
    X (4) = l;
    R0 (B, C, D, A, X (3), 22, 0xc1bdceeeL);
    HOST_c2l (data, l);
    X (5) = l;
    R0 (A, B, C, D, X (4), 7, 0xf57c0fafL);
    HOST_c2l (data, l);
    X (6) = l;
    R0 (D, A, B, C, X (5), 12, 0x4787c62aL);
    HOST_c2l (data, l);
    X (7) = l;
    R0 (C, D, A, B, X (6), 17, 0xa8304613L);
    HOST_c2l (data, l);
    X (8) = l;
    R0 (B, C, D, A, X (7), 22, 0xfd469501L);
    HOST_c2l (data, l);
    X (9) = l;
    R0 (A, B, C, D, X (8), 7, 0x698098d8L);
    HOST_c2l (data, l);
    X (10) = l;
    R0 (D, A, B, C, X (9), 12, 0x8b44f7afL);
    HOST_c2l (data, l);
    X (11) = l;
    R0 (C, D, A, B, X (10), 17, 0xffff5bb1L);
    HOST_c2l (data, l);
    X (12) = l;
    R0 (B, C, D, A, X (11), 22, 0x895cd7beL);
    HOST_c2l (data, l);
    X (13) = l;
    R0 (A, B, C, D, X (12), 7, 0x6b901122L);
    HOST_c2l (data, l);
    X (14) = l;
    R0 (D, A, B, C, X (13), 12, 0xfd987193L);
    HOST_c2l (data, l);
    X (15) = l;
    R0 (C, D, A, B, X (14), 17, 0xa679438eL);
    R0 (B, C, D, A, X (15), 22, 0x49b40821L);
    /* Round 1 */
    R1 (A, B, C, D, X (1), 5, 0xf61e2562L);
    R1 (D, A, B, C, X (6), 9, 0xc040b340L);
    R1 (C, D, A, B, X (11), 14, 0x265e5a51L);
    R1 (B, C, D, A, X (0), 20, 0xe9b6c7aaL);
    R1 (A, B, C, D, X (5), 5, 0xd62f105dL);
    R1 (D, A, B, C, X (10), 9, 0x02441453L);
    R1 (C, D, A, B, X (15), 14, 0xd8a1e681L);
    R1 (B, C, D, A, X (4), 20, 0xe7d3fbc8L);
    R1 (A, B, C, D, X (9), 5, 0x21e1cde6L);
    R1 (D, A, B, C, X (14), 9, 0xc33707d6L);
    R1 (C, D, A, B, X (3), 14, 0xf4d50d87L);
    R1 (B, C, D, A, X (8), 20, 0x455a14edL);
    R1 (A, B, C, D, X (13), 5, 0xa9e3e905L);
    R1 (D, A, B, C, X (2), 9, 0xfcefa3f8L);
    R1 (C, D, A, B, X (7), 14, 0x676f02d9L);
    R1 (B, C, D, A, X (12), 20, 0x8d2a4c8aL);
    /* Round 2 */
    R2 (A, B, C, D, X (5), 4, 0xfffa3942L);
    R2 (D, A, B, C, X (8), 11, 0x8771f681L);
    R2 (C, D, A, B, X (11), 16, 0x6d9d6122L);
    R2 (B, C, D, A, X (14), 23, 0xfde5380cL);
    R2 (A, B, C, D, X (1), 4, 0xa4beea44L);
    R2 (D, A, B, C, X (4), 11, 0x4bdecfa9L);
    R2 (C, D, A, B, X (7), 16, 0xf6bb4b60L);
    R2 (B, C, D, A, X (10), 23, 0xbebfbc70L);
    R2 (A, B, C, D, X (13), 4, 0x289b7ec6L);
    R2 (D, A, B, C, X (0), 11, 0xeaa127faL);
    R2 (C, D, A, B, X (3), 16, 0xd4ef3085L);
    R2 (B, C, D, A, X (6), 23, 0x04881d05L);
    R2 (A, B, C, D, X (9), 4, 0xd9d4d039L);
    R2 (D, A, B, C, X (12), 11, 0xe6db99e5L);
    R2 (C, D, A, B, X (15), 16, 0x1fa27cf8L);
    R2 (B, C, D, A, X (2), 23, 0xc4ac5665L);
    /* Round 3 */
    R3 (A, B, C, D, X (0), 6, 0xf4292244L);
    R3 (D, A, B, C, X (7), 10, 0x432aff97L);
    R3 (C, D, A, B, X (14), 15, 0xab9423a7L);
    R3 (B, C, D, A, X (5), 21, 0xfc93a039L);
    R3 (A, B, C, D, X (12), 6, 0x655b59c3L);
    R3 (D, A, B, C, X (3), 10, 0x8f0ccc92L);
    R3 (C, D, A, B, X (10), 15, 0xffeff47dL);
    R3 (B, C, D, A, X (1), 21, 0x85845dd1L);
    R3 (A, B, C, D, X (8), 6, 0x6fa87e4fL);
    R3 (D, A, B, C, X (15), 10, 0xfe2ce6e0L);
    R3 (C, D, A, B, X (6), 15, 0xa3014314L);
    R3 (B, C, D, A, X (13), 21, 0x4e0811a1L);
    R3 (A, B, C, D, X (4), 6, 0xf7537e82L);
    R3 (D, A, B, C, X (11), 10, 0xbd3af235L);
    R3 (C, D, A, B, X (2), 15, 0x2ad7d2bbL);
    R3 (B, C, D, A, X (9), 21, 0xeb86d391L);

    A = c->A += A;
    B = c->B += B;
    C = c->C += C;
    D = c->D += D;
  }
}
#endif

#ifdef undef
int
printit (unsigned long *l)
{
  int i, ii;

  for (i = 0; i < 2; i++) {
    for (ii = 0; ii < 8; ii++) {
      fprintf (stderr, "%08lx ", l[i * 8 + ii]);
    }
    fprintf (stderr, "\n");
  }
}
#endif
