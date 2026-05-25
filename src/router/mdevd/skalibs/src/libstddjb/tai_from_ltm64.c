/* ISC license. */

#include <skalibs/djbtime.h>
#include "djbtime-internal.h"

int tai_from_ltm64 (tai *t, uint64_t u)
{
  switch (skalibs_tzisright())
  {
    case 1 : return tai_u64(t, u + 10U) ;
    case 0 : return tai_from_utc(t, u) ;
    default : return 0 ;
  }
}
