/* ISC license. */

#include <skalibs/djbunix.h>
#include <skalibs/unixmessage.h>

void unixmessage_sender_closecb (int fd, void *p)
{
  fd_close(fd) ;
  (void)p ;
}
