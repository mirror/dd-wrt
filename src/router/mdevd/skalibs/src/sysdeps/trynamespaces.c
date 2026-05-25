/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE
#endif
#ifndef _INCOMPLETE_XOPEN_C063
#define _INCOMPLETE_XOPEN_C063
#endif

#include <sched.h>

int main (void)
{
  return unshare(0) ;
}
