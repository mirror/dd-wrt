/* ISC license. */

#include <sys/uio.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>

ssize_t buffer_flush1read (int fd, struct iovec const *v, unsigned int n)
{
  if (!buffer_flush(buffer_1)) return -1 ;
  return fd_readv(fd, v, n) ;
}
