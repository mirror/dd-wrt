/* ISC license. */

#include <unistd.h>
#include <errno.h>

#include <skalibs/djbunix.h>

int fd_move (int to, int from)
{
  int e = errno ;
  int r ;
  if (to == from) return uncoe(to) ;
  do r = dup2(from, to) ;
  while (r == -1 && errno == EINTR) ;
  if (r < 0) return r ;
  errno = e ;
  fd_close(from) ;
  return 0 ;
}
