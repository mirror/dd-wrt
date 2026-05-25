/* ISC license. */

#include <sys/wait.h>
#include <skalibs/djbunix.h>

int wait_pids_nohang (pid_t const *pids, unsigned int len, int *wstat)
{
  for (;;)
  {
    int w ;
    pid_t r = wait_nohang(&w) ;
    if (!r || (r == (pid_t)-1)) return (int)r ;
    {
      unsigned int i = 0 ;
      for (; i < len ; i++) if (r == pids[i]) break ;
      if (i < len)
      {
        *wstat = w ;
        return 1+i ;
      }
    }
  }
}
