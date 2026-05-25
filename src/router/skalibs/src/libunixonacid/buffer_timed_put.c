/* ISC license. */

#include <stddef.h>

#include <skalibs/buffer.h>
#include <skalibs/unix-timed.h>

size_t buffer_timed_put (buffer *b, char const *s, size_t len, tain const *deadline, tain *stamp)
{
  size_t w = 0 ;
  for (;;)
  {
    w += buffer_putnoflush(b, s + w, len - w) ;
    if (w >= len || !buffer_timed_flush(b, deadline, stamp)) break ;
  }
  return w ;
}
