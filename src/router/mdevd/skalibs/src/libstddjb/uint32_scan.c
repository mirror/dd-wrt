/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/uint64.h>
#include "fmtscan-internal.h"

size_t uint32_scan_base (char const *s, uint32_t *u, uint8_t base)
{
  uint64_t uu ;
  size_t pos = uint64_scan_base_max(s, &uu, base, UINT32_MAX) ;
  if (pos) *u = (uint32_t)uu ;
  return pos ;
}
