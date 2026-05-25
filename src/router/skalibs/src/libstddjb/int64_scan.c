/* ISC license. */

#include <skalibs/uint64.h>
#include "fmtscan-internal.h"

size_t int64_scan_base (char const *s, int64_t *d, uint8_t base)
{
  return int64_scan_base_max(s, d, base, INT64_MAX) ;
}
