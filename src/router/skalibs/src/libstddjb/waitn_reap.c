/* ISC license. */

#include <sys/wait.h>

#include <skalibs/djbunix.h>

int waitn_reap (pid_t *pids, unsigned int len)
{
  int dummy ;
  return waitn_reap_posix(pids, len, &dummy) ;
}
