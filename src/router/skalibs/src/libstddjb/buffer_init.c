/* ISC license. */

#include <errno.h>
#include <skalibs/cbuffer.h>
#include <skalibs/buffer.h>

int buffer_init (buffer *b, iov_func_ref op, int fd, char *s, size_t len)
{
  if (!cbuffer_init(&b->c, s, len)) return 0 ;
  b->fd = fd ;
  b->op = op ;
  return 1 ;
}
