/* ISC license. */

#include <sys/uio.h>
#include <string.h>
#include <errno.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/error.h>
#include <skalibs/djbunix.h>
#include <skalibs/unix-timed.h>
#include <skalibs/ancil.h>
#include <skalibs/textmessage.h>
#include <skalibs/posixishard.h>

static int getfd (void *p)
{
  return ((int *)p)[0] ;
}

static ssize_t get (void *p)
{
  int *fd = p ;
  int r = ancil_recv_fd(fd[0], '|') ;
  if (r < 0) return error_isagain(errno) ? (errno = 0, 0) : r ;
  fd[1] = r ;
  return 1 ;
}
  
int textmessage_recv_channel (int sock, textmessage_receiver *asyncin, char *asyncbuf, size_t asyncbufsize, char const *after, size_t afterlen, tain const *deadline, tain *stamp)
{
  struct iovec v ;
  int fd[2] = { sock, -1 } ;
  ssize_t r = timed_get(fd, &getfd, &get, deadline, stamp) ;
  if (!r) errno = EPIPE ;
  if (r <= 0) return 0 ;
  textmessage_receiver_init(asyncin, fd[1], asyncbuf, asyncbufsize, TEXTMESSAGE_MAXLEN) ;
  if (sanitize_read(textmessage_timed_receive(asyncin, &v, deadline, stamp)) <= 0) goto serr ;
  if (v.iov_len != afterlen || memcmp(v.iov_base, after, afterlen)) goto berr ;
  return 1 ;

 berr:
  errno = EPROTO ;
 serr:
  textmessage_receiver_free(asyncin) ;
  fd_close(fd[1]) ;
  return 0 ;
}
