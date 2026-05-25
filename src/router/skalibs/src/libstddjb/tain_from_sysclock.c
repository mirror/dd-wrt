/* ISC license. */

#include <skalibs/tai.h>

int tain_from_sysclock (tain *a, tain const *u)
{
  if (!tai_from_sysclock(&a->sec, u->sec.x)) return 0 ;
  a->nano = u->nano ;
  return 1 ;
}
