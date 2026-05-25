/* ISC license. */

#include <skalibs/uint64.h>
#include <skalibs/fmtscan.h>

size_t uint64_scan_base_max (char const *s, uint64_t *u, uint8_t base, uint64_t max)
{
  uint64_t result = 0 ;
  size_t pos = 0 ;
  for (;; pos++)
  {
    uint8_t c = fmtscan_num(s[pos], base) ;
    if (c >= base || result > ((max - c) / base)) break ;
    result = result * base + c ;
  }
  if (pos) *u = result ;
  return pos ;
}
