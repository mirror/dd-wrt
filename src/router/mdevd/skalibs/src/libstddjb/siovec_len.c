/* ISC license. */

#include <sys/uio.h>
#include <skalibs/siovec.h>

size_t siovec_len (struct iovec const *v, unsigned int n)
{
  size_t w = 0 ;
  while (n--) w += v[n].iov_len ;
  return w ;
}
