 /* ISC license. */

#include <stdint.h>

#include <skalibs/uint32.h>
#include <skalibs/blake2s.h>
#include "blake2s-internal.h"

static uint32_t const blake2s_iv[8] =
{
  0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
  0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

static uint8_t const blake2s_sigma[10][16] =
{
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
  { 14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
  { 11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 },
  { 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 },
  { 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 },
  { 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 },
  { 12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11 },
  { 13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10 },
  { 6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5 },
  { 10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0 }
} ;

static uint32_t ror32 (uint32_t word, unsigned int shift)
{
  return (word >> (shift & 31)) | (word << ((-shift) & 31)) ;
}

#define G(r, i, a, b, c, d) do \
{ \
  a += b + m[blake2s_sigma[r][i<<1]] ; \
  d = ror32(d ^ a, 16) ; \
  c += d ; \
  b = ror32(b ^ c, 12) ; \
  a += b + m[blake2s_sigma[r][(i<<1)+1]] ; \
  d = ror32(d ^ a, 8) ; \
  c += d ; \
  b = ror32(b ^ c, 7) ; \
} while (0)

void blake2s_transform (blake2s_ctx *ctx, char const *block, size_t n, uint32_t inc)
{
  uint32_t m[16] ;
  uint32_t v[16] ;

  while (n--)
  {
    unsigned int i = 0 ;
    ctx->t[0] += inc ;
    ctx->t[1] += (ctx->t[0] < inc) ;
    for (; i < 16 ; i++) uint32_unpack(block + (i << 2), m + i) ;
    for (i = 0 ; i < 8 ; i++) v[i] = ctx->h[i] ;
    v[8] = blake2s_iv[0] ;
    v[9] = blake2s_iv[1] ;
    v[10] = blake2s_iv[2] ;
    v[11] = blake2s_iv[3] ;
    v[12] = blake2s_iv[4] ^ ctx->t[0] ;
    v[13] = blake2s_iv[5] ^ ctx->t[1] ;
    v[14] = blake2s_iv[6] ^ ctx->f[0] ;
    v[15] = blake2s_iv[7] ^ ctx->f[1] ;

    for (i = 0 ; i < 10 ; i++)
    {
      G(i, 0, v[0], v[ 4], v[ 8], v[12]) ;
      G(i, 1, v[1], v[ 5], v[ 9], v[13]) ;
      G(i, 2, v[2], v[ 6], v[10], v[14]) ;
      G(i, 3, v[3], v[ 7], v[11], v[15]) ;
      G(i, 4, v[0], v[ 5], v[10], v[15]) ;
      G(i, 5, v[1], v[ 6], v[11], v[12]) ;
      G(i, 6, v[2], v[ 7], v[ 8], v[13]) ;
      G(i, 7, v[3], v[ 4], v[ 9], v[14]) ;
    }

    for (i = 0 ; i < 8 ; i++) ctx->h[i] ^= v[i] ^ v[i+8] ;
    block += 64 ;
  }
}
