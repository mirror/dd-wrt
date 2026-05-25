/* ISC license. */

#include <sys/wait.h>

#include <skalibs/djbunix.h>

int waitn (pid_t *pids, unsigned int n)
{
  int dummy ;
  return waitn_posix(pids, n, &dummy) ;
}
