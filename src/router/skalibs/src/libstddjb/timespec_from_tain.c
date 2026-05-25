/* ISC license. */

#include <time.h>
#include <skalibs/tai.h>

int timespec_from_tain (struct timespec *ts, tain const *t)
{
  if (!time_from_tai(&ts->tv_sec, &t->sec)) return 0 ;
  ts->tv_nsec = t->nano ;
  return 1 ;
}
