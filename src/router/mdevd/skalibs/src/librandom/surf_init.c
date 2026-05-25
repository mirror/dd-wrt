/* ISC license. */

#include <stdint.h>

#include <skalibs/uint32.h>
#include <skalibs/surf.h>

void surf_init (SURFSchedule *ctx, char const *s)
{
  SURFSchedule z = SURFSCHEDULE_ZERO ;
  for (uint32_t i = 4 ; i < 12 ; i++) uint32_unpack(s + (i<<2) - 16, z.in + i) ;
  for (uint32_t i = 0 ; i < 32 ; i++) uint32_unpack(s + 32 + (i<<2), z.seed + i) ;
  *ctx = z ;
}
