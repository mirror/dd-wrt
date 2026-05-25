/* ISC license. */

#include <stddef.h>

#include <skalibs/uint16.h>

void uint16_bswapn (uint16_t *x, size_t n)
{
  while (n--) uint16_bswapp(x++) ;
}
