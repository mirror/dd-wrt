/* ISC license. */

#include <sys/types.h>

#include <skalibs/functypes.h>
#include <skalibs/unix-timed.h>
#include <skalibs/unixmessage.h>

typedef struct unixmessage_getter_s unixmessage_getter, *unixmessage_getter_ref ;
struct unixmessage_getter_s
{
  unixmessage_receiver *b ;
  unixmessage *m ;
} ;

static int getfd (unixmessage_getter *g)
{
  return unixmessage_receiver_fd(g->b) ;
}

static ssize_t get (unixmessage_getter *g)
{
  return unixmessage_receive(g->b, g->m) ;
}

int unixmessage_timed_receive (unixmessage_receiver *b, unixmessage *m, tain const *deadline, tain *stamp)
{
  unixmessage_getter g = { .b = b, .m = m } ;
  return timed_get(&g, (init_func_ref)&getfd, (get_func_ref)&get, deadline, stamp) ;
}
