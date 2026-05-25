/* ISC license. */

#include <skalibs/tai.h>

int tain_less (tain const *t, tain const *u)
{
  if (tai_sec(&t->sec) < tai_sec(&u->sec)) return 1 ;
  if (tai_sec(&t->sec) > tai_sec(&u->sec)) return 0 ;
  return t->nano < u->nano ;
}
