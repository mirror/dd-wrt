/* ISC license. */

#include <sys/uio.h>
#include <skalibs/siovec.h>
#include <skalibs/cbuffer.h>

size_t cbuffer_put (cbuffer *b, char const *s, size_t len)
{
  struct iovec v[2] ;
  size_t w ;
  cbuffer_wpeek(b, v) ;
  w = siovec_scatter(v, 2, s, len) ;
  return cbuffer_WSEEK(b, w) ;
}
