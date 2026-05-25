/* ISC license. */

#include <sys/uio.h>
#include <skalibs/cbuffer.h>

void cbuffer_rpeek (cbuffer const *b, struct iovec *v)
{
  v[0].iov_base = b->x + b->p ;
  if (b->n >= b->p)
  {
    v[0].iov_len = b->n - b->p ;
    v[1].iov_base = 0 ;
    v[1].iov_len = 0 ;
  }
  else
  {
    v[0].iov_len = b->a - b->p ;
    v[1].iov_base = b->x ;
    v[1].iov_len = b->n ;
  }
}
