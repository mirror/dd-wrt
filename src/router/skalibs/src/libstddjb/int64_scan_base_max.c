/* ISC license. */

#include <skalibs/uint64.h>
#include "fmtscan-internal.h"

size_t int64_scan_base_max (char const *s, int64_t *d, uint8_t base, uint64_t max)
{
  if (s[0] == '-')
  {
    uint64_t u ;
    size_t pos = uint64_scan_base_max(s+1, &u, base, max+1) ;
    return pos ? (*d = -u, 1 + pos) : 0 ;
  }
  else if (s[0] == '+')
  {
    size_t pos = uint64_scan_base_max(s+1, (uint64_t *)d, base, max) ;
    return pos ? 1 + pos : 0 ;
  }
  else return uint64_scan_base_max(s, (uint64_t *)d, base, max) ;
}
