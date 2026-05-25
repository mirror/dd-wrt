/* ISC license. */

#include <signal.h>
#include <skalibs/segfault.h>

int sigsegv (void)
{
  return raise(SIGSEGV) == 0 ;
}
