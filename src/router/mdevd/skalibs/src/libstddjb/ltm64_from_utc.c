/* ISC license. */

#include <skalibs/uint64.h>
#include <skalibs/djbtime.h>
#include "djbtime-internal.h"

int ltm64_from_utc (uint64_t *u)
{
  switch (skalibs_tzisright())
  {
    case 1 : leapsecs_add(u, 0) ;
    case 0 : return 1 ;
    default : return 0 ;
  }
}
