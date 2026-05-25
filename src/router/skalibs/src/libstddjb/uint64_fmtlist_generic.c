/* ISC license. */

#include <skalibs/uint64.h>

size_t uint64_fmtlist_generic (char *s, void const *tab, size_t n, uint8_t base, uint64_t (*get)(void const *, size_t))
{
  size_t i = 0, len = 0 ;
  for (; i < n ; i++)
  {
    size_t w = uint64_fmt_generic(s, (*get)(tab, i), base) ;
    len += w ;
    if (s) s += w ;
    if (i < n-1) { len++ ; if (s) *s++ = ',' ; }
  }
  return len ;
}
