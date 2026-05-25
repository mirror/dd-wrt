/* ISC license. */

#include <unistd.h>
#include <errno.h>

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

void fd_unlock (int fd)
{
  static struct flock const fl =
  {
    .l_type = F_UNLCK,
    .l_whence = SEEK_SET,
    .l_start = 0,
    .l_len = 0
  } ;
  int e = errno ;
  fcntl(fd, F_SETLK, &fl) ;
  errno = e ;
}
