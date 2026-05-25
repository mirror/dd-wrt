 /* ISC license. */

#include <string.h>

#include <skalibs/uint32.h>
#include <skalibs/blake2s.h>
#include "blake2s-internal.h"

void blake2s_final (blake2s_ctx *ctx, char *out)
{
  ctx->f[0] = -1 ;
  memset(ctx->buf + ctx->buflen, 0, 64 - ctx->buflen) ;
  blake2s_transform(ctx, ctx->buf, 1, ctx->buflen) ;
  uint32_littlen(ctx->h, 8) ;
  memcpy(out, ctx->h, ctx->outlen) ;
}
