/* ISC license. */

#include <errno.h>
#include <string.h>

#include <skalibs/djbunix.h>
#include <skalibs/unix-timed.h>
#include <skalibs/ancil.h>
#include <skalibs/textmessage.h>
#include <skalibs/textclient.h>
#include <skalibs/posixishard.h>

static int getfd (void *p)
{
  return ((int *)p)[0] ;
}

static int one (void *p)
{
  (void)p ;
  return 1 ;
}

static int sendit (void *p)
{
  int *fd = p ;
  return ancil_send_fd(fd[0], fd[1], '|') ;
}

int textmessage_create_send_channel (int sock, textmessage_sender *asyncout, char const *after, size_t afterlen, tain const *deadline, tain *stamp)
{
  int fd[3] = { sock } ;
  int r ;
  if (pipenbcoe(fd+1) < 0) return 0 ;
  r = timed_flush(fd, &getfd, &one, &sendit, deadline, stamp) ;
  fd_close(fd[1]) ;
  if (!r) goto err ;
  textmessage_sender_init(asyncout, fd[2]) ;
  if (!textmessage_timed_send(asyncout, after, afterlen, deadline, stamp)) goto ferr ;
  return 1 ;

 ferr:
  textmessage_sender_free(asyncout) ;
 err:
  fd_close(fd[2]) ;
  return 0 ;
}
