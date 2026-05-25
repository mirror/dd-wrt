/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <sys/auxv.h>

int main (void)
{
  unsigned long x = getauxval(AT_EXECFN) ;
  return 0 ;
}
