/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.	This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MHD_MD5Init, call MHD_MD5Update as
 * needed on buffers full of bytes, and then call MHD_MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */

/* Based on OpenBSD modifications.
 * Optimized by Karlson2k (Evgeny Grin). */

#include "md5.h"
#include <string.h>
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif /* HAVE_MEMORY_H */
#include "mhd_bithelpers.h"
#include "mhd_assert.h"

/**
 * Number of bytes in single MD5 word
 * used to process data
 */
#define MD5_BYTES_IN_WORD (32 / 8)


/**
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 *
 * @param ctx must be a `struct MD5Context *`
 */
void
MHD_MD5Init (void *ctx_)
{
  struct MD5Context *ctx = ctx_;

  mhd_assert (ctx != NULL);
  ctx->count = 0;
  ctx->state[0] = UINT32_C (0x67452301);
  ctx->state[1] = UINT32_C (0xefcdab89);
  ctx->state[2] = UINT32_C (0x98badcfe);
  ctx->state[3] = UINT32_C (0x10325476);
}


static void
MD5Transform (uint32_t state[4],
              const uint8_t block[MD5_BLOCK_SIZE]);


/**
 * Final wrapup, fill in digest and zero out ctx.
 *
 * @param ctx must be a `struct MD5Context *`
 */
void
MHD_MD5Final (void *ctx_,
              uint8_t digest[MD5_DIGEST_SIZE])
{
  struct MD5Context *ctx = ctx_;
  uint64_t count_bits;
  size_t have_bytes;

  mhd_assert (ctx != NULL);
  mhd_assert (digest != NULL);

  /* Convert count to 8 bytes in little endian order. */
  have_bytes = (ctx->count) & (MD5_BLOCK_SIZE - 1);

  /* Pad data */
  /* Buffer always have space for one byte or more. */
  ctx->buffer[have_bytes++] = 0x80; /* First padding byte is 0x80 */

  if (MD5_BLOCK_SIZE - have_bytes < 8)
  {   /* Not enough space to put number of bits */
    while (have_bytes < MD5_BLOCK_SIZE)
      ctx->buffer[have_bytes++] = 0;
    MD5Transform (ctx->state, ctx->buffer);
    have_bytes = 0;   /* Additional block */
  }
  /* Pad out to 56 */
  memset (ctx->buffer + have_bytes, 0, MD5_BLOCK_SIZE - have_bytes - 8);

  /* Put number of bits */
  count_bits = ctx->count << 3;
  _MHD_PUT_64BIT_LE_SAFE (ctx->buffer + 56, count_bits);
  MD5Transform (ctx->state, ctx->buffer);

  /* Put digest in LE mode */
#ifndef _MHD_PUT_32BIT_LE_UNALIGNED
  if (0 != ((uintptr_t) digest) % _MHD_UINT32_ALIGN)
  {
    uint32_t alig_dgst[MD5_DIGEST_SIZE / MD5_BYTES_IN_WORD];
    _MHD_PUT_32BIT_LE (alig_dgst + 0, ctx->state[0]);
    _MHD_PUT_32BIT_LE (alig_dgst + 1, ctx->state[1]);
    _MHD_PUT_32BIT_LE (alig_dgst + 2, ctx->state[2]);
    _MHD_PUT_32BIT_LE (alig_dgst + 3, ctx->state[3]);
    /* Copy result to unaligned destination address */
    memcpy (digest, alig_dgst, MD5_DIGEST_SIZE);
  }
  else
#else  /* _MHD_PUT_32BIT_LE_UNALIGNED */
  if (1)
#endif /* _MHD_PUT_32BIT_LE_UNALIGNED */
  {
    _MHD_PUT_32BIT_LE (digest, ctx->state[0]);
    _MHD_PUT_32BIT_LE (digest + 4, ctx->state[1]);
    _MHD_PUT_32BIT_LE (digest + 8, ctx->state[2]);
    _MHD_PUT_32BIT_LE (digest + 12, ctx->state[3]);
  }

  /* Erase buffer */
  memset (ctx, 0, sizeof(*ctx));
}


/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1 (z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
  (w += f (x, y, z) + data,  w = _MHD_ROTL32(w, s),  w += x)

/**
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MHD_MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void
MD5Transform (uint32_t state[4],
              const uint8_t block[MD5_BLOCK_SIZE])
{
  uint32_t a, b, c, d;
  uint32_t data_buf[MD5_BLOCK_SIZE / MD5_BYTES_IN_WORD];
  const uint32_t *in;

#if (_MHD_BYTE_ORDER == _MHD_LITTLE_ENDIAN) || \
  ! defined(_MHD_GET_32BIT_LE_UNALIGNED)
  if (0 != (((uintptr_t) block) % _MHD_UINT32_ALIGN))
  {
    /* Copy data to the aligned buffer */
    memcpy (data_buf, block, MD5_BLOCK_SIZE);
    in = data_buf;
  }
  else
    in = (const uint32_t *) block;
