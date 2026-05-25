/* ISC license. */

#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include <skalibs/uint32.h>
#include <skalibs/pthread.h>
#include <skalibs/textclient.h>
#include <skalibs/gensetdyn.h>
#include <skalibs/sassclient.h>
#include "sassclient-internal.h"

int sassclient_cancel (sassclient *a, uint32_t id, tain const *deadline, tain *stamp)
{
  int e = pthread_mutex_tailock(&a->connection_mutex, deadline, stamp) ;
  if (e) return (errno = e, 0) ;
  e = sassclient_cancel_internal(a, id, deadline, stamp) ;
  if (e) goto err ;
  gensetdyn_delete(&a->store, id) ;
  pthread_mutex_unlock(&a->connection_mutex) ;
  return 1 ;

 err:
  pthread_mutex_unlock(&a->connection_mutex) ;
  errno = e ;
  return 0 ;
}
