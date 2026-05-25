/* ISC license. */

#include <skalibs/buffer.h>

ssize_t buffer_put (buffer *b, char const *s, size_t len)
{
  size_t w = 0 ;
  if (!buffer_putall(b, s, len, &w)) return -1 ;
  return w ;
}