#endif /* _MHD_BYTE_ORDER == _MHD_LITTLE_ENDIAN) || \
          ! _MHD_GET_32BIT_LE_UNALIGNED */
#if _MHD_BYTE_ORDER != _MHD_LITTLE_ENDIAN
  data_buf[0] = _MHD_GET_32BIT_LE (in + 0);
  data_buf[1] = _MHD_GET_32BIT_LE (in + 1);
  data_buf[2] = _MHD_GET_32BIT_LE (in + 2);
  data_buf[3] = _MHD_GET_32BIT_LE (in + 3);
  data_buf[4] = _MHD_GET_32BIT_LE (in + 4);
  data_buf[5] = _MHD_GET_32BIT_LE (in + 5);
  data_buf[6] = _MHD_GET_32BIT_LE (in + 6);
  data_buf[7] = _MHD_GET_32BIT_LE (in + 7);
  data_buf[8] = _MHD_GET_32BIT_LE (in + 8);
  data_buf[9] = _MHD_GET_32BIT_LE (in + 9);
  data_buf[10] = _MHD_GET_32BIT_LE (in + 10);
  data_buf[11] = _MHD_GET_32BIT_LE (in + 11);
  data_buf[12] = _MHD_GET_32BIT_LE (in + 12);
  data_buf[13] = _MHD_GET_32BIT_LE (in + 13);
  data_buf[14] = _MHD_GET_32BIT_LE (in + 14);
  data_buf[15] = _MHD_GET_32BIT_LE (in + 15);
  in = data_buf;
