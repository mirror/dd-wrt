/* ISC license. */

#include <errno.h>

#include <skalibs/tai.h>
#include <skalibs/iopause.h>

int iopause_stamp (iopause_fd *x, unsigned int n, tain const *deadline, tain *stamp)
{
  int e = errno ;
  int r ;
  do r = iopause(x, n, deadline, stamp) ;
  while (r == -1 && errno == EINTR) ;
  if (stamp) tain_now(stamp) ;
  if (r >= 0) errno = e ;
  return r ;
}
