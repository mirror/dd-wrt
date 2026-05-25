/* ISC license. */

#include <time.h>
#include <skalibs/uint64.h>
#include <skalibs/djbtime.h>

int localtm_from_tai (struct tm *l, tai const *t, int tz)
{
  uint64_t u ;
  int h = ltm64_from_tai(&u, t) ;
  return h ? localtm_from_ltm64(l, u, !!tz | (h & 2)) : 0 ;
}
