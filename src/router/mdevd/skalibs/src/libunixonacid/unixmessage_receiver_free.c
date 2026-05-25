/* ISC license. */

#include <sys/types.h>
#include <skalibs/cbuffer.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>
#include <skalibs/unixmessage.h>

void unixmessage_receiver_free (unixmessage_receiver *b)
{
  size_t maindatalen = b->maindata.len ;
  int h ;
  b->fd = -1 ;
  stralloc_free(&b->maindata) ;
  h = maindatalen != b->mainlen || b->auxdata.len != b->auxlen || cbuffer_len(&b->auxb) ;
  if (h)
  {
    size_t n = b->auxdata.len / sizeof(int) ;
    while (n--) fd_close(((int *)b->auxdata.s)[n]) ;
  }
  stralloc_free(&b->auxdata) ;
  if (h)
  {
    size_t n = cbuffer_len(&b->auxb) / sizeof(int) ;
    if (n)
    {
      int fds[n] ;
      cbuffer_get(&b->auxb, (char *)fds, n * sizeof(int)) ;
      while (n--) fd_close(fds[n]) ;
    }
  }
}
