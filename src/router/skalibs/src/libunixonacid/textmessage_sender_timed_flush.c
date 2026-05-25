/* ISC license. */

#include <skalibs/functypes.h>
#include <skalibs/unix-timed.h>
#include <skalibs/textmessage.h>

static int textmessage_sender_isnonempty (textmessage_sender *ts)
{
  return !textmessage_sender_isempty(ts) ;
}

int textmessage_sender_timed_flush (textmessage_sender *ts, tain const *deadline, tain *stamp)
{
  return timed_flush(ts, (init_func_ref)&textmessage_sender_getfd, (init_func_ref)&textmessage_sender_isnonempty, (init_func_ref)&textmessage_sender_flush, deadline, stamp) ;
}
