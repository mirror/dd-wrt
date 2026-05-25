/* ISC license. */

/* MT-unsafe */

#include <unistd.h>

#include <skalibs/strerr.h>

void strerr_diev (int e, char const *const *v, unsigned int n)
{
  strerr_warnv(v, n) ;
  _exit(e) ;
}
