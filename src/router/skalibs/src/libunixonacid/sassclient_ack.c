/* ISC license. */

#include <errno.h>
#include <pthread.h>

#include <skalibs/uint32.h>
#include <skalibs/genqdyn.h>
#include <skalibs/sassclient.h>

int sassclient_ack (sassclient *a, uint32_t *id, int *status)
{
  uint32_t x ;
  char const *s ;
  int e = pthread_mutex_lock(&a->results_mutex) ;
  if (e) return (errno = e, -1) ;
  if (!genqdyn_n(&a->results)) { pthread_mutex_unlock(&a->results_mutex) ; return 0 ; }
  s = GENQDYN_PEEK(char const, &a->results) ;
  uint32_unpack_big(s, id) ;
  uint32_unpack_big(s + 4, &x) ;
  genqdyn_pop(&a->results) ;
  pthread_mutex_unlock(&a->results_mutex) ;
  *status = x ;
  return 1 ;
}
