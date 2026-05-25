/* ISC license. */

#include <stddef.h>

#include <skalibs/uint64.h>

void uint64_bswapn (uint64_t *x, size_t n)
{
  while (n--) uint64_bswapp(x++) ;
}
