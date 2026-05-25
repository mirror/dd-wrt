/* ISC license. */

#include <skalibs/uint64.h>
#include <skalibs/tai.h>

int tain_to_millisecs (tain const *a)
{
  if (a->sec.x > (uint64_t)2147483) return -1 ;
  if ((a->sec.x == (uint64_t)2147483) && (a->nano > 646000000U)) return -1 ;
  return a->sec.x * 1000 + (a->nano + 999999) / 1000000U ;
}
