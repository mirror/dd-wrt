/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASPPOLL

#include <skalibs/nonposix.h>
#include <errno.h>
#include <time.h>
#include <poll.h>
#include <skalibs/tai.h>
#include <skalibs/iopause.h>

int iopause_ppoll (iopause_fd *x, unsigned int len, tain const *deadline, tain const *stamp)
{
  int r ;
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 } ;
  if (deadline && tain_less(stamp, deadline))
  {
    tain delta ;
    tain_sub(&delta, deadline, stamp) ;
    if (!timespec_from_tain_relative(&ts, &delta))
    {
      if (errno != EOVERFLOW) return -1 ;
      else deadline = 0 ;
    }
  }
  r = ppoll(x, len, deadline ? &ts : 0, 0) ;
  if (r > 0)
    for (unsigned int i = 0 ; i < len ; i++)
      if (x[i].revents & IOPAUSE_EXCEPT)
        x[i].revents |= x[i].events ;
  return r ;
}

#else

#include <errno.h>
#include <skalibs/iopause.h>

int iopause_ppoll (iopause_fd *x, unsigned int len, tain const *deadline, tain const *stamp)
{
  (void)x ;
  (void)len ;
  (void)deadline ;
  (void)stamp ;
  return (errno = ENOSYS, -1) ;
}

#endif
