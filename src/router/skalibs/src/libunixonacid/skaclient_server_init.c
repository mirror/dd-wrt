/* ISC license. */

#include <skalibs/allreadwrite.h>
#include <skalibs/skaclient.h>
#include <skalibs/unixmessage.h>

int skaclient_server_init (unixmessage_receiver *in, unixmessage_sender *out, unixmessage_sender *asyncout, char const *before, size_t beforelen, char const *after, size_t afterlen, tain const *deadline, tain *stamp)
{
  unixmessage m ;
  return (sanitize_read(unixmessage_timed_receive(in, &m, deadline, stamp)) >= 0)
   && skaclient_server_ack(&m, out, asyncout, before, beforelen, after, afterlen)
   && unixmessage_sender_timed_flush(out, deadline, stamp) ;
}
