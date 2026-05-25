/* ISC license. */

#include <unistd.h>
#include <errno.h>

#include <skalibs/djbunix.h>

int fd_chown (int fd, uid_t uid, gid_t gid)
{
  int e = errno ;
  int r ;
  do r = fchown(fd, uid, gid) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}
