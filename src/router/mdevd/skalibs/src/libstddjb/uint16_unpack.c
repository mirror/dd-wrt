/* ISC license. */

#include <stdint.h>
#include <string.h>

#include <skalibs/uint16.h>

void uint16_unpack (char const *s, uint16_t *u)
{
  uint16_t x ;
  memcpy(&x, s, sizeof(uint16_t)) ;
  *u = uint16_little(x) ;
}
