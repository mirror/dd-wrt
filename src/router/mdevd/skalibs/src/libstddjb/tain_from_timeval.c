/* ISC license. */

#include <sys/time.h>
#include <skalibs/tai.h>

int tain_from_timeval (tain *t, struct timeval const *tv)
{
  if (!tai_from_time(&t->sec, tv->tv_sec)) return 0 ;
  t->nano = 1000 * tv->tv_usec ;
  return 1 ;
}
