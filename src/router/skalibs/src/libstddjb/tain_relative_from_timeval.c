/* ISC license. */

#include <sys/time.h>
#include <skalibs/tai.h>

int tain_relative_from_timeval (tain *a, struct timeval const *tv)
{
  tai_relative_from_time(&a->sec, tv->tv_sec) ;
  a->nano = 1000 * tv->tv_usec ;
  return 1 ;
}
