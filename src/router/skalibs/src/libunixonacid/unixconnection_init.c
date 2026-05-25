 /* ISC license. */

#include <skalibs/unixmessage.h>
#include <skalibs/unixconnection.h>

void unixconnection_init (unixconnection *io, int fdin, int fdout)
{
  unixconnection_init_withclosecb(io, fdin, fdout, &unixmessage_sender_closecb, 0) ;
}
