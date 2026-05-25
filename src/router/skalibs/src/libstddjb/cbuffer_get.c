/* ISC license. */

#include <sys/uio.h>
#include <skalibs/siovec.h>
#include <skalibs/cbuffer.h>

size_t cbuffer_get (cbuffer *b, char *s, size_t len)
{
  struct iovec v[2] ;
  size_t w ;
  cbuffer_rpeek(b, v) ;
  w = siovec_gather(v, 2, s, len) ;
  return cbuffer_RSEEK(b, w) ;
}
