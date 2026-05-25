/* ISC license. */

#include <string.h>

#include <skalibs/uint64.h>
#include <skalibs/fmtscan.h>

size_t uint640_fmt_generic (char *s, uint64_t x, size_t n, uint8_t base)
{
  size_t len = uint64_fmt_generic(0, x, base) ;
  if (s)
  {
    if (len < n)
    {
      memset(s, '0', n - len) ;
      s += n - len ;
    }
    uint64_fmt_generic(s, x, base) ;
  }
  return len > n ? len : n ;
}
