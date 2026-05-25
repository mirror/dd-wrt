/* ISC license. */

#include <unistd.h>
#include <errno.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>

#define BSIZE 65536

off_t fd_catn (int from, int to, off_t n)
{
  off_t w = 0 ;
  char buf[BSIZE] ;
  while (n >= BSIZE)
  {
    size_t v ;
    ssize_t r = fd_read(from, buf, BSIZE) ;
    if (r == -1) return w ;
    if (!r) return (errno = EPIPE, w) ;
    v = allwrite(to, buf, r) ;
    if (v < r) return w + v ;
    n -= r ; w += r ;
  }

  if (n)
  {
    size_t r = allread(from, buf, n) ;
    size_t v = 0 ;
    if (r) v = allwrite(to, buf, r) ;
    w += v ;
  }
  return w ;
}
