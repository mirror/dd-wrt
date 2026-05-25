/* ISC license. */

#include <skalibs/uint16.h>
#include <skalibs/uint64.h>
#include "fmtscan-internal.h"

size_t int16_scan_base (char const *s, int16_t *d, uint8_t base)
{
  int64_t dd ;
  size_t pos = int64_scan_base_max(s, &dd, base, UINT16_MAX) ;
  if (pos) *d = (uint16_t)dd ;
  return pos ;
}
