/* ISC license. */

#include <errno.h>

#include <skalibs/error.h>
#include <skalibs/iopause.h>
#include <skalibs/socket.h>

int ipc_timed_connect (int s, char const *path, tain const *deadline, tain *stamp)
{
  if (!ipc_connect(s, path))
  {
    iopause_fd x = { s, IOPAUSE_WRITE, 0 } ;
    if (!error_isagain(errno) && !error_isalready(errno)) return 0 ;
    for (;;)
    {
      int r = iopause_stamp(&x, 1, deadline, stamp) ;
      if (r < 0) return 0 ;
      else if (!r) return (errno = ETIMEDOUT, 0) ;
      else if (x.revents & IOPAUSE_EXCEPT) return 0 ;
      else if (x.revents & IOPAUSE_WRITE) break ;
    }
    if (!ipc_connected(s)) return 0 ;
  }
  return 1 ;
}
