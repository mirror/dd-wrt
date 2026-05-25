/* ISC license. */

#include <errno.h>

#include <skalibs/djbunix.h>

unsigned int wait_reap ()
{
  int e = errno ;
  unsigned int n = 0 ;
  int wstat ;
  while (wait_nohang(&wstat) > 0) n++ ;
  errno = e ;
  return n ;
}
