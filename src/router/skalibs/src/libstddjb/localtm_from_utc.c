/* ISC license. */

#include <time.h>
#include <skalibs/uint64.h>
#include <skalibs/djbtime.h>

int localtm_from_utc (struct tm *l, uint64_t u, int tz)
{
  if (!ltm64_from_utc(&u)) return 0 ;
  return localtm_from_ltm64(l, u, !!tz) ;
}
