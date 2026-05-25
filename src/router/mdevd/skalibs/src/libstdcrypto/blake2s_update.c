 /* ISC license. */

#include <string.h>

#include <skalibs/blake2s.h>
#include "blake2s-internal.h"

void blake2s_update (blake2s_ctx *ctx, char const *s, size_t len)
{
  size_t w = 64 - ctx->buflen ;
  if (!len) return ;
  if (len > w)
  {
    memcpy(ctx->buf + ctx->buflen, s, w) ;
    blake2s_transform(ctx, ctx->buf, 1, 64) ;
    ctx->buflen = 0 ;
    s += w ; len -= w ;
  }
  if (len > 64)
  {
    size_t n = (len+63)/64 - 1 ;
    blake2s_transform(ctx, s, n, 64) ;
    n <<= 6 ; s += n ; len -= n ;
  }
  memcpy(ctx->buf + ctx->buflen, s, len) ;
  ctx->buflen += len ;
}
