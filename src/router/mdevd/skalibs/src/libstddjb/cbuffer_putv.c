/* ISC license. */

#include <sys/uio.h>
#include <skalibs/siovec.h>
#include <skalibs/cbuffer.h>

size_t cbuffer_putv (cbuffer *b, struct iovec const *v, unsigned int n)
{
  struct iovec vdest[2] ;
  size_t w ;
  cbuffer_wpeek(b, vdest) ;
  w = siovec_deal(vdest, 2, v, n) ;
  return cbuffer_WSEEK(b, w) ;
}
