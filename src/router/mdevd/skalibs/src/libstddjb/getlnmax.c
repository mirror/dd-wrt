/* ISC license. */

#include <sys/uio.h>
#include <errno.h>

#include <skalibs/buffer.h>
#include <skalibs/siovec.h>
#include <skalibs/skamisc.h>

int getlnmax (buffer *b, char *d, size_t max, size_t *w, char sep)
{
  if (*w > max) return (errno = EINVAL, -1) ;
  for (;;)
  {
    size_t len = buffer_len(b) ;
    struct iovec v[2] ;
    size_t pos ;
    ssize_t r ;
    buffer_rpeek(b, v) ;
    if (len > max - *w) len = max - *w ;
    pos = siovec_bytechr(v, 2, sep) ;
    if (pos > len) pos = len ;
    r = pos < len ; pos += r ;
    buffer_getnofill(b, d + *w, pos) ; *w += pos ;
    if (r) return 1 ;
    if (*w >= max) return (errno = ERANGE, -1) ;
    r = buffer_fill(b) ;
    if (r <= 0) return r ;
  }
}
