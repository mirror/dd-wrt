/* ISC license. */

#include <skalibs/skaclient.h>
#include <skalibs/unixmessage.h>

int skaclient_sendv (skaclient *a, struct iovec const *v, unsigned int vlen, unixmessage_handler_func *cb, void *result, tain const *deadline, tain *stamp)
{
  unixmessagev m = { .v = (struct iovec *)v, .vlen = vlen, .fds = 0, .nfds = 0 } ;
  return skaclient_sendmsgv(a, &m, cb, result, deadline, stamp) ;
}
