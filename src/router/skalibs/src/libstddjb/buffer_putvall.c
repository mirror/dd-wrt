/* ISC license. */

#include <sys/uio.h>
#include <errno.h>
#include <skalibs/siovec.h>
#include <skalibs/buffer.h>

int buffer_putvall (buffer *b, struct iovec const *v, unsigned int n, size_t *written)
{
  size_t len = siovec_len(v, n) ;
  size_t w = n ;
  struct iovec vv[n ? n : 1] ;
  if (*written > len) return (errno = EINVAL, 0) ;
  while (w--) vv[w] = v[w] ;
  w = *written ;
  for (;;)
  {
    siovec_seek(vv, n, w) ;
    w = buffer_putvnoflush(b, vv, n) ;
    *written += w ;
    if (*written >= len) return 1 ;
    buffer_flush(b) ;
    if (buffer_isfull(b)) return 0 ;
  }
}
