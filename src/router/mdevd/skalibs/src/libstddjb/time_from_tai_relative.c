 /* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/bsdsnowflake.h>
#include <time.h>
#include <errno.h>
#include <skalibs/tai.h>

int time_from_tai_relative (time_t *u, tai const *t)
{
#if SKALIBS_SIZEOFTIME < 8
  if ((t->x >> 32) && (t->x >> 32) != 0xffffffffUL)
    return (errno = EOVERFLOW, 0) ;
#endif
  *u = (time_t)t->x ;
  return 1 ;
}
