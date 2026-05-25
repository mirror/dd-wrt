/* ISC license. */

#include <unistd.h>

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

int fd_islocked (int fd)
{
  struct flock fl =
  {
    .l_type = F_RDLCK,
    .l_whence = SEEK_SET,
    .l_start = 0,
    .l_len = 0
  } ;
  return fcntl(fd, F_GETLK, &fl) == -1 ? -1 : fl.l_type != F_UNLCK ;
}
