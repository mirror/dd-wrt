/* ISC license. */

#include <skalibs/unixmessage.h>

int unixmessage_handle (unixmessage_receiver *b, unixmessage_handler_func_ref f, void *p)
{
  unsigned int count = 0 ;
  while (count < UNIXMESSAGE_MAXREADS || unixmessage_receiver_hasmsginbuf(b))
  {
    unixmessage m ;
    int r = unixmessage_receive(b, &m) ;
    if (r < 0) return -1 ;
    if (!r) break ;
    r = (*f)(&m, p) ;
    if (r <= 0) return r-2 ;
    count++ ;
  }
  return (int)count ;
}
