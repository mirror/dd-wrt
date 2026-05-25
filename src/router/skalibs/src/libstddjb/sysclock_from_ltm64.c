/* ISC license. */

#include <skalibs/config.h>
#include <skalibs/uint64.h>
#include <skalibs/djbtime.h>

#ifdef SKALIBS_FLAG_CLOCKISTAI

int sysclock_from_ltm64 (uint64_t *u)
{
  tai t ;
  if (!tai_from_ltm64(&t, *u)) return 0 ;
  *u = t.x - 10U ;
  return 1 ;
}

#else

int sysclock_from_ltm64 (uint64_t *u)
{
  return utc_from_sysclock(u) ;
}

#endif
