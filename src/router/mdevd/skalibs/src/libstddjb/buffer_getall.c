/* ISC license. */

#include <errno.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>

int buffer_getall (buffer *b, char *buf, size_t len, size_t *w)
{
  if (*w > len) return (errno = EINVAL, -1) ;
  for (;;)
  {
    ssize_t r ;
    *w += buffer_getnofill(b, buf + *w, len - *w) ;
    if (*w >= len) break ;
    r = sanitize_read(buffer_fill(b)) ;
    if (r <= 0) return r ;
  }
  return 1 ;
}
