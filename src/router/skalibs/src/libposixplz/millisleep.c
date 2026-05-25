/* ISC license. */

#include <time.h>

#include <skalibs/posixplz.h>

unsigned int millisleep (unsigned int msec)
{
  struct timespec ts = { .tv_sec = msec / 1000, .tv_nsec = (long)(msec % 1000) * 1000000U } ;
  struct timespec tr ;
  return nanosleep(&ts, &tr) == -1 ? 1000 * tr.tv_sec + tr.tv_nsec / 1000000U : 0 ;
}
