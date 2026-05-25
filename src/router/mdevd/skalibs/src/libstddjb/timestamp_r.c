/* ISC license. */

#include <skalibs/tai.h>

int timestamp_r (char *s, tain *stamp)
{
  if (!tain_wallclock_read(stamp)) return 0 ;
  timestamp_fmt(s, stamp) ;
  return 1 ;
}
