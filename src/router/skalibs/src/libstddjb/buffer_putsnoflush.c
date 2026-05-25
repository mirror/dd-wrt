/* ISC license. */

#include <string.h>
#include <skalibs/buffer.h>

size_t buffer_putsnoflush (buffer *b, char const *s)
{
  return buffer_putnoflush(b, s, strlen(s)) ;
}
