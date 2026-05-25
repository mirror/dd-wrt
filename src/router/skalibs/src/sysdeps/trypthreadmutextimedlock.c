/* ISC license. */

#include <time.h>
#include <pthread.h>

int main (void)
{
  pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER ;
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 1 } ;
  return pthread_mutex_timedlock(&mtx, &ts) ;
}
