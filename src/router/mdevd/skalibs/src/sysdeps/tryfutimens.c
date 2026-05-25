/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/stat.h>

int main (void)
{
  struct timespec foo[2] = { { .tv_sec = 0, .tv_nsec = 0 }, { .tv_sec = 0, .tv_nsec = 0 } } ;
  futimens(0, foo) ;
  return 0 ;
}
