/* ISC license. */

#include <string.h>
#include <skalibs/bytestr.h>

size_t byte_chr (char const *s, size_t n, int c)
{
  void *p ;
  if (!n) return 0 ;
  p = memchr(s, c, n) ;
  return p ? (char *)p - s : n ;
}
