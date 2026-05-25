/* ISC license. */

#include <sys/uio.h>
#include <skalibs/siovec.h>
#include <skalibs/cbuffer.h>

size_t cbuffer_getv (cbuffer *b, struct iovec const *v, unsigned int n)
{
  struct iovec vsrc[2] ;
  size_t w ;
  cbuffer_rpeek(b, vsrc) ;
  w = siovec_deal(v, n, vsrc, 2) ;
  return cbuffer_RSEEK(b, w) ;
}
