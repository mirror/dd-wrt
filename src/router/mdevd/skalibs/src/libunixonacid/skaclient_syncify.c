/* ISC license. */

#include <errno.h>

#include <skalibs/skaclient.h>

int skaclient_syncify (skaclient *a, tain const *deadline, tain *stamp)
{
  int r ;
  if (!skaclient_timed_flush(a, deadline, stamp)) return 0 ;
  r = skaclient_timed_supdate(a, deadline, stamp) ;
  return r < 0 ? 0 : !r ? (errno = EPIPE, 0) : 1 ;
}
