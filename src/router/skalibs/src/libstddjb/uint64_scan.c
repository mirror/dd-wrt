/* ISC license. */

#include <skalibs/uint64.h>
#include "fmtscan-internal.h"

size_t uint64_scan_base (char const *s, uint64_t *u, uint8_t base)
{
  return uint64_scan_base_max(s, u, base, UINT64_MAX) ;
}
