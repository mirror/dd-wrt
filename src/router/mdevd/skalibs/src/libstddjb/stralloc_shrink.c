/* ISC license. */

#include <skalibs/alloc.h>
#include <skalibs/stralloc.h>

int stralloc_shrink (stralloc *sa)
{
  if (sa->a > sa->len)
  {
    if (!alloc_re(&sa->s, sa->a, sa->len)) return 0 ;
    sa->a = sa->len ;
  }
  return 1 ;
}
