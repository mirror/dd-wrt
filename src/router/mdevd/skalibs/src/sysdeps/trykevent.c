/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>

int main (void)
{
  int q = kqueue() ;
  struct kevent ke ;
  struct timespec ts = { .tv_sec = 1, .tv_nsec = 0 } ;
  EV_SET(&ke, getpid(), EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, 0) ;
  kevent(q, &ke, 1, &ke, 1, &ts) ;
  return 0 ;
}
