/* ISC license. */

#include <errno.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>

ssize_t readnclose (int fd, char *s, size_t n)
{
  int e = errno ;
  size_t r ;
  errno = 0 ;
  r = allread(fd, s, n) ;
  fd_close(fd) ;
  if (errno) return -1 ;
  errno = e ;
  return r ;
}
