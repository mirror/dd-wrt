/* ISC license. */

#include <sys/time.h>
#include <skalibs/tai.h>

int timeval_from_tain_relative (struct timeval *tv, tain const *t)
{
  if (!time_from_tai_relative(&tv->tv_sec, &t->sec)) return 0 ;
  tv->tv_usec = (t->nano + 500) / 1000 ;
  return 1 ;
}
