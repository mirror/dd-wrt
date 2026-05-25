/* ISC license. */

#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#endif

#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

static void alrm_handler (int sig)
{
  (void)sig ;
  _exit(0) ;
}

int main (void)
{
  struct sigaction action = { .sa_handler = &alrm_handler, .sa_flags = SA_NOCLDSTOP } ;
  struct timeval tv = { .tv_sec = 100000001, .tv_usec = 0 } ;
  fd_set r, w, x ;
  FD_ZERO(&r) ;
  FD_ZERO(&w) ;
  FD_ZERO(&x) ;
  sigfillset(&action.sa_mask) ;
  if (sigaction(SIGALRM, &action, 0) == -1) _exit(111) ;
  alarm(1) ;
  select(1, &r, &w, &x, &tv) ;
  _exit(errno == EINVAL ? 1 : 111) ;
}
