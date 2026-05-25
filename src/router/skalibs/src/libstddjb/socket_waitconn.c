/* ISC license. */

#include <errno.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/iopause.h>
#include <skalibs/socket.h>

int socket_waitconn (int s, tain const *deadline, tain *stamp)
{
  iopause_fd x = { s, IOPAUSE_WRITE, 0 } ;
  for (;;)
  {
    int r = iopause_stamp(&x, 1, deadline, stamp) ;
    if (r < 0) return 0 ;
    if (!r) return (errno = ETIMEDOUT, 0) ;
    if (x.revents & IOPAUSE_WRITE) break ;
    if (x.revents & IOPAUSE_EXCEPT)
    {
      fd_write(s, "", 1) ; /* sets errno */
      return 0 ;
    }
  }
  return socket_connected(s) ;
}
