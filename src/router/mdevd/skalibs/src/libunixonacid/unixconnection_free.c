 /* ISC license. */

#include <skalibs/unixmessage.h>
#include <skalibs/unixconnection.h>

void unixconnection_free (unixconnection *io)
{
  unixmessage_sender_free(&io->out) ;
  unixmessage_receiver_free(&io->in) ;
  *io = unixconnection_zero ;
}
