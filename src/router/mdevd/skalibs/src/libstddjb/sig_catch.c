/* ISC license. */

/* MT-unsafe */

#include <skalibs/nonposix.h>

#include <signal.h>
#include <errno.h>

#include <skalibs/functypes.h>
#include <skalibs/sig.h>
#include <skalibs/nsig.h>

 /*
   We don't want to fail on non-catchable signals,
   even if sigaction() does.
 */

int sig_catch (int sig, sig_func_ref f)
{
  struct sigaction action = { .sa_handler = f, .sa_flags = SA_RESTART | SA_NOCLDSTOP } ;
  sigfillset(&action.sa_mask) ;
  return sigaction(sig, &action, 0) >= 0
   || (errno == EINVAL && sig >= 1 && sig < SKALIBS_NSIG) ;
}
