/* ISC license. */

#include <sys/time.h>
#include <skalibs/tai.h>

int tain_from_timeval_sysclock (tain *t, struct timeval const *tv)
{
  if (!tai_from_time_sysclock(&t->sec, tv->tv_sec)) return 0 ;
  t->nano = 1000 * tv->tv_usec ;
  return 1 ;
}
