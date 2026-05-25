/* ISC license. */

#include <skalibs/tai.h>
#include <skalibs/djbtime.h>
#include "djbtime-internal.h"

int tai_from_utc (tai *t, uint64_t u)
{
  leapsecs_add(&u, 0) ;
  return tai_u64(t, u + 10) ;
}
