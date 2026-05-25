/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/textmessage.h>

void textmessage_receiver_free (textmessage_receiver *ts)
{
  stralloc_free(&ts->indata) ;
  *ts = textmessage_receiver_zero ;
}
