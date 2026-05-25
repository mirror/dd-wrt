/* ISC license. */

#include <string.h>
#include <skalibs/stralloc.h>

int stralloc_catb (stralloc *sa, char const *s, size_t n)
{
  if (!stralloc_readyplus(sa, n)) return 0 ;
  memmove(sa->s + sa->len, s, n) ;
  sa->len += n ;
  return 1 ;
}
