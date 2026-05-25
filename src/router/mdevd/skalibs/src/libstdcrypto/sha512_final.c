/* ISC license. */

#include <string.h>

#include <skalibs/uint64.h>
#include <skalibs/sha512.h>
#include "sha512-internal.h"

void sha512_final (SHA512Schedule *ctx, char *digest)
{
  unsigned int pad = ctx->len & 0x7fU ;

  ctx->buf[pad++] = 0x80 ;
  if (pad > 112)
  {
    memset(ctx->buf + pad, 0, 128 - pad) ;
    sha512_transform(ctx, ctx->buf) ;
    pad = 0 ;
  }
  memset(ctx->buf + pad, 0, 120 - pad) ;
  uint64_pack_big((char *)ctx->buf + 120, ctx->len << 3) ;
  sha512_transform(ctx, ctx->buf) ;

  for (unsigned int i = 0 ; i < 8 ; i++) uint64_pack_big(digest + (i << 3), ctx->h[i]) ;
}
