/* ISC license. */

#include <skalibs/tai.h>

int tain_ulong (tain *t, unsigned long s)
{
  if (!tai_u64(&t->sec, s)) return 0 ;
  t->nano = 0 ;
  return 1 ;
}
