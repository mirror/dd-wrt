/* ISC license. */

#include <sys/uio.h>
#include <skalibs/bytestr.h>
#include <skalibs/siovec.h>

size_t siovec_bytechr (struct iovec const *v, unsigned int n, char c)
{
  size_t w = 0 ;
  unsigned int i = 0 ;
  for (; i < n ; i++)
  {
    size_t pos = byte_chr((char const *)v[i].iov_base, v[i].iov_len, c) ;
    w += pos ;
    if (pos < v[i].iov_len) break ;
  }
  return w ;
}
