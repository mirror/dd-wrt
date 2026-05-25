/* ISC license. */

/* MT-unsafe */

#include <skalibs/tai.h>

int tain_setnow (tain const *a)
{
  tain aa ;
  if (!sysclock_from_tai(&aa.sec.x, &a->sec)) return 0 ;
  aa.nano = a->nano ;
  return sysclock_set(&aa) ;
}
