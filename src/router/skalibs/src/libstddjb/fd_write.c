/* ISC license. */

#include <unistd.h>
#include <errno.h>

#include <skalibs/allreadwrite.h>

ssize_t fd_write (int fd, char const *buf, size_t len)
{
  int e = errno ;
  ssize_t r ;
  do r = write(fd, buf, len) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}
