/* ISC license. */

#include <time.h>
#include <skalibs/uint64.h>
#include <skalibs/djbtime.h>

int utc_from_localtm (uint64_t *uu, struct tm const *l)
{
  if (!ltm64_from_localtm(uu, l)) return 0 ;
  utc_from_ltm64(uu) ;
  return 1 ;
}
