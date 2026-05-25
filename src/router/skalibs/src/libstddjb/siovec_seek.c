/* ISC license. */

#include <sys/uio.h>
#include <skalibs/bytestr.h>
#include <skalibs/siovec.h>

size_t siovec_seek (struct iovec *v, unsigned int n, size_t len)
{
  size_t w = 0 ;
  unsigned int i = 0 ;
  for (; i < n ; i++)
  {
    if (len < v[i].iov_len) break ;
    w += v[i].iov_len ;
    len -= v[i].iov_len ;
    v[i].iov_base = 0 ;
    v[i].iov_len = 0 ;
  }
  if (i < n)
  {
    v[i].iov_base = (char *)v[i].iov_base + len ;
    v[i].iov_len -= len ;
    w += len ;
  }
  return w ;
}
