/* ISC license. */

#include <skalibs/uint64.h>
#include <skalibs/fmtscan.h>

size_t uint64_fmt_generic (char *s, uint64_t x, uint8_t base)
{
  size_t len = 1 ;
  uint64_t q = x ;
  while (q >= base) { len++ ; q /= base ; }
  if (s)
  {
    s += len ;
    do { *--s = fmtscan_asc(x % base) ; x /= base ; } while (x) ;
  }
  return len ;
}
