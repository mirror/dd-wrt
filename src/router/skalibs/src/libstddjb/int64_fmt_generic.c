/* ISC license. */

#include <skalibs/uint64.h>

size_t int64_fmt_generic (char *fmt, int64_t d, uint8_t base)
{
  if (d >= 0) return uint64_fmt_generic(fmt, (uint64_t)d, base) ;
  if (fmt) *fmt++ = '-' ;
  return 1 + uint64_fmt_generic(fmt, -(uint64_t)d, base) ;
}
