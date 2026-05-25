/* ISC license. */

#include <skalibs/nonposix.h>
#include <errno.h>
#include <sys/socket.h>

#include <skalibs/djbunix.h>

void fd_shutdown (int fd, int h)
{
  int e = errno ;
  shutdown(fd, h ? SHUT_WR : SHUT_RD) ;
  errno = e ;
}
