/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/bsdsnowflake.h>

#include <string.h>  /* Solaris... */
#include <errno.h>
#include <sys/time.h>  /* MacOS... */
#include <sys/select.h>

#include <skalibs/tai.h>
#include <skalibs/iopause.h>

int iopause_select (iopause_fd *x, unsigned int len, tain const *deadline, tain const *stamp)
{
  struct timeval tv = { .tv_sec = 0, .tv_usec = 0 } ;
  int nfds = 0 ;
  fd_set rfds, wfds, xfds ;
  int r ;
  
  FD_ZERO(&rfds) ;
  FD_ZERO(&wfds) ;
  FD_ZERO(&xfds) ;
  if (deadline && tain_less(stamp, deadline))
  {
    tain delta ;
    tain_sub(&delta, deadline, stamp) ;
    if (!timeval_from_tain_relative(&tv, &delta))
    {
      if (errno != EOVERFLOW) return -1 ;
      else deadline = 0 ;
    }
#ifndef SKALIBS_HASSELECTINFINITE
    if (deadline && tv.tv_sec >= 100000000)
    {
      tv.tv_sec = 100000000 ;
      tv.tv_usec = 0 ;
    }
#endif
  }

  for (unsigned int i = 0 ; i < len ; i++)
  {
    x[i].revents = 0 ;
    if (x[i].fd >= 0)
    {
      if (x[i].fd >= FD_SETSIZE) return (errno = EMFILE, -1) ;
      if (x[i].fd >= nfds) nfds = x[i].fd + 1 ;
      if ((x[i].events & IOPAUSE_READ) == IOPAUSE_READ) FD_SET(x[i].fd, &rfds) ;
      if ((x[i].events & IOPAUSE_WRITE) == IOPAUSE_WRITE) FD_SET(x[i].fd, &wfds) ;
      if ((x[i].events & IOPAUSE_EXCEPT) == IOPAUSE_EXCEPT) FD_SET(x[i].fd, &xfds) ;
    }
  }

  r = select(nfds, &rfds, &wfds, &xfds, deadline ? &tv : 0) ;

  if (r > 0)
    for (unsigned int i = 0 ; i < len ; i++) if (x[i].fd >= 0)
    {
      if (FD_ISSET(x[i].fd, &rfds)) x[i].revents |= IOPAUSE_READ ;
      if (FD_ISSET(x[i].fd, &wfds)) x[i].revents |= IOPAUSE_WRITE ;
      if (FD_ISSET(x[i].fd, &xfds)) x[i].revents |= IOPAUSE_EXCEPT | x[i].events ;
    }

  return r ;
}
