/* ISC license. */

#include <skalibs/config.h>
#include <skalibs/uint64.h>
#include <skalibs/djbtime.h>
#include "djbtime-internal.h"

int utc_from_ltm64 (uint64_t *u)
{
#ifdef SKALIBS_FLAG_CLOCKISTAI
  leapsecs_sub(u) ;
#endif
  return 1 ;
}
