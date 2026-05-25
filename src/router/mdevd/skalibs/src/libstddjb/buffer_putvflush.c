/* ISC license. */

#include <sys/uio.h>
#include <skalibs/buffer.h>

ssize_t buffer_putvflush (buffer *b, struct iovec const *v, unsigned int n)
{
  ssize_t r = buffer_putv(b, v, n) ;
  if (r < 0) return -1 ;
  if (!buffer_flush(b)) return -1 ;
  return r ;
}
