/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE
#endif

#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif

#include <sys/types.h>
#include <unistd.h>
#include <grp.h>

int main (void)
{
  gid_t g = 0 ;
  setgroups(1, &g) ;
  return 0 ;
}
