/* ISC license. */

#include <sys/uio.h>
#include <skalibs/bytestr.h>
#include <skalibs/siovec.h>

size_t siovec_bytein (struct iovec const *v, unsigned int n, char const *sep, size_t seplen)
{
  size_t w = 0 ;
  unsigned int i = 0 ;
  for (; i < n ; i++)
  {
    size_t pos = byte_in((char const *)v[i].iov_base, v[i].iov_len, sep, seplen) ;
    w += pos ;
    if (pos < v[i].iov_len) break ;
  }
  return w ;
}
