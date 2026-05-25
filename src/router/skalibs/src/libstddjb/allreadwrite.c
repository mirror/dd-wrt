/* ISC license. */

#include <skalibs/allreadwrite.h>

size_t allreadwrite (io_func_ref op, int fd, char *buf, size_t len)
{
  size_t written = 0 ;
  while (len)
  {
    ssize_t w = (*op)(fd, buf, len) ;
    if (w <= 0) break ;
    written += w ;
    buf += w ;
    len -= w ;
  }
  return written ;
}
