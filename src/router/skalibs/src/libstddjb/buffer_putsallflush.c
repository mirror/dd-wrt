/* ISC license. */

#include <string.h>
#include <skalibs/buffer.h>

int buffer_putsallflush (buffer *b, char const *s, size_t *w)
{
  return buffer_putallflush(b, s, strlen(s), w) ;
}
