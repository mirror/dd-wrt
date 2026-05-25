/* ISC license. */

#include <time.h>
#include <skalibs/uint64.h>
#include <skalibs/djbtime.h>

int tai_from_localtm (tai *t, struct tm const *l)
{
  uint64_t u ;
  if (!ltm64_from_localtm(&u, l)) return 0 ;
  return tai_from_ltm64(t, u) ;
}
