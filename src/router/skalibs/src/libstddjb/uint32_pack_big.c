/* ISC license. */

#include <stdint.h>
#include <string.h>

#include <skalibs/uint32.h>

void uint32_pack_big (char *s, uint32_t u)
{
  uint32_bigp(&u) ;
  memcpy(s, &u, sizeof(uint32_t)) ;
}
