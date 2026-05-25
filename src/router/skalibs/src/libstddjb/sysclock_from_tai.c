/* ISC license. */

#include <errno.h>
#include <skalibs/config.h>
#include <skalibs/tai.h>

#ifdef SKALIBS_FLAG_CLOCKISTAI

int sysclock_from_tai (uint64_t *u, tai const *t)
{
  if (t->x < 10U) return (errno = EINVAL, 0) ;
  *u = t->x - 10U ;
  return 1 ;
}

#else

#include <skalibs/djbtime.h>

int sysclock_from_tai (uint64_t *u, tai const *t)
{
  return utc_from_tai(u, t) ;
}

#endif
