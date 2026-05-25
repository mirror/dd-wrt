/* ISC license. */

#include <skalibs/tai.h>

void tain_earliest1 (tain *t, tain const *u)
{
  if (tain_less(u, t)) *t = *u ;
}
