/* ISC license. */

#include <skalibs/sysdeps.h>

#if defined(SKALIBS_PROCSELFEXE)  /* Linux, midipix, Solaris */

#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sagetexecname (stralloc *sa)
{
  return sareadlink(sa, SKALIBS_PROCSELFEXE) ;
}

#elif defined(SKALIBS_HASKERNPROCPATHNAME)  /* FreeBSD, NetBSD */

#include <skalibs/nonposix.h>
#ifdef __NetBSD__
#include <sys/param.h>
#endif
#include <limits.h>
#include <sys/sysctl.h>

#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sagetexecname (stralloc *sa)
{
  int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 } ;
  char buf[PATH_MAX] ;
  size_t len = sizeof(buf) ;
  if (sysctl(mib, 4, buf, &len, 0, 0) == -1) return -1 ;
  if (!stralloc_catb(sa, buf, len) || !stralloc_0(sa)) return -1 ;
  return 0 ;
}

#elif defined(SKALIBS_HAS_NSGETEXECUTABLEPATH)  /* MacOS */

#include <skalibs/nonposix.h>
#include <sys/types.h>
#include <stdint.h>
#include <mach-o/dyld.h>

#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sagetexecname (stralloc *sa)
{
  char buf[4096] ;
  uint32_t len = sizeof(buf) ;
  if (_NSGetExecutablePath(buf, &len) == -1) return -1 ;
  return sarealpath(sa, buf) ;
}

#elif defined(SKALIBS_HASGETAUXVAL)  /* Hurd */

#include <skalibs/nonposix.h>
#include <sys/auxv.h>

#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sagetexecname (stralloc *sa)
{
  unsigned long x = getauxval(AT_EXECFN) ;
  return sarealpath(sa, (char const *)x) ;
}

#elif defined(SKALIBS_HASDLADDR)  /* hack that only works with dynamic binaries */

#include <skalibs/nonposix.h>
#include <dlfcn.h>

#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

extern int main () ;
int sagetexecname (stralloc *sa)
{
  Dl_info info ;
  if (!dladdr(&main, &info)) return -1 ;
  if (!stralloc_cats(sa, info.dli_fname) || !stralloc_0(sa)) return -1 ;
  return 0 ;
}

#elif defined(SKALIBS_HASGETEXECNAME)  /* bad: it will probably give argv[0] */

#include <skalibs/nonposix.h>
#include <stdlib.h>

#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sagetexecname (stralloc *sa)
{
  return sarealpath(sa, getexecname()) ;
}

#else  /* we tried */

#include <errno.h>

#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sagetexecname (stralloc *sa)
{
  errno = ENOSYS ;
  return -1 ;
}

#endif
