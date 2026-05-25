/* ISC license. */

#include <skalibs/sha256.h>

void sha256_init (SHA256Schedule *ctx)
{
  SHA256Schedule empty = SHA256_INIT() ;
  *ctx = empty ;
}
