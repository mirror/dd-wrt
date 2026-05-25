/* ISC license. */

#include <skalibs/tai.h>

int tain_sub (tain *t, tain const *u, tain const *v)
{
  tain tt ;
  tt.sec.x = u->sec.x - v->sec.x ;
  tt.nano = u->nano - v->nano ;
  if (tt.nano > u->nano)
  {
    tt.sec.x-- ;
    tt.nano += 1000000000U ;
  }
  *t = tt ;
  return 1 ;
}
