/* ISC license. */

#include <time.h>

#include <skalibs/djbtime.h>

int timespec_cmp (struct timespec const *a, struct timespec const *b)
{
  if (a->tv_sec < b->tv_sec) return -1 ;
  if (a->tv_sec > b->tv_sec) return 1 ;
  if (a->tv_nsec < b->tv_nsec) return -1 ;
  return a->tv_nsec > b->tv_nsec ;
}
