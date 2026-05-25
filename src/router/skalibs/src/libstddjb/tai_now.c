/* ISC license. */

/* MT-unsafe */

#include <time.h>
#include <skalibs/tai.h>

int tai_now (tai *t)
{
  time_t u = time(0) ;
  if (u == (time_t)-1) return 0 ;
  return tai_from_time_sysclock(t, u) ;
}
