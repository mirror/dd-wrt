/* ISC license. */

#include <skalibs/config.h>
#include <skalibs/uint64.h>
#include <skalibs/tai.h>

#ifdef SKALIBS_FLAG_CLOCKISTAI

int tai_from_sysclock (tai *t, uint64_t u)
{
  return tai_u64(t, u + 10U) ;
}

#else

#include <skalibs/djbtime.h>

int tai_from_sysclock (tai *t, uint64_t u)
{
  return tai_from_utc(t, u) ;
}

#endif
