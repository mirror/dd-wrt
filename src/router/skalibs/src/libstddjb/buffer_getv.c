/* ISC license. */

#include <stddef.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>

ssize_t buffer_getv (buffer *b, struct iovec const *v, unsigned int n)
{
  size_t w = 0 ;
  ssize_t r = unsanitize_read(buffer_getvall(b, v, n, &w)) ;
  return r < 0 ? r : w ;
}
