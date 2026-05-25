/* ISC license. */

#include <errno.h>
#include <unistd.h>
#include <skalibs/djbunix.h>

void fd_close (int fd)
{
  int e = errno ;
  close(fd) ;
  errno = e ;
}
