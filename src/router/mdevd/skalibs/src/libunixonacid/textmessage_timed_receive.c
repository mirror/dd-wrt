/* ISC license. */

#include <sys/uio.h>

#include <skalibs/functypes.h>
#include <skalibs/unix-timed.h>
#include <skalibs/textmessage.h>

typedef struct textmessage_getter_s textmessage_getter, *textmessage_getter_ref ;
struct textmessage_getter_s
{
  textmessage_receiver *tr ;
  struct iovec *v ;
} ;

static int getfd (textmessage_getter *g)
{
  return textmessage_receiver_fd(g->tr) ;
}

static ssize_t get (textmessage_getter *g)
{
  return textmessage_receive(g->tr, g->v) ;
}

int textmessage_timed_receive (textmessage_receiver *tr, struct iovec *v, tain const *deadline, tain *stamp)
{
  textmessage_getter g = { .tr = tr, .v = v } ;
  return timed_get(&g, (init_func_ref)&getfd, (get_func_ref)&get, deadline, stamp) ;
}
