/* ISC license. */

#include <errno.h>
#include <string.h>
#include <sys/uio.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/textmessage.h>
#include <skalibs/textclient.h>
#include <skalibs/posixishard.h>

int textclient_server_init_fromsocket (textmessage_receiver *in, textmessage_sender *syncout, textmessage_sender *asyncout, char const *before, size_t beforelen, char const *after, size_t afterlen, tain const *deadline, tain *stamp)
{
  struct iovec v ;
  if (sanitize_read(textmessage_timed_receive(in, &v, deadline, stamp)) <= 0) return 0 ;
  if (v.iov_len != beforelen || memcmp(v.iov_base, before, beforelen)) return (errno = EPROTO, 0) ;
  if (!textmessage_create_send_channel(textmessage_sender_fd(syncout), asyncout, after, afterlen, deadline, stamp)) return 0 ;
  if (!textmessage_timed_send(syncout, after, afterlen, deadline, stamp)) return 0 ;
  return 1 ;
}
