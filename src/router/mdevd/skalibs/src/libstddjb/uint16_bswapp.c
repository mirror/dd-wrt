/* ISC license. */

#include <skalibs/uint16.h>

void uint16_bswapp (uint16_t *x)
{
  *x = uint16_bswap(*x) ;
}
