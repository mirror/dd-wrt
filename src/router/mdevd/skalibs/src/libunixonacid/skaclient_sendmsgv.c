/* ISC license. */

#include <skalibs/skaclient.h>

int skaclient_sendmsgv_and_close (skaclient *a, unixmessagev const *m, unsigned char const *bits, unixmessage_handler_func_ref cb, void *result, tain const *deadline, tain *stamp)
{
  return skaclient_putmsgv_and_close(a, m, bits, cb, result)
    && skaclient_syncify(a, deadline, stamp) ;
}
