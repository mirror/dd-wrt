/* ISC license. */

#include <errno.h>
#include <skalibs/djbunix.h>

int fd_copy2 (int to1, int from1, int to2, int from2)
{
  if ((to1 == from2) || (to2 == from1)) return (errno = EINVAL, -1) ;
  if (fd_copy(to1, from1) == -1) return -1 ;
  if (fd_copy(to2, from2) == -1)
  {
    if (to1 != from1) fd_close(to1) ;
    return -1 ;
  }
  return 0 ;
}
