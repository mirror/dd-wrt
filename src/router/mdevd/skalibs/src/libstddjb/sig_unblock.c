/* ISC license. */

#include <signal.h>

#include <skalibs/sig.h>

void sig_unblock (int sig)
{
  sigset_t ss ;
  sigemptyset(&ss) ;
  sigaddset(&ss, sig) ;
  sigprocmask(SIG_UNBLOCK, &ss, 0) ;
}
