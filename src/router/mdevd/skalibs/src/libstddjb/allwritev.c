/* ISC license. */

#include <sys/uio.h>
#include <skalibs/allreadwrite.h>

size_t allwritev (int fd, struct iovec const *v, unsigned int vlen)
{
  return allreadwritev(&fd_writev, fd, v, vlen) ;
}
