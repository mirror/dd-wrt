/* ISC license. */

#include <skalibs/bytestr.h>

size_t byte_rchr (char const *s, size_t n, int c)
{
  size_t i = n ;
  char ch = c ;
  s += n ;
  while (i--) if (*--s == ch) return i ;
  return n ;
}
