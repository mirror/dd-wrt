/* ISC license. */

#include <sys/socket.h>
#include <errno.h>

#include <skalibs/allreadwrite.h>

ssize_t fd_recv (int fd, char *buf, size_t len, unsigned int flags)
{
  int e = errno ;
  ssize_t r ;
  do r = recv(fd, buf, len, flags) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}
