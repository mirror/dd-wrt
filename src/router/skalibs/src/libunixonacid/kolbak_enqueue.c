/* ISC license. */

#include <sys/types.h>
#include <errno.h>

#include <skalibs/kolbak.h>
#include <skalibs/unixmessage.h>

int kolbak_enqueue (kolbak_queue *q, unixmessage_handler_func_ref f, void *data)
{
  size_t newtail = (q->tail + 1) % q->n ;
  if (newtail == q->head) return (errno = ENOBUFS, 0) ;
  q->x[q->tail].f = f ;
  q->x[q->tail].data = data ;
  q->tail = newtail ;
  return 1 ;
}
