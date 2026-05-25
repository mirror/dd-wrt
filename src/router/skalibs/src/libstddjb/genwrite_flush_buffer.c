/* ISC license. */

#include <skalibs/buffer.h>
#include <skalibs/genwrite.h>

int genwrite_flush_buffer (void *target)
{
  return buffer_flush((buffer *)target) ;
}
