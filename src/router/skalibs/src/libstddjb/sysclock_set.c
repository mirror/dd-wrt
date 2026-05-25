/* ISC license. */

/* MT-unsafe */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASCLOCKRT

#include <time.h>
#include <skalibs/tai.h>

int sysclock_set (tain const *a)
{
  struct timespec now ;
  tain aa ;
  tain_add(&aa, a, &tain_nano500) ;
  if (!timespec_from_tain(&now, &aa)) return 0 ;
  if (clock_settime(CLOCK_REALTIME, &now) < 0) return 0 ;
  return 1 ;
}

#else
#ifdef SKALIBS_HASSETTIMEOFDAY

#include <skalibs/nonposix.h>
#include <sys/time.h>
#include <skalibs/tai.h>

int sysclock_set (tain const *a)
{
  struct timeval now ;
  tain aa ;
  tain_add(&aa, a, &tain_nano500) ;
  if (!timeval_from_tain(&now, &aa)) return 0 ;
  if (settimeofday(&now, 0) < 0) return 0 ;
  return 1 ;
}

#else

#error "neither clockrt nor settimeofday sysdeps are present. How do your set your system clock?"

#endif
#endif
