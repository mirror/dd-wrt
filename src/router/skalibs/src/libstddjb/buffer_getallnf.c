/* ISC license. */

#include <errno.h>
#include <skalibs/buffer.h>

int buffer_getallnofill (buffer *b, char *s, size_t len)
{
  size_t r = buffer_getnofill(b, s, len) ;
  if (r < len)
  {
    buffer_unget(b, r) ;
    return (errno = ENOBUFS, 0) ;
  }
  return 1 ;
}
