/* ISC license. */

#include <unistd.h>
#include <errno.h>

#include <skalibs/djbunix.h>

int fd_chdir (int d)
{
  int e = errno ;
  int r ;
  do r = fchdir(d) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}
