/* ISC license. */

#include <sys/wait.h>
#include <errno.h>

#include <skalibs/djbunix.h>

pid_t wait_nointr (int *wstat)
{
  int e = errno ;
  pid_t r ;
  do r = wait(wstat) ;
  while (r == (pid_t)-1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}
