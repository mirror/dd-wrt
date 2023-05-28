/*
     This file is part of libmicrohttpd
     Copyright (C) 2019-2021 Karlson2k (Evgeny Grin)

     libmicrohttpd is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library.
     If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file microhttpd/sha1.c
 * @brief  Calculation of SHA-1 digest as defined in FIPS PUB 180-4 (2015)
 * @author Karlson2k (Evgeny Grin)
 */

#include "sha1.h"

#include <string.h>
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif /* HAVE_MEMORY_H */
#include "mhd_bithelpers.h"
#include "mhd_assert.h"

/**
 * Initialise structure for SHA-1 calculation.
 *
 * @param ctx_ must be a `struct sha1_ctx *`
 */
void
MHD_SHA1_init (void *ctx_)
{
  struct sha1_ctx *const ctx = ctx_;
  /* Initial hash values, see FIPS PUB 180-4 paragraph 5.3.1 */
  /* Just some "magic" numbers defined by standard */
  ctx->H[0] = UINT32_C (0x67452301);
  ctx->H[1] = UINT32_C (0xefcdab89);
  ctx->H[2] = UINT32_C (0x98badcfe);
  ctx->H[3] = UINT32_C (0x10325476);
  ctx->H[4] = UINT32_C (0xc3d2e1f0);

  /* Initialise number of bytes. */
  ctx->count = 0;
}


/**
 * Base of SHA-1 transformation.
 * Gets full 512 bits / 64 bytes block of data and updates hash values;
 * @param H     hash values
 * @param data  data, must be exactly 64 bytes long
 */
static void
sha1_transform (uint32_t H[_SHA1_DIGEST_LENGTH],
                const uint8_t data[SHA1_BLOCK_SIZE])
{
  /* Working variables,
     see FIPS PUB 180-4 paragraph 6.1.3 */
  uint32_t a = H[0];
  uint32_t b = H[1];
  uint32_t c = H[2];
  uint32_t d = H[3];
  uint32_t e = H[4];

  /* Data buffer, used as cyclic buffer.
     See FIPS PUB 180-4 paragraphs 5.2.1, 6.1.3 */
  uint32_t W[16];

  /* 'Ch' and 'Maj' macro functions are defined with
     widely-used optimization.
     See FIPS PUB 180-4 formulae 4.1. */
#define Ch(x,y,z)     ( (z) ^ ((x) & ((y) ^ (z))) )
#define Maj(x,y,z)    ( ((x) & (y)) ^ ((z) & ((x) ^ (y))) )
  /* Unoptimized (original) versions: */
/* #define Ch(x,y,z)  ( ( (x) & (y) ) ^ ( ~(x) & (z) ) )          */
/* #define Maj(x,y,z) ( ((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)) ) */
#define Par(x,y,z)    ( (x) ^ (y) ^ (z) )

  /* Single step of SHA-1 computation,
     see FIPS PUB 180-4 paragraph 6.1.3 step 3.
   * Note: instead of reassigning all working variables on each step,
           variables are rotated for each step:
             SHA1STEP32 (a, b, c, d, e, func, K00, W[0]);
             SHA1STEP32 (e, a, b, c, d, func, K00, W[1]);
           so current 'vC' will be used as 'vD' on the next step,
           current 'vE' will be used as 'vA' on the next step.
   * Note: 'wt' must be used exactly one time in this macro as it change other data as well
           every time when used. */

#define SHA1STEP32(vA,vB,vC,vD,vE,ft,kt,wt) do {                         \
    (vE) += _MHD_ROTL32 ((vA), 5) + ft ((vB), (vC), (vD)) + (kt) + (wt); \
    (vB) = _MHD_ROTL32 ((vB), 30); } while (0)

  /* Get value of W(t) from input data buffer,
     See FIPS PUB 180-4 paragraph 6.1.3.
     Input data must be read in big-endian bytes order,
     see FIPS PUB 180-4 paragraph 3.1.2. */
#define GET_W_FROM_DATA(buf,t) \
  _MHD_GET_32BIT_BE (((const uint8_t*) (buf)) + (t) * SHA1_BYTES_IN_WORD)

#ifndef _MHD_GET_32BIT_BE_UNALIGNED
  if (0 != (((uintptr_t) data) % _MHD_UINT32_ALIGN))
  {
    /* Copy the unaligned input data to the aligned buffer */
    memcpy (W, data, SHA1_BLOCK_SIZE);
    /* The W[] buffer itself will be used as the source of the data,
     * but data will be reloaded in correct bytes order during
     * the next steps */
    data = (uint8_t*) W;
  }
