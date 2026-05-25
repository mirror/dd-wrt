/* ISC license. */

#include <errno.h>
#include <pthread.h>

#include <skalibs/sassclient.h>

void sassclient_release (sassclient *a, uint32_t id)
{
  int e = errno ;
  pthread_mutex_lock(&a->connection_mutex) ;
  gensetdyn_delete(&a->store, id) ;
  pthread_mutex_unlock(&a->connection_mutex) ;
  errno = e ;
}
