/* ISC license. */

#include <string.h>

#include <skalibs/uint64.h>

void uint64_pack_big (char *s, uint64_t u)
{
  uint64_bigp(&u) ;
  memcpy(s, &u, sizeof(uint64_t)) ;
}
