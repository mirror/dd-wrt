/* ISC license. */

#include <time.h>
#include <skalibs/tai.h>

int tain_relative_from_timespec (tain *a, struct timespec const *ts)
{
  tai_relative_from_time(&a->sec, ts->tv_sec) ;
  a->nano = ts->tv_nsec ;
  return 1 ;
}
