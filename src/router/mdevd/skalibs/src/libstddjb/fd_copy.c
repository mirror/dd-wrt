/* ISC license. */

#include <unistd.h>
#include <errno.h>

#include <skalibs/djbunix.h>

int fd_copy (int to, int from)
{
  int e = errno ;
  int r ;
  if (to == from) return (errno = EINVAL, -1) ;
  do r = dup2(from, to) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}
