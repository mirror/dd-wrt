/* ISC license. */

#include <errno.h>
#include <skalibs/stralloc.h>

int stralloc_readyplus_tuned (stralloc *sa, size_t n, size_t base, size_t a, size_t b)
{
  size_t newlen = sa->len + n ;
  return newlen < sa->len ?
    (errno = ERANGE, 0) :
    stralloc_ready_tuned(sa, newlen, base, a, b) ;
}
