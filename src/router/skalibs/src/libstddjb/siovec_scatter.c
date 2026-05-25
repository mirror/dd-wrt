/* ISC license. */

#include <sys/uio.h>
#include <string.h>
#include <skalibs/siovec.h>

size_t siovec_scatter (struct iovec const *v, unsigned int n, char const *s, size_t len)
{
  size_t w = 0 ;
  unsigned int i = 0 ;
  for (; i < n && w < len ; i++)
  {
    size_t chunklen = v[i].iov_len ;
    if (w + chunklen > len) chunklen = len - w ;
    memmove(v[i].iov_base, s + w, chunklen) ;
    w += chunklen ;
  }
  return w ;
}
