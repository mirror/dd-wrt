/* ISC license. */

#include <errno.h>
#include <signal.h>

#include <skalibs/sig.h>

void sig_blocknone (void)
{
  int e = errno ;
  sigset_t ss ;
  sigemptyset(&ss) ;
  sigprocmask(SIG_SETMASK, &ss, 0) ;
  errno = e ;
}
