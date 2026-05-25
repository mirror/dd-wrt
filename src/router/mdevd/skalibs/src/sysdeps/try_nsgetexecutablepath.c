/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <stdint.h>
#include <mach-o/dyld.h>

int main (void)
{
  char buf[1024] ;
  uint32_t len = sizeof(buf) ;
  return _NSGetExecutablePath(buf, &len) >= 0 ;
}
