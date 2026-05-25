/* ISC license. */

 /* OpenBSD manages to bork the fchmod declaration */

#include <skalibs/nonposix.h>

#include <sys/stat.h>
#include <errno.h>

#include <skalibs/djbunix.h>

int fd_chmod (int fd, unsigned int mode)
{
  int e = errno ;
  int r ;
  do r = fchmod(fd, (mode_t)mode) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}
