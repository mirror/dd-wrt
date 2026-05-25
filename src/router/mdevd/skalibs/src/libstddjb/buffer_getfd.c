/* ISC license. */

#include <skalibs/buffer.h>

int buffer_getfd (buffer const *b)
{
  return buffer_fd(b) ;
}