#endif /* _MHD_GET_32BIT_BE_UNALIGNED */

/* SHA-1 values of Kt for t=0..19, see FIPS PUB 180-4 paragraph 4.2.1. */
#define K00      UINT32_C(0x5a827999)
/* SHA-1 values of Kt for t=20..39, see FIPS PUB 180-4 paragraph 4.2.1.*/
#define K20      UINT32_C(0x6ed9eba1)
/* SHA-1 values of Kt for t=40..59, see FIPS PUB 180-4 paragraph 4.2.1.*/
#define K40      UINT32_C(0x8f1bbcdc)
/* SHA-1 values of Kt for t=60..79, see FIPS PUB 180-4 paragraph 4.2.1.*/
#define K60      UINT32_C(0xca62c1d6)

  /* During first 16 steps, before making any calculations on each step,
     the W element is read from input data buffer as big-endian value and
     stored in array of W elements. */
  /* Note: instead of using K constants as array, all K values are specified
     individually for each step. */
  SHA1STEP32 (a, b, c, d, e, Ch, K00, W[0] = GET_W_FROM_DATA (data, 0));
  SHA1STEP32 (e, a, b, c, d, Ch, K00, W[1] = GET_W_FROM_DATA (data, 1));
  SHA1STEP32 (d, e, a, b, c, Ch, K00, W[2] = GET_W_FROM_DATA (data, 2));
  SHA1STEP32 (c, d, e, a, b, Ch, K00, W[3] = GET_W_FROM_DATA (data, 3));
  SHA1STEP32 (b, c, d, e, a, Ch, K00, W[4] = GET_W_FROM_DATA (data, 4));
  SHA1STEP32 (a, b, c, d, e, Ch, K00, W[5] = GET_W_FROM_DATA (data, 5));
  SHA1STEP32 (e, a, b, c, d, Ch, K00, W[6] = GET_W_FROM_DATA (data, 6));
  SHA1STEP32 (d, e, a, b, c, Ch, K00, W[7] = GET_W_FROM_DATA (data, 7));
  SHA1STEP32 (c, d, e, a, b, Ch, K00, W[8] = GET_W_FROM_DATA (data, 8));
  SHA1STEP32 (b, c, d, e, a, Ch, K00, W[9] = GET_W_FROM_DATA (data, 9));
  SHA1STEP32 (a, b, c, d, e, Ch, K00, W[10] = GET_W_FROM_DATA (data, 10));
  SHA1STEP32 (e, a, b, c, d, Ch, K00, W[11] = GET_W_FROM_DATA (data, 11));
  SHA1STEP32 (d, e, a, b, c, Ch, K00, W[12] = GET_W_FROM_DATA (data, 12));
  SHA1STEP32 (c, d, e, a, b, Ch, K00, W[13] = GET_W_FROM_DATA (data, 13));
  SHA1STEP32 (b, c, d, e, a, Ch, K00, W[14] = GET_W_FROM_DATA (data, 14));
  SHA1STEP32 (a, b, c, d, e, Ch, K00, W[15] = GET_W_FROM_DATA (data, 15));

  /* 'W' generation and assignment for 16 <= t <= 79.
     See FIPS PUB 180-4 paragraph 6.1.3.
     As only last 16 'W' are used in calculations, it is possible to
     use 16 elements array of W as cyclic buffer. */
