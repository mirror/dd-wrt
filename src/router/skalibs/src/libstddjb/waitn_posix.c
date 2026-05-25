/* ISC license. */

#include <sys/wait.h>

#include <skalibs/djbunix.h>

int waitn_posix (pid_t *pids, unsigned int n, int *w)
{
  pid_t wanted = n ? pids[n-1] : 0 ;
  while (n)
  {
    int wstat ;
    unsigned int i = 0 ;
    pid_t pid = wait_nointr(&wstat) ;
    if (pid < 0) return 0 ;
    for (; i < n ; i++) if (pid == pids[i]) break ;
    if (i < n) pids[i] = pids[--n] ;
    if (pid == wanted) *w = wstat ;
  }
  return 1 ;
}
