/* ISC license. */

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

int coe (int fd)
{
  int flags = fcntl(fd, F_GETFD, 0) ;
  return flags < 0 ? flags : flags & FD_CLOEXEC ? 0 : fcntl(fd, F_SETFD, flags | FD_CLOEXEC) ;
}