#endif /* _MHD_BYTE_ORDER != _MHD_LITTLE_ENDIAN */

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];

  MD5STEP (F1, a, b, c, d, in[0]  + UINT32_C (0xd76aa478), 7);
  MD5STEP (F1, d, a, b, c, in[1]  + UINT32_C (0xe8c7b756), 12);
  MD5STEP (F1, c, d, a, b, in[2]  + UINT32_C (0x242070db), 17);
  MD5STEP (F1, b, c, d, a, in[3]  + UINT32_C (0xc1bdceee), 22);
  MD5STEP (F1, a, b, c, d, in[4]  + UINT32_C (0xf57c0faf), 7);
  MD5STEP (F1, d, a, b, c, in[5]  + UINT32_C (0x4787c62a), 12);
  MD5STEP (F1, c, d, a, b, in[6]  + UINT32_C (0xa8304613), 17);
  MD5STEP (F1, b, c, d, a, in[7]  + UINT32_C (0xfd469501), 22);
  MD5STEP (F1, a, b, c, d, in[8]  + UINT32_C (0x698098d8), 7);
  MD5STEP (F1, d, a, b, c, in[9]  + UINT32_C (0x8b44f7af), 12);
  MD5STEP (F1, c, d, a, b, in[10] + UINT32_C (0xffff5bb1), 17);
  MD5STEP (F1, b, c, d, a, in[11] + UINT32_C (0x895cd7be), 22);
  MD5STEP (F1, a, b, c, d, in[12] + UINT32_C (0x6b901122), 7);
  MD5STEP (F1, d, a, b, c, in[13] + UINT32_C (0xfd987193), 12);
  MD5STEP (F1, c, d, a, b, in[14] + UINT32_C (0xa679438e), 17);
  MD5STEP (F1, b, c, d, a, in[15] + UINT32_C (0x49b40821), 22);

  MD5STEP (F2, a, b, c, d, in[1]  + UINT32_C (0xf61e2562), 5);
  MD5STEP (F2, d, a, b, c, in[6]  + UINT32_C (0xc040b340), 9);
  MD5STEP (F2, c, d, a, b, in[11] + UINT32_C (0x265e5a51), 14);
  MD5STEP (F2, b, c, d, a, in[0]  + UINT32_C (0xe9b6c7aa), 20);
  MD5STEP (F2, a, b, c, d, in[5]  + UINT32_C (0xd62f105d), 5);
  MD5STEP (F2, d, a, b, c, in[10] + UINT32_C (0x02441453), 9);
  MD5STEP (F2, c, d, a, b, in[15] + UINT32_C (0xd8a1e681), 14);
  MD5STEP (F2, b, c, d, a, in[4]  + UINT32_C (0xe7d3fbc8), 20);
  MD5STEP (F2, a, b, c, d, in[9]  + UINT32_C (0x21e1cde6), 5);
  MD5STEP (F2, d, a, b, c, in[14] + UINT32_C (0xc33707d6), 9);
  MD5STEP (F2, c, d, a, b, in[3]  + UINT32_C (0xf4d50d87), 14);
  MD5STEP (F2, b, c, d, a, in[8]  + UINT32_C (0x455a14ed), 20);
  MD5STEP (F2, a, b, c, d, in[13] + UINT32_C (0xa9e3e905), 5);
  MD5STEP (F2, d, a, b, c, in[2]  + UINT32_C (0xfcefa3f8), 9);
  MD5STEP (F2, c, d, a, b, in[7]  + UINT32_C (0x676f02d9), 14);
  MD5STEP (F2, b, c, d, a, in[12] + UINT32_C (0x8d2a4c8a), 20);

  MD5STEP (F3, a, b, c, d, in[5]  + UINT32_C (0xfffa3942), 4);
  MD5STEP (F3, d, a, b, c, in[8]  + UINT32_C (0x8771f681), 11);
  MD5STEP (F3, c, d, a, b, in[11] + UINT32_C (0x6d9d6122), 16);
  MD5STEP (F3, b, c, d, a, in[14] + UINT32_C (0xfde5380c), 23);
  MD5STEP (F3, a, b, c, d, in[1]  + UINT32_C (0xa4beea44), 4);
  MD5STEP (F3, d, a, b, c, in[4]  + UINT32_C (0x4bdecfa9), 11);
  MD5STEP (F3, c, d, a, b, in[7]  + UINT32_C (0xf6bb4b60), 16);
  MD5STEP (F3, b, c, d, a, in[10] + UINT32_C (0xbebfbc70), 23);
  MD5STEP (F3, a, b, c, d, in[13] + UINT32_C (0x289b7ec6), 4);
  MD5STEP (F3, d, a, b, c, in[0]  + UINT32_C (0xeaa127fa), 11);
  MD5STEP (F3, c, d, a, b, in[3]  + UINT32_C (0xd4ef3085), 16);
  MD5STEP (F3, b, c, d, a, in[6]  + UINT32_C (0x04881d05), 23);
  MD5STEP (F3, a, b, c, d, in[9]  + UINT32_C (0xd9d4d039), 4);
  MD5STEP (F3, d, a, b, c, in[12] + UINT32_C (0xe6db99e5), 11);
  MD5STEP (F3, c, d, a, b, in[15] + UINT32_C (0x1fa27cf8), 16);
  MD5STEP (F3, b, c, d, a, in[2]  + UINT32_C (0xc4ac5665), 23);

  MD5STEP (F4, a, b, c, d, in[0]  + UINT32_C (0xf4292244), 6);
  MD5STEP (F4, d, a, b, c, in[7]  + UINT32_C (0x432aff97), 10);
  MD5STEP (F4, c, d, a, b, in[14] + UINT32_C (0xab9423a7), 15);
  MD5STEP (F4, b, c, d, a, in[5]  + UINT32_C (0xfc93a039), 21);
  MD5STEP (F4, a, b, c, d, in[12] + UINT32_C (0x655b59c3), 6);
  MD5STEP (F4, d, a, b, c, in[3]  + UINT32_C (0x8f0ccc92), 10);
  MD5STEP (F4, c, d, a, b, in[10] + UINT32_C (0xffeff47d), 15);
  MD5STEP (F4, b, c, d, a, in[1]  + UINT32_C (0x85845dd1), 21);
  MD5STEP (F4, a, b, c, d, in[8]  + UINT32_C (0x6fa87e4f), 6);
  MD5STEP (F4, d, a, b, c, in[15] + UINT32_C (0xfe2ce6e0), 10);
  MD5STEP (F4, c, d, a, b, in[6]  + UINT32_C (0xa3014314), 15);
  MD5STEP (F4, b, c, d, a, in[13] + UINT32_C (0x4e0811a1), 21);
  MD5STEP (F4, a, b, c, d, in[4]  + UINT32_C (0xf7537e82), 6);
  MD5STEP (F4, d, a, b, c, in[11] + UINT32_C (0xbd3af235), 10);
  MD5STEP (F4, c, d, a, b, in[2]  + UINT32_C (0x2ad7d2bb), 15);
  MD5STEP (F4, b, c, d, a, in[9]  + UINT32_C (0xeb86d391), 21);

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
}


/**
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void
MHD_MD5Update (void *ctx_,
               const uint8_t *input,
               size_t len)
{
  struct MD5Context *ctx = ctx_;
  size_t have, need;

  mhd_assert (ctx != NULL);
  mhd_assert ((ctx != NULL) || (len == 0));

  /* Check how many bytes we already have and how many more we need. */
  have = (size_t) ((ctx->count) & (MD5_BLOCK_SIZE - 1));
  need = MD5_BLOCK_SIZE - have;

  /* Update bytecount */
  ctx->count += (uint64_t) len;

  if (len >= need)
  {
    if (have != 0)
    {
      memcpy (ctx->buffer + have,
              input,
              need);
      MD5Transform (ctx->state, ctx->buffer);
      input += need;
      len -= need;
      have = 0;
    }

    /* Process data in MD5_BLOCK_SIZE-byte chunks. */
    while (len >= MD5_BLOCK_SIZE)
    {
      MD5Transform (ctx->state,
                    (const unsigned char *) input);
      input += MD5_BLOCK_SIZE;
      len -= MD5_BLOCK_SIZE;
    }
  }

  /* Handle any remaining bytes of data. */
  if (0 != len)
    memcpy (ctx->buffer + have,
            input,
            len);
}


/* end of md5.c */
