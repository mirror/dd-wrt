/* ISC license. */

#include <skalibs/tai.h>

int tai_add (tai *t, tai const *u, tai const *v)
{
  t->x = u->x + v->x ;
  return 1 ;
}
