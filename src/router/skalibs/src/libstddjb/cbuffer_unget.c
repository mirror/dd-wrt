/* ISC license. */

#include <skalibs/cbuffer.h>

size_t cbuffer_unget (cbuffer *b, size_t len)
{
  size_t max = cbuffer_available(b) ;
  if (len > max) len = max ;
  return cbuffer_UNGET(b, len) ;
}
