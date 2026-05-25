/* ISC license. */

/* MT-unsafe */

#include <skalibs/sig.h>

static void sig_nop (int sig)
{
  (void)sig ;
}

int sig_altignore (int sig)
{
  return sig_catch(sig, &sig_nop) ;
}
