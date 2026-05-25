/* ISC license. */

#include <errno.h>
#include <skalibs/stralloc.h>
#include <skalibs/skamisc.h>

int skagetlnsep_loose (buffer *b, stralloc *sa, char const *sep, size_t seplen)
{
  int e = errno ;
  int r = skagetlnsep(b, sa, sep, seplen) ;
  if (r >= 0) return r ;
  if (errno != EPIPE) return -1 ;
  if (!stralloc_0(sa)) return -1 ;
  return (errno = e, 3) ;
}
