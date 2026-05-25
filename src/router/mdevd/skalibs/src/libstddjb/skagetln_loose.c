/* ISC license. */

#include <errno.h>
#include <skalibs/stralloc.h>
#include <skalibs/skamisc.h>

int skagetln_loose (buffer *b, stralloc *sa, char sep)
{
  int e = errno ;
  int r = skagetln(b, sa, sep) ;
  if (r >= 0) return r ;
  if (errno != EPIPE) return -1 ;
  if (!stralloc_0(sa)) return -1 ;
  return (errno = e, 3) ;
}
