/* ISC license. */

#include <skalibs/sha1.h>
#include "sha1-internal.h"

void sha1_update (SHA1Schedule *ctx, char const *buf, size_t len)
{
  size_t i = 0 ;
  for (; i < len ; i++) sha1_feed(ctx, (unsigned char)buf[i]) ;
}
