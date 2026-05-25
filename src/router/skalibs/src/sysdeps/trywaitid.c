/* ISC license. */

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

int main (void)
{
  siginfo_t info ;
  waitid(P_PID, 0, &info, WEXITED) ;
  return 0 ;
}
