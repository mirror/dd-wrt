/* ISC license. */

#include <sys/uio.h>
#include <errno.h>

#include <skalibs/allreadwrite.h>

ssize_t fd_writev (int fd, struct iovec const *v, unsigned int vlen)
{
  int e = errno ;
  ssize_t r ;
  do r = writev(fd, v, vlen) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}
