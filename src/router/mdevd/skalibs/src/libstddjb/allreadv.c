/* ISC license. */

#include <sys/uio.h>
#include <skalibs/allreadwrite.h>

size_t allreadv (int fd, struct iovec const *v, unsigned int vlen)
{
  return allreadwritev(&fd_readv, fd, v, vlen) ;
}
