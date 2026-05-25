/* ISC license. */

#include <signal.h>
#include <sys/signalfd.h>

int main (void)
{
  sigset_t foo ;
  sigemptyset(&foo) ;
  if (signalfd(-1, &foo, SFD_NONBLOCK) < 0) return 1 ;
  return 0 ;
}
