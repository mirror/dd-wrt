/* ISC license. */

#include <string.h>
#include <sys/uio.h>
#include <skalibs/siovec.h>
#include <skalibs/stralloc.h>

int stralloc_catv (stralloc *sa, struct iovec const *v, unsigned int n)
{
  unsigned int i = 0 ;
  if (!stralloc_readyplus(sa, siovec_len(v, n))) return 0 ;
  for ( ; i < n ; i++)
  {
    memmove(sa->s + sa->len, v[i].iov_base, v[i].iov_len) ;
    sa->len += v[i].iov_len ;
  }
  return 1 ;
}
