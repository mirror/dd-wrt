/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE
#endif

#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif

#include <sys/time.h>

int main (void)
{
  struct timeval tv ;
  gettimeofday(&tv, 0) ;
  settimeofday(&tv, 0) ;
  return 0 ;
}
