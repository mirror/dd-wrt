/* ISC license. */

#include <string.h>
#include <skalibs/buffer.h>

int buffer_putsallnoflush (buffer *b, char const *s)
{
  return buffer_putallnoflush(b, s, strlen(s)) ;
}
