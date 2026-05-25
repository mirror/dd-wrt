/* ISC license. */

#include <stdint.h>
#include <skalibs/bytestr.h>
#include <skalibs/sha1.h>
#include "sha1-internal.h"

void sha1_feed (SHA1Schedule *ctx, unsigned char inb)
{
  uint32_t tmp ;
  ctx->in[ctx->b>>2] <<= 8 ;
  ctx->in[ctx->b>>2] |= T8(inb) ;
  if (++ctx->b >= 64)
  {
    sha1_transform(ctx->buf, ctx->in) ;
    ctx->b = 0 ;
    for (uint32_t i = 0 ; i < 16 ; i++) ctx->in[i] = 0 ;
  }
  tmp = ctx->bits[0] ;
  ctx->bits[0] += 8 ;
  if (tmp > ctx->bits[0]) ctx->bits[1]++ ;
}
