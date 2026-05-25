/* ISC license. */

#include <sys/uio.h>
#include <skalibs/textmessage.h>

int textmessage_handle (textmessage_receiver *tr, textmessage_handler_func_ref f, void *p)
{
  unsigned int count = 0 ;
  while (count < TEXTMESSAGE_MAXREADS || textmessage_receiver_hasmsginbuf(tr))
  {
    struct iovec v ;
    int r = textmessage_receive(tr, &v) ;
    if (r < 0) return -1 ;
    if (!r) break ;
    r = (*f)(&v, p) ;
    if (r <= 0) return r-2 ;
    count++ ;
  }
  return (int)count ;
}
