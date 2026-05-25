/* ISC license. */

#include <skalibs/djbtime.h>

int sysclock_from_localtmn (tain *a, localtmn const *l)
{
  if (!sysclock_from_localtm(&a->sec.x, &l->tm)) return 0 ;
  a->nano = l->nano ;
  return 1 ;
}
