/* ISC license. */

#include <string.h>
#include <skalibs/buffer.h>

int buffer_putsall (buffer *b, char const *s, size_t *w)
{
  return buffer_putall(b, s, strlen(s), w) ;
}
