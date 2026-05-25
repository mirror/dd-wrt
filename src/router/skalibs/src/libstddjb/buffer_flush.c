/* ISC license. */

#include <sys/uio.h>
#include <skalibs/cbuffer.h>
#include <skalibs/buffer.h>

int buffer_flush (buffer *b)
{
  for (;;)
  {
    struct iovec v[2] ;
    ssize_t r ;
    buffer_rpeek(b, v) ;
    if (!v[0].iov_len && !v[1].iov_len) break ;
    r = (*b->op)(b->fd, v, 2) ;
    if (r <= 0) return 0 ;
    cbuffer_RSEEK(&b->c, r) ;
  }
  return 1 ;
}
