/* ISC license. */

#include <skalibs/skaclient.h>
#include <skalibs/unixmessage.h>

int skaclient_put (skaclient *a, char const *s, size_t len, unixmessage_handler_func *cb, void *result)
{
  unixmessage m = { .s = (char *)s, .len = len, .fds = 0, .nfds = 0 } ;
  return skaclient_putmsg(a, &m, cb, result) ;
}
