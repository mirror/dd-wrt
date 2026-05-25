/* ISC license. */

/* MT-unsafe */

#include <errno.h>
#include <signal.h>

#include <skalibs/sig.h>

void sig_restoreto (sigset_t const *set, unsigned int n)
{
  int e = errno ;
  unsigned int i = 1 ;
  for (; i < n ; i++)
  {
    int h = sigismember(set, i) ;
    if (h < 0) continue ;
    if (h) sig_restore(i) ;
  }
  errno = e ;
}