#define Wgen(w,t) _MHD_ROTL32((w)[(t + 13) & 0xf] ^ (w)[(t + 8) & 0xf] \
                              ^ (w)[(t + 2) & 0xf] ^ (w)[t & 0xf], 1)

  /* During last 60 steps, before making any calculations on each step,
     W element is generated from W elements of cyclic buffer and generated value
     stored back in cyclic buffer. */
  /* Note: instead of using K constants as array, all K values are specified
     individually for each step, see FIPS PUB 180-4 paragraph 4.2.1. */
  SHA1STEP32 (e, a, b, c, d, Ch, K00, W[16 & 0xf] = Wgen (W, 16));
  SHA1STEP32 (d, e, a, b, c, Ch, K00, W[17 & 0xf] = Wgen (W, 17));
  SHA1STEP32 (c, d, e, a, b, Ch, K00, W[18 & 0xf] = Wgen (W, 18));
  SHA1STEP32 (b, c, d, e, a, Ch, K00, W[19 & 0xf] = Wgen (W, 19));
  SHA1STEP32 (a, b, c, d, e, Par, K20, W[20 & 0xf] = Wgen (W, 20));
  SHA1STEP32 (e, a, b, c, d, Par, K20, W[21 & 0xf] = Wgen (W, 21));
  SHA1STEP32 (d, e, a, b, c, Par, K20, W[22 & 0xf] = Wgen (W, 22));
  SHA1STEP32 (c, d, e, a, b, Par, K20, W[23 & 0xf] = Wgen (W, 23));
  SHA1STEP32 (b, c, d, e, a, Par, K20, W[24 & 0xf] = Wgen (W, 24));
  SHA1STEP32 (a, b, c, d, e, Par, K20, W[25 & 0xf] = Wgen (W, 25));
  SHA1STEP32 (e, a, b, c, d, Par, K20, W[26 & 0xf] = Wgen (W, 26));
  SHA1STEP32 (d, e, a, b, c, Par, K20, W[27 & 0xf] = Wgen (W, 27));
  SHA1STEP32 (c, d, e, a, b, Par, K20, W[28 & 0xf] = Wgen (W, 28));
  SHA1STEP32 (b, c, d, e, a, Par, K20, W[29 & 0xf] = Wgen (W, 29));
  SHA1STEP32 (a, b, c, d, e, Par, K20, W[30 & 0xf] = Wgen (W, 30));
  SHA1STEP32 (e, a, b, c, d, Par, K20, W[31 & 0xf] = Wgen (W, 31));
  SHA1STEP32 (d, e, a, b, c, Par, K20, W[32 & 0xf] = Wgen (W, 32));
  SHA1STEP32 (c, d, e, a, b, Par, K20, W[33 & 0xf] = Wgen (W, 33));
  SHA1STEP32 (b, c, d, e, a, Par, K20, W[34 & 0xf] = Wgen (W, 34));
  SHA1STEP32 (a, b, c, d, e, Par, K20, W[35 & 0xf] = Wgen (W, 35));
  SHA1STEP32 (e, a, b, c, d, Par, K20, W[36 & 0xf] = Wgen (W, 36));
  SHA1STEP32 (d, e, a, b, c, Par, K20, W[37 & 0xf] = Wgen (W, 37));
  SHA1STEP32 (c, d, e, a, b, Par, K20, W[38 & 0xf] = Wgen (W, 38));
  SHA1STEP32 (b, c, d, e, a, Par, K20, W[39 & 0xf] = Wgen (W, 39));
  SHA1STEP32 (a, b, c, d, e, Maj, K40, W[40 & 0xf] = Wgen (W, 40));
  SHA1STEP32 (e, a, b, c, d, Maj, K40, W[41 & 0xf] = Wgen (W, 41));
  SHA1STEP32 (d, e, a, b, c, Maj, K40, W[42 & 0xf] = Wgen (W, 42));
  SHA1STEP32 (c, d, e, a, b, Maj, K40, W[43 & 0xf] = Wgen (W, 43));
  SHA1STEP32 (b, c, d, e, a, Maj, K40, W[44 & 0xf] = Wgen (W, 44));
  SHA1STEP32 (a, b, c, d, e, Maj, K40, W[45 & 0xf] = Wgen (W, 45));
  SHA1STEP32 (e, a, b, c, d, Maj, K40, W[46 & 0xf] = Wgen (W, 46));
  SHA1STEP32 (d, e, a, b, c, Maj, K40, W[47 & 0xf] = Wgen (W, 47));
  SHA1STEP32 (c, d, e, a, b, Maj, K40, W[48 & 0xf] = Wgen (W, 48));
  SHA1STEP32 (b, c, d, e, a, Maj, K40, W[49 & 0xf] = Wgen (W, 49));
  SHA1STEP32 (a, b, c, d, e, Maj, K40, W[50 & 0xf] = Wgen (W, 50));
  SHA1STEP32 (e, a, b, c, d, Maj, K40, W[51 & 0xf] = Wgen (W, 51));
  SHA1STEP32 (d, e, a, b, c, Maj, K40, W[52 & 0xf] = Wgen (W, 52));
  SHA1STEP32 (c, d, e, a, b, Maj, K40, W[53 & 0xf] = Wgen (W, 53));
  SHA1STEP32 (b, c, d, e, a, Maj, K40, W[54 & 0xf] = Wgen (W, 54));
  SHA1STEP32 (a, b, c, d, e, Maj, K40, W[55 & 0xf] = Wgen (W, 55));
  SHA1STEP32 (e, a, b, c, d, Maj, K40, W[56 & 0xf] = Wgen (W, 56));
  SHA1STEP32 (d, e, a, b, c, Maj, K40, W[57 & 0xf] = Wgen (W, 57));
  SHA1STEP32 (c, d, e, a, b, Maj, K40, W[58 & 0xf] = Wgen (W, 58));
  SHA1STEP32 (b, c, d, e, a, Maj, K40, W[59 & 0xf] = Wgen (W, 59));
  SHA1STEP32 (a, b, c, d, e, Par, K60, W[60 & 0xf] = Wgen (W, 60));
  SHA1STEP32 (e, a, b, c, d, Par, K60, W[61 & 0xf] = Wgen (W, 61));
  SHA1STEP32 (d, e, a, b, c, Par, K60, W[62 & 0xf] = Wgen (W, 62));
  SHA1STEP32 (c, d, e, a, b, Par, K60, W[63 & 0xf] = Wgen (W, 63));
  SHA1STEP32 (b, c, d, e, a, Par, K60, W[64 & 0xf] = Wgen (W, 64));
  SHA1STEP32 (a, b, c, d, e, Par, K60, W[65 & 0xf] = Wgen (W, 65));
  SHA1STEP32 (e, a, b, c, d, Par, K60, W[66 & 0xf] = Wgen (W, 66));
  SHA1STEP32 (d, e, a, b, c, Par, K60, W[67 & 0xf] = Wgen (W, 67));
  SHA1STEP32 (c, d, e, a, b, Par, K60, W[68 & 0xf] = Wgen (W, 68));
  SHA1STEP32 (b, c, d, e, a, Par, K60, W[69 & 0xf] = Wgen (W, 69));
  SHA1STEP32 (a, b, c, d, e, Par, K60, W[70 & 0xf] = Wgen (W, 70));
  SHA1STEP32 (e, a, b, c, d, Par, K60, W[71 & 0xf] = Wgen (W, 71));
  SHA1STEP32 (d, e, a, b, c, Par, K60, W[72 & 0xf] = Wgen (W, 72));
  SHA1STEP32 (c, d, e, a, b, Par, K60, W[73 & 0xf] = Wgen (W, 73));
  SHA1STEP32 (b, c, d, e, a, Par, K60, W[74 & 0xf] = Wgen (W, 74));
  SHA1STEP32 (a, b, c, d, e, Par, K60, W[75 & 0xf] = Wgen (W, 75));
  SHA1STEP32 (e, a, b, c, d, Par, K60, W[76 & 0xf] = Wgen (W, 76));
  SHA1STEP32 (d, e, a, b, c, Par, K60, W[77 & 0xf] = Wgen (W, 77));
  SHA1STEP32 (c, d, e, a, b, Par, K60, W[78 & 0xf] = Wgen (W, 78));
  SHA1STEP32 (b, c, d, e, a, Par, K60, W[79 & 0xf] = Wgen (W, 79));

  /* Compute intermediate hash.
     See FIPS PUB 180-4 paragraph 6.1.3 step 4. */
  H[0] += a;
  H[1] += b;
  H[2] += c;
  H[3] += d;
  H[4] += e;
}


