/* ISC license. */

#include <skalibs/tai.h>

double tain_approx (tain const *t)
{
  return tai_approx(&t->sec) + tain_frac(t) ;
}
