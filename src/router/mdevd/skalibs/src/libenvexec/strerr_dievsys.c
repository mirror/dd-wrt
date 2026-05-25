/* ISC license. */

/* MT-unsafe */

#include <unistd.h>

#include <skalibs/strerr.h>

void strerr_dievsys (int e, char const *const *v, unsigned int n)
{
  strerr_warnvsys(v, n) ;
  _exit(e) ;
}
