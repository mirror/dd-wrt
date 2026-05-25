/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <sys/prctl.h>

int main (void)
{
  prctl(PR_SET_CHILD_SUBREAPER, 1) ;
  return 0 ;
}
