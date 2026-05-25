/* ISC license. */

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

int ndelay_off (int fd)
{
  int got = fcntl(fd, F_GETFL) ;
  return got < 0 ? got : got & O_NONBLOCK ? fcntl(fd, F_SETFL, got & ~O_NONBLOCK) : 0 ;
}
