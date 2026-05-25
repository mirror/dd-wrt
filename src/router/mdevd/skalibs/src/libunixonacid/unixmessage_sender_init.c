/* ISC license. */

#include <skalibs/unixmessage.h>

void unixmessage_sender_init (unixmessage_sender *b, int fd)
{
  unixmessage_sender_init_withclosecb(b, fd, &unixmessage_sender_closecb, 0) ;
}
