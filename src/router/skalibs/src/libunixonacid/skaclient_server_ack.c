/* ISC license. */

#include <errno.h>
#include <string.h>

#include <skalibs/djbunix.h>
#include <skalibs/skaclient.h>
#include <skalibs/unixmessage.h>
#include <skalibs/socket.h>
#include <skalibs/posixishard.h>

int skaclient_server_ack (unixmessage const *clientmsg, unixmessage_sender *out, unixmessage_sender *asyncout, char const *before, size_t beforelen, char const *after, size_t afterlen)
{
  int fd[2] ;
  unixmessage m = { .s = (char *)after, .len = afterlen, .fds = fd, .nfds = 1 } ;
  static unsigned char const bits = 0xff ;
  if (clientmsg->nfds
   || clientmsg->len != beforelen
   || memcmp(clientmsg->s, before, beforelen)) return (errno = EPROTO, 0) ;
  if (ipc_pair_nbcoe(fd) < 0) return 0 ;
  unixmessage_sender_init(asyncout, fd[1]) ;
  if (!unixmessage_put_and_close(out, &m, &bits))
  {
    fd_close(fd[1]) ;
    fd_close(fd[0]) ;
    return 0 ;
  }
  return 1 ;
}
