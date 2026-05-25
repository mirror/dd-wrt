/* ISC license. */

#include <pthread.h>

#include <skalibs/textclient.h>
#include <skalibs/gensetdyn.h>
#include <skalibs/genqdyn.h>
#include <skalibs/sassclient.h>

void sassclient_end (sassclient *a)
{
  static sassclient const zero = SASSCLIENT_ZERO ;
  if (textclient_fd(&a->connection) == -1) return ;
  textclient_end(&a->connection) ;
  gensetdyn_free(&a->store) ;
  genqdyn_free(&a->results) ;
  pthread_mutex_destroy(&a->connection_mutex) ;
  pthread_mutex_destroy(&a->results_mutex) ;
  *a = zero ;
}
