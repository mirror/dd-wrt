/* ISC license. */

#include <sys/wait.h>
#include <skalibs/djbunix.h>

pid_t wait_pid_nohang (pid_t pid, int *wstat)
{
  int w = 0 ;
  pid_t r = 0 ;
  while (r != pid)
  {
    r = wait_nohang(&w) ;
    if (!r || (r == (pid_t)-1)) return r ;
  }
  *wstat = w ;
  return r ;
}
