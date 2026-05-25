/* ISC license. */

#include <time.h>
#include <skalibs/tai.h>

int tain_from_timespec (tain *t, struct timespec const *ts)
{
  if (!tai_from_time(&t->sec, ts->tv_sec)) return 0 ;
  t->nano = ts->tv_nsec ;
  return 1 ;
}
