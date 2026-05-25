/* ISC license. */

#include <skalibs/djbunix.h>
#include <skalibs/unix-transactional.h>

int open_appendatb (int dirfd, char const *name)
{
  int fd = open_appendat(dirfd, name) ;
  if (fd == -1) return -1 ;
  if (ndelay_off(fd) == -1)
  {
    fd_close(fd) ;
    return -1 ;
  }
  return fd ;
}
