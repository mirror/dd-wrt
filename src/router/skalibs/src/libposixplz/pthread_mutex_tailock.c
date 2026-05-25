/* ISC license. */

#include <errno.h>
#include <time.h>
#include <pthread.h>

#include <skalibs/sysdeps.h>
#include <skalibs/tai.h>
#include <skalibs/pthread.h>

#ifdef SKALIBS_HASPTHREADMUTEXTIMEDLOCK

int pthread_mutex_tailock (pthread_mutex_t *mtx, tain const *deadline, tain *stamp)
{
  int e ;
  struct timespec ts ;
  tain tto ;
  if (!stamp) return pthread_mutex_lock(mtx) ;
  tain_sub(&tto, deadline, stamp) ;
  if (!timespec_from_tain_relative(&ts, &tto)) return errno ;
#ifdef SKALIBS_HASPTHREADMUTEXCLOCKLOCK
  if (&tain_now != &tain_wallclock_read)
    e = pthread_mutex_clocklock(mtx, CLOCK_MONOTONIC, &ts) ;
  else
#else
    e = pthread_mutex_timedlock(mtx, &ts) ;
#endif
  tain_now(stamp) ;
  return e ;
}

#else

int pthread_mutex_tailock (pthread_mutex_t *mtx, tain const *deadline, tain *stamp)
{
  (void)deadline ;
  (void)stamp ;
  return pthread_mutex_lock(mtx) ;
}

#endif
