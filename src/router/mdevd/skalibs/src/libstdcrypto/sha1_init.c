/* ISC license. */

#include <skalibs/sha1.h>

void sha1_init (SHA1Schedule *ctx)
{
  ctx->buf[0] = 0x67452301U ;
  ctx->buf[1] = 0xefcdab89U ;
  ctx->buf[2] = 0x98badcfeU ;
  ctx->buf[3] = 0x10325476U ;
  ctx->buf[4] = 0xc3d2e1f0U ;
  ctx->bits[0] = ctx->bits[1] = 0 ;
  for (unsigned int i = 0 ; i < 16 ; i++) ctx->in[i] = 0 ;
  ctx->b = 0 ;
}
