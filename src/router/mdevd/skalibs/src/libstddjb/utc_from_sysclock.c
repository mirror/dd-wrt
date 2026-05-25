/* ISC license. */

#include <skalibs/config.h>
#include <skalibs/uint64.h>
#include <skalibs/djbtime.h>

#ifdef SKALIBS_FLAG_CLOCKISTAI

#include <skalibs/tai.h>

int utc_from_sysclock (uint64_t *u)
{
  tai t = { .x = *u + 10U } ;
  return utc_from_tai(u, &t) ;
}

#else

int utc_from_sysclock (uint64_t *u)
{
  (void)u ;
  return 1 ;
}

#endif
