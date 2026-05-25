/* ISC license. */

#include <errno.h>

#include <skalibs/unixmessage.h>
#include <skalibs/kolbak.h>

int kolbak_call (unixmessage const *m, kolbak_queue *q)
{
  if (q->head == q->tail) return (errno = EILSEQ, 0) ;
  if (!(*q->x[q->head].f)(m, q->x[q->head].data)) return 0 ;
  q->head = (q->head + 1) % q->n ;
  return 1 ;
}
