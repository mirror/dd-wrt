/* ISC license. */

#include <errno.h>
#include <skalibs/djbtime.h>
#include "djbtime-internal.h"

int utc_from_tai (uint64_t *u, tai const *t)
{
  int r = 1 ;
  uint64_t tt = t->x - 10 ;
  if (t->x < 10U) return (errno = EINVAL, 0) ;
  r += leapsecs_sub(&tt) ;
  *u = tt ;
  return r ;
}
