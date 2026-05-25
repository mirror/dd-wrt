/* ISC license. */

#include <string.h>
#include <skalibs/bytestr.h>

size_t byte_in (char const *s, size_t n, char const *sep, size_t len)
{
  char const *t = s ;
  while (n--)
  {
    if (memchr(sep, *t, len)) break ;
    ++t ;
  }
  return t - s ;
}
