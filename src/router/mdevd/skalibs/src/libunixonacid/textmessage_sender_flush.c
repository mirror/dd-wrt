/* ISC license. */

#include <skalibs/bufalloc.h>
#include <skalibs/textmessage.h>

int textmessage_sender_flush (textmessage_sender *ts)
{
  return bufalloc_flush(&ts->out) ;
}
