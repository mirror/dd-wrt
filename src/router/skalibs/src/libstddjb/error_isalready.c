/* ISC license. */

#include <skalibs/bsdsnowflake.h>
#include <errno.h>
#include <skalibs/error.h>

int error_isalready (int e)
{
  return e == EALREADY || e == EINPROGRESS
#ifdef SKALIBS_BSD_SUCKS
   || e == EADDRINUSE
#endif
  ;
}
