/* ISC license. */

#include <sys/uio.h>
#include <string.h>
#include <errno.h>

#include <skalibs/socket.h>
#include <skalibs/djbunix.h>
#include <skalibs/textmessage.h>
#include <skalibs/textclient.h>
#include <skalibs/posixishard.h>

int textclient_start (textclient *a, char const *path, uint32_t options, char const *before, size_t beforelen, char const *after, size_t afterlen, tain const *deadline, tain *stamp)
{
  struct iovec v ;
  int fd = ipc_stream_nbcoe() ;
  if (fd < 0) return 0 ;
  if (!ipc_timed_connect(fd, path, deadline, stamp)) goto err ;
  textmessage_sender_init(&a->syncout, fd) ;
  if (!textmessage_timed_send(&a->syncout, before, beforelen, deadline, stamp)) goto ferr ;
  if (!textmessage_recv_channel(fd, &a->asyncin, a->asyncbuf, TEXTCLIENT_BUFSIZE, after, afterlen, deadline, stamp)) goto ferr ;
  textmessage_receiver_init(&a->syncin, fd, a->syncbuf, TEXTCLIENT_BUFSIZE, TEXTMESSAGE_MAXLEN) ;
  if (!textclient_timed_get(a, &v, deadline, stamp)) goto aerr ;
  if (v.iov_len != afterlen || memcmp(v.iov_base, after, afterlen)) goto berr ;
  a->pid = 0 ;
  a->options = options & ~TEXTCLIENT_OPTION_WAITPID ;
  return 1 ;

 berr:
  errno = EPROTO ;
 aerr:
  textmessage_receiver_free(&a->syncin) ;
  textmessage_receiver_free(&a->asyncin) ;
 ferr:
  textmessage_sender_free(&a->syncout) ;
 err:
  fd_close(fd) ;
  return 0 ;
}
