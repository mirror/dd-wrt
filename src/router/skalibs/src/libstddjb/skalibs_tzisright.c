/* ISC license. */

#include <time.h>
#include "djbtime-internal.h"

int skalibs_tzisright ()
{
  static int tzisright = -1 ;
  if (tzisright < 0)
  {
    struct tm tm ;
    time_t t = 78796800 ;
    if (localtime_r(&t, &tm)) tzisright = tm.tm_sec == 60 ;
  }
  return tzisright ;
}
