/* ISC license. */

#include <time.h>
#include <skalibs/uint64.h>
#include <skalibs/djbtime.h>

int sysclock_from_localtm (uint64_t *uu, struct tm const *l)
{
  uint64_t u ;
  if (!ltm64_from_localtm(&u, l)) return 0 ;
  if (!sysclock_from_ltm64(&u)) return 0 ;
  *uu = u ;
  return 1 ;
}
