/* ISC license. */

#include <skalibs/unixmessage.h>

int unixmessage_sender_getfd (unixmessage_sender const *b)
{
  return b->fd ;
}
