/* ISC license. */

#include <sys/uio.h>
#include <string.h>

#include <skalibs/siovec.h>

size_t siovec_gather (struct iovec const *v, unsigned int n, char *s, size_t max)
{
  size_t w = 0 ;
  for (unsigned int i = 0 ; i < n && w < max ; i++)
  {
    size_t len = v[i].iov_len ;
    if (len > max - w) len = max - w ;
    if (len) memmove(s + w, v[i].iov_base, len) ;
    w += len ;
  }
  return w ;
}
