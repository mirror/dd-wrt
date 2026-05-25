/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/unixmessage.h>

void unixmessage_sender_init_withclosecb (unixmessage_sender *b, int fd, unixmessage_sender_closecb_func_ref f, void *p)
{
  b->fd = fd ;
  b->data = stralloc_zero ;
  b->fds = genalloc_zero ;
  b->offsets = genalloc_zero ;
  b->head = 0 ;
  b->shorty = 0 ;
  b->closecb = f ;
  b->closecbdata = p ;
}
