/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASKEVENT

#include <skalibs/nonposix.h>

#include <pthread.h>

#include <skalibs/djbunix.h>
#include <skalibs/keventbridge.h>

void keventbridge_end (keventbridge *kbp)
{
  pthread_cancel(kbp->th) ;
  pthread_join(kbp->th, 0) ;
  fd_close(kbp->kq) ;
  if (kbp->p[1] >= 0) fd_close(kbp->p[1]) ;
  fd_close(kbp->p[0]) ;
}

#endif