/**
 * Process portion of bytes.
 *
 * @param ctx_ must be a `struct sha1_ctx *`
 * @param data bytes to add to hash
 * @param length number of bytes in @a data
 */
void
MHD_SHA1_update (void *ctx_,
                 const uint8_t *data,
                 size_t length)
{
  struct sha1_ctx *const ctx = ctx_;
  unsigned bytes_have; /**< Number of bytes in buffer */

  mhd_assert ((data != NULL) || (length == 0));

  if (0 == length)
    return; /* Do nothing */

  /* Note: (count & (SHA1_BLOCK_SIZE-1))
           equal (count % SHA1_BLOCK_SIZE) for this block size. */
  bytes_have = (unsigned) (ctx->count & (SHA1_BLOCK_SIZE - 1));
  ctx->count += length;

  if (0 != bytes_have)
  {
    unsigned bytes_left = SHA1_BLOCK_SIZE - bytes_have;
    if (length >= bytes_left)
    {     /* Combine new data with the data in the buffer and
             process the full block. */
      memcpy (ctx->buffer + bytes_have,
              data,
              bytes_left);
      data += bytes_left;
      length -= bytes_left;
      sha1_transform (ctx->H, ctx->buffer);
      bytes_have = 0;
    }
  }

  while (SHA1_BLOCK_SIZE <= length)
  {   /* Process any full blocks of new data directly,
         without copying to the buffer. */
    sha1_transform (ctx->H, data);
    data += SHA1_BLOCK_SIZE;
    length -= SHA1_BLOCK_SIZE;
  }

  if (0 != length)
  {   /* Copy incomplete block of new data (if any)
         to the buffer. */
    memcpy (ctx->buffer + bytes_have, data, length);
  }
}


