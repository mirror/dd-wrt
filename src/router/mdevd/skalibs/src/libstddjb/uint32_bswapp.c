/* ISC license. */

#include <skalibs/uint32.h>

void uint32_bswapp (uint32_t *x)
{
  *x = uint32_bswap(*x) ;
}
