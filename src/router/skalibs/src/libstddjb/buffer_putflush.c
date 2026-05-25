/* ISC license. */

#include <skalibs/buffer.h>

ssize_t buffer_putflush (buffer *b, char const *s, size_t len)
{
  ssize_t r = buffer_put(b, s, len) ;
  if (r < 0) return -1 ;
  if (!buffer_flush(b)) return -1 ;
  return r ;
}
