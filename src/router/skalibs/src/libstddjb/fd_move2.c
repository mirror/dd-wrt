/* ISC license. */

#include <unistd.h>
#include <errno.h>
#include <skalibs/djbunix.h>

int fd_move2 (int to1, int from1, int to2, int from2)
{
  int tmp = from2 ;
  if (to1 == from1) return fd_move(to2, from2) ;
  if (to2 == from2) return fd_move(to1, from1) ;
  if (from1 == from2) return (to1 == to2) ? fd_move(to1, from1) : (errno = EINVAL, -1) ;
  if (to1 == to2) return (errno = EINVAL, -1) ;
  if (from2 == to1)
  {
    tmp = dup(from2) ;
    if (tmp == -1) return -1 ;
  }
  if (fd_copy(to1, from1) == -1)
  {
    if (from2 != tmp) fd_close(tmp) ;
    return -1 ;
  }
  if (fd_copy(to2, tmp) == -1)
  {
    int e = errno ;
    fd_close(to1) ;
    if (from2 != tmp) fd_move(from2, tmp) ;
    errno = e ;
    return -1 ;
  }
  if (from1 != to2) fd_close(from1) ;
  fd_close(tmp) ;
  return 0 ;
}
