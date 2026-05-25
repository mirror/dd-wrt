/* ISC license. */

#include <skalibs/tai.h>

void tain_half (tain *t, tain const *u)
{
  t->nano = u->nano >> 1 ;
  if (u->sec.x & 1) t->nano += 500000000U ;
  t->sec.x = u->sec.x >> 1 ;
}
