/* ISC license. */

#include <sys/uio.h>
#include <skalibs/buffer.h>

ssize_t buffer_putv (buffer *b, struct iovec const *v, unsigned int n)
{
  size_t w = 0 ;
  return buffer_putvall(b, v, n, &w) ? (ssize_t)w : -1 ;
}
