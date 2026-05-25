/* ISC license. */

#include <skalibs/tai.h>

void tain_earliestv (tain *t, tain const *const *v, unsigned int n)
{
  for (unsigned int i = 0 ; i < n ; i++)
    if (tain_less(v[i], t)) *t = *v[i] ;
}
