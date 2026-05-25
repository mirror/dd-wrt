/* ISC license. */

#include <skalibs/skaclient.h>

int skaclient_sendmsg_and_close (skaclient *a, unixmessage const *m, unsigned char const *bits, unixmessage_handler_func_ref cb, void *result, tain const *deadline, tain *stamp)
{
  return skaclient_putmsg_and_close(a, m, bits, cb, result)
   && skaclient_syncify(a, deadline, stamp) ;
}