/**
 * Size of "length" padding addition in bytes.
 * See FIPS PUB 180-4 paragraph 5.1.1.
 */
#define SHA1_SIZE_OF_LEN_ADD (64 / 8)

/**
 * Finalise SHA-1 calculation, return digest.
 *
 * @param ctx_ must be a `struct sha1_ctx *`
 * @param[out] digest set to the hash, must be #SHA1_DIGEST_SIZE bytes
 */
void
MHD_SHA1_finish (void *ctx_,
                 uint8_t digest[SHA1_DIGEST_SIZE])
{
  struct sha1_ctx *const ctx = ctx_;
  uint64_t num_bits;   /**< Number of processed bits */
  unsigned bytes_have; /**< Number of bytes in buffer */

  num_bits = ctx->count << 3;
  /* Note: (count & (SHA1_BLOCK_SIZE-1))
           equals (count % SHA1_BLOCK_SIZE) for this block size. */
  bytes_have = (unsigned) (ctx->count & (SHA1_BLOCK_SIZE - 1));

  /* Input data must be padded with bit "1" and with length of data in bits.
     See FIPS PUB 180-4 paragraph 5.1.1. */
  /* Data is always processed in form of bytes (not by individual bits),
     therefore position of first padding bit in byte is always predefined (0x80). */
  /* Buffer always have space at least for one byte (as full buffers are
     processed immediately). */
  ctx->buffer[bytes_have++] = 0x80;

  if (SHA1_BLOCK_SIZE - bytes_have < SHA1_SIZE_OF_LEN_ADD)
  {   /* No space in current block to put total length of message.
         Pad current block with zeros and process it. */
    if (SHA1_BLOCK_SIZE > bytes_have)
      memset (ctx->buffer + bytes_have, 0, SHA1_BLOCK_SIZE - bytes_have);
    /* Process full block. */
    sha1_transform (ctx->H, ctx->buffer);
    /* Start new block. */
    bytes_have = 0;
  }

  /* Pad the rest of the buffer with zeros. */
  memset (ctx->buffer + bytes_have, 0,
          SHA1_BLOCK_SIZE - SHA1_SIZE_OF_LEN_ADD - bytes_have);
  /* Put the number of bits in the processed message as a big-endian value. */
  _MHD_PUT_64BIT_BE_SAFE (ctx->buffer + SHA1_BLOCK_SIZE - SHA1_SIZE_OF_LEN_ADD,
                          num_bits);
  /* Process the full final block. */
  sha1_transform (ctx->H, ctx->buffer);

  /* Put final hash/digest in BE mode */
#ifndef _MHD_PUT_32BIT_BE_UNALIGNED
  if (0 != ((uintptr_t) digest) % _MHD_UINT32_ALIGN)
  {
    uint32_t alig_dgst[_SHA1_DIGEST_LENGTH];
    _MHD_PUT_32BIT_BE (alig_dgst + 0, ctx->H[0]);
    _MHD_PUT_32BIT_BE (alig_dgst + 1, ctx->H[1]);
    _MHD_PUT_32BIT_BE (alig_dgst + 2, ctx->H[2]);
    _MHD_PUT_32BIT_BE (alig_dgst + 3, ctx->H[3]);
    _MHD_PUT_32BIT_BE (alig_dgst + 4, ctx->H[4]);
    /* Copy result to unaligned destination address */
    memcpy (digest, alig_dgst, SHA1_DIGEST_SIZE);
  }
  else
#else  /* _MHD_PUT_32BIT_BE_UNALIGNED */
  if (1)
#endif /* _MHD_PUT_32BIT_BE_UNALIGNED */
  {
    _MHD_PUT_32BIT_BE (digest + 0 * SHA1_BYTES_IN_WORD, ctx->H[0]);
    _MHD_PUT_32BIT_BE (digest + 1 * SHA1_BYTES_IN_WORD, ctx->H[1]);
    _MHD_PUT_32BIT_BE (digest + 2 * SHA1_BYTES_IN_WORD, ctx->H[2]);
    _MHD_PUT_32BIT_BE (digest + 3 * SHA1_BYTES_IN_WORD, ctx->H[3]);
    _MHD_PUT_32BIT_BE (digest + 4 * SHA1_BYTES_IN_WORD, ctx->H[4]);
  }

  /* Erase potentially sensitive data. */
  memset (ctx, 0, sizeof(struct sha1_ctx));
}
