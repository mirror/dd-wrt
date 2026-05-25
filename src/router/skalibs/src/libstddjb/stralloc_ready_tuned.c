/* ISC license. */

#include <string.h>
#include <errno.h>

#include <skalibs/alloc.h>
#include <skalibs/stralloc.h>

int stralloc_ready_tuned (stralloc *sa, size_t n, size_t base, size_t a, size_t b)
{
  size_t t ;
  if (!b) return (errno = EINVAL, 0) ;
  t = n + base + a * n / b ;
  if (t < n) return (errno = ERANGE, 0) ;
  if (!sa->s)
  {
    sa->s = alloc(t) ;
    if (!sa->s) return 0 ;
    sa->a = t ;
    memset(sa->s, 0, t) ;
  }
  else if (n > sa->a)
  {
    if (!alloc_re(&sa->s, sa->a, t)) return 0 ;
    memset(sa->s + sa->a, 0, t - sa->a) ;
    sa->a = t ;
  }
  return 1 ;
}
