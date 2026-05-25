/* ISC license. */

#include <skalibs/functypes.h>
#include <skalibs/unix-timed.h>
#include <skalibs/textmessage.h>

typedef struct textmessage_handler_blah_s textmessage_handler_blah, *textmessage_handler_blah_ref ;
struct textmessage_handler_blah_s
{
  textmessage_receiver *tr ;
  textmessage_handler_func_ref f ;
  void *p ;
} ;

static int getfd (textmessage_handler_blah *blah)
{
  return textmessage_receiver_fd(blah->tr) ;
}

static ssize_t get (textmessage_handler_blah *blah)
{
  return textmessage_handle(blah->tr, blah->f, blah->p) ;
}

int textmessage_timed_handle (textmessage_receiver *tr, textmessage_handler_func_ref f, void *p, tain const *deadline, tain *stamp)
{
  textmessage_handler_blah blah = { .tr = tr, .f = f, .p = p } ;
  return timed_get(&blah, (init_func_ref)&getfd, (get_func_ref)&get, deadline, stamp) ;
}
