/* ISC license. */

#include <skalibs/uint64.h>

void uint64_bswapp (uint64_t *x)
{
  *x = uint64_bswap(*x) ;
}
