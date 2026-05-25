/* ISC license. */

#include <sys/uio.h>
#include <errno.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>
#include <skalibs/siovec.h>

int buffer_getvall (buffer *b, struct iovec const *v, unsigned int n, size_t *written)
{
  size_t len = siovec_len(v, n) ;
  struct iovec vv[n ? n : 1] ;
  if (*written > len) return (errno = EINVAL, -1) ;
  {
    unsigned int i = n ;
    while (i--) vv[i] = v[i] ;
  }
  siovec_seek(vv, n, *written) ;
  for (;;)
  {
    ssize_t r ;
    size_t w = buffer_getvnofill(b, vv, n) ;
    *written += w ;
    if (*written >= len) break ;
    siovec_seek(vv, n, w) ;
    r = sanitize_read(buffer_fill(b)) ;
    if (r <= 0) return r ;
  }
  return 1 ;
}
