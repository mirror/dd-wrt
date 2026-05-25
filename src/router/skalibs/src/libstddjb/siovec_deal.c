/* ISC license. */

#include <sys/uio.h>
#include <string.h>
#include <skalibs/siovec.h>

size_t siovec_deal (struct iovec const *vj, unsigned int nj, struct iovec const *vi, unsigned int ni)
{
  size_t w = 0 ;
  size_t wi = 0 ;
  size_t wj = 0 ;
  unsigned int i = 0 ;
  unsigned int j = 0 ;
  while (i < ni && j < nj)
  {
    size_t tor = vi[i].iov_len - wi ;
    size_t tow = vj[j].iov_len - wj ;
    size_t len = tor < tow ? tor : tow ;
    memmove((char *)vj[j].iov_base + wj, (char const *)vi[i].iov_base + wi, len) ;
    wi += len ; wj += len ; w += len ;
    if (wi >= vi[i].iov_len) { wi = 0 ; i++ ; }
    if (wj >= vj[j].iov_len) { wj = 0 ; j++ ; }
  }
  return w ;
}
