/* ISC license. */

#include <sys/uio.h>
#include <skalibs/siovec.h>
#include <skalibs/allreadwrite.h>

size_t allreadwritev (iov_func_ref op, int fd, struct iovec const *v, unsigned int vlen)
{
  size_t written = 0 ;
  struct iovec vv[vlen ? vlen : 1] ;
  unsigned int i = vlen ;
  while (i--) vv[i] = v[i] ;
  while (siovec_len(vv, vlen))
  {
    ssize_t w = (*op)(fd, vv, vlen) ;
    if (w <= 0) break ;
    w = siovec_seek(vv, vlen, w) ;
    written += w ;
  }
  return written ;
}
