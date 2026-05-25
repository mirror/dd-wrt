/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/bsdsnowflake.h>
#include <time.h>
#include <errno.h>
#include <skalibs/uint64.h>
#include <skalibs/tai.h>

int time_sysclock_from_tai (time_t *u, tai const *t)
{
  uint64_t uu ;
  if (!sysclock_from_tai(&uu, t)) return 0 ;
  uu -= TAI_MAGIC ;
#if SKALIBS_SIZEOFTIME < 8
  if ((uu >> 32) && (uu >> 32) != 0xffffffffUL)
    return (errno = EOVERFLOW, 0) ;
#endif
  *u = (time_t)uu ;
  return 1 ;
}
