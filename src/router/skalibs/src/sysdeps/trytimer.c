/* ISC license. */

#include <signal.h>
#include <time.h>

int main (void)
{
  timer_t blah ;
  struct itimerspec it = { .it_interval = { .tv_sec = 0, .tv_nsec = 0 }, .it_value = { .tv_sec = 1, .tv_nsec = 0 } } ;
  struct sigevent se = { .sigev_notify = SIGEV_SIGNAL, .sigev_signo = SIGALRM, .sigev_value = { .sival_int = 0 }, .sigev_notify_function = 0, .sigev_notify_attributes = 0 } ;
  if (timer_create(CLOCK_REALTIME, &se, &blah) < 0) return 111 ;
  if (timer_settime(blah, TIMER_ABSTIME, &it, 0) < 0) return 111 ;
  return 0 ;
}
