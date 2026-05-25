/* ISC license. */

#include <skalibs/tai.h>

int tain_add (tain *t, tain const *u, tain const *v)
{
  t->sec.x = u->sec.x + v->sec.x ;
  t->nano = u->nano + v->nano ;
  if (t->nano > 999999999U)
  {
    t->sec.x++ ;
    t->nano -= 1000000000U ;
  }
  return 1 ;
}
