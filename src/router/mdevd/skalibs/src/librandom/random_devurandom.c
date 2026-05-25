/* ISC license. */

#include <stddef.h>
#include <errno.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/strerr.h>
#include <skalibs/djbunix.h>
#include <skalibs/random.h>

void random_devurandom (char *s, size_t n)
{
  static int random_fd = -1 ;
  size_t r ;
  int e = errno ;
  if (random_fd < 0)
  {
    random_fd = openbc_read("/dev/urandom") ;
    if (random_fd < 0)
      strerr_diefu2sys(111, "open ", "/dev/urandom") ;
  }
  errno = EPIPE ;
  r = allread(random_fd, s, n) ;
  if (r < n) strerr_diefu2sys(111, "read from ", "/dev/urandom") ;
  errno = e ;
}
