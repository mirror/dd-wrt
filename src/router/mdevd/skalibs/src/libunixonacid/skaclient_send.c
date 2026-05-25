/* ISC license. */

#include <skalibs/unixmessage.h>
#include <skalibs/skaclient.h>

int skaclient_send (skaclient *a, char const *s, size_t len, unixmessage_handler_func *cb, void *result, tain const *deadline, tain *stamp)
{
  unixmessage m = { .s = (char *)s, .len = len, .fds = 0, .nfds = 0 } ;
  return skaclient_sendmsg(a, &m, cb, result, deadline, stamp) ;
}
