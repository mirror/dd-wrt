/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/sha1.h>
#include "sha1-internal.h"

void sha1_final (SHA1Schedule *ctx, char *digest)
{
  char pack[8] ;
  uint32_pack_big(pack, ctx->bits[1]) ;
  uint32_pack_big(pack+4, ctx->bits[0]) ;
  sha1_feed(ctx, 0x80) ;
  while (ctx->b != 56) sha1_feed(ctx, 0) ;
  sha1_update(ctx, pack, 8) ;
  for (unsigned int i = 0 ; i < 5 ; i++)
    uint32_pack_big(digest + (i<<2), ctx->buf[i]) ;
}
