/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/tai.h>

#ifdef SKALIBS_HASCLOCKRT

#include <time.h>

int sysclock_get (tain *a)
{
  tain aa ;
  struct timespec now ;
  if (clock_gettime(CLOCK_REALTIME, &now) < 0) return 0 ;
  if (!tain_from_timespec(&aa, &now)) return 0 ;
  tain_add(a, &aa, &tain_nano500) ;
  return 1 ;
}

#else

#include <sys/time.h>

int sysclock_get (tain *a)
{
  tain aa ;
  struct timeval now ;
  if (gettimeofday(&now, 0) < 0) return 0 ;
  if (!tain_from_timeval(&aa, &now)) return 0 ;
  tain_add(a, &aa, &tain_nano500) ;
  return 1 ;
}

#endif
