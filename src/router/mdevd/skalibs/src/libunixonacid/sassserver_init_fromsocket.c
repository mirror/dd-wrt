/* ISC license. */

#include <skalibs/strerr.h>
#include <skalibs/textclient.h>
#include <skalibs/sassserver.h>
#include "sassserver-internal.h"

void sassserver_init_fromsocket (sassserver *a, char const *banner1, char const *banner2, sassserver_send_func_ref sendf, sassserver_cancel_func_ref cancelf, size_t datasize, free_func_ref cleanupf, void *aux, tain const *deadline, tain *stamp)
{
  if (!textclient_server_01x_init_fromsocket(banner1, strlen(banner1), banner2, strlen(banner2), deadline, stamp))
    strerr_diefu1sys(111, "sync with client") ;
  sassserver_init_structures(a, sendf, cancelf, datasize, cleanupf, aux) ;
}
