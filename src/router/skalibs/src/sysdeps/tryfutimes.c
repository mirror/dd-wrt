/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <sys/time.h>

int main (void)
{
  struct timeval foo[2] = { { .tv_sec = 0, .tv_usec = 0 }, { .tv_sec = 0, .tv_usec = 0 } } ;
  futimes(0, foo) ;
  return 0 ;
}
