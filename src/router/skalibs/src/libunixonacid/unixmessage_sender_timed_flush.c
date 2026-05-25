/* ISC license. */

#include <skalibs/functypes.h>
#include <skalibs/unix-timed.h>
#include <skalibs/unixmessage.h>

static int unixmessage_sender_isnonempty (unixmessage_sender *b)
{
  return !unixmessage_sender_isempty(b) ;
}

int unixmessage_sender_timed_flush (unixmessage_sender *b, tain const *deadline, tain *stamp)
{
  return timed_flush(b, (init_func_ref)&unixmessage_sender_getfd, (init_func_ref)&unixmessage_sender_isnonempty, (init_func_ref)&unixmessage_sender_flush, deadline, stamp) ;
}
