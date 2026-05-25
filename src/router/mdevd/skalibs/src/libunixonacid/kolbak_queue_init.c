/* ISC license. */

#include <errno.h>

#include <skalibs/kolbak.h>

int kolbak_queue_init (kolbak_queue *q, kolbak_closure *s, size_t len)
{
  if (len < 2) return (errno = EINVAL, 0) ;
  q->x = s ;
  q->n = len ;
  q->head = 0 ;
  q->tail = 0 ;
  return 1 ;
}
