/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/tai.h>
#include <skalibs/alarm.h>

#ifdef SKALIBS_HASTIMER

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include "alarm-internal.h"

#undef MYCLOCK
#ifdef SKALIBS_HASCLOCKMON
# define MYCLOCK CLOCK_MONOTONIC
#else
# define MYCLOCK CLOCK_REALTIME
#endif

int alarm_timeout (tain const *tto)
{
  struct itimerspec it = { .it_interval = { .tv_sec = 0, .tv_nsec = 0 } } ;
  struct sigevent se = { .sigev_notify = SIGEV_SIGNAL, .sigev_signo = SIGALRM, .sigev_value = { .sival_int = 0 }, .sigev_notify_function = 0, .sigev_notify_attributes = 0 } ;
  if (!timespec_from_tain_relative(&it.it_value, tto))
  {
    it.it_value.tv_sec = INT_MAX ;
    it.it_value.tv_nsec = 0 ;
  }
  if (timer_create(MYCLOCK, &se, &timer_here) < 0) return 0 ;
  if (timer_settime(timer_here, 0, &it, 0) < 0)
  {
    int e = errno ;
    timer_delete(timer_here) ;
    errno = e ;
    return 0 ;
  }
  return 1 ;
}

#else
#ifdef SKALIBS_HASITIMER

#include <sys/time.h>
#include <errno.h>
#include <limits.h>

int alarm_timeout (tain const *tto)
{
  struct itimerval it = { .it_interval = { .tv_sec = 0, .tv_usec = 0 } } ;
  if (!timeval_from_tain_relative(&it.it_value, tto))
  {
    it.it_value.tv_sec = INT_MAX ;
    it.it_value.tv_usec = 0 ;
  }
  if (setitimer(ITIMER_REAL, &it, 0) < 0)
  {
    if (errno != EINVAL) return 0 ;
    it.it_value.tv_sec = 9999999 ;
    if (setitimer(ITIMER_REAL, &it, 0) < 0) return 0 ;
  }
  return 1 ;
}

#else

#include <unistd.h>
#include <limits.h>

int alarm_timeout (tain const *tto)
{
  int t = tain_to_millisecs(tto) ;
  if (t < 0 || t > INT_MAX - 999) t = INT_MAX - 999 ;
  t = (t + 999) / 1000 ;
  alarm(t) ;
  return 1 ;
}

#endif
#endif
