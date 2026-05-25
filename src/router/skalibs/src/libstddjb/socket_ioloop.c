/* ISC license. */

#include <errno.h>

#include <skalibs/error.h>
#include <skalibs/iopause.h>
#include <skalibs/socket.h>

ssize_t socket_ioloop (int s, char *buf, size_t len, char *ip, uint16_t *port, socket_io_func_ref f, int w, tain const *deadline, tain *stamp)
{
  iopause_fd x = { .fd = s, .events = w ? IOPAUSE_WRITE : IOPAUSE_READ, .revents = 0 } ;
  for (;;)
  {
    ssize_t r = iopause_stamp(&x, 1, deadline, stamp) ;
    if (r < 0) return -1 ;
    if (!r) return (errno = ETIMEDOUT, -1) ;
    if (x.revents & IOPAUSE_EXCEPT) return (errno = EIO, -1) ;
    if (x.revents & x.events)
    {
      r = (*f)(s, buf, len, ip, port) ;
      if (r < 0)
      {
        if (!error_isagain(errno)) return -1 ;
      }
      else return r ;
    }
  }
}

