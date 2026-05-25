/* ISC license. */

#include <string.h>
#include <errno.h>
#include <skalibs/stralloc.h>

int stralloc_insertb (stralloc *sa, size_t offset, char const *s, size_t n)
{
  if (offset > sa->len) return (errno = EINVAL, 0) ;
  if (!stralloc_readyplus(sa, n)) return 0 ;
  memmove(sa->s + offset + n, sa->s + offset, sa->len - offset) ;
  sa->len += n ;
  memmove(sa->s + offset, s, n) ;
  return 1 ;
}
