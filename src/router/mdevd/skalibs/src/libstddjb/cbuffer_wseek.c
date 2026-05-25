/* ISC license. */

#include <skalibs/cbuffer.h>

size_t cbuffer_wseek (cbuffer *b, size_t len)
{
  size_t max = cbuffer_available(b) ;
  if (len > max) len = max ; 
  return cbuffer_WSEEK(b, len) ;
}
