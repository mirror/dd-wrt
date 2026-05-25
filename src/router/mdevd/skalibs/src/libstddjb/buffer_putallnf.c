/* ISC license. */

#include <errno.h>
#include <skalibs/buffer.h>

int buffer_putallnoflush (buffer *b, char const *s, size_t len)
{
  ssize_t r = buffer_putnoflush(b, s, len) ;
  if (r < len)
  {
    buffer_unput(b, r) ;
    return (errno = ENOBUFS, 0) ;
  }
  return 1 ;
}
