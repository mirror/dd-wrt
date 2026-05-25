/* ISC license. */

#include <sys/time.h>
#include <skalibs/tai.h>

int timeval_sysclock_from_tain (struct timeval *tv, tain const *t)
{
  if (!time_sysclock_from_tai(&tv->tv_sec, &t->sec)) return 0 ;
  tv->tv_usec = (t->nano + 500) / 1000 ;
  return 1 ;
}
