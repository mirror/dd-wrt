/* ISC license. */

#include <sys/uio.h>
#include <skalibs/siovec.h>

unsigned int siovec_trunc (struct iovec *v, unsigned int n, size_t len)
{
  size_t w = siovec_len(v, n) ;
  if (w < len) return n ;
  len = w - len ;
  while (n && len)
  {
    w = len > v[n-1].iov_len ? v[n-1].iov_len : len ;
    v[n-1].iov_len -= w ; len -= w ;
    if (!v[n-1].iov_len) n-- ;
  }
  return n ;
}
