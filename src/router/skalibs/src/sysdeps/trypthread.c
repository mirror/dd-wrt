/* ISC license. */

#include <time.h>
#include <pthread.h>

void *pstart (void *arg)
{
  (void)arg ;
  return 0 ;
}

int main (void)
{
  pthread_t th ;
  pthread_attr_t attr ;
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER ;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER ;
  struct timespec ts = { .tv_sec = 1, .tv_nsec = 0 } ;
  void *p ;
  int e = pthread_attr_init(&attr) ;
  e = pthread_create(&th, &attr, &pstart, 0) ;
  e = pthread_cond_timedwait(&cond, &mutex, &ts) ;
  e = pthread_join(th, &p) ;
  return 0 ;
}
