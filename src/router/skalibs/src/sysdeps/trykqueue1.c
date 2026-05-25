/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>

int main (void)
{
  int q = kqueue1(O_CLOEXEC) ;
  return 0 ;
}
