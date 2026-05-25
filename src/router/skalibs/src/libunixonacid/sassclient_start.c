/* ISC license. */

#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <skalibs/posixplz.h>
#include <skalibs/textclient.h>
#include <skalibs/gensetdyn.h>
#include <skalibs/genqdyn.h>
#include <skalibs/sassclient.h>
#include "sassclient-internal.h"

int sassclient_start (sassclient *a, char const *const *argv, char const *banner1, char const *banner2, tain const *deadline, tain *stamp)
{
  int e ;
  if (sassclient_fd(a) >= 0) return (errno = EBUSY, 0) ;
  e = pthread_mutex_init(&a->connection_mutex, 0) ;
  if (e) goto err ;
  e = pthread_mutex_init(&a->results_mutex, 0) ;
  if (e) goto err0 ;
  if (!textclient_startf(&a->connection, argv, (char const *const *)environ, TEXTCLIENT_OPTION_WAITPID, banner1, strlen(banner1), banner2, strlen(banner2), deadline, stamp))
    goto err1 ;
  gensetdyn_init(&a->store, sizeof(sassclient_data), 8, 3, 8) ;
  genqdyn_init(&a->results, 8, 1, 10) ;
  return 1 ;

 err1:
  e = errno ;
  pthread_mutex_destroy(&a->results_mutex) ;
 err0:
  pthread_mutex_destroy(&a->connection_mutex) ;
 err:
  errno = e ;
  return 0 ;
}
