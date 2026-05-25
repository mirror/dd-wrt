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

#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>

int main (void)
{
  struct pollfd x = { .events = POLLIN } ;
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 10 } ;
  x.fd = open("src/sysdeps/tryppoll.c", O_RDONLY);
  if (x.fd < 0) return 111 ;
  if (ppoll(&x, 1, &ts, 0) < 0) return 1 ;
  if (x.revents != POLLIN) return 1 ;
  return 0 ;
}
