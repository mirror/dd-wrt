/* ISC license. */

#include <string.h>
#include <skalibs/buffer.h>

ssize_t buffer_puts (buffer *b, char const *s)
{
  return buffer_put(b, s, strlen(s)) ;
}
