/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/alarm.h>

#ifdef SKALIBS_HASTIMER

#include <errno.h>
#include <signal.h>
#include <time.h>
#include "alarm-internal.h"

#undef MYCLOCK
#ifdef SKALIBS_HASCLOCKMON
# define MYCLOCK CLOCK_MONOTONIC
#else
# define MYCLOCK CLOCK_REALTIME
#endif

int alarm_milliseconds (unsigned int t)
{
  struct itimerspec it = { .it_interval = { .tv_sec = 0, .tv_nsec = 0 }, .it_value = { .tv_sec = t / 1000, .tv_nsec = 1000000 * (t % 1000) } } ;
  struct sigevent se = { .sigev_notify = SIGEV_SIGNAL, .sigev_signo = SIGALRM, .sigev_value = { .sival_int = 0 }, .sigev_notify_function = 0, .sigev_notify_attributes = 0 } ;
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

int alarm_milliseconds (unsigned int t)
{
  struct itimerval it = { .it_interval = { .tv_sec = 0, .tv_usec = 0 }, .it_value = { .tv_sec = t / 1000, .tv_usec = 1000 * (t % 1000) } } ;
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

int alarm_milliseconds (unsigned int t)
{
  if (t > UINT_MAX - 999) return 0 ;
  t = (t + 999) / 1000 ;
  alarm(t) ;
  return 1 ;
}

#endif
#endif
