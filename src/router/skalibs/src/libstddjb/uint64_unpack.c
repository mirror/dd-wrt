/* ISC license. */

#include <string.h>

#include <skalibs/uint64.h>

void uint64_unpack (char const *s, uint64_t *u)
{
  uint64_t x ;
  memcpy(&x, s, sizeof(uint64_t)) ;
  *u = uint64_little(x) ;
}
