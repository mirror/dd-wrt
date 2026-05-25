/* ISC license. */

#include <unistd.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>

#define BSIZE 65536

off_t fd_cat (int from, int to)
{
  off_t n = 0 ;
  char buf[BSIZE] ;
  for (;;)
  {
    ssize_t r = fd_read(from, buf, BSIZE) ;
    if (r == -1) return -1 ;
    if (!r) break ;
    if (allwrite(to, buf, r) < r) return -1 ;
    n += r ;
  }
  return n ;
}
