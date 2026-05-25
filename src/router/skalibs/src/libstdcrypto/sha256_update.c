/* ISC license. */

#include <skalibs/sha256.h>
#include "sha256-internal.h"

void sha256_update (SHA256Schedule *ctx, char const *buf, size_t len)
{
  for (size_t i = 0 ; i < len ; i++) sha256_feed(ctx, (unsigned char)buf[i]) ;
}
