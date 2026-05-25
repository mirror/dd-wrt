/* ISC license. */

#include <skalibs/cbuffer.h>
#include <skalibs/stralloc.h>
#include <skalibs/unixmessage.h>

int unixmessage_receiver_init (unixmessage_receiver *b, int fd, char *mainbuf, size_t mainlen, char *auxbuf, size_t auxlen)
{
  if (!cbuffer_init(&b->mainb, mainbuf, mainlen)
   || !cbuffer_init(&b->auxb, auxbuf, auxlen)) return 0 ;
  b->fd = fd ;
  b->mainlen = b->auxlen = 0 ;
  b->maindata = stralloc_zero ;
  b->auxdata = stralloc_zero ;
  b->fds_ok = 3 ;
  return 1 ;
}
