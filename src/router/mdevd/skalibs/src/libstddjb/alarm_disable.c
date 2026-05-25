/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/alarm.h>

#ifdef SKALIBS_HASTIMER

#include <time.h>
#include "alarm-internal.h"

timer_t timer_here ;

void alarm_disable ()
{
  struct itimerspec stopit = { .it_value = { .tv_sec = 0, .tv_nsec = 0 }, .it_interval = { .tv_sec = 0, .tv_nsec = 0 } } ;
  timer_settime(timer_here, 0, &stopit, 0) ;
  timer_delete(timer_here) ;
}

#else
#ifdef SKALIBS_HASITIMER

#include <sys/time.h>

void alarm_disable ()
{
  struct itimerval stopit = { .it_value = { .tv_sec = 0, .tv_usec = 0 }, .it_interval = { .tv_sec = 0, .tv_usec = 0 } } ;
  setitimer(ITIMER_REAL, &stopit, 0) ;
}

#else

#include <unistd.h>

void alarm_disable ()
{
  alarm(0) ;
}

#endif
#endif
