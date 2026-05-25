/* ISC license. */

#include <errno.h>

#include <skalibs/kolbak.h>

int kolbak_unenqueue (kolbak_queue *q)
{
  if (q->head == q->tail) return (errno = EINVAL, 0) ;
  q->tail = (q->tail + q->n - 1) % q->n ;
  return 1 ;
}
