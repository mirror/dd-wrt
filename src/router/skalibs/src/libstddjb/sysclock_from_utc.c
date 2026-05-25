/* ISC license. */

#include <errno.h>
#include <skalibs/config.h>
#include <skalibs/uint64.h>
#include <skalibs/djbtime.h>

#ifdef SKALIBS_FLAG_CLOCKISTAI

int sysclock_from_utc (uint64_t *u)
{
  tai t ;
  if (!tai_from_utc(&t, *u)) return 0 ;
  if (t.x < 10) return (errno = EINVAL, 0) ;
  *u = t.x - 10 ;
  return 1 ;
}

#else

int sysclock_from_utc (uint64_t *u)
{
  (void)u ;
  return 1 ;
}

#endif
