/* ISC license. */

#include <skalibs/bsdsnowflake.h>
#include <errno.h>
#include <skalibs/uint64.h>
#include <skalibs/tai.h>

int tai_u64 (tai *t, uint64_t u)
{
  if (u & ((uint64_t)1 << 63)) return (errno = EOVERFLOW, 0) ;
  t->x = u ;
  return 1 ;
}
