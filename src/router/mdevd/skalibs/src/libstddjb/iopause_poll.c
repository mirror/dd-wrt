/* ISC license. */

#include <errno.h>
#include <poll.h>

#include <skalibs/tai.h>
#include <skalibs/iopause.h>

int iopause_poll (iopause_fd *x, unsigned int len, tain const *deadline, tain const *stamp)
{
  int r ;
  int millisecs = 0 ;
  if (!deadline) millisecs = -1 ;
  else if (tain_less(stamp, deadline))
  {
    tain t ;
    tain_sub(&t, deadline, stamp) ;
    millisecs = tain_to_millisecs(&t) ;
  }
  r = poll(x, len, millisecs) ;
  if (r > 0)
    for (unsigned int i = 0 ; i < len ; i++)
      if (x[i].revents & IOPAUSE_EXCEPT)
        x[i].revents |= x[i].events ;
  return r ;
}
