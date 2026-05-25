/* ISC license. */

#include <string.h>

#include <skalibs/unix-timed.h>

size_t buffer_timed_puts (buffer *b, char const *s, tain const *deadline, tain *stamp)
{
  return buffer_timed_put(b, s, strlen(s), deadline, stamp) ;
}
