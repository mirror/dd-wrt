/* ISC license. */

#include <string.h>
#include <sys/uio.h>
#include <skalibs/posixplz.h>
#include <skalibs/siovec.h>

size_t siovec_search (struct iovec const *v, unsigned int n, char const *needle, size_t nlen)
{
  size_t vlen = siovec_len(v, n) ;
  size_t w = 0 ;
  unsigned int i = 0 ;
  for (; i < n ; i++)
  {
    char *p = memmem(v[i].iov_base, v[i].iov_len, needle, nlen) ;
    if (p) return w + (p - (char *)v[i].iov_base) ;
    if (i < n-1 && nlen > 1 && v[i].iov_len)
    {
      size_t prelen = v[i].iov_len < nlen ? v[i].iov_len : nlen ;
      size_t postlen = vlen - w - v[i].iov_len < nlen ? vlen - w - v[i].iov_len : nlen ;
      char buf[prelen + postlen - 2] ;
      memcpy(buf, (char *)v[i].iov_base + v[i].iov_len - prelen + 1, prelen - 1) ;
      siovec_gather(v + i + 1, n - 1 - i, buf + prelen - 1, postlen - 1) ;
      p = memmem(buf, prelen + postlen - 2, needle, nlen) ;
      if (p) return w + v[i].iov_len - prelen + 1 + (p - buf) ;
    }
    w += v[i].iov_len ;
  }
  return w ;
}
