/* ISC license. */

#include <errno.h>
#include <skalibs/tai.h>

int tain_from_millisecs (tain *a, int millisecs)
{
  if (millisecs < 0) return (errno = EINVAL, 0) ;
  a->sec.x = millisecs / 1000 ;
  a->nano = (millisecs % 1000) * 1000000U ;
  return 1 ;
}
