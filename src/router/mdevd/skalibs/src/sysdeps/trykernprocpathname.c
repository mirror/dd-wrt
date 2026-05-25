/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <sys/types.h>
#ifdef __NetBSD__
#include <sys/param.h>
#endif
#include <sys/sysctl.h>

int main (void)
{
  int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 } ;
  char buf[1024] ;
  size_t len = sizeof(buf) ;
  return sysctl(mib, 4, buf, &len, 0, 0) >= 0 ;
}
