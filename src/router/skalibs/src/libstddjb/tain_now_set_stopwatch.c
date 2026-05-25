/* ISC license. */

/* MT-unsafe */

#include <skalibs/sysdeps.h>
#include <skalibs/tai.h>

#if defined(SKALIBS_HASCLOCKRT) && (defined(SKALIBS_HASCLOCKMON) || defined(SKALIBS_HASCLOCKBOOT))

#include <time.h>

#ifdef SKALIBS_HASCLOCKMON
# define SKALIBS_STOPWATCH CLOCK_MONOTONIC
#else
# define SKALIBS_STOPWATCH CLOCK_BOOTTIME
#endif

static tain offset ;

static int tain_now_stopwatch (tain *now)
{
  return tain_stopwatch_read(now, SKALIBS_STOPWATCH, &offset) ;
}

int tain_now_set_stopwatch (tain *now)
{
  if (!tain_stopwatch_init(now, SKALIBS_STOPWATCH, &offset))
    return tain_now_set_wallclock(now) ;
  tain_now = &tain_now_stopwatch ;
  return 1 ;
}

#else

int tain_now_set_stopwatch (tain *now)
{
  return tain_now_set_wallclock(now) ;
}

#endif
