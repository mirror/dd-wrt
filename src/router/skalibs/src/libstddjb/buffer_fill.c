/* ISC license. */

#include <sys/uio.h>
#include <errno.h>
#include <skalibs/buffer.h>

ssize_t buffer_fill (buffer *b)
{
  struct iovec v[2] ;
  ssize_t r ;
  if (buffer_isfull(b)) return (errno = ENOBUFS, -1) ;
  buffer_wpeek(b, v) ;
  r = (*b->op)(b->fd, v, 2) ;
  if (r <= 0) return r ;
  buffer_wseek(b, r) ;
  return r ;
}
