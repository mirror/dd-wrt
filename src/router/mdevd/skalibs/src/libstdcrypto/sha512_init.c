/* ISC license. */

#include <skalibs/sha512.h>

void sha512_init (SHA512Schedule *ctx)
{
  SHA512Schedule empty = SHA512_INIT() ;
  *ctx = empty ;
}
