/* ISC license. */

#include <signal.h>

#include <skalibs/sig.h>

void sig_block (int sig)
{
  sigset_t ss ;
  sigemptyset(&ss) ;
  sigaddset(&ss, sig) ;
  sigprocmask(SIG_BLOCK, &ss, 0) ;
}
