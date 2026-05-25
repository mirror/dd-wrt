/* ISC license. */

#include <string.h>

#include <skalibs/sha512.h>
#include "sha512-internal.h"

void sha512_update (SHA512Schedule *ctx, char const *buf, size_t len)
{
  unsigned int pad = ctx->len & 0x7fU ;
  ctx->len += len ;
  if (pad && len >= 128 - pad)
  {
    memcpy((char *)ctx->buf + pad, buf, 128 - pad) ;
    buf += 128 - pad ; len -= 128 - pad ; pad = 0 ;
    sha512_transform(ctx, ctx->buf) ;
  }

  while (len >= 128)
  {
    sha512_transform(ctx, (unsigned char const *)buf) ;
    buf += 128 ; len -= 128 ;
  }
  memcpy((char *)ctx->buf + pad, buf, len) ;
}
