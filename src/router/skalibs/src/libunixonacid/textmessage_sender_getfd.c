/* ISC license. */

#include <skalibs/bufalloc.h>
#include <skalibs/textmessage.h>

int textmessage_sender_getfd (textmessage_sender const *ts)
{
  return bufalloc_fd(&ts->out) ;
}
