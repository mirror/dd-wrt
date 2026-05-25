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

int alarm_deadline (tain const *deadline)
{
  struct itimerspec it = { .it_interval = { .tv_sec = 0, .tv_nsec = 0 } } ;
  struct sigevent se = { .sigev_notify = SIGEV_SIGNAL, .sigev_signo = SIGALRM, .sigev_value = { .sival_int = 0 }, .sigev_notify_function = 0, .sigev_notify_attributes = 0 } ;
  if (!timespec_from_tain(&it.it_value, deadline))
  {
    it.it_value.tv_sec = INT_MAX ;
    it.it_value.tv_nsec = 0 ;
  }
  if (timer_create(MYCLOCK, &se, &timer_here) < 0) return 0 ;
  if (timer_settime(timer_here, TIMER_ABSTIME, &it, 0) < 0)
  {
    int e = errno ;
    timer_delete(timer_here) ;
    errno = e ;
    return 0 ;
  }
  return 1 ;
}

#else

int alarm_deadline (tain const *deadline)
{
  tain tto ;
  tain_now(&tto) ;
  tain_sub(&tto, deadline, &tto) ;
  return alarm_timeout(&tto) ;
}

#endif
