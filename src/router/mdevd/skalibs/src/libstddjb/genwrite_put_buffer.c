/* ISC license. */

#include <skalibs/buffer.h>
#include <skalibs/genwrite.h>

ssize_t genwrite_put_buffer (void *target, char const *s, size_t len)
{
  return buffer_put((buffer *)target, s, len) ;
}
