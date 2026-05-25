/* ISC license. */

#include <sys/wait.h>

#include <skalibs/djbunix.h>

int waitn_reap_posix (pid_t *pids, unsigned int len, int *w)
{
  pid_t wanted = len ? pids[len-1] : 0 ;
  unsigned int n = 0 ;
  while (len)
  {
    int wstat ;
    int r = wait_pids_nohang(pids, len, &wstat) ;
    if (r < 0) return r ;
    else if (!r) break ;
    if (pids[r-1] == wanted) *w = wstat ;
    pids[r-1] = pids[--len] ;
    n++ ;
  }
  return n ;
}
