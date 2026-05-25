/* ISC license. */

#include <errno.h>

#include <skalibs/djbunix.h>
#include <skalibs/textmessage.h>
#include <skalibs/textclient.h>

void textclient_end (textclient *a)
{
  fd_close(textmessage_sender_fd(&a->syncout)) ;
  if (textmessage_receiver_fd(&a->syncin) != textmessage_sender_fd(&a->syncout))
    fd_close(textmessage_receiver_fd(&a->syncin)) ;
  fd_close(textmessage_receiver_fd(&a->asyncin)) ;
  textmessage_sender_free(&a->syncout) ;
  textmessage_receiver_free(&a->syncin) ;
  textmessage_receiver_free(&a->asyncin) ;
  if (a->pid && a->options & TEXTCLIENT_OPTION_WAITPID)
  {
    int e = errno ;
    int wstat ;
    waitpid_nointr(a->pid, &wstat, 0) ;
    errno = e ;
  }
  *a = textclient_zero ;
}
