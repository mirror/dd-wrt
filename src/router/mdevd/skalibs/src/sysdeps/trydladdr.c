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
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE
#endif
#ifndef _INCOMPLETE_XOPEN_C063
#define _INCOMPLETE_XOPEN_C063
#endif

#include <dlfcn.h>

int main (void)
{
  Dl_info info ;
  if (!dladdr(&main, &info)) return 1 ;
  if (!info.dli_fname) return 1 ;
  return 0 ;
}
