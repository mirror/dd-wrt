/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/uint64.h>
#include "fmtscan-internal.h"

size_t int32_scan_base (char const *s, int32_t *d, uint8_t base)
{
  int64_t dd ;
  size_t pos = int64_scan_base_max(s, &dd, base, UINT32_MAX) ;
  if (pos) *d = (int32_t)dd ;
  return pos ;
}
