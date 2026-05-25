/* ISC license. */

#include <sys/time.h>

int main (void)
{
  struct itimerval blah ;
  if (getitimer(ITIMER_REAL, &blah) < 0) return 111 ;
  return 0 ;
}
