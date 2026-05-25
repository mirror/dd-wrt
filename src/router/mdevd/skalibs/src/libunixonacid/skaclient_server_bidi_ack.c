/* ISC license. */

#include <skalibs/skaclient.h>
#include <skalibs/unixmessage.h>

int skaclient_server_bidi_ack (unixmessage const *clientmsg, unixmessage_sender *out, unixmessage_sender *asyncout, unixmessage_receiver *asyncin, char *mainbuf, size_t mainlen, char *auxbuf, size_t auxlen, char const *before, size_t beforelen, char const *after, size_t afterlen)
{
  if (!unixmessage_receiver_init(asyncin, -1, mainbuf, mainlen, auxbuf, auxlen)) return 0 ;
  if (!skaclient_server_ack(clientmsg, out, asyncout, before, beforelen, after, afterlen)) return 0 ;
  asyncin->fd = unixmessage_sender_fd(asyncout) ;
  return 1 ;
}
