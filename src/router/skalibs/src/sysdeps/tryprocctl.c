/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <sys/procctl.h>

int main (void)
{
  procctl(P_PID, 0, PROC_REAP_ACQUIRE, 0) ;
  return 0 ;
}
