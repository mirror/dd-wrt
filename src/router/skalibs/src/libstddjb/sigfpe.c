/* ISC license. */

#include <signal.h>
#include <skalibs/segfault.h>

int sigfpe (void)
{
  return raise(SIGFPE) == 0 ;
}
