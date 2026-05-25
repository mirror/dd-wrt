/* ISC license. */

#include <skalibs/uint16.h>
#include <skalibs/uint64.h>
#include "fmtscan-internal.h"

size_t uint16_scan_base (char const *s, uint16_t *u, uint8_t base)
{
  uint64_t uu ;
  size_t pos = uint64_scan_base_max(s, &uu, base, UINT16_MAX) ;
  if (pos) *u = (uint16_t)uu ;
  return pos ;
}
