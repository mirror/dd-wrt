/* ISC license. */

/* MT-unsafe */

#include <skalibs/tai.h>

int tain_now_set_wallclock (tain *now)
{
  tain_now = &tain_wallclock_read ;
  return tain_wallclock_read(now) ;
}
