/* ISC license. */

#include <skalibs/functypes.h>
#include <skalibs/unix-timed.h>
#include <skalibs/unixmessage.h>

typedef struct unixmessage_handler_blah_s unixmessage_handler_blah, *unixmessage_handler_blah_ref ;
struct unixmessage_handler_blah_s
{
  unixmessage_receiver *b ;
  unixmessage_handler_func_ref f ;
  void *p ;
} ;

static int getfd (unixmessage_handler_blah *blah)
{
  return unixmessage_receiver_fd(blah->b) ;
}

static ssize_t get (unixmessage_handler_blah *blah)
{
  return unixmessage_handle(blah->b, blah->f, blah->p) ;
}

int unixmessage_timed_handle (unixmessage_receiver *b, unixmessage_handler_func_ref f, void *p, tain const *deadline, tain *stamp)
{
  unixmessage_handler_blah blah = { .b = b, .f = f, .p = p } ;
  return timed_get(&blah, (init_func_ref)&getfd, (get_func_ref)&get, deadline, stamp) ;
}
