/* ISC license. */

#include <stddef.h>

#include <skalibs/uint32.h>

void uint32_bswapn (uint32_t *x, size_t n)
{
  while (n--) uint32_bswapp(x++) ;
}
