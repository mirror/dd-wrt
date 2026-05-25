/* ISC license. */

#include <stddef.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>

ssize_t buffer_get (buffer *b, char *s, size_t len)
{
  size_t w = 0 ;
  ssize_t r = unsanitize_read(buffer_getall(b, s, len, &w)) ;
  return r < 0 ? r : w ;
}
